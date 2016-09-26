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
    char * format = "f:hnB";
    
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
                    // printf("\n\n!!!Please Force Run.!!!\n\n");
                    //Set flag which can be used later to handle this case.
                break;
            case 'h':
            default:
                show_error_message(argv[0]);
                exit(1);
        }
    }

    if (is_file_exist(szMakefile) == -1) // Check the 
    {
        fprintf(stderr, "Error: file '%s' does not exist.\n", szMakefile);
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
    if (find_target(szTarget, targets, nTargetCount) == -1)
    {
        fprintf(stderr, "Error: target '%s' does not exist.\n", szTarget);
        return EXIT_FAILURE;
    }

    int processing_matrix[MAX_NODES][MAX_NODES];
    int processing_matrix_len[MAX_NODES];
    memset(processing_matrix, -1, sizeof processing_matrix);
    memset(processing_matrix_len, 0, sizeof processing_matrix_len);
    build_processing_matrix(nTargetCount,
                            targets,
                            processing_matrix,
                            processing_matrix_len,
                            find_target(szTarget, targets, nTargetCount),
                            force_repeat
                            );

    int lost_number = 0;
    if ((lost_number = check_dependencies_by_matrix(targets,
                                                    processing_matrix,
                                                    processing_matrix_len,
                                                    nTargetCount
                                                    )) > 0)
    {
        fprintf(stderr, "Error: %d needed file(s) lost.\n", lost_number);
        return EXIT_FAILURE;
    }

    int tot_steps = 0;
    for (i = 0; i < MAX_NODES; i++)
    {
        tot_steps += processing_matrix_len[i];
    }
    if (tot_steps == 0)
    {
        fprintf(stderr, "make4061: all files needed by '%s' are up-to-date.\n", szTarget);
    }

    if (donot_exec)
    {
        // '-n', just show
        display_processing_matrix(processing_matrix, targets, processing_matrix_len);
    }
    else
    {
        execute_commands_by_matrix(processing_matrix, targets, processing_matrix_len);
    }

    return EXIT_SUCCESS;
}

