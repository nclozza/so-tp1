#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <semaphore.h>


void viewProcess(char* mainPid)
{
	
	char semReadName[32], semEndName[32];
	sem_t *semRead, *semEnd;
	
	sprintf(semReadName,"/viewRead%s",mainPid);
	sprintf(semEndName,"/viewEnd%s",mainPid);

	semRead = sem_open(semReadName,0);
	semEnd = sem_open(semEndName,0);

	if(semRead ==  SEM_FAILED || semEnd == SEM_FAILED)
	{
		perror("Error opening semaphore");
		return;
	}


    /* the size (in bytes) of shared memory object */
    const int SIZE = 4096;
 
    /* name of the shared memory object */
	// LU/CONY/FEDE CAMBIAR ESTE NOMBRE
    const char* name = "OS";
 
    /* shared memory file descriptor */
    int shm_fd;
 
    /* pointer to shared memory object */
    void* ptr;
 
    /* open the shared memory object */
    shm_fd = shm_open(name, O_RDONLY, 0666);
 
 	if(shm_fd == -1)
 	{
 		perror("Error opening shared memory");
 		return;
 	}
    /* memory map the shared memory object */
    ptr = mmap(0, SIZE, PROT_READ, MAP_SHARED, shm_fd, 0);
 
    /* read from the shared memory object */
    char* c = (char*)ptr;
    
    while(*c != '\0')
    {
    	sem_wait(semRead);       	
    	while(*c != '\n')
    	{
    		printf("%c", *c);
    		c++;
    	}    	  
    	printf("\n");  	
    	c++;	
    }
 	
 	sem_close(semRead);
 	sem_unlink(semReadName);

 	sem_post(semEnd);
 	sem_close(semEnd);
    /* remove the shared memory object */
    //shm_unlink(name);

} 

int main(int argc, char *argv[])
{
	if(argc != 2)
	{
		printf("Usage: ./view PID\n");
		return 1;
	}
	viewProcess(argv[1]);
	return 0;
}