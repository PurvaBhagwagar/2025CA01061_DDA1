// SPDX-License-Identifier: GPL-2.0
#define pr_fmt(fmt) "bits7seg: " fmt

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/bitops.h>
#include <linux/version.h>

#define BITS7SEG_NR_DEVS   4
#define REG_CTRL_EN        BIT(0)
#define REG_CTRL_BLANK     BIT(1)
#define REG_STATUS_READY   BIT(0)

/* Emulated 3-register block, one per minor, allocated with kzalloc */
struct seg_regs {
	u8 digit;   /* REG_DIGIT  : 0-9                       */
	u8 ctrl;    /* REG_CTRL   : EN=bit0, BLANK=bit1        */
	u8 status;  /* REG_STATUS : READY=bit0, set at probe   */
};

struct seg_dev {
	struct cdev cdev;
	struct device *device;
	struct seg_regs *regs;
	struct mutex lock;      /* protects regs */
	int minor;
};

static dev_t bits7seg_devt;
static struct class *bits7seg_class;
static struct seg_dev *bits7seg_devs;

static int bits7seg_open(struct inode *inode, struct file *filp)
{
	struct seg_dev *dev = container_of(inode->i_cdev, struct seg_dev, cdev);

	filp->private_data = dev;
	return 0;
}

static int bits7seg_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static ssize_t bits7seg_read(struct file *filp, char __user *buf,
			      size_t count, loff_t *ppos)
{
	struct seg_dev *dev = filp->private_data;
	char kbuf[64];
	int len;
	u8 digit, en, blank;

	mutex_lock(&dev->lock);
	digit = dev->regs->digit;
	en    = !!(dev->regs->ctrl & REG_CTRL_EN);
	blank = !!(dev->regs->ctrl & REG_CTRL_BLANK);
	mutex_unlock(&dev->lock);

	len = scnprintf(kbuf, sizeof(kbuf), "digit=%u en=%u blank=%u\n",
			digit, en, blank);

	if (*ppos >= len)
		return 0;

	if (count > len - *ppos)
		count = len - *ppos;

	if (copy_to_user(buf, kbuf + *ppos, count))
		return -EFAULT;

	*ppos += count;
	return count;
}

static ssize_t bits7seg_write(struct file *filp, const char __user *buf,
			       size_t count, loff_t *ppos)
{
	struct seg_dev *dev = filp->private_data;
	char c;

	if (count < 1)
		return -EINVAL;

	if (copy_from_user(&c, buf, 1))
		return -EFAULT;

	if (c < '0' || c > '9')
		return -EINVAL;

	mutex_lock(&dev->lock);
	dev->regs->digit = c - '0';
	mutex_unlock(&dev->lock);

	return count;
}

static const struct file_operations bits7seg_fops = {
	.owner   = THIS_MODULE,
	.open    = bits7seg_open,
	.release = bits7seg_release,
	.read    = bits7seg_read,
	.write   = bits7seg_write,
};

static ssize_t digit_show(struct device *device, struct device_attribute *attr,
			   char *buf)
{
	struct seg_dev *dev = dev_get_drvdata(device);
	u8 digit;

	mutex_lock(&dev->lock);
	digit = dev->regs->digit;
	mutex_unlock(&dev->lock);

	return sysfs_emit(buf, "%u\n", digit);
}

static ssize_t digit_store(struct device *device, struct device_attribute *attr,
			    const char *buf, size_t count)
{
	struct seg_dev *dev = dev_get_drvdata(device);
	u8 val;
	int ret;

	ret = kstrtou8(buf, 10, &val);
	if (ret)
		return ret;

	if (val > 9)
		return -EINVAL;

	mutex_lock(&dev->lock);
	dev->regs->digit = val;
	mutex_unlock(&dev->lock);

	return count;
}

static DEVICE_ATTR_RW(digit);

static struct class *bits7seg_class_create(void)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 4, 0)
	return class_create("bits7seg");
#else
	return class_create(THIS_MODULE, "bits7seg");
#endif
}

static void bits7seg_teardown(int upto)
{
	int i;

	for (i = upto; i >= 0; i--) {
		struct seg_dev *dev = &bits7seg_devs[i];

		device_remove_file(dev->device, &dev_attr_digit);
		device_destroy(bits7seg_class, MKDEV(MAJOR(bits7seg_devt), i));
		cdev_del(&dev->cdev);
		kfree(dev->regs);
	}
}

static int __init bits7seg_init(void)
{
	int ret, i;

	ret = alloc_chrdev_region(&bits7seg_devt, 0, BITS7SEG_NR_DEVS, "bits7seg");
	if (ret < 0) {
		pr_err("alloc_chrdev_region failed: %d\n", ret);
		return ret;
	}

	bits7seg_class = bits7seg_class_create();
	if (IS_ERR(bits7seg_class)) {
		ret = PTR_ERR(bits7seg_class);
		goto err_unregister;
	}

	bits7seg_devs = kcalloc(BITS7SEG_NR_DEVS, sizeof(*bits7seg_devs), GFP_KERNEL);
	if (!bits7seg_devs) {
		ret = -ENOMEM;
		goto err_class;
	}

	for (i = 0; i < BITS7SEG_NR_DEVS; i++) {
		struct seg_dev *dev = &bits7seg_devs[i];

		dev->minor = i;
		mutex_init(&dev->lock);

		dev->regs = kzalloc(sizeof(*dev->regs), GFP_KERNEL);
		if (!dev->regs) {
			ret = -ENOMEM;
			goto err_teardown;
		}
		dev->regs->ctrl   |= REG_CTRL_EN;
		dev->regs->status |= REG_STATUS_READY;

		cdev_init(&dev->cdev, &bits7seg_fops);
		dev->cdev.owner = THIS_MODULE;

		ret = cdev_add(&dev->cdev, MKDEV(MAJOR(bits7seg_devt), i), 1);
		if (ret) {
			kfree(dev->regs);
			goto err_teardown;
		}

		dev->device = device_create(bits7seg_class, NULL,
					     MKDEV(MAJOR(bits7seg_devt), i),
					     dev, "bits7seg%d", i);
		if (IS_ERR(dev->device)) {
			ret = PTR_ERR(dev->device);
			cdev_del(&dev->cdev);
			kfree(dev->regs);
			goto err_teardown;
		}

		ret = device_create_file(dev->device, &dev_attr_digit);
		if (ret) {
			device_destroy(bits7seg_class, MKDEV(MAJOR(bits7seg_devt), i));
			cdev_del(&dev->cdev);
			kfree(dev->regs);
			goto err_teardown;
		}
	}

	pr_info("loaded, major=%d, %d devices ready\n",
		MAJOR(bits7seg_devt), BITS7SEG_NR_DEVS);
	return 0;

err_teardown:
	bits7seg_teardown(i - 1);
	kfree(bits7seg_devs);
err_class:
	class_destroy(bits7seg_class);
err_unregister:
	unregister_chrdev_region(bits7seg_devt, BITS7SEG_NR_DEVS);
	return ret;
}

static void __exit bits7seg_exit(void)
{
	bits7seg_teardown(BITS7SEG_NR_DEVS - 1);
	kfree(bits7seg_devs);
	class_destroy(bits7seg_class);
	unregister_chrdev_region(bits7seg_devt, BITS7SEG_NR_DEVS);

	pr_info("unloaded\n");
}

module_init(bits7seg_init);
module_exit(bits7seg_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Purva Bhagwagar, BITS ID 2025CA01061");
MODULE_DESCRIPTION("bits7seg: emulated 4-minor seven-segment display char driver");
