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

static void hdw_printValue(hdw_value *value) {
	if (!value) {
		printf("SystemError\n");
	}
	
	if (value->type == HDW_FALSE) {
		printf("false\n");
	}
	else if (value->type == HDW_TRUE) {
		printf("true\n");
	}
	else if (value->type == HDW_NULL) {
		printf("null\n");
	}
	else if (value->type == HDW_STRING) {
		printf("'%s'\n", value->as_string);
	}
	else if (value->type == HDW_NUMBER) {
		printf("%f\n", value->as_number);
	}
	else if (value->type == HDW_INTEGER) {
		printf("%lld\n", value->as_integer);
	}
	else {
		printf("<Object: type %d at 0x%.16X>", value->type, value);
	}
}
