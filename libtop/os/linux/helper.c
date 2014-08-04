/**
 *
 */

#include "helper.h"

/*
 * Read functions
 */
enum TRY_FILE_TO_BUFFER
{
	TRY_FILE_TO_BUFFER_OK	= 0,
	TRY_FILE_TO_BUFFER_OPEN = -1,
	TRY_FILE_TO_BUFFER_READ = -2
};

/*
 * Doesn't handle bufsiz == 0
 */
int tryFileToBuffer(char* buffer, size_t bufsiz, const char* format, ...)
{
	char	path[4096];
	int		fd;
	size_t	len		= 0;
	ssize_t	nread	= 0;
	va_list	pa;

	if(bufsiz <= sizeof(char*))
	{
		logWarning("buffer size of %lu looks bad [%d]\n", (unsigned long)bufsiz, __LINE__);
	}

	va_start(pa, format);

	// C99 also provides vsnprintf
	vsnprintf(path, sizeof path, format, pa);
	va_end(pa);

	// reserve 1 for trailing NULL
	bufsiz--;
	buffer [0] = '\0';

	if((fd = open (path, O_RDONLY)) < 0) return TRY_FILE_TO_BUFFER_OPEN;

	while(len < bufsiz)
	{
		nread = read(fd, buffer + len, bufsiz - len);

		if(nread < 0)
		{
			if(errno == EINTR) continue;
			else break;
		}

		len += nread;

		if(nread == 0) break;
	}

	close (fd);

	if(nread < 0) return TRY_FILE_TO_BUFFER_READ;

	buffer [len] = '\0';

	return TRY_FILE_TO_BUFFER_OK;
}


int fileToBuffer(char* buffer, size_t bufsiz, const char* filename)
{
	switch(tryFileToBuffer(buffer, bufsiz, "%s", filename))
	{
		case TRY_FILE_TO_BUFFER_OPEN:
			return TRY_FILE_TO_BUFFER_OPEN;

		case TRY_FILE_TO_BUFFER_READ:
			return TRY_FILE_TO_BUFFER_READ;

		case TRY_FILE_TO_BUFFER_OK:
			return TRY_FILE_TO_BUFFER_OK;
	}

	// XXX: IMPORTANT - control never comes here
	return TRY_FILE_TO_BUFFER_OK;
}

unsigned long long getScaled(const char* buffer, const char* key)
{
	const char*			ptr = buffer;
	char*				next;
	unsigned long long	value;

	if(key)
	{
		if((ptr = strstr(buffer, key))) ptr += strlen(key);
		else
		{
			logWarning("Could not read key '%s' in buffer '%s' [%d]\n", key, buffer, __LINE__);
			return 0;
		}
	}

	value = strtoull(ptr, &next, 0);

	for( ; *next; ++next)
	{
		if(*next == 'k')
		{
			value *= 1024;
			break;
		}
		else if(*next == 'M')
		{
			value *= 1024 * 1024;
			break;
		}
	}

	return value;
}

int getScaledTask(const char* buffer, const char* key)
{
	const char*			ptr = buffer;
	char*				next;
	unsigned long long	value;

	if(key)
	{
		if((ptr = strstr(buffer, key))) ptr += strlen(key);
		else
		{
			logWarning("Could not read key '%s' in buffer '%s' [%d]\n", key, buffer, __LINE__);
			return 0;
		}
	}

	value = strtoull(ptr, &next, 0);

	for( ; *next; ++next)
	{
		if(*next == 'R')
		{
			return MONITA_TASK_RUNNING;
			break;
		}
		else if(*next == 'S')
		{
			return MONITA_TASK_SLEEPING;
			break;
		}
		else if(*next == 'D')
		{
			return MONITA_TASK_DEAD;
			break;
		}
		else if(*next == 'T')
		{
			return MONITA_TASK_STOPPED;
			break;
		}
		else if(*next == 'Z')
		{
			return MONITA_TASK_ZOMBIED;
			break;
		}
	}

	return -1;
}

char* skipToken(const char* p)
{
	p = nextToken(p);

	while(*p && !isspace(*p)) p++;

	p = nextToken(p);

	return (char*)p;
}

void* xMalloc(int nI)
{
	void* p;
    p = (void*) malloc(nI);
    //Some malloc's don't return a valid pointer if you malloc(0), so check for that only if necessary.
#if ! HAVE_MALLOC
		if(nI == 0)
		{
			logError("xMalloc passed a broken malloc 0 in %s [%d]", __FILE__, __LINE__);
			return NULL;
		}
#endif
    if(p == NULL)
    {
    	logError("xMalloc Failed to initialize in %s [%d]", __FILE__, __LINE__);
    	return NULL;
    }
    return p;
}

/**
 * Explode a string
 * @param - pszString source string to be expl@ded
 * @param - pszSep seperator string
 * @param - nItems it's just a counter to contain num. of exploded element(s)
 * @return - character pointer array contain NULL if error or no match pszSep
 * otherwise array of string.
 *
	char* pszTestString = "HELLO boy I am Fine boy thats great boy how about a boy and girl";
	int num_items;
	char** pszReturn = explodeString(pszTestString, "boy", &num_items);
	printf("Explode Elem: %d\n", num_items);
	if(pszReturn != NULL){
		for(nI=0; nI< num_items; nI++){
			printf("Result: %s\n", pszReturn[nI]);
		}
	}
	freeExplodedString(pszReturn);
 */
char** explodeString(char* pszString, char* pszSep, int* nItems)
{
    int nI;
    int nNumItems;
    char** pszRetArray;
    char* pszPtr;
    char* pszPtr1;
    char* pszPtr2;

    if(nItems != NULL) *nItems = 0;
    if(!pszString || !pszString[0]) return NULL;
    //calculate number of items
    pszPtr = pszString;
    nI = 1;
    int bIsSepExist = FALSE;
    while((pszPtr = strstr(pszPtr, pszSep)))
    {
    	bIsSepExist = TRUE;
        while(strchr(pszSep, pszPtr[0]) != NULL)
            pszPtr++;
        nI++;
    }
    if(!bIsSepExist)
    {
    	*nItems = 0;
    	return NULL;
    }
    nNumItems = nI;
    pszRetArray = (char**) xMalloc((nNumItems + 1) * sizeof(char *));
    if(NULL == pszRetArray)
    {
    	*nItems = 0;
    	logError("Unable to allocate memory at %d in %s\n", __LINE__, __FILE__);
    	return NULL;
    }
    pszPtr1 = pszString;
    pszPtr2 = pszString;
    for(nI = 0; nI<nNumItems; nI++)
    {
        while(strchr(pszSep, pszPtr1[0]) != NULL) pszPtr1++;
        if(nI == (nNumItems - 1) || (pszPtr2 = strstr(pszPtr1, pszSep)) == NULL)
            if((pszPtr2 = strchr(pszPtr1, '\r')) == NULL)
                if((pszPtr2 = strchr(pszPtr1, '\n')) == NULL)
                    pszPtr2 = strchr(pszPtr1, '\0');
        if((pszPtr1 == NULL) || (pszPtr2 == NULL))
        {
            pszRetArray[nI] = NULL;
        }
        else
        {
            if(pszPtr2 - pszPtr1 > 0)
            {
                pszRetArray[nI] = (char *) xMalloc((pszPtr2 - pszPtr1 + 1) * sizeof(char));
                pszRetArray[nI] = strncpy(pszRetArray[nI], pszPtr1, pszPtr2 - pszPtr1);
                pszRetArray[nI][pszPtr2 - pszPtr1] = '\0';
                pszPtr1 = ++pszPtr2;
            }
            else
            {
                pszRetArray[nI] = NULL;
            }
        }
    }
    pszRetArray[nI] = NULL;
    if(nItems != NULL) *nItems = nI;

    return pszRetArray;
}

/**
 * free_exploded_string: free an exploded string
 */
void freeExplodedString(char** pszString)
{
    int nI;
    if(pszString)
    {
    	for(nI=0; pszString[nI]; nI++) FREE(pszString[nI]);
        FREE(pszString);
    }
}

int checkCPULine(const char* line, unsigned i)
{
	char start[10];

	snprintf(start, sizeof start, "cpu%u", i);

	return (strncmp(line, start, strlen(start)) == 0) ? 1 : 0;
}

/**
 * Check if this is a directory.
 * @param pszDirName An absolute  directory path
 * @return TRUE if directory exist and is a regular directory, otherwise
 * FALSE
 */
int isDirectory(char* pszDirName)
{
	struct stat buff;
	ASSERT(pszDirName);
	return (stat(pszDirName, &buff) == 0 && S_ISDIR(buff.st_mode));
}

int hasSysFS(void)
{
	static int init;
	static int sysfs;

	if(!init)
	{
		sysfs = isDirectory("/sys");
		init = TRUE;
	}

	return sysfs;
}
