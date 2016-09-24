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

    char special_control_tag = '0';
    while((ch = getopt(argc, argv, format)) != -1) 
    {
        switch(ch) 
        {
            case 'f':
                strcpy(szMakefile, strdup(optarg));
                break;
            case 'n':
                special_control_tag = 'n';
                break;
            case 'B':
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

    int processing_queue[MAX_NODES];
    int processing_queue_len = 0;
    memset(processing_queue, 0, sizeof processing_queue);
    build_processing_queue(dependency_num_len, 
                           nTargetCount, 
                           dependency_num, 
                           processing_queue,
                           &processing_queue_len
                           );
    // for (i=0; i < processing_queue_len; i++)
    // {
    //     printf("%d, \n", processing_queue[i]);
    // }

    switch (special_control_tag)
    {
        case 'n':
            display_processing_queue(processing_queue_len,
                                     processing_queue,
                                     targets
                                     );
        break;

        default:
        break;
    }
    return EXIT_SUCCESS;
}
