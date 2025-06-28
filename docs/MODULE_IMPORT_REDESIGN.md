# Module Import Syntax Redesign Roadmap

This document outlines the steps required to migrate Orus from the current `use module::path` syntax to the new dot separated syntax:

```
use tests.modules.hello_module
```

The goal is to remove the `::` separator entirely and replace it with `.` for module paths.

**Status:** Phase 4 (compatibility warnings in parser) implemented. Migration cleanup is next.

## 1. Lexer and Parser

1. **Update the scanner**
   - Remove the `TOKEN_DOUBLE_COLON` token definition and handling.
   - Ensure `.` (`TOKEN_DOT`) is returned for single dots in module paths.

2. **Update the parser**
   - Modify `useStatement` to parse identifiers separated by `.` instead of `::`.
   - Adjust error messages that reference the old syntax.
   - Update the precedence table to drop `TOKEN_DOUBLE_COLON` if unused elsewhere.

3. **Update error handling**
   - Search for references to `"::"` in error helpers and revise examples to use dot notation.

## 2. AST and Compiler

1. **AST changes**
   - Ensure module path representation (typically an array of strings) is unaffected by the separator change.
   - Validate any functions that split module paths by `::` are updated to use `.`.

2. **Compiler adjustments**
   - Update module loader to interpret dot-separated paths when resolving filenames.

## 3. Standard Library and Examples

1. **Update shipped modules** to use the new syntax.
2. **Update documentation** in `docs/`, examples in `web/`, and any code snippets referencing the old syntax.
3. **Update all tests** under `tests/` to reflect the new import style.

## 4. Migration Strategy

1. The parser now emits a compatibility warning when it encounters `::` to guide users toward the new `.` syntax.
2. After a deprecation period, support for `::` will be removed entirely.

## 5. Testing

1. Adjust existing test cases to use dot notation.
2. Run the full test suite with `tests/run_all_tests.sh` to ensure no regressions.

---

Following this roadmap will transition Orus to the simpler `use module.path` syntax while keeping backward compatibility during the migration period.
