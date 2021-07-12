/**
 * DewScript Main Header
 * =====================
 * 
 * This is the main file for Honeydew. This language is self-contained into this
 * single file.
 * 
 * In a single source code file, you should define DEW_IMPLEMENTATION to get the
 * implementation into that compilation unit. In the rest of the files, you 
 * should just include this header normally. This is similar to how the stb
 * series of single-file libraries work.
 * 
 * You can define a custom allocator using the macros DEW_ALLOCATE, 
 * DEW_REALLOCATE and DEW_FREE respective to their functions.
 */

#ifndef DEW_INCLUDED
#define DEW_INCLUDED

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>

// Basic types
typedef uint8_t dew_Byte;         // byte
typedef size_t dew_Index;         // index
typedef int64_t dew_Integer;      // integer
typedef uint64_t dew_Type;        // type
typedef double dew_Number;        // number
typedef const char * dew_String;  // string
typedef bool dew_Boolean;         // boolean
typedef void * dew_Pointer;       // pointer

typedef struct dew_Error {
	dew_Integer offset;
	dew_String message;
} dew_Error;

typedef struct dew_Script {
	/**
	 * Contains all the "global" state for a single script.
	 */
	
	dew_Error *error;
	dew_Index  error_count;
	
	
} dew_Script;

void dew_init(dew_Script *script);
void dew_free(dew_Script *script);
void dew_pushError(dew_Script *script, dew_Error error);
dew_Error dew_popError(dew_Script *script);
dew_Error dew_runChunk(dew_Script *script, dew_String code);

#endif

/**
 * =============================================================================
 * =============================================================================
 * =============================================================================
 */

#ifdef DEW_IMPLEMENTATION
#undef DEW_IMPLEMENTATION

#include <string.h>

/**
 * =============================================================================
 * Defines
 * =============================================================================
 */

// Custom allocator
#ifndef DEW_ALLOCATE
#define DEW_ALLOCATE malloc
#endif // DEW_ALLOCATE

#ifndef DEW_REALLOCATE
#define DEW_REALLOCATE realloc
#endif // DEW_REALLOCATE

#ifndef DEW_FREE
#define DEW_FREE(x) free((void *) x)
#endif // DEW_FREE

/**
 * =============================================================================
 * Script Instance Mangement
 * =============================================================================
 */

void dew_init(dew_Script *script) {
	/**
	 * Initialises the script at the given address.
	 */
	
	memset(script, 0, sizeof *script);
}

void dew_free(dew_Script *script) {
	/**
	 * Frees the script at the given address.
	 */
	
	if (script->error) {
		DEW_FREE(script->error);
	}
}

/**
 * =============================================================================
 * Errors
 * =============================================================================
 */

void dew_pushError(dew_Script *script, dew_Error error) {
	/**
	 * Push a new error on top of the script.
	 */
	
	script->error = DEW_REALLOCATE(script->error, sizeof *script->error * ++script->error_count);
	script->error[script->error_count - 1] = error;
}

dew_Error dew_popError(dew_Script *script) {
	/**
	 * Pop an error off the top of the error stack.
	 */
	
	// Get the error
	if (script->error_count == 0) {
		return (dew_Error) {0, NULL};
	}
	
	dew_Error error = script->error[0];
	
	// Pop the stack
	if (script->error_count - 1 > 0) {
		memmove(script->error, &script->error[1], sizeof *script->error * --script->error_count);
		script->error = DEW_REALLOCATE(script->error, sizeof *script->error * script->error_count);
	}
	else {
		DEW_FREE(script->error);
		script->error = NULL;
		script->error_count = 0;
	}
	
	return error;
}

/**
 * =============================================================================
 * Tokeniser
 * =============================================================================
 */

enum {
	DEW_TOKEN_INVALID = 0,
	DEW_TOKEN_NUMBER,
	DEW_TOKEN_STRING,
	DEW_TOKEN_SYMBOL,
	DEW_TOKEN_INTEGER,
	DEW_TOKEN_KEYWORD,
	
	DEW_TOKEN_PLUS,
	DEW_TOKEN_MINUS,
	DEW_TOKEN_ASTRESK,
	DEW_TOKEN_BACKSLASH,
};

typedef union dew_Value {
	dew_Integer as_integer;
	dew_Number as_number;
	dew_String as_string;
	dew_Boolean as_boolean;
} dew_Value;

typedef struct dew_Token {
	dew_Integer type;
	dew_Value value;
	
	// Location information
	dew_Index offset;
	dew_Index end;
} dew_Token;

typedef struct dew_TokenArray {
	dew_Token *data;
	size_t count;
} dew_TokenArray;

static dew_String dew_strndup(dew_String str, const dew_Index max) {
	/**
	 * The same as C23 or POSIX strndup.
	 */
	
	dew_Index len = 0;
	
	// Find needed length
	while ((str[len] != '\0') && (len <= max)) {
		++len;
	}
	
	// Allocate memory
	char *res = DEW_ALLOCATE(len + 1);
	
	if (!res) {
		return NULL;
	}
	
	// Copy chars
	for (dew_Index i = 0; i < len; i++) {
		res[i] = str[i];
	}
	
	// Set null termination
	res[len] = '\0';
	
	// Return new string
	return res;
}

static dew_Boolean dew_isAlpha(char c) {
	/**
	 * Return true if the char is alphabetical, false otherwise.
	 */
	
	return ( ((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')) || c == '_' );
}

static dew_Boolean dew_isNumeric(char c) {
	/**
	 * Return true if the char is numerical, false otherwise.
	 */
	
	return ( ((c >= '0') && (c <= '9')) || c == '.' );
}

static dew_Boolean dew_isAlphaNumeric(char c) {
	/**
	 * Return true if char is alpha or numeric, false otherwise.
	 */
	
	return ( dew_isAlpha(c) || dew_isNumeric(c) );
}

static void dew_pushToken(dew_Script *script, dew_TokenArray *array, dew_Token token) {
	/**
	 * Add a token to a token array.
	 */
	
	array->data = DEW_REALLOCATE(array->data, sizeof *array->data * ++array->count);
	
	if (!array->data) {
		dew_pushError(script, (dew_Error) {-1, "Failed to allocate memory."});
		return;
	}
	
	array->data[array->count - 1] = token;
}

static void dew_tokenise(dew_Script *script, dew_TokenArray *array, dew_String code) {
	/**
	 * Tokenise a string of code.
	 */
	
	const dew_Index len = strlen(code);
	
	for (dew_Index i = 0; i < len; i++) {
		const char current = code[i];
		
		dew_Token tok;
		
		tok.offset = i;
		
		// + token
		if (current == '+') {
			tok.type = DEW_TOKEN_PLUS;
			tok.value.as_string = NULL;
		}
		
		// * token
		else if (current == '*') {
			tok.type = DEW_TOKEN_ASTRESK;
			tok.value.as_string = NULL;
		}
		
		// - token
		else if (current == '-') {
			tok.type = DEW_TOKEN_MINUS;
			tok.value.as_string = NULL;
		}
		
		// / token
		else if (current == '/') {
			// Single-Line Comment
			if (code[i + 1] == '/') {
				while (++i < len && code[i] != '\n');
				continue;
			}
			
			// Multi-line Comment
			else if (code[i + 1] == '*') {
				while (i++ < len && code[i++] != '*' && code[i] != '/');
				continue;
			}
			
			// Single Backslash
			else {
				tok.type = DEW_TOKEN_BACKSLASH;
				tok.value.as_string = NULL;
			}
		}
		
		else if (dew_isNumeric(current)) {
			dew_Boolean isint = true;
			const dew_Index start = i;
			
			// Read numbers. If has a decimal, set to not being an integer
			while (dew_isNumeric(code[++i]) && i < len) {
				if (code[i] == '.') {
					isint = false;
				}
			}
			
			dew_String numstr = dew_strndup(&code[start], i - start);
			
			if (isint) {
				tok.type = DEW_TOKEN_INTEGER;
				tok.value.as_integer = atoll(numstr);
			}
			else {
				tok.type = DEW_TOKEN_NUMBER;
				tok.value.as_number = strtod(numstr, NULL);
			}
			
			DEW_FREE(numstr);
		}
		
		else {
			dew_pushError(script, (dew_Error) {i, "Invalid token."});
			continue;
		}
		
		tok.end = i;
		
		dew_pushToken(script, array, tok);
	}
}

/**
 * =============================================================================
 * Parser
 * =============================================================================
 */

typedef enum dew_Rule {
	DEW_RULE_DEFAULT = 0,
	DEW_RULE_LITERAL,
} dew_Rule;

enum {
	DEW_NODE_INVALID = 0,
	DEW_NODE_INTEGER,
	DEW_NODE_NUMBER,
	DEW_NODE_STRING,
	DEW_NODE_SYMBOL,
	
	DEW_NODE_ADD,
	DEW_NODE_SUBTRACT,
	DEW_NODE_MULTIPLY,
	DEW_NODE_DIVIDE,
	DEW_NODE_MODULO,
};

typedef struct dew_TreeNode {
	dew_Integer type;
	dew_Value value;
	struct dew_TreeNode *sub;
	dew_Index sub_count;
	
	// Location information
	dew_Index offset;
	dew_Index end;
} dew_TreeNode;

typedef struct dew_Parser {
	dew_TreeNode *root;
	dew_TokenArray *code;
	dew_Index head;
} dew_Parser;

static dew_TreeNode *dew_makeTree(dew_Index subnodes) {
	dew_TreeNode *node = DEW_ALLOCATE(sizeof *node);
	
	if (!node) {
		return NULL;
	}
	
	if (subnodes > 0) {
		dew_TreeNode *sub = DEW_ALLOCATE(sizeof *sub * subnodes);
		
		if (!sub) {
			DEW_FREE(node);
			return NULL;
		}
		
		node->sub = sub;
		node->sub_count = subnodes;
	}
	else {
		node->sub = NULL;
		node->sub_count = 0;
	}
	
	return node;
}

static dew_TreeNode *dew_makeLiteralNode(dew_Integer type, dew_Value value) {
	dew_TreeNode *node = dew_makeTree(0);
	
	node->type = type;
	node->value = value;
	
	return node;
}

#define CURRENT_TOKEN parser->code->data[parser->head]
#define GET_TOKEN(OFFSET) parser->code->data[parser->head + OFFSET]

static dew_TreeNode *dew_match(dew_Parser *parser, dew_TreeNode *tree, dew_Rule rule) {
	if (rule == DEW_RULE_LITERAL || rule == DEW_RULE_DEFAULT) {
		dew_Integer type;
		
		switch (CURRENT_TOKEN.type) {
			case DEW_TOKEN_NUMBER: type = DEW_NODE_NUMBER; break;
			case DEW_TOKEN_INTEGER: type = DEW_NODE_INTEGER; break;
			case DEW_TOKEN_STRING: type = DEW_NODE_STRING; break;
			case DEW_TOKEN_SYMBOL: type = DEW_NODE_SYMBOL; break;
			default: type = DEW_NODE_INVALID; break;
		}
		
		return dew_makeLiteralNode(type, CURRENT_TOKEN.value);
	}
	
	return NULL;
}

#undef CURRENT_TOKEN
#undef GET_TOKEN

static void dew_parse(dew_Script *script, dew_TreeNode *tree, dew_TokenArray *code) {
	// init parser
	dew_Parser parser;
	memset(&parser, 0, sizeof parser);
	parser.code = code;
	
	// parse code
	dew_TreeNode *node = dew_match(&parser, tree, DEW_RULE_DEFAULT);
	
	if (!node) {
		dew_pushError(script, (dew_Error) {parser.code->data[parser.head].offset, "Failed to create parse node."});
	}
}

/**
 * =============================================================================
 * Script Chunk Running
 * =============================================================================
 */

dew_Error dew_runChunk(dew_Script *script, dew_String code) {
	/**
	 * Run a chunk of code.
	 */
	
	// Initialise token array
	dew_TokenArray tokens;
	memset(&tokens, 0, sizeof tokens);
	
	// Tokenise code
	dew_tokenise(script, &tokens, code);
	
	// Check for errors
	if (!tokens.count || !tokens.data) {
		return (dew_Error) {-1, "No tokens to be had, which cannot be a valid input."};
	}
	
	// Initialise tree node
	dew_TreeNode tree;
	memset(&tree, 0, sizeof tree);
	
	// Parse tokens
	dew_parse(script, &tree, &tokens);
	
	for (size_t i = 0; i < tokens.count; i++) {
		printf("%.3d -> %.3d : %.16X\n", i, tokens.data[i].type, tokens.data[i].value.as_integer);
	}
	
	return (dew_Error) {0, "Finished okay!"};
}

#endif
