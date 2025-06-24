#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <emscripten.h>

#include "../../include/chunk.h"
#include "../../include/common.h"
#include "../../include/compiler.h"
#include "../../include/debug.h"
#include "../../include/parser.h"
#include "../../include/vm.h"
#include "../../include/modules.h"
#include "../../include/error.h"
#include "../../include/string_utils.h"
#include "../../include/version.h"
#include "../../include/value.h"

extern VM vm;

// JavaScript callback functions (set from JS)
static char* (*js_input_callback)(const char* prompt) = NULL;
static void (*js_output_callback)(const char* text) = NULL;

// Web-specific implementations of I/O functions
char* web_input(const char* prompt) {
    if (js_input_callback) {
        return js_input_callback(prompt);
    }
    // Default: return empty string if no callback set
    char* result = malloc(1);
    result[0] = '\0';
    return result;
}

void web_output(const char* text) {
    if (js_output_callback) {
        js_output_callback(text);
    }
    // If no callback, output goes nowhere (silent)
}

// WebAssembly exported functions
EMSCRIPTEN_KEEPALIVE
int initWebVM() {
    // Initialize VM with minimal setup for web
    initVM();
    
    // Verify initialization worked
    if (vm.stack == NULL || vm.stackTop == NULL) {
        return -1;
    }
    
    return 0;
}

EMSCRIPTEN_KEEPALIVE
int runSource(const char* source) {
    if (!source) return INTERPRET_COMPILE_ERROR;
    
    // Clear any previous error state
    vm.lastError = NIL_VAL;
    
    // Interpret the source code
    InterpretResult result = interpret(source);
    
    return (int)result;
}

EMSCRIPTEN_KEEPALIVE
void freeWebVM() {
    freeVM();
}

EMSCRIPTEN_KEEPALIVE
const char* getVersion() {
    return ORUS_VERSION;
}

EMSCRIPTEN_KEEPALIVE
void setInputCallback(char* (*callback)(const char*)) {
    js_input_callback = callback;
}

EMSCRIPTEN_KEEPALIVE
void setOutputCallback(void (*callback)(const char*)) {
    js_output_callback = callback;
}

// Web-specific built-in function overrides
EMSCRIPTEN_KEEPALIVE
void registerWebBuiltins() {
    // Override input() function to use JavaScript callback
    // Override print() function to use JavaScript callback
    // This would need to be integrated with the native function system
}

// Get last error message for web display
EMSCRIPTEN_KEEPALIVE
const char* getLastError() {
    if (IS_NIL(vm.lastError)) {
        return NULL;
    }
    
    if (IS_ERROR(vm.lastError)) {
        ObjError* error = AS_ERROR(vm.lastError);
        return error->message->chars;
    }
    
    return "Unknown error";
}

// Clear the last error
EMSCRIPTEN_KEEPALIVE
void clearLastError() {
    vm.lastError = NIL_VAL;
}

// Check if VM is in valid state
EMSCRIPTEN_KEEPALIVE
bool isVMReady() {
    return vm.stack != NULL && vm.stackTop != NULL;
}

// Reset VM state for new execution
EMSCRIPTEN_KEEPALIVE
void resetVMState() {
    // Reset stack
    vm.stackTop = vm.stack;
    
    // Clear any errors
    vm.lastError = NIL_VAL;
    
    // Reset frame count
    vm.frameCount = 0;
    
    // Note: openUpvalues field doesn't exist in this VM implementation
}

// Get VM statistics for debugging
EMSCRIPTEN_KEEPALIVE
int getVMStackSize() {
    return (int)(vm.stackTop - vm.stack);
}

EMSCRIPTEN_KEEPALIVE
int getVMFrameCount() {
    return vm.frameCount;
}

EMSCRIPTEN_KEEPALIVE
int getVMModuleCount() {
    return vm.moduleCount;
}