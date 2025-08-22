/**
 * @file ResourceManager.hpp
 * @brief VoxelCraft Resource Management System
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_CORE_RESOURCE_MANAGER_HPP
#define VOXELCRAFT_CORE_RESOURCE_MANAGER_HPP

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <atomic>
#include <thread>
#include <functional>
#include <chrono>

namespace VoxelCraft {

// Forward declarations
class Config;

/**
 * @enum ResourceType
 * @brief Types of resources that can be managed
 */
enum class ResourceType {
    TEXTURE = 0,    ///< Image/texture resources
    MODEL,          ///< 3D model resources
    SHADER,         ///< Shader program resources
    AUDIO,          ///< Audio/sound resources
    FONT,           ///< Font resources
    CONFIG,         ///< Configuration file resources
    SCRIPT,         ///< Script resources
    CUSTOM          ///< Custom resource types
};

/**
 * @enum ResourceState
 * @brief Current state of a resource
 */
enum class ResourceState {
    UNLOADED = 0,   ///< Resource not yet loaded
    LOADING,        ///< Resource is currently being loaded
    LOADED,         ///< Resource is loaded and ready to use
    UNLOADING,      ///< Resource is being unloaded
    ERROR           ///< Resource loading failed
};

/**
 * @struct ResourceMetrics
 * @brief Resource system performance metrics
 */
struct ResourceMetrics {
    uint64_t totalResources;          ///< Total number of resources
    uint64_t loadedResources;         ///< Number of currently loaded resources
    uint64_t loadingResources;        ///< Number of resources currently loading
    size_t memoryUsage;               ///< Current memory usage by resources
    size_t peakMemoryUsage;           ///< Peak memory usage
    uint64_t cacheHits;               ///< Number of cache hits
    uint64_t cacheMisses;             ///< Number of cache misses
    double averageLoadTime;           ///< Average resource load time
    uint32_t activeThreads;           ///< Number of active loading threads
};

/**
 * @class Resource
 * @brief Base class for all resources
 */
class Resource {
public:
    /**
     * @brief Constructor
     * @param name Resource name
     * @param type Resource type
     */
    Resource(const std::string& name, ResourceType type);

    /**
     * @brief Destructor
     */
    virtual ~Resource() = default;

    /**
     * @brief Get resource name
     * @return Resource name
     */
    const std::string& GetName() const { return m_name; }

    /**
     * @brief Get resource type
     * @return Resource type
     */
    ResourceType GetType() const { return m_type; }

    /**
     * @brief Get resource state
     * @return Resource state
     */
    ResourceState GetState() const { return m_state; }

    /**
     * @brief Get memory usage
     * @return Memory usage in bytes
     */
    virtual size_t GetMemoryUsage() const = 0;

    /**
     * @brief Check if resource is loaded
     * @return true if loaded, false otherwise
     */
    bool IsLoaded() const { return m_state == ResourceState::LOADED; }

    /**
     * @brief Get last access time
     * @return Last access time
     */
    std::chrono::steady_clock::time_point GetLastAccessTime() const { return m_lastAccessTime; }

protected:
    /**
     * @brief Load resource (implemented by derived classes)
     * @return true if successful, false otherwise
     */
    virtual bool Load() = 0;

    /**
     * @brief Unload resource (implemented by derived classes)
     * @return true if successful, false otherwise
     */
    virtual bool Unload() = 0;

    std::string m_name;                                    ///< Resource name
    ResourceType m_type;                                   ///< Resource type
    std::atomic<ResourceState> m_state;                   ///< Resource state
    std::chrono::steady_clock::time_point m_lastAccessTime; ///< Last access time
    std::chrono::steady_clock::time_point m_loadTime;      ///< Load completion time
};

/**
 * @class ResourceManager
 * @brief Main resource management system
 *
 * The ResourceManager handles:
 * - Asynchronous resource loading
 * - Resource caching and reuse
 * - Memory management for resources
 * - Hot-reloading of resources
 * - Resource dependency management
 * - Performance monitoring
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

    /**
     * @brief Initialize the resource manager
     * @param config Resource configuration
     * @return true if successful, false otherwise
     */
    bool Initialize(const Config& config);

    /**
     * @brief Shutdown the resource manager
     */
    void Shutdown();

    /**
     * @brief Update resource manager state
     * @param deltaTime Time elapsed since last update
     */
    void Update(double deltaTime);

    /**
     * @brief Get resource metrics
     * @return Resource performance metrics
     */
    const ResourceMetrics& GetMetrics() const { return m_metrics; }

    /**
     * @brief Load resource synchronously
     * @param name Resource name
     * @param type Resource type
     * @return Resource pointer, nullptr on failure
     */
    std::shared_ptr<Resource> LoadResource(const std::string& name, ResourceType type);

    /**
     * @brief Load resource asynchronously
     * @param name Resource name
     * @param type Resource type
     * @param callback Completion callback
     */
    void LoadResourceAsync(const std::string& name, ResourceType type,
                          std::function<void(std::shared_ptr<Resource>)> callback);

    /**
     * @brief Get loaded resource
     * @param name Resource name
     * @param type Resource type
     * @return Resource pointer, nullptr if not found
     */
    std::shared_ptr<Resource> GetResource(const std::string& name, ResourceType type);

    /**
     * @brief Unload resource
     * @param name Resource name
     * @param type Resource type
     * @return true if successful, false otherwise
     */
    bool UnloadResource(const std::string& name, ResourceType type);

    /**
     * @brief Unload all resources
     */
    void UnloadAllResources();

    /**
     * @brief Preload resources from list
     * @param resourceList List of resource names and types
     * @return true if all loaded successfully
     */
    bool PreloadResources(const std::vector<std::pair<std::string, ResourceType>>& resourceList);

    /**
     * @brief Check if resource exists
     * @param name Resource name
     * @param type Resource type
     * @return true if exists, false otherwise
     */
    bool ResourceExists(const std::string& name, ResourceType type) const;

    /**
     * @brief Get resource loading progress
     * @param name Resource name
     * @param type Resource type
     * @return Progress (0-1), -1 if not loading
     */
    float GetResourceLoadProgress(const std::string& name, ResourceType type) const;

    /**
     * @brief Set maximum cache size
     * @param maxSize Maximum cache size in bytes
     */
    void SetMaxCacheSize(size_t maxSize) { m_maxCacheSize = maxSize; }

    /**
     * @brief Get maximum cache size
     * @return Maximum cache size in bytes
     */
    size_t GetMaxCacheSize() const { return m_maxCacheSize; }

    /**
     * @brief Set resource load timeout
     * @param timeout Timeout in seconds
     */
    void SetLoadTimeout(double timeout) { m_loadTimeout = timeout; }

    /**
     * @brief Get resource load timeout
     * @return Timeout in seconds
     */
    double GetLoadTimeout() const { return m_loadTimeout; }

    /**
     * @brief Enable/disable hot reloading
     * @param enabled Enable hot reloading
     */
    void SetHotReloadingEnabled(bool enabled) { m_hotReloadingEnabled = enabled; }

    /**
     * @brief Check if hot reloading is enabled
     * @return true if enabled
     */
    bool IsHotReloadingEnabled() const { return m_hotReloadingEnabled; }

    /**
     * @brief Add resource search path
     * @param path Search path
     */
    void AddSearchPath(const std::string& path);

    /**
     * @brief Remove resource search path
     * @param path Search path to remove
     */
    void RemoveSearchPath(const std::string& path);

    /**
     * @brief Get all search paths
     * @return Vector of search paths
     */
    std::vector<std::string> GetSearchPaths() const;

private:
    // Resource storage
    std::unordered_map<std::string, std::shared_ptr<Resource>> m_resources;
    mutable std::mutex m_resourcesMutex;

    // Loading queue
    struct LoadRequest {
        std::string name;
        ResourceType type;
        std::function<void(std::shared_ptr<Resource>)> callback;
        std::chrono::steady_clock::time_point startTime;
    };
    std::vector<LoadRequest> m_loadQueue;
    mutable std::mutex m_loadQueueMutex;

    // Search paths
    std::vector<std::string> m_searchPaths;
    mutable std::mutex m_searchPathsMutex;

    // Metrics
    ResourceMetrics m_metrics;
    mutable std::mutex m_metricsMutex;

    // Configuration
    size_t m_maxCacheSize;           ///< Maximum cache size in bytes
    double m_loadTimeout;            ///< Load timeout in seconds
    bool m_hotReloadingEnabled;      ///< Enable hot reloading
    uint32_t m_maxLoadingThreads;    ///< Maximum loading threads

    // Threading
    std::unique_ptr<std::thread> m_loadingThread; ///< Resource loading thread
    std::atomic<bool> m_loadingThreadRunning; ///< Loading thread running flag

    /**
     * @brief Resource loading thread function
     */
    void LoadingThread();

    /**
     * @brief Process load request
     * @param request Load request to process
     * @return Loaded resource, nullptr on failure
     */
    std::shared_ptr<Resource> ProcessLoadRequest(const LoadRequest& request);

    /**
     * @brief Create resource instance
     * @param name Resource name
     * @param type Resource type
     * @return Resource instance, nullptr on failure
     */
    std::shared_ptr<Resource> CreateResource(const std::string& name, ResourceType type);

    /**
     * @brief Find resource file path
     * @param name Resource name
     * @param type Resource type
     * @return File path, empty if not found
     */
    std::string FindResourcePath(const std::string& name, ResourceType type);

    /**
     * @brief Check cache size and evict if necessary
     */
    void CheckCacheSize();

    /**
     * @brief Update resource metrics
     */
    void UpdateMetrics();

    /**
     * @brief Clean up timed out load requests
     */
    void CleanupTimedOutRequests();
};

} // namespace VoxelCraft

#endif // VOXELCRAFT_CORE_RESOURCE_MANAGER_HPP