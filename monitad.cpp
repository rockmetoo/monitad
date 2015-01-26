/*
 * watch -n 1 'netstat -an | grep :6536 | wc -l'
 * ngrep -q -d lo -W byline host localhost and port 6536
 * sudo sysctl -w net.core.somaxconn=4096
 * sudo sysctl -w net.ipv4.tcp_tw_recycle=1
 * sudo sysctl -w net.ipv4.tcp_tw_reuse=1
 * sudo ulimit -s unlimited
 * sudo ulimit -n 4096
 * sudo ulimit -u 2048
 * sudo ulimit -v unlimited
 * sudo ulimit -x unlimited
 * sudo ulimit -m unlimited
 */

#include "monitad.h"

#include <stdio.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include <exception>
#include <sys/resource.h>

#include "log.h"
#include "helper.h"
#include "hook.h"
#include "iniparser.h"
#include "cpu.h"
#include "mem.h"
#include "swap.h"
#include "task.h"
#include "oauth.h"

#define BUF_SIZE		10*1024
#define ASCIILINESZ		(1024)

char					BASE_DIRECTORY[1024];
char					COCKPIT_MONITA_POST_URL[256];


static					sigset_t				mask;

bool					bIsDaemon				= TRUE;
pid_t					gDaemonPid;
uid_t					gDaemonUid;
gid_t					gDaemonGid;
int						nCountThreadToRun 		= 0;
char*					MONITA_PID_FILE			= new char[1024];
char*					MONITA_CONFIG_FILE		= new char[1024];
char*					MONITA_CONSUMER_KEY		= new char[256];
char*					MONITA_CONSUMER_SECRET	= new char[256];

char*					MONITA_TOKEN_KEY		= new char[256];
char*					MONITA_TOKEN_SECRET		= new char[256];


void sigHUP(int nSigno);
void sigTERM(int nSigno);
void sigQUIT(int nSigno);
void monitaCPU(void* argument);
void monitaMEMORY(void* argument);

void runTheWholePlot()
{
	int		ret;
	int		nJ;
	void*	statusp;

	// XXX: IMPORTANT - start doing actual test job here
	HOOKER hooker;

	// Create a hooker of 'nCountThreadToRun' thread workers
	if((hooker = newHooker(nCountThreadToRun, nCountThreadToRun + 5, FALSE)) == NULL)
	{
		logError("Failed to create a thread pool struct [%d]\n", __LINE__);
		return;
	}

	monita_cpu cpuBuff;
	monita_mem memBuff;

	// XXX: IMPORTANT - CPU monita
	if(MONITA_CPU_ACTIVE)
	{
		ret = hookerAdd(hooker, monitaCPU, &cpuBuff);

		if(ret == FALSE)
		{
			logError("An error had occurred while monitoring CPU [%d]\n", __LINE__);
			return;
		}
	}

	// XXX: IMPORTANT - MEMOERY monita
	if(MONITA_MEMOERY_ACTIVE)
	{
		ret = hookerAdd(hooker, monitaMEMORY, &memBuff);

		if(ret == -1)
		{
			logError("An error had occurred while monitoring MEMOERY [%d]\n", __LINE__);
			return;
		}
	}

	hookerJoin(hooker, TRUE, &statusp);

	// OAUTH post string variable
	char szPostString[2048] = {0};
	unsigned int nSize		= 0;
	char*	pszReqUrl		= NULL;
	char*	pszReply		= NULL;
	char*	pszPostarg		= NULL;

	// make round time 1st
	time_t ttNow		= (unsigned)time(NULL);
	int modOfTimestamp	= ttNow % 60;
	ttNow				= ttNow - modOfTimestamp;

	if(modOfTimestamp == 59)
	{
		nSize += sprintf(
			&szPostString[nSize], "%s?data={t0:%lu, t1:%lu,", COCKPIT_MONITA_POST_URL, ttNow, (ttNow + 60)
		);
	}
	else
	{
		nSize += sprintf(&szPostString[nSize], "%s?data={t0:%lu,", COCKPIT_MONITA_POST_URL, ttNow);
	}

	if(MONITA_CPU_ACTIVE)
	{
		nSize += sprintf(
			&szPostString[nSize],
			"cpu:{total:%lu,user:%lu,sys:%lu,nice:%lu,idle:%lu,frequency:%lu},",
			cpuBuff.total, cpuBuff.user, cpuBuff.sys, cpuBuff.nice, cpuBuff.idle, cpuBuff.frequency
		);
	}

	if(MONITA_MEMOERY_ACTIVE)
	{
		nSize += sprintf(
			&szPostString[nSize],
			"mem:{total:%lu,used:%lu,free:%lu,shared:%lu,buffer:%lu,cached:%lu,user:%lu,locked:%lu},",
			memBuff.total, memBuff.used, memBuff.free, memBuff.shared, memBuff.buffer,
			memBuff.cached, memBuff.user, memBuff.locked
		);
	}

	// XXX: IMPORTANT - collect all the data from all the threads that were spawned by the run.
	//for(nJ=0; nJ<((hookerGetTotal(hooker) > nCountThreadToRun || hookerGetTotal(hooker)==0) ? nCountThreadToRun : hookerGetTotal(hooker)); nJ++)
	//{

	//	logDebug("PLEASE ADD NETWORK COMMUNICATION WITH COCKPIT SERVER\n");
	//}

	nSize += sprintf(&szPostString[nSize-1], "}");

	// XXX: IMPORTANT - cleanup hooker
	hookerDestroy(hooker);

	pszReqUrl = oauth_sign_url2(
		szPostString, &pszPostarg, OA_HMAC, NULL, MONITA_CONSUMER_KEY, MONITA_CONSUMER_SECRET,
		MONITA_TOKEN_KEY, MONITA_TOKEN_SECRET
	);

	pszReply = oauth_http_post(pszReqUrl, pszPostarg, 5);

	if(strcmp(pszReply, "") == 0)
	{
		logError("Possible server error! Please contact at <%s>\n", CONTACT_MAIL);

		if(pszReqUrl)	FREE(pszReqUrl);
		if(pszPostarg)	FREE(pszPostarg);
	}
	else
	{
		/*logDebug(
			"Catch Reply from Server: %s   CKEY: %s, CSECRET: %s,  TOKEN: %s  TOKENSECRET: %s \n",
			pszReply, MONITA_CONSUMER_KEY, MONITA_CONSUMER_SECRET, MONITA_TOKEN_KEY, MONITA_TOKEN_SECRET
		);*/

		logInfo("Catch Reply from Server: %s\n", pszReply);

		if(pszReply)	FREE(pszReply);
		if(pszReqUrl)	FREE(pszReqUrl);
		if(pszPostarg)	FREE(pszPostarg);
	}

	// XXX: IMPORTANT - take a short nap for cosmetic effect. This does NOT affect performance stats.
	uSleep(10000);

	return;
}

/**
 * daemon daemonize()
 */
static int daemoNize(void)
{
	int nFd;
	int nI;
	pid_t pid;
	struct rlimit srR1;
	struct sigaction srSa;
	// clear file creation mask
	umask(0);
	// get maximum number of file descriptors
	if(getrlimit(RLIMIT_NOFILE, &srR1) < 0)
	{
		logError("monita daemon failed to maximum no. of file descriptor in %s at %d\n", __FILE__, __LINE__);
		return (-1);
	}
	// become a session leader to lose controlling TTY
	if((pid = fork()) < 0)
	{
		logError("monita daemon failed to become session leader in %s at %d\n", __FILE__, __LINE__);
		return (-1);
	}
	else if(pid != 0)
	{//parent
		_exit(0);
	}

	setsid();

	// ensure future opens won't allocate controlling TTYs
	srSa.sa_handler = SIG_IGN;
	sigemptyset(&srSa.sa_mask);
	srSa.sa_flags = 0;
	if(sigaction(SIGHUP, &srSa, NULL) < 0)
	{
		//TODO: LOG something
		return (-1);
	}
	if((pid = fork()) < 0)
	{
		//TODO: LOG something
		return (-1);
	}
	else if(pid != 0)
	{//parent
		_exit(0);
	}

	int status = chdir("/");

	// close all open file descriptors
	if(srR1.rlim_max == RLIM_INFINITY) srR1.rlim_max = 1024;
	for(nI=0; nI<srR1.rlim_max; nI++) close(nI);
	if((nFd = open(PATH_DEVNULL, O_RDWR)) == -1)
	{
		logError("monita daemon failed to open /dev/null in %s at %d\n", __FILE__, __LINE__);
		return (-1);
	}
	(void)dup2(nFd, STDIN_FILENO);
	(void)dup2(nFd, STDOUT_FILENO);
	(void)dup2(nFd, STDERR_FILENO);
	return (0);
}

/**
 * Perform any termination cleanup and exit the server with the exit status
 * This function does not return.
 */
static void daemonExit(int nCode)
{
	struct stat st;

	if(stat((const char*)MONITA_PID_FILE, &st) == 0)
	{
		logNotice("monitad unlinking PID file %s in %s at %d\n", MONITA_PID_FILE, __FILE__, __LINE__);

		if(unlink((const char*)MONITA_PID_FILE) == -1)
		{
			logCritical("monitad failed to unlink PID file %s in %s at %d\n", MONITA_PID_FILE, __FILE__, __LINE__);
		}
	}

	logNotice("monitad exiting... at %s [%d]\n", __FILE__, __LINE__);

	exit(nCode);
}

/**
 * daemonWritePid(void)
 * Write the process ID of the server into the file specified in <path>.
 * The file is automatically deleted on a call to daemonExit().
 */
static int daemonWritePid(void){

	int nFd;
	char szBuff[16] = {0};

	nFd = safeOpen((const char*)MONITA_PID_FILE, O_RDWR | O_CREAT, LOCKMODE);

	if(nFd < 0)
	{
		logError("monitad failed to open \"%s\" in %s at %d\n", MONITA_PID_FILE, __FILE__, __LINE__);
		return (-1);
	}

	if(writeLockFile(nFd) < 0)
	{
		if(errno == EACCES || errno == EAGAIN)
		{
			close(nFd);
			return (1);
		}

		logCritical("monitad can't lock %s in %s at %d\n", MONITA_PID_FILE, __FILE__, __LINE__);
		exit(1);
	}

	int status = ftruncate(nFd, 0);
	gDaemonPid = getpid();
	sprintf(szBuff, "%ld", (long)gDaemonPid);
	safeBwrite(nFd, (uint8_t*)szBuff, strlen(szBuff)+1);
	return (0);
}

/**
 * Drop server privileges by changing our effective group and user IDs to
 * the ones specified.
 * Returns 0 on success, or -1 on failure.
 */
static int daemonPrivDrop(void)
{
	 // Start by the group, otherwise we can't call setegid() if we've
	 // already dropped user privileges.
	if((getegid() != gDaemonGid) && (setegid(gDaemonGid) == -1))
	{
		logError("cockpitd failed to set group ID to %u in %s at %d\n", gDaemonGid, __FILE__, __LINE__);
		return (-1);
	}

	if((geteuid() != gDaemonUid) && (seteuid(gDaemonUid) == -1))
	{
		logError("cockpitd failed to set user ID to %u in %s at %d\n", gDaemonUid, __FILE__, __LINE__);
		return (-1);
	}

	logInfo("cockpitd dropped privileges to %u:%u in %s at %d\n" , gDaemonUid, gDaemonGid, __FILE__, __LINE__);
	return (0);
}

/**
 * Deal with signals
 * LOCKING: acquires and releases configlock
 */
static void* signalThread(void* arg)
{
	int	nError, nSigno;

	nError = sigwait(&mask, &nSigno);

	if(nError != 0)
	{
		logCritical("signalThread - cockpitd sigwait failed: %s, in %s at %d\n", strerror(nError), __FILE__, __LINE__);
		daemonExit(1);
	}

	switch(nSigno)
	{
		case SIGHUP:
			sigHUP(nSigno);
			break;

		case SIGTERM:
			sigTERM(nSigno);
			break;

		case SIGQUIT:
			sigQUIT(nSigno);
			break;

		default:
			logInfo("signalThread - unexpected signal %d\n", nSigno);
			break;
	}

	return (void*)1;
}

static int parseServiceConfigFile(char* pszFileName)
{
    dictionary* ini;

    int             nI;
    char*			pszSecname = NULL;
    char			szKeym[ASCIILINESZ+1];

    ini = iniparser_load(pszFileName);

    if(ini == NULL)
    {
    	logError("Cannot parse monitad config file %s [%d]\n", pszFileName, __LINE__);
        daemonExit(1);
    }

	getcwd(BASE_DIRECTORY, 1024);

    int nSec = iniparser_getnsec(ini);

    for(nI=0 ; nI<nSec ; nI++)
    {
    	pszSecname = iniparser_getsecname(ini, nI);

		if(strstr(pszSecname, "cpu"))
		{
			PLOT_KEY(szKeym, pszSecname, "active")
			MONITA_CPU_ACTIVE = iniparser_getint(ini, szKeym, 0);

			PLOT_KEY(szKeym, pszSecname, "offline")
			MONITA_CPU_OFFLINE = iniparser_getint(ini, szKeym, 0);

			if(MONITA_CPU_ACTIVE) nCountThreadToRun++;
		}
		else if(strstr(pszSecname, "memory"))
		{
			PLOT_KEY(szKeym, pszSecname, "active")
			MONITA_MEMOERY_ACTIVE = iniparser_getint(ini, szKeym, 0);

			PLOT_KEY(szKeym, pszSecname, "offline")
			MONITA_MEMOERY_OFFLINE = iniparser_getint(ini, szKeym, 0);

			if(MONITA_MEMOERY_ACTIVE) nCountThreadToRun++;
		}
		else if(strstr(pszSecname, "swap"))
		{
			PLOT_KEY(szKeym, pszSecname, "active")
			MONITA_SWAP_ACTIVE = iniparser_getint(ini, szKeym, 0);

			PLOT_KEY(szKeym, pszSecname, "offline")
			MONITA_SWAP_OFFLINE = iniparser_getint(ini, szKeym, 0);

			if(MONITA_SWAP_ACTIVE) nCountThreadToRun++;
		}
		else if(strstr(pszSecname, "task"))
		{
			PLOT_KEY(szKeym, pszSecname, "active")
			MONITA_TASK_ACTIVE = iniparser_getint(ini, szKeym, 0);

			PLOT_KEY(szKeym, pszSecname, "offline")
			MONITA_TASK_OFFLINE = iniparser_getint(ini, szKeym, 0);

			if(MONITA_TASK_ACTIVE) nCountThreadToRun++;
		}
		else if(strstr(pszSecname, "process"))
		{
			// TODO:
		}
		else if(strstr(pszSecname, "io"))
		{
			PLOT_KEY(szKeym, pszSecname, "active")
			MONITA_IO_ACTIVE = iniparser_getint(ini, szKeym, 0);

			PLOT_KEY(szKeym, pszSecname, "offline")
			MONITA_IO_OFFLINE = iniparser_getint(ini, szKeym, 0);

			if(MONITA_IO_ACTIVE) nCountThreadToRun++;
		}
		else if(strstr(pszSecname, "network"))
		{
			PLOT_KEY(szKeym, pszSecname, "active")
			MONITA_NETWORK_ACTIVE = iniparser_getint(ini, szKeym, 0);

			PLOT_KEY(szKeym, pszSecname, "offline")
			MONITA_NETWORK_OFFLINE = iniparser_getint(ini, szKeym, 0);

			if(MONITA_NETWORK_ACTIVE) nCountThreadToRun++;
		}
		else if(strstr(pszSecname, "port"))
		{
			// TODO:
		}
		else if(strstr(pszSecname, "monitad"))
		{
			PLOT_KEY(szKeym, pszSecname, "ckey")
			strcpy(MONITA_CONSUMER_KEY, iniparser_getstring(ini, szKeym, NULL));

			PLOT_KEY(szKeym, pszSecname, "csecret")
			strcpy(MONITA_CONSUMER_SECRET, iniparser_getstring(ini, szKeym, NULL));

			PLOT_KEY(szKeym, pszSecname, "token")
			strcpy(MONITA_TOKEN_KEY, iniparser_getstring(ini, szKeym, NULL));

			PLOT_KEY(szKeym, pszSecname, "tokenSecret")
			strcpy(MONITA_TOKEN_SECRET, iniparser_getstring(ini, szKeym, NULL));
		}
    }

    iniparser_freedict(ini);
    return 1;
}

void sigHUP(int nSigno)
{
	logInfo("signalThread - monitad receive SIGHUP signal %s\n", strsignal(nSigno));
	//TODO: Schedule to re-read the configuration file.
	//reReadConf();
}

void sigTERM(int nSigno)
{
	logInfo("signalThread - monitad terminate with signal %s\n", strsignal(nSigno));
	daemonExit(1);
}

void sigQUIT(int nSigno)
{
	logInfo("signalThread - monitad terminate with signal %s\n", strsignal(nSigno));
	daemonExit(1);
}

void monitaCPU(void* argument)
{
	try
	{
		monita_cpu* cpuBuff = (monita_cpu*) argument;

		getCPUUsage(cpuBuff);

		logDebug("CPU: total %lu, user %lu, sys %lu, nice %lu, idle %lu, freq %lu\n",
			(unsigned long) cpuBuff->total,
			(unsigned long) cpuBuff->user,
			(unsigned long) cpuBuff->sys,
			(unsigned long) cpuBuff->nice,
			(unsigned long) cpuBuff->idle,
			(unsigned long) cpuBuff->frequency
		);
	}
	catch(int e)
	{
		return;
	}

	return;
}

void monitaMEMORY(void* argument)
{
	try
	{
		monita_mem* memBuff = (monita_mem*) argument;

		getMemoryUsage(memBuff);

		logDebug("Memory: %lu, %lu, %lu, %lu, %lu, %lu, %lu, %lu\n",
			(unsigned long) memBuff->total,
			(unsigned long) memBuff->used,
			(unsigned long) memBuff->free,
			(unsigned long) memBuff->shared,
			(unsigned long) memBuff->buffer,
			(unsigned long) memBuff->cached,
			(unsigned long) memBuff->user,
			(unsigned long) memBuff->locked
		);
	}
	catch(int e)
	{
		return;
	}

	return;
}

/**
 * display help message
 */
void help(void)
{
	printf("monitad by Autobloom <info@autobloom.com>\n");
	printf(
		" --pid			Set pid file. Default at /var/run/monitad.pid\n"
		" --config		Set config file. Default at /etc/monitad.conf\n"
		" --help        -h -?               Print this help\n"
	);
}

/**
 *
 */
void processOptions()
{
	static const struct option longOptions[] =
	{
		{"pid",		required_argument,	NULL, 0		},
		{"config",	required_argument,	NULL, 0		},
		{"help",	0, 					NULL, 'h'	},
		{0,			0, 					NULL, 0		}
	};


	char* pszPid	= NULL;
	char* pszConfig	= NULL;

	// default settings
	strcpy(MONITA_PID_FILE, "/var/run/monitad.pid");
	strcpy(MONITA_CONFIG_FILE, "/etc/monitad.conf");

	for(;;)
	{
		int nRes;
		int optionIndex = 0;
		struct option* pszOpt = 0;

		nRes = getopt_long(m_argc, m_argv, "h?", longOptions, &optionIndex);

		if(nRes == -1) break;

		switch(nRes){

			case '?' :

			case 'h' :

				help();
				exit(1);
				break;

			case 0 :

				pszOpt = (struct option*)&(longOptions[optionIndex]);

				if(strcmp(pszOpt->name, "pid") == 0)
				{
					if((pszPid = safeStrdup(optarg)) == NULL)
					{
						strcpy(MONITA_PID_FILE, "/var/run/monitad.pid");
					}
					else
					{
						strcpy(MONITA_PID_FILE, pszPid);
					}

					pszPid = NULL;
				}

				if(strcmp(pszOpt->name, "config") == 0)
				{
					if((pszConfig = safeStrdup(optarg)) == NULL)
					{
						strcpy(MONITA_CONFIG_FILE, "/etc/monitad.conf");
					}
					else
					{
						strcpy(MONITA_CONFIG_FILE, pszConfig);
					}

					pszConfig = NULL;
				}

				break;

			default :
				//help();
				exit(1);
				break;
		}
	}
}

void init()
{
	// become a daemon and fork
	if((bIsDaemon == TRUE) && (daemoNize() == -1))
	{
		logError("Failed to become a monitad in %s at %d\n", __FILE__, __LINE__);
		exit(1);
	}

	// make sure only one copy of the daemon is running
	if(daemonWritePid())
	{
		logError("monitad already running in file %s at %d\n", __FILE__, __LINE__);
	}

	// drop privileges
	if((gDaemonUid > 0) || (gDaemonGid > 0))
	{
		if(daemonPrivDrop() == -1) daemonExit(1);
	}
	else
	{
		gDaemonGid = getgid();
		gDaemonUid = getuid();
	}
}

int main(int argc, char** argv)
{
	pthread_t	ptSignalId;

	m_argc = argc;
	m_argv = argv;

	processOptions();

	int nParseStatus = parseServiceConfigFile(MONITA_CONFIG_FILE);

	init();

	// XXX: IMPORTANT - for development environment please use local.*
	//sprintf(COCKPIT_MONITA_POST_URL, "https://monita.autobloom.com/recvMonitaData.php");
	sprintf(COCKPIT_MONITA_POST_URL, "http://local.cockpit.com:80/recvMonitaData.php");

	// only except HUP, TERM and QUIT signal
	sigemptyset(&mask);
	sigaddset(&mask, SIGHUP);
	sigaddset(&mask, SIGTERM);
	sigaddset(&mask, SIGQUIT);
	pthread_sigmask(SIG_BLOCK, &mask, NULL);
	pthread_create(&ptSignalId, NULL, signalThread, NULL);

	while(true)
	{
		sSleep(60);

		runTheWholePlot();
	}

	pause();

	return 0;
}
