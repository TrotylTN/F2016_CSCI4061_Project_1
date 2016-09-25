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

//    printf("%s - %d  :  %s - %d\n", lpsz1, nTime1, lpsz2, nTime2);

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
        else    //Target
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

void build_dependency_dag(target_t * const t, 
                          int const nTargetCount,
                          int dependency_num[MAX_NODES][MAX_NODES],
                          int dependency_num_len[MAX_NODES]
                          )
{
    int i = 0;
    int j = 0;
    int k = 0;
    char *tempTargetName;

    //translate the target name into number, -1 mean the dependency is not a target in makefile
    for (i = 0; i < nTargetCount; i++)
    {
        for (j = 0; j < t[i].nDependencyCount; j++)
        {
            tempTargetName = t[i].szDependencies[j];
            for (k = 0; k < nTargetCount; k++)
            {
                if (strcmp(tempTargetName, t[k].szTarget) == 0)
                {
                    dependency_num[i][dependency_num_len[i]++] = k;
                    break;
                }
            }
        }
    }
}

void show_targets_in_number(int const dependency_num_len[MAX_NODES], 
                            int const nTargetCount, 
                            int const dependency_num[MAX_NODES][MAX_NODES]
                            )
{
    int i = 0;
    int j = 0;
    for (i = 0; i < nTargetCount; i++)
    {
        printf("This node number: %d\nTotal connected nodes: %d\n", i, dependency_num_len[i]);
        for (j = 0; j < dependency_num_len[i]; j++)
        {
            printf("%d, ", dependency_num[i][j]);
        }
        printf("\n\n");
    }
}

// build_processing_queue() may be trashed after completing the matrix part.
// void build_processing_queue(int const dependency_num_len[MAX_NODES],
//                             int const nTargetCount,
//                             int const dependency_num[MAX_NODES][MAX_NODES],
//                             int processing_queue[MAX_NODES],
//                             int *processing_queue_len,
//                             int const init_node_num
//                             )
// {
//     int node_in_queue[MAX_NODES];
//     memset(node_in_queue, 0, sizeof node_in_queue);
//     build_processing_queue_dfs(init_node_num,
//                                processing_queue_len,
//                                processing_queue,
//                                dependency_num_len,
//                                nTargetCount,
//                                dependency_num,
//                                node_in_queue
//                                );
// }

// void build_processing_queue_dfs(int const curr_pos,
//                                 int *processing_queue_len,
//                                 int processing_queue[MAX_NODES],
//                                 int const dependency_num_len[MAX_NODES],
//                                 int const nTargetCount,
//                                 int const dependency_num[MAX_NODES][MAX_NODES],
//                                 int node_in_queue[MAX_NODES]
//                                 )
// {
//     int i = 0;
//     // printf("Here is %d\n", curr_pos);
//     for (i = 0; i < dependency_num_len[curr_pos]; i++)
//     {
//         if (node_in_queue[i] == 0)
//         {
//             build_processing_queue_dfs(dependency_num[curr_pos][i],
//                                        processing_queue_len,
//                                        processing_queue,
//                                        dependency_num_len,
//                                        nTargetCount,
//                                        dependency_num,
//                                        node_in_queue
//                                        );
//         }        
//     }
//     node_in_queue[curr_pos] = 1;
//     processing_queue[(*processing_queue_len)++] = curr_pos;
// }

// // this function may be adjusted to match matrix after completing maxtrix part
// int check_dependencies(target_t * const t,
//                        int const nTargetCount,
//                        int const dependency_num[MAX_NODES][MAX_NODES],
//                        int const dependency_num_len[MAX_NODES],
//                        int const processing_queue[MAX_NODES],
//                        int const processing_queue_len
//                        )
// {
//     int i = 0;
//     int j = 0;
//     int return_num = 0; //0 represents everthing OK, other represents at least one file lost
//     for (i = 0; i < processing_queue_len; i++)
//     {
//         for (j = 0; j < t[processing_queue[i]].nDependencyCount; j++)
//         {
//             if (find_target(t[processing_queue[i]].szDependencies[j], t, nTargetCount) != -1)
//             {
//                 // everything ok;
//             }
//             else
//             {
//                 if (is_file_exist(t[processing_queue[i]].szDependencies[j]) != -1)
//                 {
//                     // everthing ok;
//                 }
//                 else
//                 {
//                     return_num++;
//                     fprintf(stderr, "Error: '%s' is missing, needed by '%s'.\n", t[processing_queue[i]].szDependencies[j], t[processing_queue[i]].szTarget);
//                 }
//             }
//         }
//     }
//     return return_num; // return the count of missing files
// }

void build_processing_matrix(int const nTargetCount,
                             target_t * const t,
                             int processing_matrix[MAX_NODES][MAX_NODES],
                             int processing_matrix_len[MAX_NODES],
                             int const init_node_num,
                             int const force_repeat   // 0 represents no, 1 represents '-B' becomes active
                             )
{
    int level[MAX_NODES]; //the max height of this point from the all the leaf nodes
    memset(level, 0, sizeof level);
    build_processing_matrix_dfs(init_node_num,
                                processing_matrix_len,
                                processing_matrix,
                                nTargetCount,
                                t,
                                level,
                                force_repeat
                                );
}

// the returned int is the timestamp of current target. If it does not exist, return -1
int build_processing_matrix_dfs(int const curr_pos, //curr_pos is the number of this target
                                int processing_matrix_len[MAX_NODES],
                                int processing_matrix[MAX_NODES][MAX_NODES],
                                int const nTargetCount,
                                target_t * const t,
                                int level[MAX_NODES],
                                int const force_repeat  // 0 represents no, 1 represents '-B' becomes active
                                )
{
    int temp_timestamp = get_file_modification_time(t[curr_pos].szTarget);
    int dependency_timestamp = 0x7fffffff;
    int i = 0;
    int k = 0;
    char *temp_name;

    for (i = 0; i < t[curr_pos].nDependencyCount; i++)
    {
        temp_name = t[curr_pos].szDependencies[i]; // temp_name is this dependency's name
        k = find_target(temp_name, t, nTargetCount); //look for the child target
        if (k == -1)
        {
            // this is a source file
            dependency_timestamp = get_file_modification_time(temp_name);
            level[curr_pos] = max(level[curr_pos], 0);
        }
        else
        {
            // this is a target
            dependency_timestamp = build_processing_matrix_dfs(k,
                                                               processing_matrix_len,
                                                               processing_matrix,
                                                               nTargetCount,
                                                               t,
                                                               level,
                                                               force_repeat
                                                               );
            level[curr_pos] = max(level[curr_pos], level[k] + 1);
        }
        // if the child source file/target is newer than curr_pos itself or one of them doesn't exist, mark temp_timestamp as -1 in order to re-compile
        if ((dependency_timestamp > temp_timestamp) || (dependency_timestamp == -1))
            temp_timestamp = -1;        
    }
    if (force_repeat || temp_timestamp == -1)
    {
        processing_matrix[level[curr_pos]][processing_matrix_len[level[curr_pos]]++] = curr_pos;
    }
    return temp_timestamp;
}

void display_processing_matrix(int const processing_matrix[MAX_NODES][MAX_NODES],
                               target_t * const t,
                               int const processing_matrix_len[MAX_NODES]
                               )
{
    int i = 0;
    int j = 0;
    int p = -1; // refer to prcessing_matrix[i][j]
    for (i = 0; i < MAX_NODES; i++)
        for (j = 0; j < processing_matrix_len[i]; j++)
        {
            p = processing_matrix[i][j];
            printf("%s\n", t[p].szCommand);
        }
}

void execute_commands_by_matrix(int const processing_matrix[MAX_NODES][MAX_NODES],
                                target_t * const t,
                                int const processing_matrix_len[MAX_NODES]
                                )
{
    int i = 0;
    int j = 0;
    int p = -1;
    int temp_pid = 0;
    for (i = 0; i < MAX_NODES; i++)
    {
        for (j = 0; j < processing_matrix_len[i]; j++)
        {
            if (fork() == 0)
            {
                // child
                p = processing_matrix[i][j];
                t[p].pid = getpid();
                if (execvp(t[p].prog_args[0], t[p].prog_args) != 0)
                {
                    //fail to execute
                    perror("error on exec");
                    exit(0);
                }
            }
            else
            {
                // parent, continue to next loop to fork next command
            }
        }
        for (j = 0; j < processing_matrix_len[i]; j++)
        {
            p = processing_matrix[i][j];
            wait(&t[p].pid);
        }

    }
}

int check_dependencies_by_matrix(target_t * const t,
                                 int const processing_matrix[MAX_NODES][MAX_NODES],
                                 int const processing_matrix_len[MAX_NODES],
                                 int const nTargetCount
                                 )
{
    int i = 0;
    int j = 0;
    int k = 0;
    int p = -1; // temp var for matrix[i][j]
    int return_num = 0; //0 represents everthing OK, other represents at least one file lost
    for (i = 0; i < MAX_NODES; i++)
    {
        for (j = 0; j < processing_matrix_len[i]; j++)
        {
            p = processing_matrix[i][j];
            for (k = 0; k < t[p].nDependencyCount; k++)
            {
                if (find_target(t[p].szDependencies[k], t, nTargetCount) != -1)
                {
                    // everything ok;
                }
                else
                {
                    if (is_file_exist(t[p].szDependencies[k]) != -1)
                    {
                        // everthing ok;
                    }
                    else
                    {
                        return_num++;
                        fprintf(stderr, "Error: '%s' is missing, needed by '%s'.\n", t[p].szDependencies[k], t[p].szTarget);
                    }
                }
            }
        }
    }
    return return_num; // return the count of missing files
}