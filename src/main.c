/**
 * Honeydew Main File
 * ==================
 * 
 * This is the main file for the Honeydew built-in interpreter.
 */

#include <stdio.h>
#include <stdlib.h>

#define DEW_IMPLEMENTATION
#include "hdw.h"

int main(int argc, const char *argv[]) {
	dew_Script script;
	dew_init(&script);
	
	char next[256];
	
	while (!feof(stdin)) {
		printf("\033[1;35m  → \033[0m");
		
		fgets(next, 256, stdin);
		
		dew_runChunk(&script, next);
		
		dew_Error err = dew_popError(&script);
		
		while (err.message != NULL) {
			printf("%.3d: %s\n", err.offset, err.message);
			err = dew_popError(&script);
		}
		
		printf("\n");
	}
	
	dew_free(&script);
}
