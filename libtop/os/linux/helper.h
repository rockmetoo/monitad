/**
 * helper.h
 *
 */

#ifndef __helper_h__
#define __helper_h__

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"
#include "log.h"


#define MONITA_TASK_RUNNING		0
#define MONITA_TASK_SLEEPING	1
#define MONITA_TASK_DEAD		2
#define MONITA_TASK_STOPPED		3
#define MONITA_TASK_ZOMBIED		4

#define BUF_SIZE 10*1024


static inline char* nextToken(const char* p)
{
	while(isspace(*p)) p++;
	return (char*) p;
}

static inline char* skipLine(const char* p)
{
	while(*p && *p != '\n') p++;
	return (char *) (*p ? p+1 : p);
}

static inline int checkCPULineWarn(const char* line, unsigned i)
{
	int ret;

	ret = checkCPULine(line, i);

	if(!ret) logWarning("'%s' does not start with 'cpu%u' [%d]\n", line, i, __LINE__);

	return ret;
}

static inline int isNumeric(const char* p)
{
	if(*p)
	{
		char c;

		while((c=*p++))
		{
			if (!isdigit(c)) return 0;
		}

		return 1;
	}

	return 0;
}

int					tryFileToBuffer(char* buffer, size_t bufsiz, const char* format, ...);
int					fileToBuffer(char* buffer, size_t bufsiz, const char* filename);
unsigned long long	getScaled(const char* buffer, const char* key);
int					getScaledTask(const char* buffer, const char* key);
char*				skipToken(const char* p);
void*				xMalloc(int nI);

char**				explodeString(char* pszString, char* pszSep, int* nItems);
void				freeExplodedString(char** pszString);
int					checkCPULine(const char* line, unsigned i);
int					isDirectory(char* pszDirName);
int					hasSysFS(void);

#ifdef __cplusplus
}
#endif
#endif
