# Roadmap: Automatic Integer Promotion

This document outlines the planned steps to deliver automatic integer promotion in Orus.
It complements the design in `auto_integer_promotion.md` and breaks down the work into actionable phases.

## Phase 1 – Runtime Promotion (2-3 weeks)
- [x] Detect `i32` overflows in `binaryOpI32` and related helpers using `__builtin_*_overflow`.
- [x] On overflow, widen operands to `i64`, perform the operation again and push an `i64` result.
- [x] Update the VM stack helpers so `vmPushI64` and `vmPopI64` work seamlessly with promoted values.
- [x] Integrate overflow checks into shift, increment and other integer opcodes.
- [x] Provide an environment flag to disable overflow warnings during rollout.

## Phase 2 – IR and Type Adjustments (3-4 weeks)

- [x] Allow `Value` instances to upgrade from `VAL_I32` to `VAL_I64` without type errors.
- [x] Extend `checkValueAgainstType` so an `i64` value is acceptable for `i32` variables when it still fits.
- [x] Review built‑in functions and the standard library for assumptions about fixed widths.
- [x] Update serialization and bytecode formats if necessary to record promoted widths.

- [x] Enhance the type checker to analyse loops and constant expressions for potential overflow.
- [x] Automatically generate `i64` bytecode in cases where overflow is provably possible.
- [x] Emit promotion hints in debug builds to validate the inference behaviour.


## Testing and Validation
- Extend the unit test suite with the scenarios listed in `auto_integer_promotion.md`.
- Stress test edge cases such as subtraction underflow and bit shifts past 31/63 bits.
- Measure performance overhead to ensure promotions do not degrade common code paths.

Implementing these phases will eliminate the current overflow warnings and enable Orus programs to seamlessly use larger integers when needed.
