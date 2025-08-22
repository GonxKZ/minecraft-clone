/**
 * @file World.hpp
 * @brief VoxelCraft World Management System
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines the main World class that manages the game world,
 * including terrain generation, chunk management, lighting, weather,
 * and all world-related functionality.
 */

#ifndef VOXELCRAFT_WORLD_WORLD_HPP
#define VOXELCRAFT_WORLD_WORLD_HPP

#include <memory>
#include <unordered_map>
#include <vector>
#include <string>
#include <functional>
#include <atomic>
#include <mutex>
#include <shared_mutex>
#include <optional>
#include <deque>
#include <chrono>

#include "../core/Config.hpp"
#include "../entities/Entity.hpp"

namespace VoxelCraft {

    // Forward declarations
    class Chunk;
    class TerrainGenerator;
    class Biome;
    class NoiseGenerator;
    class LightingEngine;
    class WeatherSystem;
    class WorldRenderer;
    class BlockRegistry;
    class ChunkManager;

    /**
     * @enum WorldType
     * @brief Type of world generation
     */
    enum class WorldType {
        Normal,         ///< Standard world with biomes
        Flat,          ///< Flat world for creative mode
        SuperFlat,     ///< Super flat world (single layer)
        Custom,        ///< Custom world with specific settings
        Debug,         ///< Debug world for testing
        Amplified,     ///< Amplified terrain (extreme heights)
        LargeBiomes,   ///< Large biome world
        SingleBiome    ///< Single biome world
    };

    /**
     * @enum WorldState
     * @brief Current state of the world
     */
    enum class WorldState {
        Unloaded,      ///< World not loaded
        Loading,       ///< World is loading
        Loaded,        ///< World is loaded and ready
        Generating,    ///< World is generating terrain
        Saving,        ///< World is saving
        Error          ///< World encountered an error
    };

    /**
     * @struct WorldInfo
     * @brief Information about the world
     */
    struct WorldInfo {
        std::string name;                    ///< World name
        std::string seed;                    ///< World seed
        WorldType type;                      ///< World type
        int version;                         ///< World format version
        double creationTime;                 ///< World creation time
        double lastPlayedTime;               ///< Last played time
        int spawnX, spawnY, spawnZ;          ///< Spawn position
        int maxHeight;                       ///< Maximum world height
        int minHeight;                       ///< Minimum world height
        int seaLevel;                        ///< Sea level
        bool allowCheats;                    ///< Allow cheats
        bool difficultyLocked;               ///< Difficulty locked
        std::unordered_map<std::string, std::any> metadata; ///< Custom metadata
    };

    /**
     * @struct WorldMetrics
     * @brief Performance metrics for the world system
     */
    struct WorldMetrics {
        // Chunk metrics
        uint32_t totalChunks;                ///< Total number of chunks
        uint32_t loadedChunks;               ///< Number of loaded chunks
        uint32_t visibleChunks;              ///< Number of visible chunks
        uint32_t pendingChunks;              ///< Number of pending chunk operations

        // Generation metrics
        double averageGenerationTime;        ///< Average chunk generation time (ms)
        uint64_t chunksGenerated;            ///< Total chunks generated
        uint32_t generationQueueSize;        ///< Size of generation queue

        // Performance metrics
        double chunkLoadTime;                ///< Average chunk load time (ms)
        double chunkSaveTime;                ///< Average chunk save time (ms)
        uint32_t memoryUsage;                ///< Memory usage in MB
        double chunkUpdateRate;              ///< Chunk updates per second

        // Lighting metrics
        uint32_t lightUpdates;               ///< Number of light updates
        double lightingTime;                 ///< Time spent on lighting (ms)

        // World metrics
        uint32_t activeEntities;             ///< Number of active entities
        uint32_t blockUpdates;               ///< Block updates this frame
        double simulationTime;               ///< World simulation time (ms)
    };

    /**
     * @struct ChunkCoordinate
     * @brief 2D coordinate for chunk positioning
     */
    struct ChunkCoordinate {
        int x, z;                            ///< Chunk coordinates

        ChunkCoordinate(int x = 0, int z = 0) : x(x), z(z) {}
        bool operator==(const ChunkCoordinate& other) const { return x == other.x && z == other.z; }
        bool operator!=(const ChunkCoordinate& other) const { return !(*this == other); }

        // Calculate distance to another chunk
        double DistanceTo(const ChunkCoordinate& other) const {
            int dx = x - other.x;
            int dz = z - other.z;
            return std::sqrt(dx * dx + dz * dz);
        }

        // Manhattan distance
        int ManhattanDistance(const ChunkCoordinate& other) const {
            return std::abs(x - other.x) + std::abs(z - other.z);
        }
    };

    /**
     * @struct BlockCoordinate
     * @brief 3D coordinate for block positioning
     */
    struct BlockCoordinate {
        int x, y, z;                         ///< Block coordinates

        BlockCoordinate(int x = 0, int y = 0, int z = 0) : x(x), y(y), z(z) {}
        bool operator==(const BlockCoordinate& other) const {
            return x == other.x && y == other.y && z == other.z;
        }
        bool operator!=(const BlockCoordinate& other) const { return !(*this == other); }

        // Convert to chunk coordinate
        ChunkCoordinate ToChunkCoordinate() const {
            return ChunkCoordinate(x >> 4, z >> 4); // Assuming 16x16 chunks
        }

        // Get local chunk position
        BlockCoordinate GetLocalPosition() const {
            return BlockCoordinate(x & 15, y, z & 15); // 16x16 chunks
        }
    };

    /**
     * @class World
     * @brief Main world management class
     *
     * The World class is responsible for:
     * - Managing all chunks in the world
     * - Coordinating terrain generation
     * - Handling lighting and weather systems
     * - Managing world state and persistence
     * - Providing access to blocks and entities
     * - Optimizing performance through LOD and culling
     */
    class World {
    public:
        /**
         * @brief Constructor
         * @param name World name
         * @param seed World seed
         * @param type World type
         */
        explicit World(const std::string& name,
                      const std::string& seed = "default",
                      WorldType type = WorldType::Normal);

        /**
         * @brief Destructor
         */
        ~World();

        /**
         * @brief Deleted copy constructor
         */
        World(const World&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        World& operator=(const World&) = delete;

        // World lifecycle

        /**
         * @brief Initialize the world
         * @return true if successful, false otherwise
         */
        bool Initialize();

        /**
         * @brief Load world from disk
         * @param path World save path
         * @return true if successful, false otherwise
         */
        bool Load(const std::string& path);

        /**
         * @brief Save world to disk
         * @param path World save path
         * @return true if successful, false otherwise
         */
        bool Save(const std::string& path);

        /**
         * @brief Unload the world
         */
        void Unload();

        /**
         * @brief Update world simulation
         * @param deltaTime Time elapsed since last update
         */
        void Update(double deltaTime);

        /**
         * @brief Render the world
         * @param frustum View frustum for culling
         */
        void Render(const class Frustum& frustum);

        // World information

        /**
         * @brief Get world information
         * @return World info
         */
        const WorldInfo& GetInfo() const { return m_info; }

        /**
         * @brief Get world state
         * @return World state
         */
        WorldState GetState() const { return m_state; }

        /**
         * @brief Get world metrics
         * @return World metrics
         */
        const WorldMetrics& GetMetrics() const { return m_metrics; }

        /**
         * @brief Get world name
         * @return World name
         */
        const std::string& GetName() const { return m_info.name; }

        /**
         * @brief Get world seed
         * @return World seed
         */
        const std::string& GetSeed() const { return m_info.seed; }

        // Chunk management

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
         * @brief Load chunk at coordinates
         * @param x Chunk X coordinate
         * @param z Chunk Z coordinate
         * @param priority Loading priority
         * @return true if loading started, false otherwise
         */
        bool LoadChunk(int x, int z, int priority = 0);

        /**
         * @brief Unload chunk at coordinates
         * @param x Chunk X coordinate
         * @param z Chunk Z coordinate
         * @return true if unloading started, false otherwise
         */
        bool UnloadChunk(int x, int z);

        /**
         * @brief Check if chunk is loaded
         * @param x Chunk X coordinate
         * @param z Chunk Z coordinate
         * @return true if loaded, false otherwise
         */
        bool IsChunkLoaded(int x, int z) const;

        /**
         * @brief Get loaded chunks count
         * @return Number of loaded chunks
         */
        size_t GetLoadedChunksCount() const;

        // Block access

        /**
         * @brief Get block at world coordinates
         * @param x World X coordinate
         * @param y World Y coordinate
         * @param z World Z coordinate
         * @return Block ID or 0 if not loaded
         */
        uint32_t GetBlock(int x, int y, int z) const;

        /**
         * @brief Set block at world coordinates
         * @param x World X coordinate
         * @param y World Y coordinate
         * @param z World Z coordinate
         * @param blockId Block ID to set
         * @return true if successful, false otherwise
         */
        bool SetBlock(int x, int y, int z, uint32_t blockId);

        /**
         * @brief Get block at block coordinate
         * @param coord Block coordinate
         * @return Block ID or 0 if not loaded
         */
        uint32_t GetBlock(const BlockCoordinate& coord) const;

        /**
         * @brief Set block at block coordinate
         * @param coord Block coordinate
         * @param blockId Block ID to set
         * @return true if successful, false otherwise
         */
        bool SetBlock(const BlockCoordinate& coord, uint32_t blockId);

        // Entity management

        /**
         * @brief Add entity to world
         * @param entity Entity to add
         * @return true if successful, false otherwise
         */
        bool AddEntity(std::unique_ptr<Entity> entity);

        /**
         * @brief Remove entity from world
         * @param entity Entity to remove
         * @return true if successful, false otherwise
         */
        bool RemoveEntity(Entity* entity);

        /**
         * @brief Get entities in area
         * @param minX Minimum X coordinate
         * @param minY Minimum Y coordinate
         * @param minZ Minimum Z coordinate
         * @param maxX Maximum X coordinate
         * @param maxY Maximum Y coordinate
         * @param maxZ Maximum Z coordinate
         * @return Vector of entities in area
         */
        std::vector<Entity*> GetEntitiesInArea(int minX, int minY, int minZ,
                                               int maxX, int maxY, int maxZ);

        /**
         * @brief Get entities at position
         * @param x X coordinate
         * @param y Y coordinate
         * @param z Z coordinate
         * @return Vector of entities at position
         */
        std::vector<Entity*> GetEntitiesAt(int x, int y, int z);

        // World generation

        /**
         * @brief Generate terrain at coordinates
         * @param x World X coordinate
         * @param z World Z coordinate
         * @param force Force regeneration if already exists
         * @return true if generation started, false otherwise
         */
        bool GenerateTerrain(int x, int z, bool force = false);

        /**
         * @brief Generate terrain for chunk
         * @param chunkX Chunk X coordinate
         * @param chunkZ Chunk Z coordinate
         * @param force Force regeneration if already exists
         * @return true if generation started, false otherwise
         */
        bool GenerateChunk(int chunkX, int chunkZ, bool force = false);

        // Lighting system

        /**
         * @brief Update lighting for area
         * @param x Center X coordinate
         * @param z Center Z coordinate
         * @param radius Update radius in chunks
         */
        void UpdateLighting(int x, int z, int radius = 8);

        /**
         * @brief Get light level at position
         * @param x X coordinate
         * @param y Y coordinate
         * @param z Z coordinate
         * @return Light level (0-15)
         */
        uint8_t GetLightLevel(int x, int y, int z) const;

        /**
         * @brief Set light level at position
         * @param x X coordinate
         * @param y Y coordinate
         * @param z Z coordinate
         * @param level Light level (0-15)
         */
        void SetLightLevel(int x, int y, int z, uint8_t level);

        // Weather system

        /**
         * @brief Get current weather
         * @return Current weather type
         */
        std::string GetCurrentWeather() const;

        /**
         * @brief Set weather
         * @param weather Weather type
         * @param duration Duration in seconds (0 = permanent)
         */
        void SetWeather(const std::string& weather, double duration = 0.0);

        /**
         * @brief Get weather intensity
         * @return Weather intensity (0.0 - 1.0)
         */
        float GetWeatherIntensity() const;

        // World utilities

        /**
         * @brief Convert world coordinates to block coordinates
         * @param x World X coordinate
         * @param y World Y coordinate
         * @param z World Z coordinate
         * @return Block coordinate
         */
        BlockCoordinate WorldToBlock(int x, int y, int z) const {
            return BlockCoordinate(x, y, z);
        }

        /**
         * @brief Convert block coordinates to world coordinates
         * @param coord Block coordinate
         * @return World coordinates (x, y, z)
         */
        std::tuple<int, int, int> BlockToWorld(const BlockCoordinate& coord) const {
            return {coord.x, coord.y, coord.z};
        }

        /**
         * @brief Get height at position
         * @param x X coordinate
         * @param z Z coordinate
         * @return Height at position
         */
        int GetHeight(int x, int z) const;

        /**
         * @brief Get biome at position
         * @param x X coordinate
         * @param z Z coordinate
         * @return Biome name
         */
        std::string GetBiome(int x, int z) const;

        /**
         * @brief Get spawn position
         * @return Spawn position
         */
        BlockCoordinate GetSpawnPosition() const {
            return BlockCoordinate(m_info.spawnX, m_info.spawnY, m_info.spawnZ);
        }

        /**
         * @brief Set spawn position
         * @param x X coordinate
         * @param y Y coordinate
         * @param z Z coordinate
         */
        void SetSpawnPosition(int x, int y, int z);

        // World settings

        /**
         * @brief Get world setting
         * @param key Setting key
         * @param defaultValue Default value
         * @return Setting value
         */
        template<typename T>
        T GetSetting(const std::string& key, const T& defaultValue = T{}) const;

        /**
         * @brief Set world setting
         * @param key Setting key
         * @param value Setting value
         */
        template<typename T>
        void SetSetting(const std::string& key, const T& value);

        // Performance optimization

        /**
         * @brief Set view distance
         * @param distance View distance in chunks
         */
        void SetViewDistance(int distance);

        /**
         * @brief Get view distance
         * @return View distance in chunks
         */
        int GetViewDistance() const { return m_viewDistance; }

        /**
         * @brief Set LOD distance
         * @param distance LOD distance in chunks
         */
        void SetLODDistance(int distance);

        /**
         * @brief Get LOD distance
         * @return LOD distance in chunks
         */
        int GetLODDistance() const { return m_lodDistance; }

        /**
         * @brief Enable/disable vertical sync
         * @param enabled Enable state
         */
        void SetVSyncEnabled(bool enabled);

        /**
         * @brief Check if VSync is enabled
         * @return true if enabled, false otherwise
         */
        bool IsVSyncEnabled() const { return m_vsyncEnabled; }

    private:
        /**
         * @brief Initialize subsystems
         * @return true if successful, false otherwise
         */
        bool InitializeSubsystems();

        /**
         * @brief Update chunk loading/unloading
         * @param cameraPos Camera position
         */
        void UpdateChunkLoading(const glm::vec3& cameraPos);

        /**
         * @brief Process pending chunk operations
         */
        void ProcessPendingOperations();

        /**
         * @brief Update world simulation
         * @param deltaTime Time elapsed
         */
        void UpdateSimulation(double deltaTime);

        /**
         * @brief Update world metrics
         * @param deltaTime Time elapsed
         */
        void UpdateMetrics(double deltaTime);

        // World state
        WorldInfo m_info;                                    ///< World information
        WorldState m_state;                                  ///< Current world state
        WorldMetrics m_metrics;                              ///< Performance metrics

        // Core systems
        std::unique_ptr<ChunkManager> m_chunkManager;        ///< Chunk management system
        std::unique_ptr<TerrainGenerator> m_terrainGenerator; ///< Terrain generation system
        std::unique_ptr<LightingEngine> m_lightingEngine;    ///< Lighting system
        std::unique_ptr<WeatherSystem> m_weatherSystem;      ///< Weather system
        std::unique_ptr<WorldRenderer> m_renderer;           ///< World renderer

        // World data
        std::unordered_map<ChunkCoordinate, std::unique_ptr<Chunk>> m_chunks;
        std::vector<std::unique_ptr<Entity>> m_entities;     ///< World entities
        std::unordered_map<std::string, std::any> m_settings; ///< World settings

        // Performance settings
        int m_viewDistance;                                  ///< View distance in chunks
        int m_lodDistance;                                   ///< LOD distance in chunks
        bool m_vsyncEnabled;                                 ///< VSync enabled flag

        // Threading and synchronization
        mutable std::shared_mutex m_worldMutex;              ///< World access synchronization
        mutable std::shared_mutex m_chunkMutex;              ///< Chunk access synchronization
        mutable std::shared_mutex m_entityMutex;             ///< Entity access synchronization

        // Pending operations
        std::deque<std::function<void()>> m_pendingOperations; ///< Pending operations queue
        mutable std::mutex m_operationsMutex;                ///< Operations synchronization

        // Statistics
        std::chrono::steady_clock::time_point m_startTime;   ///< World start time
        double m_lastUpdateTime;                             ///< Last update time
        double m_lastSaveTime;                               ///< Last save time
        double m_lastMetricsUpdate;                          ///< Last metrics update
    };

    // Template implementations

    template<typename T>
    T World::GetSetting(const std::string& key, const T& defaultValue) const {
        std::shared_lock<std::shared_mutex> lock(m_worldMutex);
        auto it = m_settings.find(key);
        if (it != m_settings.end()) {
            try {
                return std::any_cast<T>(it->second);
            } catch (const std::bad_any_cast&) {
                return defaultValue;
            }
        }
        return defaultValue;
    }

    template<typename T>
    void World::SetSetting(const std::string& key, const T& value) {
        std::unique_lock<std::shared_mutex> lock(m_worldMutex);
        m_settings[key] = value;
    }

} // namespace VoxelCraft

#endif // VOXELCRAFT_WORLD_WORLD_HPP
