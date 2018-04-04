#include <stdio.h>
#include "appendStringToFile.h"

int appendStringToFile(char *stringToAppend) {
	
	FILE *file = fopen("hashes.txt", "a");

	if (file == NULL) {
		return 0;
	}		

	fprintf(file, "%s\n", stringToAppend);
	
	fclose(file);

	return 1;
}
