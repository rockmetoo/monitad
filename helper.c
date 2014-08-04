/**
 *
 */

#include "helper.h"
#include "iniparser.h"
#include "oauth/oauth.h"

#include <curl/curl.h>
#include <curl/easy.h>

int commandExecute(const char* cmdstring)
{
	pid_t				pid;
	int					status;
	struct sigaction	ignore, saveintr, savequit;
	sigset_t			chldmask, savemask;

	// always a command processor with UNIX
	if(cmdstring == NULL) return(1);

	// ignore SIGINT and SIGQUIT
	ignore.sa_handler = SIG_IGN;
	sigemptyset(&ignore.sa_mask);
	ignore.sa_flags = 0;
	if(sigaction(SIGINT, &ignore, &saveintr) < 0) return(-1);
	if(sigaction(SIGQUIT, &ignore, &savequit) < 0) return(-1);
	// now block SIGCHLD
	sigemptyset(&chldmask);
	sigaddset(&chldmask, SIGCHLD);
	if(sigprocmask(SIG_BLOCK, &chldmask, &savemask) < 0) return(-1);

	if((pid = fork()) < 0){
		// probably out of processes
		status = -1;
	}else if(pid == 0){ // child
		// restore previous signal actions & reset signal mask
		sigaction(SIGINT, &saveintr, NULL);
		sigaction(SIGQUIT, &savequit, NULL);
		sigprocmask(SIG_SETMASK, &savemask, NULL);

		execl("/bin/sh", "sh", "-c", cmdstring,(char *)0);
		// exec error
		_exit(127);
	}else{ // parent
		while(waitpid(pid, &status, 0) < 0)
			if(errno != EINTR){
				// error other than EINTR from waitpid()
				status = -1;
				break;
			}
	}
	// restore previous signal actions & reset signal mask
	if(sigaction(SIGINT, &saveintr, NULL) < 0) return(-1);
	if(sigaction(SIGQUIT, &savequit, NULL) < 0) return(-1);
	if(sigprocmask(SIG_SETMASK, &savemask, NULL) < 0) return(-1);
	return(status);
}

/**
 * POSIX level write lock
 */
int writeLockFile(int nFd)
{
	struct flock stF1;
	stF1.l_type = F_WRLCK;
	stF1.l_start = 0;
	stF1.l_whence = SEEK_SET;
	stF1.l_len = 0;
	return (fcntl(nFd, F_SETLK, &stF1));
}

/**
 * int nFd = safeOpen("test.dat", O_CREAT | O_RDWR, 0640);
 */
int safeOpen(const char* pszPath, int nFlags, mode_t mtMode)
{
	int nFd;
	nFd = open(pszPath, nFlags, mtMode);
	if(nFd == -1)
	{
		logError("safeOpen: open %s failed in %s at %d\n", pszPath, __FILE__, __LINE__);
	}
	return nFd;
}

ssize_t safeBwrite(int nFd, const uint8_t* byBuff, size_t stCount)
{
	ssize_t sstRetval;
	sstRetval = write(nFd, byBuff, stCount);
	if(sstRetval == -1)
	{
		logError("safeBwrite: write %l bytes to fd %d failed in %s at %d\n", stCount, nFd, __FILE__, __LINE__);
	}
	return sstRetval;
}

/**
 * Like strdup but make sure the resulting string perfectly duplicated.
 */
char* safeStrdup(const char* pszString)
{
	char* pszRetval;
	pszRetval = strdup(pszString);
	if(!pszRetval)
	{
		logError("safeStrdup: dup %s failed in %s at %d\n", pszString, __FILE__, __LINE__);
		return (NULL);
	}
	return pszRetval;
}

/**
 * Function: thread-safe implementation of sleep
 * Purpose: generate a delay mechanism in second
 */
unsigned sSleep(unsigned int nSecs)
{
	int nI;
	unsigned uSlept;
	time_t ttStart, ttEnd;
	struct timeval tv;
	tv.tv_sec = nSecs;
	tv.tv_usec = 0;
	time(&ttStart);
	nI = select(0, NULL, NULL, NULL, &tv);
	if(nI == 0) return (0);
	time(&ttEnd);
	uSlept = ttEnd - ttStart;
	if(uSlept >= nSecs) return (0);
	return (nSecs - uSlept);
}

/**
 * generate a delay mechanism in milisecond
 */
void mSleep(int nMsecs)
{
	struct timeval tv;
	tv.tv_sec = (int)(nMsecs / 1000);
	tv.tv_usec = (nMsecs % 1000) * 1000;
	select(0, NULL, NULL, NULL, &tv);
}

/**
 * generate a delay mechanism in microsecond
 */
void uSleep(unsigned int nUsecs)
{
	struct timeval	tv;
	tv.tv_sec = nUsecs / 1000000;
	tv.tv_usec = nUsecs % 1000000;
	select(0, NULL, NULL, NULL, &tv);
}

/**
 * calculate microsecond
 */
double getMicroSecond()
{
	struct timeval	tvCurrent;
	struct timezone	tzZone;
	//Get current.
	if(gettimeofday(&tvCurrent, &tzZone) == 0)
	{
		//Calculate microsecond.
		return (double)(tvCurrent.tv_usec / MICRO_SEC);
	}
	//if gettimeofday is failed then
	return (0.0);
}

/**
 * calculate microsecond
 */
double getMilliSecond()
{
	struct timeval	tvCurrent;
	struct timezone	tzZone;
	//Get current.
	if(gettimeofday(&tvCurrent, &tzZone) == 0)
	{
		//Calculate microsecond.
		return (double)(tvCurrent.tv_usec / MILLI_SEC);
	}
	//if gettimeofday is failed then
	return (0.0);
}

/**
 * Check if the file exist on the system without open it
 * @pszFilename A path to the file to check
 * @return TRUE if file exist otherwise FALSE
 */
int fileExist(char* pszFilename)
{
	struct stat buff;
	ASSERT(pszFilename);
	return (stat(pszFilename, &buff) == 0);
}

/**
 * Check if the file is a regular file
 * @param pszFilename A path to the file to check
 * @return TRUE if file exist and is a regular file, otherwise FALSE
 */
int isFile(char* pszFilename)
{
	struct stat buff;
	ASSERT(pszFilename);
	return (stat(pszFilename, &buff) == 0 && S_ISREG(buff.st_mode));
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

/**
 * Replace all occurrences of the sub-string old in the string src
 * with the sub-string new. The method is case sensitive for the
 * sub-strings new and old. The string parameter src must be an
 * allocated string, not a character array.
 * @param pszHayStack An allocated string reference (e.g. &string)
 * @param pszSearch The old sub-string
 * @param pszReplace The new sub-string
 * @return pszHayStack where all occurrences of the old sub-string are
 * replaced with the new sub-string.
 * MUST FREE in caller side and MUST place pszReplace with some data.
 */
char* strReplace(const char* pszSearch, const char* pszReplace, const char* pszHayStack)
{
	char* tok		= NULL;
	char* newstr	= NULL;
	char* oldstr	= NULL;
	char* head		= NULL;

	/* if either pszSearch or pszReplace is NULL, duplicate string a let caller handle it */
	if(pszSearch == NULL || pszReplace == NULL) return strdup(pszHayStack);

	newstr = strdup(pszHayStack);
	head = newstr;
	while((tok = strstr(head, pszSearch)))
	{
		oldstr = newstr;
		newstr = xMalloc(strlen(oldstr) - strlen(pszSearch) + strlen(pszReplace) + 1);
		/*failed to alloc mem, free old string and return NULL */
		if(newstr == NULL)
		{
			FREE(oldstr);
			return NULL;
		}
		memcpy(newstr, oldstr, tok - oldstr);
		memcpy(newstr + (tok - oldstr), pszReplace, strlen(pszReplace));
		memcpy(newstr + (tok - oldstr) + strlen(pszReplace), tok + strlen(pszSearch), strlen(oldstr) - strlen(pszSearch) - (tok - oldstr));
		memset(newstr + strlen(oldstr) - strlen(pszSearch) + strlen(pszReplace) , 0, 1);
		/* move back head right after the last pszReplace */
		head = newstr + (tok - oldstr) + strlen(pszReplace);
		FREE(oldstr);
	}

	return newstr;
}

int readTheWholeFile(char* pszFname, char* pszArray)
{
	FILE* 	fp;
	char	szTemp[515] = {0};
	int		nSize = 0;
	// Can we open the file?
	if((fp = fopen(pszFname, "r")) == NULL)
	{
		return (-1);
	}

	// Start reading the file
	while((fgets(szTemp, 512, fp) != (char*)NULL))
	{
		nSize += sprintf(&pszArray[nSize], "%s", szTemp);
	}

	fclose(fp);
	return nSize;
}

void* xMalloc(int nI)
{
	void* p;
    p = (void*) malloc(nI);
    //Some malloc's don't return a valid pointer if you malloc(0), so check for that only if necessary.
#if ! HAVE_MALLOC
		if(nI == 0)
		{
			logError("xMalloc passed a broken malloc 0 in %s at %d", __FILE__, __LINE__);
			return NULL;
		}
#endif
    if(p == NULL)
    {
    	logError("xMalloc Failed to initialize in %s at %d", __FILE__, __LINE__);
    	return NULL;
    }
    return p;
}

uint32_t string2Int(uint8_t* byString, size_t stStringSize)
{
	uint32_t	dwRetval;
	uint32_t	dwI;
	for(dwI = 0, dwRetval = 0; dwI < stStringSize; dwI++, byString++)
		dwRetval = (dwRetval << 8) | *byString;
	return dwRetval;
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
