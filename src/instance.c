// =============================================================================
// Script Instance Management
// =============================================================================
 
hdw_script *hdw_create(void) {
	/**
	 * Creates a new script state.
	 */
	
	hdw_script *c = (hdw_script *) malloc(sizeof(hdw_script));
	
	if (!c) {
		return NULL;
	}
	
	memset(c, 0, sizeof(hdw_script));
	
	return c;
}

void hdw_destroy(hdw_script *c) {
	/**
	 * Destroys a script state.
	 */
	
	free(c);
} 
