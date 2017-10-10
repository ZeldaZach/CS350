#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <linux/unistd.h>
#include <time.h>

#define __sys_x_time 326

int main(void)
{
	int lnReturnValue;
	struct timespec loTimeSet;

	while (1)
	{
		ret = syscall(__sys_x_time, &loTimeSet);
		printf("ret=%d errno=%d val=%d\n", lnReturnValue, errno, loTimeSet);
		sleep(1);
	}

	return 0;
}
