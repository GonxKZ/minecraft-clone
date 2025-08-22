/**
 * @file Renderer.hpp
 * @brief VoxelCraft Advanced Graphics Renderer
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines the Renderer class that provides advanced graphics rendering
 * capabilities for the VoxelCraft game engine, including deferred shading,
 * PBR materials, advanced lighting, post-processing effects, and GPU optimization
 * techniques for modern graphics pipelines.
 */

#ifndef VOXELCRAFT_GRAPHICS_RENDERER_HPP
#define VOXELCRAFT_GRAPHICS_RENDERER_HPP

#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <chrono>
#include <queue>
#include <optional>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../core/Config.hpp"
#include "../core/System.hpp"

namespace VoxelCraft {

    // Forward declarations
    class Window;
    class Camera;
    class World;
    class EntityManager;
    class Player;
    class ShaderManager;
    class MaterialSystem;
    class PostProcess;
    class ParticleSystem;
    class LightingSystem;
    class TextureManager;
    class RenderPipeline;
    class RenderTarget;
    class Mesh;
    class Model;
    class RenderCommand;

    /**
     * @enum RenderAPI
     * @brief Supported rendering APIs
     */
    enum class RenderAPI {
        OpenGL,                 ///< OpenGL 4.6+
        Vulkan,                 ///< Vulkan 1.3+
        DirectX12,              ///< DirectX 12
        Metal,                  ///< Apple Metal
        Auto                    ///< Automatic selection
    };

    /**
     * @enum RenderMode
     * @brief Rendering modes
     */
    enum class RenderMode {
        Forward,                ///< Forward rendering
        Deferred,               ///< Deferred shading
        TiledDeferred,          ///< Tiled deferred rendering
        ForwardPlus,            ///< Forward+ rendering
        Hybrid                  ///< Hybrid approach
    };

    /**
     * @enum RenderQuality
     * @brief Rendering quality levels
     */
    enum class RenderQuality {
        Low,                    ///< Low quality for performance
        Medium,                 ///< Medium quality balance
        High,                   ///< High quality for modern GPUs
        Ultra,                  ///< Ultra quality with advanced features
        Custom                  ///< Custom quality settings
    };

    /**
     * @enum AntiAliasingMode
     * @brief Anti-aliasing techniques
     */
    enum class AntiAliasingMode {
        None,                   ///< No anti-aliasing
        MSAA,                   ///< Multi-Sample Anti-Aliasing
        FXAA,                   ///< Fast Approximate Anti-Aliasing
        TAA,                    ///< Temporal Anti-Aliasing
        SMAA,                   ///< Subpixel Morphological Anti-Aliasing
        DLSS,                   ///< NVIDIA DLSS (if available)
        FSR                     ///< AMD FSR (if available)
    };

    /**
     * @enum ShadowQuality
     * @brief Shadow rendering quality
     */
    enum class ShadowQuality {
        Low,                    ///< Basic shadows
        Medium,                 ///< Cascaded shadow maps
        High,                   ///< PCF with cascaded shadows
        Ultra,                  ///< VSM/PCSS with advanced filtering
        RayTraced               ///< Ray traced shadows (if supported)
    };

    /**
     * @struct RenderConfig
     * @brief Renderer configuration settings
     */
    struct RenderConfig {
        // Basic settings
        RenderAPI api;                          ///< Rendering API
        RenderMode mode;                        ///< Rendering mode
        RenderQuality quality;                  ///< Overall quality level
        bool enableVSync;                       ///< Enable vertical sync
        int targetFPS;                          ///< Target frame rate

        // Display settings
        int screenWidth;                        ///< Screen width
        int screenHeight;                       ///< Screen height
        bool fullscreen;                        ///< Fullscreen mode
        bool enableHDR;                         ///< Enable HDR rendering
        float renderScale;                      ///< Render scale (for supersampling)

        // Advanced rendering
        AntiAliasingMode antiAliasing;          ///< Anti-aliasing mode
        int msaaSamples;                        ///< MSAA sample count
        ShadowQuality shadowQuality;            ///< Shadow quality
        bool enableSSAO;                        ///< Screen Space Ambient Occlusion
        bool enableSSR;                         ///< Screen Space Reflections
        bool enableGI;                          ///< Global Illumination
        bool enableRayTracing;                  ///< Hardware ray tracing

        // Performance settings
        bool enableCulling;                     ///< Enable frustum culling
        bool enableOcclusionCulling;            ///< Enable occlusion culling
        bool enableLOD;                         ///< Enable Level of Detail
        int maxDrawDistance;                    ///< Maximum draw distance
        int shadowDistance;                     ///< Shadow rendering distance

        // Post-processing
        bool enableBloom;                       ///< Enable bloom effect
        bool enableMotionBlur;                  ///< Enable motion blur
        bool enableDOF;                         ///< Enable depth of field
        bool enableChromaticAberration;         ///< Enable chromatic aberration
        bool enableVignette;                    ///< Enable vignette
        bool enableTonemapping;                 ///< Enable tone mapping

        // Debug settings
        bool enableWireframe;                   ///< Enable wireframe mode
        bool showNormals;                       ///< Show normal vectors
        bool showBoundingBoxes;                 ///< Show bounding boxes
        bool showRenderStats;                   ///< Show rendering statistics
        bool enableProfiling;                   ///< Enable GPU profiling

        // Advanced settings
        bool enableAsyncCompute;                ///< Enable async compute
        bool enableMultiGPU;                    ///< Enable multi-GPU support
        int gpuMemoryBudget;                    ///< GPU memory budget (MB)
        bool enableShaderCache;                 ///< Enable shader caching
        std::string shaderCachePath;            ///< Shader cache directory

        RenderConfig()
            : api(RenderAPI::Auto)
            , mode(RenderMode::Deferred)
            , quality(RenderQuality::High)
            , enableVSync(true)
            , targetFPS(60)
            , screenWidth(1920)
            , screenHeight(1080)
            , fullscreen(false)
            , enableHDR(false)
            , renderScale(1.0f)
            , antiAliasing(AntiAliasingMode::TAA)
            , msaaSamples(4)
            , shadowQuality(ShadowQuality::High)
            , enableSSAO(true)
            , enableSSR(true)
            , enableGI(false)
            , enableRayTracing(false)
            , enableCulling(true)
            , enableOcclusionCulling(true)
            , enableLOD(true)
            , maxDrawDistance(1000)
            , shadowDistance(500)
            , enableBloom(true)
            , enableMotionBlur(true)
            , enableDOF(true)
            , enableChromaticAberration(false)
            , enableVignette(true)
            , enableTonemapping(true)
            , enableWireframe(false)
            , showNormals(false)
            , showBoundingBoxes(false)
            , showRenderStats(false)
            , enableProfiling(false)
            , enableAsyncCompute(false)
            , enableMultiGPU(false)
            , gpuMemoryBudget(2048)
            , enableShaderCache(true)
            , shaderCachePath("cache/shaders")
        {}
    };

    /**
     * @struct RenderStats
     * @brief Rendering performance statistics
     */
    struct RenderStats {
        // Frame timing
        double frameTime;                       ///< Time to render frame (ms)
        double gpuTime;                         ///< GPU rendering time (ms)
        int fps;                                ///< Current frames per second
        int averageFPS;                         ///< Average FPS over time

        // Draw calls
        int totalDrawCalls;                     ///< Total draw calls per frame
        int opaqueDrawCalls;                    ///< Opaque object draw calls
        int transparentDrawCalls;               ///< Transparent object draw calls
        int shadowDrawCalls;                    ///< Shadow map draw calls

        // Geometry
        int totalTriangles;                     ///< Total triangles rendered
        int totalVertices;                      ///< Total vertices processed
        int visibleObjects;                     ///< Number of visible objects
        int culledObjects;                      ///< Number of culled objects

        // Memory usage
        size_t gpuMemoryUsed;                   ///< GPU memory usage (bytes)
        size_t textureMemory;                   ///< Texture memory usage
        size_t vertexBufferMemory;              ///< Vertex buffer memory
        size_t indexBufferMemory;               ///< Index buffer memory

        // Pipeline statistics
        int shaderSwitches;                     ///< Number of shader switches
        int textureBinds;                       ///< Number of texture bindings
        int uniformUpdates;                     ///< Number of uniform updates
        int stateChanges;                       ///< Number of render state changes

        // Advanced features
        int rayTracingCalls;                    ///< Ray tracing invocations
        int computeDispatches;                  ///< Compute shader dispatches
        int asyncComputeTime;                   ///< Async compute time (ms)
        int multiGPUSyncTime;                   ///< Multi-GPU sync time (ms)

        // Quality metrics
        float averageSSAO;                      ///< Average SSAO contribution
        float averageSSR;                       ///< Average SSR contribution
        float shadowMapCoverage;                ///< Shadow map coverage percentage
        float lodTransitionDistance;            ///< LOD transition distance
    };

    /**
     * @struct RenderCommand
     * @brief Rendering command structure
     */
    struct RenderCommand {
        enum class Type {
            DrawMesh,                           ///< Draw mesh
            DrawModel,                          ///< Draw model
            DrawParticles,                      ///< Draw particle system
            DrawUI,                             ///< Draw UI elements
            UpdateMaterial,                     ///< Update material properties
            SetRenderTarget,                    ///< Set render target
            Clear,                              ///< Clear render target
            Present,                            ///< Present frame
            Custom                              ///< Custom command
        };

        Type type;                              ///< Command type
        uint32_t sortKey;                       ///< Sorting key for command ordering
        std::any data;                          ///< Command-specific data
        std::function<void()> callback;         ///< Optional callback

        RenderCommand()
            : type(Type::Custom)
            , sortKey(0)
        {}
    };

    /**
     * @struct RenderQueue
     * @brief Rendering command queue
     */
    struct RenderQueue {
        std::vector<RenderCommand> opaqueCommands;    ///< Opaque rendering commands
        std::vector<RenderCommand> transparentCommands; ///< Transparent rendering commands
        std::vector<RenderCommand> postProcessCommands; ///< Post-processing commands
        std::vector<RenderCommand> uiCommands;         ///< UI rendering commands

        void Clear() {
            opaqueCommands.clear();
            transparentCommands.clear();
            postProcessCommands.clear();
            uiCommands.clear();
        }

        size_t TotalCommands() const {
            return opaqueCommands.size() + transparentCommands.size() +
                   postProcessCommands.size() + uiCommands.size();
        }
    };

    /**
     * @class Renderer
     * @brief Advanced graphics renderer for VoxelCraft
     *
     * The Renderer class provides comprehensive graphics rendering capabilities
     * for the VoxelCraft game engine, featuring deferred shading, PBR materials,
     * advanced lighting, post-processing effects, and GPU optimization techniques
     * for modern graphics pipelines with support for ray tracing and async compute.
     *
     * Key Features:
     * - Multiple rendering APIs (OpenGL, Vulkan, DirectX12, Metal)
     * - Advanced rendering pipelines (Forward, Deferred, Forward+)
     * - Physically Based Rendering (PBR) with material system
     * - Advanced lighting with shadows, GI, and reflections
     * - Post-processing effects (bloom, motion blur, HDR, tonemapping)
     * - Particle systems with GPU acceleration
     * - Level of Detail (LOD) and occlusion culling
     * - Shader management with hot-reloading
     * - GPU profiling and performance optimization
     * - Multi-GPU support and async compute
     * - Ray tracing integration (hardware accelerated)
     *
     * The renderer is designed to be highly configurable and extensible,
     * supporting both high-end gaming PCs and lower-end hardware through
     * quality scaling and performance optimization features.
     */
    class Renderer : public System {
    public:
        /**
         * @brief Constructor
         * @param config Renderer configuration
         */
        explicit Renderer(const RenderConfig& config);

        /**
         * @brief Destructor
         */
        ~Renderer();

        /**
         * @brief Deleted copy constructor
         */
        Renderer(const Renderer&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        Renderer& operator=(const Renderer&) = delete;

        // Renderer lifecycle

        /**
         * @brief Initialize renderer
         * @param window Game window
         * @return true if successful, false otherwise
         */
        bool Initialize(Window* window);

        /**
         * @brief Shutdown renderer
         */
        void Shutdown();

        /**
         * @brief Begin frame rendering
         * @return true if successful, false otherwise
         */
        bool BeginFrame();

        /**
         * @brief End frame rendering and present
         * @return true if successful, false otherwise
         */
        bool EndFrame();

        /**
         * @brief Render current frame
         * @param deltaTime Time elapsed since last frame
         */
        void RenderFrame(double deltaTime);

        /**
         * @brief Get renderer configuration
         * @return Current configuration
         */
        const RenderConfig& GetConfig() const { return m_config; }

        /**
         * @brief Set renderer configuration
         * @param config New configuration
         * @return true if successful, false otherwise
         */
        bool SetConfig(const RenderConfig& config);

        /**
         * @brief Get rendering statistics
         * @return Current render statistics
         */
        const RenderStats& GetStats() const { return m_stats; }

        // Rendering commands

        /**
         * @brief Submit render command
         * @param command Render command to submit
         */
        void SubmitCommand(const RenderCommand& command);

        /**
         * @brief Submit mesh for rendering
         * @param mesh Mesh to render
         * @param transform World transformation matrix
         * @param material Material to use
         */
        void SubmitMesh(std::shared_ptr<Mesh> mesh, const glm::mat4& transform,
                       std::shared_ptr<Material> material);

        /**
         * @brief Submit model for rendering
         * @param model Model to render
         * @param transform World transformation matrix
         */
        void SubmitModel(std::shared_ptr<Model> model, const glm::mat4& transform);

        /**
         * @brief Submit particle system for rendering
         * @param particleSystem Particle system to render
         */
        void SubmitParticles(std::shared_ptr<ParticleSystem> particleSystem);

        // Camera and view management

        /**
         * @brief Set active camera
         * @param camera Camera to use for rendering
         */
        void SetCamera(Camera* camera);

        /**
         * @brief Get active camera
         * @return Current camera
         */
        Camera* GetCamera() const { return m_camera; }

        /**
         * @brief Set view-projection matrix
         * @param viewProj Combined view-projection matrix
         */
        void SetViewProjection(const glm::mat4& viewProj);

        /**
         * @brief Get view-projection matrix
         * @return Current view-projection matrix
         */
        const glm::mat4& GetViewProjection() const { return m_viewProjection; }

        // Lighting and environment

        /**
         * @brief Set ambient lighting
         * @param color Ambient color
         * @param intensity Ambient intensity
         */
        void SetAmbientLight(const glm::vec3& color, float intensity);

        /**
         * @brief Add directional light
         * @param direction Light direction
         * @param color Light color
         * @param intensity Light intensity
         * @return Light ID
         */
        uint32_t AddDirectionalLight(const glm::vec3& direction,
                                    const glm::vec3& color, float intensity);

        /**
         * @brief Add point light
         * @param position Light position
         * @param color Light color
         * @param intensity Light intensity
         * @param range Light range
         * @return Light ID
         */
        uint32_t AddPointLight(const glm::vec3& position,
                              const glm::vec3& color, float intensity, float range);

        /**
         * @brief Add spot light
         * @param position Light position
         * @param direction Light direction
         * @param color Light color
         * @param intensity Light intensity
         * @param range Light range
         * @param angle Spotlight angle
         * @return Light ID
         */
        uint32_t AddSpotLight(const glm::vec3& position, const glm::vec3& direction,
                             const glm::vec3& color, float intensity, float range, float angle);

        /**
         * @brief Remove light
         * @param lightId Light ID
         * @return true if successful, false otherwise
         */
        bool RemoveLight(uint32_t lightId);

        // Material and shader management

        /**
         * @brief Create material
         * @param name Material name
         * @param shaderName Shader to use
         * @return Material instance
         */
        std::shared_ptr<Material> CreateMaterial(const std::string& name,
                                                const std::string& shaderName);

        /**
         * @brief Get material by name
         * @param name Material name
         * @return Material instance or nullptr
         */
        std::shared_ptr<Material> GetMaterial(const std::string& name) const;

        /**
         * @brief Load shader
         * @param name Shader name
         * @param vertexSource Vertex shader source
         * @param fragmentSource Fragment shader source
         * @param geometrySource Geometry shader source (optional)
         * @return Shader ID or 0 if failed
         */
        uint32_t LoadShader(const std::string& name,
                           const std::string& vertexSource,
                           const std::string& fragmentSource,
                           const std::string& geometrySource = "");

        /**
         * @brief Get shader by name
         * @param name Shader name
         * @return Shader ID or 0 if not found
         */
        uint32_t GetShader(const std::string& name) const;

        // Render target management

        /**
         * @brief Create render target
         * @param width Target width
         * @param height Target height
         * @param format Color format
         * @param hasDepth Has depth buffer
         * @param hasStencil Has stencil buffer
         * @return Render target ID
         */
        uint32_t CreateRenderTarget(int width, int height,
                                   uint32_t format = 0x1908, // RGBA
                                   bool hasDepth = true,
                                   bool hasStencil = false);

        /**
         * @brief Set active render target
         * @param targetId Render target ID (0 for default framebuffer)
         */
        void SetRenderTarget(uint32_t targetId);

        /**
         * @brief Get render target texture
         * @param targetId Render target ID
         * @return Texture ID or 0 if not found
         */
        uint32_t GetRenderTargetTexture(uint32_t targetId) const;

        // Post-processing effects

        /**
         * @brief Enable post-processing effect
         * @param effectName Effect name
         * @param enabled Enable state
         * @return true if successful, false otherwise
         */
        bool EnablePostEffect(const std::string& effectName, bool enabled);

        /**
         * @brief Set post-processing effect parameter
         * @param effectName Effect name
         * @param parameterName Parameter name
         * @param value Parameter value
         * @return true if successful, false otherwise
         */
        bool SetPostEffectParameter(const std::string& effectName,
                                   const std::string& parameterName,
                                   const std::any& value);

        /**
         * @brief Add custom post-processing effect
         * @param effect Post-processing effect
         * @return Effect ID or 0 if failed
         */
        uint32_t AddPostEffect(std::shared_ptr<PostProcess> effect);

        // Particle system integration

        /**
         * @brief Create particle system
         * @param config Particle system configuration
         * @return Particle system instance
         */
        std::shared_ptr<ParticleSystem> CreateParticleSystem(const ParticleConfig& config);

        /**
         * @brief Update particle systems
         * @param deltaTime Time elapsed
         */
        void UpdateParticles(double deltaTime);

        // Debug and visualization

        /**
         * @brief Enable debug visualization
         * @param type Debug visualization type
         * @param enabled Enable state
         */
        void SetDebugVisualization(const std::string& type, bool enabled);

        /**
         * @brief Capture screenshot
         * @param filename Screenshot filename
         * @return true if successful, false otherwise
         */
        bool CaptureScreenshot(const std::string& filename);

        /**
         * @brief Start GPU profiling
         * @return true if successful, false otherwise
         */
        bool StartGPUProfiling();

        /**
         * @brief Stop GPU profiling
         * @return Profiling results
         */
        std::unordered_map<std::string, double> StopGPUProfiling();

        // Performance and optimization

        /**
         * @brief Get GPU memory usage
         * @return Memory usage in bytes
         */
        size_t GetGPUMemoryUsage() const;

        /**
         * @brief Get GPU memory budget
         * @return Memory budget in bytes
         */
        size_t GetGPUMemoryBudget() const { return m_config.gpuMemoryBudget * 1024 * 1024; }

        /**
         * @brief Optimize rendering for current hardware
         * @return Optimization results
         */
        std::unordered_map<std::string, size_t> OptimizeForHardware();

        /**
         * @brief Check feature support
         * @param feature Feature name
         * @return true if supported, false otherwise
         */
        bool IsFeatureSupported(const std::string& feature) const;

        // Utility functions

        /**
         * @brief Get renderer API name
         * @return API name string
         */
        std::string GetAPIName() const;

        /**
         * @brief Get renderer capabilities
         * @return Capability flags
         */
        uint32_t GetCapabilities() const;

        /**
         * @brief Get supported render modes
         * @return Vector of supported render modes
         */
        std::vector<RenderMode> GetSupportedModes() const;

        /**
         * @brief Validate renderer state
         * @return true if valid, false otherwise
         */
        bool Validate() const;

        /**
         * @brief Get renderer status report
         * @return Detailed status report
         */
        std::string GetStatusReport() const;

        /**
         * @brief Get performance report
         * @return Detailed performance report
         */
        std::string GetPerformanceReport() const;

    private:
        /**
         * @brief Initialize rendering API
         * @return true if successful, false otherwise
         */
        bool InitializeAPI();

        /**
         * @brief Initialize render pipeline
         * @return true if successful, false otherwise
         */
        bool InitializePipeline();

        /**
         * @brief Initialize subsystems
         * @return true if successful, false otherwise
         */
        bool InitializeSubsystems();

        /**
         * @brief Render geometry pass
         */
        void RenderGeometryPass();

        /**
         * @brief Render shadow maps
         */
        void RenderShadowPass();

        /**
         * @brief Render lighting pass
         */
        void RenderLightingPass();

        /**
         * @brief Render transparent objects
         */
        void RenderTransparentPass();

        /**
         * @brief Render post-processing effects
         */
        void RenderPostProcessPass();

        /**
         * @brief Render UI elements
         */
        void RenderUIPass();

        /**
         * @brief Execute render commands
         * @param commands Render commands to execute
         */
        void ExecuteRenderCommands(const std::vector<RenderCommand>& commands);

        /**
         * @brief Sort render commands
         * @param commands Commands to sort
         */
        void SortRenderCommands(std::vector<RenderCommand>& commands);

        /**
         * @brief Update render statistics
         * @param deltaTime Time elapsed
         */
        void UpdateStats(double deltaTime);

        /**
         * @brief Handle renderer errors
         * @param error Error message
         */
        void HandleError(const std::string& error);

        // Renderer data
        RenderConfig m_config;                         ///< Renderer configuration
        RenderStats m_stats;                           ///< Performance statistics

        // Core systems
        Window* m_window;                              ///< Game window
        Camera* m_camera;                              ///< Active camera
        World* m_world;                                ///< Game world

        // Rendering systems
        std::unique_ptr<ShaderManager> m_shaderManager;    ///< Shader management
        std::unique_ptr<MaterialSystem> m_materialSystem;  ///< Material system
        std::unique_ptr<LightingSystem> m_lightingSystem;  ///< Lighting system
        std::unique_ptr<TextureManager> m_textureManager;  ///< Texture management
        std::unique_ptr<RenderPipeline> m_renderPipeline;  ///< Render pipeline

        // Post-processing
        std::vector<std::shared_ptr<PostProcess>> m_postEffects; ///< Post-processing effects
        std::unordered_map<std::string, bool> m_enabledEffects; ///< Enabled effects

        // Particle systems
        std::vector<std::shared_ptr<ParticleSystem>> m_particleSystems; ///< Active particle systems

        // Render targets
        std::unordered_map<uint32_t, std::shared_ptr<RenderTarget>> m_renderTargets; ///< Render targets
        uint32_t m_currentRenderTarget;                 ///< Current render target

        // Render queues
        RenderQueue m_renderQueue;                      ///< Current render queue
        mutable std::shared_mutex m_queueMutex;         ///< Queue synchronization

        // Materials and shaders
        std::unordered_map<std::string, std::shared_ptr<Material>> m_materials; ///< Loaded materials
        std::unordered_map<std::string, uint32_t> m_shaders; ///< Loaded shaders
        mutable std::shared_mutex m_resourcesMutex;      ///< Resources synchronization

        // Lighting
        glm::vec3 m_ambientColor;                       ///< Ambient light color
        float m_ambientIntensity;                        ///< Ambient light intensity
        std::unordered_map<uint32_t, DirectionalLight> m_directionalLights; ///< Directional lights
        std::unordered_map<uint32_t, PointLight> m_pointLights; ///< Point lights
        std::unordered_map<uint32_t, SpotLight> m_spotLights; ///< Spot lights
        mutable std::shared_mutex m_lightingMutex;       ///< Lighting synchronization

        // View and projection
        glm::mat4 m_viewProjection;                      ///< Combined view-projection matrix
        glm::mat4 m_viewMatrix;                          ///< View matrix
        glm::mat4 m_projectionMatrix;                    ///< Projection matrix

        // State tracking
        bool m_isInitialized;                            ///< Renderer is initialized
        double m_frameStartTime;                         ///< Frame start time
        double m_lastFrameTime;                          ///< Last frame time
        uint32_t m_frameNumber;                          ///< Current frame number

        // Debug and profiling
        std::unordered_map<std::string, bool> m_debugFlags; ///< Debug visualization flags
        bool m_gpuProfilingActive;                       ///< GPU profiling active
        double m_profilingStartTime;                     ///< Profiling start time

        static std::atomic<uint32_t> s_nextRenderTargetId; ///< Next render target ID
        static std::atomic<uint32_t> s_nextLightId;       ///< Next light ID
        static std::atomic<uint32_t> s_nextMaterialId;    ///< Next material ID
        static std::atomic<uint32_t> s_nextShaderId;      ///< Next shader ID
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_GRAPHICS_RENDERER_HPP
