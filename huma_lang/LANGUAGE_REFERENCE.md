# HUMA X Language Reference

HUMA X (Human-Oriented Universal Machine Language) is designed to combine the clarity of natural English with the performance and predictability of modern systems languages.

---

## 1. Variables and Constants

### Mutable Variables: `make`
Variables are declared using the `make` keyword:
```huma
make user_count = 100
make user_name = "Manojit"
make is_active = yes
```

### Type Annotations (Optional)
```huma
make age as integer = 25
make score as decimal = 98.5
make title as text = "Engineer"
```

### Immutable Constants: `fixed`
Constants cannot be reassigned after declaration:
```huma
fixed MAX_CONNECTIONS = 1024
fixed PI = 3.14159265
```

### Reassignment: `change ... to ...`
```huma
change user_count to user_count + 1
```

Attempting to reassign a `fixed` variable triggers a compile-time semantic error.

---

## 2. Built-in Data Types

| Type | Examples | Description |
|---|---|---|
| `number` / `integer` / `decimal` | `42`, `-10`, `3.14` | 64-bit IEEE double or signed integer |
| `text` / `string` | `"Hello"`, `"HUMA X"` | UTF-8 encoded string |
| `boolean` | `yes`, `no` (or `true`, `false`) | Boolean logic |
| `nothing` / `null` | `nothing` | Absence of a value |
| `list` | `[1, 2, "three", yes]` | Dynamic ordered array |
| `dictionary` | `{"id": 1, "name": "Manojit"}` | Key-value associative mapping |

---

## 3. Operators

### Arithmetic Operators
- `+` : Addition or string concatenation (`"Hello " + "World"`)
- `-` : Subtraction or unary negation (`-5`)
- `*` : Multiplication
- `/` : Division
- `%` : Modulo (remainder)
- `^` : Exponentiation (`2 ^ 8 = 256`)

### English-like Comparison Operators
- `is greater than` / `>`
- `is less than` / `<`
- `is greater than or equal to` / `>=`
- `is less than or equal to` / `<=`
- `is equal to` / `is` / `==`
- `is not equal to` / `is not` / `!=`

### Boolean Logic
- `and` : Logical AND
- `or` : Logical OR
- `not` : Logical negation

---

## 4. Conditional Statements

```huma
if score is greater than or equal to 90 then
    show "Grade: A"
else if score is greater than or equal to 80 then
    show "Grade: B"
otherwise
    show "Grade: Needs improvement"
end
```

---

## 5. Loops and Iteration

### Repeat N Times
```huma
repeat 5 times
    show "Echo"
end
```

### Repeat While Condition
```huma
make i = 0
repeat while i is less than 5
    show i
    change i to i + 1
end
```

### Repeat For Each Item
```huma
make fruits = ["Apple", "Banana", "Cherry"]
repeat for each fruit in fruits
    show "Fruit:", fruit
end
```

### Loop Control
- `break`: Terminate the enclosing loop.
- `continue`: Skip to the next iteration.

---

## 6. Functions

Functions are declared with `create function` and closed with `end`:

```huma
create function calculate_area using width, height
    make area = width * height
    give back area
end

make total = call calculate_area using 10, 5
show total
```

Functions can be called using either natural call syntax:
```huma
call function_name using arg1, arg2
```
or standard function call syntax:
```huma
function_name(arg1, arg2)
```

---

## 7. Collections

### Lists
```huma
make items = [10, 20, 30]
show items[0]              # 10
push(items, 40)            # [10, 20, 30, 40]
show length(items)         # 4
```

### Dictionaries
```huma
make config = {
    "host": "localhost",
    "port": 8080,
    "ssl": yes
}
show config["port"]
```

---

## 8. Classes and Object-Oriented Programming

```huma
create class Server
    make host
    make port

    create function start using self
        show "Server listening on port:", self.port
    end
end
```
