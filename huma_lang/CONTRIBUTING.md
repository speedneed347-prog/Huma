# Contributing to HUMA X

We welcome contributions to HUMA X!

## Development Workflow
1. Fork the repository and create a feature branch (`git checkout -b feature/awesome-feature`).
2. Implement your changes adhering to C++20 standard practices.
3. Add unit test coverage in `tests/test_suite.cpp`.
4. Run the full test suite locally:
   ```bash
   cmake -B build -S huma_lang
   cmake --build build -j4
   ./build/huma_test_suite
   ```
5. Commit and open a Pull Request.
