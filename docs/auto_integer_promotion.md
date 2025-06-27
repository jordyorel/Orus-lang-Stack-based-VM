# Automatic Integer Promotion Proposal

This document outlines a design for eliminating silent `i32` wrap‑arounds in the
Orus VM by introducing automatic integer promotion. The goal is that whenever an
arithmetic operation on a 32‑bit value would overflow, the VM traps the
condition, widens the operands to a larger integer type and re‑executes the
operation so the program continues with the mathematically correct result.

## IR changes

* **Value Types** – Keep existing `VAL_I32`/`VAL_I64` but allow a value to be
  upgraded from `VAL_I32` to `VAL_I64` at runtime.
* **Opcodes** – `OP_ADD_I32`, `OP_SUBTRACT_I32`, etc. behave as before but when
  an overflow is detected they perform the operation again using the wider
  `i64` instruction and push an `i64` result.
* **Type Information** – When a variable of type `i32` receives an `i64` value
  due to promotion the VM accepts it. `checkValueAgainstType` now allows an `i64`
  value for an `i32` type if the number still fits within the 32‑bit range.  This
  permits gradual widening without immediate type errors.

## VM behaviour

1. **Overflow detection** – `binaryOpI32` uses `__builtin_*_overflow` to detect
   overflow. On overflow it converts both operands to `int64_t` and performs the
   calculation again using `binaryOpI64` semantics.
2. **Promotion** – The result of the widened operation is pushed as an `i64`
   value. Subsequent arithmetic will operate on the larger type. Overflows beyond
   `i64` currently trigger a warning.
3. **Parsing literals** – Unsuffixed integer literals continue to parse as
   `i32` when they fit in 32 bits. Larger literals become `i64`. Explicit
   suffixes (e.g. `123_i32`) can still be used to force a specific type.

## Static inference

The compiler already tracks variable types. By examining constant ranges and
loop bounds during type inference it can conservatively upgrade a variable from
`i32` to `i64` when calculations prove it may exceed the 32‑bit limit. This keeps
bytecode compact and avoids promotions at runtime.

## Example pseudocode

```
if (operation_on_i32_overflows(a, b)) {
    int64_t la = (int64_t)a;
    int64_t lb = (int64_t)b;
    int64_t lr;
    if (__builtin_add_overflow(la, lb, &lr)) {
        // Overflow beyond i64 currently results in a wraparound
    }
    push_i64(lr);
} else {
    push_i32(result);
}
```

## Unit tests

Each scenario below creates values that overflow `i32` but succeed with
promotion to `i64`.

```
// 1. Summation
let mut sum = 0
for i in 0..1_000_000 {
  sum += i
}
assert(sum == 499_999_500_000_i64)

// 2. Factorials
let mut prod = 1
for i in 1..20 {
  prod *= i
}
assert(prod == 2432902008176640000_i64)

// 3. Exponentiation
let x = 10_i32.pow(9)
let y = 10_i32.pow(10)
assert(x == 1_000_000_000_i64)
assert(y == 10_000_000_000_i64)

// 4. Geometric Series
let mut v = 1
for _ in 0..32 { v *= 2 }
assert(v == 4294967296_i64)

// 5. Bit shifts
let a = 1 << 31
let b = 1 << 32
assert(a == -2147483648_i64)
assert(b == 4294967296_i64)

// 6. Large collection
let data = load_big_array()
let mut total = 0
for x in data { total += x }
// total promoted to i64 automatically

// 7. Parsing big literals
let n = parse_int("3000000000")
assert(n == 3000000000_i64)

// 8. Index calculations
let idx = length * page + offset

// 9. Underflow on subtraction
let mut x = i32::MIN
x -= 1
assert(x == -2147483649_i64)

//10. Mixed widths
let big: i64 = compute_big()
let small: i32 = big as i32
let res = (a * b) + c - d
```

These tests confirm that each overflowing computation returns an `i64` result
without producing a wraparound warning.
