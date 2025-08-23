/**
 * @file BiomeManager.hpp
 * @brief VoxelCraft Biome Manager - Manages all biomes and terrain generation
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_BIOME_BIOME_MANAGER_HPP
#define VOXELCRAFT_BIOME_BIOME_MANAGER_HPP

#include <memory>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <array>
#include <random>
#include <chrono>
#include <glm/glm.hpp>

#include "Biome.hpp"
#include "../world/World.hpp"

namespace VoxelCraft {

    class Player;
    class MobManager;

    /**
     * @enum BiomeGenerationMode
     * @brief Different modes for biome generation
     */
    enum class BiomeGenerationMode {
        VANILLA = 0,           ///< Standard Minecraft-style generation
        REALISTIC,             ///< More realistic biome distribution
        CUSTOM,                ///< Custom biome rules
        CHAOTIC,               ///< Random biome distribution
        ISLAND,                ///< Island-style generation
        CONTINENTAL            ///< Large continental biomes
    };

    /**
     * @enum BiomeTransitionType
     * @brief Types of biome transitions
     */
    enum class BiomeTransitionType {
        HARD = 0,              ///< Sharp transition between biomes
        SOFT,                  ///< Gradual transition
        BLENDED,               ///< Mixed features from both biomes
        RIVER,                 ///< River-based transition
        BEACH,                 ///< Beach-based transition
        MOUNTAIN,              ///< Mountain-based transition
        NONE                   ///< No transition
    };

    /**
     * @struct BiomeTransition
     * @brief Definition of a biome transition
     */
    struct BiomeTransition {
        BiomeType fromBiome;
        BiomeType toBiome;
        BiomeTransitionType type;
        float transitionWidth;         ///< Width of transition zone in blocks
        std::function<float(float)> blendFunction; ///< How to blend biomes
        std::vector<BiomeType> intermediateBiomes; ///< Biomes in between
    };

    /**
     * @struct BiomeRegion
     * @brief A region of connected biomes
     */
    struct BiomeRegion {
        BiomeType primaryBiome;
        std::vector<BiomeType> subBiomes;
        glm::vec3 center;              ///< Center of the region
        float radius;                  ///< Approximate radius of region
        float cohesion;                ///< How similar biomes in region are
        std::unordered_map<BiomeType, float> biomeWeights;
        uint32_t seed;                 ///< Region-specific seed
    };

    /**
     * @struct BiomeNoiseLayer
     * @brief A noise layer for biome generation
     */
    struct BiomeNoiseLayer {
        std::string name;
        float frequency;               ///< Noise frequency
        float amplitude;               ///< Noise amplitude
        float lacunarity;              ///< Noise lacunarity
        float persistence;             ///< Noise persistence
        int octaves;                   ///< Number of octaves
        uint32_t seed;                 ///< Layer-specific seed
        std::function<float(float, float)> modifier; ///< Optional modifier function
    };

    /**
     * @struct BiomeGenerationConfig
     * @brief Configuration for biome generation
     */
    struct BiomeGenerationConfig {
        BiomeGenerationMode mode;
        int biomeSize;                 ///< Size of biomes in chunks
        int regionSize;                ///< Size of biome regions
        float temperatureNoiseScale;   ///< Scale for temperature noise
        float humidityNoiseScale;      ///< Scale for humidity noise
        float continentalnessScale;    ///< Scale for continentalness noise
        float erosionScale;            ///< Scale for erosion noise
        float weirdnessScale;          ///< Scale for weirdness noise
        bool enableTransitions;        ///< Whether to enable biome transitions
        bool enableRivers;             ///< Whether to generate rivers
        bool enableBeaches;            ///< Whether to generate beaches
        bool enableOceans;             ///< Whether to generate oceans
        int seaLevel;                  ///< Sea level height
        float riverThreshold;          ///< Threshold for river generation
        float beachThreshold;          ///< Threshold for beach generation
        int minOceanSize;              ///< Minimum ocean size
        int maxOceanSize;              ///< Maximum ocean size
        std::vector<BiomeNoiseLayer> noiseLayers;
    };

    /**
     * @struct BiomeCache
     * @brief Cached biome data for performance
     */
    struct BiomeCache {
        std::unordered_map<glm::ivec2, BiomeType> biomeMap;
        std::unordered_map<glm::ivec2, int> heightMap;
        std::unordered_map<glm::ivec2, float> temperatureMap;
        std::unordered_map<glm::ivec2, float> humidityMap;
        std::chrono::steady_clock::time_point lastUpdate;
        int cacheRadius;               ///< Radius of cached data in chunks

        /**
         * @brief Check if position is in cache
         * @param position Chunk position
         * @return true if cached
         */
        bool IsCached(const glm::ivec2& position) const {
            return biomeMap.find(position) != biomeMap.end();
        }

        /**
         * @brief Clear cache
         */
        void Clear() {
            biomeMap.clear();
            heightMap.clear();
            temperatureMap.clear();
            humidityMap.clear();
        }
    };

    /**
     * @class BiomeManager
     * @brief Central manager for all biome-related functionality
     */
    class BiomeManager {
    public:
        /**
         * @brief Get singleton instance
         * @return BiomeManager instance
         */
        static BiomeManager& GetInstance();

        /**
         * @brief Initialize biome manager
         * @param world World reference
         * @return true if successful
         */
        bool Initialize(World* world);

        /**
         * @brief Shutdown biome manager
         */
        void Shutdown();

        /**
         * @brief Update biome system
         * @param deltaTime Time since last update
         */
        void Update(float deltaTime);

        /**
         * @brief Get biome at position
         * @param position World position
         * @return Biome instance or nullptr
         */
        std::shared_ptr<Biome> GetBiomeAt(const glm::vec3& position) const;

        /**
         * @brief Get biome type at position
         * @param position World position
         * @return Biome type
         */
        BiomeType GetBiomeTypeAt(const glm::vec3& position) const;

        /**
         * @brief Get all biomes
         * @return Vector of all biomes
         */
        std::vector<std::shared_ptr<Biome>> GetAllBiomes() const;

        /**
         * @brief Get biome by type
         * @param type Biome type
         * @return Biome instance or nullptr
         */
        std::shared_ptr<Biome> GetBiome(BiomeType type) const;

        /**
         * @brief Generate terrain at position
         * @param position World position
         * @return Generated height
         */
        int GenerateTerrain(const glm::vec3& position);

        /**
         * @brief Generate biomes for chunk
         * @param chunkX Chunk X coordinate
         * @param chunkZ Chunk Z coordinate
         * @return Map of positions to biome types
         */
        std::unordered_map<glm::ivec2, BiomeType> GenerateChunkBiomes(int chunkX, int chunkZ);

        /**
         * @brief Generate features for biome
         * @param position World position
         * @param biome Biome to generate for
         */
        void GenerateBiomeFeatures(const glm::vec3& position, std::shared_ptr<Biome> biome);

        /**
         * @brief Generate structures in biomes
         * @param position World position
         * @param biome Biome to generate for
         */
        void GenerateBiomeStructures(const glm::vec3& position, std::shared_ptr<Biome> biome);

        /**
         * @brief Get temperature at position
         * @param position World position
         * @return Temperature (0-1)
         */
        float GetTemperatureAt(const glm::vec3& position) const;

        /**
         * @brief Get humidity at position
         * @param position World position
         * @return Humidity (0-1)
         */
        float GetHumidityAt(const glm::vec3& position) const;

        /**
         * @brief Get biome colors at position
         * @param position World position
         * @return Biome colors
         */
        BiomeColors GetBiomeColorsAt(const glm::vec3& position) const;

        /**
         * @brief Check if position is in biome transition
         * @param position World position
         * @return Transition information
         */
        BiomeTransitionType GetBiomeTransitionAt(const glm::vec3& position) const;

        /**
         * @brief Get spawnable mobs for biome at position
         * @param position World position
         * @param timeOfDay Time of day (0-1)
         * @return Vector of spawnable mob types
         */
        std::vector<MobType> GetSpawnableMobsAt(const glm::vec3& position, float timeOfDay) const;

        /**
         * @brief Set generation mode
         * @param mode New generation mode
         */
        void SetGenerationMode(BiomeGenerationMode mode);

        /**
         * @brief Get current generation mode
         * @return Current generation mode
         */
        BiomeGenerationMode GetGenerationMode() const { return m_config.mode; }

        /**
         * @brief Set biome size
         * @param size Biome size in chunks
         */
        void SetBiomeSize(int size) { m_config.biomeSize = size; }

        /**
         * @brief Get biome size
         * @return Biome size in chunks
         */
        int GetBiomeSize() const { return m_config.biomeSize; }

        /**
         * @brief Add custom biome
         * @param biome Biome to add
         * @return true if added successfully
         */
        bool AddCustomBiome(std::shared_ptr<Biome> biome);

        /**
         * @brief Remove custom biome
         * @param type Biome type to remove
         * @return true if removed successfully
         */
        bool RemoveCustomBiome(BiomeType type);

        /**
         * @brief Add biome transition
         * @param transition Transition to add
         * @return true if added successfully
         */
        bool AddBiomeTransition(const BiomeTransition& transition);

        /**
         * @brief Clear biome cache
         */
        void ClearBiomeCache();

        /**
         * @brief Get biome cache statistics
         * @return Cache statistics
         */
        const BiomeCache& GetBiomeCache() const { return m_cache; }

        /**
         * @brief Get biome statistics
         * @return Biome statistics
         */
        const BiomeStats& GetStats() const { return m_stats; }

        /**
         * @brief Enable/disable biome transitions
         * @param enabled Whether transitions are enabled
         */
        void SetTransitionsEnabled(bool enabled) { m_config.enableTransitions = enabled; }

        /**
         * @brief Check if transitions are enabled
         * @return true if enabled
         */
        bool AreTransitionsEnabled() const { return m_config.enableTransitions; }

    private:
        BiomeManager() = default;
        ~BiomeManager() = default;

        // Prevent copying
        BiomeManager(const BiomeManager&) = delete;
        BiomeManager& operator=(const BiomeManager&) = delete;

        World* m_world;
        std::unordered_map<BiomeType, std::shared_ptr<Biome>> m_biomes;
        std::vector<BiomeTransition> m_transitions;
        std::vector<BiomeRegion> m_regions;
        std::mt19937 m_randomEngine;
        BiomeGenerationConfig m_config;
        BiomeCache m_cache;
        BiomeStats m_stats;
        bool m_initialized;

        // Noise generation
        std::vector<BiomeNoiseLayer> m_noiseLayers;

        // Biome generation
        void InitializeDefaultBiomes();
        void InitializeDefaultTransitions();
        void InitializeNoiseLayers();
        void GenerateBiomeRegions();

        /**
         * @brief Generate biome at position using noise
         * @param position World position
         * @return Generated biome type
         */
        BiomeType GenerateBiomeAtPosition(const glm::vec3& position) const;

        /**
         * @brief Calculate biome weights at position
         * @param position World position
         * @return Map of biome types to weights
         */
        std::unordered_map<BiomeType, float> CalculateBiomeWeights(const glm::vec3& position) const;

        /**
         * @brief Select biome based on weights
         * @param weights Biome weights
         * @return Selected biome type
         */
        BiomeType SelectBiomeFromWeights(const std::unordered_map<BiomeType, float>& weights) const;

        /**
         * @brief Apply biome transitions
         * @param position World position
         * @param biome Current biome
         * @return Transitioned biome
         */
        BiomeType ApplyBiomeTransitions(const glm::vec3& position, BiomeType biome) const;

        /**
         * @brief Generate noise value
         * @param layer Noise layer
         * @param x X coordinate
         * @param z Z coordinate
         * @return Noise value
         */
        float GenerateNoise(const BiomeNoiseLayer& layer, float x, float z) const;

        /**
         * @brief Generate continentalness noise
         * @param x X coordinate
         * @param z Z coordinate
         * @return Continentalness value
         */
        float GenerateContinentalness(float x, float z) const;

        /**
         * @brief Generate erosion noise
         * @param x X coordinate
         * @param z Z coordinate
         * @return Erosion value
         */
        float GenerateErosion(float x, float z) const;

        /**
         * @brief Generate weirdness noise
         * @param x X coordinate
         * @param z Z coordinate
         * @return Weirdness value
         */
        float GenerateWeirdness(float x, float z) const;

        /**
         * @brief Check if position should be ocean
         * @param position World position
         * @return true if should be ocean
         */
        bool ShouldBeOcean(const glm::vec3& position) const;

        /**
         * @brief Check if position should be river
         * @param position World position
         * @return true if should be river
         */
        bool ShouldBeRiver(const glm::vec3& position) const;

        /**
         * @brief Check if position should be beach
         * @param position World position
         * @param biome Current biome
         * @return true if should be beach
         */
        bool ShouldBeBeach(const glm::vec3& position, BiomeType biome) const;

        /**
         * @brief Cache biome data for position
         * @param position Chunk position
         * @param biome Biome type
         * @param height Generated height
         */
        void CacheBiomeData(const glm::ivec2& position, BiomeType biome, int height);

        /**
         * @brief Update biome cache around players
         */
        void UpdateBiomeCache();

        /**
         * @brief Generate smooth biome transition
         * @param fromBiome Starting biome
         * @param toBiome Ending biome
         * @param distance Distance along transition
         * @param maxDistance Maximum transition distance
         * @return Interpolated biome type
         */
        BiomeType GenerateSmoothTransition(BiomeType fromBiome, BiomeType toBiome,
                                         float distance, float maxDistance) const;
    };

    /**
     * @struct BiomeStats
     * @brief Statistics for biome system
     */
    struct BiomeStats {
        int totalBiomes = 0;
        int cachedChunks = 0;
        int generatedChunks = 0;
        float averageGenerationTime = 0.0f;
        std::unordered_map<BiomeType, int> biomeDistribution;
        std::unordered_map<BiomeTransitionType, int> transitionUsage;
        std::vector<std::pair<BiomeType, int>> mostCommonBiomes;
        int oceanChunks = 0;
        int riverChunks = 0;
        int beachChunks = 0;
        int mountainChunks = 0;
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_BIOME_BIOME_MANAGER_HPP
