#!/bin/bash

# Orus WebAssembly Build Script
# Run this from the project root directory

set -e  # Exit on any error

echo "Building Orus WebAssembly..."

# Generate embedded standard library first
echo "Generating embedded standard library..."
python3 tools/gen_stdlib.py std src/vm/builtin_stdlib.c include/builtin_stdlib.h

# Create build directory
mkdir -p build/web
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
    obj="build/web/$(basename "$src" .c).o"
    echo "  Compiling $src -> $obj"
    emcc -I./include -O2 -c "$src" -o "$obj"
done

# Link all object files into WebAssembly
echo "Linking WebAssembly module..."
emcc -I./include -O2 -s WASM=1 \
     -s EXPORTED_FUNCTIONS='["_initWebVM", "_runSource", "_freeWebVM", "_getVersion", "_getLastError", "_clearLastError", "_isVMReady", "_resetVMState", "_getVMStackSize", "_getVMFrameCount", "_getVMModuleCount"]' \
     -s EXTRA_EXPORTED_RUNTIME_METHODS='["ccall", "cwrap", "lengthBytesUTF8", "stringToUTF8", "_malloc", "_free"]' \
     -s ALLOW_MEMORY_GROWTH=1 \
     -s STACK_SIZE=2MB \
     -s INITIAL_MEMORY=16MB \
     -s MODULARIZE=1 \
     -s EXPORT_NAME='OrusModule' \
     --pre-js src/web/pre.js \
     -lm \
     -o web/orus-web.js \
     build/web/*.o

echo "WebAssembly build complete!"
echo "Files generated:"
echo "  web/orus-web.js"
echo "  web/orus-web.wasm"
echo ""
echo "To run the playground:"
echo "  cd web"
echo "  python3 -m http.server 8000"
echo "  Open http://localhost:8000"