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

#include "osinfo.h"
#include "log.h"
#include "helper.h"

/* =====================================================
 * Linux kernel version information for procps utilities
 * Copyright (c) 1996 Charles Blake <cblake@bbn.com>
 */
#include <sys/utsname.h>

#define FILENAME		"/proc/stat"
#define STAT_BUFSIZ     81920

unsigned long getLinuxVersionCode()
{
	struct utsname uts;

	// cleared in case sscanf() < 3
	unsigned x = 0;
	unsigned y = 0;
	unsigned z = 0;

	// failure most likely implies impending death
	if(uname(&uts) == -1)
	{
		logError("uname() failed [%d]\n", __LINE__);
	}

	if(sscanf(uts.release, "%u.%u.%u", &x, &y, &z) < 3)
	{
		logWarning(
			"Non-standard uts for running kernel:\nrelease %s=%u.%u.%u gives version code %d\n",
			uts.release, x, y, z, LINUX_VERSION_CODE(x,y,z)
		);
	}

	if(LINUX_VERSION_CODE(x, y, z) >= LINUX_VERSION_CODE(2, 6, 0) && !hasSysFS())
	{
		logWarning("You're running a 2.6 kernel without /sys. You should mount it [%d]\n", __LINE__);
	}

    return LINUX_VERSION_CODE(x, y, z);
}

void realNumberOfCPU(int* nRealCPU, int* nCPU)
{
	char buffer [STAT_BUFSIZ];
	char* p = buffer;

	fileToBuffer(buffer, sizeof buffer, FILENAME);

	// cpu
	p = skipLine(p);

	for(nRealCPU = 0; ; nRealCPU++)
	{
		if(!checkCPULine(p, nRealCPU))
		{
			nRealCPU--;
			break;
		}

		p = skipLine(p);
	}

	nCPU = MIN(MONITA_NCPU - 1, nRealCPU);
}
