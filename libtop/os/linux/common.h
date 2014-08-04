#ifndef __common_h__
#define __common_h__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <assert.h>
#include <syslog.h>
#include <malloc.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <stddef.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <dirent.h>

#define	TRUE			1
#define	FALSE			0

#define ASSERT(e) do { if(!(e)) { logCritical("AssertException: " #e \
        " at %s:%d\naborting..\n", __FILE__, __LINE__); abort(); } } while(0)

#define LOCK(mutex) do { pthread_mutex_t *_yymutex = &(mutex); \
        assert(pthread_mutex_lock(_yymutex)==0);

#define END_LOCK 		assert(pthread_mutex_unlock(_yymutex)==0); } while (0)

#define STRERROR		strerror(errno)

#define EVILPTR(a)		(!(a) || (*(a) == '\0'))
#define FREE(p)			((void)(free(p), (p)= 0))
#define FREEP(p)		if(!EVILPTR(p)) free(p)

#define LINUX_VERSION_CODE(x,y,z)   (0x10000*(x) + 0x100*(y) + z)

#endif
