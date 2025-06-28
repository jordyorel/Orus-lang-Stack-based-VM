# Roadmap for Improved Array Syntax

This document outlines the steps to add more ergonomic array declarations in Orus. The goal is to support Rust-style fill expressions, length inference from initializer values, and const-safe fills.

**Status:** Phases 1–5 implemented in the interpreter (AST support, parsing, constant evaluation, type checking and code generation). Remaining phases are still in progress.

## 1. Extend the AST ✅ Done
- Introduce an `AST_ARRAY_FILL` node in `include/ast.h` and `src/compiler/ast.c`.
- The node stores the fill `value` expression and the `length` expression.

## 2. Update Parsing Logic ✅ Done
- Modify `parseArray()` in `src/parser/parser.c` to detect `[expr; expr]`.
- Create an `AST_ARRAY_FILL` when this pattern is parsed.
- Preserve current handling of `[expr, expr, ...]` lists.

## 3. Enhance Constant Evaluation ✅ Done
- Expand `evaluateConstantInt()` in `src/compiler/compiler.c` to resolve variables that refer to `const` declarations.
- This allows expressions like `const N = 10; [0; N]`.

## 4. Type Checking Adjustments ✅ Done
- During type checking of `let` bindings:
  - Infer array length from the `AST_ARRAY_FILL` length expression.
  - Infer length from the number of elements in an `AST_ARRAY` when the declaration omits the size.
- Emit an error if the fill length is not a compile-time constant or if element types are ambiguous.

## 5. Code Generation ✅ Done
- Generate bytecode to allocate and fill arrays using the new syntax.
- Introduce a VM opcode such as `OP_ARRAY_FILL` or emit repeated element pushes.
- Ensure the generated array is fixed-size when length is inferred at compile time.

## 6. Runtime Semantics
- Arrays produced via fill syntax or length inference behave as fixed-size arrays.
- Disallow `push` or `pop` on these arrays at compile time.
- Dynamic arrays remain supported when explicitly declared.

## 7. Tests
- Add unit tests under `tests/` to verify:
  - `[0; 30]` yields `[i32; 30]`.
  - `const N = 4; let a: [i32; N] = [0; N]` succeeds.
  - `[0; n]` where `n` is a runtime variable fails.
  - `[1, 2, 3]` without a length annotation creates `[i32; 3]` and rejects `push`.

## 8. Documentation Updates
- Extend `docs/LANGUAGE.md` with examples of the new array syntax.
- Document the compile-time constant requirement for fill lengths.

## 9. Backward Compatibility
- Maintain existing `[T; N]` and dynamic array behavior.
- Optionally warn if a declaration might be ambiguous.

## 10. Timeline & Milestones
- **Week 1–2:** Implement AST and parser changes, constant evaluation.
- **Week 3:** Update type checking and code generation.
- **Week 4:** Write tests and documentation; validate the test suite.
- **Week 5:** Code review, improve error messages, merge into `main`.
