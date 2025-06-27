# Built-in Functions

The Orus runtime provides a small set of native functions that are always
available in every module. They offer basic utilities for working with
arrays, strings and simple I/O without importing additional modules.

| Function | Description |
| -------- | ----------- |
| `print(values...)` | Print values with a trailing newline. |
| `len(value)` | Length of an array or string. |
| `substring(str, start, len)` | Extract a portion of a string. `start` and `len` may be `i32` or `i64`. |
| `push(array, value)` | Append a value to an array. |
| `pop(array)` | Remove and return the last element of an array. |
| `range(start, end)` | Create a lazy integer iterator. |
| `sum(array)` | Sum the numeric elements of an array, promoting to `i64` on overflow. |
| `min(array)` / `max(array)` | Minimum or maximum element of an array. Results promote to `i64` when needed. |
| `sorted(array, reverse)` | Return a sorted copy of an array. |
| `type_of(value)` | Return the name of a value's type. |
| `is_type(value, name)` | Check if a value is of a given type. |
| `input(prompt)` | Read a line of text from the user. |
| `int(text)` / `float(text)` | Convert a string to a number. |
| `timestamp()` | Get the current UNIX timestamp. |
| `module_name(path)` | Module name without extension. |
| `module_path(path)` | Resolve a module's full path. |
| `native_pow(base, exp)` | Fast power using the host math library. The exponent may be `i32` or `i64`. |
| `native_sqrt(x)` | Fast square root using the host math library. |

The `int()` builtin returns an `i32` when the parsed value fits in 32 bits.
If the number is larger, it yields an `i64` instead.

Additional functionality is provided by the standard library modules in
`std/`. See `docs/ORUS_ROADMAP.md` for planned future built-ins.
