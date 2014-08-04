#include <unistd.h>
#include "swap.h"

int main(int argc, char **argv)
{
	monita_swap buf;

	getSwapUsage(&buf);

	printf ("SWAP: total %lu, used %lu, free %lu, cached %lu, pagein %lu, pageout %lu\n",
			(unsigned long) buf.total,
			(unsigned long) buf.used,
			(unsigned long) buf.free,
			(unsigned long) buf.cached,
			(unsigned long) buf.pagein,
			(unsigned long) buf.pageout);

	return 0;
}
