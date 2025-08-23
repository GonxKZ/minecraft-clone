/**
 * @file Pathfinding.hpp
 * @brief VoxelCraft AI Pathfinding System with A* Algorithm
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_AI_PATHFINDING_HPP
#define VOXELCRAFT_AI_PATHFINDING_HPP

#include <vector>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <functional>
#include <chrono>
#include <glm/glm.hpp>

namespace VoxelCraft {

    class World;
    class Entity;

    /**
     * @struct PathNode
     * @brief Node in the pathfinding graph
     */
    struct PathNode {
        glm::ivec3 position;
        float gCost = 0.0f;  // Cost from start to this node
        float hCost = 0.0f;  // Heuristic cost to goal
        float fCost = 0.0f;  // Total cost (g + h)
        PathNode* parent = nullptr;
        bool walkable = true;
        int jumpHeight = 0;  // How high we can jump from this node

        PathNode(const glm::ivec3& pos = glm::ivec3(0))
            : position(pos) {}

        float GetFCost() const { return gCost + hCost; }

        bool operator==(const PathNode& other) const {
            return position == other.position;
        }

        bool operator!=(const PathNode& other) const {
            return position != other.position;
        }
    };

    /**
     * @struct PathNodeHash
     * @brief Hash function for PathNode
     */
    struct PathNodeHash {
        std::size_t operator()(const PathNode& node) const {
            return std::hash<int>()(node.position.x) ^
                   std::hash<int>()(node.position.y) ^
                   std::hash<int>()(node.position.z);
        }
    };

    /**
     * @class PathfindingGrid
     * @brief Grid representation for pathfinding
     */
    class PathfindingGrid {
    public:
        /**
         * @brief Constructor
         * @param world World reference
         * @param chunkRadius Radius in chunks to search
         */
        PathfindingGrid(World* world, int chunkRadius = 3);

        /**
         * @brief Check if a position is walkable
         * @param position Position to check
         * @return true if walkable
         */
        bool IsWalkable(const glm::ivec3& position) const;

        /**
         * @brief Get neighbors of a position
         * @param position Current position
         * @return Vector of neighboring positions
         */
        std::vector<glm::ivec3> GetNeighbors(const glm::ivec3& position) const;

        /**
         * @brief Get movement cost between two positions
         * @param from From position
         * @param to To position
         * @return Movement cost
         */
        float GetMovementCost(const glm::ivec3& from, const glm::ivec3& to) const;

        /**
         * @brief Check if we can jump from one position to another
         * @param from From position
         * @param to To position
         * @return true if can jump
         */
        bool CanJump(const glm::ivec3& from, const glm::ivec3& to) const;

        /**
         * @brief Get jump height at position
         * @param position Position to check
         * @return Jump height
         */
        int GetJumpHeight(const glm::ivec3& position) const;

        /**
         * @brief Find ground level at position
         * @param position Position to check
         * @return Ground level Y coordinate
         */
        int FindGroundLevel(const glm::ivec3& position) const;

        /**
         * @brief Check if position is in water
         * @param position Position to check
         * @return true if in water
         */
        bool IsInWater(const glm::ivec3& position) const;

        /**
         * @brief Check if position is in lava
         * @param position Position to check
         * @return true if in lava
         */
        bool IsInLava(const glm::ivec3& position) const;

    private:
        World* m_world;
        int m_chunkRadius;

        /**
         * @brief Check if block at position is solid
         * @param position Position to check
         * @return true if solid
         */
        bool IsSolidBlock(const glm::ivec3& position) const;

        /**
         * @brief Check if block at position is liquid
         * @param position Position to check
         * @return true if liquid
         */
        bool IsLiquidBlock(const glm::ivec3& position) const;
    };

    /**
     * @class Pathfinding
     * @brief A* pathfinding implementation
     */
    class Pathfinding {
    public:
        /**
         * @brief Constructor
         * @param world World reference
         */
        Pathfinding(World* world);

        /**
         * @brief Find path between two points
         * @param start Start position
         * @param goal Goal position
         * @param entity Entity doing the pathfinding (for size considerations)
         * @return Vector of positions representing the path
         */
        std::vector<glm::ivec3> FindPath(const glm::vec3& start,
                                        const glm::vec3& goal,
                                        Entity* entity = nullptr);

        /**
         * @brief Find path with callback for progress updates
         * @param start Start position
         * @param goal Goal position
         * @param entity Entity doing the pathfinding
         * @param progressCallback Callback for progress updates
         * @return Vector of positions representing the path
         */
        std::vector<glm::ivec3> FindPathAsync(const glm::vec3& start,
                                             const glm::vec3& goal,
                                             Entity* entity,
                                             std::function<void(float)> progressCallback = nullptr);

        /**
         * @brief Cancel current pathfinding operation
         */
        void CancelPathfinding();

        /**
         * @brief Simplify path by removing unnecessary waypoints
         * @param path Original path
         * @return Simplified path
         */
        std::vector<glm::ivec3> SimplifyPath(const std::vector<glm::ivec3>& path) const;

        /**
         * @brief Check if path is still valid
         * @param path Path to check
         * @return true if path is still valid
         */
        bool IsPathValid(const std::vector<glm::ivec3>& path) const;

        /**
         * @brief Get pathfinding statistics
         * @return Pathfinding stats
         */
        const PathfindingStats& GetStats() const { return m_stats; }

        /**
         * @brief Set maximum search distance
         * @param distance Maximum distance
         */
        void SetMaxSearchDistance(int distance) { m_maxSearchDistance = distance; }

        /**
         * @brief Set maximum path length
         * @param length Maximum length
         */
        void SetMaxPathLength(int length) { m_maxPathLength = length; }

        /**
         * @brief Enable diagonal movement
         * @param enable Whether to enable diagonal movement
         */
        void EnableDiagonalMovement(bool enable) { m_allowDiagonal = enable; }

        /**
         * @brief Enable jumping
         * @param enable Whether to enable jumping
         */
        void EnableJumping(bool enable) { m_allowJumping = enable; }

        /**
         * @brief Set heuristic weight
         * @param weight Heuristic weight
         */
        void SetHeuristicWeight(float weight) { m_heuristicWeight = weight; }

    private:
        World* m_world;
        std::unique_ptr<PathfindingGrid> m_grid;
        PathfindingStats m_stats;

        // Configuration
        int m_maxSearchDistance = 64;
        int m_maxPathLength = 256;
        bool m_allowDiagonal = true;
        bool m_allowJumping = true;
        float m_heuristicWeight = 1.0f;
        bool m_cancelled = false;

        // A* data structures
        std::unordered_map<PathNode, PathNode, PathNodeHash> m_openSet;
        std::unordered_map<PathNode, PathNode, PathNodeHash> m_closedSet;

        struct NodeCompare {
            bool operator()(const PathNode& a, const PathNode& b) const {
                return a.GetFCost() > b.GetFCost();
            }
        };

        /**
         * @brief Calculate heuristic cost (Manhattan distance)
         * @param from From position
         * @param to To position
         * @return Heuristic cost
         */
        float CalculateHeuristic(const glm::ivec3& from, const glm::ivec3& to) const;

        /**
         * @brief Reconstruct path from goal to start
         * @param goalNode Goal node
         * @return Vector of positions
         */
        std::vector<glm::ivec3> ReconstructPath(const PathNode& goalNode) const;

        /**
         * @brief Get node with lowest F cost from open set
         * @return Lowest cost node
         */
        PathNode GetLowestCostNode();

        /**
         * @brief Check if node is in open set
         * @param node Node to check
         * @return true if in open set
         */
        bool IsInOpenSet(const PathNode& node) const;

        /**
         * @brief Check if node is in closed set
         * @param node Node to check
         * @return true if in closed set
         */
        bool IsInClosedSet(const PathNode& node) const;

        /**
         * @brief Add node to open set
         * @param node Node to add
         */
        void AddToOpenSet(const PathNode& node);

        /**
         * @brief Add node to closed set
         * @param node Node to add
         */
        void AddToClosedSet(const PathNode& node);

        /**
         * @brief Remove node from open set
         * @param node Node to remove
         */
        void RemoveFromOpenSet(const PathNode& node);

        /**
         * @brief Clear pathfinding data
         */
        void ClearData();
    };

    /**
     * @struct PathfindingStats
     * @brief Statistics for pathfinding operations
     */
    struct PathfindingStats {
        int totalSearches = 0;
        int successfulSearches = 0;
        int failedSearches = 0;
        float averageSearchTime = 0.0f;
        float minSearchTime = 999999.0f;
        float maxSearchTime = 0.0f;
        int totalNodesExplored = 0;
        int averagePathLength = 0;
        int longestPathFound = 0;
        int shortestPathFound = 999999;
    };

    /**
     * @class Path
     * @brief Represents a calculated path
     */
    class Path {
    public:
        /**
         * @brief Constructor
         * @param waypoints Vector of waypoints
         */
        Path(const std::vector<glm::ivec3>& waypoints = {});

        /**
         * @brief Get next waypoint
         * @return Next waypoint or current position if no more waypoints
         */
        glm::vec3 GetNextWaypoint() const;

        /**
         * @brief Move to next waypoint
         * @return true if moved to next waypoint
         */
        bool MoveToNextWaypoint();

        /**
         * @brief Check if path is finished
         * @return true if no more waypoints
         */
        bool IsFinished() const;

        /**
         * @brief Get current waypoint index
         * @return Current index
         */
        size_t GetCurrentIndex() const { return m_currentIndex; }

        /**
         * @brief Get total number of waypoints
         * @return Total waypoints
         */
        size_t GetWaypointCount() const { return m_waypoints.size(); }

        /**
         * @brief Get all waypoints
         * @return Vector of waypoints
         */
        const std::vector<glm::vec3>& GetWaypoints() const { return m_waypoints; }

        /**
         * @brief Get path length
         * @return Path length in blocks
         */
        float GetPathLength() const;

        /**
         * @brief Clear path
         */
        void Clear();

        /**
         * @brief Check if path is valid
         * @return true if path has waypoints
         */
        bool IsValid() const { return !m_waypoints.empty(); }

    private:
        std::vector<glm::vec3> m_waypoints;
        size_t m_currentIndex = 0;
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_AI_PATHFINDING_HPP
