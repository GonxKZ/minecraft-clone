/**
 * @file MobManager.hpp
 * @brief VoxelCraft Mob Manager - Manages all mobs in the world
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_MOB_MOB_MANAGER_HPP
#define VOXELCRAFT_MOB_MOB_MANAGER_HPP

#include <memory>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <chrono>
#include <glm/glm.hpp>

#include "Mob.hpp"

namespace VoxelCraft {

    class World;
    class Player;
    class Biome;

    /**
     * @enum MobSpawnReason
     * @brief Reasons why mobs spawn
     */
    enum class MobSpawnReason {
        NATURAL = 0,           ///< Natural spawning
        SPAWNER,               ///< Spawned by spawner
        BREEDING,              ///< Spawned by breeding
        SPAWN_EGG,             ///< Spawned by spawn egg
        COMMAND,               ///< Spawned by command
        CUSTOM,                ///< Custom spawning
        REINFORCEMENTS         ///< Spawned as reinforcements
    };

    /**
     * @struct MobSpawnInfo
     * @brief Information about mob spawning
     */
    struct MobSpawnInfo {
        MobType type;
        glm::vec3 position;
        MobSpawnReason reason;
        Entity* spawner;              ///< Entity that caused spawning
        float spawnTime;              ///< Time when spawned
        std::unordered_map<std::string, std::any> customData;
    };

    /**
     * @struct MobSpawnRules
     * @brief Rules for mob spawning in different conditions
     */
    struct MobSpawnRules {
        int minLightLevel;            ///< Minimum light level for spawning
        int maxLightLevel;            ///< Maximum light level for spawning
        int minY;                     ///< Minimum Y level for spawning
        int maxY;                     ///< Maximum Y level for spawning
        int minGroupSize;             ///< Minimum group size
        int maxGroupSize;             ///< Maximum group size
        float spawnWeight;            ///< Spawn weight (higher = more common)
        std::vector<std::string> requiredBiomes; ///< Biomes where this mob can spawn
        std::vector<std::string> blockedBiomes;  ///< Biomes where this mob cannot spawn
        bool needsSurface;            ///< Whether mob needs to spawn on surface
        bool needsDarkness;           ///< Whether mob needs darkness to spawn
        bool canSpawnInWater;         ///< Whether mob can spawn in water
        bool canSpawnUnderground;     ///< Whether mob can spawn underground
        int minDistanceFromPlayer;    ///< Minimum distance from nearest player
        int maxDistanceFromPlayer;    ///< Maximum distance from nearest player
    };

    /**
     * @struct MobPack
     * @brief A group of mobs that spawn together
     */
    struct MobPack {
        MobType leaderType;
        std::vector<MobType> memberTypes;
        int minSize;
        int maxSize;
        float cohesion;               ///< How closely the pack stays together
        float packRadius;             ///< Maximum distance between pack members
        MobBehavior packBehavior;     ///< Behavior exhibited by the pack
    };

    /**
     * @struct MobSpawner
     * @brief A mob spawner block and its configuration
     */
    struct MobSpawner {
        glm::ivec3 position;
        MobType spawnType;
        int spawnDelay;               ///< Current spawn delay
        int minSpawnDelay;            ///< Minimum spawn delay
        int maxSpawnDelay;            ///< Maximum spawn delay
        int spawnCount;               ///< Number of mobs to spawn at once
        int maxNearbyEntities;        ///< Maximum entities that can be near spawner
        int requiredPlayerRange;      ///< Player must be within this range
        int spawnRange;               ///< Spawning area radius
        bool isActive;                ///< Whether spawner is active
        std::chrono::steady_clock::time_point lastSpawnTime;
        std::vector<MobSpawnInfo> spawnedMobs;
    };

    /**
     * @class MobManager
     * @brief Central manager for all mob-related functionality
     */
    class MobManager {
    public:
        /**
         * @brief Get singleton instance
         * @return MobManager instance
         */
        static MobManager& GetInstance();

        /**
         * @brief Initialize mob manager
         * @param world World reference
         * @return true if successful
         */
        bool Initialize(World* world);

        /**
         * @brief Shutdown mob manager
         */
        void Shutdown();

        /**
         * @brief Update all mobs and spawning systems
         * @param deltaTime Time since last update
         */
        void Update(float deltaTime);

        /**
         * @brief Spawn a mob at position
         * @param type Mob type to spawn
         * @param position Spawn position
         * @param reason Spawn reason
         * @param spawner Entity that caused spawning
         * @return Spawned mob or nullptr
         */
        std::shared_ptr<Mob> SpawnMob(MobType type, const glm::vec3& position,
                                    MobSpawnReason reason = MobSpawnReason::NATURAL,
                                    Entity* spawner = nullptr);

        /**
         * @brief Remove a mob
         * @param mob Mob to remove
         * @return true if removed successfully
         */
        bool RemoveMob(std::shared_ptr<Mob> mob);

        /**
         * @brief Remove mob by ID
         * @param mobId Mob ID to remove
         * @return true if removed successfully
         */
        bool RemoveMob(uint32_t mobId);

        /**
         * @brief Get mob by ID
         * @param mobId Mob ID
         * @return Mob or nullptr
         */
        std::shared_ptr<Mob> GetMob(uint32_t mobId) const;

        /**
         * @brief Get all mobs
         * @return Vector of all mobs
         */
        std::vector<std::shared_ptr<Mob>> GetAllMobs() const;

        /**
         * @brief Get mobs in area
         * @param center Center position
         * @param radius Search radius
         * @return Vector of mobs in area
         */
        std::vector<std::shared_ptr<Mob>> GetMobsInArea(const glm::vec3& center, float radius) const;

        /**
         * @brief Get mobs by type
         * @param type Mob type
         * @return Vector of mobs of specified type
         */
        std::vector<std::shared_ptr<Mob>> GetMobsByType(MobType type) const;

        /**
         * @brief Get nearest mob to position
         * @param position Search position
         * @param maxDistance Maximum search distance
         * @return Nearest mob or nullptr
         */
        std::shared_ptr<Mob> GetNearestMob(const glm::vec3& position, float maxDistance = 32.0f) const;

        /**
         * @brief Get mob count by type
         * @param type Mob type
         * @return Number of mobs of specified type
         */
        size_t GetMobCount(MobType type) const;

        /**
         * @brief Get total mob count
         * @return Total number of mobs
         */
        size_t GetTotalMobCount() const;

        /**
         * @brief Check if mob can spawn at position
         * @param type Mob type
         * @param position Position to check
         * @return true if can spawn
         */
        bool CanMobSpawnAt(MobType type, const glm::vec3& position) const;

        /**
         * @brief Get spawn rules for mob type
         * @param type Mob type
         * @return Spawn rules
         */
        const MobSpawnRules& GetSpawnRules(MobType type) const;

        /**
         * @brief Set spawn rules for mob type
         * @param type Mob type
         * @param rules Spawn rules
         */
        void SetSpawnRules(MobType type, const MobSpawnRules& rules);

        /**
         * @brief Create mob spawner
         * @param position Spawner position
         * @param type Mob type to spawn
         * @return true if created successfully
         */
        bool CreateMobSpawner(const glm::ivec3& position, MobType type);

        /**
         * @brief Remove mob spawner
         * @param position Spawner position
         * @return true if removed successfully
         */
        bool RemoveMobSpawner(const glm::ivec3& position);

        /**
         * @brief Get mob spawner at position
         * @param position Position to check
         * @return Mob spawner or nullptr
         */
        MobSpawner* GetMobSpawner(const glm::ivec3& position);

        /**
         * @brief Get all mob spawners
         * @return Vector of all spawners
         */
        std::vector<MobSpawner*> GetAllMobSpawners();

        /**
         * @brief Register mob pack
         * @param pack Mob pack to register
         */
        void RegisterMobPack(const MobPack& pack);

        /**
         * @brief Spawn mob pack
         * @param packType Pack type (index in registered packs)
         * @param position Spawn position
         * @return Vector of spawned mobs
         */
        std::vector<std::shared_ptr<Mob>> SpawnMobPack(size_t packType, const glm::vec3& position);

        /**
         * @brief Set mob spawning enabled/disabled
         * @param enabled Whether spawning is enabled
         */
        void SetSpawningEnabled(bool enabled);

        /**
         * @brief Check if mob spawning is enabled
         * @return true if enabled
         */
        bool IsSpawningEnabled() const { return m_spawningEnabled; }

        /**
         * @brief Set maximum mob count
         * @param maxCount Maximum number of mobs
         */
        void SetMaxMobCount(size_t maxCount) { m_maxMobCount = maxCount; }

        /**
         * @brief Get maximum mob count
         * @return Maximum mob count
         */
        size_t GetMaxMobCount() const { return m_maxMobCount; }

        /**
         * @brief Get mob statistics
         * @return Mob statistics
         */
        const MobStats& GetStats() const { return m_stats; }

        /**
         * @brief Clear all mobs
         */
        void ClearAllMobs();

        /**
         * @brief Clear all mob spawners
         */
        void ClearAllMobSpawners();

    private:
        MobManager() = default;
        ~MobManager() = default;

        // Prevent copying
        MobManager(const MobManager&) = delete;
        MobManager& operator=(const MobManager&) = delete;

        World* m_world;
        std::unordered_map<uint32_t, std::shared_ptr<Mob>> m_mobs;
        std::unordered_map<glm::ivec3, MobSpawner> m_spawners;
        std::unordered_map<MobType, MobSpawnRules> m_spawnRules;
        std::vector<MobPack> m_mobPacks;
        std::unordered_map<MobType, std::function<std::shared_ptr<Mob>(const glm::vec3&, World*)>> m_mobFactories;

        uint32_t m_nextMobId;
        bool m_initialized;
        bool m_spawningEnabled;
        size_t m_maxMobCount;
        MobStats m_stats;

        float m_naturalSpawnTimer;
        float m_spawnerUpdateTimer;
        float m_packSpawnTimer;

        // Natural spawning
        void UpdateNaturalSpawning(float deltaTime);
        void AttemptNaturalSpawn(const glm::vec3& position);
        bool ShouldSpawnMob(const glm::vec3& position) const;
        MobType SelectMobTypeForSpawn(const glm::vec3& position) const;

        // Mob spawners
        void UpdateMobSpawners(float deltaTime);
        void UpdateMobSpawner(MobSpawner& spawner, float deltaTime);
        bool ShouldSpawnerActivate(const MobSpawner& spawner) const;

        // Mob packs
        void UpdateMobPacks(float deltaTime);
        void SpawnRandomPack();

        // Helper functions
        uint32_t GenerateMobId();
        bool IsPositionValid(const glm::vec3& position) const;
        bool IsAreaClear(const glm::vec3& position, float radius) const;
        float GetDistanceToNearestPlayer(const glm::vec3& position) const;

        // Factory functions for creating specific mobs
        static std::shared_ptr<Mob> CreateCreeper(const glm::vec3& position, World* world);
        static std::shared_ptr<Mob> CreateZombie(const glm::vec3& position, World* world);
        static std::shared_ptr<Mob> CreateSkeleton(const glm::vec3& position, World* world);
        static std::shared_ptr<Mob> CreateSpider(const glm::vec3& position, World* world);
        static std::shared_ptr<Mob> CreateEnderman(const glm::vec3& position, World* world);
        static std::shared_ptr<Mob> CreateWolf(const glm::vec3& position, World* world);
        static std::shared_ptr<Mob> CreateChicken(const glm::vec3& position, World* world);
        static std::shared_ptr<Mob> CreateCow(const glm::vec3& position, World* world);
        static std::shared_ptr<Mob> CreatePig(const glm::vec3& position, World* world);
        static std::shared_ptr<Mob> CreateSheep(const glm::vec3& position, World* world);
        static std::shared_ptr<Mob> CreateVillager(const glm::vec3& position, World* world);

        void RegisterMobFactories();
        void InitializeDefaultSpawnRules();
        void InitializeDefaultMobPacks();
    };

    /**
     * @struct MobStats
     * @brief Statistics for mob system
     */
    struct MobStats {
        size_t totalMobsSpawned = 0;
        size_t totalMobsKilled = 0;
        size_t activeMobs = 0;
        size_t activeSpawners = 0;
        size_t totalSpawners = 0;
        float averageMobLifespan = 0.0f;
        std::unordered_map<MobType, size_t> mobsByType;
        std::unordered_map<MobSpawnReason, size_t> spawnReasons;
        std::unordered_map<std::string, size_t> deathsByCause;
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_MOB_MOB_MANAGER_HPP
