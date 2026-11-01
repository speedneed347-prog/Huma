# Installation Guide for HUMA X

## System Requirements
- Linux (x86-64 / ARM64), macOS (Apple Silicon / Intel), or Windows (x86-64)
- C++20 compliant compiler (GCC 12+, Clang 15+, MSVC 2022+)
- CMake 3.20 or newer

---

## Linux (Ubuntu / Debian)

```bash
# 1. Install prerequisites
sudo apt-get update
sudo apt-get install -y build-essential cmake git

# 2. Clone and build
git clone https://github.com/huma-lang/huma-x.git
cd huma-x/huma_lang
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)

# 3. Install binary system-wide
sudo cp build/huma /usr/local/bin/

# 4. Verify installation
huma version
```

---

## macOS

```bash
# 1. Install prerequisites
brew install cmake

# 2. Clone and build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(sysctl -n hw.ncpu)
sudo cp build/huma /usr/local/bin/
```

---

## Windows

```powershell
# In Developer Command Prompt for Visual Studio 2022:
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```
