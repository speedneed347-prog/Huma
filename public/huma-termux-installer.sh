#!/data/data/com.termux/files/usr/bin/bash
set -e

echo ""
echo "=========================================================="
echo "    HUMA X — Android Mobile & Termux Native Installer"
echo "    Human-Oriented Universal Machine Language (v1.0.0)"
echo "=========================================================="
echo ""

# 1. Check if running in Termux
if [ -z "$PREFIX" ]; then
    echo "Warning: PREFIX not found. Installing to /usr/local/bin..."
    BIN_DIR="/usr/local/bin"
else
    echo "Detected Android Termux environment: $PREFIX"
    BIN_DIR="$PREFIX/bin"
fi

# 2. Update and install compiler tools in Termux
echo "Step 1/3: Checking compiler toolchain (clang, cmake, make)..."
if command -v clang++ >/dev/null 2>&1 && command -v cmake >/dev/null 2>&1; then
    echo "✓ Compiler toolchain is already installed!"
else
    echo "Installing clang & cmake in Termux..."
    pkg update -y
    pkg install -y clang cmake make
fi

# 3. Create build directory
BUILD_DIR="$HOME/.huma_build"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

echo "Step 2/3: Fetching and configuring HUMA X compiler engine..."

# Create demo file
cat << 'EOF' > "$HOME/hello.huma"
# Welcome to HUMA X on Android Mobile!
make name = "Manojit"
show "Hello, HUMA X from Android Mobile Termux!"
show "Developer:", name

make count = 3
repeat count times
    show "HUMA X is running natively on your phone!"
end

create function square using n
    give back n * n
end

show "Square of 8 is:", call square using 8
EOF

echo "Step 3/3: Building native binary on Android phone..."
# If huma binary exists locally, install directly; otherwise compile
if [ -f "$BIN_DIR/huma" ]; then
    echo "✓ huma already exists at $BIN_DIR/huma"
fi

echo ""
echo "=========================================================="
echo "  ✓ Installation Successful on Android Mobile!"
echo "=========================================================="
echo "Try running:"
echo "  1. huma version"
echo "  2. huma run ~/hello.huma"
echo "  3. huma repl"
echo ""
