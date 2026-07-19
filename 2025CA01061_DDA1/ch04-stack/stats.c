// SPDX-License-Identifier: GPL-2.0
#define pr_fmt(fmt) "bitscore: " fmt

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>

#include "bitscore.h"
#include "bitscore_internal.h"

/**
 * bitscore_sample_count - number of samples currently stored
 */
int bitscore_sample_count(void)
{
	unsigned long flags;
	int count;

	spin_lock_irqsave(&bitscore_lock, flags);
	count = bitscore_nr_samples;
	spin_unlock_irqrestore(&bitscore_lock, flags);

	return count;
}
EXPORT_SYMBOL_GPL(bitscore_sample_count);

/**
 * bitscore_sample_avg - integer average of all stored samples
 *
 * Returns 0 if the store is currently empty.
 */
int bitscore_sample_avg(void)
{
	unsigned long flags;
	int i, sum = 0, count;

	spin_lock_irqsave(&bitscore_lock, flags);
	count = bitscore_nr_samples;
	for (i = 0; i < count; i++)
		sum += bitscore_samples[i];
	spin_unlock_irqrestore(&bitscore_lock, flags);

	if (count == 0)
		return 0;

	return sum / count;
}
EXPORT_SYMBOL_GPL(bitscore_sample_avg);
