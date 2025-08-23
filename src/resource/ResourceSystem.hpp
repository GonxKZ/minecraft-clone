/**
 * @file ResourceSystem.hpp
 * @brief VoxelCraft Advanced Resource Management System Header
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_RESOURCE_RESOURCE_SYSTEM_HPP
#define VOXELCRAFT_RESOURCE_RESOURCE_SYSTEM_HPP

#include <memory>
#include <atomic>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <functional>
#include <queue>
#include <chrono>
#include <thread>
#include <condition_variable>
#include <future>
#include <variant>
#include <any>

#include <glm/glm.hpp>

#include "../core/System.hpp"
#include "../memory/MemorySystem.hpp"

namespace VoxelCraft {

    // Forward declarations
    class Resource;
    class ResourceLoader;
    class ResourceCache;
    class ResourceStreamer;
    class ResourceCompressor;
    class ResourceOptimizer;
    class ResourceAnalyzer;

    /**
     * @brief Resource types
     */
    enum class ResourceType {
        TEXTURE,
        MODEL,
        MESH,
        MATERIAL,
        SHADER,
        AUDIO,
        VIDEO,
        ANIMATION,
        FONT,
        SCRIPT,
        CONFIG,
        SAVE_DATA,
        TERRAIN,
        WORLD,
        ENTITY,
        PARTICLE,
        UI_LAYOUT,
        UI_STYLE,
        LOCALIZATION,
        BLUEPRINT,
        PREFAB,
        SCENE,
        LIGHTMAP,
        SHADOWMAP,
        REFLECTION_PROBE,
        NAVMESH,
        COLLISION_MESH,
        PROCEDURAL_DATA,
        CUSTOM_1,
        CUSTOM_2,
        CUSTOM_3,
        CUSTOM_4,
        MAX_TYPES
    };

    /**
     * @brief Resource loading priority
     */
    enum class ResourcePriority {
        CRITICAL,       // Load immediately (blocking)
        HIGH,           // Load as soon as possible
        NORMAL,         // Standard loading priority
        LOW,            // Load when system is idle
        BACKGROUND,     // Load in background thread
        LAZY            // Load only when needed
    };

    /**
     * @brief Resource states
     */
    enum class ResourceState {
        UNLOADED,       // Not loaded
        QUEUED,         // Queued for loading
        LOADING,        // Currently loading
        LOADED,         // Successfully loaded
        READY,          // Ready to use
        ERROR,          // Loading failed
        UNLOADING,      // Being unloaded
        COMPRESSED,     // Compressed in memory
        STREAMING       // Being streamed
    };

    /**
     * @brief Resource compression types
     */
    enum class CompressionType {
        NONE,
        LZ4,
        LZMA,
        ZLIB,
        GZIP,
        BROTLI,
        CUSTOM
    };

    /**
     * @brief Resource cache policies
     */
    enum class CachePolicy {
        NEVER_CACHE,    // Don't cache this resource
        MEMORY_ONLY,    // Cache only in memory
        DISK_ONLY,      // Cache only on disk
        MEMORY_AND_DISK,// Cache in both memory and disk
        STREAMING       // Use streaming cache
    };

    /**
     * @brief Resource loading modes
     */
    enum class LoadingMode {
        SYNCHRONOUS,    // Load immediately (blocking)
        ASYNCHRONOUS,   // Load in background thread
        STREAMING,      // Stream resource data
        DEMAND          // Load on demand
    };

    /**
     * @brief Resource metadata
     */
    struct ResourceMetadata {
        std::string name;
        ResourceType type;
        std::string path;
        uint64_t size = 0;
        uint64_t compressedSize = 0;
        std::chrono::system_clock::time_point createdTime;
        std::chrono::system_clock::time_point modifiedTime;
        std::chrono::system_clock::time_point lastAccessTime;
        uint32_t version = 0;
        std::unordered_map<std::string, std::any> properties;
        std::vector<std::string> dependencies;
        std::vector<std::string> tags;
    };

    /**
     * @brief Resource loading parameters
     */
    struct ResourceLoadParams {
        ResourcePriority priority = ResourcePriority::NORMAL;
        LoadingMode mode = LoadingMode::ASYNCHRONOUS;
        bool useCache = true;
        bool decompressOnLoad = true;
        bool generateMipmaps = true;
        uint32_t quality = 100;
        std::unordered_map<std::string, std::any> customParams;
    };

    /**
     * @brief Resource base class
     */
    class Resource {
    public:
        Resource(const std::string& id, ResourceType type);
        virtual ~Resource() = default;

        const std::string& GetId() const { return m_id; }
        ResourceType GetType() const { return m_type; }
        ResourceState GetState() const { return m_state; }

        const ResourceMetadata& GetMetadata() const { return m_metadata; }
        void SetMetadata(const ResourceMetadata& metadata) { m_metadata = metadata; }

        uint32_t GetReferenceCount() const { return m_refCount; }
        void AddReference();
        void RemoveReference();

        bool IsReady() const { return m_state == ResourceState::READY; }
        bool IsLoading() const { return m_state == ResourceState::LOADING; }
        bool HasError() const { return m_state == ResourceState::ERROR; }

        const std::string& GetErrorMessage() const { return m_errorMessage; }
        void SetErrorMessage(const std::string& message) { m_errorMessage = message; }

        float GetLoadProgress() const { return m_loadProgress; }
        void SetLoadProgress(float progress) { m_loadProgress = progress; }

        virtual size_t GetMemoryUsage() const = 0;
        virtual bool Validate() const = 0;

        // Resource-specific methods
        virtual bool Load(const std::vector<uint8_t>& data) = 0;
        virtual bool Unload() = 0;
        virtual bool Reload() = 0;

        // Serialization
        virtual bool Serialize(std::vector<uint8_t>& data) const { return false; }
        virtual bool Deserialize(const std::vector<uint8_t>& data) { return false; }

    protected:
        std::string m_id;
        ResourceType m_type;
        ResourceState m_state;
        ResourceMetadata m_metadata;
        std::atomic<uint32_t> m_refCount;
        std::string m_errorMessage;
        std::atomic<float> m_loadProgress;
        mutable std::mutex m_resourceMutex;
    };

    /**
     * @brief Texture resource
     */
    class TextureResource : public Resource {
    public:
        TextureResource(const std::string& id);
        ~TextureResource() override;

        size_t GetMemoryUsage() const override;
        bool Validate() const override;

        bool Load(const std::vector<uint8_t>& data) override;
        bool Unload() override;
        bool Reload() override;

        // Texture-specific properties
        const glm::ivec2& GetSize() const { return m_size; }
        int GetChannels() const { return m_channels; }
        int GetMipLevels() const { return m_mipLevels; }
        bool HasAlpha() const { return m_hasAlpha; }

        void* GetTextureHandle() const { return m_textureHandle; }

    private:
        glm::ivec2 m_size;
        int m_channels;
        int m_mipLevels;
        bool m_hasAlpha;
        void* m_textureHandle;
        std::vector<uint8_t> m_pixelData;
    };

    /**
     * @brief Model resource
     */
    class ModelResource : public Resource {
    public:
        ModelResource(const std::string& id);
        ~ModelResource() override;

        size_t GetMemoryUsage() const override;
        bool Validate() const override;

        bool Load(const std::vector<uint8_t>& data) override;
        bool Unload() override;
        bool Reload() override;

        // Model-specific properties
        uint32_t GetVertexCount() const { return m_vertexCount; }
        uint32_t GetIndexCount() const { return m_indexCount; }
        uint32_t GetMeshCount() const { return m_meshes.size(); }
        bool HasAnimations() const { return m_hasAnimations; }
        bool HasSkeleton() const { return m_hasSkeleton; }

        void* GetModelHandle() const { return m_modelHandle; }
        const std::vector<void*>& GetMeshes() const { return m_meshes; }

    private:
        uint32_t m_vertexCount;
        uint32_t m_indexCount;
        bool m_hasAnimations;
        bool m_hasSkeleton;
        void* m_modelHandle;
        std::vector<void*> m_meshes;
        std::vector<uint8_t> m_vertexData;
        std::vector<uint8_t> m_indexData;
    };

    /**
     * @brief Audio resource
     */
    class AudioResource : public Resource {
    public:
        AudioResource(const std::string& id);
        ~AudioResource() override;

        size_t GetMemoryUsage() const override;
        bool Validate() const override;

        bool Load(const std::vector<uint8_t>& data) override;
        bool Unload() override;
        bool Reload() override;

        // Audio-specific properties
        float GetDuration() const { return m_duration; }
        uint32_t GetSampleRate() const { return m_sampleRate; }
        uint32_t GetChannels() const { return m_channels; }
        uint32_t GetBitsPerSample() const { return m_bitsPerSample; }

        void* GetAudioHandle() const { return m_audioHandle; }
        const std::vector<float>& GetPCMData() const { return m_pcmData; }

    private:
        float m_duration;
        uint32_t m_sampleRate;
        uint32_t m_channels;
        uint32_t m_bitsPerSample;
        void* m_audioHandle;
        std::vector<float> m_pcmData;
    };

    /**
     * @brief Resource loader interface
     */
    class ResourceLoader {
    public:
        ResourceLoader(ResourceType type);
        virtual ~ResourceLoader() = default;

        ResourceType GetType() const { return m_type; }
        const std::string& GetName() const { return m_name; }

        virtual bool CanLoad(const std::string& path) const = 0;
        virtual std::shared_ptr<Resource> LoadResource(const std::string& id,
                                                      const std::string& path,
                                                      const ResourceLoadParams& params) = 0;
        virtual bool SaveResource(const std::shared_ptr<Resource>& resource,
                                 const std::string& path) = 0;

        virtual std::vector<std::string> GetSupportedExtensions() const = 0;
        virtual ResourceMetadata GetMetadata(const std::string& path) const = 0;

    protected:
        ResourceType m_type;
        std::string m_name;
    };

    /**
     * @brief Resource cache
     */
    class ResourceCache {
    public:
        ResourceCache(size_t maxMemorySize, size_t maxDiskSize);
        ~ResourceCache() = default;

        bool AddResource(const std::shared_ptr<Resource>& resource, CachePolicy policy);
        std::shared_ptr<Resource> GetResource(const std::string& id) const;
        bool RemoveResource(const std::string& id);
        void Clear();

        size_t GetMemoryUsage() const;
        size_t GetDiskUsage() const;
        float GetMemoryUtilization() const;
        float GetDiskUtilization() const;

        void SetMaxMemorySize(size_t size) { m_maxMemorySize = size; }
        void SetMaxDiskSize(size_t size) { m_maxDiskSize = size; }

        void EnableCompression(bool enable) { m_compressionEnabled = enable; }
        bool IsCompressionEnabled() const { return m_compressionEnabled; }

        // Cache maintenance
        void Update(float deltaTime);
        void EvictOldResources();
        void Optimize();

    private:
        mutable std::shared_mutex m_cacheMutex;
        std::unordered_map<std::string, std::shared_ptr<Resource>> m_memoryCache;
        std::unordered_map<std::string, std::string> m_diskCache; // id -> file path
        size_t m_maxMemorySize;
        size_t m_maxDiskSize;
        size_t m_currentMemorySize;
        size_t m_currentDiskSize;
        bool m_compressionEnabled;

        bool ShouldEvict(const std::shared_ptr<Resource>& resource) const;
        std::vector<std::string> GetEvictionCandidates() const;
    };

    /**
     * @brief Resource streamer for streaming resources
     */
    class ResourceStreamer {
    public:
        ResourceStreamer(size_t bufferSize);
        ~ResourceStreamer() = default;

        bool StartStreaming(const std::string& id, const std::string& path);
        bool StopStreaming(const std::string& id);
        bool IsStreaming(const std::string& id) const;

        size_t GetStreamedData(const std::string& id, std::vector<uint8_t>& buffer, size_t offset, size_t size);
        float GetStreamingProgress(const std::string& id) const;

        void Update(float deltaTime);
        void SetBufferSize(size_t size) { m_bufferSize = size; }

    private:
        struct StreamInfo {
            std::string path;
            std::ifstream file;
            size_t fileSize;
            size_t currentPosition;
            std::vector<uint8_t> buffer;
            float progress;
            bool active;
        };

        mutable std::shared_mutex m_streamMutex;
        std::unordered_map<std::string, StreamInfo> m_activeStreams;
        size_t m_bufferSize;
    };

    /**
     * @brief Resource loading task
     */
    struct ResourceLoadTask {
        std::string resourceId;
        std::string path;
        ResourceType type;
        ResourceLoadParams params;
        std::function<void(std::shared_ptr<Resource>)> callback;
        std::chrono::steady_clock::time_point startTime;
        std::future<std::shared_ptr<Resource>> future;
        ResourcePriority priority;

        bool operator<(const ResourceLoadTask& other) const {
            return static_cast<int>(priority) < static_cast<int>(other.priority);
        }
    };

    /**
     * @brief Main resource system
     */
    class ResourceSystem : public System {
    public:
        static ResourceSystem& GetInstance();

        ResourceSystem();
        ~ResourceSystem() override;

        // System interface
        bool Initialize() override;
        void Update(float deltaTime) override;
        void Shutdown() override;
        std::string GetName() const override { return "ResourceSystem"; }
        System::Type GetType() const override { return System::Type::RESOURCE; }

        // Resource loading
        std::shared_ptr<Resource> LoadResource(const std::string& id,
                                              const std::string& path,
                                              ResourceType type,
                                              const ResourceLoadParams& params = ResourceLoadParams());

        std::future<std::shared_ptr<Resource>> LoadResourceAsync(const std::string& id,
                                                                const std::string& path,
                                                                ResourceType type,
                                                                const ResourceLoadParams& params = ResourceLoadParams());

        bool UnloadResource(const std::string& id);
        bool ReloadResource(const std::string& id);

        std::shared_ptr<Resource> GetResource(const std::string& id) const;
        bool IsResourceLoaded(const std::string& id) const;
        bool IsResourceLoading(const std::string& id) const;

        // Resource management
        void RegisterLoader(std::shared_ptr<ResourceLoader> loader);
        void UnregisterLoader(ResourceType type);
        std::shared_ptr<ResourceLoader> GetLoader(ResourceType type) const;

        // Cache management
        bool AddToCache(const std::shared_ptr<Resource>& resource, CachePolicy policy);
        bool RemoveFromCache(const std::string& id);
        void ClearCache();
        void OptimizeCache();

        // Streaming
        bool StartStreaming(const std::string& id, const std::string& path);
        bool StopStreaming(const std::string& id);
        size_t GetStreamedData(const std::string& id, std::vector<uint8_t>& buffer, size_t offset, size_t size);

        // Resource queries
        std::vector<std::string> GetLoadedResources(ResourceType type = ResourceType::MAX_TYPES) const;
        std::vector<std::string> GetLoadingResources() const;
        std::vector<std::string> GetFailedResources() const;

        // Statistics
        struct ResourceStats {
            uint32_t totalResources;
            uint32_t loadedResources;
            uint32_t loadingResources;
            uint32_t failedResources;
            uint64_t totalMemoryUsage;
            uint64_t cacheMemoryUsage;
            uint64_t cacheDiskUsage;
            uint32_t activeStreams;
            float averageLoadTime;
            uint32_t cacheHits;
            uint32_t cacheMisses;
        };

        ResourceStats GetStats() const;

        // Configuration
        void SetMaxConcurrentLoads(uint32_t max) { m_maxConcurrentLoads = max; }
        uint32_t GetMaxConcurrentLoads() const { return m_maxConcurrentLoads; }

        void SetCacheMemoryLimit(size_t limit);
        size_t GetCacheMemoryLimit() const;

        void SetCacheDiskLimit(size_t limit);
        size_t GetCacheDiskLimit() const;

        // Utility functions
        std::string GetResourceInfo() const;
        void PrintResourceReport() const;

        bool ValidateAllResources() const;
        void CleanupUnusedResources();

        // Resource dependencies
        bool AddResourceDependency(const std::string& resourceId, const std::string& dependencyId);
        std::vector<std::string> GetResourceDependencies(const std::string& resourceId) const;
        bool CheckResourceDependencies(const std::string& resourceId) const;

        // Resource optimization
        bool OptimizeResource(const std::string& id, const std::unordered_map<std::string, std::any>& options);
        bool CompressResource(const std::string& id, CompressionType type);
        bool DecompressResource(const std::string& id);

    private:
        // Core components
        mutable std::shared_mutex m_resourcesMutex;
        std::unordered_map<std::string, std::shared_ptr<Resource>> m_resources;

        mutable std::shared_mutex m_loadersMutex;
        std::unordered_map<ResourceType, std::shared_ptr<ResourceLoader>> m_loaders;

        ResourceCache m_cache;
        ResourceStreamer m_streamer;

        // Loading system
        std::priority_queue<ResourceLoadTask> m_loadQueue;
        std::unordered_map<std::string, ResourceLoadTask> m_activeLoads;
        mutable std::shared_mutex m_loadMutex;
        std::condition_variable_any m_loadCondition;
        std::vector<std::thread> m_loaderThreads;

        // Configuration
        uint32_t m_maxConcurrentLoads;
        size_t m_cacheMemoryLimit;
        size_t m_cacheDiskLimit;

        // Statistics
        mutable std::shared_mutex m_statsMutex;
        ResourceStats m_stats;

        // Internal methods
        void StartLoaderThreads();
        void StopLoaderThreads();
        void LoaderThreadFunction();

        std::shared_ptr<Resource> LoadResourceInternal(const ResourceLoadTask& task);
        bool ProcessLoadQueue();

        void UpdateResourceState(const std::string& id, ResourceState state);
        void UpdateStats(const std::string& id, bool loaded, bool failed);

        bool LoadFromCache(const std::string& id, std::shared_ptr<Resource>& resource);
        bool SaveToCache(const std::shared_ptr<Resource>& resource);

        ResourceType DetectResourceType(const std::string& path) const;
        std::shared_ptr<ResourceLoader> FindLoaderForPath(const std::string& path) const;
    };

    // Resource system utility macros
    #define VOXELCRAFT_LOAD_RESOURCE(id, path, type) \
        ResourceSystem::GetInstance().LoadResource(id, path, type)

    #define VOXELCRAFT_LOAD_RESOURCE_ASYNC(id, path, type, callback) \
        ResourceSystem::GetInstance().LoadResourceAsync(id, path, type).then(callback)

    #define VOXELCRAFT_GET_RESOURCE(id) \
        ResourceSystem::GetInstance().GetResource(id)

    #define VOXELCRAFT_UNLOAD_RESOURCE(id) \
        ResourceSystem::GetInstance().UnloadResource(id)

} // namespace VoxelCraft

#endif // VOXELCRAFT_RESOURCE_RESOURCE_SYSTEM_HPP