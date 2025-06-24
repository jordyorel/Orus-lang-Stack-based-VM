// Orus Playground JavaScript Interface

let orusModule = null;
let isRunning = false;

// Example code snippets
const examples = {
    hello: `// Hello World Example
print("Hello, Orus World!")
print("Welcome to the Orus Programming Language!")`,

    variables: `// Variables and Types Example
let name = "Orus"
let version = 0.2
let isAwesome = true

print("Language:", name)
print("Version:", version)
print("Is awesome?", isAwesome)

// Type conversion
let numberStr = "42"
let number = int(numberStr)
print("Converted:", numberStr, "->", number)`,

    functions: `// Functions Example
fn greet(name: string) -> string {
    return "Hello, " + name + "!"
}

fn add(a: i32, b: i32) -> i32 {
    return a + b
}

fn factorial(n: i32) -> i32 {
    if n <= 1 {
        return 1
    }
    return n * factorial(n - 1)
}

print(greet("World"))
print("5 + 3 =", add(5, 3))
print("5! =", factorial(5))`,

    generics: `// Generics Example
struct Container<T> {
    value: T
}

impl<T> Container<T> {
    fn new(val: T) -> Container<T> {
        return Container { value: val }
    }
    
    fn get(self) -> T {
        return self.value
    }
}

let intContainer = Container::new(42)
let stringContainer = Container::new("Hello")

print("Int container:", intContainer.get())
print("String container:", stringContainer.get())`,

    math: `// Math Library Example
use std::math

let x = 16.0
let y = 25.0

print("sqrt(", x, ") =", math::sqrt(x))
print("sqrt(", y, ") =", math::sqrt(y))
print("PI =", math::PI)
print("E =", math::E)

let angle = math::PI / 4.0  // 45 degrees
print("sin(π/4) =", math::sin(angle))
print("cos(π/4) =", math::cos(angle))

print("max(", x, ",", y, ") =", math::max(x, y))
print("min(", x, ",", y, ") =", math::min(x, y))`
};

// Initialize when the WASM module is ready
function initializePlayground() {
    const statusEl = document.getElementById('status');
    const versionEl = document.getElementById('version');
    const runBtn = document.getElementById('runBtn');
    const vmStatsEl = document.getElementById('vmStats');

    try {
        // Get version info
        const version = orusModule.Orus.version();
        versionEl.textContent = version;
        
        // Update status
        statusEl.textContent = 'Ready';
        statusEl.className = 'success';
        
        // Enable run button
        runBtn.disabled = false;
        
        // Update VM stats
        updateVMStats();
        
        console.log('Orus playground initialized successfully');
        
        // Set up I/O handlers
        orusModule.Orus.onOutput(function(text) {
            appendOutput(text);
        });
        
        orusModule.Orus.onInput(function(prompt) {
            // For now, return empty string. Could be enhanced with a proper input dialog
            console.log('Input requested:', prompt);
            return "";
        });
        
    } catch (error) {
        console.error('Error initializing playground:', error);
        statusEl.textContent = 'Error: ' + error.message;
        statusEl.className = 'error';
    }
}

// Run Orus code
function runCode() {
    if (isRunning || !orusModule) {
        return;
    }
    
    const editor = document.getElementById('editor');
    const source = editor.value;
    const statusEl = document.getElementById('status');
    const loadingEl = document.getElementById('loading');
    const runBtn = document.getElementById('runBtn');
    
    if (!source.trim()) {
        appendOutput('Error: No code to execute\n');
        return;
    }
    
    isRunning = true;
    runBtn.disabled = true;
    statusEl.textContent = 'Running...';
    statusEl.className = '';
    loadingEl.classList.add('show');
    
    // Clear any previous errors
    orusModule.Orus.clearError();
    
    try {
        // Run the code
        const result = orusModule.Orus.run(source);
        
        statusEl.textContent = 'Execution completed';
        statusEl.className = 'success';
        
        updateVMStats();
        
    } catch (error) {
        console.error('Execution error:', error);
        appendOutput('Error: ' + error.message + '\n');
        statusEl.textContent = 'Execution failed';
        statusEl.className = 'error';
    } finally {
        isRunning = false;
        runBtn.disabled = false;
        loadingEl.classList.remove('show');
    }
}

// Clear output
function clearOutput() {
    const output = document.getElementById('output');
    output.textContent = '';
}

// Reset VM state
function resetVM() {
    if (!orusModule) return;
    
    try {
        orusModule.Orus.reset();
        appendOutput('VM state reset.\n');
        updateVMStats();
        
        const statusEl = document.getElementById('status');
        statusEl.textContent = 'Ready';
        statusEl.className = 'success';
    } catch (error) {
        console.error('Error resetting VM:', error);
        appendOutput('Error resetting VM: ' + error.message + '\n');
    }
}

// Append text to output
function appendOutput(text) {
    const output = document.getElementById('output');
    output.textContent += text + '\n';
    output.scrollTop = output.scrollHeight;
}

// Update VM statistics display
function updateVMStats() {
    if (!orusModule) return;
    
    try {
        const stats = orusModule.Orus.getStats();
        const vmStatsEl = document.getElementById('vmStats');
        vmStatsEl.textContent = `VM: Stack=${stats.stackSize} Frames=${stats.frameCount} Modules=${stats.moduleCount}`;
    } catch (error) {
        console.error('Error getting VM stats:', error);
    }
}

// Load example code
function loadExample(exampleName) {
    const editor = document.getElementById('editor');
    if (examples[exampleName]) {
        editor.value = examples[exampleName];
        clearOutput();
        appendOutput('Loaded example: ' + exampleName + '\n');
    }
}

// Handle keyboard shortcuts
document.addEventListener('keydown', function(event) {
    // Ctrl+Enter or Cmd+Enter to run
    if ((event.ctrlKey || event.metaKey) && event.key === 'Enter') {
        event.preventDefault();
        runCode();
    }
    
    // Ctrl+L or Cmd+L to clear output
    if ((event.ctrlKey || event.metaKey) && event.key === 'l') {
        event.preventDefault();
        clearOutput();
    }
});

// Handle tab key in editor for indentation
document.getElementById('editor').addEventListener('keydown', function(event) {
    if (event.key === 'Tab') {
        event.preventDefault();
        const start = this.selectionStart;
        const end = this.selectionEnd;
        
        // Insert 4 spaces
        this.value = this.value.substring(0, start) + '    ' + this.value.substring(end);
        this.selectionStart = this.selectionEnd = start + 4;
    }
});

// Initialize when WASM module loads
function loadOrusModule() {
    console.log('Attempting to load Orus module...');
    
    if (typeof OrusModule !== 'undefined') {
        console.log('OrusModule found, instantiating...');
        OrusModule().then(function(Module) {
            console.log('Module instantiated successfully');
            orusModule = Module;
            Module.onOrusReady = initializePlayground;
        }).catch(function(error) {
            console.error('Failed to load Orus module:', error);
            const statusEl = document.getElementById('status');
            statusEl.textContent = 'Failed to load: ' + error.message;
            statusEl.className = 'error';
        });
    } else {
        console.log('OrusModule not found, waiting for script to load...');
        // Set up for when the module script loads
        setTimeout(loadOrusModule, 100); // Retry in 100ms
    }
}

// Start loading after a short delay to ensure DOM is ready
setTimeout(loadOrusModule, 100);

console.log('Orus playground script loaded');