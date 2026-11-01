# HUMA X Compiler Architecture

The HUMA X compiler is built with a modular, multi-stage pipeline written in C++20.

```
                    HUMA X Source (.huma)
                              ↓
                      [Lexer / Tokenizer]
                              ↓
                           Tokens
                              ↓
                      [Recursive Parser]
                              ↓
                     Abstract Syntax Tree
                              ↓
            [Semantic Analyzer & Symbol Table]
               (Type checking & Typo Suggestion)
                              ↓
                   Valid Typed AST
                    /        |        \
                   /         |         \
                  ↓          ↓          ↓
            [Bytecode]    [IR / TAC]  [Native Codegen]
            [Compiler]    [Optimizer] [C++20 & GCC]
                ↓             ↓           ↓
             Bytecode     Optimized     Machine Binary
             (Virtual        IR       (ELF x86-64/ARM64)
             Machine)
```

---

## 1. Lexical Analysis (`huma::Lexer`)
- Converts source text into discrete typed `Token` objects.
- Multi-word keyword lookahead recognizes composite tokens like `is greater than`, `give back`, and `repeat while`.
- Tracks exact filename, line, and column coordinates.

## 2. Syntax Analysis (`huma::Parser`)
- Recursive-descent parser producing strongly-typed AST nodes (`huma::Stmt`, `huma::Expr`).
- Precedence climbing handles arithmetic and logical operator associativity.
- Error recovery synchronizes to statement boundaries.

## 3. Semantic Analysis (`huma::SemanticAnalyzer`)
- Hierarchical scoped symbol table (`huma::Scope`).
- Variable immutability enforcement (`fixed` variables).
- Levenshtein-distance typo suggestion matches undefined identifiers with valid candidates.

## 4. Intermediate Representation (`huma::IRGenerator`)
- Three-Address Code (TAC) linear instructions.
- Basic blocks and Control Flow Graph (CFG).
- `huma::IROptimizer` performs constant folding and dead code elimination.

## 5. Bytecode & Virtual Machine (`huma::VM`)
- Stack-based evaluation engine.
- Call frames manage local slots and execution pointers.
- Reference-counted runtime value system (`huma::Value`).

## 6. Native Backend (`huma::NativeCodeGenerator`)
- Translates HUMA AST to standalone C++20 code with embedded runtime.
- Invokes host compiler (`g++ -O2 -std=c++20`) to produce standalone native binaries.

## 7. WebAssembly & JS Generator (`huma::WasmCodeGenerator`)
- Compiles AST to browser-compatible runtime scripts.
