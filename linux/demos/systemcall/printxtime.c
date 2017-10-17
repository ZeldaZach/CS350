#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <linux/unistd.h>
#include <time.h>

#define __sys_my_xtime 326

int main(void)
{
	int lnReturnValue;
	struct timespec loTimeStruct;

	while (1)
	{
		lnReturnValue = syscall(__sys_my_xtime, &loTimeStruct);
		printf("ret=%d errno=%d val=%ld nano sec\n", lnReturnValue, errno, loTimeStruct.tv_sec*1000000000UL + loTimeStruct.tv_nsec);
		sleep(1);
	}

	return 0;
}
