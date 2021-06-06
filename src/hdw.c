#include <stdbool.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "hdw.h"

// =============================================================================
// Utilites
// =============================================================================

static char *hdw_strndup(const char * const src, size_t max) {
	size_t len = 0;
	
	while (len < max && src[len] != '\0') {
		++len;
	}
	
	char *string = malloc((len + 1) * sizeof(char));
	
	if (!string) {
		return NULL;
	}
	
	memcpy(string, src, len);
	
	string[len] = '\0';
	
	return string;
}

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

// =============================================================================
// Tokeniser
// =============================================================================

static void hdw_freetokens(hdw_tokenarray *array) {
	/**
	 * Frees an array of tokens.
	 */
	
	free(array->tokens);
}

static int32_t hdw_addtoken(hdw_tokenarray * const array, const uint16_t type, const char *name, const uint32_t line, const uint16_t col) {
	/**
	 * Adds a token to the list at array.
	 */
	
	array->count++;
	array->tokens = realloc(array->tokens, sizeof(hdw_tokenarray) * array->count);
	
	if (!array->tokens) {
		return -1;
	}
	
	array->tokens[array->count - 1].name = name;
	array->tokens[array->count - 1].line = line;
	array->tokens[array->count - 1].col = col;
	array->tokens[array->count - 1].type = type;
	
	return 0;
}

static bool hdw_endtoken(hdw_tokeniser *tokeniser) {
	if (tokeniser->code[tokeniser->head] == '\0') {
		return true;
	}
	
	return false;
}

static char hdw_peektoken(const char * const code, size_t * const head) {
	return code[*head];
}

static char hdw_advancetoken(const char * const code, size_t * const head) {
	return code[(*head)++];
}

static bool hdw_matchtoken(const char * const code, size_t * const head, char what) {
	/**
	 * If the tokens matches the next unconsumed token, this function will 
	 * consume a token and return true; otherwise, it will just return false.
	 */
	
	if (code[*head] == what) {
		(*head)++;
		return true;
	}
	
	return false;
}

static bool hdw_stringtoken(hdw_tokeniser *tokeniser) {
	/**
	 * This function handles a string token, returns true if the string is not
	 * properly terminated.
	 */
	
	size_t start = (tokeniser->head);
	
	while (hdw_peektoken(tokeniser->code, &tokeniser->head) != '"' && !hdw_endtoken(tokeniser)) {
		hdw_advancetoken(tokeniser->code, &tokeniser->head);
	}
	
	if (hdw_endtoken(tokeniser)) {
		return true;
	}
	
	size_t last = (tokeniser->head);
	
	hdw_addtoken(tokeniser->tokens, HDW_STRING, hdw_strndup(&tokeniser->code[start], last - start), 0, 0);
	
	return false;
}

#define SIMPL_TOKEN(TYPE, NAME) hdw_addtoken(tokeniser.tokens, TYPE, NAME, tokeniser.line, tokeniser.col)
#define MATCH(CHAR) hdw_matchtoken(tokeniser.code, &tokeniser.head, CHAR)

int32_t hdw_tokenise(hdw_script * const restrict script, hdw_tokenarray *tokens, const char * const code) {
	/**
	 * Tokenise a stream of characters.
	 */
	
	hdw_tokeniser tokeniser = {
		.tokens = tokens,
		.code = code,
		.len = strlen((char *) code),
		.head = 0,
		.line = 1,
		.col = 1,
		.error = 0,
	};
	
	memset(tokens, 0, sizeof(hdw_tokenarray));
	
	while (tokeniser.head < tokeniser.len) {
		// NOTE: Taking advantage of the postfix ++ operator returning the
		// unicremented value first.
		char current = hdw_advancetoken(tokeniser.code, &tokeniser.head);
		
		// Matching simple tokens //
		/* Parenthesis */
		if      (current == '(') { SIMPL_TOKEN(HDW_PARL, NULL); }
		else if (current == ')') { SIMPL_TOKEN(HDW_PARE, NULL); }
		/* Curly Brackets */
		else if (current == '{') { SIMPL_TOKEN(HDW_CURLYL, NULL); }
		else if (current == '}') { SIMPL_TOKEN(HDW_CURLYE, NULL); }
		/* Brackets */
		else if (current == '[') { SIMPL_TOKEN(HDW_BRAKL, NULL); }
		else if (current == ']') { SIMPL_TOKEN(HDW_BRAKE, NULL); }
		/* Math Operators */
		else if (current == '+') { SIMPL_TOKEN(HDW_PLUS, NULL); }
		else if (current == '-') { SIMPL_TOKEN(HDW_MINUS, NULL); }
		else if (current == '*') { SIMPL_TOKEN(HDW_ASTRESK, NULL); }
		else if (current == '/') { 
			if (MATCH('/')) { // Comment is a special case
				while (tokeniser.code[tokeniser.head] != '\n' && tokeniser.code[tokeniser.head] != '\0') {
					++tokeniser.head;
				}
			}
			else {
				SIMPL_TOKEN(HDW_BACK, NULL);
			}
		}
		else if (current == '%') { SIMPL_TOKEN(HDW_MOD, NULL); }
		/* Comparison and Flow (mostly) */
		else if (current == '!') { SIMPL_TOKEN(MATCH('=') ? HDW_NOTEQ : HDW_NOT, NULL); }
		else if (current == '=') { SIMPL_TOKEN(MATCH('=') ? HDW_EQ : HDW_SET, NULL); }
		else if (current == '<') { SIMPL_TOKEN(MATCH('=') ? HDW_LTEQ : HDW_LT, NULL); }
		else if (current == '>') { SIMPL_TOKEN(MATCH('=') ? HDW_GTEQ : HDW_GT, NULL); }
		else if (current == '&') { SIMPL_TOKEN(MATCH('&') ? HDW_AND : HDW_AMP, NULL); }
		else if (current == '|') { SIMPL_TOKEN(MATCH('|') ? HDW_OR : HDW_BAR, NULL); }
		/* Literals */
		else if (current == '"') {
			if (hdw_stringtoken(&tokeniser)) {
				char *msg = (char *) malloc(256 * sizeof(char));
				if (msg) {
					snprintf(msg, 256, "Line %u, Column %u: Non-terminated string.", tokeniser.line, tokeniser.col);
					hdw_puterror(script, msg);
					tokeniser.error += 1;
				}
			}
		}
		/* Misc */
		else if (current == ';') { SIMPL_TOKEN(HDW_SEMI, NULL); }
		else if (current == ' ' || current == '\t' || current == '\n' || current == '\r') { /* IGNORE */ }
		else {
			char *msg = (char *) malloc(256 * sizeof(char));
			if (msg) {
				snprintf(msg, 256, "Line %u, Column %u: Unrecogised tokeniser character '%c'.", tokeniser.line, tokeniser.col, current);
				hdw_puterror(script, msg);
				tokeniser.error += 1;
			}
		}
		
		if (current == '\n') { tokeniser.line++; tokeniser.col = 0; }
		tokeniser.col++;
	}
	
	return tokeniser.error;
}

#undef MATCH
#undef SIMPL_TOKEN

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
	
	int32_t status = hdw_tokenise(script, &tokens, code);
	
	if (status) {
		hdw_freetokens(&tokens);
		if (script_temp) { hdw_destroy(script); }
		return -3;
	}
	
	for (size_t i = 0; i < tokens.count; i++) {
		printf("%.3d @ (line=%d, col=%d) = %s\n", tokens.tokens[i].type, tokens.tokens[i].line, tokens.tokens[i].col, tokens.tokens[i].name ? tokens.tokens[i].name : "<NULL>");
	}
	
	hdw_freetokens(&tokens);
	//script->errmsg = "Feature is not supported.";
	
	// handle temporary script cleanup
	if (script_temp) {
		hdw_destroy(script);
	}
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
