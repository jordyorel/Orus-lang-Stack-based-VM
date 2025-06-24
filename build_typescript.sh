#!/bin/bash

# Build TypeScript version of Orus WebAssembly
# Requires: TypeScript compiler (npm install -g typescript)

set -e

echo "Building Orus WebAssembly with TypeScript support..."

# Check if TypeScript compiler is available
if ! command -v tsc &> /dev/null; then
    echo "TypeScript compiler not found. Install with:"
    echo "  npm install -g typescript"
    echo "Or use npx: npx tsc instead of tsc"
    exit 1
fi

# Build the WebAssembly module first
echo "Building WebAssembly module..."
make web

# Create TypeScript config if it doesn't exist
if [ ! -f "web/tsconfig.json" ]; then
    echo "Creating TypeScript configuration..."
    cat > web/tsconfig.json << 'EOF'
{
  "compilerOptions": {
    "target": "ES2017",
    "module": "none",
    "lib": ["ES2017", "DOM"],
    "outDir": "./",
    "rootDir": "./",
    "strict": true,
    "esModuleInterop": true,
    "skipLibCheck": true,
    "forceConsistentCasingInFileNames": true,
    "moduleResolution": "node",
    "declaration": false,
    "sourceMap": true
  },
  "include": [
    "*.ts"
  ],
  "exclude": [
    "node_modules",
    "*.d.ts"
  ]
}
EOF
fi

# Compile TypeScript
echo "Compiling TypeScript..."
cd web
tsc playground.ts

echo "TypeScript build complete!"
echo ""
echo "Files generated:"
echo "  web/orus-web.js (WebAssembly module)"
echo "  web/orus-web.wasm (WebAssembly binary)"
echo "  web/orus-web.d.ts (Generated TypeScript definitions)"
echo "  web/orus.d.ts (Custom TypeScript definitions)"
echo "  web/playground.js (Compiled from playground.ts)"
echo ""
echo "For development with TypeScript:"
echo "  1. Use any TypeScript-enabled IDE (VS Code, WebStorm, etc.)"
echo "  2. Edit playground.ts with full IntelliSense support"
echo "  3. Compile with: tsc playground.ts"
echo "  4. Test with: open index-ts.html"
echo ""
echo "For production:"
echo "  - Use the compiled playground.js"
echo "  - Include orus.d.ts for type checking"
echo "  - Optionally minify with tools like terser"