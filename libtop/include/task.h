#ifndef __task_h__
#define __task_h__

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"


#define MONITA_TASK_RUNNING		0
#define MONITA_TASK_SLEEPING	1
#define MONITA_TASK_DEAD		2
#define MONITA_TASK_STOPPED		3
#define MONITA_TASK_ZOMBIED		4

typedef struct _monita_task	monita_task;

struct _monita_task
{
	uint64_t total;
	uint64_t running;
	uint64_t sleeping;
	uint64_t stopped;
	uint64_t zombie;
};

void getTaskUsage(monita_task* buf);


#ifdef __cplusplus
}
#endif
#endif
