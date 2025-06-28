/**
 * @file modules.c
 * @brief Module loading and compilation.
 */
#include "../../include/modules.h"
#include "../../include/file_utils.h"
#include "../../include/parser.h"
#include "../../include/compiler.h"
#include "../../include/vm.h"
#include "../../include/builtin_stdlib.h"
#include "../../include/bytecode_io.h"
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <limits.h>
#include <sys/stat.h>

static Module module_cache[UINT8_COUNT];
static uint8_t module_cache_count = 0;
static const char* loading_stack[UINT8_COUNT];
static uint8_t loading_stack_count = 0;

bool traceImports = false;

const char* moduleError = NULL;
static char module_error_buffer[256];

extern VM vm;

static char* normalize_module_path(const char* path) {
    size_t len = strlen(path);
    bool hasSlash = strchr(path, '/') != NULL;
    bool hasExt = len > 5 && strcmp(path + len - 5, ".orus") == 0;

    // Convert dot-separated paths to slashes when no slashes present
    if (!hasSlash && strchr(path, '.')) {
        char buf[256];
        size_t pos = 0;
        for (size_t i = 0; i < len && pos < sizeof(buf) - 6; i++) {
            buf[pos++] = (path[i] == '.') ? '/' : path[i];
        }
        if (!hasExt && pos < sizeof(buf) - 5) {
            memcpy(buf + pos, ".orus", 5);
            pos += 5;
        }
        buf[pos] = '\0';
        return strdup(buf);
    }

    if (!hasExt) {
        char* result = malloc(len + 6);
        memcpy(result, path, len);
        memcpy(result + len, ".orus", 5);
        result[len + 5] = '\0';
        return result;
    }

    return strdup(path);
}

static char* cache_path_for(const char* module_path) {
    if (!vm.cachePath) return NULL;
    const char* base = strrchr(module_path, '/');
    base = base ? base + 1 : module_path;
    char buf[512];
    snprintf(buf, sizeof(buf), "%s/%s.obc", vm.cachePath, base);
    return strdup(buf);
}

/**
 * Read a module's source code from disk.
 *
 * @param resolved_path Filesystem path to module.
 * @return              Newly allocated buffer with module source or NULL.
 */
char* load_module_source(const char* resolved_path) {
    return readFileSilent(resolved_path);
}

char* load_module_with_fallback(const char* path, char** disk_path, long* mtime,
                                bool* from_embedded) {
    if (disk_path) *disk_path = NULL;
    if (mtime) *mtime = 0;
    if (from_embedded) *from_embedded = false;

    char* source = load_module_source(path);
    if (source) {
        if (disk_path) *disk_path = strdup(path);
        if (mtime) {
            struct stat st;
            if (stat(path, &st) == 0) *mtime = st.st_mtime;
        }
        return source;
    }
    char full[256];
    const char* base = vm.stdPath ? vm.stdPath : "std";
    const char* sub = path;
    if (strncmp(path, "std/", 4) == 0) sub = path + 4;
    snprintf(full, sizeof(full), "%s/%s", base, sub);
    source = load_module_source(full);
    if (source) {
        if (disk_path) *disk_path = strdup(full);
        if (mtime) {
            struct stat st;
            if (stat(full, &st) == 0) *mtime = st.st_mtime;
        }
        return source;
    }
    const char* embedded = getEmbeddedModule(path);
    if (embedded) {
        if (from_embedded) *from_embedded = true;
        return strdup(embedded);
    }
    return NULL;
}

/**
 * Parse the source code of a module into an AST.
 *
 * @param source_code Source text of the module.
 * @param module_name Name used for error reporting.
 * @return            AST root or NULL on failure.
 */
ASTNode* parse_module_source(const char* source_code, const char* module_name) {
    ASTNode* ast;
    if (!parse(source_code, module_name, &ast)) {
        return NULL;
    }
    return ast;
}

/**
 * Compile an AST into bytecode for a module.
 *
 * @param ast         Parsed AST of the module.
 * @param module_name Module identifier.
 * @return            Newly allocated chunk or NULL on error.
 */
Chunk* compile_module_ast(ASTNode* ast, const char* module_name) {
    Chunk* chunk = malloc(sizeof(Chunk));
    if (!chunk) return NULL;
    initChunk(chunk);
    Compiler compiler;
    initCompiler(&compiler, chunk, module_name, NULL);
    if (!compile(ast, &compiler, false)) {
        freeChunk(chunk);
        free(chunk);
        return NULL;
    }
    return chunk;
}

/**
 * Add a compiled module to the VM's cache.
 *
 * @param module Module descriptor.
 * @return       True on success.
 */
bool register_module(Module* module) {
    if (module_cache_count == UINT8_MAX) return false;
    module_cache[module_cache_count++] = *module;
    return true;
}

/**
 * Retrieve a loaded module by name.
 *
 * @param name Module identifier.
 * @return     Pointer to cached module or NULL.
 */
Module* get_module(const char* name) {
    for (int i = 0; i < module_cache_count; i++) {
        if (strcmp(module_cache[i].module_name, name) == 0) {
            return &module_cache[i];
        }
    }
    return NULL;
}

/**
 * Look up an exported symbol within a module.
 *
 * @param module Module to search.
 * @param name   Export name.
 * @return       Pointer to export entry or NULL.
 */
Export* get_export(Module* module, const char* name) {
    for (int i = 0; i < module->export_count; i++) {
        if (strcmp(module->exports[i].name, name) == 0) {
            return &module->exports[i];
        }
    }
    return NULL;
}

/**
 * Load, parse and compile a module without executing it.
 *
 * @param path File path or module name.
 * @return     Interpretation result code.
 */
InterpretResult compile_module_only(const char* path) {
    char* normPath = normalize_module_path(path);
    moduleError = NULL;
    if (traceImports) fprintf(stderr, "[import] loading %s\n", normPath);
    for (int i = 0; i < loading_stack_count; i++) {
        if (strcmp(loading_stack[i], normPath) == 0) {
            snprintf(module_error_buffer, sizeof(module_error_buffer),
                     "Import cycle detected for module `%s`", normPath);
            moduleError = module_error_buffer;
            free(normPath);
            return INTERPRET_COMPILE_ERROR;
        }
    }

    if (loading_stack_count < UINT8_MAX)
        loading_stack[loading_stack_count++] = normPath;

    if (get_module(normPath)) {
        loading_stack_count--;
        free(normPath);
        return INTERPRET_OK;
    }

    char* diskPath = NULL;
    long mtime = 0;
    bool fromEmbedded = false;
    char* source = load_module_with_fallback(normPath, &diskPath, &mtime, &fromEmbedded);
    if (!source) {
        snprintf(module_error_buffer, sizeof(module_error_buffer),
                 "Module `%s` not found", normPath);
        moduleError = module_error_buffer;
        loading_stack_count--;
        return INTERPRET_RUNTIME_ERROR;
    }

    int startGlobals = vm.variableCount;
    char* cacheFile = cache_path_for(normPath);
    Chunk* chunk = NULL;
    if (cacheFile) {
        long cached_mtime;
        chunk = readChunkFromFile(cacheFile, &cached_mtime);
        if (chunk && cached_mtime != mtime) { freeChunk(chunk); free(chunk); chunk = NULL; }
    }
    ASTNode* ast = NULL;
    if (!chunk) {
        ast = parse_module_source(source, normPath);
        if (!ast) {
            free(source);
            loading_stack_count--;
            if (cacheFile) free(cacheFile);
            return INTERPRET_COMPILE_ERROR;
        }

        chunk = compile_module_ast(ast, normPath);
        if (!chunk) {
            free(source);
            loading_stack_count--;
            if (cacheFile) free(cacheFile);
            return INTERPRET_COMPILE_ERROR;
        }
        if (cacheFile) writeChunkToFile(chunk, cacheFile, mtime);
    }

    Module mod;
    mod.module_name = strdup(normPath);
    const char* base = strrchr(normPath, '/');
    base = base ? base + 1 : normPath;
    size_t len = strlen(base);
    if (len > 5 && strcmp(base + len - 5, ".orus") == 0) len -= 5;
    mod.name = (char*)malloc(len + 1);
    memcpy(mod.name, base, len);
    mod.name[len] = '\0';
    mod.bytecode = chunk;
    mod.export_count = 0;
    mod.executed = false;
    mod.disk_path = diskPath;
    mod.mtime = mtime;
    mod.from_embedded = fromEmbedded;

    for (int i = startGlobals; i < vm.variableCount && mod.export_count < UINT8_MAX; i++) {
        Export ex;
        ex.name = vm.variableNames[i].name ? vm.variableNames[i].name->chars : NULL;
        if (ex.name && vm.publicGlobals[i]) {
            ex.name = strdup(ex.name);
            ex.value = vm.globals[i];
            ex.index = i;
            mod.exports[mod.export_count++] = ex;
        }
    }

    register_module(&mod);
    loading_stack_count--;
    free(source);
    if (cacheFile) free(cacheFile);
    free(normPath);
    return INTERPRET_OK;
}
