// SPDX-License-Identifier: GPL-2.0
#define pr_fmt(fmt) "bitscore: " fmt

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/spinlock.h>

#include "bitscore.h"
#include "bitscore_internal.h"

/* Shared in-kernel sample store, accessed by core.c and stats.c */
int bitscore_samples[BITSCORE_MAX_SAMPLES];
int bitscore_nr_samples;
DEFINE_SPINLOCK(bitscore_lock);

/**
 * bitscore_add_sample - append a sample to the in-kernel store
 * @value: sample value to record
 *
 * When the store is full, the oldest sample is dropped so the most
 * recent BITSCORE_MAX_SAMPLES values are always kept.
 */
void bitscore_add_sample(int value)
{
	unsigned long flags;

	spin_lock_irqsave(&bitscore_lock, flags);

	if (bitscore_nr_samples < BITSCORE_MAX_SAMPLES) {
		bitscore_samples[bitscore_nr_samples++] = value;
	} else {
		int i;

		for (i = 0; i < BITSCORE_MAX_SAMPLES - 1; i++)
			bitscore_samples[i] = bitscore_samples[i + 1];
		bitscore_samples[BITSCORE_MAX_SAMPLES - 1] = value;
	}

	spin_unlock_irqrestore(&bitscore_lock, flags);
}
EXPORT_SYMBOL_GPL(bitscore_add_sample);

static int __init bitscore_init(void)
{
	pr_info("loaded, sample store ready (capacity=%d)\n",
		BITSCORE_MAX_SAMPLES);
	return 0;
}

static void __exit bitscore_exit(void)
{
	pr_info("unloaded\n");
}

module_init(bitscore_init);
module_exit(bitscore_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Purva Bhagwagar, BITS ID 2025CA01061");
MODULE_DESCRIPTION("bitscore: in-kernel sample store exporting add/count/avg API");
