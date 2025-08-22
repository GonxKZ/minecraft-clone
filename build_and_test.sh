#!/bin/bash

# VoxelCraft Build and Test Script - Enhanced Version
echo "=== VoxelCraft Build and Test Script ==="
echo

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[1;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

# Parse command line arguments
BUILD_TYPE="Release"
CLEAN_BUILD=false
RUN_TESTS=false
ENABLE_PROFILING=false
VERBOSE=false

while [[ $# -gt 0 ]]; do
    case $1 in
        -d|--debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        -r|--release)
            BUILD_TYPE="Release"
            shift
            ;;
        -c|--clean)
            CLEAN_BUILD=true
            shift
            ;;
        -t|--test)
            RUN_TESTS=true
            shift
            ;;
        -p|--profile)
            ENABLE_PROFILING=true
            shift
            ;;
        -v|--verbose)
            VERBOSE=true
            shift
            ;;
        -h|--help)
            echo "Usage: $0 [OPTIONS]"
            echo "Options:"
            echo "  -d, --debug           Build in debug mode"
            echo "  -r, --release         Build in release mode (default)"
            echo "  -c, --clean           Clean build directory before building"
            echo "  -t, --test            Run tests after building"
            echo "  -p, --profile         Enable profiling"
            echo "  -v, --verbose         Verbose output"
            echo "  -h, --help            Show this help message"
            exit 0
            ;;
        *)
            print_error "Unknown option: $1"
            exit 1
            ;;
    esac
done

# Check if we're in the project root
if [ ! -f "CMakeLists.txt" ]; then
    print_error "Please run this script from the project root directory"
    exit 1
fi

# Clean build directory if requested
if [ "$CLEAN_BUILD" = true ]; then
    print_status "Cleaning build directory..."
    rm -rf build
fi

# Create build directory
print_status "Creating build directory..."
mkdir -p build
cd build

# Configure with CMake
print_status "Configuring project with CMake..."
CMAKE_FLAGS="-DCMAKE_BUILD_TYPE=$BUILD_TYPE"

if [ "$ENABLE_PROFILING" = true ]; then
    CMAKE_FLAGS="$CMAKE_FLAGS -DVOXELCRAFT_ENABLE_PROFILING=ON"
fi

if [ "$VERBOSE" = true ]; then
    CMAKE_FLAGS="$CMAKE_FLAGS -DCMAKE_VERBOSE_MAKEFILE=ON"
fi

if ! cmake .. $CMAKE_FLAGS; then
    print_error "CMake configuration failed"
    exit 1
fi

# Build the project
print_status "Building project in $BUILD_TYPE mode..."
if [ "$VERBOSE" = true ]; then
    if ! cmake --build . --config $BUILD_TYPE; then
        print_error "Build failed"
        exit 1
    fi
else
    if ! cmake --build . --config $BUILD_TYPE > build.log 2>&1; then
        print_error "Build failed"
        if [ -f "build.log" ]; then
            print_info "Build log (last 20 lines):"
            tail -20 build.log
        fi
        exit 1
    fi
fi

# Check if executables exist
GAME_EXECUTABLE="./bin/VoxelCraftGame"
TEST_EXECUTABLE="./bin/VoxelCraftTest"

if [ ! -f "$GAME_EXECUTABLE" ]; then
    print_warning "Game executable not found at $GAME_EXECUTABLE"
else
    print_success "Game executable found: $GAME_EXECUTABLE"
    EXEC_SIZE=$(du -h "$GAME_EXECUTABLE" | cut -f1)
    print_info "Executable size: $EXEC_SIZE"
fi

# Run tests if requested and executable exists
if [ "$RUN_TESTS" = true ]; then
    if [ -f "$TEST_EXECUTABLE" ]; then
        print_status "Running tests..."
        echo "----------------------------------------"
        if ./bin/VoxelCraftTest; then
            print_success "All tests passed! ✅"
        else
            print_error "Tests failed"
            exit 1
        fi
        echo "----------------------------------------"
    else
        print_warning "Test executable not found at $TEST_EXECUTABLE"
    fi
fi

# Test game executable briefly if it exists
if [ -f "$GAME_EXECUTABLE" ]; then
    print_status "Testing game executable..."
    echo "----------------------------------------"
    timeout 3 ./bin/VoxelCraftGame --help 2>/dev/null || true
    echo "----------------------------------------"
fi

print_success "Build and test completed successfully! 🎉"
print_info ""
print_info "Build Information:"
print_info "  Build Type: $BUILD_TYPE"
print_info "  Build Directory: $(pwd)"
print_info "  Source Directory: $(dirname $(pwd))"

if [ "$ENABLE_PROFILING" = true ]; then
    print_info "  Profiling: Enabled"
else
    print_info "  Profiling: Disabled"
fi

print_info ""
print_info "System Information:"
print_info "  CPU Cores: $(nproc)"
print_info "  Memory: $(free -h | grep '^Mem:' | awk '{print $2}')"
print_info "  Disk Space: $(df -h . | tail -1 | awk '{print $4}') available"

print_info ""
print_info "Available executables:"
if [ -f "$GAME_EXECUTABLE" ]; then
    print_info "  $GAME_EXECUTABLE  - Run the Minecraft-like game engine"
fi
if [ -f "$TEST_EXECUTABLE" ]; then
    print_info "  $TEST_EXECUTABLE  - Run the test suite"
fi

print_info ""
print_info "Next steps:"
print_info "  1. Implement remaining game systems (graphics, world generation)"
print_info "  2. Add game content and assets (models, textures, sounds)"
print_info "  3. Test gameplay mechanics"
print_info "  4. Optimize performance"
print_info ""
print_info "The engine is ready for game development! 🚀"
print_info "Run '$GAME_EXECUTABLE --help' to see available options"

cd ..
