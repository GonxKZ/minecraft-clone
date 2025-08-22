/**
 * @file Renderer.hpp
 * @brief VoxelCraft Graphics System - OpenGL Renderer
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_GRAPHICS_RENDERER_HPP
#define VOXELCRAFT_GRAPHICS_RENDERER_HPP

#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <functional>

#include "../window/Window.hpp"
#include "../blocks/Block.hpp"

namespace VoxelCraft {

    // Forward declarations
    class Shader;
    class Camera;
    class Mesh;
    struct Vec3;
    struct Vec4;
    struct Mat4;

    /**
     * @enum RenderMode
     * @brief Rendering modes for different types of objects
     */
    enum class RenderMode {
        SOLID,           // Solid geometry (blocks, terrain)
        TRANSPARENT,     // Transparent objects (water, glass)
        PARTICLES,       // Particle effects
        UI,              // User interface elements
        DEBUG            // Debug visualization
    };

    /**
     * @enum RenderPass
     * @brief Render passes for multi-pass rendering
     */
    enum class RenderPass {
        DEPTH_PREPASS,   // Depth-only pass for optimization
        GEOMETRY,        // Main geometry pass
        TRANSPARENT,     // Transparent objects
        POST_PROCESS,    // Post-processing effects
        UI               // UI rendering
    };

    /**
     * @struct RenderStats
     * @brief Rendering performance statistics
     */
    struct RenderStats {
        int drawCalls;           // Number of draw calls per frame
        int vertices;            // Number of vertices rendered
        int triangles;           // Number of triangles rendered
        float renderTime;        // Time spent rendering (ms)
        int visibleChunks;       // Number of visible chunks
        int visibleEntities;     // Number of visible entities
        int fps;                 // Current FPS
    };

    /**
     * @struct RenderCommand
     * @brief A single render command
     */
    struct RenderCommand {
        RenderMode mode;
        Mat4 transform;
        Vec3 position;
        Vec4 color;
        std::shared_ptr<Mesh> mesh;
        std::shared_ptr<Shader> shader;
        float distance;          // Distance from camera (for sorting)

        RenderCommand()
            : mode(RenderMode::SOLID)
            , color(1.0f, 1.0f, 1.0f, 1.0f)
            , distance(0.0f)
        {}
    };

    /**
     * @class Renderer
     * @brief Main OpenGL renderer for VoxelCraft
     */
    class Renderer {
    public:
        /**
         * @brief Constructor
         * @param window Window to render to
         */
        Renderer(std::shared_ptr<Window> window);

        /**
         * @brief Destructor
         */
        ~Renderer();

        /**
         * @brief Initialize the renderer
         * @return true if successful
         */
        bool Initialize();

        /**
         * @brief Shutdown the renderer
         */
        void Shutdown();

        /**
         * @brief Begin a new frame
         */
        void BeginFrame();

        /**
         * @brief End the current frame
         */
        void EndFrame();

        /**
         * @brief Render all queued commands
         */
        void Render();

        /**
         * @brief Clear the screen
         * @param color Clear color
         */
        void Clear(const Vec4& color = Vec4(0.2f, 0.3f, 0.8f, 1.0f));

        /**
         * @brief Submit a render command
         * @param command Render command to submit
         */
        void SubmitCommand(const RenderCommand& command);

        /**
         * @brief Set the camera for rendering
         * @param camera Camera to use
         */
        void SetCamera(std::shared_ptr<Camera> camera);

        /**
         * @brief Get the current camera
         * @return Current camera
         */
        std::shared_ptr<Camera> GetCamera() const { return m_camera; }

        /**
         * @brief Get render statistics
         * @return Render statistics
         */
        const RenderStats& GetStats() const { return m_stats; }

        /**
         * @brief Set render mode
         * @param mode Render mode
         */
        void SetRenderMode(RenderMode mode) { m_currentMode = mode; }

        /**
         * @brief Enable/disable wireframe mode
         * @param enabled Enable wireframe
         */
        void SetWireframeMode(bool enabled);

        /**
         * @brief Enable/disable VSync
         * @param enabled Enable VSync
         */
        void SetVSync(bool enabled);

        /**
         * @brief Create a basic cube mesh for testing
         * @param size Cube size
         * @return Cube mesh
         */
        std::shared_ptr<Mesh> CreateCubeMesh(float size = 1.0f);

        /**
         * @brief Create a block mesh
         * @param blockType Block type
         * @return Block mesh
         */
        std::shared_ptr<Mesh> CreateBlockMesh(BlockType blockType);

        /**
         * @brief Load a shader
         * @param name Shader name
         * @param vertexSource Vertex shader source
         * @param fragmentSource Fragment shader source
         * @return Shader instance
         */
        std::shared_ptr<Shader> LoadShader(const std::string& name,
                                          const std::string& vertexSource,
                                          const std::string& fragmentSource);

    private:
        std::shared_ptr<Window> m_window;
        std::shared_ptr<Camera> m_camera;
        std::vector<RenderCommand> m_renderQueue;
        RenderStats m_stats;
        RenderMode m_currentMode;
        bool m_initialized;
        bool m_wireframeMode;

        // OpenGL resources
        unsigned int m_vao;
        unsigned int m_vbo;
        unsigned int m_ebo;

        // Default shaders
        std::shared_ptr<Shader> m_defaultShader;
        std::shared_ptr<Shader> m_blockShader;

        // Mesh cache
        std::unordered_map<BlockType, std::shared_ptr<Mesh>> m_blockMeshes;
        std::unordered_map<std::string, std::shared_ptr<Mesh>> m_meshCache;

        /**
         * @brief Initialize OpenGL settings
         */
        void InitializeOpenGL();

        /**
         * @brief Initialize default shaders
         */
        void InitializeShaders();

        /**
         * @brief Setup vertex buffers
         */
        void SetupBuffers();

        /**
         * @brief Render solid geometry
         */
        void RenderSolidPass();

        /**
         * @brief Render transparent geometry
         */
        void RenderTransparentPass();

        /**
         * @brief Render UI elements
         */
        void RenderUIPass();

        /**
         * @brief Sort render commands by distance
         */
        void SortRenderCommands();

        /**
         * @brief Update render statistics
         */
        void UpdateStats();

        /**
         * @brief Get default vertex shader source
         * @return Shader source code
         */
        std::string GetDefaultVertexShader();

        /**
         * @brief Get default fragment shader source
         * @return Shader source code
         */
        std::string GetDefaultFragmentShader();

        /**
         * @brief Get block shader source
         * @return Shader source code
         */
        std::string GetBlockVertexShader();
        std::string GetBlockFragmentShader();
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_GRAPHICS_RENDERER_HPP