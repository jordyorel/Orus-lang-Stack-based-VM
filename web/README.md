# Orus WebAssembly Playground

This directory contains the WebAssembly build and web playground for the Orus programming language.

## Prerequisites

To build the WebAssembly version, you need:

1. **Emscripten SDK (emsdk)**
   ```bash
   # Install emsdk
   git clone https://github.com/emscripten-core/emsdk.git
   cd emsdk
   ./emsdk install latest
   ./emsdk activate latest
   source ./emsdk_env.sh
   ```

2. **Python 3** (for generating embedded standard library)

## Building

From the project root directory:

```bash
# Build the WebAssembly version
make web
```

This will:
1. Generate the embedded standard library
2. Compile all source files to WebAssembly
3. Create `web/orus-web.js` and `web/orus-web.wasm`

## Running the Playground

Since WebAssembly requires HTTP(S) to load, you can't just open `index.html` directly. Use a local server:

```bash
# Using Python
cd web
python3 -m http.server 8000

# Using Node.js
cd web
npx serve

# Using PHP
cd web
php -S localhost:8000
```

Then open http://localhost:8000 in your browser.

## Files

- **`index.html`** - Main playground interface
- **`playground.js`** - JavaScript interface for the playground
- **`orus-web.js`** - Generated Emscripten JavaScript wrapper
- **`orus-web.wasm`** - Generated WebAssembly binary
- **`../src/web/web_main.c`** - WebAssembly-specific C entry point
- **`../src/web/pre.js`** - Pre-JavaScript for WASM setup

## Features

### Playground Interface
- **Code editor** with syntax highlighting (basic)
- **Real-time output** from print() statements
- **Error display** with stack traces
- **VM statistics** (stack size, frame count, etc.)
- **Example code** snippets for learning

### Keyboard Shortcuts
- **Ctrl/Cmd + Enter**: Run code
- **Ctrl/Cmd + L**: Clear output
- **Tab**: Insert 4 spaces in editor

### Example Programs
- **Hello World**: Basic output
- **Variables**: Type system demonstration
- **Functions**: Function definitions and calls
- **Generics**: Generic types and functions
- **Math**: Standard library math functions

## API

The playground exposes a JavaScript API via `Module.Orus`:

```javascript
// Initialize VM (called automatically)
Module.Orus.init()

// Run Orus source code
Module.Orus.run(source)

// Reset VM state
Module.Orus.reset()

// Get version
Module.Orus.version()

// Set I/O handlers
Module.Orus.onInput(callback)
Module.Orus.onOutput(callback)

// Get VM statistics
Module.Orus.getStats()

// Error handling
Module.Orus.getLastError()
Module.Orus.clearError()
```

## Limitations

Current WebAssembly build limitations:

1. **No file I/O**: File operations are not supported in the browser
2. **No process execution**: System calls are disabled
3. **Limited input**: input() function returns empty string by default
4. **Memory constraints**: Limited by browser WebAssembly memory limits

## Development

To modify the WebAssembly build:

1. **Edit source files** in `src/` (shared with native build)
2. **Modify web-specific code** in `src/web/`
3. **Update build flags** in `Makefile` WASM_FLAGS section
4. **Rebuild** with `make web`

### Adding New Exported Functions

1. Add function to `src/web/web_main.c` with `EMSCRIPTEN_KEEPALIVE`
2. Add function name to `EXPORTED_FUNCTIONS` in Makefile
3. Add wrapper in `src/web/pre.js`
4. Use from `playground.js`

### Debugging

- **Browser DevTools**: Check console for errors
- **WASM debugging**: Use browser WASM debugging tools
- **VM state**: Use `Module.Orus.getStats()` for inspection
- **Error messages**: Use `Module.Orus.getLastError()`

## Performance

The WebAssembly build should perform similarly to the native version for most operations. Some considerations:

- **Startup time**: Initial WASM loading (~100-200ms)
- **Memory usage**: Efficient thanks to custom GC
- **Execution speed**: Near-native performance for compute-intensive code

## Future Enhancements

Planned improvements for the playground:

1. **Syntax highlighting**: Full Orus language syntax highlighting
2. **Auto-completion**: IntelliSense-style code completion
3. **Error annotations**: Inline error display in editor
4. **Share code**: URL-based code sharing
5. **File system simulation**: Virtual file system for modules
6. **Performance profiling**: Built-in performance analysis
7. **Mobile optimization**: Touch-friendly interface

## Troubleshooting

**Build fails with "emcc not found"**
- Install Emscripten SDK and source the environment

**Playground loads but VM fails to initialize**
- Check browser console for errors
- Ensure WASM is enabled in browser

**Code runs but no output**
- Check that output handler is properly set
- Verify print() statements in code

**Memory errors during execution**
- Increase INITIAL_MEMORY in Makefile WASM_FLAGS
- Check for infinite loops or excessive memory usage