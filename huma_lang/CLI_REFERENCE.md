# HUMA X CLI Reference

The `huma` CLI is the official developer tool for the HUMA X ecosystem.

---

## Command Overview

```bash
huma version
huma run <file> [--target vm|native|wasm]
huma check <file> [--json]
huma build <file> [-o <output>]
huma compile <file> [--target native|bytecode|wasm] [-o <output>]
huma test [dir]
huma format <file> [-o <output>]
huma repl
huma package [init|read]
huma docs <file> [-o <output>]
huma dump-ast <file>
huma dump-ir <file>
huma dump-bytecode <file>
```

---

## Detailed Command Specifications

### 1. `huma version`
Displays compiler, runtime, target backend, and specification version.

### 2. `huma run <file>`
Executes a HUMA source file.
Options:
- `--target vm`: High-speed bytecode VM (default).
- `--target native`: Compiles to native binary and runs.
- `--json`: Outputs machine-readable execution results.

### 3. `huma check <file>`
Performs complete lexical, syntactic, and semantic validation without executing.
Options:
- `--json`: Outputs structured diagnostics with errors, warnings, lines, columns, and suggestions for IDE integration.

### 4. `huma build <file>`
Compiles HUMA program into a standalone native machine-code binary (ELF executable on Linux).
Example:
```bash
huma build app.huma -o app
./app
```

### 5. `huma compile <file>`
Compiles to the requested target:
- `--target native`: Native machine code binary.
- `--target bytecode`: Bytecode disassembly archive (`.humac`).
- `--target wasm`: Browser JavaScript / WebAssembly bundle (`.js`).

### 6. `huma format <file>`
Formats HUMA source code adhering to canonical HUMA X styling conventions.

### 7. `huma repl`
Launches the interactive read-eval-print loop with multi-line input support.

### 8. `huma package init <name>`
Scaffolds a new HUMA project with directory layout, `huma.pkg`, `src/main.huma`, and unit test suite.
