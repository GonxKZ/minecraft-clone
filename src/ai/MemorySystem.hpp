/**
 * @file MemorySystem.hpp
 * @brief VoxelCraft AI Memory System for Entity Memory and Learning
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_AI_MEMORY_SYSTEM_HPP
#define VOXELCRAFT_AI_MEMORY_SYSTEM_HPP

#include <vector>
#include <memory>
#include <unordered_map>
#include <queue>
#include <functional>
#include <chrono>
#include <glm/glm.hpp>

namespace VoxelCraft {

    class Entity;
    class Player;
    class World;

    /**
     * @enum MemoryType
     * @brief Types of memories that can be stored
     */
    enum class MemoryType {
        PLAYER_SIGHT = 0,       ///< Remembered player sighting
        PLAYER_ATTACK,          ///< Player attacked entity
        PLAYER_DAMAGE,          ///< Player damaged entity
        PLAYER_LOCATION,        ///< Known player location
        DANGER_ZONE,            ///< Dangerous area
        SAFE_ZONE,              ///< Safe area
        FOOD_SOURCE,            ///< Food location
        WATER_SOURCE,           ///< Water location
        HOME_LOCATION,          ///< Home/base location
        THREAT_ENTITY,          ///< Threatening entity
        ALLY_ENTITY,            ///< Allied entity
        ITEM_LOCATION,          ///< Item pickup location
        BLOCK_INTERACTION,      ///< Block interaction memory
        PATH_MEMORY,            ///< Remembered path
        GOAL_COMPLETION,        ///< Completed goal
        FAILURE_EXPERIENCE      ///< Failed action
    };

    /**
     * @enum MemoryPriority
     * @brief Priority levels for memories
     */
    enum class MemoryPriority {
        LOW = 0,
        NORMAL,
        HIGH,
        CRITICAL
    };

    /**
     * @struct MemoryEntry
     * @brief Represents a single memory entry
     */
    struct MemoryEntry {
        MemoryType type;
        MemoryPriority priority;
        glm::vec3 position;
        std::chrono::steady_clock::time_point timestamp;
        float strength = 1.0f;          // How strong/important the memory is
        float decayRate = 0.01f;        // How fast memory fades
        Entity* relatedEntity = nullptr;
        std::string description;
        std::unordered_map<std::string, std::any> metadata;

        MemoryEntry(MemoryType t = MemoryType::PLAYER_SIGHT,
                   MemoryPriority p = MemoryPriority::NORMAL,
                   const glm::vec3& pos = glm::vec3(0.0f))
            : type(t), priority(p), position(pos), timestamp(std::chrono::steady_clock::now()) {}

        /**
         * @brief Get memory age in seconds
         * @return Age in seconds
         */
        float GetAge() const {
            auto now = std::chrono::steady_clock::now();
            return std::chrono::duration<float>(now - timestamp).count();
        }

        /**
         * @brief Check if memory is expired
         * @param maxAge Maximum age before expiration
         * @return true if expired
         */
        bool IsExpired(float maxAge) const {
            return GetAge() > maxAge;
        }

        /**
         * @brief Get current memory strength after decay
         * @return Current strength
         */
        float GetCurrentStrength() const {
            float age = GetAge();
            return strength * std::exp(-decayRate * age);
        }
    };

    /**
     * @struct MemoryQuery
     * @brief Query for searching memories
     */
    struct MemoryQuery {
        MemoryType type = MemoryType::PLAYER_SIGHT;
        glm::vec3 position = glm::vec3(0.0f);
        float maxDistance = 32.0f;
        float minStrength = 0.1f;
        float maxAge = 300.0f; // 5 minutes
        MemoryPriority minPriority = MemoryPriority::LOW;
        Entity* relatedEntity = nullptr;

        /**
         * @brief Check if memory matches query
         * @param memory Memory to check
         * @return true if matches
         */
        bool Matches(const MemoryEntry& memory) const {
            if (type != memory.type) return false;
            if (relatedEntity && relatedEntity != memory.relatedEntity) return false;
            if (memory.priority < minPriority) return false;
            if (memory.GetCurrentStrength() < minStrength) return false;
            if (memory.IsExpired(maxAge)) return false;

            if (maxDistance > 0.0f) {
                float distance = glm::distance(position, memory.position);
                if (distance > maxDistance) return false;
            }

            return true;
        }
    };

    /**
     * @struct MemoryPattern
     * @brief Pattern recognition for memories
     */
    struct MemoryPattern {
        std::string name;
        std::vector<MemoryType> requiredTypes;
        float timeWindow = 60.0f; // Time window in seconds
        int minOccurrences = 2;
        std::function<void(const std::vector<MemoryEntry>&)> callback;

        /**
         * @brief Check if pattern matches in memory list
         * @param memories List of memories to check
         * @return true if pattern matches
         */
        bool Matches(const std::vector<MemoryEntry>& memories) const;
    };

    /**
     * @class MemorySystem
     * @brief Memory system for AI entities
     */
    class MemorySystem {
    public:
        /**
         * @brief Constructor
         * @param entity Owner entity
         * @param maxMemories Maximum number of memories to store
         */
        MemorySystem(Entity* entity, int maxMemories = 1000);

        /**
         * @brief Destructor
         */
        ~MemorySystem() = default;

        /**
         * @brief Update memory system
         * @param deltaTime Time since last update
         */
        void Update(float deltaTime);

        /**
         * @brief Add new memory
         * @param memory Memory to add
         */
        void AddMemory(const MemoryEntry& memory);

        /**
         * @brief Add new memory with parameters
         * @param type Memory type
         * @param position Memory position
         * @param priority Memory priority
         * @param strength Initial strength
         * @param description Memory description
         */
        void AddMemory(MemoryType type, const glm::vec3& position,
                      MemoryPriority priority = MemoryPriority::NORMAL,
                      float strength = 1.0f, const std::string& description = "");

        /**
         * @brief Remove memory
         * @param memory Memory to remove
         */
        void RemoveMemory(const MemoryEntry& memory);

        /**
         * @brief Clear all memories
         */
        void ClearMemories();

        /**
         * @brief Query memories
         * @param query Memory query
         * @return Vector of matching memories
         */
        std::vector<MemoryEntry> QueryMemories(const MemoryQuery& query) const;

        /**
         * @brief Get memories by type
         * @param type Memory type
         * @return Vector of memories of specified type
         */
        std::vector<MemoryEntry> GetMemoriesByType(MemoryType type) const;

        /**
         * @brief Get memories near position
         * @param position Center position
         * @param radius Search radius
         * @return Vector of nearby memories
         */
        std::vector<MemoryEntry> GetMemoriesNearPosition(const glm::vec3& position, float radius) const;

        /**
         * @brief Get strongest memory of type
         * @param type Memory type
         * @return Strongest memory or nullptr if none found
         */
        const MemoryEntry* GetStrongestMemory(MemoryType type) const;

        /**
         * @brief Get most recent memory of type
         * @param type Memory type
         * @return Most recent memory or nullptr if none found
         */
        const MemoryEntry* GetMostRecentMemory(MemoryType type) const;

        /**
         * @brief Get memory count
         * @return Number of stored memories
         */
        size_t GetMemoryCount() const { return m_memories.size(); }

        /**
         * @brief Get memory statistics
         * @return Memory stats
         */
        const MemoryStats& GetStats() const { return m_stats; }

        /**
         * @brief Set memory parameters
         * @param maxMemories Maximum memories
         * @param defaultDecayRate Default decay rate
         * @param cleanupInterval Cleanup interval
         */
        void SetParameters(int maxMemories, float defaultDecayRate, float cleanupInterval);

        /**
         * @brief Add memory pattern for recognition
         * @param pattern Pattern to add
         */
        void AddPattern(const MemoryPattern& pattern);

        /**
         * @brief Remove memory pattern
         * @param patternName Pattern name to remove
         */
        void RemovePattern(const std::string& patternName);

    private:
        Entity* m_entity;
        std::vector<MemoryEntry> m_memories;
        std::vector<MemoryPattern> m_patterns;
        MemoryStats m_stats;

        // Parameters
        int m_maxMemories;
        float m_defaultDecayRate = 0.01f;
        float m_cleanupInterval = 10.0f;
        float m_cleanupTimer = 0.0f;

        /**
         * @brief Clean up expired and decayed memories
         */
        void CleanupMemories();

        /**
         * @brief Update memory strengths (decay)
         * @param deltaTime Time since last update
         */
        void UpdateMemoryStrengths(float deltaTime);

        /**
         * @brief Check for memory patterns
         */
        void CheckPatterns();

        /**
         * @brief Sort memories by priority and strength
         */
        void SortMemories();

        /**
         * @brief Remove excess memories if over limit
         */
        void EnforceMemoryLimit();
    };

    /**
     * @struct MemoryStats
     * @brief Statistics for memory system
     */
    struct MemoryStats {
        int totalMemories = 0;
        int addedMemories = 0;
        int removedMemories = 0;
        int expiredMemories = 0;
        int decayedMemories = 0;
        float averageMemoryAge = 0.0f;
        float averageMemoryStrength = 0.0f;
        std::unordered_map<MemoryType, int> memoriesByType;
        int patternMatches = 0;
    };

    /**
     * @class MemoryManager
     * @brief Global manager for memory systems
     */
    class MemoryManager {
    public:
        /**
         * @brief Get singleton instance
         * @return MemoryManager instance
         */
        static MemoryManager& GetInstance();

        /**
         * @brief Register entity memory system
         * @param entity Entity to register
         * @param system Memory system
         */
        void RegisterEntity(Entity* entity, std::unique_ptr<MemorySystem> system);

        /**
         * @brief Unregister entity memory system
         * @param entity Entity to unregister
         */
        void UnregisterEntity(Entity* entity);

        /**
         * @brief Update all memory systems
         * @param deltaTime Time since last update
         */
        void Update(float deltaTime);

        /**
         * @brief Get memory system for entity
         * @param entity Entity to get system for
         * @return Memory system or nullptr
         */
        MemorySystem* GetMemorySystem(Entity* entity) const;

        /**
         * @brief Broadcast memory to all entities
         * @param memory Memory to broadcast
         * @param sourceEntity Entity that generated the memory
         * @param range Broadcast range
         */
        void BroadcastMemory(const MemoryEntry& memory, Entity* sourceEntity, float range = 32.0f);

    private:
        MemoryManager() = default;
        ~MemoryManager() = default;
        MemoryManager(const MemoryManager&) = delete;
        MemoryManager& operator=(const MemoryManager&) = delete;

        std::unordered_map<Entity*, std::unique_ptr<MemorySystem>> m_memorySystems;
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_AI_MEMORY_SYSTEM_HPP
