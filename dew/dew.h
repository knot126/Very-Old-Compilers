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
 * 
 * I am very aware that the project's current state is not really very usable. I
 * hope that I can fix this without rewriting the whole thing again, but 
 */

#ifndef DEW_INCLUDED
#define DEW_INCLUDED

#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>
#include <setjmp.h>

// Basic types
typedef uint8_t dew_Byte;         // byte
typedef size_t dew_Index;         // index
typedef int64_t dew_Integer;      // integer
typedef uint64_t dew_Type;        // type
typedef double dew_Number;        // number
typedef const char * dew_String;  // string
typedef bool dew_Boolean;         // boolean
typedef void * dew_Pointer;       // pointer

// Errors
typedef struct dew_Error {
	dew_Integer offset;
	dew_String message;
} dew_Error;

// Chunk of bytecode
typedef struct dew_Chunk {
	dew_Byte *data;
	size_t count;
	size_t alloc;
} dew_Chunk;

// A script
typedef struct dew_Script {
	/**
	 * Contains all the "global" state for a single script.
	 */
	
	dew_Error *error;
	dew_Index  error_count;
	
	jmp_buf    onError;
	
	dew_Chunk *chunk;
	dew_Index  chunk_count;
} dew_Script;

void dew_init(dew_Script *script);
void dew_free(dew_Script *script);
void dew_pushError(dew_Script *script, dew_Error error);
dew_Error dew_popError(dew_Script *script);
dew_Index dew_countErrors(dew_Script *script);
int dew_raiseError(dew_Script *script, dew_Error error);
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

dew_Index dew_countErrors(dew_Script *script) {
	/**
	 * Return the count of errors.
	 */
	
	return script->error_count;
}

int dew_raiseError(dew_Script *script, dew_Error error) {
	/**
	 * Raise an error and jump back to the safe point.
	 */
	
	dew_pushError(script, error);
	longjmp(script->onError, 2147483647);
}

void dew_panic(const char * const reason) {
	/**
	 * Panic and exit the application.
	 */
	
	printf("\033[1mPANIC\033[0m: %s\n\n", reason);
	
	exit(1);
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
	
	DEW_TOKEN_PLUS,            // '+'
	DEW_TOKEN_MINUS,           // '-'
	DEW_TOKEN_ASTRESK,         // '*'
	DEW_TOKEN_BACKSLASH,       // '/'
	DEW_TOKEN_BANG,            // '!'
	DEW_TOKEN_PERCENT,         // '%'
	DEW_TOKEN_EQUAL,           // '='
	DEW_TOKEN_SEMICOLON,       // ';'
	DEW_TOKEN_PAREN_OPEN,      // '('
	DEW_TOKEN_PAREN_CLOSE,     // ')'
	
	DEW_TOKEN_COMPARE,         // '=='
	DEW_TOKEN_NOTCOMPARE,      // '!='
	DEW_TOKEN_POINTYOPEN,      // '<'
	DEW_TOKEN_POINTYCLOSE,     // '>'
	DEW_TOKEN_LESSEQUAL,       // '<='
	DEW_TOKEN_MOREEQUAL,       // '>='
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

static void dew_freeToken(dew_Token *token) {
	/**
	 * Free a single token.
	 */
	
	if (token->type == DEW_TOKEN_STRING || token->type == DEW_TOKEN_SYMBOL) {
		DEW_FREE(token->value.as_string);
	}
}

static void dew_freeTokenArray(volatile dew_TokenArray *array) {
	/**
	 * Free an array of tokens.
	 */
	
	for (dew_Index i = 0; i < array->count; i++) {
		dew_freeToken(&array->data[i]);
	}
	
	DEW_FREE(array->data);
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
		
		// % token
		else if (current == '%') {
			tok.type = DEW_TOKEN_PERCENT;
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
		
		else if (current == '!') {
			if (code[i + 1] == '=') {
				i++;
				tok.type = DEW_TOKEN_NOTCOMPARE;
				tok.value.as_string = NULL;
			}
			else {
				tok.type = DEW_TOKEN_BANG;
				tok.value.as_string = NULL;
			}
		}
		
		else if (current == '=') {
			if (code[i + 1] == '=') {
				i++;
				tok.type = DEW_TOKEN_COMPARE;
				tok.value.as_string = NULL;
			}
			else {
				tok.type = DEW_TOKEN_EQUAL;
				tok.value.as_string = NULL;
			}
		}
		
		else if (current == '<') {
			if (code[i + 1] == '=') {
				i++;
				tok.type = DEW_TOKEN_LESSEQUAL;
				tok.value.as_string = NULL;
			}
			else {
				tok.type = DEW_TOKEN_POINTYOPEN;
				tok.value.as_string = NULL;
			}
		}
		
		else if (current == '>') {
			if (code[i + 1] == '=') {
				i++;
				tok.type = DEW_TOKEN_MOREEQUAL;
				tok.value.as_string = NULL;
			}
			else {
				tok.type = DEW_TOKEN_POINTYCLOSE;
				tok.value.as_string = NULL;
			}
		}
		
		else if (current == ';') {
			tok.type = DEW_TOKEN_SEMICOLON;
			tok.value.as_string = NULL;
		}
		
		else if (current == '(') {
			tok.type = DEW_TOKEN_PAREN_OPEN;
			tok.value.as_string = NULL;
		}
		
		else if (current == ')') {
			tok.type = DEW_TOKEN_PAREN_CLOSE;
			tok.value.as_string = NULL;
		}
		
		else if (current == '\0') {
			break;
		}
		
		else if (current == ' ' || current == '\t' || current == '\r' || current == '\n') {
			continue;
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
			
			i--;
			
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
		
		else if (current == '"') {
			const dew_Index start = ++i;
			
			// Read the symbol.
			while (code[++i] != '"' && i < len);
			i--;
			
			tok.type = DEW_TOKEN_STRING;
			tok.value.as_string = dew_strndup(&code[start], i - start);
		}
		
		else if (dew_isAlpha(current)) {
			const dew_Index start = i;
			
			// Read the symbol.
			while (dew_isAlphaNumeric(code[++i]) && i < len);
			i--;
			
			tok.type = DEW_TOKEN_SYMBOL;
			tok.value.as_string = dew_strndup(&code[start], i - start);
		}
		
		else {
			dew_pushError(script, (dew_Error) {i + 1, "The character is not recognised."});
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
	DEW_RULE_URANRY,
	DEW_RULE_LINEAR,
	DEW_RULE_SUBLINEAR,
	DEW_RULE_COMPARE,
	DEW_RULE_EQUALITY,
	DEW_RULE_EXPRESSION,
	DEW_RULE_STATEMENT,
	DEW_RULE_EXPR_STATEMENT,
	DEW_RULE_VAR_DECLARE,
	DEW_RULE_ASSIGN,
} dew_Rule;

enum {
	DEW_NODE_INVALID = 0,
	DEW_NODE_SEQUENCE,
	DEW_NODE_GROUPING,
	
	DEW_NODE_INTEGER,
	DEW_NODE_NUMBER,
	DEW_NODE_STRING,
	DEW_NODE_SYMBOL,
	DEW_NODE_NULL,
	
	DEW_NODE_ADD,
	DEW_NODE_SUBTRACT,
	DEW_NODE_MULTIPLY,
	DEW_NODE_DIVIDE,
	DEW_NODE_MODULO,
	
	DEW_NODE_NOT,
	DEW_NODE_OPPOSITE,
	
	DEW_NODE_LESS,
	DEW_NODE_LESS_EQUAL,
	DEW_NODE_GREATER,
	DEW_NODE_GREATER_EQUAL,
	
	DEW_NODE_EQUAL,
	DEW_NODE_NOT_EQUAL,
	
	DEW_NODE_VAR_DECLARE,
	DEW_NODE_ASSIGN,
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
	/**
	 * Lower-level function to allocate trees.
	 */
	
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

static void dew_treeFree(dew_TreeNode *node, size_t primary) {
	if (node->type == DEW_NODE_SYMBOL || node->type == DEW_NODE_STRING) {
// 		DEW_FREE((void *) node->value.as_string);
	}
	
	for (size_t i = 0; i < node->sub_count; i++) {
		dew_treeFree(&node->sub[i], 1);
	}
	
	DEW_FREE(node->sub);
	
	if (primary == 0) {
		DEW_FREE(node);
	}
}

static dew_TreeNode *dew_appendNode(dew_TreeNode *node, dew_TreeNode *app) {
	/**
	 * Append a node to another node.
	 */
	
	node->sub = DEW_REALLOCATE(node->sub, sizeof *node->sub * ++node->sub_count);
	
	if (!node->sub) {
		DEW_FREE(app);
		return NULL;
	}
	
	node->sub[node->sub_count - 1] = *app;
	
	DEW_FREE(app);
	
	return node;
}

static dew_TreeNode *dew_makeLiteralNode(dew_Integer type, dew_Value value) {
	/**
	 * Creates a node with no children.
	 */
	
	dew_TreeNode *node = dew_makeTree(0);
	
	if (!node) {
		return NULL;
	}
	
	node->type = type;
	node->value = value;
	
	return node;
}

static dew_TreeNode *dew_uranryTree(dew_Integer type, dew_Value value, dew_TreeNode *left) {
	/**
	 * Allocates a uranry tree.
	 */
	
	dew_TreeNode *node = dew_makeTree(1);
	
	if (!node) return NULL;
	
	node->type = type;
	node->value = value;
	node->sub[0] = *left;
	
	DEW_FREE(left);
	
	return node;
}

static dew_TreeNode *dew_binaryTree(dew_Integer type, dew_Value value, dew_TreeNode *left, dew_TreeNode *right) {
	/**
	 * Allocates a binary tree.
	 */
	
	dew_TreeNode *node = dew_makeTree(2);
	
	if (!node) {
		return NULL;
	}
	
	node->type = type;
	node->value = value;
	node->sub[0] = *left;
	node->sub[1] = *right;
	
	DEW_FREE(left);
	DEW_FREE(right);
	
	return node;
}

static dew_TreeNode *dew_trinaryTree(dew_Integer type, dew_Value value, dew_TreeNode *left, dew_TreeNode *centre, dew_TreeNode *right) {
	/**
	 * Allocates a binary tree.
	 */
	
	dew_TreeNode *node = dew_makeTree(3);
	
	if (!node) {
		return NULL;
	}
	
	node->type = type;
	node->value = value;
	node->sub[0] = *left;
	node->sub[1] = *centre;
	node->sub[2] = *right;
	
	DEW_FREE(left);
	DEW_FREE(centre);
	DEW_FREE(right);
	
	return node;
}

#define CURRENT_TOKEN parser->code->data[parser->head]
#define GET_TOKEN(OFFSET) parser->code->data[parser->head + OFFSET]

static dew_TreeNode *dew_match(dew_Script *script, dew_Parser *parser, dew_Rule rule) {
	/**
	 * Match the given rule, and return a tree of all that encompasses the 
	 * matched rule.
	 */
	
	// Literals
	if (rule == DEW_RULE_LITERAL) {
		dew_Integer type;
		
		switch (CURRENT_TOKEN.type) {
			case DEW_TOKEN_NUMBER: type = DEW_NODE_NUMBER; break;
			case DEW_TOKEN_INTEGER: type = DEW_NODE_INTEGER; break;
			case DEW_TOKEN_STRING: type = DEW_NODE_STRING; break;
			case DEW_TOKEN_SYMBOL: type = DEW_NODE_SYMBOL; break;
			case DEW_TOKEN_PAREN_OPEN: type = DEW_NODE_GROUPING; break;
			default: type = DEW_NODE_INVALID; break;
		}
		
		if (type == DEW_NODE_GROUPING) {
			parser->head++;
			
			dew_TreeNode *left = dew_match(script, parser, DEW_RULE_EXPRESSION);
			
			if (!left) {
				return NULL;
			}
			
			dew_TreeNode *n = dew_uranryTree(DEW_NODE_GROUPING, (dew_Value) {0}, left);
			
			if (!n) {
				dew_treeFree(left, 0);
				return NULL;
			}
			
			if (n && CURRENT_TOKEN.type != DEW_TOKEN_PAREN_CLOSE) {
				dew_raiseError(script, (dew_Error) {parser->head, "Error: Expected ')' to end grouping."});
			}
			else {
				parser->head++;
			}
			
			return n;
		}
		else if (type != DEW_NODE_INVALID) {
			dew_TreeNode *res = dew_makeLiteralNode(type, CURRENT_TOKEN.value);
			
			if (!res) {
				return NULL;
			}
			
			parser->head++;
			
			return res;
		}
		
		/* Fallthrough to NULL */
	}
	
	// Uranary Operators
	else if (rule == DEW_RULE_URANRY) {
		if (CURRENT_TOKEN.type == DEW_TOKEN_MINUS || CURRENT_TOKEN.type == DEW_TOKEN_BANG) {
			dew_Integer type;
			
			switch (CURRENT_TOKEN.type) {
				case DEW_TOKEN_MINUS: type = DEW_NODE_OPPOSITE; break;
				case DEW_TOKEN_BANG: type = DEW_NODE_NOT; break;
			}
			
			parser->head++;
			
			dew_TreeNode *left = dew_match(script, parser, DEW_RULE_URANRY);
			
			if (!left) {
				dew_treeFree(left, 0);
				return NULL;
			}
			
			dew_TreeNode *tree = dew_uranryTree(type, (dew_Value) {0}, left);
			
			return tree;
		}
		
		return dew_match(script, parser, DEW_RULE_LITERAL);
	}
	
	// Multiply, Divide and Modulo (The linear operators)
	else if (rule == DEW_RULE_LINEAR) {
		dew_TreeNode *left = dew_match(script, parser, DEW_RULE_URANRY);
		
		while (CURRENT_TOKEN.type == DEW_TOKEN_ASTRESK || CURRENT_TOKEN.type == DEW_TOKEN_BACKSLASH || CURRENT_TOKEN.type == DEW_TOKEN_PERCENT) {
			dew_Integer type;
			
			switch (CURRENT_TOKEN.type) {
				case DEW_TOKEN_ASTRESK: type = DEW_NODE_MULTIPLY; break;
				case DEW_TOKEN_BACKSLASH: type = DEW_NODE_DIVIDE; break;
				case DEW_TOKEN_PERCENT: type = DEW_NODE_MODULO; break;
			}
			
			parser->head++;
			
			dew_TreeNode *right = dew_match(script, parser, DEW_RULE_URANRY);
			
			left = dew_binaryTree(type, (dew_Value) {0}, left, right);
		}
		
		return left;
	}
	
	// Sublinear (addition and subtraction)
	else if (rule == DEW_RULE_SUBLINEAR) {
		dew_TreeNode *left = dew_match(script, parser, DEW_RULE_LINEAR);
		
		while (CURRENT_TOKEN.type == DEW_TOKEN_PLUS || CURRENT_TOKEN.type == DEW_TOKEN_MINUS) {
			dew_Integer type;
			
			switch (CURRENT_TOKEN.type) {
				case DEW_TOKEN_PLUS: type = DEW_NODE_ADD; break;
				case DEW_TOKEN_MINUS: type = DEW_NODE_SUBTRACT; break;
			}
			
			parser->head++;
			
			dew_TreeNode *right = dew_match(script, parser, DEW_RULE_LINEAR);
			
			left = dew_binaryTree(type, (dew_Value) {0}, left, right);
		}
		
		return left;
	}
	
	// Compare (on numbers)
	else if (rule == DEW_RULE_COMPARE) {
		dew_TreeNode *left = dew_match(script, parser, DEW_RULE_SUBLINEAR);
		
		while (CURRENT_TOKEN.type == DEW_TOKEN_POINTYOPEN || CURRENT_TOKEN.type == DEW_TOKEN_POINTYCLOSE || CURRENT_TOKEN.type == DEW_TOKEN_LESSEQUAL || CURRENT_TOKEN.type == DEW_TOKEN_MOREEQUAL) {
			dew_Integer type;
			
			switch (CURRENT_TOKEN.type) {
				case DEW_TOKEN_POINTYOPEN: type = DEW_NODE_LESS; break;
				case DEW_TOKEN_POINTYCLOSE: type = DEW_NODE_GREATER; break;
				case DEW_TOKEN_LESSEQUAL: type = DEW_NODE_LESS_EQUAL; break;
				case DEW_TOKEN_MOREEQUAL: type = DEW_NODE_GREATER_EQUAL; break;
			}
			
			parser->head++;
			
			dew_TreeNode *right = dew_match(script, parser, DEW_RULE_SUBLINEAR);
			
			left = dew_binaryTree(type, (dew_Value) {0}, left, right);
		}
		
		return left;
	}
	
	// Compare (on numbers)
	else if (rule == DEW_RULE_EQUALITY) {
		dew_TreeNode *left = dew_match(script, parser, DEW_RULE_COMPARE);
		
		while (CURRENT_TOKEN.type == DEW_TOKEN_NOTCOMPARE || CURRENT_TOKEN.type == DEW_TOKEN_COMPARE) {
			dew_Integer type;
			
			switch (CURRENT_TOKEN.type) {
				case DEW_TOKEN_NOTCOMPARE: type = DEW_NODE_NOT_EQUAL; break;
				case DEW_TOKEN_COMPARE: type = DEW_NODE_EQUAL; break;
			}
			
			parser->head++;
			
			dew_TreeNode *right = dew_match(script, parser, DEW_RULE_COMPARE);
			
			left = dew_binaryTree(type, (dew_Value) {0}, left, right);
		}
		
		return left;
	}
	
	// Expression
	else if (rule == DEW_RULE_EXPRESSION) {
		dew_TreeNode *left = dew_match(script, parser, DEW_RULE_EQUALITY);
		
		return left;
	}
	
	// ExprStatement
	else if (rule == DEW_RULE_EXPR_STATEMENT) {
		dew_TreeNode *left = dew_match(script, parser, DEW_RULE_EXPRESSION);
		
		if (left && CURRENT_TOKEN.type != DEW_TOKEN_SEMICOLON) {
			dew_raiseError(script, (dew_Error) {parser->head, "Error: Expected ';' to end statement."});
		}
		
		parser->head++;
		
		return left;
	}
	
	// Variable Declaration
	else if (rule == DEW_RULE_VAR_DECLARE) {
		dew_TreeNode *type = dew_makeLiteralNode(DEW_NODE_SYMBOL, CURRENT_TOKEN.value);
		parser->head++;
		
		dew_TreeNode *name = dew_makeLiteralNode(DEW_NODE_SYMBOL, CURRENT_TOKEN.value);
		parser->head++;
		
		dew_TreeNode *value;
		
		if (CURRENT_TOKEN.type == DEW_TOKEN_EQUAL) {
			parser->head++;
			
			value = dew_match(script, parser, DEW_RULE_EXPRESSION);
		}
		else {
			value = dew_makeLiteralNode(DEW_NODE_NULL, (dew_Value) {0});
		}
		
		return dew_trinaryTree(DEW_NODE_VAR_DECLARE, (dew_Value) {0}, type, name, value);
	}
	
	// Expression
	else if (rule == DEW_RULE_STATEMENT || rule == DEW_RULE_DEFAULT) {
		if (CURRENT_TOKEN.type == DEW_TOKEN_SYMBOL && GET_TOKEN(1).type == DEW_TOKEN_SYMBOL) {
			return dew_match(script, parser, DEW_RULE_VAR_DECLARE);
		}
		
		return dew_match(script, parser, DEW_RULE_EXPR_STATEMENT);
	}
	
	return NULL;
}

#undef CURRENT_TOKEN
#undef GET_TOKEN

static dew_TreeNode *dew_parse(dew_Script *script, dew_TokenArray *code) {
	/**
	 * Parse a token array into an AST.
	 */
	
	// init parser
	dew_Parser parser;
	memset(&parser, 0, sizeof parser);
	parser.code = code;
	
	// parse code
	dew_TreeNode *root = dew_makeTree(0);
	
	root->type = DEW_NODE_SEQUENCE;
	root->value.as_integer = 0;
	
	dew_TreeNode *next;
	
	do {
		next = dew_match(script, &parser, DEW_RULE_DEFAULT);
		if (next) {
			dew_appendNode(root, next);
		}
	} while (next);
	
	if (!root) {
		dew_pushError(script, (dew_Error) {parser.code->data[parser.head].offset, "Failed to create parse node."});
	}
	
	return root;
}

static const char *dew_nodeTypeString(dew_Index i) {
	switch (i) {
		case DEW_NODE_INVALID: return "DEW_NODE_INVALID";
		
		case DEW_NODE_SEQUENCE: return "DEW_NODE_SEQUENCE";
		case DEW_NODE_GROUPING: return "DEW_NODE_GROUPING";
		
		case DEW_NODE_NULL: return "DEW_NODE_NULL";
		case DEW_NODE_INTEGER: return "DEW_NODE_INTEGER";
		case DEW_NODE_NUMBER: return "DEW_NODE_NUMBER";
		case DEW_NODE_STRING: return "DEW_NODE_STRING";
		case DEW_NODE_SYMBOL: return "DEW_NODE_SYMBOL";
		
		case DEW_NODE_ADD: return "DEW_NODE_ADD";
		case DEW_NODE_SUBTRACT: return "DEW_NODE_SUBTRACT";
		case DEW_NODE_MULTIPLY: return "DEW_NODE_MULTIPLY";
		case DEW_NODE_DIVIDE: return "DEW_NODE_DIVIDE";
		case DEW_NODE_MODULO: return "DEW_NODE_MODULO";
		
		case DEW_NODE_LESS: return "DEW_NODE_LESS";
		case DEW_NODE_LESS_EQUAL: return "DEW_NODE_LESS_EQUAL";
		case DEW_NODE_GREATER: return "DEW_NODE_GREATER";
		case DEW_NODE_GREATER_EQUAL: return "DEW_NODE_GREATER_EQUAL";
		case DEW_NODE_EQUAL: return "DEW_NODE_EQUAL";
		case DEW_NODE_NOT_EQUAL: return "DEW_NODE_NOT_EQUAL";
		
		case DEW_NODE_VAR_DECLARE: return "DEW_NODE_VAR_DECLARE";
		case DEW_NODE_ASSIGN: return "DEW_NODE_ASSIGN";
		
		default: return "Node";
	}
}

static void dew_printTree(dew_TreeNode *node, const dew_Index level) {
	/**
	 * Prints out a tree node.
	 */
	
	if (node) {
		for (dew_Index i = 0; i < level; i++) {
			printf("\t");
		}
		
		printf("\033[1m%s\033[0m (%.16X", dew_nodeTypeString(node->type), node->value.as_integer);
		if (node->type == DEW_NODE_STRING || node->type == DEW_NODE_SYMBOL) {
			printf(" = \"%s\"", node->value.as_string);
		}
		else if (node->type == DEW_NODE_INTEGER) {
			printf(" = %d", node->value.as_integer);
		}
		else if (node->type == DEW_NODE_NUMBER) {
			printf(" = %f", node->value.as_number);
		}
		printf("):\n");
		
		for (dew_Index i = 0; i < node->sub_count; i++) {
			dew_printTree(&node->sub[i], level + 1);
		}
	}
	else {
		puts("(null)");
	}
}

/**
 * =============================================================================
 * Virtual Machine
 * =============================================================================
 */

// Opcodes
enum {
	DEW_OP_NOP = 0,
	DEW_OP_RET,
	DEW_OP_SET,
};

// Dynamic Array Implementation
// typedef struct dew_Chunk {
//   dew_Byte *data;
//   size_t count;
//   size_t alloc;
// } dew_Chunk;

static dew_Chunk *dew_chunkInit(void) {
	dew_Chunk *chunk = DEW_ALLOCATE(sizeof *chunk);
	
	if (!chunk) {
		return NULL;
	}
	
	chunk->data = NULL;
	chunk->count = 0;
	chunk->alloc = 0;
	
	return chunk;
}

static void dew_chunkFree(dew_Chunk *chunk) {
	if (chunk->data) {
		DEW_FREE(chunk);
	}
	
	DEW_FREE(chunk);
}

static void dew_addChunk(dew_Chunk *chunk, uint8_t byte) {
	if (chunk->count >= chunk->alloc) {
		chunk->alloc = 2 + chunk->alloc * 2;
		chunk->data = DEW_REALLOCATE(chunk->data, chunk->alloc);
		
		if (!chunk->data) {
			dew_panic("Failed to allocate chunk memory.");
		}
	}
}

/**
 * =============================================================================
 * Script Chunk Running
 * =============================================================================
 */

dew_Error dew_runChunk(dew_Script *script, dew_String code) {
	/**
	 * Runs a chunk of code. ´code´ should be a string to the code, and ´script´
	 * should be an active scripting instance.
	 * 
	 * IMPLEMENTATION NOTES
	 * ====================
	 *   * I use a lot of casts to avoid adding volatile to the function
	 *     declarations because I wrote them before the error infrastrcture was
	 *     in place. In the future, these should be changed so that the
	 *     functions accept volatile arguments.
	 */
	
	// Initialise token array
	// Note: Use volatite otherwise the exact contents of tokens and tree will
	// not be defined after a longjmp.
	// https://man7.org/linux/man-pages/man3/setjmp.3.html § NOTES
	volatile dew_TokenArray tokens = {NULL, 0};
	volatile dew_TreeNode *tree = NULL;
	
	int result = setjmp(script->onError);
	
	// We are running for the first time
	if (!result) {
		// Tokenise code
		dew_tokenise(script, (dew_TokenArray *) &tokens, code);
		
		// Check for errors
		if (!tokens.count || !tokens.data) {
			return (dew_Error) {-1, "No tokens to be had, which cannot be a valid input."};
		}
		
		if (dew_countErrors(script)) {
			return (dew_Error) {-1, "Tokenising failed."};
		}
		
		// Parse tokens
		tree = dew_parse(script, (dew_TokenArray *) &tokens);
		
		for (size_t i = 0; i < tokens.count; i++) {
			printf("Char(%.3d) -> %.3d : %.16X\n", i + 1, tokens.data[i].type, tokens.data[i].value.as_integer);
		}
		
		if (dew_countErrors(script)) {
			return (dew_Error) {-1, "Parsing failed."};
		}
		
		dew_printTree((dew_TreeNode *) tree, 0);
		
		if (tokens.count) {
			dew_freeTokenArray(&tokens);
		}
		
		if (tree) {
			dew_treeFree((dew_TreeNode *) tree, 0);
		}
		
		return (dew_Error) {0, "Finished okay!"};
	}
	// There was an error, note that it's on the error stack so this is 
	// (probably) a bit more acceptable than if we just returned normally.
	else {
		if (tokens.count) {
			dew_freeTokenArray(&tokens);
		}
		
		if (tree) {
			dew_treeFree((dew_TreeNode *) tree, 0);
		}
		
		return (dew_Error) {result, "Failed to run program."};
	}
}

#endif
