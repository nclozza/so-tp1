#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char* format(char* hashedString);
char* hash(char* stringToHash)
{
	char command[256];
	sprintf(command, "md5sum %s", stringToHash);
	FILE* hashedFile = popen(command,"r");
	if(hashedFile == NULL)
		return NULL;

	char * result = malloc(512);
	fgets(result, 512, hashedFile); 
	pclose(hashedFile);
	return format(result);
}

char * format(char* hashedString)
{
	
	char filename[512], hash[512];
	char* formatedString = malloc(1024);
	int i = 0, j = 0, k = 0;	
	while(hashedString[i] != ' ')
	{
		hash[j] = hashedString[i];
		j++;
		i++;
	}
	hash[j] = '\0';
	i++;
	i++;
	while(hashedString[i] != '\n')
	{
		filename[k] = hashedString[i];		
		i++;
		k++;
	}
	filename[k] = '\0';	
	sprintf(formatedString,"<%s> : <%s>\n",filename,hash);
	return formatedString;
}