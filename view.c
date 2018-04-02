#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcnt1.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <sys/mman.h>

void viewProcess(char* requestedPid)
{
	sem_t *semRead,*semEnd;
	char sharedMemName[32],semRead[32],semEndSignal[32];
	sprintf(sharedMemName,"/view%s",requestedPid);
	sprintf(semRead,"/viewRead%s",requestedPid);
	sprintf(semEndSignal,"/viewEnd%s",requestedPid);
	semRead = sem_open(semRead,0);
	sem = sem_open(semEndSignal,0);
	if(semRead == SEM_FAILED || semEnd == SEM_FAILED)
	{
		perror("Error opening POSIX semaphore");
		return;
	}
	int fileDescriptor;
	fileDescriptor = shm_open(sharedMemName,O_RDWR,0777);
	if(fileDescriptor == -1)
	{
		perror("Error opening shared memory space");
		return;
	}

	struct stat statbuf;
	fstat(fileDescriptor,&statbuf);

	void * sharedMem = mmap(NULL, statbuf.st_size, )

	while(*c != EOF)
	{
		sem_wait(semRead);
		while(*c != 0)
		{
			printf("%c\n", *c);
			c++;
		}
		c++;
	}
}




int main(int argc, char const *argv[])
{
	if(argc != 2)
	{
		printf("Usage: ./view pid\n");
	}

	char* requestedPid = argv[1];
	viewProcess(requestedPid);
	return 0;
}