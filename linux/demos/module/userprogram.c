#include 

#define N 1500

int main()
{
	struct timeval gtodTimes[N];
	char *procClockTimes[N];
	int i;

	fd = open("/dev/mytime", O_RDONLY);

	for (i = 0; i < N; i++)
	{
		printf("...", gtodTimes[i], procClockTime[i]);
	}

	return 0;
}

