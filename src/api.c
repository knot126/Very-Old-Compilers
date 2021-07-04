// =============================================================================
// Higher-Level Wrapper Functions
// =============================================================================

#define sread(f, s, b) fread(b, 1, s, f) 
int hdw_dofile(const char * const path) {
	/**
	 * Loads a file and executes its contents.
	 */
	
	FILE *file = fopen(path, "rb");
	
	if (!file) {
		printf("Error: Failed to open file stream '%s'.\n", path);
		return -1;
	}
	
	// get length
	fseek(file, 0, SEEK_END);
	int length = ftell(file);
	rewind(file);
	
	_Static_assert (sizeof(char) == 1);
	
	// create a buffer
	char *data = (char *) malloc((length + 1) * sizeof(char));
	
	if (!data) {
		printf("Error: Failed to allocate memory to store file.\n");
		fclose(file);
		return -1;
	}
	
	// read in file contents
	size_t intake = sread(file, length, data);
	assert(intake == length);
	
	data[length] = '\0';
	
	// execute the file
	hdw_script *script;
	int status = hdw_crexec(&script, data);
	
	if (hdw_haserror(script)) {
		hdw_printerror(script);
	}
	
	// free resources
	hdw_destroy(script);
	free(data);
	fclose(file);
	
	// return status code
	return status;
}
#undef sread

void hdw_bulitin_prompt(void) {
	/**
	 * Run the built-in interpreter
	 */
	
	char buffer[348];
	hdw_script *context = hdw_create();
	
	if (!context) {
		printf("Error: Failed to init hdw_script structure.\n");
		return;
	}
	
	while (true) {
		// get line
		printf("> ");
		char *p = fgets(buffer, 348, stdin);
		
		// check for error with line
		if (feof(stdin) || !p) {
			printf("\n");
			break;
		}
		
		// act on line
		hdw_exec(context, buffer);
		
		// print any error
		if (hdw_haserror(context)) {
			hdw_printerror(context);
			hdw_reseterror(context);
		}
	}
	
	hdw_destroy(context);
}
