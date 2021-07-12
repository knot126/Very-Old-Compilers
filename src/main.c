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
		printf("  → ");
		
		fgets(next, 256, stdin);
		
		dew_runChunk(&script, next);
		
		printf("\n");
	}
	
	dew_free(&script);
}
