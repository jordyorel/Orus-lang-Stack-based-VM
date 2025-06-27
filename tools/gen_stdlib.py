#!/usr/bin/env python3
"""
Generate embedded stdlib for Orus compiler.
This script reads .orus files from the std/ directory and generates
builtin_stdlib.c and builtin_stdlib.h files with embedded source code.
"""

import os
import sys
import glob

def escape_c_string(content):
    """Escape a string for use in C source code."""
    # Replace backslashes first, then other special characters
    content = content.replace('\\', '\\\\')
    content = content.replace('"', '\\"')
    content = content.replace('\n', '\\n')
    content = content.replace('\r', '\\r')
    content = content.replace('\t', '\\t')
    return content

def generate_stdlib_files(std_dir, c_file, h_file):
    """Generate the builtin_stdlib.c and builtin_stdlib.h files."""
    
    # Find all .orus files in the std directory
    orus_files = glob.glob(os.path.join(std_dir, "*.orus"))
    orus_files.sort()  # Ensure consistent ordering
    
    modules = []
    
    # Read each .orus file
    for file_path in orus_files:
        module_name = f"std/{os.path.basename(file_path)}"
        
        try:
            with open(file_path, 'r', encoding='utf-8') as f:
                content = f.read()
            
            # Escape the content for C string
            escaped_content = escape_c_string(content)
            
            modules.append({
                'name': module_name,
                'content': escaped_content
            })
            
            print(f"Processed: {module_name}")
            
        except Exception as e:
            print(f"Error reading {file_path}: {e}")
            sys.exit(1)
    
    # Generate header file
    header_content = f"""#ifndef BUILTIN_STDLIB_H
#define BUILTIN_STDLIB_H

typedef struct {{
    const char* name;
    const char* source;
}} EmbeddedModule;

extern const EmbeddedModule embeddedStdlib[];
extern const int embeddedStdlibCount;

const char* getEmbeddedModule(const char* name);
void dumpEmbeddedStdlib(const char* dir);

#endif // BUILTIN_STDLIB_H
"""
    
    # Generate C file
    c_content = f"""#include "../../include/builtin_stdlib.h"
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>

const EmbeddedModule embeddedStdlib[] = {{
"""
    
    # Add each module to the array
    for module in modules:
        c_content += f'    {{"{module["name"]}", "{module["content"]}"}},\n'
    
    c_content += f"""}};\nconst int embeddedStdlibCount = sizeof(embeddedStdlib)/sizeof(EmbeddedModule);

const char* getEmbeddedModule(const char* name){{
    for(int i=0;i<embeddedStdlibCount;i++){{
        if(strcmp(embeddedStdlib[i].name,name)==0) return embeddedStdlib[i].source;
    }}
    return NULL;
}}

static void ensure_dir(const char* path){{
    char tmp[512];
    strncpy(tmp,path,sizeof(tmp)-1);
    tmp[sizeof(tmp)-1]=0;
    for(char* p=tmp+1; *p; p++){{ if(*p=='/'){{ *p=0; mkdir(tmp,0755); *p='/'; }} }}
    mkdir(tmp,0755);
}}

void dumpEmbeddedStdlib(const char* dir){{
    char full[512];
    for(int i=0;i<embeddedStdlibCount;i++){{
        snprintf(full,sizeof(full),"%s/%s",dir,embeddedStdlib[i].name);
        char* slash=strrchr(full,'/');
        if(slash){{ *slash=0; ensure_dir(full); *slash='/'; }} else {{ ensure_dir(full); }}
        FILE* f=fopen(full,"w"); if(f){{ fputs(embeddedStdlib[i].source,f); fclose(f); }}
    }}
}}
"""
    
    # Write header file
    try:
        os.makedirs(os.path.dirname(h_file), exist_ok=True)
        with open(h_file, 'w', encoding='utf-8') as f:
            f.write(header_content)
        print(f"Generated: {h_file}")
    except Exception as e:
        print(f"Error writing {h_file}: {e}")
        sys.exit(1)
    
    # Write C file
    try:
        os.makedirs(os.path.dirname(c_file), exist_ok=True)
        with open(c_file, 'w', encoding='utf-8') as f:
            f.write(c_content)
        print(f"Generated: {c_file}")
    except Exception as e:
        print(f"Error writing {c_file}: {e}")
        sys.exit(1)

def main():
    if len(sys.argv) != 4:
        print("Usage: python3 gen_stdlib.py <std_dir> <output_c_file> <output_h_file>")
        print("Example: python3 gen_stdlib.py std src/vm/builtin_stdlib.c include/builtin_stdlib.h")
        sys.exit(1)
    
    std_dir = sys.argv[1]
    c_file = sys.argv[2]
    h_file = sys.argv[3]
    
    if not os.path.isdir(std_dir):
        print(f"Error: Directory {std_dir} does not exist")
        sys.exit(1)
    
    print(f"Generating stdlib from {std_dir}...")
    generate_stdlib_files(std_dir, c_file, h_file)
    print("Done!")

if __name__ == "__main__":
    main()
