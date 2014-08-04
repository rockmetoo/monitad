/* Copyright (C) 1998-99 Martin Baulig
   This file is part of LibGTop 1.0.

   Contributed by Martin Baulig <martin@home-of-linux.org>, April 1998.

   LibGTop is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License,
   or (at your option) any later version.

   LibGTop is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
   for more details.

   You should have received a copy of the GNU General Public License
   along with LibGTop; see the file COPYING. If not, write to the
   Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "cpu.h"
#include "log.h"
#include "helper.h"
#include "osinfo.h"


#define FILENAME	"/proc/stat"
#define STAT_BUFSIZ	81920

/**
 * XXX: IMPORTANT - show user, sys, nice, idle, cached
 */
void getCPUUsage(monita_cpu* buf)
{
	char buffer [STAT_BUFSIZ];
	char* p;
	int i;
	int nRealCPU = 0;
	int nCPU = 0;

	unsigned long osVersionCode = getLinuxVersionCode();
	realNumberOfCPU(&nRealCPU, &nCPU);

	memset(buf, 0, sizeof(monita_cpu));

	fileToBuffer(buffer, sizeof buffer, FILENAME);

	int numItems;
	int nI;

	// cpuinfo records are seperated by a blank line
	char** processors = explodeString(buffer, "\n\n", &numItems);

	if(processors != NULL)
	{
		for(nI = 0; nI < MONITA_NCPU && processors[nI] && *processors[nI]; nI++)
		{
			nCPU = nI;
		}
	}

	freeExplodedString(processors);

	// XXX: IMPORTANT - Global CPU result. "cpu"
	p = skipToken(buffer);

	buf->user	= strtoull(p, &p, 0);
	buf->nice	= strtoull(p, &p, 0);
	buf->sys	= strtoull(p, &p, 0);
	buf->idle	= strtoull(p, &p, 0);
	buf->total	= buf->user + buf->nice + buf->sys + buf->idle;

	// 2.6 kernel
	if(osVersionCode >= LINUX_VERSION_CODE(2, 6, 0))
	{
		buf->iowait  = strtoull(p, &p, 0);
		buf->irq     = strtoull(p, &p, 0);
		buf->softirq = strtoull(p, &p, 0);

		buf->total += buf->iowait + buf->irq + buf->softirq;
	}

	buf->frequency	= 100;
	buf->nCPU		= nCPU;
	buf->nRealCPU	= nRealCPU;

	// PER CPU
	for(i = 0; i <= nCPU; i++)
	{
		// move to ^
		p = skipLine(p);

		if(!checkCPULineWarn(p, i)) break;

		// "cpuN"
		p = skipToken(p);

		buf->xcpu_user [i] = strtoull(p, &p, 0);
		buf->xcpu_nice [i] = strtoull(p, &p, 0);
		buf->xcpu_sys  [i] = strtoull(p, &p, 0);
		buf->xcpu_idle [i] = strtoull(p, &p, 0);
		buf->xcpu_total[i] = buf->xcpu_user [i] + buf->xcpu_nice [i] + buf->xcpu_sys  [i] + buf->xcpu_idle [i];

		// 2.6 kernel
		if(osVersionCode >= LINUX_VERSION_CODE(2, 6, 0))
		{
			buf->xcpu_iowait[i]		= strtoull(p, &p, 0);
			buf->xcpu_irq [i]		= strtoull(p, &p, 0);
			buf->xcpu_softirq[i]	= strtoull(p, &p, 0);

			buf->xcpu_total[i]		+= buf->xcpu_iowait [i] + buf->xcpu_irq [i] + buf->xcpu_softirq [i];
		}
	}
}
