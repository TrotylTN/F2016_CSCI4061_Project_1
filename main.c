#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h> 
#include <unistd.h>

#include "util.h"

void show_error_message(char * lpszFileName)
{
    fprintf(stderr, "Usage: %s [options] [target] : only single target is allowed.\n", lpszFileName);
    fprintf(stderr, "-f FILE\t\tRead FILE as a makefile.\n");
    fprintf(stderr, "-h\t\tPrint this message and exit.\n");
    fprintf(stderr, "-n\t\tDon't actually execute commands, just print them.\n");
    fprintf(stderr, "-B\t\tDon't check files timestamps.\n");
    exit(0);
}

int main(int argc, char **argv) 
{
    target_t targets[MAX_NODES]; //List of all the targets. Check structure target_t in util.h to understand what each target will contain.
    int nTargetCount = 0;

    // Declarations for getopt
    extern int optind;
    extern char * optarg;
    int ch;
    char * format = "f:hnB:";
    
    // Variables you'll want to use
    char szMakefile[64] = "Makefile";
    char szTarget[64] = "";
    int i=0;
    
    //init targets 
    for(i=0;i<MAX_NODES;i++)
    {
        targets[i].pid=0 ;
        targets[i].nDependencyCount = 0;
        strcpy(targets[i].szTarget, "");
        strcpy(targets[i].szCommand, "");
        targets[i].nStatus = FINISHED;
    }

    int force_repeat = 0;   //1 represents '-B' becomes active
    int donot_exec = 0; //1 represents '-n' becomes active
    while((ch = getopt(argc, argv, format)) != -1) 
    {
        switch(ch) 
        {
            case 'f':
                strcpy(szMakefile, strdup(optarg));
                break;
            case 'n':
                    donot_exec = 1;
                break;
            case 'B':
                    force_repeat = 1;
                    //Set flag which can be used later to handle this case.
                break;
            case 'h':
            default:
                show_error_message(argv[0]);
                exit(1);
        }
    }

    argc -= optind;
    argv += optind;

    if(argc > 1)
    {
        show_error_message(argv[0]);
        return EXIT_FAILURE;
    }

    /* Parse graph file or die */
    if((nTargetCount = parse(szMakefile, targets)) == -1) 
    {
        return EXIT_FAILURE;
    }

    //Setting Targetname
    //if target is not set, set it to default (first target from makefile)
    if(argc == 1)
    {
        strcpy(szTarget, argv[0]);
    }
    else
    {
        strcpy(szTarget, targets[0].szTarget);
    }

    // show_targets(targets, nTargetCount);

    //Now, the file has been parsed and the targets have been named. You'll now want to check all dependencies (whether they are available targets or files) and then execute the target that was specified on the command line, along with their dependencies, etc. Else if no target is mentioned then build the first target found in Makefile.
    /*
      INSERT YOUR CODE HERE
    */
    int dependency_num[MAX_NODES][MAX_NODES];
    //dependency_num_len[i] is the number of other targets depended by targets[i]
    int dependency_num_len[MAX_NODES];
    memset(dependency_num_len, 0, sizeof dependency_num_len);
    memset(dependency_num, 0, sizeof dependency_num);
    build_dependency_dag(targets, nTargetCount, dependency_num, dependency_num_len);

    // show_targets_in_number(dependency_num_len, nTargetCount, dependency_num);

    //check all depended files exist
    int processing_queue[MAX_NODES];
    int processing_queue_len = 0;   // processing_queue may be trashed after completing matrix
    memset(processing_queue, 0, sizeof processing_queue);
    build_processing_queue(dependency_num_len, 
                           nTargetCount, 
                           dependency_num, 
                           processing_queue,
                           &processing_queue_len,
                           0 // !!!Warning!!!: the default start node is 0, maybe change in further development
                           );
    if (check_dependencies(targets,
                           nTargetCount,
                           dependency_num,
                           dependency_num_len,
                           processing_queue,
                           processing_queue_len) > 0)
    {
        fprintf(stderr, "Error: at least one depended file lost.\n");
        return EXIT_FAILURE;
    }
    // for (i=0; i < processing_queue_len; i++)
    // {
    //     printf("%d, \n", processing_queue[i]);
    // }

    int processing_matrix[MAX_NODES][MAX_NODES];
    int processing_matrix_len[MAX_NODES];
    memset(processing_matrix, -1, sizeof processing_matrix);
    memset(processing_matrix_len, 0, sizeof processing_matrix_len);
    build_processing_matrix(nTargetCount,
                            targets,
                            processing_matrix,
                            processing_matrix_len,
                            0, // !!!Warning!!!: the default start node is 0, maybe change in further development
                            force_repeat
                            );
    // int j = 0;
    // for (i = 0; i < 10; i++)
    // {
    //     for (j = 0; j < 10; j++)
    //     {
    //         printf("%d, ",processing_matrix[i][j]);
    //     }
    //     printf("\n");
    // }

    /*
        
    */
    if (donot_exec)
    {
        // '-n', just show
        display_processing_matrix(processing_matrix, targets, nTargetCount);
    }
    else
    {
        //execute the compile commands
    }

    return EXIT_SUCCESS;
}
/*
For the test example, the matrix should be:
5, 4, 2, -1, -1, -1, -1, -1, -1, -1, 
3, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
0, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
*/
