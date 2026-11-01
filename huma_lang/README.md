# HUMA X — Human-Oriented Universal Machine Language

[![HUMA CI](https://github.com/huma-lang/huma-x/actions/workflows/ci.yml/badge.svg)](https://github.com/huma-lang/huma-x)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://isocpp.org/)
[![Version](https://img.shields.io/badge/version-1.0.0-green.svg)](https://huma-lang.org)

**HUMA X** is a production-grade, human-oriented programming language, bytecode virtual machine, and native compiler ecosystem designed around English-like readable syntax, deterministic semantics, high performance, and developer ergonomics.

---

## Key Highlights

- **Natural Syntax**: Program with readable declarations (`make`, `change`, `fixed`), explicit conditions (`if age is greater than 18 then ... otherwise ... end`), and intuitive loops (`repeat 5 times`, `repeat for each item in list`).
- **Real Native Compilation**: Generates native machine-code binaries (ELF x86-64/ARM64, Mach-O, PE) via an optimized C++20 code generator backed by GCC / Clang.
- **Fast Bytecode Virtual Machine**: High-speed stack-based execution engine with call frames, dynamic symbol tables, and safe collection indexing.
- **WebAssembly / JS Target**: Exports portable browser-ready JavaScript / WebAssembly bundles.
- **Intelligent Diagnostics**: Typo detection powered by Levenshtein distance suggestions (e.g. *"Did you mean 'amount'?"*).
- **Rich Standard Library**: Built-in modules for Math, Strings, Collections, File I/O, Time, System, JSON, Base64 Crypto, and Unit Testing.
- **Developer Tooling**: First-class CLI (`huma`), code formatter (`huma format`), documentation generator (`huma docs`), interactive REPL (`huma repl`), and package manager (`huma package`).
- **Interactive Cloud & Browser IDE**: Full HUMA Studio IDE with AST viewer, IR disassembly, and code execution.

---

## Quick Example

```huma
make name = "Manojit"
make age = 18

show "Hello, HUMA X!"
show name

change age to age + 1

if age is greater than 18 then
    show "Adult"
otherwise
    show "Underage"
end

repeat 5 times
    show "HUMA X"
end

create function add using a, b
    give back a + b
end

make result = call add using 10, 20
show result
```

---

## Quick Start

### 1. Build the Compiler

```bash
git clone https://github.com/huma-lang/huma-x.git
cd huma-x/huma_lang
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j4
sudo cp build/huma /usr/local/bin/
```

### 2. Run a Program

```bash
huma run examples/hello.huma
```

### 3. Build a Native Standalone Binary

```bash
huma build examples/variables.huma -o my_app
./my_app
```

---

## Documentation

- [Installation Guide](INSTALLATION.md)
- [Building from Source](BUILDING.md)
- [Language Reference](LANGUAGE_REFERENCE.md)
- [Syntax Specification](SYNTAX.md)
- [Compiler Architecture](COMPILER_ARCHITECTURE.md)
- [Standard Library Guide](STANDARD_LIBRARY.md)
- [CLI Reference](CLI_REFERENCE.md)
- [Error Diagnostics](ERRORS.md)
- [Contributing Guidelines](CONTRIBUTING.md)
- [Security Model](SECURITY.md)
- [Future Roadmap](ROADMAP.md)
