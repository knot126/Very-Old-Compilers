// =============================================================================
// Error Handling
// =============================================================================

void hdw_puterror(hdw_script * const restrict script, const char * const restrict message) {
	/**
	 * Put an error message to the list.
	 */
	
	script->errors.count++;
	script->errors.content = realloc(script->errors.content, sizeof(hdw_error) * script->errors.count);
	
	script->errors.content[script->errors.count - 1].message = message;
}

void hdw_printerror(const hdw_script * const restrict script) {
	/**
	 * Print out a list of all error messages.
	 */
	
	for (size_t i = 0; i < script->errors.count; i++) {
		printf("Error: %s\n", script->errors.content[i].message);
	}
}

void hdw_reseterror(hdw_script * const restrict script) {
	/**
	 * Free all the error messages.
	 */
	
	for (size_t i = 0; i < script->errors.count; i++) {
		free((void *) script->errors.content[i].message);
	}
	
	free(script->errors.content);
	
	memset(&script->errors, 0, sizeof(hdw_errorarray));
}

int32_t hdw_haserror(hdw_script * const restrict script) {
	/**
	 * Return if there are any errors.
	 */
	
	return !!script->errors.count;
}

