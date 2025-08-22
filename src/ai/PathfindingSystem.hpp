/**
 * @file PathfindingSystem.hpp
 * @brief VoxelCraft Pathfinding System - A* Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines the PathfindingSystem class that provides advanced
 * A* pathfinding capabilities optimized for voxel-based worlds with
 * support for multi-level navigation, dynamic obstacles, and performance
 * optimizations for large-scale environments.
 */

#ifndef VOXELCRAFT_AI_PATHFINDING_SYSTEM_HPP
#define VOXELCRAFT_AI_PATHFINDING_SYSTEM_HPP

#include <memory>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <string>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <chrono>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../core/Config.hpp"
#include "../world/World.hpp"
#include "../physics/PhysicsTypes.hpp"

namespace VoxelCraft {

    // Forward declarations
    class Mob;
    class NavigationSystem;

    /**
     * @enum PathfindingAlgorithm
     * @brief Pathfinding algorithms available
     */
    enum class PathfindingAlgorithm {
        AStar,                  ///< Standard A* algorithm
        ThetaStar,              ///< Theta* algorithm (any-angle pathfinding)
        LazyThetaStar,          ///< Lazy Theta* (optimized Theta*)
        JumpPointSearch,        ///< Jump Point Search (JPS)
        HierarchicalAStar,      ///< Hierarchical A* for multi-level navigation
        FlowField,              ///< Flow field pathfinding
        PotentialField,         ///< Potential field navigation
        Hybrid                 ///< Hybrid approach using multiple algorithms
    };

    /**
     * @enum PathStatus
     * @brief Status of pathfinding operations
     */
    enum class PathStatus {
        Pending,                ///< Pathfinding is in progress
        Success,                ///< Path found successfully
        Partial,                ///< Partial path found
        Failed,                 ///< Pathfinding failed
        Timeout,                ///< Pathfinding timed out
        Cancelled              ///< Pathfinding was cancelled
    };

    /**
     * @enum PathType
     * @brief Types of paths that can be found
     */
    enum class PathType {
        Ground,                 ///< Ground-based path
        Air,                    ///< Flying/air path
        Water,                  ///< Swimming/water path
        Climbing,               ///< Climbing path
        Burrowing,              ///< Underground/burrowing path
        Teleport,               ///< Teleportation path
        Custom                 ///< Custom path type
    };

    /**
     * @enum HeuristicType
     * @brief Heuristic functions for A* algorithm
     */
    enum class HeuristicType {
        Manhattan,              ///< Manhattan distance
        Euclidean,              ///< Euclidean distance
        Chebyshev,              ///< Chebyshev distance
        Octile,                 ///< Octile distance (8-directional)
        Diagonal,               ///< Diagonal distance
        Custom                 ///< Custom heuristic function
    };

    /**
     * @struct PathNode
     * @brief Node in the pathfinding graph
     */
    struct PathNode {
        glm::vec3 position;                     ///< World position
        float gCost;                            ///< Cost from start to this node
        float hCost;                            ///< Heuristic cost to goal
        float fCost;                            ///< Total cost (g + h)
        PathNode* parent;                       ///< Parent node in path
        int x, y, z;                            ///< Grid coordinates
        bool isWalkable;                        ///< Node is walkable
        bool isVisited;                         ///< Node has been visited
        bool isOpen;                            ///< Node is in open set
        bool isClosed;                          ///< Node is in closed set
        uint32_t nodeId;                        ///< Unique node ID
        float movementCost;                     ///< Cost to move through this node
        int heightLevel;                        ///< Height level for multi-level navigation
    };

    /**
     * @struct PathRequest
     * @brief Pathfinding request data
     */
    struct PathRequest {
        uint32_t requestId;                     ///< Unique request ID
        glm::vec3 startPosition;                ///< Starting position
        glm::vec3 goalPosition;                 ///< Goal position
        Mob* requester;                         ///< Mob requesting the path
        PathType pathType;                      ///< Type of path requested
        PathfindingAlgorithm algorithm;         ///< Algorithm to use
        HeuristicType heuristic;                ///< Heuristic function
        float maxPathLength;                    ///< Maximum path length
        float timeout;                          ///< Timeout in seconds
        bool allowPartialPaths;                 ///< Allow partial paths
        bool useDynamicObstacles;               ///< Consider dynamic obstacles
        bool useHeightLevels;                   ///< Use multi-level navigation
        float agentRadius;                      ///< Agent collision radius
        float agentHeight;                      ///< Agent height
        int priority;                           ///< Request priority
        std::function<void(const PathRequest&, const PathResult&)> callback; ///< Completion callback
    };

    /**
     * @struct PathResult
     * @brief Result of a pathfinding operation
     */
    struct PathResult {
        uint32_t requestId;                     ///< Original request ID
        PathStatus status;                      ///< Pathfinding status
        std::vector<glm::vec3> path;            ///< Found path (empty if failed)
        std::vector<PathNode> pathNodes;        ///< Detailed path nodes
        float pathLength;                       ///< Total path length
        double executionTime;                   ///< Time taken to find path
        uint32_t nodesExplored;                 ///< Number of nodes explored
        uint32_t nodesInPath;                   ///< Number of nodes in final path
        std::string failureReason;              ///< Reason for failure (if any)
        bool isPartial;                         ///< Path is partial
        float partialProgress;                  ///< Progress towards goal (0.0 - 1.0)
    };

    /**
     * @struct PathfindingConfig
     * @brief Configuration for pathfinding system
     */
    struct PathfindingConfig {
        // Algorithm settings
        PathfindingAlgorithm defaultAlgorithm;  ///< Default pathfinding algorithm
        HeuristicType defaultHeuristic;         ///< Default heuristic function
        bool enableMultiThreading;              ///< Enable multithreaded pathfinding
        int maxConcurrentRequests;              ///< Maximum concurrent requests

        // Performance settings
        float updateInterval;                   ///< Update interval (seconds)
        float defaultTimeout;                   ///< Default timeout (seconds)
        int maxPathLength;                      ///< Maximum path length (nodes)
        int maxNodesExplored;                   ///< Maximum nodes to explore
        bool enablePathCaching;                 ///< Enable path caching
        int cacheSize;                          ///< Path cache size

        // Grid settings
        float nodeSize;                         ///< Size of each pathfinding node
        float heightStep;                       ///< Height difference for level changes
        int maxHeightLevels;                    ///< Maximum height levels
        bool enableDynamicGridUpdates;          ///< Update grid when world changes

        // Agent settings
        float defaultAgentRadius;               ///< Default agent collision radius
        float defaultAgentHeight;               ///< Default agent height
        bool enableAgentCollision;              ///< Enable agent-to-agent collision

        // Optimization settings
        bool enableJumpLinks;                   ///< Enable jump point optimization
        bool enableHierarchicalSearch;          ///< Enable hierarchical search
        bool enableFlowFieldOptimization;       ///< Enable flow field optimization
        float simplificationThreshold;          ///< Path simplification threshold

        // Debug settings
        bool enableDebugDrawing;                ///< Enable pathfinding debug drawing
        bool showOpenNodes;                     ///< Show open nodes
        bool showClosedNodes;                   ///< Show closed nodes
        bool showPath;                          ///< Show final path
        bool showGrid;                          ///< Show navigation grid
    };

    /**
     * @struct PathfindingMetrics
     * @brief Performance metrics for pathfinding system
     */
    struct PathfindingMetrics {
        // Performance metrics
        uint64_t requestsProcessed;             ///< Total requests processed
        uint64_t successfulPaths;               ///< Successful pathfindings
        uint64_t failedPaths;                   ///< Failed pathfindings
        uint64_t partialPaths;                  ///< Partial paths found
        uint64_t timedOutRequests;              ///< Timed out requests

        // Timing metrics
        double totalProcessingTime;             ///< Total processing time (ms)
        double averageProcessingTime;           ///< Average processing time (ms)
        double maxProcessingTime;               ///< Maximum processing time (ms)
        double minProcessingTime;               ///< Minimum processing time (ms)

        // Node metrics
        uint64_t totalNodesExplored;            ///< Total nodes explored
        uint64_t averageNodesPerPath;           ///< Average nodes per successful path
        uint64_t maxNodesExplored;              ///< Maximum nodes explored for one path
        uint32_t gridSize;                      ///< Current navigation grid size

        // Cache metrics
        uint64_t cacheHits;                     ///< Path cache hits
        uint64_t cacheMisses;                   ///< Path cache misses
        float cacheHitRate;                     ///< Cache hit rate (0.0 - 1.0)
        uint32_t cachedPaths;                   ///< Number of cached paths

        // Algorithm metrics
        std::unordered_map<PathfindingAlgorithm, uint64_t> algorithmUsage; ///< Algorithm usage counts
        std::unordered_map<PathfindingAlgorithm, double> algorithmTimes;   ///< Algorithm execution times

        // Memory metrics
        size_t memoryUsage;                     ///< Current memory usage
        size_t peakMemoryUsage;                 ///< Peak memory usage
        uint32_t allocations;                   ///< Memory allocations
        uint32_t deallocations;                 ///< Memory deallocations
    };

    /**
     * @struct NavigationGrid
     * @brief Navigation grid for pathfinding
     */
    struct NavigationGrid {
        std::vector<std::vector<std::vector<PathNode>>> nodes; ///< 3D grid of path nodes
        glm::vec3 gridOrigin;                     ///< Grid origin position
        glm::vec3 gridSize;                       ///< Grid dimensions
        float nodeSize;                           ///< Size of each node
        bool isValid;                             ///< Grid is valid
        double lastUpdateTime;                    ///< Last grid update time
        uint32_t version;                         ///< Grid version for cache invalidation
    };

    /**
     * @struct PathCacheEntry
     * @brief Cached pathfinding result
     */
    struct PathCacheEntry {
        glm::vec3 startPosition;                ///< Start position
        glm::vec3 goalPosition;                 ///< Goal position
        PathType pathType;                      ///< Path type
        std::vector<glm::vec3> path;            ///< Cached path
        double cacheTime;                       ///< Time when cached
        float expirationTime;                   ///< Cache expiration time
        uint32_t usageCount;                    ///< Number of times used
        bool isValid;                           ///< Cache entry is valid
    };

    /**
     * @class PathfindingSystem
     * @brief Advanced A* pathfinding system for voxel-based worlds
     *
     * The PathfindingSystem provides high-performance pathfinding capabilities
     * optimized for voxel-based environments. It supports multiple algorithms,
     * multi-level navigation, dynamic obstacle avoidance, and performance
     * optimizations for large-scale worlds.
     *
     * Key Features:
     * - Multiple pathfinding algorithms (A*, Theta*, JPS, Flow Field)
     * - Multi-level navigation for complex terrain
     * - Dynamic obstacle avoidance
     * - Path caching for performance
     * - Multi-threaded pathfinding
     * - Hierarchical navigation for large worlds
     * - Flow field optimization for crowd movement
     *
     * The system is designed to handle hundreds of concurrent pathfinding
     * requests efficiently while maintaining real-time performance.
     */
    class PathfindingSystem {
    public:
        /**
         * @brief Constructor
         * @param config Pathfinding system configuration
         */
        explicit PathfindingSystem(const PathfindingConfig& config);

        /**
         * @brief Destructor
         */
        ~PathfindingSystem();

        /**
         * @brief Deleted copy constructor
         */
        PathfindingSystem(const PathfindingSystem&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        PathfindingSystem& operator=(const PathfindingSystem&) = delete;

        // System lifecycle

        /**
         * @brief Initialize pathfinding system
         * @param world Game world
         * @return true if successful, false otherwise
         */
        bool Initialize(World* world);

        /**
         * @brief Shutdown pathfinding system
         */
        void Shutdown();

        /**
         * @brief Update pathfinding system
         * @param deltaTime Time elapsed since last update
         */
        void Update(double deltaTime);

        // Pathfinding requests

        /**
         * @brief Request pathfinding
         * @param request Pathfinding request
         * @return Request ID or 0 if failed
         */
        uint32_t RequestPath(const PathRequest& request);

        /**
         * @brief Cancel pathfinding request
         * @param requestId Request ID to cancel
         * @return true if cancelled, false otherwise
         */
        bool CancelRequest(uint32_t requestId);

        /**
         * @brief Get pathfinding result
         * @param requestId Request ID
         * @return Path result or empty optional if not ready
         */
        std::optional<PathResult> GetResult(uint32_t requestId);

        /**
         * @brief Check if request is complete
         * @param requestId Request ID
         * @return true if complete, false otherwise
         */
        bool IsRequestComplete(uint32_t requestId) const;

        /**
         * @brief Get pending request count
         * @return Number of pending requests
         */
        size_t GetPendingRequestCount() const;

        // Synchronous pathfinding (blocking)

        /**
         * @brief Find path synchronously
         * @param start Start position
         * @param goal Goal position
         * @param pathType Type of path
         * @param agentRadius Agent collision radius
         * @param timeout Timeout in seconds
         * @return Path result
         */
        PathResult FindPath(const glm::vec3& start, const glm::vec3& goal,
                           PathType pathType = PathType::Ground,
                           float agentRadius = 0.5f, float timeout = 5.0f);

        // Configuration

        /**
         * @brief Get pathfinding configuration
         * @return Current configuration
         */
        const PathfindingConfig& GetConfig() const { return m_config; }

        /**
         * @brief Set pathfinding configuration
         * @param config New configuration
         */
        void SetConfig(const PathfindingConfig& config);

        /**
         * @brief Get default algorithm
         * @return Default algorithm
         */
        PathfindingAlgorithm GetDefaultAlgorithm() const { return m_config.defaultAlgorithm; }

        /**
         * @brief Set default algorithm
         * @param algorithm New default algorithm
         */
        void SetDefaultAlgorithm(PathfindingAlgorithm algorithm) { m_config.defaultAlgorithm = algorithm; }

        // Navigation grid

        /**
         * @brief Get navigation grid
         * @return Navigation grid
         */
        const NavigationGrid& GetNavigationGrid() const { return m_navigationGrid; }

        /**
         * @brief Rebuild navigation grid
         * @param force Force rebuild even if not needed
         * @return true if rebuilt, false otherwise
         */
        bool RebuildNavigationGrid(bool force = false);

        /**
         * @brief Update navigation grid region
         * @param minPos Minimum position
         * @param maxPos Maximum position
         * @return true if updated, false otherwise
         */
        bool UpdateNavigationGridRegion(const glm::vec3& minPos, const glm::vec3& maxPos);

        /**
         * @brief Check if position is walkable
         * @param position World position
         * @param agentRadius Agent collision radius
         * @return true if walkable, false otherwise
         */
        bool IsPositionWalkable(const glm::vec3& position, float agentRadius = 0.5f) const;

        /**
         * @brief Get nearest walkable position
         * @param position Target position
         * @param agentRadius Agent collision radius
         * @param searchRadius Search radius
         * @return Nearest walkable position or original position
         */
        glm::vec3 GetNearestWalkablePosition(const glm::vec3& position, float agentRadius = 0.5f, float searchRadius = 10.0f) const;

        // Path utilities

        /**
         * @brief Simplify path by removing unnecessary waypoints
         * @param path Original path
         * @param tolerance Simplification tolerance
         * @return Simplified path
         */
        std::vector<glm::vec3> SimplifyPath(const std::vector<glm::vec3>& path, float tolerance = 0.1f) const;

        /**
         * @brief Smooth path by adding curves
         * @param path Original path
         * @param smoothness Smoothing factor (0.0 - 1.0)
         * @return Smoothed path
         */
        std::vector<glm::vec3> SmoothPath(const std::vector<glm::vec3>& path, float smoothness = 0.5f) const;

        /**
         * @brief Calculate path length
         * @param path Path to calculate
         * @return Total path length
         */
        float CalculatePathLength(const std::vector<glm::vec3>& path) const;

        /**
         * @brief Validate path against current world state
         * @param path Path to validate
         * @param agentRadius Agent collision radius
         * @return true if valid, false otherwise
         */
        bool ValidatePath(const std::vector<glm::vec3>& path, float agentRadius = 0.5f) const;

        // Metrics and monitoring

        /**
         * @brief Get pathfinding metrics
         * @return Performance metrics
         */
        const PathfindingMetrics& GetMetrics() const { return m_metrics; }

        /**
         * @brief Reset metrics
         */
        void ResetMetrics();

        /**
         * @brief Get performance report
         * @return Detailed performance report
         */
        std::string GetPerformanceReport() const;

        // Cache management

        /**
         * @brief Clear path cache
         */
        void ClearPathCache();

        /**
         * @brief Get cache size
         * @return Number of cached paths
         */
        size_t GetCacheSize() const;

        /**
         * @brief Get cache hit rate
         * @return Cache hit rate (0.0 - 1.0)
         */
        float GetCacheHitRate() const;

        // Debug and visualization

        /**
         * @brief Enable debug drawing
         * @param enabled Enable state
         * @return Previous state
         */
        bool SetDebugDrawingEnabled(bool enabled);

        /**
         * @brief Check if debug drawing is enabled
         * @return true if enabled, false otherwise
         */
        bool IsDebugDrawingEnabled() const { return m_config.enableDebugDrawing; }

        /**
         * @brief Get debug render data
         * @return Debug render data
         */
        const std::vector<DebugRenderData>& GetDebugRenderData() const;

        /**
         * @brief Clear debug render data
         */
        void ClearDebugRenderData();

        // Utility functions

        /**
         * @brief Get world the pathfinding system is in
         * @return World instance
         */
        World* GetWorld() const { return m_world; }

        /**
         * @brief Set world for pathfinding system
         * @param world New world
         */
        void SetWorld(World* world);

        /**
         * @brief Validate pathfinding system
         * @return true if valid, false otherwise
         */
        bool Validate() const;

        /**
         * @brief Get system status report
         * @return Detailed status report
         */
        std::string GetStatusReport() const;

        /**
         * @brief Optimize pathfinding performance
         * @return Optimization results
         */
        std::unordered_map<std::string, size_t> OptimizeSystem();

    private:
        /**
         * @brief Initialize navigation grid
         * @return true if successful, false otherwise
         */
        bool InitializeNavigationGrid();

        /**
         * @brief Initialize worker threads
         * @return true if successful, false otherwise
         */
        bool InitializeWorkerThreads();

        /**
         * @brief Worker thread function
         * @param threadId Thread ID
         */
        void WorkerThreadFunction(int threadId);

        /**
         * @brief Process pathfinding requests
         */
        void ProcessRequests();

        /**
         * @brief Execute pathfinding using A* algorithm
         * @param request Pathfinding request
         * @return Path result
         */
        PathResult ExecuteAStar(const PathRequest& request);

        /**
         * @brief Execute pathfinding using Theta* algorithm
         * @param request Pathfinding request
         * @return Path result
         */
        PathResult ExecuteThetaStar(const PathRequest& request);

        /**
         * @brief Execute pathfinding using Jump Point Search
         * @param request Pathfinding request
         * @return Path result
         */
        PathResult ExecuteJumpPointSearch(const PathRequest& request);

        /**
         * @brief Execute pathfinding using flow field
         * @param request Pathfinding request
         * @return Path result
         */
        PathResult ExecuteFlowField(const PathRequest& request);

        /**
         * @brief Calculate heuristic cost
         * @param from Starting position
         * @param to Goal position
         * @param heuristic Heuristic type
         * @return Heuristic cost
         */
        float CalculateHeuristic(const glm::vec3& from, const glm::vec3& to, HeuristicType heuristic) const;

        /**
         * @brief Get neighbors of a path node
         * @param node Current node
         * @param algorithm Pathfinding algorithm
         * @return Vector of neighbor nodes
         */
        std::vector<PathNode*> GetNeighbors(PathNode* node, PathfindingAlgorithm algorithm) const;

        /**
         * @brief Check if path between nodes is valid
         * @param from Starting node
         * @param to Ending node
         * @param agentRadius Agent collision radius
         * @return true if path is valid, false otherwise
         */
        bool IsPathValid(const PathNode* from, const PathNode* to, float agentRadius) const;

        /**
         * @brief Reconstruct path from goal to start
         * @param goalNode Goal node
         * @return Reconstructed path
         */
        std::vector<glm::vec3> ReconstructPath(const PathNode* goalNode) const;

        /**
         * @brief Check path cache for existing result
         * @param request Pathfinding request
         * @return Cached result or empty optional
         */
        std::optional<PathResult> CheckPathCache(const PathRequest& request) const;

        /**
         * @brief Add result to path cache
         * @param request Original request
         * @param result Path result
         */
        void AddToPathCache(const PathRequest& request, const PathResult& result);

        /**
         * @brief Update navigation grid for dynamic obstacles
         */
        void UpdateDynamicObstacles();

        /**
         * @brief Update pathfinding metrics
         * @param result Pathfinding result
         * @param executionTime Execution time
         */
        void UpdateMetrics(const PathResult& result, double executionTime);

        /**
         * @brief Collect debug render data
         */
        void CollectDebugRenderData();

        /**
         * @brief Handle pathfinding errors
         * @param error Error message
         */
        void HandlePathfindingError(const std::string& error);

        // Pathfinding data
        PathfindingConfig m_config;                      ///< System configuration
        PathfindingMetrics m_metrics;                    ///< Performance metrics

        // Navigation data
        NavigationGrid m_navigationGrid;                 ///< Navigation grid
        World* m_world;                                  ///< Game world

        // Request management
        std::priority_queue<PathRequest, std::vector<PathRequest>, std::function<bool(const PathRequest&, const PathRequest&)>> m_requestQueue;
        std::unordered_map<uint32_t, PathResult> m_completedResults;
        mutable std::shared_mutex m_requestMutex;        ///< Request synchronization

        // Cache system
        std::unordered_map<uint64_t, PathCacheEntry> m_pathCache;
        mutable std::shared_mutex m_cacheMutex;          ///< Cache synchronization

        // Threading
        std::vector<std::thread> m_workerThreads;       ///< Worker threads
        std::atomic<bool> m_workersRunning;             ///< Workers running flag
        std::atomic<int> m_activeThreads;              ///< Active thread count

        // Debug data
        std::vector<DebugRenderData> m_debugRenderData;
        mutable std::shared_mutex m_debugMutex;         ///< Debug synchronization

        // State flags
        bool m_isInitialized;                           ///< System is initialized
        double m_lastUpdateTime;                        ///< Last update timestamp
        double m_lastGridUpdateTime;                    ///< Last grid update time

        static std::atomic<uint32_t> s_nextRequestId;  ///< Next request ID counter
        static std::atomic<uint32_t> s_nextNodeId;     ///< Next node ID counter
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_AI_PATHFINDING_SYSTEM_HPP
