# Manual WebAssembly Build Instructions

Since the automated build requires emcc in PATH, here are the manual steps:

## 1. Ensure emcc is available

In your terminal, run:
```bash
# Activate Emscripten (adjust path if needed)
source ~/emsdk/emsdk_env.sh

# Verify it works
emcc --version
```

## 2. Generate embedded standard library

From the Orus project directory:
```bash
python3 tools/gen_stdlib.py std src/vm/builtin_stdlib.c include/builtin_stdlib.h
```

## 3. Create build directories

```bash
mkdir -p build/web
mkdir -p web
```

## 4. Compile all source files

```bash
# Compile each source file to object
emcc -I./include -O2 -c src/util/string_utils.c -o build/web/string_utils.o
emcc -I./include -O2 -c src/util/file_utils.c -o build/web/file_utils.o
emcc -I./include -O2 -c src/parser/parser.c -o build/web/parser.o
emcc -I./include -O2 -c src/vm/builtin_stdlib.c -o build/web/builtin_stdlib.o
emcc -I./include -O2 -c src/vm/modules.c -o build/web/modules.o
emcc -I./include -O2 -c src/vm/builtins.c -o build/web/builtins.o
emcc -I./include -O2 -c src/vm/memory.c -o build/web/memory.o
emcc -I./include -O2 -c src/vm/vm.c -o build/web/vm.o
emcc -I./include -O2 -c src/vm/debug.c -o build/web/debug.o
emcc -I./include -O2 -c src/vm/chunk.c -o build/web/chunk.o
emcc -I./include -O2 -c src/vm/bytecode_io.c -o build/web/bytecode_io.o
emcc -I./include -O2 -c src/vm/value.c -o build/web/value.o
emcc -I./include -O2 -c src/scanner/scanner.c -o build/web/scanner.o
emcc -I./include -O2 -c src/compiler/ast.c -o build/web/ast.o
emcc -I./include -O2 -c src/compiler/symtable.c -o build/web/symtable.o
emcc -I./include -O2 -c src/compiler/type.c -o build/web/type.o
emcc -I./include -O2 -c src/compiler/error.c -o build/web/error.o
emcc -I./include -O2 -c src/compiler/compiler.c -o build/web/compiler.o
emcc -I./include -O2 -c src/web/web_main.c -o build/web/web_main.o
```

## 5. Link to WebAssembly

```bash
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
```

## 6. Run the playground

```bash
cd web
python3 -m http.server 8000
```

Then open http://localhost:8000

## Expected output files

After successful build:
- `web/orus-web.js` - JavaScript wrapper
- `web/orus-web.wasm` - WebAssembly binary
- `web/index.html` - Playground interface (already created)
- `web/playground.js` - Playground JavaScript (already created)

## Troubleshooting

**emcc command not found:**
- Run `source ~/emsdk/emsdk_env.sh` first
- Or find your emsdk installation with `find /Users/hierat -name "emsdk_env.sh" 2>/dev/null`

**Compilation errors:**
- Ensure all include files are present
- Check that embedded stdlib was generated properly

**Runtime errors:**
- Check browser console for detailed error messages
- Verify WASM files loaded correctly