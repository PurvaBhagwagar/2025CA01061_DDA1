#ifndef _BITSCORE_INTERNAL_H
#define _BITSCORE_INTERNAL_H

#include <linux/spinlock.h>

#define BITSCORE_MAX_SAMPLES 64

extern int bitscore_samples[BITSCORE_MAX_SAMPLES];
extern int bitscore_nr_samples;
extern spinlock_t bitscore_lock;

#endif /* _BITSCORE_INTERNAL_H */
