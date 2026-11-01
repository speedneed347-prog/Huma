# HUMA X Standard Library Guide

The HUMA X standard library is built-in and accessible without manual configuration.

---

## 1. `huma.core`

| Function | Parameters | Description |
|---|---|---|
| `show(args...)` | Any | Prints arguments to standard output separated by space with trailing newline. |
| `length(col)` | string / list / dict | Returns count of characters or elements. |
| `type_of(val)` | Any | Returns type name string (`"number"`, `"text"`, `"list"`, etc.). |
| `to_string(val)` | Any | Converts any value to string representation. |
| `to_number(val)` | Any | Converts string to number. |
| `range(start, end, step)` | int, int, int | Returns list of integers in range. |
| `assert(condition, [msg])` | bool, string | Asserts condition or raises runtime error. |

---

## 2. `huma.math`

Constants:
- `PI`: 3.141592653589793
- `E`: 2.718281828459045

Functions:
- `sqrt(x)`: Square root.
- `pow(base, exp)`: Power / exponentiation.
- `abs(x)`: Absolute value.
- `sin(rad)`, `cos(rad)`, `tan(rad)`: Trigonometric functions.
- `floor(x)`, `ceil(x)`, `round(x)`: Rounding operations.
- `min(a, b)`, `max(a, b)`: Minimum and maximum.
- `random()`: Uniform random float in `[0.0, 1.0)`.

---

## 3. `huma.text`

- `to_upper(text)`: Converts text to uppercase.
- `to_lower(text)`: Converts text to lowercase.
- `trim(text)`: Strips whitespace from both ends.
- `starts_with(text, prefix)`: Returns `yes`/`no`.
- `ends_with(text, suffix)`: Returns `yes`/`no`.
- `contains(haystack, needle)`: Checks substring or element presence.
- `split(text, delimiter)`: Splits string into list of strings.
- `join(list, delimiter)`: Joins list into delimited string.

---

## 4. `huma.collections`

- `push(list, item)`: Appends element to list.
- `pop(list)`: Removes and returns last element.
- `keys(dict)`: Returns list of dictionary keys.
- `values(dict)`: Returns list of dictionary values.

---

## 5. `huma.files`

- `read_file(path)`: Reads entire file as string.
- `write_file(path, content)`: Overwrites file with content.
- `file_exists(path)`: Checks if file exists.

---

## 6. `huma.system` and `huma.time`

- `platform()`: Returns `"linux"`, `"macos"`, or `"windows"`.
- `clock()`: High-resolution monotonic clock in seconds.
- `now()`: Current Unix epoch timestamp in milliseconds.
- `sleep(ms)`: Suspends execution for `ms` milliseconds.

---

## 7. `huma.crypto` and `huma.json`

- `base64_encode(str)`: Encodes string to Base64 format.
- `json_stringify(val)`: Serializes value to JSON string.

---

## 8. `huma.testing`

- `assert_equal(actual, expected, [desc])`: Tests equality.
- `assert_true(cond, [desc])`: Tests boolean truth.
