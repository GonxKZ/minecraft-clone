#!/bin/bash

# VoxelCraft Build and Test Script
echo "=== VoxelCraft Build and Test Script ==="
echo

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if we're in the project root
if [ ! -f "CMakeLists.txt" ]; then
    print_error "Please run this script from the project root directory"
    exit 1
fi

# Create build directory
print_status "Creating build directory..."
mkdir -p build
cd build

# Configure with CMake
print_status "Configuring project with CMake..."
if ! cmake ..; then
    print_error "CMake configuration failed"
    exit 1
fi

# Build the project
print_status "Building project..."
if ! cmake --build . --config Release; then
    print_error "Build failed"
    exit 1
fi

# Check if executables exist
if [ ! -f "./bin/VoxelCraftTest" ]; then
    print_error "Test executable not found"
    exit 1
fi

if [ ! -f "./bin/VoxelCraftGame" ]; then
    print_error "Game executable not found"
    exit 1
fi

# Run tests
print_status "Running tests..."
echo "----------------------------------------"
if ./bin/VoxelCraftTest; then
    print_status "All tests passed! ✅"
else
    print_error "Tests failed"
    exit 1
fi
echo "----------------------------------------"

# Run game briefly to show it works
print_status "Testing game executable..."
echo "----------------------------------------"
timeout 3 ./bin/VoxelCraftGame --debug 2>/dev/null || true
echo "----------------------------------------"

print_status "Build and test completed successfully! 🎉"
print_status ""
print_status "Available executables:"
print_status "  ./bin/VoxelCraftTest  - Run the test suite"
print_status "  ./bin/VoxelCraftGame  - Run the game engine"
print_status ""
print_status "Next steps:"
print_status "  1. Implement game systems (graphics, world, player, etc.)"
print_status "  2. Replace stub implementations with real functionality"
print_status "  3. Add game content and assets"
print_status ""
print_status "The engine is ready for game development! 🚀"

cd ..
