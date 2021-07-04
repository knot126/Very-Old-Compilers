// =============================================================================
// Codeblock Execution
// =============================================================================

int32_t hdw_exec(hdw_script * restrict script, const char * const code) {
	/**
	 * Execute a line in the context of the script. If NULL is supplied for the
	 * script, a temporary script is created if possible to do so and destroyed
	 * once it has finished executing. If NULL is supplied for code, then
	 * nothing will happen.
	 */
	
	hdw_tokenarray tokens;
	hdw_treenode *tree;
	int32_t status;
	
	// handle code == NULL
	if (!code) {
		return -2;
	}
	
	// handle temporary script
	bool script_temp = false;
	
	if (!script) {
		script_temp = true;
		script = hdw_create();
		if (!script) {
			return -2;
		}
	}
	
	status = hdw_tokenise(script, &tokens, code);
	
	if (status) {
		hdw_freetokens(&tokens);
		
		if (script_temp) {
			hdw_destroy(script);
		}
		
		return status;
	}
	
	//hdw_printtokens(&tokens);
	
	status = hdw_parse(script, &tree, &tokens);
	
	hdw_freetokens(&tokens);
	
	if (status) {
		if (script_temp) {
			hdw_destroy(script);
		}
		
		return status;
	}
	
	hdw_treenodeprint(tree, 0);
	
	hdw_value *result;
	
	status = hdw_interpret(tree, &result);
	
	if (status) {
		if (script_temp) {
			hdw_destroy(script);
		}
		
		return status;
	}
	
	hdw_printValue(result);
	
	hdw_treefree(tree);
	
	// handle temporary script cleanup
	if (script_temp) {
		hdw_destroy(script);
	}
	
	return HDW_ERR_OKAY;
}

int32_t hdw_crexec(hdw_script ** restrict script, const char * const code) {
	/**
	 * Create a script context and then execute the given programtext. The 
	 * difference between hdw_exec and hdw_crexec is that this will allow you
	 * to keep the context for error checking and the like.
	 */
	
	*script = hdw_create();
	
	if (!(*script)) {
		return HDW_ERR_ENV;
	}
	
	return hdw_exec(*script, code);
} 
