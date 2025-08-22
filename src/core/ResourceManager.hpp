/**
 * @file ResourceManager.hpp
 * @brief VoxelCraft Engine Advanced Resource Management System
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file provides a comprehensive resource management system for the VoxelCraft engine
 * with support for asynchronous loading, caching, streaming, compression, and hot-reloading
 * of game assets including textures, models, audio, and configuration files.
 */

#ifndef VOXELCRAFT_CORE_RESOURCE_MANAGER_HPP
#define VOXELCRAFT_CORE_RESOURCE_MANAGER_HPP

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>
#include <queue>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <chrono>
#include <filesystem>
#include <future>
#include <optional>
#include <variant>

namespace VoxelCraft {

    /**
     * @typedef ResourceID
     * @brief Unique identifier for resources
     */
    using ResourceID = uint64_t;

    /**
     * @typedef ResourceHandle
     * @brief Handle for accessing loaded resources
     */
    using ResourceHandle = std::shared_ptr<void>;

    /**
     * @enum ResourceType
     * @brief Type of game resource
     */
    enum class ResourceType {
        Texture,        ///< 2D/3D texture
        Mesh,           ///< 3D mesh/geometry
        Material,       ///< Material/shader combination
        Animation,      ///< Animation data
        Audio,          ///< Audio clip
        Font,           ///< Font data
        Script,         ///< Script file
        Config,         ///< Configuration file
        Scene,          ///< Scene/prefab data
        Shader,         ///< Shader program
        Binary,         ///< Binary data
        Custom          ///< Custom resource type
    };

    /**
     * @enum ResourceState
     * @brief Current state of a resource
     */
    enum class ResourceState {
        Unloaded,       ///< Resource not loaded
        Loading,        ///< Resource is being loaded
        Loaded,         ///< Resource is loaded and ready
        Unloading,      ///< Resource is being unloaded
        Error           ///< Resource loading failed
    };

    /**
     * @enum ResourcePriority
     * @brief Loading priority for resources
     */
    enum class ResourcePriority {
        Critical,       ///< Critical resources (load immediately)
        High,           ///< High priority resources
        Normal,         ///< Normal priority resources
        Low,            ///< Low priority resources
        Background      ///< Background loading only
    };

    /**
     * @struct ResourceInfo
     * @brief Information about a resource
     */
    struct ResourceInfo {
        ResourceID id;                      ///< Unique resource ID
        std::string name;                   ///< Resource name
        std::string path;                   ///< Resource file path
        ResourceType type;                  ///< Resource type
        ResourceState state;                ///< Current state
        ResourcePriority priority;          ///< Loading priority
        size_t size;                        ///< Resource size in bytes
        double loadTime;                    ///< Time spent loading (seconds)
        double lastAccessTime;              ///< Last access timestamp
        uint32_t referenceCount;            ///< Number of active references
        bool isCompressed;                  ///< Resource is compressed
        bool isStreamed;                    ///< Resource supports streaming
        std::string loader;                 ///< Loader used for this resource
        std::unordered_map<std::string, std::string> metadata; ///< Resource metadata
    };

    /**
     * @struct ResourceLoadRequest
     * @brief Request to load a resource
     */
    struct ResourceLoadRequest {
        std::string path;                   ///< Resource path
        ResourceType type;                  ///< Resource type
        ResourcePriority priority;          ///< Loading priority
        bool async;                         ///< Asynchronous loading
        bool compressed;                    ///< Load compressed
        std::function<void(ResourceHandle)> onComplete;  ///< Completion callback
        std::function<void(const std::string&)> onError; ///< Error callback
        std::unordered_map<std::string, std::any> options; ///< Loading options
    };

    /**
     * @struct ResourceStatistics
     * @brief Resource system performance statistics
     */
    struct ResourceStatistics {
        // Overall statistics
        uint64_t totalResources;            ///< Total number of resources
        uint64_t loadedResources;           ///< Number of loaded resources
        uint64_t loadingResources;          ///< Number of resources currently loading
        size_t totalMemoryUsage;            ///< Total memory used by resources
        size_t peakMemoryUsage;             ///< Peak memory usage

        // Loading statistics
        uint64_t totalLoadRequests;         ///< Total load requests
        uint64_t successfulLoads;           ///< Successful loads
        uint64_t failedLoads;               ///< Failed loads
        double averageLoadTime;             ///< Average load time (seconds)
        double cacheHitRate;                ///< Cache hit rate (0.0 - 1.0)

        // Cache statistics
        uint64_t cacheHits;                 ///< Number of cache hits
        uint64_t cacheMisses;               ///< Number of cache misses
        uint64_t cacheEvictions;            ///< Number of cache evictions
        double cacheUtilization;            ///< Cache utilization (0.0 - 1.0)

        // Performance metrics
        double loadRate;                    ///< Resources loaded per second
        double memoryEfficiency;            ///< Memory efficiency ratio
        uint32_t activeLoaders;             ///< Number of active loaders
        uint32_t pendingRequests;           ///< Pending load requests
    };

    /**
     * @class ResourceLoader
     * @brief Base class for resource loaders
     */
    class ResourceLoader {
    public:
        /**
         * @brief Constructor
         * @param type Resource type this loader handles
         */
        explicit ResourceLoader(ResourceType type);

        /**
         * @brief Destructor
         */
        virtual ~ResourceLoader() = default;

        /**
         * @brief Load resource from file
         * @param path Resource path
         * @param options Loading options
         * @return Resource handle or nullptr on failure
         */
        virtual ResourceHandle Load(const std::string& path,
                                  const std::unordered_map<std::string, std::any>& options = {}) = 0;

        /**
         * @brief Unload resource
         * @param handle Resource handle
         * @return true if unloaded successfully, false otherwise
         */
        virtual bool Unload(ResourceHandle handle) = 0;

        /**
         * @brief Get resource type
         * @return Resource type
         */
        ResourceType GetResourceType() const { return m_type; }

        /**
         * @brief Check if loader can handle file extension
         * @param extension File extension
         * @return true if can handle, false otherwise
         */
        virtual bool CanHandleExtension(const std::string& extension) const = 0;

        /**
         * @brief Get loader name
         * @return Loader name
         */
        virtual std::string GetName() const = 0;

        /**
         * @brief Get supported file extensions
         * @return Vector of supported extensions
         */
        virtual std::vector<std::string> GetSupportedExtensions() const = 0;

    private:
        ResourceType m_type;                ///< Resource type
    };

    /**
     * @class TextureLoader
     * @brief Loader for texture resources
     */
    class TextureLoader : public ResourceLoader {
    public:
        TextureLoader();

        ResourceHandle Load(const std::string& path,
                          const std::unordered_map<std::string, std::any>& options = {}) override;

        bool Unload(ResourceHandle handle) override;

        bool CanHandleExtension(const std::string& extension) const override;

        std::string GetName() const override { return "TextureLoader"; }

        std::vector<std::string> GetSupportedExtensions() const override;
    };

    /**
     * @class MeshLoader
     * @brief Loader for 3D mesh resources
     */
    class MeshLoader : public ResourceLoader {
    public:
        MeshLoader();

        ResourceHandle Load(const std::string& path,
                          const std::unordered_map<std::string, std::any>& options = {}) override;

        bool Unload(ResourceHandle handle) override;

        bool CanHandleExtension(const std::string& extension) const override;

        std::string GetName() const override { return "MeshLoader"; }

        std::vector<std::string> GetSupportedExtensions() const override;
    };

    /**
     * @class AudioLoader
     * @brief Loader for audio resources
     */
    class AudioLoader : public ResourceLoader {
    public:
        AudioLoader();

        ResourceHandle Load(const std::string& path,
                          const std::unordered_map<std::string, std::any>& options = {}) override;

        bool Unload(ResourceHandle handle) override;

        bool CanHandleExtension(const std::string& extension) const override;

        std::string GetName() const override { return "AudioLoader"; }

        std::vector<std::string> GetSupportedExtensions() const override;
    };

    /**
     * @class ResourceCache
     * @brief LRU cache for loaded resources
     */
    class ResourceCache {
    public:
        /**
         * @brief Constructor
         * @param maxSize Maximum cache size in bytes
         * @param maxItems Maximum number of cached items
         */
        explicit ResourceCache(size_t maxSize = 512 * 1024 * 1024, // 512MB
                             size_t maxItems = 1000);

        /**
         * @brief Destructor
         */
        ~ResourceCache();

        /**
         * @brief Add resource to cache
         * @param id Resource ID
         * @param handle Resource handle
         * @param info Resource information
         * @return true if added, false if cache full
         */
        bool Add(ResourceID id, ResourceHandle handle, const ResourceInfo& info);

        /**
         * @brief Get resource from cache
         * @param id Resource ID
         * @return Resource handle or nullptr if not found
         */
        ResourceHandle Get(ResourceID id);

        /**
         * @brief Remove resource from cache
         * @param id Resource ID
         * @return true if removed, false if not found
         */
        bool Remove(ResourceID id);

        /**
         * @brief Clear cache
         */
        void Clear();

        /**
         * @brief Get cache statistics
         * @return Cache statistics
         */
        std::unordered_map<std::string, size_t> GetStatistics() const;

        /**
         * @brief Set cache size limit
         * @param maxSize Maximum cache size in bytes
         */
        void SetMaxSize(size_t maxSize);

        /**
         * @brief Set maximum items limit
         * @param maxItems Maximum number of items
         */
        void SetMaxItems(size_t maxItems);

    private:
        /**
         * @brief Evict least recently used items
         * @param targetSize Target cache size
         */
        void EvictLRU(size_t targetSize);

        struct CacheEntry {
            ResourceHandle handle;          ///< Resource handle
            ResourceInfo info;              ///< Resource information
            std::chrono::steady_clock::time_point lastAccess; ///< Last access time
        };

        std::unordered_map<ResourceID, CacheEntry> m_cache;
        std::deque<ResourceID> m_accessOrder;  ///< Access order for LRU
        mutable std::shared_mutex m_cacheMutex;

        size_t m_maxSize;                    ///< Maximum cache size
        size_t m_maxItems;                   ///< Maximum items
        size_t m_currentSize;                ///< Current cache size
        size_t m_currentItems;               ///< Current item count

        // Statistics
        std::atomic<uint64_t> m_hits;        ///< Cache hits
        std::atomic<uint64_t> m_misses;      ///< Cache misses
        std::atomic<uint64_t> m_evictions;   ///< Cache evictions
    };

    /**
     * @class ResourceManager
     * @brief Central resource management system
     */
    class ResourceManager {
    public:
        /**
         * @brief Constructor
         */
        ResourceManager();

        /**
         * @brief Destructor
         */
        ~ResourceManager();

        /**
         * @brief Deleted copy constructor
         */
        ResourceManager(const ResourceManager&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        ResourceManager& operator=(const ResourceManager&) = delete;

        // Resource loading

        /**
         * @brief Load resource synchronously
         * @param path Resource path
         * @param type Resource type
         * @param priority Loading priority
         * @return Resource handle or nullptr on failure
         */
        ResourceHandle Load(const std::string& path,
                          ResourceType type = ResourceType::Binary,
                          ResourcePriority priority = ResourcePriority::Normal);

        /**
         * @brief Load resource asynchronously
         * @param path Resource path
         * @param type Resource type
         * @param priority Loading priority
         * @return Future resource handle
         */
        std::future<ResourceHandle> LoadAsync(const std::string& path,
                                            ResourceType type = ResourceType::Binary,
                                            ResourcePriority priority = ResourcePriority::Normal);

        /**
         * @brief Load resource with callbacks
         * @param request Load request
         * @return Resource ID for tracking
         */
        ResourceID LoadWithCallback(const ResourceLoadRequest& request);

        /**
         * @brief Unload resource
         * @param handle Resource handle
         * @return true if unloaded, false otherwise
         */
        bool Unload(ResourceHandle handle);

        /**
         * @brief Unload resource by path
         * @param path Resource path
         * @return true if unloaded, false otherwise
         */
        bool Unload(const std::string& path);

        // Resource access

        /**
         * @brief Get resource by path
         * @param path Resource path
         * @return Resource handle or nullptr if not loaded
         */
        ResourceHandle Get(const std::string& path);

        /**
         * @brief Get resource by ID
         * @param id Resource ID
         * @return Resource handle or nullptr if not found
         */
        ResourceHandle Get(ResourceID id);

        /**
         * @brief Check if resource is loaded
         * @param path Resource path
         * @return true if loaded, false otherwise
         */
        bool IsLoaded(const std::string& path) const;

        /**
         * @brief Get resource information
         * @param path Resource path
         * @return Resource information or empty optional
         */
        std::optional<ResourceInfo> GetResourceInfo(const std::string& path) const;

        // Resource management

        /**
         * @brief Preload resources
         * @param paths Vector of resource paths
         * @param type Resource type
         * @return Number of resources queued for loading
         */
        size_t Preload(const std::vector<std::string>& paths,
                      ResourceType type = ResourceType::Binary);

        /**
         * @brief Unload unused resources
         * @return Number of resources unloaded
         */
        size_t UnloadUnused();

        /**
         * @brief Cleanup all resources
         */
        void Cleanup();

        // Loader management

        /**
         * @brief Register resource loader
         * @param loader Loader to register
         * @return true if registered, false if type already has loader
         */
        bool RegisterLoader(std::unique_ptr<ResourceLoader> loader);

        /**
         * @brief Unregister resource loader
         * @param type Resource type
         * @return true if unregistered, false if not found
         */
        bool UnregisterLoader(ResourceType type);

        /**
         * @brief Get loader for resource type
         * @param type Resource type
         * @return Loader pointer or nullptr if not found
         */
        ResourceLoader* GetLoader(ResourceType type);

        // Cache management

        /**
         * @brief Enable/disable resource caching
         * @param enabled Enable state
         */
        void SetCacheEnabled(bool enabled);

        /**
         * @brief Clear resource cache
         */
        void ClearCache();

        /**
         * @brief Get cache statistics
         * @return Cache statistics
         */
        std::unordered_map<std::string, size_t> GetCacheStatistics() const;

        // Resource monitoring

        /**
         * @brief Get resource statistics
         * @return Resource statistics
         */
        ResourceStatistics GetStatistics() const;

        /**
         * @brief Get all loaded resources
         * @return Vector of resource information
         */
        std::vector<ResourceInfo> GetLoadedResources() const;

        /**
         * @brief Get resource loading progress
         * @param path Resource path
         * @return Loading progress (0.0 - 1.0) or -1.0 if not loading
         */
        double GetLoadingProgress(const std::string& path) const;

        // Hot-reloading

        /**
         * @brief Enable hot-reloading for resource type
         * @param type Resource type
         * @param enabled Enable state
         */
        void SetHotReloadEnabled(ResourceType type, bool enabled);

        /**
         * @brief Check if file has changed and reload if necessary
         * @param path Resource path
         * @return true if reloaded, false otherwise
         */
        bool CheckAndReload(const std::string& path);

        /**
         * @brief Set hot-reload callback
         * @param callback Callback function
         */
        void SetHotReloadCallback(std::function<void(const std::string&)> callback);

        // Resource streaming

        /**
         * @brief Enable streaming for resource
         * @param path Resource path
         * @param enabled Enable state
         * @return true if successful, false otherwise
         */
        bool SetStreamingEnabled(const std::string& path, bool enabled);

        /**
         * @brief Update streaming resources
         * @param cameraPos Camera position for streaming priority
         * @param loadDistance Load distance
         * @param unloadDistance Unload distance
         */
        void UpdateStreaming(const glm::vec3& cameraPos,
                           float loadDistance,
                           float unloadDistance);

        // Utility functions

        /**
         * @brief Add resource search path
         * @param path Search path
         */
        void AddSearchPath(const std::string& path);

        /**
         * @brief Remove resource search path
         * @param path Search path
         */
        void RemoveSearchPath(const std::string& path);

        /**
         * @brief Find resource file
         * @param name Resource name
         * @param type Resource type
         * @return Full path or empty string if not found
         */
        std::string FindResource(const std::string& name, ResourceType type) const;

        /**
         * @brief Get resource file modification time
         * @param path Resource path
         * @return Modification time or empty optional if not found
         */
        std::optional<std::filesystem::file_time_type> GetFileModificationTime(const std::string& path) const;

    private:
        /**
         * @brief Process load requests
         */
        void ProcessLoadRequests();

        /**
         * @brief Background loading thread function
         */
        void BackgroundLoadingThread();

        /**
         * @brief Load resource with specific loader
         * @param request Load request
         * @return Resource handle or nullptr on failure
         */
        ResourceHandle LoadWithLoader(const ResourceLoadRequest& request);

        /**
         * @brief Get or create resource ID for path
         * @param path Resource path
         * @return Resource ID
         */
        ResourceID GetOrCreateResourceID(const std::string& path);

        // Resource storage
        std::unordered_map<ResourceID, ResourceInfo> m_resources;
        std::unordered_map<std::string, ResourceID> m_pathToId;
        std::unordered_map<ResourceID, ResourceHandle> m_handles;
        mutable std::shared_mutex m_resourceMutex;

        // Resource loaders
        std::unordered_map<ResourceType, std::unique_ptr<ResourceLoader>> m_loaders;
        mutable std::shared_mutex m_loaderMutex;

        // Resource cache
        std::unique_ptr<ResourceCache> m_cache;
        bool m_cacheEnabled;

        // Loading system
        std::queue<ResourceLoadRequest> m_loadQueue;
        std::unordered_map<ResourceID, std::future<ResourceHandle>> m_pendingLoads;
        mutable std::shared_mutex m_loadMutex;
        std::condition_variable_any m_loadCondition;
        std::atomic<bool> m_loadingThreadRunning;
        std::thread m_loadingThread;

        // Search paths
        std::vector<std::string> m_searchPaths;
        mutable std::mutex m_searchPathMutex;

        // Hot-reloading
        std::unordered_map<ResourceType, bool> m_hotReloadEnabled;
        std::unordered_map<std::string, std::filesystem::file_time_type> m_fileTimestamps;
        std::function<void(const std::string&)> m_hotReloadCallback;
        mutable std::mutex m_hotReloadMutex;

        // Statistics
        ResourceStatistics m_statistics;
        mutable std::mutex m_statsMutex;

        // Configuration
        size_t m_maxConcurrentLoads;         ///< Maximum concurrent loads
        size_t m_maxCacheSize;               ///< Maximum cache size
        bool m_autoStreaming;                ///< Auto-streaming enabled
        double m_cacheCleanupInterval;       ///< Cache cleanup interval

        // Next resource ID
        std::atomic<ResourceID> m_nextResourceId;
    };

    /**
     * @brief Get global resource manager instance
     * @return Resource manager reference
     */
    ResourceManager& GetResourceManager();

    // Resource loading macros

    /**
     * @def VOXELCRAFT_LOAD_TEXTURE(path)
     * @brief Load texture resource
     */
    #define VOXELCRAFT_LOAD_TEXTURE(path) \
        GetResourceManager().Load(path, ResourceType::Texture)

    /**
     * @def VOXELCRAFT_LOAD_MESH(path)
     * @brief Load mesh resource
     */
    #define VOXELCRAFT_LOAD_MESH(path) \
        GetResourceManager().Load(path, ResourceType::Mesh)

    /**
     * @def VOXELCRAFT_LOAD_AUDIO(path)
     * @brief Load audio resource
     */
    #define VOXELCRAFT_LOAD_AUDIO(path) \
        GetResourceManager().Load(path, ResourceType::Audio)

    /**
     * @def VOXELCRAFT_LOAD_ASYNC(path, type)
     * @brief Load resource asynchronously
     */
    #define VOXELCRAFT_LOAD_ASYNC(path, type) \
        GetResourceManager().LoadAsync(path, type)

} // namespace VoxelCraft

#endif // VOXELCRAFT_CORE_RESOURCE_MANAGER_HPP
