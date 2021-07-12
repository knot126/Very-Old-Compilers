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

static hdw_treenode *hdw_treeAppend(hdw_treenode *dest, hdw_treenode *src) {
	/**
	 * Append to the end of a list-type node.
	 */
	
	dest->children = (hdw_treenode *) realloc(sizeof *dest->children * ++dest->children_count);
	
	if (!dest->children) {
		return NULL;
	}
	
	dest->children[dest->children_count - 1] = *src;
	
	free(src);
	
	return dest;
}

static hdw_treenode *hdw_treeTrinary(uint32_t type, hdw_treenode *a, hdw_treenode *b, hdw_treenode *c) {
	/**
	 * Allocate a tree node that is trinary and set its type.
	 */
	
	hdw_treenode *tn = hdw_treeAlloc(3);
	
	if (!tn) {
		return NULL;
	}
	
	tn->type = type;
	tn->children[0] = *a;
	tn->children[1] = *b;
	tn->children[2] = *c;
	
	// Free left and right since their memory is not needed anymore
	free(a);
	free(b);
	free(c);
	
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
	
	if (!tree) {
		return;
	}
	
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
#define HDW_GETTOKEN(OFFSET) parser->tokens->tokens[parser->head + OFFSET]

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
	
	while (HDW_CURRENT.type == HDW_LT || HDW_CURRENT.type == HDW_GT || HDW_CURRENT.type == HDW_LTEQ || HDW_CURRENT.type == HDW_GTEQ) {
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

static hdw_treenode *hdw_ExpressionLevel0(hdw_parser * const restrict parser) {
	hdw_treenode *tn = hdw_ExpressionLevel1(parser);
	
	while (HDW_CURRENT.type == HDW_QUERY) {
		parser->head++;
		
		hdw_treenode *b = hdw_ExpressionLevel0(parser);
		
		if (HDW_CURRENT.type != HDW_COLON) {
			hdw_parseError(parser, "Expected matching ':' for '?' in ternary operator.");
			return NULL;
		}
		
		parser->head++;
		
		hdw_treenode *c = hdw_ExpressionLevel0(parser);
		
		tn = hdw_treeTrinary(HDW_TERNARY, tn, b, c);
	}
	
	return tn;
}

static hdw_treenode *hdw_Expression(hdw_parser * const restrict parser) {
	hdw_treenode *tn = hdw_ExpressionLevel0(parser);
	
	while (HDW_CURRENT.type == HDW_COMMA) {
		parser->head++;
		
		hdw_treenode *right = hdw_Expression(parser);
		
		tn = hdw_treeBinary(HDW_EXPRGRP, tn, right);
	}
	
	return tn;
}

static hdw_treenode *hdw_Statement(hdw_parser * const restrict parser) {
	hdw_treenode *tn = hdw_Expression(parser);
	
	if (HDW_CURRENT.type == HDW_PARL) {
		hdw_treenode *next = hdw_FunctionCall();
	}
}

static hdw_treenode *hdw_Program(hdw_parser * const restrict parser) {
	hdw_treenode *tn = hdw_treeAlloc(0);
	
	for (size_t i = 0; i < parser->tokens->count; i++) {
		hdw_treenode *next = hdw_Statement(parser);
		
		hdw_treeAppend(tn, next);
	}
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
	
	*tree = hdw_Program(&parser);
	
	if (!tree) {
		return HDW_ERR_PARSER;
	}
	
	return 0;
}
