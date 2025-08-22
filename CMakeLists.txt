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
option(VOXELCRAFT_BUILD_TESTS "Build unit tests" ON)
option(VOXELCRAFT_BUILD_TOOLS "Build development tools" ON)
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

# OpenGL
find_package(OpenGL REQUIRED)

# GLFW
find_package(glfw3 3.3 REQUIRED)
if(NOT glfw3_FOUND)
    message(FATAL_ERROR "GLFW3 not found. Please install GLFW3.")
endif()

# GLM
find_package(glm REQUIRED)
if(NOT glm_FOUND)
    message(FATAL_ERROR "GLM not found. Please install GLM.")
endif()

# spdlog
find_package(spdlog REQUIRED)
if(NOT spdlog_FOUND)
    message(FATAL_ERROR "spdlog not found. Please install spdlog.")
endif()

# nlohmann_json
find_package(nlohmann_json 3.10 REQUIRED)
if(NOT nlohmann_json_FOUND)
    message(FATAL_ERROR "nlohmann_json not found. Please install nlohmann_json.")
endif()

# Optional: Vulkan
if(VOXELCRAFT_USE_VULKAN)
    find_package(Vulkan REQUIRED)
    if(NOT Vulkan_FOUND)
        message(FATAL_ERROR "Vulkan not found. Install Vulkan SDK or disable VOXELCRAFT_USE_VULKAN.")
    endif()
endif()

# Optional: Boost for networking
find_package(Boost 1.70 COMPONENTS system thread REQUIRED)
if(NOT Boost_FOUND)
    message(FATAL_ERROR "Boost not found. Please install Boost libraries.")
endif()

# Optional: OpenAL for audio
find_package(OpenAL REQUIRED)
if(NOT OpenAL_FOUND)
    message(FATAL_ERROR "OpenAL not found. Please install OpenAL.")
endif()

# =============================================================================
# Source Files
# =============================================================================
# Core engine sources
set(VOXELCRAFT_CORE_SOURCES
    src/core/Application.cpp
    src/core/Engine.cpp
    src/core/Config.cpp
    src/core/Logger.cpp
    src/core/Timer.cpp
    src/core/EventSystem.cpp
    src/core/MemoryManager.cpp
    src/core/ResourceManager.cpp
)

# Engine subsystem sources
set(VOXELCRAFT_ENGINE_SOURCES
    src/engine/graphics/Renderer.cpp
    src/engine/graphics/Shader.cpp
    src/engine/graphics/Material.cpp
    src/engine/graphics/Mesh.cpp
    src/engine/graphics/Texture.cpp
    src/engine/graphics/Light.cpp
    src/engine/graphics/Camera.cpp
    src/engine/physics/PhysicsEngine.cpp
    src/engine/physics/PhysicsWorld.cpp
    src/engine/physics/RigidBody.cpp
    src/engine/physics/CollisionShape.cpp
    src/engine/physics/Constraint.cpp
    src/engine/audio/AudioEngine.cpp
    src/engine/audio/SoundBuffer.cpp
    src/engine/audio/SoundSource.cpp
    src/engine/audio/MusicPlayer.cpp
    src/engine/input/InputManager.cpp
    src/engine/input/Keyboard.cpp
    src/engine/input/Mouse.cpp
    src/engine/input/Gamepad.cpp
)

# World system sources
set(VOXELCRAFT_WORLD_SOURCES
    src/world/World.cpp
    src/world/Chunk.cpp
    src/world/ChunkManager.cpp
    src/world/TerrainGenerator.cpp
    src/world/Biome.cpp
    src/world/NoiseGenerator.cpp
    src/world/LightingEngine.cpp
    src/world/WeatherSystem.cpp
)

# Block system sources
set(VOXELCRAFT_BLOCK_SOURCES
    src/blocks/Block.cpp
    src/blocks/BlockRegistry.cpp
    src/blocks/BlockMeshGenerator.cpp
    src/blocks/TextureAtlas.cpp
    src/blocks/BlockBehavior.cpp
    src/blocks/BlockData.cpp
)

# Entity system sources
set(VOXELCRAFT_ENTITY_SOURCES
    src/entities/Entity.cpp
    src/entities/Component.cpp
    src/entities/System.cpp
    src/entities/EntityManager.cpp
    src/entities/TransformComponent.cpp
    src/entities/RenderComponent.cpp
    src/entities/PhysicsComponent.cpp
    src/entities/ScriptComponent.cpp
)

# Player system sources
set(VOXELCRAFT_PLAYER_SOURCES
    src/player/Player.cpp
    src/player/PlayerController.cpp
    src/player/PlayerCamera.cpp
    src/player/Inventory.cpp
    src/player/PlayerPhysics.cpp
    src/player/PlayerNetwork.cpp
)

# AI system sources
set(VOXELCRAFT_AI_SOURCES
    src/ai/AISystem.cpp
)

# UI system sources
set(VOXELCRAFT_UI_SOURCES
    src/ui/UIManager.cpp
    src/ui/HUD.cpp
    src/ui/Menu.cpp
    src/ui/InventoryUI.cpp
    src/ui/ChatSystem.cpp
    src/ui/Console.cpp
)

# Multiplayer system sources
set(VOXELCRAFT_MULTIPLAYER_SOURCES
    src/multiplayer/NetworkManager.cpp
    src/multiplayer/Client.cpp
    src/multiplayer/Server.cpp
    src/multiplayer/Packet.cpp
    src/multiplayer/Protocol.cpp
    src/multiplayer/Authentication.cpp
)

# Graphics advanced sources
set(VOXELCRAFT_GRAPHICS_SOURCES
    src/graphics/AdvancedRenderer.cpp
    src/graphics/PostProcessor.cpp
    src/graphics/ParticleSystem.cpp
    src/graphics/ShadowMapping.cpp
    src/graphics/DeferredRendering.cpp
    src/graphics/PBREngine.cpp
)

# Audio advanced sources
set(VOXELCRAFT_AUDIO_SOURCES
    src/audio/AdvancedAudioEngine.cpp
    src/audio/SpatialAudio.cpp
    src/audio/ReverbEngine.cpp
    src/audio/AudioMixer.cpp
)

# Save system sources
set(VOXELCRAFT_SAVE_SOURCES
    src/save/SaveManager.cpp
    src/save/Serializer.cpp
    src/save/WorldSerializer.cpp
    src/save/PlayerSerializer.cpp
    src/save/Compression.cpp
)

# Development tools sources
set(VOXELCRAFT_TOOLS_SOURCES
    src/tools/WorldEditor.cpp
    src/tools/Profiler.cpp
    src/tools/Debugger.cpp
    src/tools/Console.cpp
    src/tools/MemoryTracker.cpp
)

# Test sources
set(VOXELCRAFT_TEST_SOURCES
    src/tests/UnitTests.cpp
    src/tests/IntegrationTests.cpp
    src/tests/PerformanceTests.cpp
)

# =============================================================================
# Main Library
# =============================================================================
add_library(VoxelCraft STATIC
    ${VOXELCRAFT_CORE_SOURCES}
    ${VOXELCRAFT_ENGINE_SOURCES}
    ${VOXELCRAFT_WORLD_SOURCES}
    ${VOXELCRAFT_BLOCK_SOURCES}
    ${VOXELCRAFT_ENTITY_SOURCES}
    ${VOXELCRAFT_PLAYER_SOURCES}
    ${VOXELCRAFT_AI_SOURCES}
    ${VOXELCRAFT_UI_SOURCES}
    ${VOXELCRAFT_MULTIPLAYER_SOURCES}
    ${VOXELCRAFT_GRAPHICS_SOURCES}
    ${VOXELCRAFT_AUDIO_SOURCES}
    ${VOXELCRAFT_SAVE_SOURCES}
)

if(VOXELCRAFT_BUILD_TOOLS)
    target_sources(VoxelCraft PRIVATE ${VOXELCRAFT_TOOLS_SOURCES})
endif()

# Include directories
target_include_directories(VoxelCraft
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${OPENGL_INCLUDE_DIR}
        ${Boost_INCLUDE_DIRS}
        ${OPENAL_INCLUDE_DIR}
        external/includes
)

# Link libraries
target_link_libraries(VoxelCraft
    PUBLIC
        Threads::Threads
        glfw
        glm::glm
        spdlog::spdlog
        nlohmann_json::nlohmann_json
        ${OPENGL_LIBRARIES}
        ${Boost_LIBRARIES}
        ${OPENAL_LIBRARY}
)

if(VOXELCRAFT_USE_VULKAN)
    target_link_libraries(VoxelCraft PUBLIC Vulkan::Vulkan)
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
