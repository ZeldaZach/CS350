#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

int main()
{
	int pfds[2]; // Pipe stuff. [0] = Read, [1] = Write
	int mnBuffer;
	
	// Create a pipe
	if (pipe(pfds) == -1)
	{
		perror("pipe");
		exit(1);
	}
	
	int lnDieRoll = (int)(rand() % 6 + 1);
	
	// Fork the process
	int lnForkPID = fork();
	
	if (lnForkPID < 0)
	{
		perror("fork");
		exit(1);
	}
	
	// Child process
	if (lnForkPID == 0)
	{
		int lnDieRollGuess = (int)(rand() % 6 + 1);
		
		// Close read end of pipe
		close(pfds[0]);
		
		// Send the number to parent via pipe
		if (write(pfds[1], &lnDieRollGuess, 1) <= 0)
		{
			perror("Child");
			exit(1);
		}
	}
	else // Parent process
	{
		// Close write end of pipe
		close(pfds[1]);
		
		if (read(pfds[0], &mnBuffer, 1) <= 0)
		{
			perror("Parent");
			exit(1);
		}
		
		int lnChildRetVal;
		if (mnBuffer == lnDieRoll)
		{
			lnChildRetVal = 0;
		}
		else if (mnBuffer < lnDieRoll)
		{
			lnChildRetVal = -1;
		}
		
		// Close read end of pipe
		//close(pfds[0]);
		
		if (write(pfds[1], &lnChildRetVal, 1) <= 0)
		{
			perror("Parent 2");
			exit(1);
		}
	}
	
}