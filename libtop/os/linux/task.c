/**
 *
 */

#include "task.h"
#include "log.h"
#include "helper.h"

#define FILENAME		"/proc"

void getTaskUsage(monita_task* buf)
{
	char buffer [BUFSIZ];

	memset(buf, 0, sizeof* buf);

	DIR *dp;
	struct dirent* ep;

	dp = opendir(FILENAME);

	if(dp != NULL)
	{
		while(ep = readdir(dp))
		{
			if(ep->d_name[0] != '.' && ep->d_name[strlen(ep->d_name)-1] != '~')
			{
				if(isNumeric(ep->d_name))
				{
					char szTaskFileName[1024];
					sprintf(szTaskFileName, "%s/%s/status", FILENAME, ep->d_name);

					int nRet = fileToBuffer(buffer, sizeof buffer, szTaskFileName);

					if(nRet == 0)
					{
						buf->total += 1;

						switch(getScaledTask(buffer, "State:"))
						{
							case MONITA_TASK_RUNNING:
								buf->running++;
								break;

							case MONITA_TASK_SLEEPING:
							case MONITA_TASK_DEAD:
								buf->sleeping++;
								break;

							case MONITA_TASK_STOPPED:
								buf->stopped++;
								break;

							case MONITA_TASK_ZOMBIED:
								buf->zombie++;
								break;
						}
					}
				}
			}
		}

		(void) closedir(dp);
	}
	else
	{
		logError("Couldn't read task data [%d]\n", __LINE__);
	}
}
