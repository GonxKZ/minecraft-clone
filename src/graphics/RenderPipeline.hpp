/**
 * @file RenderPipeline.hpp
 * @brief VoxelCraft Advanced Render Pipeline - Forward, Deferred, Forward+
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines the RenderPipeline class that provides comprehensive render
 * pipeline management for the VoxelCraft game engine, including forward rendering,
 * deferred shading, forward+, tiled rendering, and hybrid approaches with support
 * for modern graphics features, performance optimization, and multi-GPU rendering.
 */

#ifndef VOXELCRAFT_GRAPHICS_RENDER_PIPELINE_HPP
#define VOXELCRAFT_GRAPHICS_RENDER_PIPELINE_HPP

#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <chrono>
#include <optional>
#include <queue>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../core/Config.hpp"

namespace VoxelCraft {

    // Forward declarations
    class Renderer;
    class ShaderManager;
    class MaterialSystem;
    class PostProcess;
    class RenderTarget;
    class Camera;
    class World;
    class EntityManager;

    /**
     * @enum RenderPassType
     * @brief Types of render passes in the pipeline
     */
    enum class RenderPassType {
        DepthPrePass,            ///< Depth pre-pass for optimization
        ShadowMap,               ///< Shadow map generation
        GBuffer,                 ///< G-buffer generation (deferred)
        Lighting,                ///< Lighting pass
        ForwardOpaque,           ///< Forward opaque rendering
        ForwardTransparent,      ///< Forward transparent rendering
        PostProcess,             ///< Post-processing effects
        UI,                      ///< UI rendering
        Debug,                   ///< Debug rendering
        Custom                   ///< Custom render pass
    };

    /**
     * @enum PipelineStage
     * @brief Pipeline execution stages
     */
    enum class PipelineStage {
        PreRender,               ///< Pre-rendering setup
        ShadowRender,            ///< Shadow rendering
        DepthRender,             ///< Depth rendering
        GBufferRender,           ///< G-buffer rendering
        LightingRender,          ///< Lighting rendering
        OpaqueRender,            ///< Opaque object rendering
        TransparentRender,       ///< Transparent object rendering
        PostRender,              ///< Post-rendering effects
        UIRender,                ///< UI rendering
        FinalRender              ///< Final composition
    };

    /**
     * @enum RenderQueue
     * @brief Render queues for different material types
     */
    enum class RenderQueue {
        Background = 1000,       ///< Background objects
        Geometry = 2000,         ///< Main geometry
        AlphaTest = 2450,        ///< Alpha-tested geometry
        Transparent = 3000,      ///< Transparent objects
        Overlay = 4000           ///< Overlay objects
    };

    /**
     * @enum LightCullingMethod
     * @brief Light culling methods for tiled rendering
     */
    enum class LightCullingMethod {
        None,                    ///< No light culling
        Frustum,                 ///< Frustum-based culling
        Tiled,                   ///< Tiled light culling
        Clustered,               ///< Clustered light culling
        Hybrid                   ///< Hybrid approach
    };

    /**
     * @enum RenderOptimization
     * @brief Rendering optimization techniques
     */
    enum class RenderOptimization {
        None,                    ///< No optimization
        Instancing,              ///< GPU instancing
        MultiDrawIndirect,       ///< Multi-draw indirect
        GPUCulling,              ///< GPU-based culling
        VisibilityBuffer,        ///< Visibility buffer
        MeshShaders              ///< Mesh shader pipeline
    };

    /**
     * @struct RenderPass
     * @brief Individual render pass configuration
     */
    struct RenderPass {
        uint32_t passId;                             ///< Unique pass ID
        std::string name;                            ///< Pass name
        RenderPassType type;                         ///< Pass type
        uint32_t renderTarget;                       ///< Target render target
        glm::vec4 clearColor;                        ///< Clear color
        float clearDepth;                            ///< Clear depth value
        uint32_t clearStencil;                       ///< Clear stencil value
        bool enableDepthTest;                        ///< Enable depth testing
        bool enableDepthWrite;                       ///< Enable depth writing
        bool enableBlending;                         ///< Enable blending
        bool enableCulling;                          ///< Enable face culling
        uint32_t cullFace;                           ///< Face culling mode
        uint32_t shaderProgram;                      ///< Associated shader program

        // Pass-specific parameters
        std::unordered_map<std::string, std::any> parameters; ///< Custom parameters

        // Performance data
        double lastExecutionTime;                    ///< Last execution time (ms)
        uint32_t drawCalls;                          ///< Number of draw calls
        uint32_t triangles;                          ///< Number of triangles rendered

        RenderPass()
            : passId(0)
            , type(RenderPassType::Custom)
            , renderTarget(0)
            , clearColor(0.0f, 0.0f, 0.0f, 1.0f)
            , clearDepth(1.0f)
            , clearStencil(0)
            , enableDepthTest(true)
            , enableDepthWrite(true)
            , enableBlending(false)
            , enableCulling(true)
            , cullFace(0x0405) // GL_BACK
            , shaderProgram(0)
            , lastExecutionTime(0.0)
            , drawCalls(0)
            , triangles(0)
        {}
    };

    /**
     * @struct PipelineConfig
     * @brief Render pipeline configuration
     */
    struct PipelineConfig {
        // Basic settings
        std::string pipelineName;                    ///< Pipeline name
        RenderMode renderMode;                       ///< Rendering mode
        int targetWidth;                            ///< Target render width
        int targetHeight;                           ///< Target render height
        bool enableHDR;                             ///< Enable HDR rendering
        bool enableMSAA;                            ///< Enable MSAA
        int msaaSamples;                            ///< MSAA sample count

        // Advanced rendering
        bool enableDeferred;                        ///< Enable deferred rendering
        bool enableForwardPlus;                     ///< Enable forward+ rendering
        bool enableTiledRendering;                  ///< Enable tiled rendering
        LightCullingMethod lightCulling;            ///< Light culling method
        int tileSize;                               ///< Tile size for tiled rendering

        // Optimization settings
        bool enableDepthPrepass;                    ///< Enable depth pre-pass
        bool enableOcclusionCulling;                ///< Enable occlusion culling
        bool enableFrustumCulling;                  ///< Enable frustum culling
        RenderOptimization optimization;             ///< Render optimization method
        bool enableAsyncCompute;                    ///< Enable async compute
        bool enableMultiGPU;                        ///< Enable multi-GPU rendering

        // Shadow settings
        bool enableShadows;                         ///< Enable shadow rendering
        ShadowQuality shadowQuality;                ///< Shadow quality
        int shadowMapSize;                          ///< Shadow map resolution
        int maxShadowDistance;                      ///< Maximum shadow distance
        int cascadeCount;                           ///< Shadow cascade count

        // Post-processing
        bool enablePostProcess;                     ///< Enable post-processing
        bool enableBloom;                           ///< Enable bloom effect
        bool enableMotionBlur;                      ///< Enable motion blur
        bool enableDOF;                             ///< Enable depth of field
        bool enableSSR;                             ///< Enable screen space reflections
        bool enableSSAO;                            ///< Enable screen space ambient occlusion

        // Quality settings
        float lodBias;                              ///< Level of detail bias
        float renderScale;                          ///< Render scale factor
        int maxDrawDistance;                        ///< Maximum draw distance
        bool enableAnisotropicFiltering;            ///< Enable anisotropic filtering
        float anisotropicLevel;                     ///< Anisotropic filtering level

        // Performance settings
        bool enableRenderDoc;                       ///< Enable RenderDoc integration
        bool enableGPUProfiling;                    ///< Enable GPU profiling
        int maxFrameLatency;                        ///< Maximum frame latency
        bool enableVSync;                           ///< Enable vertical sync
        bool enableTripleBuffering;                 ///< Enable triple buffering

        PipelineConfig()
            : pipelineName("VoxelCraft Pipeline")
            , renderMode(RenderMode::Deferred)
            , targetWidth(1920)
            , targetHeight(1080)
            , enableHDR(true)
            , enableMSAA(true)
            , msaaSamples(4)
            , enableDeferred(true)
            , enableForwardPlus(false)
            , enableTiledRendering(true)
            , lightCulling(LightCullingMethod::Tiled)
            , tileSize(16)
            , enableDepthPrepass(true)
            , enableOcclusionCulling(true)
            , enableFrustumCulling(true)
            , optimization(RenderOptimization::Instancing)
            , enableAsyncCompute(false)
            , enableMultiGPU(false)
            , enableShadows(true)
            , shadowQuality(ShadowQuality::High)
            , shadowMapSize(2048)
            , maxShadowDistance(500)
            , cascadeCount(4)
            , enablePostProcess(true)
            , enableBloom(true)
            , enableMotionBlur(true)
            , enableDOF(true)
            , enableSSR(true)
            , enableSSAO(true)
            , lodBias(0.0f)
            , renderScale(1.0f)
            , maxDrawDistance(1000)
            , enableAnisotropicFiltering(true)
            , anisotropicLevel(16.0f)
            , enableRenderDoc(false)
            , enableGPUProfiling(false)
            , maxFrameLatency(2)
            , enableVSync(false)
            , enableTripleBuffering(true)
        {}
    };

    /**
     * @struct PipelineStats
     * @brief Render pipeline performance statistics
     */
    struct PipelineStats {
        // Frame timing
        double totalFrameTime;                       ///< Total frame render time (ms)
        double averageFrameTime;                     ///< Average frame time (ms)
        double minFrameTime;                         ///< Minimum frame time (ms)
        double maxFrameTime;                         ///< Maximum frame time (ms)
        int fps;                                     ///< Current frames per second

        // Pass timing
        std::unordered_map<RenderPassType, double> passTimes; ///< Time per render pass
        std::unordered_map<RenderPassType, uint32_t> passDrawCalls; ///< Draw calls per pass
        std::unordered_map<RenderPassType, uint32_t> passTriangles; ///< Triangles per pass

        // GPU resources
        size_t renderTargetMemory;                   ///< Render target memory usage
        size_t textureMemory;                        ///< Texture memory usage
        size_t bufferMemory;                         ///< Buffer memory usage
        uint32_t activeRenderTargets;                ///< Number of active render targets

        // Pipeline efficiency
        float cullingEfficiency;                     ///< Culling efficiency (0-1)
        float occlusionCullingEfficiency;            ///< Occlusion culling efficiency
        float shadowMapEfficiency;                   ///< Shadow map rendering efficiency
        float instancingEfficiency;                  ///< Instancing efficiency

        // Quality metrics
        float averageRenderScale;                    ///< Average render scale
        float averageMSAAQuality;                    ///< Average MSAA quality
        float shadowMapCoverage;                     ///< Shadow map coverage percentage
        float lodTransitionQuality;                  ///< LOD transition quality

        // Performance bottlenecks
        std::string bottleneckPass;                  ///< Current bottleneck pass
        float bottleneckPercentage;                  ///< Bottleneck percentage
        std::vector<std::string> performanceWarnings; ///< Performance warnings

        // Advanced metrics
        uint32_t gpuMemoryTransfers;                 ///< GPU memory transfers
        uint32_t shaderSwitches;                     ///< Shader program switches
        uint32_t textureBinds;                       ///< Texture bindings
        uint32_t bufferUploads;                      ///< Buffer data uploads
    };

    /**
     * @struct RenderCommandBuffer
     * @brief Command buffer for render commands
     */
    struct RenderCommandBuffer {
        std::vector<RenderCommand> commands;         ///< Render commands
        uint32_t commandCount;                       ///< Number of commands
        size_t memoryUsage;                          ///< Memory usage
        bool isSorted;                              ///< Commands are sorted

        RenderCommandBuffer()
            : commandCount(0)
            , memoryUsage(0)
            , isSorted(false)
        {}

        void Clear() {
            commands.clear();
            commandCount = 0;
            memoryUsage = 0;
            isSorted = false;
        }

        void SortByQueue() {
            std::sort(commands.begin(), commands.end(),
                     [](const RenderCommand& a, const RenderCommand& b) {
                         // Sort by render queue, then by material, then by distance
                         return std::tie(a.sortKey, a.type) < std::tie(b.sortKey, b.type);
                     });
            isSorted = true;
        }
    };

    /**
     * @class RenderPipeline
     * @brief Advanced render pipeline manager
     *
     * The RenderPipeline class provides comprehensive render pipeline management for the
     * VoxelCraft game engine, featuring multiple rendering architectures (forward, deferred,
     * forward+, tiled), advanced optimization techniques, and support for modern graphics
     * features including ray tracing, async compute, and multi-GPU rendering.
     *
     * Key Features:
     * - Multiple rendering architectures (Forward, Deferred, Forward+, Tiled)
     * - Advanced render pass management and optimization
     * - GPU-driven rendering pipeline with command buffers
     * - Light culling and management (tiled, clustered, frustum)
     * - Shadow rendering with cascaded shadow maps
     * - Render target management and multi-pass rendering
     * - Performance profiling and bottleneck detection
     * - Quality scaling and LOD management
     * - Async compute integration
     * - Multi-GPU rendering support
     * - Render graph construction and execution
     * - Dynamic pipeline reconfiguration
     *
     * The render pipeline is designed to be highly configurable and extensible,
     * supporting everything from simple forward rendering to complex deferred
     * shading with advanced post-processing and GPU optimization techniques.
     */
    class RenderPipeline {
    public:
        /**
         * @brief Constructor
         * @param config Pipeline configuration
         */
        explicit RenderPipeline(const PipelineConfig& config);

        /**
         * @brief Destructor
         */
        ~RenderPipeline();

        /**
         * @brief Deleted copy constructor
         */
        RenderPipeline(const RenderPipeline&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        RenderPipeline& operator=(const RenderPipeline&) = delete;

        // Pipeline lifecycle

        /**
         * @brief Initialize render pipeline
         * @param renderer Renderer instance
         * @return true if successful, false otherwise
         */
        bool Initialize(Renderer* renderer);

        /**
         * @brief Shutdown render pipeline
         */
        void Shutdown();

        /**
         * @brief Update render pipeline
         * @param deltaTime Time elapsed since last update
         */
        void Update(double deltaTime);

        /**
         * @brief Execute render pipeline
         * @return true if successful, false otherwise
         */
        bool Execute();

        /**
         * @brief Get pipeline configuration
         * @return Current configuration
         */
        const PipelineConfig& GetConfig() const { return m_config; }

        /**
         * @brief Set pipeline configuration
         * @param config New configuration
         * @return true if successful, false otherwise
         */
        bool SetConfig(const PipelineConfig& config);

        // Render pass management

        /**
         * @brief Add render pass to pipeline
         * @param pass Render pass configuration
         * @return Pass ID or 0 if failed
         */
        uint32_t AddRenderPass(const RenderPass& pass);

        /**
         * @brief Remove render pass from pipeline
         * @param passId Pass ID
         * @return true if successful, false otherwise
         */
        bool RemoveRenderPass(uint32_t passId);

        /**
         * @brief Get render pass
         * @param passId Pass ID
         * @return Render pass or nullopt if not found
         */
        std::optional<RenderPass> GetRenderPass(uint32_t passId) const;

        /**
         * @brief Set render pass configuration
         * @param passId Pass ID
         * @param pass New pass configuration
         * @return true if successful, false otherwise
         */
        bool SetRenderPass(uint32_t passId, const RenderPass& pass);

        /**
         * @brief Enable render pass
         * @param passId Pass ID
         * @param enabled Enable state
         * @return true if successful, false otherwise
         */
        bool EnableRenderPass(uint32_t passId, bool enabled);

        /**
         * @brief Get render pass execution order
         * @return Vector of pass IDs in execution order
         */
        std::vector<uint32_t> GetRenderPassOrder() const;

        /**
         * @brief Set render pass execution order
         * @param passOrder Vector of pass IDs
         * @return true if successful, false otherwise
         */
        bool SetRenderPassOrder(const std::vector<uint32_t>& passOrder);

        // Command buffer management

        /**
         * @brief Begin command buffer recording
         * @return true if successful, false otherwise
         */
        bool BeginCommandBuffer();

        /**
         * @brief End command buffer recording
         * @return true if successful, false otherwise
         */
        bool EndCommandBuffer();

        /**
         * @brief Submit render command
         * @param command Render command to submit
         * @return true if successful, false otherwise
         */
        bool SubmitRenderCommand(const RenderCommand& command);

        /**
         * @brief Execute command buffer
         * @return true if successful, false otherwise
         */
        bool ExecuteCommandBuffer();

        /**
         * @brief Clear command buffer
         */
        void ClearCommandBuffer();

        /**
         * @brief Get command buffer statistics
         * @return Command buffer stats
         */
        std::unordered_map<std::string, size_t> GetCommandBufferStats() const;

        // Render target management

        /**
         * @brief Create render target
         * @param name Target name
         * @param width Target width
         * @param height Target height
         * @param format Color format
         * @param hasDepth Has depth buffer
         * @param hasStencil Has stencil buffer
         * @return Target ID or 0 if failed
         */
        uint32_t CreateRenderTarget(const std::string& name, int width, int height,
                                   uint32_t format = 0x1908, bool hasDepth = true,
                                   bool hasStencil = false);

        /**
         * @brief Get render target
         * @param targetId Target ID
         * @return Target configuration or nullopt
         */
        std::optional<RenderTargetConfig> GetRenderTarget(uint32_t targetId) const;

        /**
         * @brief Set active render target
         * @param targetId Target ID
         * @return true if successful, false otherwise
         */
        bool SetActiveRenderTarget(uint32_t targetId);

        /**
         * @brief Resize render targets
         * @param width New width
         * @param height New height
         * @return true if successful, false otherwise
         */
        bool ResizeRenderTargets(int width, int height);

        // Shadow rendering

        /**
         * @brief Enable shadow rendering
         * @param enabled Enable state
         * @return true if successful, false otherwise
         */
        bool EnableShadows(bool enabled);

        /**
         * @brief Set shadow quality
         * @param quality Shadow quality
         * @return true if successful, false otherwise
         */
        bool SetShadowQuality(ShadowQuality quality);

        /**
         * @brief Get shadow map texture
         * @param cascadeIndex Cascade index
         * @return Shadow map texture ID
         */
        uint32_t GetShadowMapTexture(int cascadeIndex = 0) const;

        /**
         * @brief Set shadow cascade distances
         * @param distances Cascade distances
         * @return true if successful, false otherwise
         */
        bool SetShadowCascadeDistances(const std::vector<float>& distances);

        // Lighting management

        /**
         * @brief Set light culling method
         * @param method Light culling method
         * @return true if successful, false otherwise
         */
        bool SetLightCullingMethod(LightCullingMethod method);

        /**
         * @brief Get visible lights count
         * @param viewFrustum View frustum for culling
         * @return Number of visible lights
         */
        uint32_t GetVisibleLightsCount(const glm::mat4& viewFrustum) const;

        /**
         * @brief Update light culling
         * @param camera Current camera
         * @return true if successful, false otherwise
         */
        bool UpdateLightCulling(Camera* camera);

        // Pipeline optimization

        /**
         * @brief Optimize pipeline for current hardware
         * @return true if successful, false otherwise
         */
        bool OptimizeForHardware();

        /**
         * @brief Set render optimization method
         * @param optimization Optimization method
         * @return true if successful, false otherwise
         */
        bool SetRenderOptimization(RenderOptimization optimization);

        /**
         * @brief Enable async compute
         * @param enabled Enable state
         * @return true if successful, false otherwise
         */
        bool EnableAsyncCompute(bool enabled);

        /**
         * @brief Set render scale
         * @param scale Render scale (0.1 - 2.0)
         * @return true if successful, false otherwise
         */
        bool SetRenderScale(float scale);

        // Performance monitoring

        /**
         * @brief Get pipeline statistics
         * @return Performance statistics
         */
        const PipelineStats& GetStats() const { return m_stats; }

        /**
         * @brief Reset statistics
         */
        void ResetStats();

        /**
         * @brief Get performance report
         * @return Detailed performance report
         */
        std::string GetPerformanceReport() const;

        /**
         * @brief Get bottleneck analysis
         * @return Bottleneck analysis report
         */
        std::string GetBottleneckAnalysis() const;

        // Debug and visualization

        /**
         * @brief Enable debug visualization
         * @param debugType Debug visualization type
         * @param enabled Enable state
         */
        void SetDebugVisualization(const std::string& debugType, bool enabled);

        /**
         * @brief Capture render pass screenshot
         * @param passId Render pass ID
         * @param filename Output filename
         * @return true if successful, false otherwise
         */
        bool CaptureRenderPass(uint32_t passId, const std::string& filename);

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

        // Utility functions

        /**
         * @brief Validate render pipeline
         * @return true if valid, false otherwise
         */
        bool Validate() const;

        /**
         * @brief Get pipeline status report
         * @return Detailed status report
         */
        std::string GetStatusReport() const;

        /**
         * @brief Get supported render modes
         * @return Vector of supported modes
         */
        std::vector<RenderMode> GetSupportedModes() const;

        /**
         * @brief Check feature support
         * @param feature Feature name
         * @return true if supported, false otherwise
         */
        bool IsFeatureSupported(const std::string& feature) const;

        /**
         * @brief Get pipeline memory usage
         * @return Memory usage in bytes
         */
        size_t GetMemoryUsage() const;

        /**
         * @brief Optimize render pipeline
         * @return Optimization results
         */
        std::unordered_map<std::string, size_t> Optimize();

    private:
        /**
         * @brief Initialize render pipeline
         * @return true if successful, false otherwise
         */
        bool InitializePipeline();

        /**
         * @brief Initialize default render passes
         */
        void InitializeDefaultPasses();

        /**
         * @brief Execute depth pre-pass
         * @return true if successful, false otherwise
         */
        bool ExecuteDepthPrePass();

        /**
         * @brief Execute shadow rendering
         * @return true if successful, false otherwise
         */
        bool ExecuteShadowRendering();

        /**
         * @brief Execute G-buffer pass
         * @return true if successful, false otherwise
         */
        bool ExecuteGBufferPass();

        /**
         * @brief Execute lighting pass
         * @return true if successful, false otherwise
         */
        bool ExecuteLightingPass();

        /**
         * @brief Execute forward rendering
         * @return true if successful, false otherwise
         */
        bool ExecuteForwardRendering();

        /**
         * @brief Execute post-processing
         * @return true if successful, false otherwise
         */
        bool ExecutePostProcessing();

        /**
         * @brief Execute UI rendering
         * @return true if successful, false otherwise
         */
        bool ExecuteUIRendering();

        /**
         * @brief Sort render commands
         * @param commands Commands to sort
         */
        void SortRenderCommands(std::vector<RenderCommand>& commands);

        /**
         * @brief Update render pass statistics
         * @param passId Pass ID
         * @param executionTime Execution time
         */
        void UpdatePassStats(uint32_t passId, double executionTime);

        /**
         * @brief Detect performance bottlenecks
         */
        void DetectBottlenecks();

        /**
         * @brief Update performance statistics
         * @param deltaTime Time elapsed
         */
        void UpdateStats(double deltaTime);

        /**
         * @brief Handle pipeline errors
         * @param error Error message
         */
        void HandleError(const std::string& error);

        // Render pipeline data
        PipelineConfig m_config;                      ///< Pipeline configuration
        PipelineStats m_stats;                        ///< Performance statistics

        // Core systems
        Renderer* m_renderer;                         ///< Renderer instance
        ShaderManager* m_shaderManager;               ///< Shader manager
        MaterialSystem* m_materialSystem;             ///< Material system
        PostProcess* m_postProcess;                   ///< Post-processing system

        // Render passes
        std::unordered_map<uint32_t, RenderPass> m_renderPasses; ///< Render passes
        std::vector<uint32_t> m_passExecutionOrder;  ///< Pass execution order
        mutable std::shared_mutex m_passesMutex;     ///< Passes synchronization

        // Command buffers
        RenderCommandBuffer m_commandBuffer;         ///< Current command buffer
        std::queue<RenderCommandBuffer> m_pendingBuffers; ///< Pending command buffers
        mutable std::shared_mutex m_commandsMutex;   ///< Commands synchronization

        // Render targets
        std::unordered_map<uint32_t, RenderTarget> m_renderTargets; ///< Render targets
        uint32_t m_activeRenderTarget;               ///< Currently active target
        mutable std::shared_mutex m_targetsMutex;    ///< Targets synchronization

        // Shadow system
        std::vector<uint32_t> m_shadowMaps;          ///< Shadow map textures
        std::vector<float> m_shadowCascadeDistances; ///< Shadow cascade distances
        std::vector<glm::mat4> m_shadowMatrices;     ///< Shadow view-projection matrices
        mutable std::shared_mutex m_shadowsMutex;    ///< Shadows synchronization

        // Light culling
        LightCullingMethod m_lightCullingMethod;     ///< Current light culling method
        std::vector<uint32_t> m_visibleLights;       ///< Currently visible lights
        std::vector<uint32_t> m_lightTiles;          ///< Light tiles for tiled rendering
        mutable std::shared_mutex m_lightsMutex;     ///< Lights synchronization

        // Performance data
        std::unordered_map<std::string, bool> m_debugFlags; ///< Debug visualization flags
        bool m_gpuProfilingActive;                   ///< GPU profiling active
        double m_profilingStartTime;                 ///< Profiling start time

        // State tracking
        bool m_isInitialized;                         ///< Pipeline is initialized
        bool m_recordingCommands;                     ///< Currently recording commands
        double m_lastUpdateTime;                      ///< Last update time
        std::string m_lastError;                      ///< Last error message

        static std::atomic<uint32_t> s_nextPassId;    ///< Next pass ID counter
        static std::atomic<uint32_t> s_nextTargetId;  ///< Next target ID counter
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_GRAPHICS_RENDER_PIPELINE_HPP
