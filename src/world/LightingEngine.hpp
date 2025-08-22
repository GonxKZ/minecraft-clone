/**
 * @file LightingEngine.hpp
 * @brief VoxelCraft Advanced Lighting System
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines the lighting engine that handles dynamic lighting,
 * shadows, ambient occlusion, and atmospheric effects for the voxel world.
 */

#ifndef VOXELCRAFT_WORLD_LIGHTING_ENGINE_HPP
#define VOXELCRAFT_WORLD_LIGHTING_ENGINE_HPP

#include <memory>
#include <vector>
#include <unordered_map>
#include <queue>
#include <atomic>
#include <mutex>
#include <shared_mutex>
#include <deque>

#include "../core/Config.hpp"
#include "World.hpp"
#include "Chunk.hpp"

namespace VoxelCraft {

    /**
     * @enum LightType
     * @brief Type of light source
     */
    enum class LightType {
        Sky,                ///< Sky/sun light
        Block,              ///< Block light (torches, etc.)
        Entity,             ///< Entity light
        Ambient,            ///< Ambient light
        Dynamic             ///< Dynamic light source
    };

    /**
     * @enum LightChannel
     * @brief Lighting channels for different light types
     */
    enum class LightChannel {
        SkyLight,           ///< Sky light channel
        BlockLight,         ///< Block light channel
        Red,                ///< Red color channel
        Green,              ///< Green color channel
        Blue                ///< Blue color channel
    };

    /**
     * @struct LightNode
     * @brief Node in the lighting propagation graph
     */
    struct LightNode {
        int x, y, z;                    ///< World coordinates
        uint8_t lightLevel;             ///< Light level at this node
        LightChannel channel;            ///< Light channel
        uint32_t propagationStep;        ///< Propagation step counter
        bool isDirty;                   ///< Node needs update
    };

    /**
     * @struct LightSource
     * @brief Represents a light source in the world
     */
    struct LightSource {
        int x, y, z;                    ///< World coordinates
        uint8_t level;                  ///< Light level (0-15)
        LightType type;                 ///< Light source type
        LightChannel channel;            ///< Light channel
        bool isDynamic;                 ///< Is dynamic light source
        float radius;                   ///< Light radius
        glm::vec3 color;                ///< Light color (RGB)
        float intensity;                ///< Light intensity
        uint32_t sourceId;              ///< Unique source ID
        double lastUpdate;              ///< Last update timestamp
        bool needsUpdate;               ///< Needs lighting update
    };

    /**
     * @struct LightingConfig
     * @brief Configuration for the lighting engine
     */
    struct LightingConfig {
        // Performance settings
        int maxPropagationSteps;        ///< Maximum light propagation steps
        int propagationBatchSize;       ///< Batch size for propagation
        float updateInterval;           ///< Lighting update interval (seconds)
        bool enableMultithreading;      ///< Enable multithreaded lighting
        int workerThreads;              ///< Number of lighting worker threads

        // Quality settings
        bool enableSmoothLighting;      ///< Enable smooth lighting
        bool enableAmbientOcclusion;    ///< Enable ambient occlusion
        bool enableColorBleeding;       ///< Enable color bleeding
        bool enableShadows;             ///< Enable shadow mapping
        float shadowQuality;            ///< Shadow quality (0.0 - 1.0)

        // Sky light settings
        float dayNightCycleSpeed;       ///< Day/night cycle speed multiplier
        float minSkyLight;              ///< Minimum sky light level
        float maxSkyLight;              ///< Maximum sky light level
        glm::vec3 dawnColor;            ///< Dawn light color
        glm::vec3 dayColor;             ///< Day light color
        glm::vec3 duskColor;            ///< Dusk light color
        glm::vec3 nightColor;           ///< Night light color

        // Block light settings
        float blockLightFalloff;        ///< Block light falloff rate
        float maxBlockLightRadius;      ///< Maximum block light radius
        bool enableFlickeringLights;    ///< Enable flickering light effects

        // Atmospheric settings
        bool enableVolumetricLighting;  ///< Enable volumetric lighting
        bool enableGodRays;             ///< Enable god rays
        float fogDensity;               ///< Fog density
        float atmosphericDensity;       ///< Atmospheric scattering density
    };

    /**
     * @struct LightingMetrics
     * @brief Performance metrics for the lighting system
     */
    struct LightingMetrics {
        uint64_t totalLightUpdates;     ///< Total lighting updates performed
        uint64_t lightSources;          ///< Number of active light sources
        uint64_t propagationSteps;      ///< Total light propagation steps
        double averageUpdateTime;       ///< Average update time (ms)
        double maxUpdateTime;           ///< Maximum update time (ms)
        uint64_t cacheHits;             ///< Lighting cache hits
        uint64_t cacheMisses;           ///< Lighting cache misses
        double cacheHitRate;            ///< Cache hit rate (0.0 - 1.0)
        size_t memoryUsage;             ///< Memory usage (bytes)
        uint32_t queuedUpdates;         ///< Updates in queue
        uint32_t activeThreads;         ///< Active lighting threads
    };

    /**
     * @class LightPropagationQueue
     * @brief Queue for managing light propagation tasks
     */
    class LightPropagationQueue {
    public:
        /**
         * @brief Add light node to propagation queue
         * @param node Light node to add
         * @param priority Priority level
         */
        void AddNode(const LightNode& node, int priority = 0);

        /**
         * @brief Get next node to process
         * @return Next light node or empty optional
         */
        std::optional<LightNode> GetNextNode();

        /**
         * @brief Check if queue is empty
         * @return true if empty, false otherwise
         */
        bool IsEmpty() const;

        /**
         * @brief Get queue size
         * @return Number of nodes in queue
         */
        size_t Size() const;

        /**
         * @brief Clear the queue
         */
        void Clear();

        /**
         * @brief Remove duplicate nodes
         */
        void RemoveDuplicates();

    private:
        struct QueuedNode {
            LightNode node;
            int priority;
            double timestamp;
        };

        struct NodeCompare {
            bool operator()(const QueuedNode& a, const QueuedNode& b) const {
                return a.priority < b.priority;
            }
        };

        std::priority_queue<QueuedNode, std::vector<QueuedNode>, NodeCompare> m_queue;
        mutable std::mutex m_mutex;
    };

    /**
     * @class LightingEngine
     * @brief Advanced lighting engine for voxel worlds
     *
     * The LightingEngine handles:
     * - Sky light propagation and day/night cycle
     * - Block light from torches and other sources
     * - Dynamic light sources and shadows
     * - Ambient occlusion and smooth lighting
     * - Atmospheric effects and volumetric lighting
     * - Efficient light propagation algorithms
     * - Multi-threaded lighting calculations
     *
     * Features:
     * - Real-time light propagation
     * - Level of Detail for lighting
     * - Caching system for performance
     * - Support for colored lighting
     * - Atmospheric scattering
     * - Dynamic shadows
     */
    class LightingEngine {
    public:
        /**
         * @brief Constructor
         * @param config Lighting configuration
         */
        explicit LightingEngine(const LightingConfig& config);

        /**
         * @brief Destructor
         */
        ~LightingEngine();

        /**
         * @brief Deleted copy constructor
         */
        LightingEngine(const LightingEngine&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        LightingEngine& operator=(const LightingEngine&) = delete;

        // Engine lifecycle

        /**
         * @brief Initialize the lighting engine
         * @return true if successful, false otherwise
         */
        bool Initialize();

        /**
         * @brief Shutdown the lighting engine
         */
        void Shutdown();

        /**
         * @brief Update lighting system
         * @param deltaTime Time elapsed since last update
         */
        void Update(double deltaTime);

        // Light source management

        /**
         * @brief Add light source to the world
         * @param source Light source to add
         * @return Light source ID or empty optional on failure
         */
        std::optional<uint32_t> AddLightSource(const LightSource& source);

        /**
         * @brief Remove light source from the world
         * @param sourceId Light source ID
         * @return true if removed, false if not found
         */
        bool RemoveLightSource(uint32_t sourceId);

        /**
         * @brief Update existing light source
         * @param sourceId Light source ID
         * @param source Updated light source
         * @return true if updated, false if not found
         */
        bool UpdateLightSource(uint32_t sourceId, const LightSource& source);

        /**
         * @brief Get light source by ID
         * @param sourceId Light source ID
         * @return Light source or empty optional if not found
         */
        std::optional<LightSource> GetLightSource(uint32_t sourceId) const;

        /**
         * @brief Get all active light sources
         * @return Vector of light sources
         */
        std::vector<LightSource> GetActiveLightSources() const;

        // Lighting queries

        /**
         * @brief Get sky light level at world coordinates
         * @param x World X coordinate
         * @param y World Y coordinate
         * @param z World Z coordinate
         * @return Sky light level (0-15)
         */
        uint8_t GetSkyLight(int x, int y, int z) const;

        /**
         * @brief Get block light level at world coordinates
         * @param x World X coordinate
         * @param y World Y coordinate
         * @param z World Z coordinate
         * @return Block light level (0-15)
         */
        uint8_t GetBlockLight(int x, int y, int z) const;

        /**
         * @brief Get combined light level at world coordinates
         * @param x World X coordinate
         * @param y World Y coordinate
         * @param z World Z coordinate
         * @return Combined light level (0-15)
         */
        uint8_t GetLightLevel(int x, int y, int z) const;

        /**
         * @brief Get light color at world coordinates
         * @param x World X coordinate
         * @param y World Y coordinate
         * @param z World Z coordinate
         * @return Light color (RGB)
         */
        glm::vec3 GetLightColor(int x, int y, int z) const;

        /**
         * @brief Check if position is in shadow
         * @param x World X coordinate
         * @param y World Y coordinate
         * @param z World Z coordinate
         * @return true if in shadow, false otherwise
         */
        bool IsInShadow(int x, int y, int z) const;

        // Lighting updates

        /**
         * @brief Update lighting for area
         * @param centerX Center X coordinate
         * @param centerZ Center Z coordinate
         * @param radius Update radius in chunks
         * @param priority Update priority
         */
        void UpdateAreaLighting(int centerX, int centerZ, int radius, int priority = 0);

        /**
         * @brief Update lighting for chunk
         * @param chunk Chunk to update
         * @param priority Update priority
         */
        void UpdateChunkLighting(Chunk* chunk, int priority = 0);

        /**
         * @brief Update sky light for entire world
         * @param timeOfDay Time of day (0.0 - 1.0)
         */
        void UpdateSkyLight(float timeOfDay);

        /**
         * @brief Recalculate all lighting
         * @param priority Recalculation priority
         */
        void RecalculateAllLighting(int priority = 0);

        // Day/night cycle

        /**
         * @brief Get current time of day
         * @return Time of day (0.0 - 1.0, 0.0 = midnight, 0.5 = noon)
         */
        float GetTimeOfDay() const { return m_timeOfDay; }

        /**
         * @brief Set time of day
         * @param time Time of day (0.0 - 1.0)
         */
        void SetTimeOfDay(float time);

        /**
         * @brief Advance time of day
         * @param deltaTime Time to advance (seconds)
         */
        void AdvanceTime(float deltaTime);

        /**
         * @brief Get current sky light level
         * @return Sky light level (0.0 - 1.0)
         */
        float GetSkyLightLevel() const;

        /**
         * @brief Get current sun direction
         * @return Sun direction vector
         */
        glm::vec3 GetSunDirection() const;

        /**
         * @brief Get current moon direction
         * @return Moon direction vector
         */
        glm::vec3 GetMoonDirection() const;

        // Atmospheric effects

        /**
         * @brief Get atmospheric scattering color
         * @param direction View direction
         * @return Scattering color
         */
        glm::vec3 GetAtmosphericScattering(const glm::vec3& direction) const;

        /**
         * @brief Get fog color at position
         * @param x World X coordinate
         * @param y World Y coordinate
         * @param z World Z coordinate
         * @return Fog color
         */
        glm::vec3 GetFogColor(int x, int y, int z) const;

        /**
         * @brief Get fog density at position
         * @param x World X coordinate
         * @param y World Y coordinate
         * @param z World Z coordinate
         * @return Fog density
         */
        float GetFogDensity(int x, int y, int z) const;

        // Configuration

        /**
         * @brief Get lighting configuration
         * @return Current configuration
         */
        const LightingConfig& GetConfig() const { return m_config; }

        /**
         * @brief Set lighting configuration
         * @param config New configuration
         */
        void SetConfig(const LightingConfig& config);

        /**
         * @brief Get lighting metrics
         * @return Performance metrics
         */
        const LightingMetrics& GetMetrics() const { return m_metrics; }

        /**
         * @brief Reset metrics
         */
        void ResetMetrics();

    private:
        /**
         * @brief Initialize lighting data structures
         */
        void InitializeLightingData();

        /**
         * @brief Initialize worker threads
         * @return true if successful, false otherwise
         */
        bool InitializeWorkerThreads();

        /**
         * @brief Worker thread function for lighting calculations
         * @param threadId Worker thread ID
         */
        void LightingWorkerThread(int threadId);

        /**
         * @brief Process light propagation
         * @param startNode Starting light node
         * @param channel Light channel to propagate
         */
        void PropagateLight(const LightNode& startNode, LightChannel channel);

        /**
         * @brief Calculate ambient occlusion at position
         * @param x World X coordinate
         * @param y World Y coordinate
         * @param z World Z coordinate
         * @return Ambient occlusion factor (0.0 - 1.0)
         */
        float CalculateAmbientOcclusion(int x, int y, int z) const;

        /**
         * @brief Calculate smooth lighting at position
         * @param x World X coordinate
         * @param y World Y coordinate
         * @param z World Z coordinate
         * @return Smooth lighting value
         */
        float CalculateSmoothLighting(int x, int y, int z) const;

        /**
         * @brief Update light source positions
         */
        void UpdateLightSources();

        /**
         * @brief Process pending lighting updates
         */
        void ProcessLightingUpdates();

        /**
         * @brief Update lighting metrics
         * @param deltaTime Time elapsed
         */
        void UpdateMetrics(double deltaTime);

        /**
         * @brief Get or create light source ID
         * @param source Light source
         * @return Light source ID
         */
        uint32_t GetOrCreateLightSourceId(const LightSource& source);

        // Configuration and state
        LightingConfig m_config;                      ///< Lighting configuration
        LightingMetrics m_metrics;                    ///< Performance metrics

        // World reference
        World* m_world;                               ///< Parent world

        // Light sources
        std::unordered_map<uint32_t, LightSource> m_lightSources; ///< Active light sources
        mutable std::shared_mutex m_lightSourceMutex; ///< Light source synchronization
        std::atomic<uint32_t> m_nextLightSourceId;   ///< Next light source ID

        // Lighting propagation
        LightPropagationQueue m_propagationQueue;    ///< Light propagation queue
        std::vector<std::thread> m_workerThreads;   ///< Worker threads
        std::atomic<bool> m_workersRunning;          ///< Worker threads running flag
        std::condition_variable_any m_workerCondition; ///< Worker condition variable

        // Day/night cycle
        float m_timeOfDay;                           ///< Current time of day (0.0 - 1.0)
        float m_cycleSpeed;                          ///< Day/night cycle speed
        glm::vec3 m_currentSkyColor;                 ///< Current sky color
        float m_currentSkyLight;                     ///< Current sky light level

        // Caching system
        std::unordered_map<uint64_t, uint8_t> m_lightCache; ///< Lighting cache
        mutable std::shared_mutex m_cacheMutex;     ///< Cache synchronization
        size_t m_maxCacheSize;                      ///< Maximum cache size

        // Pending updates
        std::deque<std::function<void()>> m_pendingUpdates; ///< Pending lighting updates
        mutable std::mutex m_updateMutex;           ///< Update synchronization

        // Statistics
        double m_lastUpdateTime;                    ///< Last update timestamp
        uint64_t m_totalPropagationSteps;           ///< Total propagation steps
        uint32_t m_activeUpdates;                   ///< Active lighting updates
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_WORLD_LIGHTING_ENGINE_HPP
