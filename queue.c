#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>

// Save all the files in a queue
void saveQueue(char* dir);
void printQueue();
void createQueue();
int isEmpty();
void enqueue(char * data);
char * dequeue();
char* peek();

struct queueElement
{
	char * filename;
	struct queueElement *next;	
};

struct queueElement *queueFront;
struct queueElement *queueLast;
int queueSize;


void saveQueue(char * dir){
    DIR * directory;
    struct dirent * dirEntry;
    struct stat file;
    char path[100];
    char * bar = "/";

    if((directory = opendir(dir)) == NULL){
        printf("Could not open the directory: %s\n", dir);
    }

    while((dirEntry = readdir(directory)) != NULL){
        stat(dirEntry->d_name,&file);
        if (strcmp(dirEntry->d_name,".") != 0 && strcmp(dirEntry->d_name,"..") != 0){
            char* node;
			node = malloc((sizeof(char) * strlen(dir)) + (sizeof(char) * strlen(dirEntry->d_name)) + (sizeof(char)*2));
            strcpy(node,dir);
            strcat(node,bar);
            strcat(node,dirEntry->d_name);
            enqueue(node);
        }
        if(dirEntry->d_type == 4 && strcmp(dirEntry->d_name,".") != 0 && strcmp(dirEntry->d_name,"..") != 0){
            strcpy(path,dir);
            strcat(path,bar);
            strcat(path,dirEntry->d_name);
            saveQueue(path);
        }
    }
    closedir(directory);
}


void createQueue()
{
	queueFront = NULL;
	queueLast = NULL;
	queueSize = 0;
}

char* peek()
{
	return queueFront->filename;
}

char * dequeue()
{
	if(isEmpty())
		return NULL;
	queueSize--;
	struct queueElement *tmp = queueFront;
	char * data = tmp->filename;
	queueFront = queueFront->next;
	free(tmp);
	return data;
}

void enqueue(char * data)
{	
	queueSize++;	
	if(queueFront==NULL)
	{		
		queueFront = (struct queueElement *)malloc(sizeof(struct queueElement));
		queueFront->filename = malloc(strlen(data)+1);
		strcpy(queueFront->filename,data);
		queueFront->next = NULL;
		queueLast = queueFront;
	}
	else
	{
		queueLast->next = (struct queueElement *)malloc(sizeof(struct queueElement));
		queueLast->filename = malloc(strlen(data)+1);
		strcpy(queueLast->filename,data);
		queueLast->next->next = NULL;
		queueLast = queueLast->next;
	}	
}

int isEmpty()
{
	return queueSize==0;
}


void printQueue(){

    struct queueElement *aux = queueFront;
    int i;
    for(i=0; i<queueSize-1; i++){
        printf("%s\n",aux->filename);
        aux = aux->next;
    }
}