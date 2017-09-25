#define _POSIX_SOURCE
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

// Functions header
int main();
void listjobs();
void addBackgroundProc(int);
char* print_status(long);
void stopProcess(int);
void pwd();
void cd(char *);

// Global variables
#define gnMAX_LINE_BUFFER_LENGTH 1024
int ganBackgroundProcIDs[gnMAX_LINE_BUFFER_LENGTH];
int gnCurrentProcessCount = 0;

int main()
{
	// 0 out the background task array
	memset(&ganBackgroundProcIDs[0], 0, sizeof(ganBackgroundProcIDs));

	// This will be the PID of the child fork
	int mnForkPID;

	while (true)
	{
		// Catch the interrupt signal and send it to custom function
		signal(SIGINT, stopProcess);

		// Line that contains STD input.
		char lacLineContent[gnMAX_LINE_BUFFER_LENGTH];
		memset(&lacLineContent[0], 0, sizeof(lacLineContent));

		// Pointer to array of strings, each of which are an argument
		// for the child process command to use. Will also 0 it out
		char *lpsCommandArguments[gnMAX_LINE_BUFFER_LENGTH];
		memset(&lpsCommandArguments[0], '\0', sizeof(lpsCommandArguments));

		// PS1
		printf("minish> ");

		// Attempting to store STDIN content into lacLineContent
		// Will exit the program if this fails
		if (fgets(lacLineContent, gnMAX_LINE_BUFFER_LENGTH, stdin) == false)
		{
			printf("\n");
			continue;
		}

		int lnNextEntry = 0;
		bool lbShouldRunInBackground = false;
		char *lpTempStrToken;

		// This will split up the elements in the STDIN to proper arguments
		lpTempStrToken = strtok(lacLineContent, " \n\t");

		while (lpTempStrToken)
		{
			// If the last element is a '&', then we know we need to run
			// this program in the background.
			if (*lpTempStrToken == '&')
			{
				lbShouldRunInBackground = true;
			}
			else
			{
				// Insert the argument at the next point in the string array
				lpsCommandArguments[lnNextEntry++] = lpTempStrToken;
			}

			// Advance this "iterator" to the next element
			lpTempStrToken = strtok(NULL, " \n\t");
		}
		// Need to insert a NULL at end of list
		lpsCommandArguments[lnNextEntry] = NULL;

		/*
		 * #################
		 * Custom Executions
		 * #################
		 */
		if (lpsCommandArguments[0] == NULL || strcmp(lpsCommandArguments[0], "") == 0)
		{
			// If the STDIN is an empty line, just skip it and wait for more inputs
			continue;
		}
		else if (strcmp(lpsCommandArguments[0], "exit") == 0)
		{
			// Exit command called, time to exit this shell
			for (int i = 0; ganBackgroundProcIDs[i] != 0; i++)
			{
				// Kill all child processes
				kill(ganBackgroundProcIDs[i], SIGTERM);
			}
			printf("Exiting minish\n");
			exit(0);
		}
		else if (strcmp(lpsCommandArguments[0], "listjobs") == 0)
		{
			// If our special "listjobs" command is called, execute it
			listjobs();
			continue;
		}
		else if (strcmp(lpsCommandArguments[0], "pwd") == 0)
		{
			// Get the current working directory then print it
			char *lpGetCWD = malloc(gnMAX_LINE_BUFFER_LENGTH);
			pwd(lpGetCWD);

			printf("%s\n", lpGetCWD);
			free(lpGetCWD);
			continue;
		}
		else if (strcmp(lpsCommandArguments[0], "cd") == 0)
		{
			// Change directory
			cd(lpsCommandArguments[1]);
			continue;
		}
		else if (strcmp(lpsCommandArguments[0], "fg") == 0)
		{
			// Bring process to foreground and wait for completion
			kill((uintptr_t)lpsCommandArguments[1], SIGCONT);
			waitpid((uintptr_t)(lpsCommandArguments[1]), NULL, 0);
		}
		else
		{
	   		/*
	   		 * At this point, we will now fork() the process so the child can
	   		 * execute any commands it needs to and the parent will either
	   		 * wait or continue depending on the options set
	   		 */
			mnForkPID = fork();
		}

		if (mnForkPID == 0) // We're in the child process
		{
			// Execute the command in the child process
			execvp(lpsCommandArguments[0], lpsCommandArguments);
		}
		else // We're in the parent process
		{
			if (lbShouldRunInBackground)
			{
				// Add the child PID to the listjobs array
				printf("[%d] %d\n", gnCurrentProcessCount, mnForkPID);
				addBackgroundProc(mnForkPID);
			}
			else
			{
				waitpid(mnForkPID, NULL, 0);
			}
		}
	}

	return 0;
}

/**
 * Print all jobs the system has fork()'d throughout its life
 * and report the status of each job
 */
void listjobs()
{
	printf("List of backgrounded processes:\n");
	for (int i = 0; ganBackgroundProcIDs[i] != 0; i++)
	{
		// Get the PID of the process
		pid_t lnPIDResult = waitpid(ganBackgroundProcIDs[i], NULL, WNOHANG);

		char *lsProcStatus;
		if (lnPIDResult == 0)
		{
			lsProcStatus = "RUNNING";
		}
		else
		{
			lsProcStatus = "FINISHED";
		}

		printf("Command %d with PID %d Status:%s\n", i+1, ganBackgroundProcIDs[i], lsProcStatus);
	}
}

/**
 * This function will add a child ID process to the global
 * array list. If there is not enough room it will exit.
 * Will attempt to insert at the first empty value
 */
void addBackgroundProc(int anPID)
{
	for (int i = 0; i < (sizeof(ganBackgroundProcIDs) / sizeof(ganBackgroundProcIDs[0])); i++)
	{
		// No process can have PID 0 (except the boot) so this is a fine check
		if (ganBackgroundProcIDs[i] == 0)
		{
			ganBackgroundProcIDs[i] = anPID;
			gnCurrentProcessCount = i+1;
			return;
		}
	}

	printf("Out of space for background tasks!");
	exit(2);
}


/**
 * If a SIGINT is called, this function will be called instead of killing the process
 */
void stopProcess(int anSig)
{
	if (SIGINT == anSig)
	{
		// Sig int detected, just exit the line
		return;
	}
}

/**
 * Print the current working directory
 */
void pwd()
{
	char lasCWD[gnMAX_LINE_BUFFER_LENGTH];

	if (getcwd(lasCWD, sizeof(lasCWD)) != NULL)
	{
		printf("%s", lasCWD);
	}
	else
	{
		printf("Error getting cwd");
		exit(3);
	}
}

/**
 * Change the current working directory
 * Found at https://stackoverflow.com/questions/16094814/implementing-cd-system-call-using-c-if-condition
 */
void cd(char* asPath)
{
	// Save arg as local array (modification easier)
	char lsPath[gnMAX_LINE_BUFFER_LENGTH];
	strcpy(lsPath, asPath);

	if (lsPath[0] == '/')
	{
		// If the path is supposed to start at the top of the system
		// Then it's a simple change to the new directory
		chdir(asPath);
	}
	else
	{
		// So now we're working with a relative path
		// Get the current dir then append to it
		char lasCWD[gnMAX_LINE_BUFFER_LENGTH];
		if (getcwd(lasCWD, sizeof(lasCWD)) != NULL)
		{
			strcat(lasCWD, "/");
			strcat(lasCWD, lsPath);
			chdir(lasCWD);
			return;
		}

		printf("Error getting cwd");
		exit(3);
	}
}
