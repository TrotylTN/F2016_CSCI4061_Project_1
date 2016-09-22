/************************
 * util.c
 *
 * utility functions
 *
 ************************/

#include "util.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/***************
 * These functions are just some handy file functions.
 * We have not yet covered opening and reading from files in C,
 * so we're saving you the pain of dealing with it, for now.
 *******/
FILE* file_open(char* filename) {
	FILE* fp = fopen(filename, "r");
	if(fp == NULL) {
		fprintf(stderr, "make4061: %s: No such file or directory.\n", filename);
		exit(1);
	}
	return fp;
}

char* file_getline(char* buffer, FILE* fp) {
	buffer = fgets(buffer, 1024, fp);
	return buffer;
}

//Return -1 if file does not exist
int is_file_exist(char * lpszFileName)
{
	return access(lpszFileName, F_OK); 
}

int get_file_modification_time(char * lpszFileName)
{
	if(is_file_exist(lpszFileName) != -1)
	{
		struct stat buf;
		int nStat = stat(lpszFileName, &buf);
		return buf.st_mtime;
	}
	
	return -1;
}

// Compares the timestamp of two files
int compare_modification_time(char * fileName1, char * fileName2)
{	
	int nTime1 = get_file_modification_time(fileName1);
	int nTime2 = get_file_modification_time(fileName2);

//	printf("%s - %d  :  %s - %d\n", lpsz1, nTime1, lpsz2, nTime2);

	if(nTime1 == -1 || nTime2 == -1)
	{
		return -1;
	}

	if(nTime1 == nTime2)
	{
		return 0;
	}
	else if(nTime1 > nTime2)
	{
		return 1;
	}
	else
	{
		return 2;
	}
}

// makeargv
/* Taken from Unix Systems Programming, Robbins & Robbins, p37 */
int makeargv(const char *s, const char *delimiters, char ***argvp) {
   int error;
   int i;
   int numtokens;
   const char *snew;
   char *t;

   if ((s == NULL) || (delimiters == NULL) || (argvp == NULL)) {
      errno = EINVAL;
      return -1;
   }
   *argvp = NULL;
   snew = s + strspn(s, delimiters);
   if ((t = malloc(strlen(snew) + 1)) == NULL)
      return -1;
   strcpy(t,snew);
   numtokens = 0;
   if (strtok(t, delimiters) != NULL)
      for (numtokens = 1; strtok(NULL, delimiters) != NULL; numtokens++) ;

   if ((*argvp = malloc((numtokens + 1)*sizeof(char *))) == NULL) {
      error = errno;
      free(t);
      errno = error;
      return -1;
   }

   if (numtokens == 0)
      free(t);
   else {
      strcpy(t,snew);
      **argvp = strtok(t,delimiters);
      for (i=1; i<numtokens; i++)
         *((*argvp) +i) = strtok(NULL,delimiters);
   }

   *((*argvp) + numtokens) = NULL;
   return numtokens;
}

void freemakeargv(char **argv) {
   if (argv == NULL)
      return;
   if (*argv != NULL)
      free(*argv);
   free(argv);
}

// Used to find the index of target with targetName = lpszTargetName from the list of targets "t"
int find_target(char * lpszTargetName, target_t * const t, int const nTargetCount)
{
	int i=0;
	for(i=0;i<nTargetCount;i++)
	{
		if(strcmp(lpszTargetName, t[i].szTarget) == 0)
		{
			return i;
		}
	}

	return -1;
}

/* Parsing function to parse the Makefile with name = lpszFileName and return the number of targets.
   "t" will point to the first target. Revise "Arrays and Pointers" section from Recitation 2.
*/
int parse(char * lpszFileName, target_t * const t)
{
	char szLine[1024];
	char * lpszLine = NULL;
	char * lpszTargetName = NULL;
	char * lpszDependency = NULL;
	char **prog_args = NULL;
	FILE * fp = file_open(lpszFileName);
	int nTargetCount = 0;
	int nLine = 0;
	target_t * pTarget = t;
	int nPreviousTarget = 0;

	if(fp == NULL)
	{
		return -1;
	}

	while(file_getline(szLine, fp) != NULL) 
	{
		nLine++;
		// this loop will go through the given file, one line at a time
		// this is where you need to do the work of interpreting
		// each line of the file to be able to deal with it later
		lpszLine = strtok(szLine, "\n"); //Remove newline character at end if there is one

		if(lpszLine == NULL || *lpszLine == '#') //skip if blank or comment
		{
			continue;
		}

		//Remove leading whitespace
		while(*lpszLine == ' ')
		{
			lpszLine++;
		}
		
		//skip if whitespace-only
		if(strlen(lpszLine) <= 0) 
		{
			continue;
		}

		//Multi target is not allowed.
		if(*lpszLine == '\t') //Commmand
		{
			lpszLine++;

			if(strlen(pTarget->szTarget) == 0)
			{
				fprintf(stderr, "%s: line:%d *** specifying multiple commands is not allowed.  Stop.\n", lpszFileName, nLine);
				return -1;
			}

			strcpy(pTarget->szCommand, lpszLine);
			if (makeargv(pTarget->szCommand, " ", &prog_args) == -1) 
			{
				perror("Error parsing command line");
				exit(EXIT_FAILURE);
			}
			pTarget->prog_args = prog_args;
			nPreviousTarget = 0;
			pTarget++;
		}
		else	//Target
		{
			//check : exist Syntax check 
			if(strchr(lpszLine, ':') == NULL)
			{
				fprintf(stderr, "%s: line:%d *** missing separator.  Stop.\n", lpszFileName, nLine);
				return -1;
			}

			//Previous target don't have a command
			if(nPreviousTarget == 1)
			{
				pTarget++;
			}

			//Not currently inside a target, look for a new one
			lpszTargetName = strtok(lpszLine, ":");

			if(lpszTargetName != NULL && strlen(lpszTargetName) > 0)
			{
				strcpy(pTarget->szTarget, lpszTargetName);
				lpszDependency = strtok(NULL, " ");

				while (lpszDependency != NULL)
				{
					strcpy(pTarget->szDependencies[pTarget->nDependencyCount], lpszDependency);
					pTarget->nDependencyCount++;
					lpszDependency = strtok(NULL, " ");
				}

				nTargetCount++;
				nPreviousTarget = 1;
			}
			else //error
			{
				fprintf(stderr, "%s: line:%d *** missing separator.  Stop.\n", lpszFileName, nLine);
				return -1;
			}
		}
	}

	fclose(fp);

	return nTargetCount;
}

// Function to print the data structure populated by parse() function.
// Use prog_args as arguments for execvp()
void show_targets(target_t * const t, int const nTargetCount)
{
	int i=0;
	int j=0;
	int k=0;
	for(i=0;i<nTargetCount;i++)
	{
		k = 0;
		printf("%d. Target: %s  Status: %d\nCommand: %s\nDependency: ", i, t[i].szTarget, t[i].nStatus, t[i].szCommand); 

		for(j=0;j<t[i].nDependencyCount;j++)
		{
			printf("%s ", t[i].szDependencies[j]); 
		}
		printf("\nDecomposition of command:\n\t");
		while (t[i].prog_args[k] != NULL) {
			printf("%d. %s\n\t", k, t[i].prog_args[k]);
			k++;
		}

		printf("\n\n");
	}
}

