#include <unistd.h>
#include "mem.h"

int main(int argc, char **argv)
{
	monita_mem buf;

	getMemoryUsage(&buf);

	printf("Memory: %lu, %lu, %lu, %lu, %lu, %lu, %lu, %lu\n",
		(unsigned long) buf.total,
		(unsigned long) buf.used,
		(unsigned long) buf.free,
		(unsigned long) buf.shared,
		(unsigned long) buf.buffer,
		(unsigned long) buf.cached,
		(unsigned long) buf.user,
		(unsigned long) buf.locked
	);

	return 0;
}
