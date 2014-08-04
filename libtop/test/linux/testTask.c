#include <unistd.h>
#include "task.h"

int main(int argc, char **argv)
{
	monita_task buf;

	getTaskUsage(&buf);

	printf("Task: total %lu, running %lu, slepping %lu, stopped %lu, zombie %lu\n",
		(unsigned long) buf.total,
		(unsigned long) buf.running,
		(unsigned long) buf.sleeping,
		(unsigned long) buf.stopped,
		(unsigned long) buf.zombie
	);

	return 0;
}
