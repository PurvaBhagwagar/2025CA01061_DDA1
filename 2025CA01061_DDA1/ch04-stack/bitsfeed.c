// SPDX-License-Identifier: GPL-2.0
#define pr_fmt(fmt) "bitsfeed: " fmt

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/jiffies.h>
#include <linux/ratelimit.h>

#include "bitscore.h"

static int nsamples = 5;
module_param(nsamples, int, 0444);
MODULE_PARM_DESC(nsamples,
		 "number of jiffies-derived samples to feed (clamped 1-50, default 5)");

static int __init bitsfeed_init(void)
{
	int i, count, avg;

	if (nsamples < 1 || nsamples > 50) {
		pr_warn_ratelimited("nsamples=%d out of range, clamping to [1,50]\n",
				    nsamples);
		if (nsamples < 1)
			nsamples = 1;
		if (nsamples > 50)
			nsamples = 50;
	}

	for (i = 0; i < nsamples; i++) {
		int value = (int)((jiffies_to_msecs(jiffies) + i * 37) % 100);

		bitscore_add_sample(value);
	}

	count = bitscore_sample_count();
	avg = bitscore_sample_avg();
	pr_info("fed %d samples, total count=%d avg=%d\n", nsamples, count, avg);

	return 0;
}

static void __exit bitsfeed_exit(void)
{
	pr_info("unloaded\n");
}

module_init(bitsfeed_init);
module_exit(bitsfeed_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Purva Bhagwagar, BITS ID 2025CA01061");
MODULE_DESCRIPTION("bitsfeed: feeds jiffies-derived samples into the bitscore API");
