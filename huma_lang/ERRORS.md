# HUMA X Diagnostics & Error System

HUMA X provides clear, human-readable compiler diagnostics.

---

## 1. Undefined Variable with Typo Suggestion

```huma
make total = 100
show totl
```
Diagnostic:
```
Error:
Unknown variable 'totl'.

Location:
main.huma:2:6

Suggestion:
Did you mean 'total'?
```

---

## 2. Fixed Constant Reassignment

```huma
fixed PI = 3.14
change PI to 3.14159
```
Diagnostic:
```
Error:
Cannot reassign fixed variable 'PI'.

Location:
main.huma:2:1

Suggestion:
Variable was declared with 'fixed'. Remove 'fixed' if it should be mutable.
```

---

## 3. Invalid Return Outside Function

```huma
give back 42
```
Diagnostic:
```
Error:
'give back' or 'return' used outside of a function body.

Location:
main.huma:1:1
```

---

## 4. Syntax Error Missing End

```huma
if x is greater than 10 then
show x
# forgot 'end'
```
Diagnostic:
```
Error:
Expected 'end' at close of 'if' block.
```
