#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include "queue.h"

int main(int argc, char **argv){

    if(argc != 2) {
        printf("Incorrect number of parameters\n");
        return 1;
    }  

    DIR *dirp;   
    struct stat fileStat;

    if(stat(argv[1],&fileStat) < 0) {
        return 1;
        printf("Error in the file\n");
    } 

    createQueue();
    enqueue(argv[1]);
    dirp = opendir(argv[1]);
    //printf("%s\n",argv[1]);
    saveQueue(argv[1]);
    //printQueue();
    return 0;
}