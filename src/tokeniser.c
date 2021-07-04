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

static bool hdw_matchtoken(hdw_tokeniser * const tokeniser, char what) {
	/**
	 * If the tokens matches the next unconsumed token, this function will 
	 * consume a token and return true; otherwise, it will just return false.
	 */
	
	if (tokeniser->code[tokeniser->head] == what) {
		tokeniser->head++;
		return true;
	}
	else {
		return false;
	}
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
		
		// Built-in types
		// TODO: Cleaner implmenetation of these...
		{"int", HDW_KWINT},
		{"number", HDW_KWNUM},
		{"string", HDW_KWSTR},
		{"bool", HDW_KWBOL},
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
#define MATCH(CHAR) hdw_matchtoken(&tokeniser, CHAR)

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
		else if (current == ':') { SIMPL_TOKEN(HDW_COLON, NULL); }
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
