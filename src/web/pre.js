// Pre-JS script for Orus WebAssembly module
// This script runs before the WASM module is loaded and sets up the environment

var Module = Module || {};

// Global variables for I/O callbacks
Module.inputCallback = null;
Module.outputCallback = null;

// Output buffer for collecting print() calls
Module.outputBuffer = "";

// Set up input callback from JavaScript
Module.setInput = function(callback) {
    Module.inputCallback = callback;
};

// Set up output callback from JavaScript  
Module.setOutput = function(callback) {
    Module.outputCallback = callback;
};

// Override console output
Module.print = function(text) {
    if (Module.outputCallback) {
        Module.outputCallback(text);
    } else {
        console.log('Orus Output:', text);
    }
};

// Override console error output
Module.printErr = function(text) {
    if (Module.outputCallback) {
        Module.outputCallback('Error: ' + text);
    } else {
        console.error('Orus Error:', text);
    }
};

// Buffer stdout/stderr for custom handling
Module.stdout = function(charCode) {
    if (charCode === 10) { // newline
        if (Module.outputCallback) {
            Module.outputCallback(Module.outputBuffer);
        }
        Module.outputBuffer = "";
    } else {
        Module.outputBuffer += String.fromCharCode(charCode);
    }
};

Module.stderr = Module.stdout; // Same handling for now

// Set up the runtime when module is ready
Module.onRuntimeInitialized = function() {
    console.log('Orus WebAssembly module loaded successfully');
    
    // Ensure all exported functions are available
    try {
        // Get references to exported functions with error checking
        if (typeof Module.ccall === 'function' && typeof Module.cwrap === 'function') {
            Module._initWebVM = Module.cwrap('initWebVM', 'number', []);
            Module._runSource = Module.cwrap('runSource', 'number', ['string']);
            Module._freeWebVM = Module.cwrap('freeWebVM', 'void', []);
            Module._getVersion = Module.cwrap('getVersion', 'string', []);
            Module._getLastError = Module.cwrap('getLastError', 'string', []);
            Module._clearLastError = Module.cwrap('clearLastError', 'void', []);
            Module._isVMReady = Module.cwrap('isVMReady', 'boolean', []);
            Module._resetVMState = Module.cwrap('resetVMState', 'void', []);
            Module._getVMStackSize = Module.cwrap('getVMStackSize', 'number', []);
            Module._getVMFrameCount = Module.cwrap('getVMFrameCount', 'number', []);
            Module._getVMModuleCount = Module.cwrap('getVMModuleCount', 'number', []);
            
            console.log('Function wrappers created successfully');
        } else {
            throw new Error('ccall/cwrap not available');
        }
        
        // Initialize the VM
        var initResult = Module._initWebVM();
        if (initResult !== 0) {
            console.error('Failed to initialize Orus VM:', initResult);
        } else {
            console.log('Orus VM initialized successfully');
            console.log('Orus version:', Module._getVersion());
        }
    } catch (error) {
        console.error('Error setting up Orus VM:', error);
        console.error('Available Module properties:', Object.keys(Module));
    }
    
    // Notify that the module is ready
    if (Module.onOrusReady) {
        Module.onOrusReady();
    }
};

// High-level API for easier use
Module.Orus = {
    // Initialize Orus (called automatically)
    init: function() {
        return Module._initWebVM();
    },
    
    // Run Orus source code
    run: function(source) {
        if (!Module._isVMReady()) {
            throw new Error('Orus VM is not ready');
        }
        
        Module._clearLastError();
        var result = Module._runSource(source);
        
        // Check for errors
        if (result !== 0) {
            var error = Module._getLastError();
            if (error) {
                throw new Error('Orus execution error: ' + error);
            } else {
                throw new Error('Orus execution failed with code: ' + result);
            }
        }
        
        return result;
    },
    
    // Reset VM state for fresh execution
    reset: function() {
        Module._resetVMState();
    },
    
    // Get version information
    version: function() {
        return Module._getVersion();
    },
    
    // Get VM debugging information
    getStats: function() {
        return {
            stackSize: Module._getVMStackSize(),
            frameCount: Module._getVMFrameCount(),
            moduleCount: Module._getVMModuleCount(),
            isReady: Module._isVMReady()
        };
    },
    
    // Set input handler for input() function
    onInput: function(callback) {
        Module.setInput(callback);
    },
    
    // Set output handler for print() function
    onOutput: function(callback) {
        Module.setOutput(callback);
    },
    
    // Get last error without throwing
    getLastError: function() {
        return Module._getLastError();
    },
    
    // Clear any error state
    clearError: function() {
        Module._clearLastError();
    }
};

// Memory management helpers
Module.allocateString = function(str) {
    var len = lengthBytesUTF8(str) + 1;
    var ptr = Module._malloc(len);
    stringToUTF8(str, ptr, len);
    return ptr;
};

Module.freeString = function(ptr) {
    Module._free(ptr);
};

// Set up cleanup on page unload
if (typeof window !== 'undefined') {
    window.addEventListener('beforeunload', function() {
        if (Module._freeWebVM) {
            Module._freeWebVM();
        }
    });
}

console.log('Orus WebAssembly pre-script loaded');