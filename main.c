#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/mman.h>
#include <mqueue.h>
#include <sys/stat.h>
#include "queue.h"
#include "helpers.h"
#include "hash.h"
#include "appendStringToFile.h"
#include <time.h>
#include <semaphore.h>

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

    // MESSAGE QUEUE
    struct mq_attr attr;
    mqd_t mqSendPaths, mqReceiveHashes;

    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MSG_SIZE;
    attr.mq_flags = O_NONBLOCK;

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

    // SHARED MEMORY

    /* the size (in bytes) of shared memory object */
    const int sharedMemorySize = 4096;
 
    /* name of the shared memory object */
    // LU/CONY/FEDE CAMBIAR ESTE NOMBRE
    const char* name = "OS";
 
    /* shared memory file descriptor */
    int shm_fd;
 
    /* pointer to shared memory obect */    
    void* ptr;
 
    /* create the shared memory object */
    shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);
 
    /* configure the size of the shared memory object */
    ftruncate(shm_fd, sharedMemorySize);
 
    /* memory map the shared memory object */
    ptr = mmap(0, sharedMemorySize, PROT_WRITE, MAP_SHARED, shm_fd, 0);

    char viewReadName[64], viewEndName[64];
    sem_t *viewRead, *viewEnd;

    sprintf(viewReadName,"/viewRead%d",getpid());
    sprintf(viewEndName,"/viewEnd%d",getpid());

    viewRead = sem_open(viewReadName, O_CREAT | O_EXCL, 0777, 0);
    viewEnd = sem_open(viewEndName, O_CREAT | O_EXCL, 0777, 0);

    if(viewRead == SEM_FAILED || viewEnd == SEM_FAILED)
    {
        perror("Error opening semaphore");
        return -1;
    }

    // CHILDRENS
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
            /* CHILDRENS */
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
            /* FATHER */

            char internalBuffer[MSG_SIZE * 10];
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
                    int hashReceivedLength = strlen(hashReceived);                    
                    strcat(internalBuffer, hashReceived);
                    strcat(internalBuffer, "\n");
                    sprintf(ptr, "%s", hashReceived);
                    strcat(ptr, "\n");
                    ptr += hashReceivedLength;
                    sem_post(viewRead);
                }
                totalPaths--;
            } while(!mqReceiveHashesQueueEmpty && totalPaths);

            strcat(ptr,"E");
            printf("My pid is %d\n",getpid());
            printf("Run the view process: Usage ./view PID\n");
            
            struct timespec ts;
            if(clock_gettime(CLOCK_REALTIME, &ts) != -1)
            {
                ts.tv_sec += 15;
                sem_timedwait(viewEnd,&ts);
            }
            else
            {
                sleep(10);
            }
            if(appendStringToFile(internalBuffer) == 0)
            {
                perror("Error while writing the hashes in the file hashes.txt");
            }

            int storage;
            for(i = 0; i < CHILD_PROCESSES; i++)
            {
                waitpid(childsPID[i], &storage, WUNTRACED);
            }

            mq_close(mqReceiveHashes);
            mq_unlink(MQ_RECEIVE_HASHES);
            mq_close(mqSendPaths);
            mq_unlink(MQ_SEND_PATHS);
            shm_unlink(name);
            sem_close(viewRead);
            sem_unlink(viewReadName);
            sem_close(viewEnd);
            sem_unlink(viewEndName);
        }
    }

    return 0;
}