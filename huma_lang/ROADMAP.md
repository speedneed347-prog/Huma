# HUMA X Project Roadmap

## Phase 1 (Completed — v1.0.0)
- [x] Natural English-like grammar (`make`, `fixed`, `change`, `if`, `repeat`, `create function`).
- [x] Lexer with multi-word token lookahead and source tracking.
- [x] Recursive descent parser with statement synchronization.
- [x] Scoped symbol table and semantic type checking.
- [x] Levenshtein-distance typo suggestion diagnostics.
- [x] Intermediate Representation (TAC) with constant folding.
- [x] Stack-based Bytecode Virtual Machine with call frames.
- [x] Real Native Compiler generating ELF x86-64/ARM64 binaries via C++20 and GCC.
- [x] WebAssembly / JavaScript code generation.
- [x] Standard library (`huma.core`, `huma.math`, `huma.text`, `huma.collections`, `huma.system`, `huma.time`).
- [x] CLI commands (`version`, `run`, `check`, `build`, `compile`, `format`, `repl`, `docs`, `package`).
- [x] Automated test suite and GitHub Actions CI matrix.
- [x] Cloud Compiler API (`/api/compile`, `/api/check`, `/api/run`, etc.).
- [x] HUMA Studio Web IDE with live execution, AST, IR, and bytecode disassembly views.

## Phase 2 (Planned — v1.1.0)
- Direct LLVM IR backend generator.
- Language Server Protocol (LSP) daemon (`huma-lsp`) for VS Code and Neovim.
- SIMD vectorized array extensions.
