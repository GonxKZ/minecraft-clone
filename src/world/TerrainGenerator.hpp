/**
 * @file TerrainGenerator.hpp
 * @brief VoxelCraft Advanced Terrain Generation System
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines the TerrainGenerator class that handles procedural
 * terrain generation using advanced noise functions, biomes, and features.
 */

#ifndef VOXELCRAFT_WORLD_TERRAIN_GENERATOR_HPP
#define VOXELCRAFT_WORLD_TERRAIN_GENERATOR_HPP

#include <memory>
#include <vector>
#include <unordered_map>
#include <functional>
#include <random>
#include <mutex>
#include <shared_mutex>

#include "../core/Config.hpp"
#include "World.hpp"
#include "Chunk.hpp"
#include "Biome.hpp"
#include "NoiseGenerator.hpp"

namespace VoxelCraft {

    // Forward declarations
    class World;
    class Chunk;
    struct Biome;

    /**
     * @enum GenerationPhase
     * @brief Phases of terrain generation
     */
    enum class GenerationPhase {
        BaseTerrain,     ///< Generate base terrain height
        Biomes,          ///< Apply biome variations
        Caves,           ///< Generate cave systems
        Features,        ///< Generate surface features
        Vegetation,      ///< Generate vegetation
        Structures,      ///< Generate structures
        Lighting,        ///< Calculate lighting
        Finalize         ///< Final cleanup and optimization
    };

    /**
     * @struct GenerationConfig
     * @brief Configuration for terrain generation
     */
    struct GenerationConfig {
        // Basic settings
        int seed;                           ///< World seed
        WorldType worldType;                ///< Type of world to generate
        int maxHeight;                      ///< Maximum world height
        int minHeight;                      ///< Minimum world height
        int seaLevel;                       ///< Sea level

        // Noise settings
        float baseFrequency;                ///< Base noise frequency
        float baseAmplitude;                ///< Base noise amplitude
        int octaves;                        ///< Number of noise octaves
        float persistence;                  ///< Noise persistence
        float lacunarity;                   ///< Noise lacunarity

        // Biome settings
        float biomeScale;                   ///< Biome scale factor
        float temperatureScale;             ///< Temperature variation scale
        float humidityScale;                ///< Humidity variation scale

        // Cave settings
        float caveDensity;                  ///< Cave generation density
        float caveSize;                     ///< Average cave size
        int minCaveHeight;                  ///< Minimum cave height
        int maxCaveHeight;                  ///< Maximum cave height

        // Feature settings
        float treeDensity;                  ///< Tree generation density
        float flowerDensity;                ///< Flower generation density
        float oreDensity;                   ///< Ore generation density

        // Structure settings
        float structureDensity;             ///< Structure generation density
        int minStructureSpacing;            ///< Minimum spacing between structures

        // Performance settings
        bool useMultithreading;             ///< Enable multithreaded generation
        int generationBatchSize;            ///< Number of chunks to generate per batch
        bool cacheGeneration;               ///< Cache generated chunks
        size_t maxCacheSize;                ///< Maximum cache size
    };

    /**
     * @struct GenerationMetrics
     * @brief Performance metrics for terrain generation
     */
    struct GenerationMetrics {
        double totalGenerationTime;         ///< Total time spent generating
        double averageChunkTime;            ///< Average time per chunk
        double maxChunkTime;                ///< Maximum time for single chunk
        double minChunkTime;                ///< Minimum time for single chunk
        uint64_t chunksGenerated;           ///< Total chunks generated
        uint64_t cacheHits;                 ///< Number of cache hits
        uint64_t cacheMisses;               ///< Number of cache misses
        double cacheHitRate;                ///< Cache hit rate (0.0 - 1.0)
        std::unordered_map<GenerationPhase, double> phaseTimes; ///< Time per phase
    };

    /**
     * @class TerrainGenerator
     * @brief Advanced procedural terrain generator
     *
     * The TerrainGenerator creates diverse and interesting terrain using:
     * - Multiple noise functions (Perlin, Simplex, Worley, Value)
     * - Biome system with temperature and humidity
     * - Cave and tunnel generation
     * - Surface features (trees, flowers, rocks)
     * - Ore generation with realistic distribution
     * - Structure placement (villages, dungeons, etc.)
     * - Multi-threaded generation for performance
     * - Caching system for frequently accessed chunks
     */
    class TerrainGenerator {
    public:
        /**
         * @brief Constructor
         * @param config Generation configuration
         */
        explicit TerrainGenerator(const GenerationConfig& config);

        /**
         * @brief Destructor
         */
        ~TerrainGenerator();

        /**
         * @brief Deleted copy constructor
         */
        TerrainGenerator(const TerrainGenerator&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        TerrainGenerator& operator=(const TerrainGenerator&) = delete;

        // Generation interface

        /**
         * @brief Generate terrain for a chunk
         * @param chunk Chunk to generate
         * @return true if successful, false otherwise
         */
        bool GenerateChunk(Chunk* chunk);

        /**
         * @brief Generate terrain for chunk coordinates
         * @param chunkX Chunk X coordinate
         * @param chunkZ Chunk Z coordinate
         * @param world World instance
         * @return Generated chunk or nullptr on failure
         */
        std::unique_ptr<Chunk> GenerateChunk(int chunkX, int chunkZ, World* world);

        /**
         * @brief Generate multiple chunks
         * @param chunks List of chunks to generate
         * @return Number of successfully generated chunks
         */
        size_t GenerateChunks(std::vector<Chunk*>& chunks);

        /**
         * @brief Pre-generate chunks in area
         * @param centerX Center X coordinate
         * @param centerZ Center Z coordinate
         * @param radius Generation radius in chunks
         * @param world World instance
         * @return Number of chunks queued for generation
         */
        size_t PreGenerateArea(int centerX, int centerZ, int radius, World* world);

        // Height and biome queries

        /**
         * @brief Get height at world coordinates
         * @param worldX World X coordinate
         * @param worldZ World Z coordinate
         * @return Height at coordinates
         */
        int GetHeight(int worldX, int worldZ);

        /**
         * @brief Get biome at world coordinates
         * @param worldX World X coordinate
         * @param worldZ World Z coordinate
         * @return Biome information
         */
        Biome GetBiome(int worldX, int worldZ);

        /**
         * @brief Get temperature at world coordinates
         * @param worldX World X coordinate
         * @param worldZ World Z coordinate
         * @return Temperature (0.0 - 1.0)
         */
        float GetTemperature(int worldX, int worldZ);

        /**
         * @brief Get humidity at world coordinates
         * @param worldX World X coordinate
         * @param worldZ World Z coordinate
         * @return Humidity (0.0 - 1.0)
         */
        float GetHumidity(int worldX, int worldZ);

        // Configuration

        /**
         * @brief Get generation configuration
         * @return Current configuration
         */
        const GenerationConfig& GetConfig() const { return m_config; }

        /**
         * @brief Set generation configuration
         * @param config New configuration
         */
        void SetConfig(const GenerationConfig& config);

        /**
         * @brief Get generation metrics
         * @return Performance metrics
         */
        const GenerationMetrics& GetMetrics() const { return m_metrics; }

        /**
         * @brief Reset metrics
         */
        void ResetMetrics();

        // Caching system

        /**
         * @brief Enable/disable caching
         * @param enabled Enable state
         */
        void SetCachingEnabled(bool enabled);

        /**
         * @brief Check if caching is enabled
         * @return true if enabled, false otherwise
         */
        bool IsCachingEnabled() const { return m_cachingEnabled; }

        /**
         * @brief Clear generation cache
         */
        void ClearCache();

        /**
         * @brief Get cache size
         * @return Number of cached chunks
         */
        size_t GetCacheSize() const;

        /**
         * @brief Get cache hit rate
         * @return Cache hit rate (0.0 - 1.0)
         */
        double GetCacheHitRate() const;

    private:
        /**
         * @brief Initialize generation systems
         */
        void Initialize();

        /**
         * @brief Generate base terrain height
         * @param chunk Chunk to generate
         */
        void GenerateBaseTerrain(Chunk* chunk);

        /**
         * @brief Apply biome variations
         * @param chunk Chunk to modify
         */
        void ApplyBiomeVariations(Chunk* chunk);

        /**
         * @brief Generate cave systems
         * @param chunk Chunk to modify
         */
        void GenerateCaves(Chunk* chunk);

        /**
         * @brief Generate surface features
         * @param chunk Chunk to modify
         */
        void GenerateFeatures(Chunk* chunk);

        /**
         * @brief Generate vegetation
         * @param chunk Chunk to modify
         */
        void GenerateVegetation(Chunk* chunk);

        /**
         * @brief Generate structures
         * @param chunk Chunk to modify
         */
        void GenerateStructures(Chunk* chunk);

        /**
         * @brief Calculate lighting
         * @param chunk Chunk to light
         */
        void CalculateLighting(Chunk* chunk);

        /**
         * @brief Finalize chunk generation
         * @param chunk Chunk to finalize
         */
        void FinalizeChunk(Chunk* chunk);

        /**
         * @brief Generate terrain using noise functions
         * @param worldX World X coordinate
         * @param worldZ World Z coordinate
         * @return Generated height
         */
        int GenerateTerrainHeight(int worldX, int worldZ);

        /**
         * @brief Generate biome at coordinates
         * @param worldX World X coordinate
         * @param worldZ World Z coordinate
         * @return Biome type
         */
        Biome GenerateBiome(int worldX, int worldZ);

        /**
         * @brief Generate cave at coordinates
         * @param worldX World X coordinate
         * @param worldY World Y coordinate
         * @param worldZ World Z coordinate
         * @return true if cave should be generated, false otherwise
         */
        bool GenerateCave(int worldX, int worldY, int worldZ);

        /**
         * @brief Generate ore deposit
         * @param worldX World X coordinate
         * @param worldY World Y coordinate
         * @param worldZ World Z coordinate
         * @param oreType Type of ore to generate
         * @return true if ore generated, false otherwise
         */
        bool GenerateOre(int worldX, int worldY, int worldZ, uint32_t oreType);

        /**
         * @brief Generate tree
         * @param worldX World X coordinate
         * @param worldY World Y coordinate
         * @param worldZ World Z coordinate
         * @param treeType Type of tree to generate
         * @return true if tree generated, false otherwise
         */
        bool GenerateTree(int worldX, int worldY, int worldZ, uint32_t treeType);

        /**
         * @brief Check if position is suitable for feature generation
         * @param worldX World X coordinate
         * @param worldY World Y coordinate
         * @param worldZ World Z coordinate
         * @param featureType Type of feature
         * @return true if suitable, false otherwise
         */
        bool IsSuitableForFeature(int worldX, int worldY, int worldZ, uint32_t featureType);

        /**
         * @brief Update generation metrics
         * @param chunk Chunk that was generated
         * @param generationTime Time taken to generate
         */
        void UpdateMetrics(Chunk* chunk, double generationTime);

        /**
         * @brief Add chunk to cache
         * @param chunk Chunk to cache
         */
        void AddToCache(std::unique_ptr<Chunk> chunk);

        /**
         * @brief Get chunk from cache
         * @param chunkX Chunk X coordinate
         * @param chunkZ Chunk Z coordinate
         * @return Cached chunk or nullptr
         */
        std::unique_ptr<Chunk> GetFromCache(int chunkX, int chunkZ);

        // Generation configuration
        GenerationConfig m_config;

        // Noise generators
        std::unique_ptr<NoiseGenerator> m_baseNoise;      ///< Base terrain noise
        std::unique_ptr<NoiseGenerator> m_detailNoise;    ///< Detail noise
        std::unique_ptr<NoiseGenerator> m_caveNoise;      ///< Cave generation noise
        std::unique_ptr<NoiseGenerator> m_biomeNoise;     ///< Biome variation noise
        std::unique_ptr<NoiseGenerator> m_temperatureNoise; ///< Temperature noise
        std::unique_ptr<NoiseGenerator> m_humidityNoise;  ///< Humidity noise

        // Random number generation
        std::mt19937 m_randomEngine;                      ///< Random number engine
        std::uniform_real_distribution<float> m_randomFloat; ///< Float distribution

        // Biome system
        std::vector<Biome> m_biomes;                      ///< Available biomes
        std::unordered_map<std::string, Biome> m_biomeMap; ///< Biome lookup map

        // Generation metrics
        GenerationMetrics m_metrics;
        mutable std::shared_mutex m_metricsMutex;         ///< Metrics synchronization

        // Caching system
        bool m_cachingEnabled;                            ///< Caching enabled flag
        std::unordered_map<ChunkCoordinate, std::unique_ptr<Chunk>> m_cache; ///< Chunk cache
        mutable std::shared_mutex m_cacheMutex;           ///< Cache synchronization
        size_t m_maxCacheSize;                            ///< Maximum cache size

        // World reference (for biome and feature queries)
        World* m_world;                                   ///< Parent world

        // Generation phases
        std::vector<std::function<void(Chunk*)>> m_generationPhases; ///< Generation phases
    };

    /**
     * @class ChunkGenerationTask
     * @brief Task for generating a single chunk
     */
    class ChunkGenerationTask {
    public:
        /**
         * @brief Constructor
         * @param generator Terrain generator
         * @param chunkX Chunk X coordinate
         * @param chunkZ Chunk Z coordinate
         * @param world World instance
         */
        ChunkGenerationTask(TerrainGenerator* generator, int chunkX, int chunkZ, World* world);

        /**
         * @brief Execute the generation task
         * @return Generated chunk
         */
        std::unique_ptr<Chunk> Execute();

        /**
         * @brief Get task priority
         * @return Priority value
         */
        int GetPriority() const { return m_priority; }

        /**
         * @brief Get chunk coordinates
         * @return Chunk coordinate
         */
        ChunkCoordinate GetCoordinate() const { return m_coordinate; }

    private:
        TerrainGenerator* m_generator;                    ///< Terrain generator
        ChunkCoordinate m_coordinate;                     ///< Chunk coordinates
        World* m_world;                                   ///< World instance
        int m_priority;                                   ///< Task priority
    };

    /**
     * @class TerrainGeneratorFactory
     * @brief Factory for creating terrain generators
     */
    class TerrainGeneratorFactory {
    public:
        /**
         * @brief Create terrain generator for world type
         * @param worldType Type of world
         * @param seed World seed
         * @return Terrain generator instance
         */
        static std::unique_ptr<TerrainGenerator> CreateGenerator(WorldType worldType, int seed);

        /**
         * @brief Create custom terrain generator
         * @param config Generation configuration
         * @return Terrain generator instance
         */
        static std::unique_ptr<TerrainGenerator> CreateCustomGenerator(const GenerationConfig& config);

        /**
         * @brief Get default configuration for world type
         * @param worldType Type of world
         * @return Default configuration
         */
        static GenerationConfig GetDefaultConfig(WorldType worldType);
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_WORLD_TERRAIN_GENERATOR_HPP
