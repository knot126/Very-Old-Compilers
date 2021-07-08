// =============================================================================
// Interpreter
// =============================================================================

static hdw_value *hdw_interpreterEvaluate(hdw_interpreter *interpreter, const hdw_treenode * const restrict tree);

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

static hdw_value *hdw_newNumberValue(double value) {
	hdw_value *obj = (hdw_value *) malloc(sizeof *obj);
	
	if (!obj) {
		return NULL;
	}
	
	obj->type = HDW_TYPE_NUMBER;
	obj->as_number = value;
	
	return obj;
}

static hdw_value *hdw_isTrue(hdw_treenode *input) {
	if (input->type == HDW_FALSE) return hdw_newValue(HDW_TYPE_BOOLEAN, 0);
	if (input->type == HDW_NULL) return hdw_newValue(HDW_TYPE_BOOLEAN, 0);
	if (input->type == HDW_STRING && input->as_string[0] == '\0') return hdw_newValue(HDW_TYPE_BOOLEAN, 0);
	if (input->type == HDW_INTEGER && input->as_integer == 0) return hdw_newValue(HDW_TYPE_BOOLEAN, 0);
	if (input->type == HDW_NUMBER && input->as_number == 0.0f) return hdw_newValue(HDW_TYPE_NUMBER, 0);
	return hdw_newValue(HDW_TYPE_BOOLEAN, 1);
}

static hdw_value *hdw_not(hdw_value *val) {
	val->as_boolean = !val->as_boolean;
	return val;
}

static hdw_value *hdw_opAdd(hdw_interpreter *interpreter, const hdw_treenode * const restrict tree) {
	hdw_value *left = hdw_interpreterEvaluate(interpreter, &tree->children[0]);
	hdw_value *right = hdw_interpreterEvaluate(interpreter, &tree->children[1]);
	hdw_value *res;
	
	if (left->type == HDW_TYPE_INTEGER && right->type == HDW_TYPE_INTEGER) {
		res = hdw_newValue(HDW_TYPE_INTEGER, left->as_integer + right->as_integer);
	}
	else if (left->type == HDW_TYPE_INTEGER && right->type == HDW_TYPE_NUMBER) {
		res = hdw_newNumberValue((double) left->as_integer + right->as_number);
	}
	else if (left->type == HDW_TYPE_NUMBER && right->type == HDW_TYPE_INTEGER) {
		res = hdw_newNumberValue(left->as_number + (double) right->as_integer);
	}
	else if (left->type == HDW_TYPE_NUMBER && right->type == HDW_TYPE_NUMBER) {
		res = hdw_newNumberValue(left->as_number + right->as_number);
	}
	else {
		res = hdw_nullValue();
	}
	
	free(left);
	free(right);
	
	return res;
}

static hdw_value *hdw_opSub(hdw_interpreter *interpreter, const hdw_treenode * const restrict tree) {
	hdw_value *left = hdw_interpreterEvaluate(interpreter, &tree->children[0]);
	hdw_value *right = hdw_interpreterEvaluate(interpreter, &tree->children[1]);
	hdw_value *res;
	
	if (left->type == HDW_TYPE_INTEGER && right->type == HDW_TYPE_INTEGER) {
		res = hdw_newValue(HDW_TYPE_INTEGER, left->as_integer - right->as_integer);
	}
	else if (left->type == HDW_TYPE_INTEGER && right->type == HDW_TYPE_NUMBER) {
		res = hdw_newNumberValue((double) left->as_integer - right->as_number);
	}
	else if (left->type == HDW_TYPE_NUMBER && right->type == HDW_TYPE_INTEGER) {
		res = hdw_newNumberValue(left->as_number - (double) right->as_integer);
	}
	else if (left->type == HDW_TYPE_NUMBER && right->type == HDW_TYPE_NUMBER) {
		res = hdw_newNumberValue(left->as_number - right->as_number);
	}
	else {
		res = hdw_nullValue();
	}
	
	free(left);
	free(right);
	
	return res;
}

static hdw_value *hdw_opMul(hdw_interpreter *interpreter, const hdw_treenode * const restrict tree) {
	hdw_value *left = hdw_interpreterEvaluate(interpreter, &tree->children[0]);
	hdw_value *right = hdw_interpreterEvaluate(interpreter, &tree->children[1]);
	hdw_value *res;
	
	if (left->type == HDW_TYPE_INTEGER && right->type == HDW_TYPE_INTEGER) {
		res = hdw_newValue(HDW_TYPE_INTEGER, left->as_integer * right->as_integer);
	}
	else if (left->type == HDW_TYPE_INTEGER && right->type == HDW_TYPE_NUMBER) {
		res = hdw_newNumberValue((double) left->as_integer * right->as_number);
	}
	else if (left->type == HDW_TYPE_NUMBER && right->type == HDW_TYPE_INTEGER) {
		res = hdw_newNumberValue(left->as_number * (double) right->as_integer);
	}
	else if (left->type == HDW_TYPE_NUMBER && right->type == HDW_TYPE_NUMBER) {
		res = hdw_newNumberValue(left->as_number * right->as_number);
	}
	else {
		res = hdw_nullValue();
	}
	
	free(left);
	free(right);
	
	return res;
}

static hdw_value *hdw_interpreterEvaluate(hdw_interpreter *interpreter, const hdw_treenode * const restrict tree) {
	hdw_value *value;
	
	// Literals
	if (tree->type == HDW_FALSE || tree->type == HDW_TRUE || tree->type == HDW_NULL || tree->type == HDW_STRING || tree->type == HDW_NUMBER || tree->type == HDW_INTEGER) {
		uint32_t type;
		
		// Remap types
		switch (tree->type) {
			case HDW_FALSE:
			case HDW_TRUE: {
				type = HDW_TYPE_BOOLEAN;
				break;
			}
			case HDW_NULL: {
				type = HDW_TYPE_NULL;
				break;
			}
			case HDW_STRING: {
				type = HDW_TYPE_STRING;
				break;
			}
			case HDW_NUMBER: {
				type = HDW_TYPE_NUMBER;
				break;
			}
			case HDW_INTEGER: {
				type = HDW_TYPE_INTEGER;
				break;
			}
		}
		
		value = hdw_newValue(type, tree->as_integer);
	}
	
	// Parenthesis - just call for the child's eval
	else if (tree->type == HDW_EXPR) {
		value = hdw_interpreterEvaluate(interpreter, &tree->children[0]);
	}
	
	// Urnary negation
	else if (tree->type == HDW_MINUS && tree->children_count == 1) {
		if (tree->children[0].type == HDW_INTEGER) {
			return hdw_newValue(HDW_TYPE_INTEGER, -tree->children[0].as_integer);
		}
		else if (tree->children[0].type == HDW_NUMBER) {
			double a = (-tree->children[0].as_number);
			return hdw_newValue(HDW_TYPE_NUMBER, *(int64_t*)&a);
		}
		else {
			hdw_interpreterError("Cannot negate something that isn't an integer or number.");
			return hdw_nullValue();
		}
	}
	
	// Urnary logical not
	else if (tree->type == HDW_NOT && tree->children_count == 1) {
		return hdw_not(hdw_isTrue(&tree->children[0]));
	}
	
	// Binary Add
	else if (tree->type == HDW_PLUS && tree->children_count == 2) {
		return hdw_opAdd(interpreter, tree);
	}
	
	// Binary Subtract
	else if (tree->type == HDW_MINUS && tree->children_count == 2) {
		return hdw_opSub(interpreter, tree);
	}
	
	// Binary Subtract
	else if (tree->type == HDW_ASTRESK && tree->children_count == 2) {
		return hdw_opMul(interpreter, tree);
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
