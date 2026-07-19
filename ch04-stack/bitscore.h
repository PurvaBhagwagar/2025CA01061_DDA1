#ifndef _BITSCORE_H
#define _BITSCORE_H

void bitscore_add_sample(int value);
int bitscore_sample_count(void);
int bitscore_sample_avg(void);

#endif /* _BITSCORE_H */
