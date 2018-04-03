#include <stdlib.h>
#include <stdio.h>

char* hash(char* stringToHash, char* result);

/*
int nextFilename(char* batchToHash, char* stringSpace, int index)
{
	int j = 0;
	while(batchToHash[index] != '\n' && batchToHash[index] != '0')
	{
		stringSpace[j] = batchToHash[index];
		j++;
		index++;
	}
	index++;
	return index;
}

int processBatch(char* batchToHash, int batchSize)
{
	if(batchToHash==NULL)
		return -1;
	int i = 0;
	char* currentFile = malloc(256);
	for(int j = 0; j < batchSize; j++)
	{
		i = nextFilename(batchToHash,currentFile,i);
		char result[512];	
		//sendToApp(hash(currentFile, result));
	}
	return 0;
}
*/

char* hash(char* stringToHash, char* result)
{
	char command[256];
	sprintf(command, "md5sum %s", stringToHash);
	FILE* hashedFile = popen(command,"r");
	if(hashedFile == NULL)
		return NULL;
	
	char* hashedString = fgets(result, 512, hashedFile); 
	pclose(hashedFile);

	return hashedString;
}
