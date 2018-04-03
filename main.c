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
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <signal.h>
#include "queue.h"
#include "helpers.h"
#include "hash.h"

#define CHILD_PROCESSES 2
#define MD5_HASH_LENGTH 32

int main(int argc, char **argv)
{

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

    int pipeMainToChild[CHILD_PROCESSES][2];
    int pipeChildsToMain[2];
    pid_t childsPID[CHILD_PROCESSES];
    
    int i;

    if(pipe(pipeChildsToMain) == -1)
    {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    for(i = 0; i < CHILD_PROCESSES; i++)
    {
        if (pipe(pipeMainToChild[i]) == -1)
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
            char* pathToHash;
            char pathToHashLength[4];
            int incomingPathLength;

            /* Close unused write end */
            close(pipeMainToChild[i][1]);
            /* Close unused read end */
            close(pipeChildsToMain[0]);

            /*Wait for other paths*/
            while(1)
            {
                /* Child reads from pipe */
                read(pipeMainToChild[i][0], pathToHashLength, 4);
                
                if(str2int(&incomingPathLength, pathToHashLength) != STR2INT_SUCCESS)
                {
                    printf("Error while converting char* to int in child N°: %i\n", i);
                }

                pathToHash = malloc(incomingPathLength + 1);
                read(pipeMainToChild[i][0], pathToHash, incomingPathLength);

                char* pathHashed;
                char* pathHashedWithLength;
                char hashLenghtString[4];
                int hashLength = MD5_HASH_LENGTH + 2 + incomingPathLength;
                
                pathHashed = malloc(hashLength);
                pathHashedWithLength = malloc(3 + hashLength);
                sprintf(hashLenghtString, "%03d", hashLength);
                
                hash(pathToHash, pathHashed);
                printf("HASH LENGTH: %s\n", hashLenghtString);
                printf("PATH HASHED: %s\n", pathHashed);
                pathHashedWithLength = concat(hashLenghtString, pathHashed);
                printf("PATH HASHED WITH LENGTH: %s\n", pathHashedWithLength);

                if(write(pipeChildsToMain[1], pathHashedWithLength, hashLength + 3) != hashLength + 3)
                {
                   printf("Childe N°: %i. Error while writting to the father process\n", i); 
                }
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
            close(pipeChildsToMain[1]);
        }
    }

    char* path;
    int pathLength;
    char pathLengthString[4];
    int allPathsToHashSended = 0;
    int allPathsHashedReceived = 0;
    int totalElementsRemainingToHash = sizeQueue();
    int hashesReceived = 0;
    char pathHashed[256];
    char pathHashedLength[3];
    int incomingPathHashedLength;

    while(!allPathsToHashSended && totalElementsRemainingToHash > 0)
    {
        if((path = dequeue()) == NULL)
        {
            allPathsToHashSended = 1;
        }
        else
        {
            pathLength = strlen(path) + 1;
            sprintf(pathLengthString, "%d", pathLength);
            
            if(write(pipeMainToChild[0][1], pathLengthString, 4) != 4)
            {
                printf("Error while writting to a child process\n");
            }
            
            if(write(pipeMainToChild[0][1], path, pathLength) != pathLength)
            {
                printf("Error while writting to a child process\n");
            }
        }
 
        if(read(pipeChildsToMain[0], pathHashedLength, 3) == 3)
        {
            totalElementsRemainingToHash--;
            str2int(&incomingPathHashedLength, pathHashedLength);

            read(pipeChildsToMain[0], pathHashed, incomingPathHashedLength - 1);
            fflush(stdout);
            printf("FILE HASHED FROM CHILD: %s\n", pathHashed);

        }
    }
       

    sleep(1);
    printf("\n");
    for(i = 0; i < CHILD_PROCESSES; i++)
    {
        close(pipeMainToChild[i][0]);
        printf("KILL PID: %i\n", childsPID[i]);
        kill(childsPID[i], SIGKILL);
    }

    return 0;
}