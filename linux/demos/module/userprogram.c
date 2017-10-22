#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 10
#define MAX_LENGTH 150

int main()
{
	struct timeval gtodTimes[N];
	char *procClockTimes[N];
	int i;
	int fd;
	int bytes_read;

	for (i = 0; i < N; i++)
	{
		procClockTimes[i] = malloc(3000);
		if (procClockTimes[i] == NULL)
		{
			printf("Malloc %d failed!\n", i);
			exit(2);
		}
	}

	fd = open("/dev/mytime", O_RDONLY);
	if (fd < 0)
	{
		printf("Opening driver failed!\n");
		exit(1);
	}

	for (i = 0; i < N; i++)
	{
		gettimeofday(&gtodTimes[i], 0);
		bytes_read = read(fd, procClockTimes[i], MAX_LENGTH);
	}

	close(fd);

	for (i = 0; i < N; i++)
	{
		long lnTimeDifference = 0;
		long lnTime = 0;

		if (procClockTimes[i] == NULL)
		{
			printf("Invalid Kernel at %d!\n", i);
			exit(1);
		}

		char *backup_copy = malloc(sizeof(char) * strlen(procClockTimes[i]));
		if (backup_copy == NULL)
		{
			printf("Malloc failed for %d!\n", i);
			exit(2);
		}
		strcpy(backup_copy, procClockTimes[i]);

		char *token = strtok(backup_copy, " ");
		for (int i = 0; i <= 4; i++)
		{
			if (i == 1)
			{
				lnTimeDifference = atol(token)*1000000000;
			}
			else if (i == 2)
			{
				lnTimeDifference += atol(token);
			}
			else if (i == 3)
			{

				lnTime = atol(token)*1000000000;
			}
			else if (i == 4)
			{
				lnTime += atol(token);
			}

			token = strtok(NULL, " ");
		}

		lnTimeDifference = lnTime - lnTimeDifference;

		free(backup_copy);

		printf("---%d---\nLibrary gettimeofday:%ld %ld\nKernel:\n%sTime Difference: %ld\n",
			i,
			gtodTimes[i].tv_sec,
			gtodTimes[i].tv_usec,
			procClockTimes[i],
			lnTimeDifference
		);

		free(procClockTimes[i]);
	}

	return 0;
}
