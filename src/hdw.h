#pragma once

#include <stdbool.h>
#include <inttypes.h>

typedef struct hdw_script {
	int test;
} hdw_script;

typedef struct hdw_token {
	uint64_t type;
	char *name;
} hdw_token;

typedef struct hdw_tokenarray {
	hdw_token *tokens;
	size_t count;
} hdw_tokenarray;

hdw_script *hdw_create();
void hdw_destroy(hdw_script *c);

int hdw_exec(hdw_script * restrict script, const char * const code);
int hdw_dofile(const char * const path);
void hdw_bulitin_prompt(void);
