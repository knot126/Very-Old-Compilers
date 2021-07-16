#include <stdio.h>

#include "hdw.h"

int main(int argc, char *argv[]) {
	if (argc > 2) {
		printf("Usage: %s [input file]\n", argv[0]);
		return 1;
	}
	else if (argc == 2) {
		int status = hdw_dofile(argv[1]);
		return status;
	}
	else if (argc == 1) {
		hdw_bulitin_prompt();
	}
	
	return 0;
}
