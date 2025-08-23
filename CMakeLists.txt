# =============================================================================
# VoxelCraft - Minecraft Clone Engine - CMake Configuration
# =============================================================================
# Version: 1.0.0
# Description: Advanced voxel engine with modern C++20 features
# =============================================================================

cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

# =============================================================================
# Project Configuration
# =============================================================================
project(VoxelCraft
    VERSION 1.0.0
    DESCRIPTION "Advanced Minecraft Clone Engine"
    LANGUAGES CXX C
)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# =============================================================================
# Build Options
# =============================================================================
option(VOXELCRAFT_BUILD_TESTS "Build unit tests" OFF)
option(VOXELCRAFT_BUILD_TOOLS "Build development tools" OFF)
option(VOXELCRAFT_BUILD_EXAMPLES "Build example projects" OFF)
option(VOXELCRAFT_ENABLE_PROFILING "Enable performance profiling" OFF)
option(VOXELCRAFT_ENABLE_DEBUG_LOGGING "Enable detailed debug logging" ON)
option(VOXELCRAFT_USE_VULKAN "Use Vulkan instead of OpenGL" OFF)

# =============================================================================
# Compiler Configuration
# =============================================================================
if(MSVC)
    # MSVC specific flags
    add_compile_options(
        /W4          # High warning level
        /permissive- # Standards conformance
        /Zc:__cplusplus # Enable correct __cplusplus macro value
        /MP          # Multi-processor compilation
    )

    # Release optimizations
    add_compile_options($<$<CONFIG:Release>:/O2>)
    add_compile_options($<$<CONFIG:Release>:/DNDEBUG>)

    # Debug options
    add_compile_options($<$<CONFIG:Debug>:/Od>)
    add_compile_options($<$<CONFIG:Debug>:/DDEBUG>)

    # Disable specific warnings
    add_compile_options(/wd4251 /wd4275 /wd4996)

else()
    # GCC/Clang flags
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -Wshadow
        -Wconversion
        -Wsign-conversion
        -Wunused
        -Wuninitialized
    )

    # Release optimizations
    add_compile_options($<$<CONFIG:Release>:-O3>)
    add_compile_options($<$<CONFIG:Release>:-DNDEBUG>)
    add_compile_options($<$<CONFIG:Release>:-march=native>)

    # Debug options
    add_compile_options($<$<CONFIG:Debug>:-O0>)
    add_compile_options($<$<CONFIG:Debug>:-g3>)
    add_compile_options($<$<CONFIG:Debug>:-DDEBUG>)

    # Sanitizers (Debug only)
    if(VOXELCRAFT_ENABLE_PROFILING)
        add_compile_options($<$<CONFIG:Debug>:-fsanitize=address>)
        add_compile_options($<$<CONFIG:Debug>:-fsanitize=undefined>)
        add_link_options($<$<CONFIG:Debug>:-fsanitize=address>)
        add_link_options($<$<CONFIG:Debug>:-fsanitize=undefined>)
    endif()
endif()

# =============================================================================
# Dependencies
# =============================================================================
find_package(Threads REQUIRED)

# Try to find optional dependencies, but don't fail if not found
find_package(OpenGL QUIET)
find_package(glfw3 3.3 QUIET)
find_package(glm QUIET)
find_package(spdlog QUIET)
find_package(nlohmann_json 3.10 QUIET)

# Create simple stub implementations if dependencies are missing
if(NOT spdlog_FOUND)
    message(WARNING "spdlog not found. Using simple logging implementation.")
endif()

if(NOT nlohmann_json_FOUND)
    message(WARNING "nlohmann_json not found. Using simple JSON implementation.")
endif()

# =============================================================================
# Source Files
# =============================================================================
# Core engine sources - only the ones we have implemented
set(VOXELCRAFT_CORE_SOURCES
    src/core/Application.cpp
    src/core/Engine.cpp
    src/core/Config.cpp
    src/core/Logger.cpp
    src/core/Timer.cpp
    src/core/EventSystem.cpp
    src/core/MemoryManager.cpp
    src/core/ResourceManager.cpp
    src/core/GameConstants.hpp
    src/core/PhysicsUtils.hpp
    src/graphics/Renderer.cpp
    src/input/InputManager.cpp
    src/window/Window.cpp
    src/player/Player.cpp
    src/world/World.cpp
    src/world/Chunk.cpp
    src/world/Biome.cpp
    src/world/LightingEngine.cpp
    src/blocks/Block.cpp
    src/blocks/BlockRegistry.cpp
    src/blocks/BlockSystem.cpp
    src/blocks/TextureAtlas.hpp
    src/blocks/BlockMeshGenerator.hpp
    src/blocks/BlockBehavior.hpp
    src/interaction/BlockInteraction.cpp
    src/ui/UIManager.cpp
    src/ui/UISystem.hpp
    src/ui/UISystem.cpp
    src/ui/UIWidgets.hpp
    src/ui/UIInventory.hpp
    src/ui/UIMenus.hpp
    src/ui/UIHUD.hpp
    src/save/SaveManager.hpp
    src/lighting/LightingSystem.hpp
    src/particles/ParticleSystem.hpp
    src/weather/WeatherSystem.hpp
    src/animation/AnimationSystem.hpp
    src/audio/AudioManager.cpp
    src/audio/SoundGenerator.hpp
    src/audio/SoundGenerator.cpp
    src/audio/SpatialAudioSystem.hpp
    src/audio/AudioEffectProcessor.hpp
    src/entities/Entity.cpp
    src/entities/Component.cpp
    src/entities/EntityManager.cpp
    src/entities/System.hpp
    src/entities/System.cpp
    src/entities/TransformComponent.cpp
    src/entities/RenderComponent.hpp
    src/entities/RenderComponent.cpp
    src/tools/Tool.hpp
    src/tools/Tool.cpp
    src/tools/ItemGenerator.hpp
    src/tools/ItemGenerator.cpp
    src/blocks/BlockGenerator.hpp
    src/blocks/BlockGenerator.cpp
    src/world/MobGenerator.hpp
    src/world/MobGenerator.cpp
    src/core/SkinGenerator.hpp
    src/core/SkinGenerator.cpp
    src/core/SoundGenerator.hpp
    src/core/SoundGenerator.cpp
    src/core/ThreadPool.hpp
    src/core/ThreadPool.cpp
    src/core/GameStateSync.hpp
    src/core/GameStateSync.cpp
    src/network/Server.hpp
    src/network/Server.cpp
    src/network/Client.hpp
    src/network/Client.cpp
    src/graphics/Camera.hpp
    src/graphics/Camera.cpp
    src/ui/UIManager.hpp
    src/ui/UIManager.cpp
    src/core/Profiler.hpp
    src/core/Profiler.cpp
    src/crafting/CraftingRecipe.hpp
    src/crafting/CraftingTable.hpp
)

# All core systems are now implemented - no stubs needed

# =============================================================================
# Main Library
# =============================================================================
add_library(VoxelCraft STATIC
    ${VOXELCRAFT_CORE_SOURCES}
)

# Include directories
target_include_directories(VoxelCraft
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:include>
)

# Link libraries - only required ones
target_link_libraries(VoxelCraft
    PUBLIC
        Threads::Threads
)

# Conditionally link optional dependencies
if(spdlog_FOUND)
    target_link_libraries(VoxelCraft PUBLIC spdlog::spdlog)
endif()

if(nlohmann_json_FOUND)
    target_link_libraries(VoxelCraft PUBLIC nlohmann_json::nlohmann_json)
endif()

if(OpenGL_FOUND)
    target_include_directories(VoxelCraft PRIVATE ${OPENGL_INCLUDE_DIR})
    target_link_libraries(VoxelCraft PUBLIC ${OPENGL_LIBRARIES})
endif()

if(glfw3_FOUND)
    target_link_libraries(VoxelCraft PUBLIC glfw)
endif()

if(glm_FOUND)
    target_link_libraries(VoxelCraft PUBLIC glm::glm)
endif()

# Compile definitions
target_compile_definitions(VoxelCraft
    PRIVATE
        $<$<CONFIG:Debug>:VOXELCRAFT_DEBUG>
        $<$<CONFIG:Release>:VOXELCRAFT_RELEASE>
        $<$<BOOL:${VOXELCRAFT_ENABLE_DEBUG_LOGGING}>:VOXELCRAFT_DEBUG_LOGGING>
        $<$<BOOL:${VOXELCRAFT_USE_VULKAN}>:VOXELCRAFT_VULKAN>
        VOXELCRAFT_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
        VOXELCRAFT_VERSION_MINOR=${PROJECT_VERSION_MINOR}
        VOXELCRAFT_VERSION_PATCH=${PROJECT_VERSION_PATCH}
)

# =============================================================================
# Main Executable
# =============================================================================
add_executable(VoxelCraftGame
    src/main.cpp
)

target_link_libraries(VoxelCraftGame
    PRIVATE
        VoxelCraft
)

# =============================================================================
# Tests
# =============================================================================
if(VOXELCRAFT_BUILD_TESTS)
    find_package(GTest REQUIRED)

    add_executable(VoxelCraftTests
        ${VOXELCRAFT_TEST_SOURCES}
    )

    target_link_libraries(VoxelCraftTests
        PRIVATE
            VoxelCraft
            GTest::gtest_main
    )

    # Enable testing
    enable_testing()
    add_test(NAME VoxelCraftUnitTests COMMAND VoxelCraftTests)
endif()

# =============================================================================
# Tools
# =============================================================================
if(VOXELCRAFT_BUILD_TOOLS)
    add_executable(WorldEditor
        src/tools/main/WorldEditorMain.cpp
    )

    add_executable(PerformanceProfiler
        src/tools/main/ProfilerMain.cpp
    )

    target_link_libraries(WorldEditor PRIVATE VoxelCraft)
    target_link_libraries(PerformanceProfiler PRIVATE VoxelCraft)
endif()

# =============================================================================
# Examples
# =============================================================================
if(VOXELCRAFT_BUILD_EXAMPLES)
    add_executable(BasicExample
        examples/basic/main.cpp
    )

    add_executable(AdvancedExample
        examples/advanced/main.cpp
    )

    target_link_libraries(BasicExample PRIVATE VoxelCraft)
    target_link_libraries(AdvancedExample PRIVATE VoxelCraft)
endif()

# =============================================================================
# Installation
# =============================================================================
install(TARGETS VoxelCraft
    EXPORT VoxelCraftTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(DIRECTORY src/
    DESTINATION include/voxelcraft
    FILES_MATCHING PATTERN "*.hpp"
)

install(EXPORT VoxelCraftTargets
    FILE VoxelCraftTargets.cmake
    NAMESPACE VoxelCraft::
    DESTINATION lib/cmake/VoxelCraft
)

# =============================================================================
# CPack Configuration
# =============================================================================
set(CPACK_PACKAGE_NAME "VoxelCraft")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION "Advanced Minecraft Clone Engine")
set(CPACK_PACKAGE_VENDOR "VoxelCraft Team")
set(CPACK_PACKAGE_CONTACT "dev@voxelcraft.dev")

# Platform specific packaging
if(WIN32)
    set(CPACK_GENERATOR "NSIS;ZIP")
    set(CPACK_NSIS_DISPLAY_NAME "VoxelCraft Engine")
    set(CPACK_NSIS_PACKAGE_NAME "VoxelCraft")
    set(CPACK_NSIS_HELP_LINK "https://voxelcraft.dev")
    set(CPACK_NSIS_URL_INFO_ABOUT "https://voxelcraft.dev")
    set(CPACK_NSIS_CONTACT "dev@voxelcraft.dev")
elseif(UNIX)
    set(CPACK_GENERATOR "DEB;TGZ")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "VoxelCraft Team")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libglfw3, libglm-dev, libspdlog-dev")
endif()

include(CPack)

# Test executable removed - no test files available

# =============================================================================
# Summary
# =============================================================================
message(STATUS "================================================================================")
message(STATUS "VoxelCraft Build Configuration Summary")
message(STATUS "================================================================================")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Tests: ${VOXELCRAFT_BUILD_TESTS}")
message(STATUS "Tools: ${VOXELCRAFT_BUILD_TOOLS}")
message(STATUS "Examples: ${VOXELCRAFT_BUILD_EXAMPLES}")
message(STATUS "Profiling: ${VOXELCRAFT_ENABLE_PROFILING}")
message(STATUS "Debug Logging: ${VOXELCRAFT_ENABLE_DEBUG_LOGGING}")
message(STATUS "Vulkan: ${VOXELCRAFT_USE_VULKAN}")
message(STATUS "================================================================================")
