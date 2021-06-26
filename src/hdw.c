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
	/**
	 * Self-made implementation of the C23/POSIX function by a similar name, strndup.
	 */
	
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

static bool hdw_isdigit(char what) {
	return (what >= '0' && what <= '9');
}

static bool hdw_isradix(char what) {
	return (what == '.');
}

static bool hdw_isalpha(char what) {
	return ((what >= 'a' && what <= 'z') || (what >= 'A' && what <= 'Z') || what == '_');
}

static bool hdw_isalphanumeric(char what) {
	return (hdw_isalpha(what) || hdw_isdigit(what));
}

static uint16_t hdw_findkeyword(const size_t size, const hdw_keywordmap * const map, const char * const key) {
	for (size_t i = 0; i < size; i++) {
		if (!strcmp(map[i].key, key)) {
			return map[i].value;
		}
	}
	
	return HDW_UNKNOWN;
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

static int32_t hdw_addtoken(hdw_tokeniser * const tokeniser, const uint16_t type, const char *name) {
	/**
	 * Adds a token to the list at array.
	 */
	
	tokeniser->tokens->count++;
	tokeniser->tokens->tokens = realloc(tokeniser->tokens->tokens, sizeof(hdw_tokenarray) * tokeniser->tokens->count);
	
	if (!tokeniser->tokens->tokens) {
		return -1;
	}
	
	tokeniser->tokens->tokens[tokeniser->tokens->count - 1].name = name;
	tokeniser->tokens->tokens[tokeniser->tokens->count - 1].line = tokeniser->line;
	tokeniser->tokens->tokens[tokeniser->tokens->count - 1].col = tokeniser->col;
	tokeniser->tokens->tokens[tokeniser->tokens->count - 1].type = type;
	
	return 0;
}

static int32_t hdw_addinttoken(hdw_tokeniser * const tokeniser, const int64_t value) {
	/**
	 * Adds a integer token to the list of tokens.
	 */
	
	tokeniser->tokens->count++;
	tokeniser->tokens->tokens = realloc(tokeniser->tokens->tokens, sizeof(hdw_tokenarray) * tokeniser->tokens->count);
	
	if (!tokeniser->tokens->tokens) {
		return -1;
	}
	
	tokeniser->tokens->tokens[tokeniser->tokens->count - 1].int_value = value;
	tokeniser->tokens->tokens[tokeniser->tokens->count - 1].line = tokeniser->line;
	tokeniser->tokens->tokens[tokeniser->tokens->count - 1].col = tokeniser->col;
	tokeniser->tokens->tokens[tokeniser->tokens->count - 1].type = HDW_INTEGER;
	
	return 0;
}

static int32_t hdw_adddectoken(hdw_tokeniser * const tokeniser, const double value) {
	/**
	 * Adds a decimal token to the list of tokens.
	 */
	
	tokeniser->tokens->count++;
	tokeniser->tokens->tokens = realloc(tokeniser->tokens->tokens, sizeof(hdw_tokenarray) * tokeniser->tokens->count);
	
	if (!tokeniser->tokens->tokens) {
		return -1;
	}
	
	tokeniser->tokens->tokens[tokeniser->tokens->count - 1].dec_value = value;
	tokeniser->tokens->tokens[tokeniser->tokens->count - 1].line = tokeniser->line;
	tokeniser->tokens->tokens[tokeniser->tokens->count - 1].col = tokeniser->col;
	tokeniser->tokens->tokens[tokeniser->tokens->count - 1].type = HDW_NUMBER;
	
	return 0;
}

static bool hdw_endtoken(hdw_tokeniser * const tokeniser) {
	/**
	 * Returns true if the next unconsumed is the end token, or false if is not.
	 */
	
	if (tokeniser->code[tokeniser->head] == '\0') {
		return true;
	}
	
	return false;
}

static char hdw_peektoken(hdw_tokeniser * const tokeniser) {
	/**
	 * Look at the next token without consuming it.
	 */
	
	return tokeniser->code[tokeniser->head];
}

static char hdw_peektoken2(hdw_tokeniser * const tokeniser) {
	/**
	 * Look at the next next token without consuming anything.
	 *          ~
	 * Ex:    A B C D E F G ...
	 *            ^   ^- look here
	 *            |_ you are here
	 */
	
	return tokeniser->code[tokeniser->head + 1];
}

static char hdw_advancetoken(hdw_tokeniser * const tokeniser) {
	/**
	 * Return a token and then advance the head.
	 */
	
	char v = tokeniser->code[tokeniser->head];
	tokeniser->head += 1;
	tokeniser->col += 1;
	return v;
}

static void hdw_newlinetoken(hdw_tokeniser * const tokeniser) {
	tokeniser->line++;
	tokeniser->col = 0;
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
	
	while (hdw_advancetoken(tokeniser) != '"' && !hdw_endtoken(tokeniser)) {
		if (hdw_peektoken(tokeniser) == '\n') {
			hdw_newlinetoken(tokeniser);
		}
	}
	
	if (hdw_endtoken(tokeniser)) {
		return true;
	}
	
	size_t end = (tokeniser->head) - 1;
	
	hdw_addtoken(tokeniser, HDW_STRING, hdw_strndup(&tokeniser->code[start], end - start));
	
	return false;
}

static bool hdw_numbertoken(hdw_tokeniser * const tokeniser) {
	/**
	 * This function will handle number tokens (Floating or Integer).
	 */
	
	bool is_integer = true;
	size_t start = (tokeniser->head) - 1;
	
	// handle more digits
	while (hdw_isdigit(hdw_peektoken(tokeniser))) {
		hdw_advancetoken(tokeniser);
	}
	
	// handle decimal point in number
	if (hdw_isradix(hdw_peektoken(tokeniser)) && hdw_isdigit(hdw_peektoken2(tokeniser))) {
		is_integer = false;
		hdw_advancetoken(tokeniser);
		
		while (hdw_isdigit(hdw_peektoken(tokeniser))) {
			hdw_advancetoken(tokeniser);
		}
	}
	
	size_t end = (tokeniser->head);
	
	// add token based on if its a float or integer
	char *s = hdw_strndup(&tokeniser->code[start], end - start);
	if (!s) {
		return true;
	}
	
	if (is_integer) {
		int64_t n = atoll(s);
		hdw_addinttoken(tokeniser, n);
	}
	else {
		double n = strtod(s, NULL);
		hdw_adddectoken(tokeniser, n);
	}
	
	free(s);
	
	return false;
}

static bool hdw_symboltoken(hdw_tokeniser * const tokeniser) {
	/**
	 * Handle a token that starts alpha character then runs until the end of an
	 * alphanumeric sequence
	 */
	
	const hdw_keywordmap keywords[] = {
		{"struct", HDW_STRUCT},
		{"class", HDW_CLASS},
		{"function", HDW_FUNCTION},
		{"if", HDW_IF},
		{"elseif", HDW_ELSEIF},
		{"else", HDW_ELSE},
		{"for", HDW_FOR},
		{"while", HDW_WHILE},
		{"return", HDW_RETURN},
		{"true", HDW_TRUE},
		{"false", HDW_FALSE},
		{"null", HDW_NULL},
	};
	
	size_t start = (tokeniser->head) - 1;
	
	while (hdw_isalphanumeric(hdw_peektoken(tokeniser))) {
		hdw_advancetoken(tokeniser);
	}
	
	size_t end = (tokeniser->head);
	
	char *s = hdw_strndup(&tokeniser->code[start], end - start);
	
	if (!s) {
		return true;
	}
	
	uint16_t kw = hdw_findkeyword(sizeof(keywords) / sizeof(hdw_keywordmap), keywords, s);
	
	if (!kw) {
		hdw_addtoken(tokeniser, HDW_SYMBOL, s);
	}
	else {
		hdw_addtoken(tokeniser, kw, NULL);
		free(s);
	}
	
	return false;
}

static void hdw_printtokens(hdw_tokenarray *tokens) {
	for (size_t i = 0; i < tokens->count; i++) {
		if (tokens->tokens[i].type == HDW_INTEGER) {
			printf("%.3d @ (line=%d, col=%d) = %d\n", tokens->tokens[i].type, tokens->tokens[i].line, tokens->tokens[i].col, tokens->tokens[i].int_value);
		}
		else if (tokens->tokens[i].type == HDW_NUMBER) {
			printf("%.3d @ (line=%d, col=%d) = %f\n", tokens->tokens[i].type, tokens->tokens[i].line, tokens->tokens[i].col, tokens->tokens[i].dec_value);
		}
		else {
			printf("%.3d @ (line=%d, col=%d) = %s\n", tokens->tokens[i].type, tokens->tokens[i].line, tokens->tokens[i].col, tokens->tokens[i].name ? tokens->tokens[i].name : "<NULL>");
		}
	}
}

#define SIMPL_TOKEN(TYPE, NAME) hdw_addtoken(&tokeniser, TYPE, NAME)
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
		.col = 0,
		.error = 0,
	};
	
	memset(tokens, 0, sizeof(hdw_tokenarray));
	
	while (tokeniser.head < tokeniser.len) {
		// NOTE: Taking advantage of the postfix ++ operator returning the
		// unicremented value first.
		char current = hdw_advancetoken(&tokeniser);
		
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
			else if (MATCH('*')) { // Handle multi-line comments as well
				current = hdw_advancetoken(&tokeniser);
				
				while (!((hdw_advancetoken(&tokeniser) == '*') && (hdw_advancetoken(&tokeniser) == '/')) && !hdw_endtoken(&tokeniser)) {
					if (hdw_peektoken(&tokeniser) == '\n') {
						hdw_newlinetoken(&tokeniser);
					}
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
		else if (hdw_isdigit(current)) {
			hdw_numbertoken(&tokeniser);
			// TODO: Proper error checking here
		}
		else if (hdw_isalpha(current)) {
			hdw_symboltoken(&tokeniser);
			// TODO: Proper error checking here
		}
		/* Misc */
		else if (current == ';') { SIMPL_TOKEN(HDW_SEMI, NULL); }
		else if (current == ',') { SIMPL_TOKEN(HDW_COMMA, NULL); }
		else if (current == '@') { SIMPL_TOKEN(HDW_AT, NULL); }
		else if (current == '#') { SIMPL_TOKEN(HDW_HASH, NULL); }
		else if (current == '^') { SIMPL_TOKEN(HDW_CARET, NULL); }
		else if (current == '~') { SIMPL_TOKEN(HDW_TILDE, NULL); }
		else if (current == '?') { SIMPL_TOKEN(HDW_QUERY, NULL); }
		else if (current == '`') { SIMPL_TOKEN(HDW_GRAVE, NULL); }
		else if (current == '.') { SIMPL_TOKEN(HDW_DOT, NULL); }
		else if (current == ' ' || current == '\t' || current == '\n' || current == '\r') { /* IGNORE */ }
		else {
			char *msg = (char *) malloc(256 * sizeof(char));
			if (msg) {
				snprintf(msg, 256, "Line %u, Column %u: Unrecogised tokeniser character '%c'.", tokeniser.line, tokeniser.col, current);
				hdw_puterror(script, msg);
				tokeniser.error += 1;
			}
		}
		
		if (current == '\n') { hdw_newlinetoken(&tokeniser); }
	}
	
	return tokeniser.error;
}

#undef MATCH
#undef SIMPL_TOKEN

// =============================================================================
// Parser
// =============================================================================

static hdw_treenode *hdw_treeAlloc(size_t size) {
	/**
	 * Allocate a tree node and subnodes of size children.
	 */
	
	hdw_treenode *tn = (hdw_treenode *) malloc(sizeof *tn);
	
	if (!tn) {
		return NULL;
	}
	
	*tn = (hdw_treenode) {
		.children = 0,
		.children_count = size,
		.type = 0,
		.as_integer = 0,
	};
	
	if (size > 0) {
		tn->children = (hdw_treenode *) malloc(sizeof *tn->children * size);
		
		if (!tn->children) {
			free(tn);
			return NULL;
		}
	}
	
	return tn;
}

static hdw_treenode *hdw_treeBinary(uint32_t type, hdw_treenode *left, hdw_treenode *right) {
	/**
	 * Allocate a tree node that is binary and set its type.
	 */
	
	hdw_treenode *tn = hdw_treeAlloc(2);
	
	if (!tn) {
		return NULL;
	}
	
	tn->type = type;
	tn->children[0] = *left;
	tn->children[1] = *right;
	
	// Free left and right since their memory is not needed anymore
	free(left);
	free(right);
	
	return tn;
}

static hdw_treenode *hdw_treeSingle(uint32_t type, hdw_treenode *node) {
	/**
	 * Allocate a tree node that is binary and set its type.
	 */
	
	hdw_treenode *tn = hdw_treeAlloc(1);
	
	if (!tn) {
		return NULL;
	}
	
	tn->type = type;
	tn->children[0] = *node;
	
	// Free left and right since their memory is not needed anymore
	free(node);
	
	return tn;
}

static hdw_treenode *hdw_treeFromToken(hdw_token token) {
	/**
	 * Allocate a tree node that is binary and set its type.
	 */
	
	hdw_treenode *tn = hdw_treeAlloc(0);
	
	if (!tn) {
		return NULL;
	}
	
	tn->type = token.type;
	tn->as_integer = token.int_value;
	
	return tn;
}

static void hdw_treefree(hdw_treenode *tree) {
	/**
	 * Free a tree node and all of its children if they are allocated.
	 */
	
	for (size_t i = 0; i < tree->children_count; i++) {
		hdw_treefree(&tree->children[i]);
	}
	
	if (tree->children) {
		free(tree->children);
	}
}

static void hdw_treenodeprint(hdw_treenode *node, int stack) {
	/**
	 * Print a tree node and all of its subtrees.
	 */
	
	if (!node) {
		return;
	}
	
	for (int i = 0; i < stack; i++) {
		printf("\t");
	}
	
	printf("(%d = <%x> -> ", node->type, node->as_string);
	
	bool sub = false;
	
	for (size_t i = 0; i < node->children_count; i++) {
		printf("\n");
		hdw_treenodeprint(&node->children[i], stack + 1);
		sub = true;
	}
	
	if (sub) {
		for (int i = 0; i < stack; i++) {
			printf("\t");
		}
	}
	
	printf(")\n", node->type, node->as_string);
}

#define HDW_CURRENT parser->tokens->tokens[parser->head]

static void hdw_parseError(hdw_parser * const restrict parser, char * restrict message) {
	printf("Parser error (Line %d, Column %d): %s.\n", HDW_CURRENT.line, HDW_CURRENT.col, message);
}

static hdw_treenode *hdw_Expression(hdw_parser * const restrict);

static hdw_treenode *hdw_ExpressionLevel6(hdw_parser * const restrict parser) {
	hdw_tokentype type = HDW_CURRENT.type;
	
	if (type == HDW_FALSE || type == HDW_TRUE || type == HDW_NULL || type == HDW_STRING || type == HDW_NUMBER || type == HDW_INTEGER) {
		hdw_treenode *a = hdw_treeFromToken(HDW_CURRENT);
		parser->head++;
		return a;
	}
	
	if (type == HDW_PARL) {
		parser->head++;
		hdw_treenode *a = hdw_Expression(parser);
		if (HDW_CURRENT.type != HDW_PARE) {
			hdw_treefree(a);
			hdw_parseError(parser, "Expected closing ')' but did not find it.");
			return NULL;
		}
		return hdw_treeSingle(HDW_EXPR, a);
	}
	
	hdw_parseError(parser, "Invalid expression.");
	return NULL;
}

static hdw_treenode *hdw_ExpressionLevel5(hdw_parser * const restrict parser) {
	if (HDW_CURRENT.type == HDW_NOT || HDW_CURRENT.type == HDW_MINUS) {
		// Get type
		hdw_tokentype type = HDW_CURRENT.type;
		
		// Push up head
		parser->head++;
		
		// Do the right expression
		hdw_treenode *node = hdw_ExpressionLevel5(parser);
		
		// Create the tree
		return hdw_treeSingle(type, node);
	}
	
	return hdw_ExpressionLevel6(parser);
}

static hdw_treenode *hdw_ExpressionLevel4(hdw_parser * const restrict parser) {
	hdw_treenode *tn = hdw_ExpressionLevel5(parser);
	
	while (HDW_CURRENT.type == HDW_ASTRESK || HDW_CURRENT.type == HDW_BACK) {
		// Get type
		hdw_tokentype type = HDW_CURRENT.type;
		
		// Push up head
		parser->head++;
		
		// Do the right expression
		hdw_treenode *right = hdw_ExpressionLevel5(parser);
		
		// Create the tree
		tn = hdw_treeBinary(type, tn, right);
	}
	
	return tn;
}

static hdw_treenode *hdw_ExpressionLevel3(hdw_parser * const restrict parser) {
	hdw_treenode *tn = hdw_ExpressionLevel4(parser);
	
	while (HDW_CURRENT.type == HDW_PLUS || HDW_CURRENT.type == HDW_MINUS) {
		// Get type
		hdw_tokentype type = HDW_CURRENT.type;
		
		// Push up head
		parser->head++;
		
		// Do the right expression
		hdw_treenode *right = hdw_ExpressionLevel4(parser);
		
		// Create the tree
		tn = hdw_treeBinary(type, tn, right);
	}
	
	return tn;
}

static hdw_treenode *hdw_ExpressionLevel2(hdw_parser * const restrict parser) {
	hdw_treenode *tn = hdw_ExpressionLevel3(parser);
	
	while (HDW_CURRENT.type == HDW_LT || HDW_CURRENT.type == HDW_GT || HDW_CURRENT.type == HDW_LTEQ || HDW_CURRENT.type == HDW_LTEQ) {
		// Get type
		hdw_tokentype type = HDW_CURRENT.type;
		
		// Push up head
		parser->head++;
		
		// Do the right expression
		hdw_treenode *right = hdw_ExpressionLevel3(parser);
		
		// Create the tree
		tn = hdw_treeBinary(type, tn, right);
	}
	
	return tn;
}

static hdw_treenode *hdw_ExpressionLevel1(hdw_parser * const restrict parser) {
	hdw_treenode *tn = hdw_ExpressionLevel2(parser);
	
	while (HDW_CURRENT.type == HDW_EQ || HDW_CURRENT.type == HDW_NOTEQ) {
		// Get type
		hdw_tokentype type = HDW_CURRENT.type;
		
		// Push up head
		parser->head++;
		
		// Do the right expression
		hdw_treenode *right = hdw_ExpressionLevel2(parser);
		
		// Create the tree
		tn = hdw_treeBinary(type, tn, right);
	}
	
	return tn;
}

static hdw_treenode *hdw_Expression(hdw_parser * const restrict parser) {
	return hdw_ExpressionLevel1(parser);
}

#undef HDW_CURRENT

int32_t hdw_parse(hdw_script * const restrict script, hdw_treenode ** const restrict tree, const hdw_tokenarray * const restrict tokens) {
	/**
	 * Parse a sequence of tokens into an abstract syntax tree.
	 */
	
	hdw_parser parser = {
		.root = *tree,
		.head = 0,
		.tokens = tokens,
	};
	
	*tree = hdw_Expression(&parser);
	
	return 0;
}

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
		
		if (script_temp) {
			hdw_destroy(script);
		}
		
		return -3;
	}
	
	//hdw_printtokens(&tokens);
	
	status = hdw_parse(script, &tree, &tokens);
	
	hdw_treenodeprint(tree, 0);
	
	hdw_freetokens(&tokens);
	
	if (status) {
		hdw_treefree(tree);
		
		if (script_temp) {
			hdw_destroy(script);
		}
		
		return HDW_ERR_PARSER;
	}
	
	hdw_treefree(tree);
	
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
