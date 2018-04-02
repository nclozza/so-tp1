#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include "queue.h"

#define CHILD_PROCESSES 2

int main(int argc, char **argv){

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <PATH>\n", argv[0]);
        exit(EXIT_FAILURE);
    } 

    struct stat fileStat;

    if(stat(argv[1],&fileStat) < 0) {
        return 1;
        printf("Error in the file\n");
    }

    createPathQueue(argv[1]);
    //printQueue();

    int pipeMainToChild[CHILD_PROCESSES][2];
    int pipeChildToMain[CHILD_PROCESSES][2];
    pid_t childsPID[CHILD_PROCESSES];
    char pathToHash[128];
    int i;

    for(i = 0; i < CHILD_PROCESSES; i++)
    {
        if (pipe(pipeMainToChild[i]) == -1 || pipe(pipeChildToMain[i]) == -1)
        {
            perror("pipe");
            exit(EXIT_FAILURE);
        }

        childsPID[i] = fork();
        if (childsPID[i] == -1)
        {
            perror("fork");
            exit(EXIT_FAILURE);
        }

        if (childsPID[i] == 0)
        {    
            /* Close unused write end */
            close(pipeMainToChild[i][1]);
            /* Close unused read end */
            close(pipeChildToMain[i][0]);


            /*Wait for other paths*/
            printf("Child number: %d", i);
            while(1)
            {
                /* Child reads from pipe */
                read(pipeMainToChild[i][0], pathToHash, 128);
                printf("%s\n", pathToHash);
                //write(STDOUT_FILENO, pathToHash, 1);
            }

            //write(STDOUT_FILENO, "\n", 1);
            //close(pipeMainToChild[i][0]);
            //_exit(EXIT_SUCCESS);

        }
        else
        {
            /* Close unused read end */
            close(pipeMainToChild[i][0]);
            /* Close unused write end */
            close(pipeChildToMain[i][1]);

            //write(pipeMainToChild[i][1], argv[1], strlen(argv[1]));

            /* Parent writes argv[1] to pipe */
            //write(pipeMainToChild[i][1], argv[1], strlen(argv[1]));
            /* Reader will see EOF */
            //close(pipeMainToChild[i][1]);
            /* Wait for child */
            //wait(NULL);
            //exit(EXIT_SUCCESS);
        }
    }

    char* element;
    int elementLenght;
    while((element = dequeue()) != NULL)
    {
        elementLenght = strlen(element);
        if(write(pipeMainToChild[0][1], element, elementLenght) != elementLenght)
        {
            printf("Error while writting to a child process");
        }
    }

    sleep(5);
    printf("\n");
    for(i = 0; i < CHILD_PROCESSES; i++)
    {
        close(pipeMainToChild[i][0]);
        printf("KILL PID: %i\n", childsPID[i]);
        kill(childsPID[i], SIGKILL);
    }

    return 0;
}