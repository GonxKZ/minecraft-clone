/**
 * @file Pathfinding.cpp
 * @brief VoxelCraft AI Pathfinding Implementation with A* Algorithm
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#include "Pathfinding.hpp"
#include "../world/World.hpp"
#include "../entities/Entity.hpp"
#include <algorithm>
#include <cmath>
#include <queue>
#include <unordered_set>

namespace VoxelCraft {

    // PathfindingGrid implementation
    PathfindingGrid::PathfindingGrid(World* world, int chunkRadius)
        : m_world(world), m_chunkRadius(chunkRadius) {
    }

    bool PathfindingGrid::IsWalkable(const glm::ivec3& position) const {
        if (!m_world) return false;

        // Check if the block at position is air or walkable
        int blockID = m_world->GetBlock(position.x, position.y, position.z);
        bool isAirOrWalkable = (blockID == 0); // Air block

        // Check if there's solid ground below
        int blockBelow = m_world->GetBlock(position.x, position.y - 1, position.z);
        bool hasGround = (blockBelow != 0); // Not air

        // Check if there's enough space above (head room)
        int blockAbove = m_world->GetBlock(position.x, position.y + 1, position.z);
        bool hasHeadRoom = (blockAbove == 0); // Air above

        return isAirOrWalkable && hasGround && hasHeadRoom;
    }

    std::vector<glm::ivec3> PathfindingGrid::GetNeighbors(const glm::ivec3& position) const {
        std::vector<glm::ivec3> neighbors;

        // 6 main directions (up, down, north, south, east, west)
        const glm::ivec3 directions[] = {
            glm::ivec3(1, 0, 0),   // East
            glm::ivec3(-1, 0, 0),  // West
            glm::ivec3(0, 0, 1),   // North
            glm::ivec3(0, 0, -1),  // South
            glm::ivec3(0, 1, 0),   // Up
            glm::ivec3(0, -1, 0)   // Down
        };

        for (const auto& dir : directions) {
            glm::ivec3 neighbor = position + dir;
            if (IsWalkable(neighbor)) {
                neighbors.push_back(neighbor);
            }
        }

        return neighbors;
    }

    float PathfindingGrid::GetMovementCost(const glm::ivec3& from, const glm::ivec3& to) const {
        glm::ivec3 diff = to - from;

        // Base cost
        float cost = 1.0f;

        // Diagonal movement costs more
        if (abs(diff.x) + abs(diff.z) > 1) {
            cost *= 1.414f; // sqrt(2)
        }

        // Going up costs more
        if (diff.y > 0) {
            cost *= 1.5f;
        }

        // Going down costs less
        if (diff.y < 0) {
            cost *= 0.8f;
        }

        return cost;
    }

    bool PathfindingGrid::CanJump(const glm::ivec3& from, const glm::ivec3& to) const {
        glm::ivec3 diff = to - from;

        // Check if it's a jump (going up)
        if (diff.y <= 0) return true;

        // Check if we can jump from current position
        int jumpHeight = GetJumpHeight(from);
        return diff.y <= jumpHeight;
    }

    int PathfindingGrid::GetJumpHeight(const glm::ivec3& position) const {
        // Simple jump height calculation - could be entity-specific
        return 2; // 2 blocks high jump
    }

    int PathfindingGrid::FindGroundLevel(const glm::ivec3& position) const {
        glm::ivec3 checkPos = position;

        // Search down for ground
        for (int y = position.y; y > 0; --y) {
            checkPos.y = y;
            if (IsSolidBlock(checkPos)) {
                return y + 1; // Return position above ground
            }
        }

        return position.y; // No ground found, return original
    }

    bool PathfindingGrid::IsInWater(const glm::ivec3& position) const {
        int blockID = m_world->GetBlock(position.x, position.y, position.z);
        return blockID == 9 || blockID == 8; // Water blocks
    }

    bool PathfindingGrid::IsInLava(const glm::ivec3& position) const {
        int blockID = m_world->GetBlock(position.x, position.y, position.z);
        return blockID == 11 || blockID == 10; // Lava blocks
    }

    bool PathfindingGrid::IsSolidBlock(const glm::ivec3& position) const {
        int blockID = m_world->GetBlock(position.x, position.y, position.z);
        return blockID != 0 && blockID != 8 && blockID != 9 && blockID != 10 && blockID != 11;
    }

    bool PathfindingGrid::IsLiquidBlock(const glm::ivec3& position) const {
        int blockID = m_world->GetBlock(position.x, position.y, position.z);
        return blockID == 8 || blockID == 9 || blockID == 10 || blockID == 11;
    }

    // Pathfinding implementation
    Pathfinding::Pathfinding(World* world)
        : m_world(world), m_grid(std::make_unique<PathfindingGrid>(world)) {
    }

    std::vector<glm::ivec3> Pathfinding::FindPath(const glm::vec3& start,
                                                 const glm::vec3& goal,
                                                 Entity* entity) {
        auto startTime = std::chrono::high_resolution_clock::now();

        // Convert to integer coordinates
        glm::ivec3 startPos = glm::round(start);
        glm::ivec3 goalPos = glm::round(goal);

        // Adjust to ground level
        startPos.y = m_grid->FindGroundLevel(startPos);
        goalPos.y = m_grid->FindGroundLevel(goalPos);

        // Early exit if start and goal are the same
        if (startPos == goalPos) {
            return {startPos};
        }

        // Initialize A* algorithm
        ClearData();
        m_cancelled = false;

        // Create start and goal nodes
        PathNode startNode(startPos);
        startNode.gCost = 0.0f;
        startNode.hCost = CalculateHeuristic(startPos, goalPos);
        startNode.fCost = startNode.GetFCost();

        PathNode goalNode(goalPos);

        // Add start node to open set
        AddToOpenSet(startNode);

        int nodesExplored = 0;

        while (!m_openSet.empty() && !m_cancelled) {
            // Get node with lowest F cost
            PathNode current = GetLowestCostNode();

            // Check if we've reached the goal
            if (current.position == goalPos) {
                auto endTime = std::chrono::high_resolution_clock::now();
                float searchTime = std::chrono::duration<float>(endTime - startTime).count();

                UpdateStats(true, searchTime, nodesExplored, 0);
                return ReconstructPath(current);
            }

            // Move current node to closed set
            RemoveFromOpenSet(current);
            AddToClosedSet(current);

            // Explore neighbors
            auto neighbors = m_grid->GetNeighbors(current.position);

            for (const auto& neighborPos : neighbors) {
                // Skip if in closed set
                PathNode neighborNode(neighborPos);
                if (IsInClosedSet(neighborNode)) {
                    continue;
                }

                // Calculate costs
                float movementCost = m_grid->GetMovementCost(current.position, neighborPos);
                float tentativeGCost = current.gCost + movementCost;

                // Check if this path is better than any previous one
                bool inOpenSet = IsInOpenSet(neighborNode);

                if (!inOpenSet || tentativeGCost < neighborNode.gCost) {
                    neighborNode.parent = new PathNode(current);
                    neighborNode.gCost = tentativeGCost;
                    neighborNode.hCost = CalculateHeuristic(neighborPos, goalPos);
                    neighborNode.fCost = neighborNode.GetFCost();

                    if (!inOpenSet) {
                        AddToOpenSet(neighborNode);
                    }
                }
            }

            nodesExplored++;

            // Check limits
            if (nodesExplored > m_maxSearchDistance * m_maxSearchDistance) {
                break;
            }
        }

        // No path found
        auto endTime = std::chrono::high_resolution_clock::now();
        float searchTime = std::chrono::duration<float>(endTime - startTime).count();

        UpdateStats(false, searchTime, nodesExplored, 0);
        return {};
    }

    std::vector<glm::ivec3> Pathfinding::FindPathAsync(const glm::vec3& start,
                                                      const glm::vec3& goal,
                                                      Entity* entity,
                                                      std::function<void(float)> progressCallback) {
        // For now, just call synchronous version
        // In a real implementation, this would run on a separate thread
        return FindPath(start, goal, entity);
    }

    void Pathfinding::CancelPathfinding() {
        m_cancelled = true;
    }

    std::vector<glm::ivec3> Pathfinding::SimplifyPath(const std::vector<glm::ivec3>& path) const {
        if (path.size() <= 2) {
            return path;
        }

        std::vector<glm::ivec3> simplifiedPath;
        simplifiedPath.push_back(path[0]);

        for (size_t i = 1; i < path.size() - 1; ++i) {
            const auto& prev = simplifiedPath.back();
            const auto& current = path[i];
            const auto& next = path[i + 1];

            // Check if we can skip this waypoint
            glm::vec3 dir1 = glm::normalize(glm::vec3(current) - glm::vec3(prev));
            glm::vec3 dir2 = glm::normalize(glm::vec3(next) - glm::vec3(current));

            if (glm::dot(dir1, dir2) < 0.99f) { // Angle threshold
                simplifiedPath.push_back(current);
            }
        }

        simplifiedPath.push_back(path.back());
        return simplifiedPath;
    }

    bool Pathfinding::IsPathValid(const std::vector<glm::ivec3>& path) const {
        for (const auto& pos : path) {
            if (!m_grid->IsWalkable(pos)) {
                return false;
            }
        }
        return true;
    }

    float Pathfinding::CalculateHeuristic(const glm::ivec3& from, const glm::ivec3& to) const {
        // Manhattan distance
        float dx = abs(from.x - to.x);
        float dy = abs(from.y - to.y);
        float dz = abs(from.z - to.z);

        if (m_allowDiagonal) {
            // Octile distance for diagonal movement
            float maxXZ = std::max(dx, dz);
            float minXZ = std::min(dx, dz);
            return (maxXZ * 1.0f + minXZ * 0.414f + dy * 1.0f) * m_heuristicWeight;
        } else {
            // Manhattan distance
            return (dx + dy + dz) * m_heuristicWeight;
        }
    }

    std::vector<glm::ivec3> Pathfinding::ReconstructPath(const PathNode& goalNode) const {
        std::vector<glm::ivec3> path;
        const PathNode* current = &goalNode;

        while (current != nullptr) {
            path.push_back(current->position);
            current = current->parent;
        }

        std::reverse(path.begin(), path.end());
        return path;
    }

    PathNode Pathfinding::GetLowestCostNode() {
        auto it = std::min_element(m_openSet.begin(), m_openSet.end(),
            [](const auto& a, const auto& b) {
                return a.second.GetFCost() < b.second.GetFCost();
            });

        PathNode node = it->second;
        m_openSet.erase(it);
        return node;
    }

    bool Pathfinding::IsInOpenSet(const PathNode& node) const {
        return m_openSet.find(node) != m_openSet.end();
    }

    bool Pathfinding::IsInClosedSet(const PathNode& node) const {
        return m_closedSet.find(node) != m_closedSet.end();
    }

    void Pathfinding::AddToOpenSet(const PathNode& node) {
        m_openSet[node] = node;
    }

    void Pathfinding::AddToClosedSet(const PathNode& node) {
        m_closedSet[node] = node;
    }

    void Pathfinding::RemoveFromOpenSet(const PathNode& node) {
        m_openSet.erase(node);
    }

    void Pathfinding::ClearData() {
        m_openSet.clear();
        m_closedSet.clear();
        m_cancelled = false;
    }

    void Pathfinding::UpdateStats(bool success, float searchTime, int nodesExplored, int pathLength) {
        m_stats.totalSearches++;
        if (success) {
            m_stats.successfulSearches++;
        } else {
            m_stats.failedSearches++;
        }

        m_stats.averageSearchTime = (m_stats.averageSearchTime * (m_stats.totalSearches - 1) + searchTime) / m_stats.totalSearches;
        m_stats.minSearchTime = std::min(m_stats.minSearchTime, searchTime);
        m_stats.maxSearchTime = std::max(m_stats.maxSearchTime, searchTime);
        m_stats.totalNodesExplored += nodesExplored;

        if (success && pathLength > 0) {
            m_stats.averagePathLength = (m_stats.averagePathLength * (m_stats.successfulSearches - 1) + pathLength) / m_stats.successfulSearches;
            m_stats.longestPathFound = std::max(m_stats.longestPathFound, pathLength);
            m_stats.shortestPathFound = std::min(m_stats.shortestPathFound, pathLength);
        }
    }

    // Path implementation
    Path::Path(const std::vector<glm::ivec3>& waypoints) {
        m_waypoints.reserve(waypoints.size());
        for (const auto& wp : waypoints) {
            m_waypoints.push_back(glm::vec3(wp));
        }
    }

    glm::vec3 Path::GetNextWaypoint() const {
        if (m_currentIndex < m_waypoints.size()) {
            return m_waypoints[m_currentIndex];
        }
        return m_waypoints.empty() ? glm::vec3(0.0f) : m_waypoints.back();
    }

    bool Path::MoveToNextWaypoint() {
        if (m_currentIndex + 1 < m_waypoints.size()) {
            m_currentIndex++;
            return true;
        }
        return false;
    }

    bool Path::IsFinished() const {
        return m_currentIndex >= m_waypoints.size();
    }

    float Path::GetPathLength() const {
        if (m_waypoints.size() < 2) {
            return 0.0f;
        }

        float length = 0.0f;
        for (size_t i = 1; i < m_waypoints.size(); ++i) {
            length += glm::distance(m_waypoints[i - 1], m_waypoints[i]);
        }
        return length;
    }

    void Path::Clear() {
        m_waypoints.clear();
        m_currentIndex = 0;
    }

} // namespace VoxelCraft
