#ifndef __swap_h__
#define __swap_h__

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"

typedef struct _monita_swap	monita_swap;

struct _monita_swap
{
	uint64_t total;
	uint64_t used;
	uint64_t free;
	uint64_t cached;
	uint64_t pagein;
	uint64_t pageout;
};

void getSwapUsage(monita_swap* buf);


#ifdef __cplusplus
}
#endif
#endif
