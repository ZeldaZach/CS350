#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <linux/unistd.h>

int main(void)
{
	int ret;
	while (1)
	{
		ret = syscall(326);
		printf("ret=%d errno=%d\n", ret, errno);
		sleep(1);
	}

	return 0;
}
