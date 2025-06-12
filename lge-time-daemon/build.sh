#!/bin/bash


set -e

echo "=== LGE Time Daemon Build Script ==="

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

print_status() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

BUILD_TYPE=${1:-Release}
print_status "Build type: $BUILD_TYPE"

BUILD_DIR="build-${BUILD_TYPE,,}"
print_status "Creating build directory: $BUILD_DIR"

if [ -d "$BUILD_DIR" ]; then
    print_warning "Build directory exists, cleaning..."
    rm -rf "$BUILD_DIR"
fi

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

print_status "Configuring with CMake..."
cmake -DCMAKE_BUILD_TYPE="$BUILD_TYPE" ..

print_status "Building project..."
make -j$(nproc)

if [ "$BUILD_TYPE" = "Debug" ] || [ "$BUILD_TYPE" = "debug" ]; then
    print_status "Running unit tests..."
    if make test; then
        print_status "All tests passed!"
    else
        print_error "Some tests failed!"
        exit 1
    fi
    
    if command -v lcov &> /dev/null; then
        print_status "Generating code coverage report..."
        make coverage
        print_status "Coverage report generated in coverage_html/"
    else
        print_warning "lcov not found, skipping coverage report"
    fi
else
    print_status "Release build completed, skipping tests"
fi

print_status "Build completed successfully!"
print_status "Executable: $BUILD_DIR/lge-time"

echo ""
echo "=== Installation Instructions ==="
echo "sudo make install                 # Install to system"
echo "sudo systemctl enable lge-time    # Enable service"
echo "sudo systemctl start lge-time     # Start service"
echo ""
echo "=== Usage ==="
echo "./lge-time                        # Run directly"
echo "sudo journalctl -u lge-time -f    # View logs"
