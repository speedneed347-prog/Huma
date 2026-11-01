# Building HUMA X from Source

## 1. Quick Build

```bash
cd huma_lang
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build -j4
```

## 2. Running Automated Tests

```bash
# Run unit and integration tests
./build/huma_test_suite

# Or run via CTest
cd build && ctest --output-on-failure
```

## 3. Build Targets
- `huma`: Main command-line compiler and VM executable.
- `huma_core`: Static C++20 library containing the lexer, parser, semantic analyzer, IR, VM, and codegen backends.
- `huma_test_suite`: Automated validation test suite.
