/**
 * @file World.hpp
 * @brief VoxelCraft World System - Main World Coordinator
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_WORLD_WORLD_HPP
#define VOXELCRAFT_WORLD_WORLD_HPP

#include <memory>
#include <unordered_map>
#include <vector>
#include <string>
#include <atomic>
#include <mutex>
#include <queue>
#include <functional>

#include "Chunk.hpp"
#include "Biome.hpp"
#include "../blocks/Block.hpp"
#include "../entities/Entity.hpp"

namespace VoxelCraft {

    // Forward declarations
    class Player;
    class WorldGenerator;
    struct Vec3;
    struct Vec2;

    /**
     * @enum WorldType
     * @brief Type of world generation
     */
    enum class WorldType {
        INFINITE,        // Infinite procedural world
        FLAT,           // Flat world
        CUSTOM          // Custom world with specific settings
    };

    /**
     * @enum WorldState
     * @brief Current state of the world
     */
    enum class WorldState {
        UNINITIALIZED,
        INITIALIZING,
        LOADING,
        READY,
        SAVING,
        ERROR
    };

    /**
     * @struct WorldSettings
     * @brief Configuration settings for world generation
     */
    struct WorldSettings {
        std::string worldName;
        std::string seed;
        WorldType worldType;
        int worldHeight;
        int seaLevel;
        int renderDistance;    // In chunks
        int simulationDistance;
        bool generateCaves;
        bool generateTrees;
        bool generateStructures;
        bool generateOres;

        WorldSettings()
            : worldName("New World")
            , seed("0")
            , worldType(WorldType::INFINITE)
            , worldHeight(256)
            , seaLevel(63)
            , renderDistance(8)
            , simulationDistance(6)
            , generateCaves(true)
            , generateTrees(true)
            , generateStructures(true)
            , generateOres(true)
        {}
    };

    /**
     * @struct WorldStats
     * @brief World statistics and performance info
     */
    struct WorldStats {
        int loadedChunks;
        int generatedChunks;
        int totalChunks;
        int entities;
        int players;
        float chunkLoadTime;   // Average chunk load time in ms
        float chunkGenTime;    // Average chunk generation time in ms
        size_t memoryUsage;    // Memory used by world data
        int chunksInQueue;     // Chunks waiting to be loaded/generated
    };

    /**
     * @typedef WorldEventCallback
     * @brief Callback for world events
     */
    using WorldEventCallback = std::function<void(const std::string& eventType, const Vec3& position)>;

    /**
     * @class World
     * @brief Main world coordinator managing chunks, entities, and world state
     */
    class World {
    public:
        /**
         * @brief Constructor
         * @param settings World settings
         */
        World(const WorldSettings& settings);

        /**
         * @brief Destructor
         */
        ~World();

        /**
         * @brief Initialize the world
         * @return true if successful
         */
        bool Initialize();

        /**
         * @brief Shutdown the world
         */
        void Shutdown();

        /**
         * @brief Update world logic
         * @param deltaTime Time since last update
         * @param playerPosition Current player position
         */
        void Update(float deltaTime, const Vec3& playerPosition);

        /**
         * @brief Render the world
         * @param cameraPosition Camera position for culling
         */
        void Render(const Vec3& cameraPosition);

        // Block operations

        /**
         * @brief Get block at world position
         * @param worldX World X coordinate
         * @param worldY World Y coordinate
         * @param worldZ World Z coordinate
         * @return Block type at position
         */
        BlockType GetBlock(int worldX, int worldY, int worldZ) const;

        /**
         * @brief Set block at world position
         * @param worldX World X coordinate
         * @param worldY World Y coordinate
         * @param worldZ World Z coordinate
         * @param blockType New block type
         * @param updateNeighbors Whether to update neighboring blocks
         */
        void SetBlock(int worldX, int worldY, int worldZ, BlockType blockType, bool updateNeighbors = true);

        /**
         * @brief Check if block position is valid
         * @param worldX World X coordinate
         * @param worldY World Y coordinate
         * @param worldZ World Z coordinate
         * @return true if position is valid
         */
        bool IsValidBlockPosition(int worldX, int worldY, int worldZ) const;

        // Chunk operations

        /**
         * @brief Get chunk at chunk position
         * @param chunkPos Chunk position
         * @return Chunk pointer or nullptr if not loaded
         */
        std::shared_ptr<Chunk> GetChunk(const ChunkPosition& chunkPos) const;

        /**
         * @brief Get chunk at world position
         * @param worldX World X coordinate
         * @param worldZ World Z coordinate
         * @return Chunk pointer or nullptr if not loaded
         */
        std::shared_ptr<Chunk> GetChunkAt(int worldX, int worldZ) const;

        /**
         * @brief Load chunk at position
         * @param chunkPos Chunk position
         * @param generateIfMissing Generate chunk if not found
         * @return true if chunk is loaded
         */
        bool LoadChunk(const ChunkPosition& chunkPos, bool generateIfMissing = true);

        /**
         * @brief Unload chunk at position
         * @param chunkPos Chunk position
         */
        void UnloadChunk(const ChunkPosition& chunkPos);

        /**
         * @brief Save chunk to disk
         * @param chunkPos Chunk position
         */
        void SaveChunk(const ChunkPosition& chunkPos);

        /**
         * @brief Generate chunk terrain
         * @param chunkPos Chunk position
         * @return true if generated successfully
         */
        bool GenerateChunk(const ChunkPosition& chunkPos);

        // Entity operations

        /**
         * @brief Add entity to world
         * @param entity Entity to add
         */
        void AddEntity(std::shared_ptr<Entity> entity);

        /**
         * @brief Remove entity from world
         * @param entity Entity to remove
         */
        void RemoveEntity(std::shared_ptr<Entity> entity);

        /**
         * @brief Get entities in area
         * @param center Center position
         * @param radius Search radius
         * @return Vector of entities in area
         */
        std::vector<std::shared_ptr<Entity>> GetEntitiesInArea(const Vec3& center, float radius) const;

        // World queries

        /**
         * @brief Get biome at position
         * @param worldX World X coordinate
         * @param worldZ World Z coordinate
         * @return Biome type
         */
        BiomeType GetBiomeAt(int worldX, int worldZ) const;

        /**
         * @brief Get height at position (top non-air block)
         * @param worldX World X coordinate
         * @param worldZ World Z coordinate
         * @return Height value
         */
        int GetHeightAt(int worldX, int worldZ) const;

        /**
         * @brief Check if position is solid (non-air block)
         * @param worldX World X coordinate
         * @param worldY World Y coordinate
         * @param worldZ World Z coordinate
         * @return true if solid
         */
        bool IsSolidBlock(int worldX, int worldY, int worldZ) const;

        /**
         * @brief Find ground level at position
         * @param worldX World X coordinate
         * @param worldZ World Z coordinate
         * @param startY Starting Y position
         * @return Ground level Y coordinate
         */
        int FindGroundLevel(int worldX, int worldZ, int startY = 255) const;

        /**
         * @brief Cast ray through world
         * @param start Ray start position
         * @param direction Ray direction (normalized)
         * @param maxDistance Maximum ray distance
         * @param hitPosition Output hit position
         * @param hitNormal Output hit normal
         * @return true if hit something
         */
        bool Raycast(const Vec3& start, const Vec3& direction, float maxDistance,
                    Vec3& hitPosition, Vec3& hitNormal) const;

        // World management

        /**
         * @brief Get world settings
         * @return World settings
         */
        const WorldSettings& GetSettings() const { return m_settings; }

        /**
         * @brief Get world statistics
         * @return World stats
         */
        const WorldStats& GetStats() const { return m_stats; }

        /**
         * @brief Get world state
         * @return World state
         */
        WorldState GetState() const { return m_state; }

        /**
         * @brief Save entire world
         * @return true if saved successfully
         */
        bool SaveWorld();

        /**
         * @brief Load world from disk
         * @return true if loaded successfully
         */
        bool LoadWorld();

        /**
         * @brief Register world event callback
         * @param callback Callback function
         * @return Callback ID
         */
        int RegisterEventCallback(WorldEventCallback callback);

        /**
         * @brief Unregister event callback
         * @param callbackId Callback ID
         */
        void UnregisterEventCallback(int callbackId);

    private:
        WorldSettings m_settings;
        WorldState m_state;
        WorldStats m_stats;

        // Chunk management
        mutable std::mutex m_chunkMutex;
        std::unordered_map<ChunkPosition, std::shared_ptr<Chunk>, ChunkPosition::Hash> m_loadedChunks;
        std::queue<ChunkPosition> m_chunkLoadQueue;
        std::queue<ChunkPosition> m_chunkUnloadQueue;

        // Entity management
        mutable std::mutex m_entityMutex;
        std::vector<std::shared_ptr<Entity>> m_entities;
        std::shared_ptr<Player> m_player;

        // World generation
        std::unique_ptr<WorldGenerator> m_worldGenerator;

        // Event system
        std::vector<WorldEventCallback> m_eventCallbacks;
        int m_nextCallbackId;

        // Threading
        std::atomic<bool> m_worldThreadRunning;
        std::thread m_worldThread;

        // Internal methods

        /**
         * @brief World update thread function
         */
        void WorldUpdateThread();

        /**
         * @brief Process chunk loading queue
         */
        void ProcessChunkQueues();

        /**
         * @brief Update chunk loading based on player position
         * @param playerPos Player position
         */
        void UpdateChunkLoading(const Vec3& playerPos);

        /**
         * @brief Generate chunks around position
         * @param centerPos Center position
         */
        void GenerateChunksAround(const Vec3& centerPos);

        /**
         * @brief Unload distant chunks
         * @param centerPos Center position
         */
        void UnloadDistantChunks(const Vec3& centerPos);

        /**
         * @brief Update world lighting
         */
        void UpdateLighting();

        /**
         * @brief Update entity positions and handle collisions
         * @param deltaTime Time since last update
         */
        void UpdateEntities(float deltaTime);

        /**
         * @brief Check entity collisions with world
         * @param entity Entity to check
         */
        void CheckEntityCollisions(std::shared_ptr<Entity> entity);

        /**
         * @brief Handle block placement/modification events
         * @param worldX World X coordinate
         * @param worldY World Y coordinate
         * @param worldZ World Z coordinate
         */
        void OnBlockChanged(int worldX, int worldY, int worldZ);

        /**
         * @brief Convert world coordinates to chunk coordinates
         * @param worldX World X coordinate
         * @param worldZ World Z coordinate
         * @param chunkX Output chunk X coordinate
         * @param chunkZ Output chunk Z coordinate
         * @param localX Output local X coordinate within chunk
         * @param localZ Output local Z coordinate within chunk
         */
        static void WorldToChunkCoordinates(int worldX, int worldZ,
                                          int& chunkX, int& chunkZ,
                                          int& localX, int& localZ);

        /**
         * @brief Update world statistics
         */
        void UpdateStats();
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_WORLD_WORLD_HPP