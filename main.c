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
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <mqueue.h>

#define CHILD_PROCESSES 2
#define MSG_SIZE 256
#define MQ_SEND_PATHS "/mqSendPaths"
#define MQ_RECEIVE_HASHES "/mqReceiveHashes"

struct mesg_buffer {
    long mesg_type;
    char mesg_text[1024];
    int count;
} message;

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <PATH>\n", argv[0]);
        exit(EXIT_FAILURE);
    } 

    struct stat fileStat;

    if(stat(argv[1], &fileStat) < 0) {
        printf("Error in the file\n");
        return 1;
    }


    struct mq_attr attr, old_attr;   // To store queue attributes
    mqd_t mqSendPaths, mqReceiveHashes;             // Message queue descriptors

    // First we need to set up the attribute structure
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MSG_SIZE;
    attr.mq_flags = O_NONBLOCK;

    // Open a queue with the attribute structure
    mqSendPaths = mq_open(MQ_SEND_PATHS, O_WRONLY | O_NONBLOCK | O_CREAT, 0666, &attr);
    if(mqSendPaths == -1)
    {
        printf("Error opening the Message Queue descriptor\n");
    }
    
    createPathQueue(argv[1]);
    int totalPaths = sizeQueue() - 1;
    char* pathFromQueue;
    
    while((pathFromQueue = dequeue()) != NULL)
    {
        int result = mq_send(mqSendPaths, pathFromQueue, MSG_SIZE, 0);
        if (result == -1)
            perror ("mq_send()");
    }

    pid_t childsPID[CHILD_PROCESSES];
    int i;

    for(i = 0; i < CHILD_PROCESSES; i++)
    {
        childsPID[i] = fork();
        if (childsPID[i] == -1)
        {
            perror("fork");
            exit(EXIT_FAILURE);
        }

        if (childsPID[i] == 0)
        {
            mqSendPaths = mq_open(MQ_SEND_PATHS, O_RDONLY | O_NONBLOCK | O_CREAT, 0666);
            if(mqSendPaths == -1)
            {
                printf("Error opening the Message Queue descriptor\n");
            }

            mqReceiveHashes = mq_open(MQ_RECEIVE_HASHES, O_WRONLY | O_NONBLOCK | O_CREAT, 0666, &attr);
            if(mqReceiveHashes == -1)
            {
                printf("Error opening the Message Queue descriptor\n");
            }
            
            char pathToBeHashed[MSG_SIZE];
            char* pathHashed;
            int queueIsEmpty = 0;

            do
            {
                mq_receive(mqSendPaths, pathToBeHashed, MSG_SIZE, NULL);
                if(errno == EAGAIN)
                {
                    queueIsEmpty = 1;
                }
                else
                {
                    pathHashed = hash(pathToBeHashed);
                    int result = mq_send(mqReceiveHashes, pathHashed, MSG_SIZE, 0);
                    if (result == -1)
                        perror ("mq_send()");
                }
            } while(!queueIsEmpty);

            mq_close(mqReceiveHashes);
            mq_close(mqSendPaths);
            _exit(EXIT_SUCCESS);

        }
        else
        {
        }
    }


    mqReceiveHashes = mq_open(MQ_RECEIVE_HASHES, O_RDONLY | O_CREAT, 0666, &attr);
    if(mqReceiveHashes == -1)
    {
        printf("Error opening the Message Queue descriptor\n");
    }
    
    char hashReceived[MSG_SIZE];
    int mqReceiveHashesQueueEmpty = 0;

    do
    {
        mq_receive(mqReceiveHashes, hashReceived, MSG_SIZE, NULL);
        if(errno == EAGAIN)
        {
            mqReceiveHashesQueueEmpty = 1;
        }
        else
        {
            printf("%s\n", hashReceived);
        }
        totalPaths--;
    } while(!mqReceiveHashesQueueEmpty && totalPaths);

    int storage;
    for(i = 0; i < CHILD_PROCESSES; i++)
    {
        waitpid(childsPID[i], &storage, WUNTRACED);
    }

    mq_close(mqReceiveHashes);
    mq_unlink(MQ_RECEIVE_HASHES);
    mq_close(mqSendPaths);
    mq_unlink(MQ_SEND_PATHS);

    return 0;
}