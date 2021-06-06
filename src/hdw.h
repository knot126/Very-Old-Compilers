#pragma once

#include <stdbool.h>
#include <inttypes.h>
#include <stdlib.h>

typedef size_t hdw_size_t;

// Tokeniser
enum {
	HDW_UNKNOWN = 0,
	
	HDW_SYMBOL,  // ex. 'myFunction', 'somevar', 'i'
	HDW_STRING,  // ex. '"string"', '"Hello world!\n"'
	HDW_NUMBER,  // ex. .0, 6.0, 11.5320
	HDW_INTEGER, // ex. 0, 206, -20
	
	HDW_PARL, // (
	HDW_PARE, // )
	
	HDW_CURLYL, // {
	HDW_CURLYE, // }
	
	HDW_BRAKL, // [
	HDW_BRAKE, // ]
	
	HDW_COMMA, // ,
	HDW_DOT,   // .
	HDW_RADIX, // '.' or ','
	HDW_PLUS,  // +
	HDW_MINUS, // -
	HDW_ASTRESK, // *
	HDW_BACK,  // /
	HDW_MOD,   // %
	HDW_FORWARD, // '\'
	HDW_AMP,   // &
	HDW_SEMI,  // ;
	HDW_AT,    // @
	HDW_HASH,  // #
	HDW_CARET, // ^
	HDW_TILDE, // ~
	HDW_GRAVE, // `
	HDW_BAR,   // |
	HDW_SET,   // =
	HDW_QUERY, // ?
	
	HDW_NOT,   // !
	HDW_EQ,    // ==
	HDW_NOTEQ, // !=
	HDW_LT,    // >
	HDW_GT,    // <
	HDW_LTEQ,  // <=
	HDW_GTEQ,  // >=
	HDW_AND,   // &&
	HDW_OR,    // ||
	
	HDW_STRUCT,   // 'struct'
	HDW_CLASS,    // 'class'
	HDW_FUNCTION, // 'function'
	HDW_IF,       // 'if'
	HDW_ELSEIF,   // 'elseif'
	HDW_ELSE,     // 'else'
	HDW_FOR,      // 'for'
	HDW_WHILE,    // 'while'
	HDW_RETURN,   // 'return'
	HDW_TRUE,     // 'true'
	HDW_FALSE,    // 'false'
	HDW_NULL,     // 'null'
	
	HDW_FEND,     // eof
};

typedef struct hdw_token {
	char *name;
	uint32_t line;
	uint16_t col;
	uint16_t type;
} hdw_token;

typedef struct hdw_tokenarray {
	hdw_token *tokens;
	size_t count;
} hdw_tokenarray;

// Script State
typedef struct hdw_script {
	// Error handling
	const char *errmsg;  // NULL if there is no error, pointer to message if there is
	
	// 
} hdw_script;

// Instances
hdw_script *hdw_create();
void hdw_destroy(hdw_script *c);

// Low level
int hdw_exec(hdw_script * restrict script, const char * const code);

// Errors
int32_t hdw_error(hdw_script * const restrict script, const char ** const restrict what);

// High level
int hdw_dofile(const char * const path);
void hdw_bulitin_prompt(void);
