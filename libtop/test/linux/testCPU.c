#include <unistd.h>
#include "cpu.h"

int main(int argc, char **argv)
{
	monita_cpu buf;

	getCPUUsage(&buf);

	printf ("CPU: total %lu, user %lu, sys %lu, nice %lu, idle %lu, freq %lu\n",
		(unsigned long) buf.total,
		(unsigned long) buf.user,
		(unsigned long) buf.sys,
		(unsigned long) buf.nice,
		(unsigned long) buf.idle,
		(unsigned long) buf.frequency
	);

	printf("Real CPU: %u  Number of CPU: %u\n", buf.nRealCPU + 1, buf.nCPU + 1);

	printf("user: %lu user1: %lu\n", buf.xcpu_user[0], buf.xcpu_user[1]);

	return 0;
}
