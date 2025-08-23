/**
 * @file StructureManager.hpp
 * @brief VoxelCraft Structure Manager - Manages all structures in the world
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_STRUCTURES_STRUCTURE_MANAGER_HPP
#define VOXELCRAFT_STRUCTURES_STRUCTURE_MANAGER_HPP

#include <memory>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <array>
#include <random>
#include <chrono>
#include <glm/glm.hpp>

#include "Structure.hpp"

namespace VoxelCraft {

    class World;
    class Player;
    class Biome;

    /**
     * @enum StructureGenerationMode
     * @brief Different modes for structure generation
     */
    enum class StructureGenerationMode {
        CLASSIC = 0,              ///< Classic Minecraft generation
        MODERN,                   ///< Modern generation with better distribution
        CLUSTERED,               ///< Structures cluster together
        SCATTERED,               ///< Structures are spread out
        CUSTOM                   ///< Custom generation rules
    };

    /**
     * @struct StructureSpawnInfo
     * @brief Information about structure spawning
     */
    struct StructureSpawnInfo {
        StructureType type;
        glm::ivec3 position;
        StructureRarity rarity;
        float spawnTime;
        std::unordered_map<std::string, std::any> customData;
    };

    /**
     * @struct StructureGenerationConfig
     * @brief Configuration for structure generation
     */
    struct StructureGenerationConfig {
        StructureGenerationMode mode;
        bool enableStructureGeneration;    ///< Whether structures can generate
        bool enableNaturalSpawning;        ///< Whether structures spawn naturally
        bool enablePlayerTriggered;        ///< Whether structures can be triggered by players
        bool respectBiomeRules;            ///< Whether to respect biome-specific rules
        bool respectTerrain;               ///< Whether to respect terrain when placing
        bool allowOverlapping;             ///< Whether structures can overlap
        float globalSpawnMultiplier;       ///< Global multiplier for spawn chances
        float rareStructureMultiplier;     ///< Multiplier for rare structures
        float epicStructureMultiplier;     ///< Multiplier for epic structures
        int minDistanceBetweenStructures;  ///< Minimum distance between any structures
        int maxStructuresPerChunk;         ///< Maximum structures per chunk
        int structureCheckRadius;          ///< Radius to check for structure conflicts
        bool generateInCreative;           ///< Whether to generate in creative mode
        bool generateInSurvival;           ///< Whether to generate in survival mode
        int minYLevel;                     ///< Minimum Y level for structure generation
        int maxYLevel;                     ///< Maximum Y level for structure generation
        std::vector<StructureType> disabledStructures; ///< Structures that are disabled
        std::vector<StructureType> forcedStructures;   ///< Structures that are forced to generate
    };

    /**
     * @struct StructureCache
     * @brief Cached structure data for performance
     */
    struct StructureCache {
        std::unordered_map<glm::ivec2, std::vector<StructureInstance>> structureMap;
        std::unordered_map<StructureType, int> structureCounts;
        std::unordered_map<glm::ivec3, StructureInstance*> positionToStructure;
        std::chrono::steady_clock::time_point lastUpdate;
        int cacheRadius;                   ///< Radius of cached data in chunks

        /**
         * @brief Check if chunk has cached structures
         * @param chunkPos Chunk position
         * @return true if cached
         */
        bool IsCached(const glm::ivec2& chunkPos) const {
            return structureMap.find(chunkPos) != structureMap.end();
        }

        /**
         * @brief Clear cache
         */
        void Clear() {
            structureMap.clear();
            structureCounts.clear();
            positionToStructure.clear();
        }

        /**
         * @brief Get total structure count
         * @return Total structures
         */
        size_t GetTotalCount() const {
            size_t total = 0;
            for (const auto& pair : structureCounts) {
                total += pair.second;
            }
            return total;
        }
    };

    /**
     * @class StructureManager
     * @brief Central manager for all structure-related functionality
     */
    class StructureManager {
    public:
        /**
         * @brief Get singleton instance
         * @return StructureManager instance
         */
        static StructureManager& GetInstance();

        /**
         * @brief Initialize structure manager
         * @param world World reference
         * @return true if successful
         */
        bool Initialize(World* world);

        /**
         * @brief Shutdown structure manager
         */
        void Shutdown();

        /**
         * @brief Update structure system
         * @param deltaTime Time since last update
         */
        void Update(float deltaTime);

        /**
         * @brief Generate structures for chunk
         * @param chunkX Chunk X coordinate
         * @param chunkZ Chunk Z coordinate
         * @return Vector of generated structures
         */
        std::vector<StructureInstance> GenerateChunkStructures(int chunkX, int chunkZ);

        /**
         * @brief Generate specific structure at position
         * @param type Structure type
         * @param position World position
         * @return Generated structure instance or nullptr
         */
        StructureInstance* GenerateStructureAt(StructureType type, const glm::ivec3& position);

        /**
         * @brief Get structure at position
         * @param position World position
         * @return Structure instance or nullptr
         */
        StructureInstance* GetStructureAt(const glm::vec3& position) const;

        /**
         * @brief Get structures in area
         * @param center Center position
         * @param radius Search radius
         * @return Vector of structures in area
         */
        std::vector<StructureInstance*> GetStructuresInArea(const glm::vec3& center, float radius) const;

        /**
         * @brief Get structures by type
         * @param type Structure type
         * @return Vector of structures of specified type
         */
        std::vector<StructureInstance*> GetStructuresByType(StructureType type) const;

        /**
         * @brief Check if position is in structure
         * @param position World position
         * @return Structure instance if in one, nullptr otherwise
         */
        StructureInstance* IsPositionInStructure(const glm::vec3& position) const;

        /**
         * @brief Find nearest structure of type
         * @param type Structure type
         * @param position Search position
         * @param maxDistance Maximum search distance
         * @return Nearest structure or nullptr
         */
        StructureInstance* FindNearestStructure(StructureType type, const glm::vec3& position, float maxDistance = 1000.0f);

        /**
         * @brief Check if structure can spawn at position
         * @param type Structure type
         * @param position World position
         * @return true if can spawn
         */
        bool CanStructureSpawnAt(StructureType type, const glm::vec3& position) const;

        /**
         * @brief Get structure by type
         * @param type Structure type
         * @return Structure instance or nullptr
         */
        std::shared_ptr<Structure> GetStructure(StructureType type) const;

        /**
         * @brief Add custom structure
         * @param structure Structure to add
         * @return true if added successfully
         */
        bool AddCustomStructure(std::shared_ptr<Structure> structure);

        /**
         * @brief Remove custom structure
         * @param type Structure type to remove
         * @return true if removed successfully
         */
        bool RemoveCustomStructure(StructureType type);

        /**
         * @brief Set generation mode
         * @param mode New generation mode
         */
        void SetGenerationMode(StructureGenerationMode mode);

        /**
         * @brief Get current generation mode
         * @return Current generation mode
         */
        StructureGenerationMode GetGenerationMode() const { return m_config.mode; }

        /**
         * @brief Enable/disable structure generation
         * @param enabled Whether generation is enabled
         */
        void SetGenerationEnabled(bool enabled) { m_config.enableStructureGeneration = enabled; }

        /**
         * @brief Check if generation is enabled
         * @return true if enabled
         */
        bool IsGenerationEnabled() const { return m_config.enableStructureGeneration; }

        /**
         * @brief Get structure cache
         * @return Structure cache reference
         */
        const StructureCache& GetCache() const { return m_cache; }

        /**
         * @brief Clear structure cache
         */
        void ClearCache();

        /**
         * @brief Get structure statistics
         * @return Structure statistics
         */
        const StructureStats& GetStats() const { return m_stats; }

        /**
         * @brief Force generate structure (for debugging)
         * @param type Structure type
         * @param position World position
         * @return Generated structure or nullptr
         */
        StructureInstance* ForceGenerateStructure(StructureType type, const glm::vec3& position);

    private:
        StructureManager() = default;
        ~StructureManager() = default;

        // Prevent copying
        StructureManager(const StructureManager&) = delete;
        StructureManager& operator=(const StructureManager&) = delete;

        World* m_world;
        std::unordered_map<StructureType, std::shared_ptr<Structure>> m_structures;
        std::vector<StructureInstance> m_activeStructures;
        std::mt19937 m_randomEngine;
        StructureGenerationConfig m_config;
        StructureCache m_cache;
        StructureStats m_stats;
        bool m_initialized;

        // Generation helpers
        void InitializeDefaultStructures();
        void GenerateStructureForChunk(int chunkX, int chunkZ);
        bool ShouldGenerateStructureInChunk(StructureType type, int chunkX, int chunkZ) const;
        glm::ivec3 FindSuitablePosition(StructureType type, int chunkX, int chunkZ) const;
        bool CheckStructureConflicts(const glm::ivec3& position, StructureType type) const;
        void CacheStructure(const StructureInstance& structure);
        void UpdateCacheAroundPlayer(const glm::vec3& playerPos);

        /**
         * @brief Create specific structure types
         * @param type Structure type
         * @return Structure instance
         */
        std::shared_ptr<Structure> CreateStructure(StructureType type) const;

        /**
         * @brief Generate structure seed for position
         * @param x X coordinate
         * @param z Z coordinate
         * @return Seed value
         */
        uint32_t GenerateStructureSeed(int x, int z) const;

        /**
         * @brief Check biome compatibility for structure
         * @param type Structure type
         * @param position World position
         * @return true if compatible
         */
        bool IsBiomeCompatible(StructureType type, const glm::vec3& position) const;

        /**
         * @brief Apply generation modifiers
         * @param type Structure type
         * @return Modified spawn chance
         */
        float ApplyGenerationModifiers(StructureType type) const;

        /**
         * @brief Validate structure position
         * @param position World position
         * @param type Structure type
         * @return true if valid
         */
        bool ValidateStructurePosition(const glm::vec3& position, StructureType type) const;
    };

    /**
     * @struct StructureStats
     * @brief Statistics for structure system
     */
    struct StructureStats {
        size_t totalStructures = 0;
        size_t cachedStructures = 0;
        size_t generatedThisSession = 0;
        size_t failedGenerations = 0;
        float averageGenerationTime = 0.0f;
        std::unordered_map<StructureType, size_t> structuresByType;
        std::unordered_map<StructureRarity, size_t> structuresByRarity;
        std::unordered_map<std::string, size_t> structuresByBiome;
        std::vector<std::pair<StructureType, size_t>> mostCommonStructures;
        int chunksGenerated = 0;
        int structuresPerChunk = 0;
        int averageStructureSize = 0;
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_STRUCTURES_STRUCTURE_MANAGER_HPP
