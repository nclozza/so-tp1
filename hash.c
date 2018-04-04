#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void format(char* hashedString, char* result);

char* hash(char* stringToHash, char* result)
{
	char command[256];
	sprintf(command, "md5sum %s", stringToHash);
	FILE* hashedFile = popen(command,"r");
	if(hashedFile == NULL)
		return NULL;

	char* hashedString = fgets(result, 512, hashedFile); 
	pclose(hashedFile);
	format(hashedString,result);
	
	return hashedString;
}

void format(char* hashedString,char* result)
{
	
	char filename[512], hash[512];
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
	sprintf(result,"<%s>: <%s>",filename,hash);

}
