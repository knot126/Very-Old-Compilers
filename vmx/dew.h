/**
 * Dew Virtual Mechine
 * ===================
 * 
 * Yet another attempt at writing a Dew VM.
 */

/**
 * Header
 * =============================================================================
 */

#ifndef DEW_VMX_INCLUDED
#define DEW_VMX_INCLUDED

#include <stdbool.h>
#include <stddef.h>
#include <inttypes.h>
#include <stdlib.h>

typedef double dew_Number;
typedef int64_t dew_Integer;
typedef bool dew_Boolean;
typedef const char * dew_String;

typedef enum {
	DEW_OP_NOP = 0,
	DEW_OP_RET,
} dew_OpCode;

typedef union {
	dew_Number asNumber;
	dew_Integer asInteger;
	dew_Boolean asBoolean;
	dew_String asString;
} dew_Value;

typedef struct {
	dew_Value *data;
	size_t count;
	size_t alloc;
} dew_Soup;

typedef struct {
	uint8_t *data;
	size_t count;
	size_t alloc;
	
	dew_Soup soup;
} dew_Chunk;

void dew_chunk_init(dew_Chunk *chunk);
void dew_chunk_write(dew_Chunk *chunk, uint8_t byte);
void dew_chunk_dissassemble(dew_Chunk *chunk, const char * const title);
void dew_chunk_free(dew_Chunk *chunk);

void dew_soup_init(dew_Soup *soup);
void dew_soup_write(dew_Soup *soup, dew_Value value);
void dew_soup_free(dew_Soup *soup);

#endif

/**
 * Implementation
 * =============================================================================
 */

#ifdef DEW_VMX_IMPLEMENTATION
#undef DEW_VMX_IMPLEMENTATION

/**
 * Chunks
 */

static void *dew_memory(void *block, const size_t size) {
	/**
	 * Allocate memory, with similar rules to realloc.
	 */
	
	void *p = realloc(block, size);
	
	if (size > 0 && !p) {
		printf("dew_alloc: out of memory or bizzare error, abort.\n");
		abort();
	}
	
	return p;
}

void dew_chunk_init(dew_Chunk *chunk) {
	/**
	 * Initialise a chunk.
	 */
	
	chunk->alloc = 8;
	chunk->data = dew_memory(NULL, chunk->alloc);
	chunk->count = 0;
}

void dew_chunk_write(dew_Chunk *chunk, uint8_t byte) {
	/**
	 * Write a byte to the chunk of code.
	 */
	
	if (chunk->count >= chunk->alloc) {
		chunk->alloc *= 2;
		chunk->data = dew_memory(chunk->data, chunk->alloc);
	}
	
	chunk->data[chunk->count] = byte;
	chunk->count++;
}

static size_t dew_chunk_diss_instr(dew_Chunk *chunk, size_t where) {
	/**
	 * Dissassemble an instruction.
	 */
	
	uint8_t opcode = chunk->data[where];
	
	switch (opcode) {
		case DEW_OP_NOP: {
			printf("nop\n");
			return 1;
		}
		case DEW_OP_RET: {
			printf("ret\n");
			return 1;
		}
		default: {
			printf("??? %.2X\n", opcode);
			return 1;
		}
	}
}

void dew_chunk_dissassemble(dew_Chunk *chunk, const char * const title) {
	/**
	 * Dissassemble a chunk.
	 */
	
	printf("## %s ##\n", title);
	
	for (size_t i = 0; i < chunk->count;) {
		i += dew_chunk_diss_instr(chunk, i);
	}
}

void dew_chunk_free(dew_Chunk *chunk) {
	/**
	 * Free a chunk.
	 */
	
	chunk->data = dew_memory(chunk->data, 0);
}

/**
 * Constant pools (soups)
 */

void dew_soup_init(dew_Soup *soup) {
	/**
	 * Initialise a soup.
	 */
	
	soup->alloc = 8;
	soup->data = dew_memory(NULL, sizeof *soup->data * soup->alloc);
	soup->count = 0;
}

void dew_soup_write(dew_Soup *soup, dew_Value value) {
	/**
	 * Write a value to the soup of values.
	 */
	
	if (soup->count >= soup->alloc) {
		soup->alloc *= 2;
		soup->data = dew_memory(soup->data, sizeof *soup->data * soup->alloc);
	}
	
	soup->data[soup->count] = value;
	soup->count++;
}

void dew_soup_free(dew_Soup *soup) {
	/**
	 * Free a value soup.
	 */
	
	soup->data = dew_memory(soup->data, 0);
}

#endif
