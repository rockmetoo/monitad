/**
 * osinfo.h
 *
 */

#ifndef __osinfo_h__
#define __osinfo_h__

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"
#include "log.h"

#define MONITA_MAX_CPU		18

/* Nobody should really be using more than 4 processors.
   Yes we are :)
   Nobody should really be using more than 32 processors.
*/
#define MONITA_NCPU		1024

unsigned long	getLinuxVersionCode();
void			realNumberOfCPU(int* nRealCPU, int* nCPU);

#ifdef __cplusplus
}
#endif
#endif
