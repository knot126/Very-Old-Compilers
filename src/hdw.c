/**
 * Honeydew - Main File
 * ====================
 * 
 * Copyright (C) 2021 Decent Games
 * -------------------------------
 * 
 * NOTE: This is my first attempt at creating an interpreter, and it's not gone
 * exremely well. Hopefully you understand :)
 * 
 * This is the main file that implements the Honeydew interpreter. It is
 * organised, from top to bottom: 
 * 
 *   - Utilities: various tools that help with tasks needed throughout the \
 *     language implementation.
 *   - Instance Management: mangement of scripts
 *   - Tokeniser: The lexical analysis part of the interpreter
 *   - Parser: The part of the interpreter that creates the tree structures
 *     (the IR).
 *   - External Functions: functions that take care of running code strings and
 *     files.
 */

#include <stdbool.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "hdw.h"

//$combine-exclude

#include "util.c"
#include "instance.c"
#include "tokeniser.c"
#include "parser.c"
#include "interpreter.c"
#include "bytecode.c"
#include "error.c"
#include "exec.c"
#include "api.c"

//$end-combine-exclude
