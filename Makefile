CC=gcc
EMCC=emcc
all: debug
CFLAGS=-I./include -Wall -g
SRC=$(shell find src -name '*.c' | grep -v 'src/web/')
WEB_SRC=$(shell find src -name '*.c' | grep -v main.c | grep -v 'src/web/') src/web/web_main.c
STDLIBC=src/vm/builtin_stdlib.c
STDLIBH=include/builtin_stdlib.h
$(STDLIBC) $(STDLIBH): tools/gen_stdlib.py $(wildcard std/*.orus)
	python3 tools/gen_stdlib.py std $(STDLIBC) $(STDLIBH)
OBJ=$(patsubst src/%.c, build/debug/orus/%.o, $(SRC))
WEB_OBJ=$(patsubst src/%.c, build/web/%.o, $(WEB_SRC))
TARGET=orusc
RELEASE_TARGET=build/release/clox
WEB_TARGET=web/orus-web

debug: $(OBJ)
	@mkdir -p $(dir $(RELEASE_TARGET))
	$(CC) -o $(RELEASE_TARGET) $^ -lm
	cp $(RELEASE_TARGET) $(TARGET)

orusc: debug

# Rule to build the final binary
$(RELEASE_TARGET): $(OBJ)
	@mkdir -p $(dir $@)
	$(CC) -o $@ $^ -lm

# Rule to compile .c files into .o files in debug directory
build/debug/orus/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# WebAssembly build target
WASM_FLAGS=-I./include -O1 -s WASM=1 \
           -s EXPORTED_FUNCTIONS='["_initWebVM", "_runSource", "_freeWebVM", "_getVersion", "_getLastError", "_clearLastError", "_isVMReady", "_resetVMState", "_getVMStackSize", "_getVMFrameCount", "_getVMModuleCount", "_malloc", "_free"]' \
           -s EXPORTED_RUNTIME_METHODS='["ccall", "cwrap", "lengthBytesUTF8", "stringToUTF8"]' \
           -s ALLOW_MEMORY_GROWTH=1 \
           -s STACK_SIZE=1MB \
           -s INITIAL_MEMORY=32MB \
           -s MAXIMUM_MEMORY=256MB \
           -s MODULARIZE=1 \
           -s EXPORT_NAME='OrusModule' \
           -s ASSERTIONS=1 \
           -s SAFE_HEAP=1 \
           --pre-js src/web/pre.js \
           -lm

web: check-emcc $(STDLIBC) $(STDLIBH) $(WEB_OBJ)
	@mkdir -p web
	$(EMCC) $(WASM_FLAGS) -o $(WEB_TARGET).js $(WEB_OBJ)

check-emcc:
	@command -v emcc >/dev/null 2>&1 || (echo "Error: emcc not found in PATH. Please ensure Emscripten is activated:" && \
	 echo "  source ~/emsdk/emsdk_env.sh" && \
	 echo "Or install Emscripten if not installed:" && \
	 echo "  cd ~ && git clone https://github.com/emscripten-core/emsdk.git" && \
	 echo "  cd emsdk && ./emsdk install latest && ./emsdk activate latest" && \
	 echo "  source ./emsdk_env.sh" && \
	 exit 1)

# Rule to compile .c files for WebAssembly
build/web/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(EMCC) -I./include -O2 -c $< -o $@

# Clean rule to remove all generated files
clean:
	rm -rf build $(TARGET) web/orus-web.js web/orus-web.wasm 
