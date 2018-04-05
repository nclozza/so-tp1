#ifndef QUEUE_H
#define QUEUE_H

void createPathQueue(char* initialPath);
void printQueue();
void createQueue();
int isEmpty();
void enqueue(char* data);
char * dequeue();
char* peek();
void saveQueue(char* dir);
int sizeQueue();

#endif
