#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcnt1.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <sys/mman.h>

void viewProcess(char* requestedPid)
{
	sem_t *semRead,*semCanEnd;
	char sharedMemName[32],semRead[32],semEndSignal[32];

	sprintf(sharedMemName,"/view%s",requestedPid);
	sprintf(semRead,"/viewRead%s",requestedPid);
	sprintf(semEndSignal,"/viewEnd%s",requestedPid);

	semRead = sem_open(semRead,0);
	semCanEnd = sem_open(semEndSignal,0);
	if(semRead == SEM_FAILED || semCanEnd == SEM_FAILED)
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

	struct stat sb;
	fstat(fileDescriptor,&sb);

	void * sharedMem = mmap(NULL, sb.st_size, PROT_READ, MAP_SHARED, fileDescriptor, 0);
	if(sharedMem == MAP_FAILED)
	{
		perror("Error creating new mapping in the virtual adress of the calling process");
		return;
	}
	char * c = (char *) sharedMem;
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

	int closed = sem_close(semRead);
	if(closed == -1)
	{
		perror("Error closing semaphore");
		return;
	}
	munmap(sharedMem,sb.st_size);
	close(fd);

	sem_post(semCanEnd);
	int closedEnd = sem_close(semCanEnd);
	if(closedEnd == -1)
	{
		perror("Error closing semaphore");
		return;
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