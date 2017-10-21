#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define N 20
#define MAX_LENGTH 150

int main()
{
	struct timeval gtodTimes[N];
	char *procClockTimes[N];
	int i;
	int fd;
	int bytes_read;

	fd = open("/dev/mytime", O_RDONLY);
	if (fd < 0)
	{
		printf("Opening driver failed!\n");
		exit(1);
	}

	for (i = 0; i < N; i++)
	{
		gettimeofday(&gtodTimes[i], 0);

		procClockTimes[i] = malloc(4000);
		bytes_read = read(fd, procClockTimes[i], MAX_LENGTH);
	}

	close(fd);


	for (i = 0; i < N; i++)
	{
		printf("Library:%ld %ld\nKernel:%s\n",
			gtodTimes[i].tv_sec,
			gtodTimes[i].tv_usec,
			procClockTimes[i]
		);
	}

	return 0;
}

