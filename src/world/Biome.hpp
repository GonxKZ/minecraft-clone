/**
 * @file Biome.hpp
 * @brief VoxelCraft World System - Biome System (Minecraft-like)
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * Comprehensive biome system with Overworld, Nether, and End biomes
 */

#ifndef VOXELCRAFT_WORLD_BIOME_HPP
#define VOXELCRAFT_WORLD_BIOME_HPP

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <functional>

#include "../blocks/Block.hpp"

namespace VoxelCraft {

    /**
     * @enum Dimension
     * @brief Game dimensions (Minecraft-like)
     */
    enum class Dimension {
        OVERWORLD,  // Normal world
        NETHER,     // Hell dimension
        END         // End dimension
    };

    /**
     * @enum Climate
     * @brief Climate types for biomes
     */
    enum class Climate {
        COLD,       // Tundra, Ice Plains
        COOL,       // Taiga, Mountains
        TEMPERATE,  // Forest, Plains, Hills
        WARM,       // Desert, Savanna
        HOT         // Jungle, Mesa
    };

    /**
     * @enum BiomeType
     * @brief All biome types in VoxelCraft
     */
    enum class BiomeType {
        // Overworld Biomes
        PLAINS,
        FOREST,
        DARK_FOREST,
        BIRCH_FOREST,
        JUNGLE,
        TAIGA,
        SNOWY_TAIGA,
        MOUNTAINS,
        SNOWY_MOUNTAINS,
        DESERT,
        SAVANNA,
        MESA,
        BADLANDS,
        ERODED_BADLANDS,
        WOODED_BADLANDS,
        SWAMP,
        BEACH,
        SNOWY_BEACH,
        STONE_SHORE,
        RIVER,
        FROZEN_RIVER,
        OCEAN,
        DEEP_OCEAN,
        WARM_OCEAN,
        LUKEWARM_OCEAN,
        COLD_OCEAN,
        DEEP_COLD_OCEAN,
        FROZEN_OCEAN,
        DEEP_FROZEN_OCEAN,

        // Nether Biomes
        NETHER_WASTES,
        CRIMSON_FOREST,
        WARPED_FOREST,
        SOUL_SAND_VALLEY,
        BASALT_DELTAS,

        // End Biomes
        END_HIGHLANDS,
        END_MIDLANDS,
        END_BARRENS,
        SMALL_END_ISLANDS,

        // Special Biomes
        THE_VOID
    };

    /**
     * @struct BiomeProperties
     * @brief Physical and environmental properties of a biome
     */
    struct BiomeProperties {
        std::string name;                    ///< Display name
        std::string description;             ///< Biome description
        Dimension dimension;                 ///< Dimension this biome belongs to
        Climate climate;                     ///< Climate type
        BiomeType type;                      ///< Biome type identifier

        // Environmental properties
        float temperature;                   ///< Temperature (0.0 - 2.0, 0.15 = snowy, 0.8 = warm)
        float humidity;                      ///< Humidity level (0.0 - 1.0)
        float downfall;                      ///< Precipitation amount (0.0 - 1.0)
        float depth;                         ///< Base height modifier
        float scale;                         ///< Height variation scale

        // Visual properties
        uint32_t skyColor;                   ///< Sky color (RGBA)
        uint32_t fogColor;                   ///< Fog color (RGBA)
        uint32_t waterColor;                 ///< Water color (RGBA)
        uint32_t grassColor;                 ///< Grass color (RGBA)
        uint32_t foliageColor;               ///< Foliage color (RGBA)

        // Block properties
        BlockType surfaceBlock;              ///< Top surface block
        BlockType subsurfaceBlock;           ///< Block below surface
        BlockType underwaterBlock;           ///< Block underwater
        BlockType stoneBlock;                ///< Stone variant

        // Vegetation properties
        float treeDensity;                   ///< Trees per chunk (0.0 - 1.0)
        float grassDensity;                  ///< Grass density (0.0 - 1.0)
        float flowerDensity;                 ///< Flower density (0.0 - 1.0)
        float mushroomDensity;               ///< Mushroom density (0.0 - 1.0)

        // Mob spawning
        std::vector<std::string> ambientMobs;    ///< Ambient creatures
        std::vector<std::string> passiveMobs;    ///< Passive creatures
        std::vector<std::string> hostileMobs;    ///< Hostile creatures
        std::vector<std::string> waterMobs;      ///< Water creatures

        // Generation features
        bool canGenerateTrees;               ///< Can generate trees
        bool canGenerateCaves;               ///< Can generate caves
        bool canGenerateLakes;               ///< Can generate lakes
        bool canGenerateOres;                ///< Can generate ores
        bool canGenerateVillages;            ///< Can generate villages
        bool canGenerateTemples;             ///< Can generate temples

        // Music and ambiance
        std::string musicTrack;              ///< Background music
        std::vector<std::string> ambientSounds; ///< Ambient sound effects

        BiomeProperties()
            : name("Unknown Biome")
            , description("An unknown biome")
            , dimension(Dimension::OVERWORLD)
            , climate(Climate::TEMPERATE)
            , type(BiomeType::PLAINS)
            , temperature(0.8f)
            , humidity(0.4f)
            , downfall(0.4f)
            , depth(0.1f)
            , scale(0.2f)
            , skyColor(0xFF87CEEB)  // Light blue
            , fogColor(0xFFC0D8FF)
            , waterColor(0xFF3F76E4)
            , grassColor(0xFF7CBD6B)
            , foliageColor(0xFF48B518)
            , surfaceBlock(BlockType::GRASS_BLOCK)
            , subsurfaceBlock(BlockType::DIRT)
            , underwaterBlock(BlockType::SAND)
            , stoneBlock(BlockType::STONE)
            , treeDensity(0.1f)
            , grassDensity(0.3f)
            , flowerDensity(0.05f)
            , mushroomDensity(0.01f)
            , canGenerateTrees(true)
            , canGenerateCaves(true)
            , canGenerateLakes(true)
            , canGenerateOres(true)
            , canGenerateVillages(false)
            , canGenerateTemples(false)
            , musicTrack("plains")
        {}
    };

    /**
     * @class Biome
     * @brief Represents a biome in the game world
     */
    class Biome {
    public:
        /**
         * @brief Constructor
         * @param type Biome type
         * @param properties Biome properties
         */
        Biome(BiomeType type, const BiomeProperties& properties);

        /**
         * @brief Get biome type
         * @return Biome type
         */
        BiomeType GetType() const { return m_type; }

        /**
         * @brief Get biome properties
         * @return Biome properties
         */
        const BiomeProperties& GetProperties() const { return m_properties; }

        /**
         * @brief Get biome name
         * @return Biome name
         */
        const std::string& GetName() const { return m_properties.name; }

        /**
         * @brief Get biome dimension
         * @return Dimension
         */
        Dimension GetDimension() const { return m_properties.dimension; }

        /**
         * @brief Check if biome is in Overworld
         * @return true if Overworld
         */
        bool IsOverworld() const { return m_properties.dimension == Dimension::OVERWORLD; }

        /**
         * @brief Check if biome is in Nether
         * @return true if Nether
         */
        bool IsNether() const { return m_properties.dimension == Dimension::NETHER; }

        /**
         * @brief Check if biome is in End
         * @return true if End
         */
        bool IsEnd() const { return m_properties.dimension == Dimension::END; }

        /**
         * @brief Get temperature at position
         * @param x World X coordinate
         * @param z World Z coordinate
         * @return Temperature value
         */
        float GetTemperature(int x, int z) const;

        /**
         * @brief Get humidity at position
         * @param x World X coordinate
         * @param z World Z coordinate
         * @return Humidity value
         */
        float GetHumidity(int x, int z) const;

        /**
         * @brief Get surface block for biome
         * @return Surface block type
         */
        BlockType GetSurfaceBlock() const { return m_properties.surfaceBlock; }

        /**
         * @brief Get subsurface block for biome
         * @return Subsurface block type
         */
        BlockType GetSubsurfaceBlock() const { return m_properties.subsurfaceBlock; }

        /**
         * @brief Get stone block for biome
         * @return Stone block type
         */
        BlockType GetStoneBlock() const { return m_properties.stoneBlock; }

        /**
         * @brief Get random mob for spawning
         * @param category Mob category (ambient, passive, hostile, water)
         * @return Mob name or empty string
         */
        std::string GetRandomMob(const std::string& category) const;

        /**
         * @brief Check if feature can generate in this biome
         * @param feature Feature name (trees, caves, lakes, etc.)
         * @return true if can generate
         */
        bool CanGenerateFeature(const std::string& feature) const;

        /**
         * @brief Get biome color for maps
         * @return Color value (RGBA)
         */
        uint32_t GetMapColor() const;

        /**
         * @brief Create default biome of specified type
         * @param type Biome type
         * @return New biome instance
         */
        static std::shared_ptr<Biome> CreateBiome(BiomeType type);

    private:
        BiomeType m_type;                    ///< Biome type identifier
        BiomeProperties m_properties;        ///< Biome properties
    };

    /**
     * @class BiomeManager
     * @brief Manages all biomes and biome generation
     */
    class BiomeManager {
    public:
        /**
         * @brief Constructor
         */
        BiomeManager();

        /**
         * @brief Get biome at world position
         * @param x World X coordinate
         * @param z World Z coordinate
         * @param seed World seed
         * @return Biome type
         */
        BiomeType GetBiomeAt(int x, int z, int seed) const;

        /**
         * @brief Get biome properties
         * @param type Biome type
         * @return Biome properties
         */
        const BiomeProperties& GetBiomeProperties(BiomeType type) const;

        /**
         * @brief Get biome instance
         * @param type Biome type
         * @return Biome instance
         */
        std::shared_ptr<Biome> GetBiome(BiomeType type) const;

        /**
         * @brief Get all biomes in dimension
         * @param dimension Target dimension
         * @return Vector of biome types
         */
        std::vector<BiomeType> GetBiomesInDimension(Dimension dimension) const;

        /**
         * @brief Register custom biome
         * @param type Biome type
         * @param biome Biome instance
         */
        void RegisterBiome(BiomeType type, std::shared_ptr<Biome> biome);

        /**
         * @brief Generate biome map for chunk
         * @param chunkX Chunk X coordinate
         * @param chunkZ Chunk Z coordinate
         * @param seed World seed
         * @return 16x16 array of biome types
         */
        std::vector<BiomeType> GenerateBiomeMap(int chunkX, int chunkZ, int seed) const;

        /**
         * @brief Get biome temperature
         * @param biome Biome type
         * @param x World X coordinate
         * @param z World Z coordinate
         * @return Temperature value
         */
        float GetBiomeTemperature(BiomeType biome, int x, int z) const;

        /**
         * @brief Get biome humidity
         * @param biome Biome type
         * @param x World X coordinate
         * @param z World Z coordinate
         * @return Humidity value
         */
        float GetBiomeHumidity(BiomeType biome, int x, int z) const;

        /**
         * @brief Find biome by temperature and humidity
         * @param temperature Temperature value
         * @param humidity Humidity value
         * @param dimension Target dimension
         * @return Best matching biome
         */
        BiomeType FindBiomeByClimate(float temperature, float humidity, Dimension dimension) const;

    private:
        /**
         * @brief Initialize default biomes
         */
        void InitializeDefaultBiomes();

        /**
         * @brief Generate noise-based biome
         * @param x World X coordinate
         * @param z World Z coordinate
         * @param seed World seed
         * @return Biome type
         */
        BiomeType GenerateBiomeNoise(int x, int z, int seed) const;

        std::unordered_map<BiomeType, std::shared_ptr<Biome>> m_biomes;  ///< All registered biomes
        mutable std::unordered_map<std::string, float> m_noiseCache;     ///< Noise cache for performance
    };

    /**
     * @struct BiomeGenerationSettings
     * @brief Settings for biome generation
     */
    struct BiomeGenerationSettings {
        int seed;                          ///< World seed
        float temperatureScale;            ///< Temperature noise scale
        float humidityScale;               ///< Humidity noise scale
        float biomeScale;                  ///< Biome size scale
        bool enableRivers;                 ///< Enable river generation
        bool enableBeaches;                ///< Enable beach generation
        bool enableBiomes;                 ///< Enable biome-based generation

        BiomeGenerationSettings()
            : seed(0)
            , temperatureScale(0.002f)
            , humidityScale(0.002f)
            , biomeScale(0.01f)
            , enableRivers(true)
            , enableBeaches(true)
            , enableBiomes(true)
        {}
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_WORLD_BIOME_HPP