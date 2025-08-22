/**
 * @file ProceduralAssetGenerator.hpp
 * @brief VoxelCraft Procedural Asset Generation System
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines the ProceduralAssetGenerator class that provides comprehensive
 * procedural asset generation for the VoxelCraft game engine, including textures,
 * 3D models, sounds, and effects generated algorithmically during game loading
 * for infinite content variety and reduced storage requirements.
 */

#ifndef VOXELCRAFT_PROCEDURAL_PROCEDURAL_ASSET_GENERATOR_HPP
#define VOXELCRAFT_PROCEDURAL_PROCEDURAL_ASSET_GENERATOR_HPP

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
#include <random>
#include <future>
#include <condition_variable>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/noise.hpp>

#include "../core/Config.hpp"
#include "../core/System.hpp"

namespace VoxelCraft {

    // Forward declarations
    class World;
    class BlockRegistry;
    class ResourceManager;
    class ProceduralTextureGenerator;
    class ProceduralModelGenerator;
    class ProceduralSoundGenerator;
    class ProceduralEffectGenerator;
    class AssetCache;
    class GenerationQueue;

    /**
     * @enum AssetType
     * @brief Types of procedural assets
     */
    enum class AssetType {
        Texture,                    ///< 2D/3D textures
        Model,                      ///< 3D models and meshes
        Sound,                      ///< Audio samples and effects
        Effect,                     ///< Particle effects and animations
        Material,                   ///< Material definitions
        Animation,                  ///< Animation data
        Custom                      ///< Custom asset types
    };

    /**
     * @enum GenerationPriority
     * @brief Asset generation priorities
     */
    enum class GenerationPriority {
        Critical,                   ///< Critical assets (must generate immediately)
        High,                       ///< High priority assets
        Normal,                     ///< Normal priority assets
        Low,                        ///< Low priority assets
        Background                  ///< Background generation
    };

    /**
     * @enum GenerationMethod
     * @brief Asset generation methods
     */
    enum class GenerationMethod {
        NoiseBased,                 ///< Noise function based generation
        Fractal,                    ///< Fractal based generation
        CellularAutomata,           ///< Cellular automata based
        LSystem,                    ///< L-System based generation
        GrammarBased,               ///< Grammar based generation
        TemplateBased,              ///< Template based with variations
        Hybrid,                     ///< Hybrid generation methods
        Custom                      ///< Custom generation method
    };

    /**
     * @enum AssetQuality
     * @brief Asset generation quality levels
     */
    enum class AssetQuality {
        Low,                        ///< Low quality (fast generation)
        Medium,                     ///< Medium quality
        High,                       ///< High quality (slower generation)
        Ultra,                      ///< Ultra quality (slowest generation)
        Custom                      ///< Custom quality settings
    };

    /**
     * @struct AssetGenerationRequest
     * @brief Asset generation request descriptor
     */
    struct AssetGenerationRequest {
        std::string assetId;                        ///< Unique asset identifier
        AssetType type;                             ///< Asset type
        GenerationPriority priority;                ///< Generation priority
        AssetQuality quality;                       ///< Generation quality
        GenerationMethod method;                    ///< Generation method

        // Asset-specific parameters
        std::unordered_map<std::string, std::any> parameters; ///< Generation parameters
        std::unordered_map<std::string, float> numericParams; ///< Numeric parameters
        std::unordered_map<std::string, std::string> stringParams; ///< String parameters

        // Generation context
        glm::vec3 position;                         ///< World position context
        uint32_t seed;                              ///< Random seed
        std::string biomeType;                      ///< Biome context
        std::string blockType;                      ///< Block type context

        // Callbacks
        std::function<void(const std::string&)> onProgress; ///< Progress callback
        std::function<void(const std::string&, std::any)> onComplete; ///< Completion callback
        std::function<void(const std::string&, const std::string&)> onError; ///< Error callback

        // Metadata
        double requestTime;                         ///< Request timestamp
        std::string requester;                      ///< System/component that requested
        bool allowCaching;                          ///< Allow caching of result

        AssetGenerationRequest()
            : type(AssetType::Texture)
            , priority(GenerationPriority::Normal)
            , quality(AssetQuality::High)
            , method(GenerationMethod::NoiseBased)
            , position(0.0f, 0.0f, 0.0f)
            , seed(0)
            , requestTime(0.0)
            , allowCaching(true)
        {}
    };

    /**
     * @struct ProceduralAsset
     * @brief Generated procedural asset
     */
    struct ProceduralAsset {
        std::string assetId;                        ///< Unique asset identifier
        AssetType type;                             ///< Asset type
        std::any data;                              ///< Asset data (texture, mesh, audio, etc.)
        AssetGenerationRequest originalRequest;     ///< Original generation request

        // Metadata
        double generationTime;                      ///< Generation timestamp
        double accessTime;                          ///< Last access timestamp
        size_t memoryUsage;                         ///< Memory usage in bytes
        std::string checksum;                       ///< Asset checksum for validation

        // Generation info
        AssetQuality quality;                       ///< Generated quality level
        GenerationMethod method;                    ///< Generation method used
        uint32_t seed;                              ///< Random seed used
        std::unordered_map<std::string, std::any> generationParams; ///< Parameters used

        // Usage tracking
        uint32_t accessCount;                       ///< Number of times accessed
        bool isCompressed;                          ///< Whether asset is compressed
        bool isPersistent;                          ///< Whether asset should persist

        ProceduralAsset()
            : type(AssetType::Texture)
            , generationTime(0.0)
            , accessTime(0.0)
            , memoryUsage(0)
            , quality(AssetQuality::High)
            , method(GenerationMethod::NoiseBased)
            , seed(0)
            , accessCount(0)
            , isCompressed(false)
            , isPersistent(false)
        {}
    };

    /**
     * @struct GenerationStats
     * @brief Asset generation performance statistics
     */
    struct GenerationStats {
        // Generation statistics
        uint64_t totalAssetsGenerated;              ///< Total assets generated
        uint64_t assetsGeneratedByType[7];          ///< Assets by type
        uint64_t assetsGeneratedByPriority[5];      ///< Assets by priority
        uint64_t assetsGeneratedByMethod[8];        ///< Assets by method

        // Performance metrics
        double averageGenerationTime;               ///< Average generation time (ms)
        double minGenerationTime;                   ///< Minimum generation time (ms)
        double maxGenerationTime;                   ///< Maximum generation time (ms)
        double totalGenerationTime;                 ///< Total generation time (ms)

        // Queue statistics
        uint64_t requestsQueued;                    ///< Total requests queued
        uint64_t requestsProcessed;                 ///< Requests processed
        uint64_t requestsCancelled;                 ///< Requests cancelled
        uint32_t currentQueueSize;                  ///< Current queue size
        uint32_t maxQueueSize;                      ///< Maximum queue size

        // Memory statistics
        size_t currentMemoryUsage;                  ///< Current memory usage
        size_t peakMemoryUsage;                     ///< Peak memory usage
        size_t cacheSize;                           ///< Current cache size
        uint64_t cacheHits;                         ///< Cache hits
        uint64_t cacheMisses;                       ///< Cache misses

        // Error statistics
        uint64_t generationErrors;                  ///< Generation errors
        uint64_t assetValidationErrors;             ///< Asset validation errors
        uint64_t memoryAllocationErrors;            ///< Memory allocation errors

        // Threading statistics
        uint32_t activeGenerationThreads;           ///< Active generation threads
        uint32_t totalGenerationThreads;            ///< Total generation threads
        double averageThreadUtilization;            ///< Average thread utilization
    };

    /**
     * @struct ProceduralGenerationConfig
     * @brief Procedural asset generation configuration
     */
    struct ProceduralGenerationConfig {
        // General settings
        bool enableProceduralGeneration;            ///< Enable procedural generation
        bool enableBackgroundGeneration;            ///< Enable background generation
        bool enableAssetCaching;                    ///< Enable asset caching
        bool enableAssetCompression;                ///< Enable asset compression

        // Performance settings
        uint32_t maxGenerationThreads;              ///< Maximum generation threads
        uint32_t maxQueueSize;                      ///< Maximum queue size
        size_t maxCacheSize;                        ///< Maximum cache size (MB)
        float generationTimeout;                    ///< Generation timeout (seconds)

        // Quality settings
        AssetQuality defaultQuality;                ///< Default generation quality
        bool enableQualityScaling;                  ///< Enable quality scaling
        bool enableLODGeneration;                   ///< Enable LOD generation

        // Resource management
        bool enableMemoryManagement;                ///< Enable memory management
        bool enableAssetCleanup;                    ///< Enable asset cleanup
        float assetExpirationTime;                  ///< Asset expiration time (minutes)
        bool enableAssetPersistence;                ///< Enable asset persistence

        // Generation settings
        bool enableParallelGeneration;              ///< Enable parallel generation
        bool enableIncrementalGeneration;           ///< Enable incremental generation
        bool enableTemplateBasedGeneration;         ///< Enable template-based generation
        uint32_t randomSeed;                        ///< Random seed for generation

        // Debug settings
        bool enableGenerationLogging;               ///< Enable generation logging
        bool enablePerformanceProfiling;            ///< Enable performance profiling
        bool enableAssetValidation;                 ///< Enable asset validation
        std::string logFile;                        ///< Log file path

        ProceduralGenerationConfig()
            : enableProceduralGeneration(true)
            , enableBackgroundGeneration(true)
            , enableAssetCaching(true)
            , enableAssetCompression(true)
            , maxGenerationThreads(4)
            , maxQueueSize(1000)
            , maxCacheSize(512)
            , generationTimeout(30.0f)
            , defaultQuality(AssetQuality::High)
            , enableQualityScaling(true)
            , enableLODGeneration(true)
            , enableMemoryManagement(true)
            , enableAssetCleanup(true)
            , assetExpirationTime(30.0f)
            , enableAssetPersistence(false)
            , enableParallelGeneration(true)
            , enableIncrementalGeneration(true)
            , enableTemplateBasedGeneration(true)
            , randomSeed(12345)
            , enableGenerationLogging(false)
            , enablePerformanceProfiling(false)
            , enableAssetValidation(true)
            , logFile("procedural_generation.log")
        {}
    };

    /**
     * @class ProceduralAssetGenerator
     * @brief Advanced procedural asset generation system
     *
     * The ProceduralAssetGenerator class provides comprehensive procedural asset
     * generation for the VoxelCraft game engine, featuring algorithmic generation
     * of textures, 3D models, sounds, and effects during game loading for infinite
     * content variety, reduced storage requirements, and dynamic content creation
     * with enterprise-grade asset management and performance optimization.
     *
     * Key Features:
     * - Multi-type asset generation (textures, models, sounds, effects)
     * - Multiple generation methods (noise, fractal, cellular automata, L-systems)
     * - Priority-based generation queue with background processing
     * - Intelligent asset caching with memory management
     * - Quality scaling and LOD generation
     * - Parallel generation with threading support
     * - Asset compression and optimization
     * - Context-aware generation based on biome, position, and game state
     * - Real-time generation during loading screens
     * - Template-based generation with procedural variations
     * - Performance monitoring and optimization
     *
     * The procedural generation system enables infinite content variety while
     * maintaining consistent quality and performance standards.
     */
    class ProceduralAssetGenerator : public System {
    public:
        /**
         * @brief Constructor
         * @param config Procedural generation configuration
         */
        explicit ProceduralAssetGenerator(const ProceduralGenerationConfig& config);

        /**
         * @brief Destructor
         */
        ~ProceduralAssetGenerator();

        /**
         * @brief Deleted copy constructor
         */
        ProceduralAssetGenerator(const ProceduralAssetGenerator&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        ProceduralAssetGenerator& operator=(const ProceduralAssetGenerator&) = delete;

        // Procedural asset generator lifecycle

        /**
         * @brief Initialize procedural asset generator
         * @return true if successful, false otherwise
         */
        bool Initialize();

        /**
         * @brief Shutdown procedural asset generator
         */
        void Shutdown();

        /**
         * @brief Update procedural asset generator
         * @param deltaTime Time elapsed since last update
         */
        void Update(double deltaTime);

        /**
         * @brief Get generation configuration
         * @return Current configuration
         */
        const ProceduralGenerationConfig& GetConfig() const { return m_config; }

        /**
         * @brief Set generation configuration
         * @param config New configuration
         * @return true if successful, false otherwise
         */
        bool SetConfig(const ProceduralGenerationConfig& config);

        /**
         * @brief Get generation statistics
         * @return Current statistics
         */
        const GenerationStats& GetStats() const { return m_stats; }

        // Asset generation interface

        /**
         * @brief Generate asset synchronously
         * @param request Asset generation request
         * @return Generated asset or nullopt if failed
         */
        std::optional<ProceduralAsset> GenerateAsset(const AssetGenerationRequest& request);

        /**
         * @brief Generate asset asynchronously
         * @param request Asset generation request
         * @return Future containing the generated asset
         */
        std::future<std::optional<ProceduralAsset>> GenerateAssetAsync(const AssetGenerationRequest& request);

        /**
         * @brief Queue asset for generation
         * @param request Asset generation request
         * @return Generation ID or 0 if failed
         */
        uint32_t QueueAssetGeneration(const AssetGenerationRequest& request);

        /**
         * @brief Cancel asset generation
         * @param generationId Generation ID
         * @return true if successful, false otherwise
         */
        bool CancelAssetGeneration(uint32_t generationId);

        /**
         * @brief Get generation progress
         * @param generationId Generation ID
         * @return Progress (0.0 - 1.0) or -1.0 if not found
         */
        float GetGenerationProgress(uint32_t generationId) const;

        /**
         * @brief Check if asset exists in cache
         * @param assetId Asset ID
         * @return true if exists, false otherwise
         */
        bool IsAssetCached(const std::string& assetId) const;

        /**
         * @brief Get cached asset
         * @param assetId Asset ID
         * @return Cached asset or nullopt if not found
         */
        std::optional<ProceduralAsset> GetCachedAsset(const std::string& assetId);

        /**
         * @brief Preload assets for area
         * @param center Center position
         * @param radius Preload radius
         * @param assetTypes Types of assets to preload
         * @return Number of assets queued for preloading
         */
        uint32_t PreloadAssets(const glm::vec3& center, float radius,
                              const std::vector<AssetType>& assetTypes);

        // Asset type generators

        /**
         * @brief Generate texture asset
         * @param request Generation request
         * @return Generated texture data or nullopt if failed
         */
        std::optional<std::any> GenerateTexture(const AssetGenerationRequest& request);

        /**
         * @brief Generate model asset
         * @param request Generation request
         * @return Generated model data or nullopt if failed
         */
        std::optional<std::any> GenerateModel(const AssetGenerationRequest& request);

        /**
         * @brief Generate sound asset
         * @param request Generation request
         * @return Generated sound data or nullopt if failed
         */
        std::optional<std::any> GenerateSound(const AssetGenerationRequest& request);

        /**
         * @brief Generate effect asset
         * @param request Generation request
         * @return Generated effect data or nullopt if failed
         */
        std::optional<std::any> GenerateEffect(const AssetGenerationRequest& request);

        // Generation method interfaces

        /**
         * @brief Generate asset using noise-based method
         * @param request Generation request
         * @return Generated asset data or nullopt if failed
         */
        std::optional<std::any> GenerateWithNoise(const AssetGenerationRequest& request);

        /**
         * @brief Generate asset using fractal method
         * @param request Generation request
         * @return Generated asset data or nullopt if failed
         */
        std::optional<std::any> GenerateWithFractal(const AssetGenerationRequest& request);

        /**
         * @brief Generate asset using cellular automata
         * @param request Generation request
         * @return Generated asset data or nullopt if failed
         */
        std::optional<std::any> GenerateWithCellularAutomata(const AssetGenerationRequest& request);

        /**
         * @brief Generate asset using L-System
         * @param request Generation request
         * @return Generated asset data or nullopt if failed
         */
        std::optional<std::any> GenerateWithLSystem(const AssetGenerationRequest& request);

        // Cache management

        /**
         * @brief Clear asset cache
         * @return Number of assets removed
         */
        uint32_t ClearAssetCache();

        /**
         * @brief Clear expired assets
         * @return Number of assets removed
         */
        uint32_t ClearExpiredAssets();

        /**
         * @brief Optimize asset cache
         * @return Memory freed in bytes
         */
        size_t OptimizeAssetCache();

        /**
         * @brief Get cache statistics
         * @return Cache statistics
         */
        std::unordered_map<std::string, size_t> GetCacheStatistics() const;

        // Performance and optimization

        /**
         * @brief Set generation quality
         * @param quality Generation quality
         * @return true if successful, false otherwise
         */
        bool SetGenerationQuality(AssetQuality quality);

        /**
         * @brief Enable background generation
         * @param enabled Enable state
         * @return true if successful, false otherwise
         */
        bool EnableBackgroundGeneration(bool enabled);

        /**
         * @brief Set maximum generation threads
         * @param count Maximum thread count
         * @return true if successful, false otherwise
         */
        bool SetMaxGenerationThreads(uint32_t count);

        /**
         * @brief Pause generation
         * @return true if successful, false otherwise
         */
        bool PauseGeneration();

        /**
         * @brief Resume generation
         * @return true if successful, false otherwise
         */
        bool ResumeGeneration();

        // Asset templates and presets

        /**
         * @brief Register asset template
         * @param templateName Template name
         * @param assetType Asset type
         * @param templateData Template data
         * @return true if successful, false otherwise
         */
        bool RegisterAssetTemplate(const std::string& templateName,
                                 AssetType assetType,
                                 const std::any& templateData);

        /**
         * @brief Generate asset from template
         * @param templateName Template name
         * @param request Generation request
         * @return Generated asset data or nullopt if failed
         */
        std::optional<std::any> GenerateFromTemplate(const std::string& templateName,
                                                   const AssetGenerationRequest& request);

        /**
         * @brief Get available templates
         * @param assetType Asset type (optional)
         * @return Vector of template names
         */
        std::vector<std::string> GetAvailableTemplates(AssetType assetType = AssetType::Custom) const;

        // Context and world integration

        /**
         * @brief Set world instance for context-aware generation
         * @param world World instance
         */
        void SetWorld(World* world);

        /**
         * @brief Set block registry for block-aware generation
         * @param blockRegistry Block registry instance
         */
        void SetBlockRegistry(BlockRegistry* blockRegistry);

        /**
         * @brief Set resource manager for asset management
         * @param resourceManager Resource manager instance
         */
        void SetResourceManager(ResourceManager* resourceManager);

        /**
         * @brief Get biome at position for context
         * @param position World position
         * @return Biome type string
         */
        std::string GetBiomeAtPosition(const glm::vec3& position) const;

        /**
         * @brief Get generation context for position
         * @param position World position
         * @return Generation context parameters
         */
        std::unordered_map<std::string, std::any> GetGenerationContext(const glm::vec3& position) const;

        // Utility functions

        /**
         * @brief Generate random seed for asset
         * @param position World position
         * @param assetType Asset type
         * @return Generated seed
         */
        uint32_t GenerateAssetSeed(const glm::vec3& position, AssetType assetType);

        /**
         * @brief Validate generated asset
         * @param asset Generated asset
         * @return true if valid, false otherwise
         */
        bool ValidateAsset(const ProceduralAsset& asset) const;

        /**
         * @brief Calculate asset checksum
         * @param asset Asset to checksum
         * @return Checksum string
         */
        std::string CalculateAssetChecksum(const ProceduralAsset& asset) const;

        /**
         * @brief Get asset type name
         * @param type Asset type
         * @return Type name string
         */
        static std::string GetAssetTypeName(AssetType type);

        /**
         * @brief Get generation method name
         * @param method Generation method
         * @return Method name string
         */
        static std::string GetGenerationMethodName(GenerationMethod method);

        /**
         * @brief Get priority name
         * @param priority Generation priority
         * @return Priority name string
         */
        static std::string GetPriorityName(GenerationPriority priority);

        /**
         * @brief Validate procedural asset generator state
         * @return true if valid, false otherwise
         */
        bool Validate() const;

        /**
         * @brief Get status report
         * @return Detailed status report
         */
        std::string GetStatusReport() const;

        /**
         * @brief Get performance report
         * @return Detailed performance report
         */
        std::string GetPerformanceReport() const;

        /**
         * @brief Optimize procedural asset generator
         * @return Optimization results
         */
        std::unordered_map<std::string, size_t> Optimize();

    private:
        /**
         * @brief Initialize asset generators
         * @return true if successful, false otherwise
         */
        bool InitializeAssetGenerators();

        /**
         * @brief Initialize generation queue
         * @return true if successful, false otherwise
         */
        bool InitializeGenerationQueue();

        /**
         * @brief Initialize asset cache
         * @return true if successful, false otherwise
         */
        bool InitializeAssetCache();

        /**
         * @brief Process generation queue
         * @param deltaTime Time elapsed
         */
        void ProcessGenerationQueue(double deltaTime);

        /**
         * @brief Generate asset with specific method
         * @param request Generation request
         * @return Generated asset data or nullopt if failed
         */
        std::optional<std::any> GenerateAssetWithMethod(const AssetGenerationRequest& request);

        /**
         * @brief Compress asset data
         * @param data Asset data
         * @param assetType Asset type
         * @return Compressed data
         */
        std::vector<uint8_t> CompressAssetData(const std::any& data, AssetType assetType);

        /**
         * @brief Decompress asset data
         * @param compressedData Compressed data
         * @param assetType Asset type
         * @return Decompressed data
         */
        std::any DecompressAssetData(const std::vector<uint8_t>& compressedData, AssetType assetType);

        /**
         * @brief Update performance statistics
         * @param generationTime Generation time in milliseconds
         * @param assetType Asset type generated
         */
        void UpdateStats(double generationTime, AssetType assetType);

        /**
         * @brief Handle generation errors
         * @param error Error message
         */
        void HandleError(const std::string& error);

        // Procedural asset generator data
        ProceduralGenerationConfig m_config;        ///< Generation configuration
        GenerationStats m_stats;                    ///< Performance statistics

        // Core systems
        World* m_world;                             ///< Game world instance
        BlockRegistry* m_blockRegistry;             ///< Block registry instance
        ResourceManager* m_resourceManager;         ///< Resource manager instance

        // Asset generators
        std::unique_ptr<ProceduralTextureGenerator> m_textureGenerator; ///< Texture generator
        std::unique_ptr<ProceduralModelGenerator> m_modelGenerator; ///< Model generator
        std::unique_ptr<ProceduralSoundGenerator> m_soundGenerator; ///< Sound generator
        std::unique_ptr<ProceduralEffectGenerator> m_effectGenerator; ///< Effect generator

        // Asset management
        std::unique_ptr<AssetCache> m_assetCache;   ///< Asset cache
        std::unique_ptr<GenerationQueue> m_generationQueue; ///< Generation queue

        // Asset templates
        std::unordered_map<std::string, std::pair<AssetType, std::any>> m_assetTemplates; ///< Asset templates
        mutable std::shared_mutex m_templatesMutex; ///< Templates synchronization

        // Generation state
        std::unordered_map<uint32_t, AssetGenerationRequest> m_activeGenerations; ///< Active generations
        std::unordered_map<uint32_t, float> m_generationProgress; ///< Generation progress
        mutable std::shared_mutex m_generationsMutex; ///< Generations synchronization

        // Threading and async
        std::vector<std::thread> m_generationThreads; ///< Generation threads
        std::atomic<bool> m_generationPaused;         ///< Generation pause state
        std::condition_variable m_generationCV;       ///< Generation condition variable
        mutable std::mutex m_generationMutex;         ///< Generation synchronization

        // Random number generation
        std::mt19937 m_randomEngine;                  ///< Random number engine
        std::uniform_int_distribution<uint32_t> m_seedDistribution; ///< Seed distribution

        // State tracking
        bool m_isInitialized;                         ///< System is initialized
        double m_lastUpdateTime;                      ///< Last update time
        std::string m_lastError;                      ///< Last error message

        static std::atomic<uint32_t> s_nextGenerationId; ///< Next generation ID counter
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_PROCEDURAL_PROCEDURAL_ASSET_GENERATOR_HPP
