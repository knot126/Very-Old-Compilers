// =============================================================================
// Interpreter
// =============================================================================

static void hdw_interpreterError(const char * const message) {
	printf("Interpreter error: %s.\n", message);
}

static hdw_value *hdw_nullValue(void) {
	hdw_value *value = (hdw_value *) malloc(sizeof *value);
	
	if (!value) {
		return NULL;
	}
	
	value->type = HDW_NULL;
	
	return value;
}

static hdw_value *hdw_newValue(uint32_t type, int64_t value) {
	hdw_value *obj = (hdw_value *) malloc(sizeof *obj);
	
	if (!obj) {
		return NULL;
	}
	
	obj->type = type;
	obj->as_integer = value;
	
	return obj;
}

static hdw_value *hdw_interpreterEvaluate(hdw_interpreter *interpreter, const hdw_treenode * const restrict tree) {
	hdw_value *value;
	
	// Literals
	if (tree->type == HDW_FALSE || tree->type == HDW_TRUE || tree->type == HDW_NULL || tree->type == HDW_STRING || tree->type == HDW_NUMBER || tree->type == HDW_INTEGER) {
		value = hdw_newValue(tree->type, tree->as_integer);
	}
	// Parenthesis - just call for the child's eval
	else if (tree->type == HDW_EXPR) {
		value = hdw_interpreterEvaluate(interpreter, &tree->children[0]);
	}
	// Urnary negation
	else if (tree->type == HDW_MINUS && tree->children_count == 1) {
		if (tree->children[0].type == HDW_INTEGER) {
			return hdw_newValue(HDW_INTEGER, -tree->children[0].as_integer);
		}
		else if (tree->children[0].type == HDW_NUMBER) {
			double a = (-tree->children[0].as_number);
			return hdw_newValue(HDW_NUMBER, *(int64_t*)&a);
		}
		else {
			hdw_interpreterError("Cannot negate something that isn't an integer or number.");
			return hdw_nullValue();
		}
	}
	// Not supported or invalid, return null.
	else {
		return hdw_nullValue();
	}
	
	return value;
}

int32_t hdw_interpret(const hdw_treenode * const restrict tree, hdw_value ** const restrict result) {
	hdw_interpreter interpreter;
	
	*result = hdw_interpreterEvaluate(&interpreter, tree);
	
	if (!*result) {
		return HDW_ERR_INTERPRETER;
	}
	
	return 0;
}
