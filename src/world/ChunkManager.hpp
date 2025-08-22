/**
 * @file ChunkManager.hpp
 * @brief VoxelCraft Chunk Management System
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines the ChunkManager class that handles efficient loading,
 * unloading, caching, and streaming of chunks in the voxel world.
 */

#ifndef VOXELCRAFT_WORLD_CHUNK_MANAGER_HPP
#define VOXELCRAFT_WORLD_CHUNK_MANAGER_HPP

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <vector>
#include <deque>
#include <atomic>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include <thread>
#include <functional>
#include <chrono>

#include "../core/Config.hpp"
#include "World.hpp"
#include "Chunk.hpp"

namespace VoxelCraft {

    // Forward declarations
    class World;
    class Chunk;
    class TerrainGenerator;

    /**
     * @enum ChunkLoadPriority
     * @brief Priority levels for chunk loading
     */
    enum class ChunkLoadPriority {
        Critical,           ///< Critical chunks (player position)
        High,               ///< High priority chunks
        Normal,             ///< Normal priority chunks
        Low,                ///< Low priority chunks
        Background          ///< Background loading only
    };

    /**
     * @enum ChunkState
     * @brief State of a chunk in the management system
     */
    enum class ChunkManagementState {
        Unloaded,           ///< Chunk not loaded
        QueuedForLoad,      ///< Chunk queued for loading
        Loading,            ///< Chunk is being loaded
        Loaded,             ///< Chunk is loaded and ready
        QueuedForUnload,    ///< Chunk queued for unloading
        Unloading,          ///< Chunk is being unloaded
        Error               ///< Chunk loading failed
    };

    /**
     * @struct ChunkLoadRequest
     * @brief Request to load a chunk
     */
    struct ChunkLoadRequest {
        ChunkCoordinate coord;              ///< Chunk coordinates
        ChunkLoadPriority priority;         ///< Loading priority
        double timestamp;                   ///< Request timestamp
        std::function<void(Chunk*)> onComplete; ///< Completion callback
        std::function<void(const std::string&)> onError; ///< Error callback
        bool generateIfMissing;             ///< Generate terrain if chunk doesn't exist
        std::unordered_map<std::string, std::any> options; ///< Loading options
    };

    /**
     * @struct ChunkInfo
     * @brief Information about a managed chunk
     */
    struct ChunkManagementInfo {
        ChunkCoordinate coord;              ///< Chunk coordinates
        ChunkManagementState state;         ///< Current management state
        ChunkLoadPriority priority;         ///< Loading priority
        double lastAccessed;                ///< Last access timestamp
        double loadTime;                    ///< Time when chunk was loaded
        size_t memoryUsage;                 ///< Memory usage
        bool needsSave;                     ///< Chunk needs to be saved
        bool isGenerated;                   ///< Chunk has generated terrain
        bool isLit;                         ///< Chunk lighting is calculated
        int accessCount;                    ///< Number of times accessed
        std::string errorMessage;           ///< Error message if failed
    };

    /**
     * @struct ChunkManagerConfig
     * @brief Configuration for the chunk manager
     */
    struct ChunkManagerConfig {
        // Distance settings
        int viewDistance;                   ///< View distance in chunks
        int simulationDistance;             ///< Simulation distance in chunks
        int loadDistance;                   ///< Load distance in chunks
        int unloadDistance;                 ///< Unload distance in chunks

        // Performance settings
        size_t maxLoadedChunks;             ///< Maximum loaded chunks
        size_t maxPendingLoads;             ///< Maximum pending loads
        size_t maxPendingUnloads;           ///< Maximum pending unloads
        int workerThreads;                  ///< Number of worker threads
        bool enableMultithreading;          ///< Enable multithreaded operations

        // Timing settings
        double loadTimeout;                 ///< Load timeout (seconds)
        double unloadTimeout;               ///< Unload timeout (seconds)
        double accessTimeout;               ///< Access timeout for LRU (seconds)
        double saveInterval;                ///< Auto-save interval (seconds)

        // Memory settings
        size_t maxMemoryUsage;              ///< Maximum memory usage (MB)
        bool enableCompression;             ///< Enable chunk compression
        bool enableStreaming;               ///< Enable chunk streaming

        // Caching settings
        bool enableCache;                   ///< Enable chunk caching
        size_t maxCacheSize;                ///< Maximum cache size
        double cacheExpirationTime;         ///< Cache expiration time (seconds)

        // Generation settings
        bool generateMissingChunks;         ///< Generate missing chunks
        bool saveGeneratedChunks;           ///< Save generated chunks
        int generationBatchSize;            ///< Generation batch size
    };

    /**
     * @struct ChunkManagerMetrics
     * @brief Performance metrics for the chunk manager
     */
    struct ChunkManagerMetrics {
        // Chunk counts
        uint32_t totalLoadedChunks;         ///< Currently loaded chunks
        uint32_t pendingLoadChunks;         ///< Chunks waiting to load
        uint32_t pendingUnloadChunks;       ///< Chunks waiting to unload
        uint32_t cachedChunks;              ///< Chunks in cache

        // Performance metrics
        double averageLoadTime;             ///< Average chunk load time (ms)
        double averageUnloadTime;           ///< Average chunk unload time (ms)
        double averageGenerationTime;       ///< Average chunk generation time (ms)
        uint64_t totalLoads;                ///< Total chunk loads
        uint64_t totalUnloads;              ///< Total chunk unloads
        uint64_t totalGenerations;          ///< Total chunk generations

        // Memory metrics
        size_t memoryUsage;                 ///< Current memory usage (bytes)
        size_t peakMemoryUsage;             ///< Peak memory usage (bytes)
        size_t cacheMemoryUsage;            ///< Cache memory usage (bytes)

        // Threading metrics
        uint32_t activeThreads;             ///< Active worker threads
        uint32_t queuedTasks;               ///< Tasks in queue
        double threadUtilization;           ///< Thread utilization (0.0 - 1.0)

        // Error metrics
        uint64_t loadFailures;              ///< Number of load failures
        uint64_t generationFailures;        ///< Number of generation failures
        uint64_t saveFailures;              ///< Number of save failures
    };

    /**
     * @class ChunkCache
     * @brief LRU cache for chunk data
     */
    class ChunkCache {
    public:
        /**
         * @brief Constructor
         * @param maxSize Maximum cache size in chunks
         * @param expirationTime Cache expiration time (seconds)
         */
        explicit ChunkCache(size_t maxSize = 1000, double expirationTime = 300.0);

        /**
         * @brief Add chunk to cache
         * @param coord Chunk coordinates
         * @param chunk Chunk data
         * @return true if added, false if cache full
         */
        bool Add(const ChunkCoordinate& coord, std::unique_ptr<Chunk> chunk);

        /**
         * @brief Get chunk from cache
         * @param coord Chunk coordinates
         * @return Chunk pointer or nullptr if not found
         */
        std::unique_ptr<Chunk> Get(const ChunkCoordinate& coord);

        /**
         * @brief Remove chunk from cache
         * @param coord Chunk coordinates
         * @return true if removed, false if not found
         */
        bool Remove(const ChunkCoordinate& coord);

        /**
         * @brief Clear cache
         */
        void Clear();

        /**
         * @brief Get cache size
         * @return Number of cached chunks
         */
        size_t Size() const;

        /**
         * @brief Get cache statistics
         * @return Cache statistics
         */
        std::unordered_map<std::string, size_t> GetStatistics() const;

        /**
         * @brief Cleanup expired entries
         * @return Number of entries removed
         */
        size_t CleanupExpired();

    private:
        struct CacheEntry {
            std::unique_ptr<Chunk> chunk;
            double timestamp;
            size_t accessCount;
        };

        std::unordered_map<ChunkCoordinate, CacheEntry> m_cache;
        std::deque<ChunkCoordinate> m_accessOrder;
        mutable std::shared_mutex m_cacheMutex;

        size_t m_maxSize;
        double m_expirationTime;
        size_t m_hits;
        size_t m_misses;
        size_t m_evictions;
    };

    /**
     * @class ChunkManager
     * @brief Central chunk management system
     *
     * The ChunkManager is responsible for:
     * - Loading and unloading chunks on demand
     * - Managing chunk lifecycle and memory usage
     * - Coordinating terrain generation
     * - Implementing LOD (Level of Detail) system
     * - Providing efficient chunk streaming
     * - Managing chunk caching and persistence
     * - Handling multi-threaded chunk operations
     * - Optimizing chunk access patterns
     *
     * Key Features:
     * - Dynamic loading based on player position
     * - Priority-based loading queue
     * - Memory-aware unloading
     * - Chunk compression and streaming
     * - LOD system for distant chunks
     * - Multi-threaded generation and loading
     * - Cache system for frequently accessed chunks
     */
    class ChunkManager {
    public:
        /**
         * @brief Constructor
         * @param config Chunk manager configuration
         */
        explicit ChunkManager(const ChunkManagerConfig& config);

        /**
         * @brief Destructor
         */
        ~ChunkManager();

        /**
         * @brief Deleted copy constructor
         */
        ChunkManager(const ChunkManager&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        ChunkManager& operator=(const ChunkManager&) = delete;

        // Manager lifecycle

        /**
         * @brief Initialize chunk manager
         * @param world Parent world
         * @param generator Terrain generator
         * @return true if successful, false otherwise
         */
        bool Initialize(World* world, TerrainGenerator* generator);

        /**
         * @brief Shutdown chunk manager
         */
        void Shutdown();

        /**
         * @brief Update chunk manager
         * @param deltaTime Time elapsed since last update
         * @param cameraPos Camera position for loading priority
         */
        void Update(double deltaTime, const glm::vec3& cameraPos);

        // Chunk access

        /**
         * @brief Get chunk at coordinates
         * @param x Chunk X coordinate
         * @param z Chunk Z coordinate
         * @return Chunk pointer or nullptr if not loaded
         */
        Chunk* GetChunk(int x, int z);

        /**
         * @brief Get chunk at coordinates (const version)
         * @param x Chunk X coordinate
         * @param z Chunk Z coordinate
         * @return Chunk pointer or nullptr if not loaded
         */
        const Chunk* GetChunk(int x, int z) const;

        /**
         * @brief Check if chunk is loaded
         * @param x Chunk X coordinate
         * @param z Chunk Z coordinate
         * @return true if loaded, false otherwise
         */
        bool IsChunkLoaded(int x, int z) const;

        /**
         * @brief Get chunk management info
         * @param x Chunk X coordinate
         * @param z Chunk Z coordinate
         * @return Chunk info or empty optional if not found
         */
        std::optional<ChunkManagementInfo> GetChunkInfo(int x, int z) const;

        // Chunk loading

        /**
         * @brief Request chunk load
         * @param request Load request
         * @return true if request queued, false otherwise
         */
        bool RequestChunkLoad(const ChunkLoadRequest& request);

        /**
         * @brief Request chunk load with coordinates
         * @param x Chunk X coordinate
         * @param z Chunk Z coordinate
         * @param priority Loading priority
         * @return true if request queued, false otherwise
         */
        bool RequestChunkLoad(int x, int z, ChunkLoadPriority priority = ChunkLoadPriority::Normal);

        /**
         * @brief Cancel chunk load request
         * @param x Chunk X coordinate
         * @param z Chunk Z coordinate
         * @return true if cancelled, false if not found
         */
        bool CancelChunkLoad(int x, int z);

        // Chunk unloading

        /**
         * @brief Request chunk unload
         * @param x Chunk X coordinate
         * @param z Chunk Z coordinate
         * @return true if request queued, false otherwise
         */
        bool RequestChunkUnload(int x, int z);

        /**
         * @brief Force immediate chunk unload
         * @param x Chunk X coordinate
         * @param z Chunk Z coordinate
         * @return true if unloaded, false if not found
         */
        bool ForceUnloadChunk(int x, int z);

        // Chunk management

        /**
         * @brief Save all modified chunks
         * @return Number of chunks saved
         */
        size_t SaveAllChunks();

        /**
         * @brief Save specific chunk
         * @param x Chunk X coordinate
         * @param z Chunk Z coordinate
         * @return true if saved, false otherwise
         */
        bool SaveChunk(int x, int z);

        /**
         * @brief Load chunk from disk
         * @param x Chunk X coordinate
         * @param z Chunk Z coordinate
         * @return true if loaded, false otherwise
         */
        bool LoadChunkFromDisk(int x, int z);

        /**
         * @brief Generate new chunk
         * @param x Chunk X coordinate
         * @param z Chunk Z coordinate
         * @return Generated chunk or nullptr on failure
         */
        std::unique_ptr<Chunk> GenerateChunk(int x, int z);

        // Configuration

        /**
         * @brief Get chunk manager configuration
         * @return Current configuration
         */
        const ChunkManagerConfig& GetConfig() const { return m_config; }

        /**
         * @brief Set chunk manager configuration
         * @param config New configuration
         */
        void SetConfig(const ChunkManagerConfig& config);

        /**
         * @brief Get chunk manager metrics
         * @return Performance metrics
         */
        const ChunkManagerMetrics& GetMetrics() const { return m_metrics; }

        /**
         * @brief Reset metrics
         */
        void ResetMetrics();

        // Distance management

        /**
         * @brief Set view distance
         * @param distance View distance in chunks
         */
        void SetViewDistance(int distance);

        /**
         * @brief Get view distance
         * @return View distance in chunks
         */
        int GetViewDistance() const { return m_config.viewDistance; }

        /**
         * @brief Set simulation distance
         * @param distance Simulation distance in chunks
         */
        void SetSimulationDistance(int distance);

        /**
         * @brief Get simulation distance
         * @return Simulation distance in chunks
         */
        int GetSimulationDistance() const { return m_config.simulationDistance; }

        // Statistics and debugging

        /**
         * @brief Get loaded chunks count
         * @return Number of loaded chunks
         */
        size_t GetLoadedChunksCount() const;

        /**
         * @brief Get pending operations count
         * @return Number of pending operations
         */
        size_t GetPendingOperationsCount() const;

        /**
         * @brief Get memory usage
         * @return Memory usage in bytes
         */
        size_t GetMemoryUsage() const;

        /**
         * @brief Get chunk loading progress
         * @param x Chunk X coordinate
         * @param z Chunk Z coordinate
         * @return Loading progress (0.0 - 1.0) or -1.0 if not loading
         */
        double GetChunkLoadingProgress(int x, int z) const;

        /**
         * @brief Get chunk manager status report
         * @return Detailed status report
         */
        std::string GetStatusReport() const;

    private:
        /**
         * @brief Initialize worker threads
         * @return true if successful, false otherwise
         */
        bool InitializeWorkerThreads();

        /**
         * @brief Worker thread function
         * @param threadId Worker thread ID
         */
        void WorkerThreadFunction(int threadId);

        /**
         * @brief Process pending load requests
         */
        void ProcessLoadRequests();

        /**
         * @brief Process pending unload requests
         */
        void ProcessUnloadRequests();

        /**
         * @brief Update chunk loading priorities
         * @param cameraPos Camera position
         */
        void UpdateLoadingPriorities(const glm::vec3& cameraPos);

        /**
         * @brief Perform chunk loading
         * @param request Load request
         */
        void PerformChunkLoad(const ChunkLoadRequest& request);

        /**
         * @brief Perform chunk unloading
         * @param coord Chunk coordinates
         */
        void PerformChunkUnload(const ChunkCoordinate& coord);

        /**
         * @brief Generate chunk terrain
         * @param coord Chunk coordinates
         * @return Generated chunk
         */
        std::unique_ptr<Chunk> GenerateChunkTerrain(const ChunkCoordinate& coord);

        /**
         * @brief Load chunk from disk
         * @param coord Chunk coordinates
         * @return Loaded chunk or nullptr on failure
         */
        std::unique_ptr<Chunk> LoadChunkFromDisk(const ChunkCoordinate& coord);

        /**
         * @brief Save chunk to disk
         * @param chunk Chunk to save
         * @return true if saved, false otherwise
         */
        bool SaveChunkToDisk(Chunk* chunk);

        /**
         * @brief Update chunk LOD based on distance
         * @param coord Chunk coordinates
         * @param distance Distance from camera
         */
        void UpdateChunkLOD(const ChunkCoordinate& coord, float distance);

        /**
         * @brief Check if chunk should be unloaded
         * @param coord Chunk coordinates
         * @param cameraPos Camera position
         * @return true if should unload, false otherwise
         */
        bool ShouldUnloadChunk(const ChunkCoordinate& coord, const glm::vec3& cameraPos);

        /**
         * @brief Calculate chunk loading priority
         * @param coord Chunk coordinates
         * @param cameraPos Camera position
         * @return Loading priority
         */
        ChunkLoadPriority CalculateLoadingPriority(const ChunkCoordinate& coord, const glm::vec3& cameraPos);

        /**
         * @brief Update manager metrics
         * @param deltaTime Time elapsed
         */
        void UpdateMetrics(double deltaTime);

        /**
         * @brief Cleanup expired cache entries
         */
        void CleanupCache();

        /**
         * @brief Handle chunk load completion
         * @param coord Chunk coordinates
         * @param chunk Loaded chunk
         */
        void OnChunkLoadComplete(const ChunkCoordinate& coord, std::unique_ptr<Chunk> chunk);

        /**
         * @brief Handle chunk load failure
         * @param coord Chunk coordinates
         * @param error Error message
         */
        void OnChunkLoadFailed(const ChunkCoordinate& coord, const std::string& error);

        // Configuration and state
        ChunkManagerConfig m_config;                    ///< Manager configuration
        ChunkManagerMetrics m_metrics;                  ///< Performance metrics
        World* m_world;                                ///< Parent world
        TerrainGenerator* m_generator;                 ///< Terrain generator

        // Chunk storage
        std::unordered_map<ChunkCoordinate, std::unique_ptr<Chunk>> m_loadedChunks;
        std::unordered_map<ChunkCoordinate, ChunkManagementInfo> m_chunkInfo;
        mutable std::shared_mutex m_chunksMutex;       ///< Chunk synchronization

        // Cache system
        std::unique_ptr<ChunkCache> m_cache;           ///< Chunk cache

        // Request queues
        std::deque<ChunkLoadRequest> m_loadQueue;      ///< Load request queue
        std::deque<ChunkCoordinate> m_unloadQueue;     ///< Unload request queue
        mutable std::mutex m_queuesMutex;              ///< Queue synchronization

        // Worker threads
        std::vector<std::thread> m_workerThreads;      ///< Worker threads
        std::atomic<bool> m_workersRunning;            ///< Worker threads running flag
        std::condition_variable_any m_workerCondition; ///< Worker condition variable

        // Pending operations tracking
        std::unordered_set<ChunkCoordinate> m_pendingLoads;    ///< Pending load operations
        std::unordered_set<ChunkCoordinate> m_pendingUnloads;  ///< Pending unload operations
        mutable std::shared_mutex m_pendingMutex;      ///< Pending operations synchronization

        // Statistics
        double m_lastUpdateTime;                       ///< Last update timestamp
        double m_lastCacheCleanupTime;                 ///< Last cache cleanup time
        size_t m_totalChunksLoaded;                    ///< Total chunks loaded
        size_t m_totalChunksUnloaded;                  ///< Total chunks unloaded
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_WORLD_CHUNK_MANAGER_HPP
