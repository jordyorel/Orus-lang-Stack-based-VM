#!/bin/bash

# Simple Orus WebAssembly Build Script (for debugging)
# Run this from the project root directory

set -e  # Exit on any error

echo "Building simple Orus WebAssembly for debugging..."

# Generate embedded standard library first
echo "Generating embedded standard library..."
python3 tools/gen_stdlib.py std src/vm/builtin_stdlib.c include/builtin_stdlib.h

# Create build directory
mkdir -p build/web-simple
mkdir -p web

# Source files (excluding main.c and web directory)
SRC_FILES=(
    "src/util/string_utils.c"
    "src/util/file_utils.c"
    "src/parser/parser.c"
    "src/vm/builtin_stdlib.c"
    "src/vm/modules.c"
    "src/vm/builtins.c"
    "src/vm/memory.c"
    "src/vm/vm.c"
    "src/vm/debug.c"
    "src/vm/chunk.c"
    "src/vm/bytecode_io.c"
    "src/vm/value.c"
    "src/scanner/scanner.c"
    "src/compiler/ast.c"
    "src/compiler/symtable.c"
    "src/compiler/type.c"
    "src/compiler/error.c"
    "src/compiler/compiler.c"
    "src/web/web_main.c"
)

# Compile each source file to object file
echo "Compiling source files..."
for src in "${SRC_FILES[@]}"; do
    obj="build/web-simple/$(basename "$src" .c).o"
    echo "  Compiling $src -> $obj"
    emcc -I./include -O0 -g -c "$src" -o "$obj"
done

# Link all object files into WebAssembly (simple version)
echo "Linking simple WebAssembly module..."
emcc -I./include -O0 -g \
     -s WASM=1 \
     -s EXPORTED_FUNCTIONS='["_initWebVM", "_runSource", "_getVersion"]' \
     -s EXPORTED_RUNTIME_METHODS='["ccall", "cwrap"]' \
     -s ALLOW_MEMORY_GROWTH=1 \
     -s INITIAL_MEMORY=64MB \
     -s ASSERTIONS=1 \
     -s SAFE_HEAP=1 \
     -s NO_EXIT_RUNTIME=1 \
     -lm \
     -o web/orus-simple.js \
     build/web-simple/*.o

echo "Simple WebAssembly build complete!"
echo "Files generated:"
echo "  web/orus-simple.js"
echo "  web/orus-simple.wasm"
echo ""
echo "Test with: open web/test-simple.html"