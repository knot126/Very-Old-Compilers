/**
 * Honeydew - Main File
 * ====================
 * 
 * Copyright (C) 2021 Decent Games
 */

#pragma once

#include <stdbool.h>
#include <inttypes.h>
#include <stdlib.h>

typedef size_t hdw_size_t; // Unused for now
typedef int32_t hdw_int_t; // Unused for now

// =============================================================================
// Tokeniser
// =============================================================================

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
	
	HDW_COMMENT, // "//"
	
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
	
	HDW_EXPR,     // '(' expression ')'
	HDW_EXPRGRP,  // Group of expressions
	HDW_FEND,     // eof
};

typedef uint16_t hdw_tokentype;
typedef uint16_t hdw_colcount;
typedef uint32_t hdw_linecount;

typedef struct hdw_token {
	union {
		const char *name;   // Text of the token
		double dec_value;
		int64_t int_value;
	};
	hdw_linecount line;      // The line the token is on
	hdw_colcount col;       // The column of the token
	hdw_tokentype type;      // The type the token is
} hdw_token;

typedef struct hdw_tokenarray {
	hdw_token *tokens;  // Pointer to the tokens
	size_t count;       // Number of tokens
} hdw_tokenarray;

typedef struct hdw_tokeniser {
	hdw_tokenarray *tokens;   // Array of tokens
	const char * const code;  // Pointer to the code
	size_t len;               // Length
	size_t head;              // The head position
	size_t line;              // The current line number
	size_t col;               // The current column number
	int32_t error;            // Number of errors in session
} hdw_tokeniser;

typedef struct hdw_keywordmap {
	const char * const key;  // The string of the type
	uint16_t value;          // The correspoding value of the type
} hdw_keywordmap;

// =============================================================================
// Parser
// =============================================================================

enum {
	HDW_DEFAULT = 0,
	HDW_ROOT = 1,
	HDW_VARIABLE = 5,
};

typedef struct hdw_treenode {
	struct hdw_treenode *children;
	union {
		double as_number;
		int64_t as_integer;
		char *as_string;
	};
	uint32_t children_count;
	uint32_t type;
} hdw_treenode;

typedef struct hdw_parser {
	hdw_treenode *root;
	const hdw_tokenarray * const tokens;
	size_t head;
} hdw_parser;

// =============================================================================
// Errors
// =============================================================================

enum hdw_retcode {
	HDW_ERR_OKAY = 0,
	HDW_ERR_ENV = -1,
	HDW_ERR_PRETOKENISER = -2,
	HDW_ERR_TOKENISER = -3,
	HDW_ERR_PARSER = -4,
};

typedef struct hdw_error {
	const char *message; // the message assocaited with the error
} hdw_error;

typedef struct hdw_errorarray {
	hdw_error *content;  // The list of error structures
	size_t count;        // The number of errors
} hdw_errorarray;

// =============================================================================
// Script State
// =============================================================================

typedef struct hdw_script {
	// Error handling
	hdw_errorarray errors;  // NULL if there is no error, pointer to message if there is
} hdw_script;

// Instances
// =============================================================================
hdw_script *hdw_create(void);
void hdw_destroy(hdw_script *c);

// Low level
// =============================================================================
int32_t hdw_tokenise(hdw_script * const restrict script, hdw_tokenarray *tokens, const char * const code);
int32_t hdw_exec(hdw_script * restrict script, const char * const code);
int32_t hdw_crexec(hdw_script ** restrict script, const char * const code);

// Errors
// =============================================================================
void hdw_puterror(hdw_script * const restrict script, const char * const restrict message);
void hdw_printerror(const hdw_script * const restrict script);
void hdw_reseterror(hdw_script * const restrict script);
int32_t hdw_haserror(hdw_script * const restrict script);

// High level
// =============================================================================
int hdw_dofile(const char * const path);
void hdw_bulitin_prompt(void);
