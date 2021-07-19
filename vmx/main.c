/**
 * Main file for the test of Dew VM.
 */

#include <stdio.h>

#define DEW_VMX_IMPLEMENTATION
#include "dew.h"

int main(int argc, char *argv[]) {
	dew_Chunk chunk;
	
	dew_chunk_init(&chunk);
	dew_chunk_write(&chunk, DEW_OP_CONST);
	dew_chunk_write(&chunk, dew_chunk_add_constant(&chunk, (dew_Value) {.asNumber = 5.2}));
	dew_chunk_write(&chunk, DEW_OP_RET);
	dew_chunk_dissassemble(&chunk, "main");
	dew_chunk_free(&chunk);
	
	return 0;
}
