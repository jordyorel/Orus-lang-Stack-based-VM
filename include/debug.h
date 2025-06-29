#ifndef ORUS_DEBUH
#define ORUS_DEBUH

#include "chunk.h"

void disassembleChunk(Chunk* chunk, const char* name);
int disassembleInstruction(Chunk* chunk, int offset);

#endif