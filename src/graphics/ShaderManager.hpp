/**
 * @file ShaderManager.hpp
 * @brief VoxelCraft Advanced Shader Management System
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines the ShaderManager class that provides comprehensive shader
 * management for the VoxelCraft game engine, including PBR shaders, advanced
 * lighting, post-processing effects, and GPU optimization with hot-reloading
 * and shader caching capabilities.
 */

#ifndef VOXELCRAFT_GRAPHICS_SHADER_MANAGER_HPP
#define VOXELCRAFT_GRAPHICS_SHADER_MANAGER_HPP

#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <chrono>
#include <filesystem>
#include <optional>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../core/Config.hpp"

namespace VoxelCraft {

    // Forward declarations
    class Renderer;
    class Material;

    /**
     * @enum ShaderType
     * @brief Types of shaders supported
     */
    enum class ShaderType {
        Vertex,                 ///< Vertex shader
        Fragment,               ///< Fragment shader
        Geometry,               ///< Geometry shader
        TessellationControl,    ///< Tessellation control shader
        TessellationEvaluation, ///< Tessellation evaluation shader
        Compute,                ///< Compute shader
        RayGen,                 ///< Ray generation shader (for ray tracing)
        Miss,                   ///< Miss shader (for ray tracing)
        ClosestHit,             ///< Closest hit shader (for ray tracing)
        AnyHit                  ///< Any hit shader (for ray tracing)
    };

    /**
     * @enum ShaderStage
     * @brief Shader pipeline stages
     */
    enum class ShaderStage {
        PreVertex,              ///< Pre-vertex processing
        Vertex,                 ///< Vertex processing
        Tessellation,           ///< Tessellation stage
        Geometry,               ///< Geometry processing
        Fragment,               ///< Fragment processing
        PostProcess,            ///< Post-processing
        Compute,                ///< Compute operations
        RayTracing              ///< Ray tracing operations
    };

    /**
     * @enum ShaderQuality
     * @brief Shader quality levels
     */
    enum class ShaderQuality {
        Low,                    ///< Low quality shaders
        Medium,                 ///< Medium quality shaders
        High,                   ///< High quality shaders
        Ultra,                  ///< Ultra quality shaders
        Custom                  ///< Custom quality shaders
    };

    /**
     * @enum PBRWorkflow
     * @brief Physically Based Rendering workflows
     */
    enum class PBRWorkflow {
        MetallicRoughness,      ///< Metallic-roughness workflow
        SpecularGlossiness,     ///< Specular-glossiness workflow
        Hybrid                  ///< Hybrid workflow
    };

    /**
     * @struct ShaderConfig
     * @brief Shader configuration settings
     */
    struct ShaderConfig {
        // Basic settings
        std::string shaderName;                     ///< Shader name/identifier
        ShaderQuality quality;                      ///< Shader quality level
        PBRWorkflow pbrWorkflow;                    ///< PBR workflow

        // Feature flags
        bool enablePBR;                             ///< Enable Physically Based Rendering
        bool enableNormalMapping;                   ///< Enable normal mapping
        bool enableParallaxMapping;                 ///< Enable parallax mapping
        bool enableDisplacementMapping;             ///< Enable displacement mapping
        bool enableAmbientOcclusion;                ///< Enable ambient occlusion
        bool enableReflections;                     ///< Enable reflections
        bool enableRefractions;                     ///< Enable refractions
        bool enableSubsurfaceScattering;            ///< Enable subsurface scattering
        bool enableAnisotropic;                     ///< Enable anisotropic materials
        bool enableClearCoat;                       ///< Enable clear coat materials
        bool enableSheen;                           ///< Enable sheen materials
        bool enableTransmission;                    ///< Enable transmission
        bool enableVolume;                          ///< Enable volumetric rendering
        bool enableRayTracing;                      ///< Enable ray tracing features

        // Performance settings
        bool enableInstancing;                      ///< Enable GPU instancing
        bool enableTessellation;                    ///< Enable tessellation
        bool enableComputeShaders;                  ///< Enable compute shader features
        int maxLights;                              ///< Maximum number of lights
        int maxTextures;                            ///< Maximum texture units
        bool enableShaderCache;                     ///< Enable shader caching

        // Debug settings
        bool enableDebugOutput;                     ///< Enable debug output
        bool enablePerformanceCounters;             ///< Enable performance counters
        bool enableShaderValidation;                ///< Enable shader validation

        ShaderConfig()
            : quality(ShaderQuality::High)
            , pbrWorkflow(PBRWorkflow::MetallicRoughness)
            , enablePBR(true)
            , enableNormalMapping(true)
            , enableParallaxMapping(true)
            , enableDisplacementMapping(false)
            , enableAmbientOcclusion(true)
            , enableReflections(true)
            , enableRefractions(false)
            , enableSubsurfaceScattering(false)
            , enableAnisotropic(false)
            , enableClearCoat(false)
            , enableSheen(false)
            , enableTransmission(false)
            , enableVolume(false)
            , enableRayTracing(false)
            , enableInstancing(true)
            , enableTessellation(false)
            , enableComputeShaders(true)
            , maxLights(8)
            , maxTextures(16)
            , enableShaderCache(true)
            , enableDebugOutput(false)
            , enablePerformanceCounters(false)
            , enableShaderValidation(true)
        {}
    };

    /**
     * @struct ShaderSource
     * @brief Shader source code structure
     */
    struct ShaderSource {
        std::string vertexSource;                   ///< Vertex shader source
        std::string fragmentSource;                 ///< Fragment shader source
        std::string geometrySource;                 ///< Geometry shader source
        std::string tessControlSource;              ///< Tessellation control source
        std::string tessEvalSource;                 ///< Tessellation evaluation source
        std::string computeSource;                  ///< Compute shader source

        // Ray tracing shaders
        std::string rayGenSource;                   ///< Ray generation source
        std::string missSource;                     ///< Miss shader source
        std::string closestHitSource;               ///< Closest hit source
        std::string anyHitSource;                   ///< Any hit source

        std::unordered_map<std::string, std::string> includes; ///< Include files
        std::unordered_map<std::string, std::any> defines;     ///< Preprocessor defines

        ShaderSource() = default;
    };

    /**
     * @struct ShaderProgram
     * @brief Compiled shader program
     */
    struct ShaderProgram {
        uint32_t programId;                         ///< OpenGL program ID
        std::string name;                           ///< Program name
        ShaderConfig config;                        ///< Program configuration

        // Shader IDs
        uint32_t vertexShader;                      ///< Vertex shader ID
        uint32_t fragmentShader;                    ///< Fragment shader ID
        uint32_t geometryShader;                    ///< Geometry shader ID
        uint32_t tessControlShader;                 ///< Tessellation control ID
        uint32_t tessEvalShader;                    ///< Tessellation evaluation ID
        uint32_t computeShader;                     ///< Compute shader ID

        // Uniform locations cache
        std::unordered_map<std::string, int> uniformLocations; ///< Cached uniform locations
        std::unordered_map<std::string, int> attributeLocations; ///< Cached attribute locations

        // Program metadata
        double compileTime;                         ///< Compilation time (ms)
        double linkTime;                            ///< Link time (ms)
        size_t programSize;                         ///< Program size (bytes)
        std::string hash;                           ///< Program hash for caching

        ShaderProgram()
            : programId(0)
            , vertexShader(0)
            , fragmentShader(0)
            , geometryShader(0)
            , tessControlShader(0)
            , tessEvalShader(0)
            , computeShader(0)
            , compileTime(0.0)
            , linkTime(0.0)
            , programSize(0)
        {}
    };

    /**
     * @struct ShaderUniform
     * @brief Shader uniform information
     */
    struct ShaderUniform {
        std::string name;                           ///< Uniform name
        uint32_t type;                              ///< Uniform type (GL enum)
        int location;                               ///< Uniform location
        int size;                                   ///< Uniform size
        std::any defaultValue;                      ///< Default value
        std::string description;                    ///< Uniform description

        ShaderUniform()
            : type(0)
            , location(-1)
            , size(0)
        {}
    };

    /**
     * @struct ShaderStats
     * @brief Shader performance statistics
     */
    struct ShaderStats {
        // Compilation stats
        uint64_t shadersCompiled;                   ///< Total shaders compiled
        uint64_t shadersLinked;                     ///< Total shaders linked
        uint64_t compilationErrors;                 ///< Compilation errors
        uint64_t linkingErrors;                     ///< Linking errors
        double averageCompileTime;                  ///< Average compile time (ms)
        double averageLinkTime;                     ///< Average link time (ms)

        // Cache stats
        uint64_t cacheHits;                         ///< Cache hits
        uint64_t cacheMisses;                       ///< Cache misses
        float cacheHitRate;                         ///< Cache hit rate (0.0 - 1.0)
        size_t cacheSize;                           ///< Cache size (bytes)

        // Runtime stats
        uint64_t programSwitches;                   ///< Program switches
        uint64_t uniformUpdates;                    ///< Uniform updates
        uint64_t textureBinds;                      ///< Texture bindings
        double totalRenderTime;                     ///< Total render time with shaders

        // Resource stats
        uint32_t activePrograms;                    ///< Active shader programs
        uint32_t totalPrograms;                     ///< Total loaded programs
        size_t memoryUsage;                         ///< Memory usage (bytes)
        uint32_t maxPrograms;                       ///< Maximum programs loaded
    };

    /**
     * @class ShaderManager
     * @brief Advanced shader management system
     *
     * The ShaderManager class provides comprehensive shader management for the
     * VoxelCraft game engine, featuring PBR shaders, advanced lighting models,
     * post-processing effects, GPU optimization, and hot-reloading capabilities
     * with intelligent shader caching and performance monitoring.
     *
     * Key Features:
     * - Physically Based Rendering (PBR) with multiple workflows
     * - Advanced material shaders with complex lighting models
     * - Post-processing shaders with HDR and tone mapping
     * - Compute shaders for advanced effects and simulation
     * - Shader hot-reloading with automatic recompilation
     * - Intelligent shader caching and optimization
     * - Performance monitoring and bottleneck detection
     * - Multi-platform shader compilation
     * - Shader debugging and validation tools
     * - Include system and preprocessor support
     * - Ray tracing shader support (when available)
     *
     * The shader manager is designed to be highly extensible and optimized,
     * supporting both cutting-edge graphics features and fallback modes for
     * older hardware through automatic feature detection and graceful degradation.
     */
    class ShaderManager {
    public:
        /**
         * @brief Constructor
         * @param config Shader manager configuration
         */
        explicit ShaderManager(const ShaderConfig& config);

        /**
         * @brief Destructor
         */
        ~ShaderManager();

        /**
         * @brief Deleted copy constructor
         */
        ShaderManager(const ShaderManager&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        ShaderManager& operator=(const ShaderManager&) = delete;

        // Shader manager lifecycle

        /**
         * @brief Initialize shader manager
         * @return true if successful, false otherwise
         */
        bool Initialize();

        /**
         * @brief Shutdown shader manager
         */
        void Shutdown();

        /**
         * @brief Update shader manager
         * @param deltaTime Time elapsed since last update
         */
        void Update(double deltaTime);

        /**
         * @brief Get shader configuration
         * @return Current configuration
         */
        const ShaderConfig& GetConfig() const { return m_config; }

        /**
         * @brief Set shader configuration
         * @param config New configuration
         */
        void SetConfig(const ShaderConfig& config);

        // Shader loading and compilation

        /**
         * @brief Load shader from files
         * @param name Shader name
         * @param vertexPath Vertex shader file path
         * @param fragmentPath Fragment shader file path
         * @param geometryPath Geometry shader file path (optional)
         * @return Shader program ID or 0 if failed
         */
        uint32_t LoadShader(const std::string& name,
                           const std::string& vertexPath,
                           const std::string& fragmentPath,
                           const std::string& geometryPath = "");

        /**
         * @brief Load shader from source code
         * @param name Shader name
         * @param vertexSource Vertex shader source
         * @param fragmentSource Fragment shader source
         * @param geometrySource Geometry shader source (optional)
         * @return Shader program ID or 0 if failed
         */
        uint32_t LoadShaderFromSource(const std::string& name,
                                     const std::string& vertexSource,
                                     const std::string& fragmentSource,
                                     const std::string& geometrySource = "");

        /**
         * @brief Load compute shader
         * @param name Shader name
         * @param computeSource Compute shader source
         * @return Shader program ID or 0 if failed
         */
        uint32_t LoadComputeShader(const std::string& name,
                                  const std::string& computeSource);

        /**
         * @brief Get shader program by name
         * @param name Shader name
         * @return Shader program or nullptr if not found
         */
        std::shared_ptr<ShaderProgram> GetShader(const std::string& name) const;

        /**
         * @brief Get shader program by ID
         * @param programId Program ID
         * @return Shader program or nullptr if not found
         */
        std::shared_ptr<ShaderProgram> GetShader(uint32_t programId) const;

        /**
         * @brief Unload shader
         * @param name Shader name
         * @return true if successful, false otherwise
         */
        bool UnloadShader(const std::string& name);

        /**
         * @brief Unload all shaders
         */
        void UnloadAllShaders();

        // Shader program management

        /**
         * @brief Use shader program
         * @param programId Program ID
         * @return true if successful, false otherwise
         */
        bool UseProgram(uint32_t programId);

        /**
         * @brief Get current shader program
         * @return Current program ID
         */
        uint32_t GetCurrentProgram() const { return m_currentProgram; }

        /**
         * @brief Create shader program from multiple shaders
         * @param name Program name
         * @param shaders Map of shader type to shader ID
         * @return Program ID or 0 if failed
         */
        uint32_t CreateProgram(const std::string& name,
                              const std::unordered_map<ShaderType, uint32_t>& shaders);

        // Uniform management

        /**
         * @brief Set uniform value (int)
         * @param programId Program ID
         * @param name Uniform name
         * @param value Uniform value
         * @return true if successful, false otherwise
         */
        bool SetUniform(uint32_t programId, const std::string& name, int value);

        /**
         * @brief Set uniform value (float)
         * @param programId Program ID
         * @param name Uniform name
         * @param value Uniform value
         * @return true if successful, false otherwise
         */
        bool SetUniform(uint32_t programId, const std::string& name, float value);

        /**
         * @brief Set uniform value (vec2)
         * @param programId Program ID
         * @param name Uniform name
         * @param value Uniform value
         * @return true if successful, false otherwise
         */
        bool SetUniform(uint32_t programId, const std::string& name, const glm::vec2& value);

        /**
         * @brief Set uniform value (vec3)
         * @param programId Program ID
         * @param name Uniform name
         * @param value Uniform value
         * @return true if successful, false otherwise
         */
        bool SetUniform(uint32_t programId, const std::string& name, const glm::vec3& value);

        /**
         * @brief Set uniform value (vec4)
         * @param programId Program ID
         * @param name Uniform name
         * @param value Uniform value
         * @return true if successful, false otherwise
         */
        bool SetUniform(uint32_t programId, const std::string& name, const glm::vec4& value);

        /**
         * @brief Set uniform value (mat4)
         * @param programId Program ID
         * @param name Uniform name
         * @param value Uniform value
         * @return true if successful, false otherwise
         */
        bool SetUniform(uint32_t programId, const std::string& name, const glm::mat4& value);

        /**
         * @brief Set uniform array
         * @param programId Program ID
         * @param name Uniform name
         * @param values Array values
         * @param count Array count
         * @return true if successful, false otherwise
         */
        bool SetUniformArray(uint32_t programId, const std::string& name,
                           const float* values, int count);

        /**
         * @brief Set uniform array (vec3)
         * @param programId Program ID
         * @param name Uniform name
         * @param values Array values
         * @param count Array count
         * @return true if successful, false otherwise
         */
        bool SetUniformArray(uint32_t programId, const std::string& name,
                           const glm::vec3* values, int count);

        /**
         * @brief Get uniform location
         * @param programId Program ID
         * @param name Uniform name
         * @return Uniform location or -1 if not found
         */
        int GetUniformLocation(uint32_t programId, const std::string& name);

        // Predefined shader loading

        /**
         * @brief Load PBR shader
         * @param name Shader name
         * @param workflow PBR workflow to use
         * @return Program ID or 0 if failed
         */
        uint32_t LoadPBRShader(const std::string& name,
                              PBRWorkflow workflow = PBRWorkflow::MetallicRoughness);

        /**
         * @brief Load terrain shader
         * @param name Shader name
         * @return Program ID or 0 if failed
         */
        uint32_t LoadTerrainShader(const std::string& name);

        /**
         * @brief Load water shader
         * @param name Shader name
         * @return Program ID or 0 if failed
         */
        uint32_t LoadWaterShader(const std::string& name);

        /**
         * @brief Load skybox shader
         * @param name Shader name
         * @return Program ID or 0 if failed
         */
        uint32_t LoadSkyboxShader(const std::string& name);

        /**
         * @brief Load particle shader
         * @param name Shader name
         * @return Program ID or 0 if failed
         */
        uint32_t LoadParticleShader(const std::string& name);

        /**
         * @brief Load post-processing shader
         * @param name Shader name
         * @param effectName Post-processing effect name
         * @return Program ID or 0 if failed
         */
        uint32_t LoadPostProcessShader(const std::string& name, const std::string& effectName);

        /**
         * @brief Load compute shader
         * @param name Shader name
         * @param computePath Compute shader file path
         * @return Program ID or 0 if failed
         */
        uint32_t LoadComputeShader(const std::string& name, const std::string& computePath);

        // Shader hot-reloading

        /**
         * @brief Enable shader hot-reloading
         * @param enabled Enable state
         */
        void SetHotReloadingEnabled(bool enabled);

        /**
         * @brief Check if hot-reloading is enabled
         * @return true if enabled, false otherwise
         */
        bool IsHotReloadingEnabled() const { return m_hotReloadingEnabled; }

        /**
         * @brief Reload shader
         * @param name Shader name
         * @return true if successful, false otherwise
         */
        bool ReloadShader(const std::string& name);

        /**
         * @brief Reload all shaders
         * @return Number of shaders reloaded
         */
        size_t ReloadAllShaders();

        /**
         * @brief Check for shader file changes
         * @return Number of changed shaders
         */
        size_t CheckForChanges();

        // Shader optimization

        /**
         * @brief Optimize shader for current hardware
         * @param programId Program ID
         * @return true if successful, false otherwise
         */
        bool OptimizeShader(uint32_t programId);

        /**
         * @brief Generate shader permutation
         * @param baseName Base shader name
         * @param defines Preprocessor defines
         * @return Permutation name
         */
        std::string GeneratePermutationName(const std::string& baseName,
                                           const std::unordered_map<std::string, std::any>& defines);

        // Shader introspection

        /**
         * @brief Get shader uniforms
         * @param programId Program ID
         * @return Map of uniform names to uniform info
         */
        std::unordered_map<std::string, ShaderUniform> GetShaderUniforms(uint32_t programId);

        /**
         * @brief Get shader attributes
         * @param programId Program ID
         * @return Vector of attribute names
         */
        std::vector<std::string> GetShaderAttributes(uint32_t programId);

        /**
         * @brief Validate shader program
         * @param programId Program ID
         * @return true if valid, false otherwise
         */
        bool ValidateShader(uint32_t programId);

        // Shader caching

        /**
         * @brief Enable shader caching
         * @param enabled Enable state
         * @param cachePath Cache directory path
         */
        void SetShaderCachingEnabled(bool enabled, const std::string& cachePath = "");

        /**
         * @brief Clear shader cache
         * @return Number of cached shaders removed
         */
        size_t ClearShaderCache();

        /**
         * @brief Get shader cache statistics
         * @return Cache statistics
         */
        std::unordered_map<std::string, size_t> GetCacheStatistics() const;

        // Performance monitoring

        /**
         * @brief Get shader statistics
         * @return Performance statistics
         */
        const ShaderStats& GetStats() const { return m_stats; }

        /**
         * @brief Reset statistics
         */
        void ResetStats();

        /**
         * @brief Get performance report
         * @return Detailed performance report
         */
        std::string GetPerformanceReport() const;

        // Error handling

        /**
         * @brief Get last compilation error
         * @return Error message
         */
        const std::string& GetLastError() const { return m_lastError; }

        /**
         * @brief Clear last error
         */
        void ClearLastError() { m_lastError.clear(); }

        /**
         * @brief Get compilation log
         * @param programId Program ID
         * @return Compilation log
         */
        std::string GetCompilationLog(uint32_t programId) const;

        // Utility functions

        /**
         * @brief Check if shader exists
         * @param name Shader name
         * @return true if exists, false otherwise
         */
        bool ShaderExists(const std::string& name) const;

        /**
         * @brief Get shader names
         * @return Vector of shader names
         */
        std::vector<std::string> GetShaderNames() const;

        /**
         * @brief Get shader count
         * @return Number of loaded shaders
         */
        size_t GetShaderCount() const { return m_shaders.size(); }

        /**
         * @brief Validate shader manager state
         * @return true if valid, false otherwise
         */
        bool Validate() const;

        /**
         * @brief Get status report
         * @return Detailed status report
         */
        std::string GetStatusReport() const;

        /**
         * @brief Optimize shader manager
         * @return Optimization results
         */
        std::unordered_map<std::string, size_t> Optimize();

    private:
        /**
         * @brief Initialize shader system
         * @return true if successful, false otherwise
         */
        bool InitializeShaderSystem();

        /**
         * @brief Compile shader
         * @param type Shader type
         * @param source Shader source
         * @param defines Preprocessor defines
         * @return Shader ID or 0 if failed
         */
        uint32_t CompileShader(ShaderType type, const std::string& source,
                              const std::unordered_map<std::string, std::any>& defines = {});

        /**
         * @brief Link shader program
         * @param shaders Map of shader type to shader ID
         * @return Program ID or 0 if failed
         */
        uint32_t LinkProgram(const std::unordered_map<ShaderType, uint32_t>& shaders);

        /**
         * @brief Load shader source from file
         * @param filePath File path
         * @param includes Include paths
         * @return Shader source or empty string if failed
         */
        std::string LoadShaderSource(const std::string& filePath,
                                   const std::vector<std::string>& includes = {});

        /**
         * @brief Process shader includes
         * @param source Shader source
         * @param includePaths Include search paths
         * @return Processed source
         */
        std::string ProcessIncludes(const std::string& source,
                                  const std::vector<std::string>& includePaths);

        /**
         * @brief Create shader permutation
         * @param baseSource Base shader source
         * @param defines Preprocessor defines
         * @return Permutated source
         */
        std::string CreatePermutation(const std::string& baseSource,
                                    const std::unordered_map<std::string, std::any>& defines);

        /**
         * @brief Cache shader program
         * @param program Shader program
         * @return true if cached, false otherwise
         */
        bool CacheShaderProgram(const std::shared_ptr<ShaderProgram>& program);

        /**
         * @brief Load shader from cache
         * @param hash Program hash
         * @return Cached program or nullptr
         */
        std::shared_ptr<ShaderProgram> LoadFromCache(const std::string& hash);

        /**
         * @brief Update shader file watch
         */
        void UpdateFileWatch();

        /**
         * @brief Handle file change notification
         * @param filePath Changed file path
         */
        void OnFileChanged(const std::string& filePath);

        /**
         * @brief Update performance statistics
         * @param deltaTime Time elapsed
         */
        void UpdateStats(double deltaTime);

        /**
         * @brief Handle shader errors
         * @param error Error message
         */
        void HandleError(const std::string& error);

        // Shader manager data
        ShaderConfig m_config;                           ///< Shader configuration
        ShaderStats m_stats;                             ///< Performance statistics

        // Shader storage
        std::unordered_map<std::string, std::shared_ptr<ShaderProgram>> m_shaders; ///< Loaded shaders
        std::unordered_map<uint32_t, std::shared_ptr<ShaderProgram>> m_programs; ///< Programs by ID
        mutable std::shared_mutex m_shadersMutex;       ///< Shaders synchronization

        // Shader cache
        std::unordered_map<std::string, std::shared_ptr<ShaderProgram>> m_shaderCache; ///< Shader cache
        std::string m_cachePath;                        ///< Cache directory path
        bool m_cachingEnabled;                          ///< Caching enabled
        mutable std::shared_mutex m_cacheMutex;         ///< Cache synchronization

        // Hot-reloading
        bool m_hotReloadingEnabled;                     ///< Hot-reloading enabled
        std::unordered_map<std::string, std::filesystem::file_time_type> m_fileTimestamps; ///< File timestamps
        std::vector<std::string> m_changedFiles;        ///< Changed files list

        // Current state
        uint32_t m_currentProgram;                      ///< Currently active program
        std::string m_lastError;                        ///< Last error message
        bool m_isInitialized;                           ///< Manager is initialized
        double m_lastUpdateTime;                        ///< Last update time

        static std::atomic<uint32_t> s_nextProgramId;   ///< Next program ID counter
        static std::atomic<uint32_t> s_nextShaderId;    ///< Next shader ID counter
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_GRAPHICS_SHADER_MANAGER_HPP
