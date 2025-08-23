/**
 * @file Biome.hpp
 * @brief VoxelCraft Biome System - Biomes, Terrain, and Features
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_BIOME_BIOME_HPP
#define VOXELCRAFT_BIOME_BIOME_HPP

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>
#include <array>
#include <glm/glm.hpp>

#include "../world/World.hpp"

namespace VoxelCraft {

    class Player;
    class MobManager;

    /**
     * @enum BiomeType
     * @brief Types of biomes available in the game
     */
    enum class BiomeType {
        // Overworld Biomes
        PLAINS = 0,              ///< Flat grasslands
        FOREST,                  ///< Dense woodlands
        TAIGA,                   ///< Snowy coniferous forests
        SWAMP,                   ///< Wet, marshy areas
        RIVER,                   ///< Water channels
        NETHER,                  ///< Hell dimension
        THE_END,                 ///< End dimension
        FROZEN_OCEAN,            ///< Frozen ocean
        FROZEN_RIVER,            ///< Frozen river
        SNOWY_TUNDRA,            ///< Snow-covered plains
        SNOWY_MOUNTAINS,         ///< Snowy mountain peaks
        MUSHROOM_FIELDS,         ///< Mushroom-covered islands
        MUSHROOM_FIELD_SHORE,    ///< Mushroom field shores
        BEACH,                   ///< Sandy beaches
        DESERT,                  ///< Arid sandy areas
        DESERT_HILLS,            ///< Hilly deserts
        DESERT_LAKES,            ///< Deserts with lakes
        SAVANNA,                 ///< Tropical grasslands
        SAVANNA_PLATEAU,        ///< Elevated savannas
        BADLANDS,                ///< Eroded orange rock formations
        BADLANDS_PLATEAU,       ///< Elevated badlands
        WOODED_BADLANDS_PLATEAU, ///< Badlands with trees
        ERODED_BADLANDS,         ///< Heavily eroded badlands
        MODIFIED_WOODED_BADLANDS_PLATEAU, ///< Modified wooded badlands
        MODIFIED_BADLANDS_PLATEAU, ///< Modified badlands plateau
        WOODED_HILLS,            ///< Hilly forests
        TAIGA_HILLS,             ///< Hilly taiga
        MOUNTAIN_EDGE,           ///< Mountain edges
        JUNGLE,                  ///< Dense tropical forests
        JUNGLE_HILLS,            ///< Hilly jungles
        JUNGLE_EDGE,             ///< Jungle borders
        MODIFIED_JUNGLE,         ///< Modified jungles
        MODIFIED_JUNGLE_EDGE,    ///< Modified jungle edges
        BAMBOO_JUNGLE,           ///< Bamboo forests
        BAMBOO_JUNGLE_HILLS,     ///< Hilly bamboo forests
        BIRCH_FOREST,            ///< Birch tree forests
        BIRCH_FOREST_HILLS,      ///< Hilly birch forests
        DARK_FOREST,             ///< Dark, dense forests
        DARK_FOREST_HILLS,       ///< Hilly dark forests
        TALL_BIRCH_FOREST,       ///< Tall birch forests
        TALL_BIRCH_HILLS,        ///< Hilly tall birch forests
        GIANT_TREE_TAIGA,        ///< Giant tree taiga
        GIANT_TREE_TAIGA_HILLS,  ///< Hilly giant tree taiga
        GIANT_SPRUCE_TAIGA,      ///< Giant spruce taiga
        GIANT_SPRUCE_TAIGA_HILLS, ///< Hilly giant spruce taiga
        STONE_SHORE,             ///< Rocky shorelines
        SNOWY_BEACH,             ///< Snowy beaches
        GRASSLAND,               ///< Grass-covered plains
        SUNFLOWER_PLAINS,        ///< Plains with sunflowers
        FLOWER_FOREST,           ///< Forests with many flowers
        ICE_SPIKES,              ///< Ice spike formations

        // Ocean Biomes
        OCEAN,                   ///< Standard ocean
        DEEP_OCEAN,              ///< Deep ocean trenches
        WARM_OCEAN,              ///< Warm ocean waters
        LUKEWARM_OCEAN,          ///< Lukewarm ocean waters
        DEEP_WARM_OCEAN,         ///< Deep warm ocean
        DEEP_LUKEWARM_OCEAN,     ///< Deep lukewarm ocean
        COLD_OCEAN,              ///< Cold ocean waters
        DEEP_COLD_OCEAN,         ///< Deep cold ocean
        DEEP_FROZEN_OCEAN,       ///< Deep frozen ocean

        // Cave Biomes
        CAVE,                    ///< Standard cave
        DRIPSTONE_CAVES,         ///< Dripstone formations
        LUSH_CAVES,              ///< Lush cave vegetation

        MAX_BIOME_TYPES
    };

    /**
     * @enum BiomeTemperature
     * @brief Temperature categories for biomes
     */
    enum class BiomeTemperature {
        ICY = 0,                 ///< Very cold (-1.0 to -0.3)
        COLD,                    ///< Cold (-0.3 to 0.2)
        NEUTRAL,                 ///< Neutral (0.2 to 1.0)
        WARM,                    ///< Warm (1.0 to 2.0)
        HOT,                     ///< Very hot (2.0+)
        OCEANIC,                 ///< Ocean temperature
        SPECIAL                  ///< Special temperature rules
    };

    /**
     * @enum BiomeHumidity
     * @brief Humidity categories for biomes
     */
    enum class BiomeHumidity {
        ARID = 0,                ///< Very dry (0.0 to 0.2)
        DRY,                     ///< Dry (0.2 to 0.5)
        NEUTRAL,                 ///< Neutral (0.5 to 0.8)
        WET,                     ///< Wet (0.8 to 1.0)
        HUMID                    ///< Very humid (1.0+)
    };

    /**
     * @struct BiomeColors
     * @brief Color schemes for biomes
     */
    struct BiomeColors {
        uint32_t grassColor = 0x7CBD6B;      ///< Grass color
        uint32_t foliageColor = 0x6EA55D;    ///< Foliage color
        uint32_t waterColor = 0x3F76E4;      ///< Water color
        uint32_t skyColor = 0x78A7FF;       ///< Sky color
        uint32_t fogColor = 0xC0D8FF;       ///< Fog color
        float waterFogDistance = 15.0f;     ///< Water fog distance
        float grassSaturation = 1.0f;       ///< Grass color saturation
    };

    /**
     * @struct BiomeTerrain
     * @brief Terrain generation parameters for biomes
     */
    struct BiomeTerrain {
        float baseHeight = 0.1f;            ///< Base terrain height (0-1)
        float heightVariation = 0.3f;       ///< Height variation (0-1)
        float temperature = 0.5f;           ///< Temperature (0-1)
        float humidity = 0.5f;              ///< Humidity (0-1)
        float roughness = 0.5f;             ///< Terrain roughness
        float hilliness = 0.5f;             ///< Hill frequency
        float mountainness = 0.0f;          ///< Mountain frequency
        float canyonDepth = 0.0f;           ///< Canyon depth
        float erosion = 0.5f;               ///< Erosion factor
        float deposition = 0.5f;            ///< Deposition factor
        int seaLevel = 63;                  ///< Sea level height
        bool hasRivers = true;              ///< Whether biome can have rivers
        bool hasLakes = true;               ///< Whether biome can have lakes
    };

    /**
     * @struct BiomeFeatures
     * @brief Natural features that spawn in biomes
     */
    struct BiomeFeatures {
        // Trees and plants
        std::vector<std::pair<std::string, float>> trees;     ///< Tree types and frequencies
        std::vector<std::pair<std::string, float>> plants;    ///< Plant types and frequencies
        std::vector<std::pair<std::string, float>> flowers;   ///< Flower types and frequencies
        std::vector<std::pair<std::string, float>> mushrooms; ///< Mushroom types and frequencies
        std::vector<std::pair<std::string, float>> crops;     ///< Crop types and frequencies

        // Rocks and minerals
        std::vector<std::pair<std::string, float>> rocks;     ///< Rock types and frequencies
        std::vector<std::pair<std::string, float>> ores;      ///< Ore types and frequencies

        // Natural structures
        std::vector<std::pair<std::string, float>> structures; ///< Structure types and frequencies
        std::vector<std::pair<std::string, float>> caves;     ///< Cave types and frequencies

        // Weather and effects
        float rainFrequency = 0.5f;         ///< Rain frequency (0-1)
        float snowFrequency = 0.0f;         ///< Snow frequency (0-1)
        float stormFrequency = 0.1f;        ///< Storm frequency (0-1)
        float fogDensity = 0.0f;            ///< Fog density (0-1)

        // Ambient effects
        std::vector<std::string> ambientSounds;              ///< Ambient sound effects
        std::vector<std::string> musicTracks;                ///< Background music tracks
        std::vector<std::string> particleEffects;            ///< Particle effects
    };

    /**
     * @struct BiomeSpawning
     * @brief Mob spawning rules for biomes
     */
    struct BiomeSpawning {
        // Passive mobs
        std::vector<std::pair<MobType, float>> passiveMobs;    ///< Passive mob types and weights
        int maxPassiveMobs = 10;                               ///< Maximum passive mobs per chunk

        // Hostile mobs
        std::vector<std::pair<MobType, float>> hostileMobs;    ///< Hostile mob types and weights
        int maxHostileMobs = 70;                               ///< Maximum hostile mobs per chunk

        // Neutral mobs
        std::vector<std::pair<MobType, float>> neutralMobs;    ///< Neutral mob types and weights
        int maxNeutralMobs = 5;                                ///< Maximum neutral mobs per chunk

        // Water mobs
        std::vector<std::pair<MobType, float>> waterMobs;      ///< Water mob types and weights
        int maxWaterMobs = 5;                                  ///< Maximum water mobs per chunk

        // Ambient mobs
        std::vector<std::pair<MobType, float>> ambientMobs;    ///< Ambient mob types and weights
        int maxAmbientMobs = 15;                               ///< Maximum ambient mobs per chunk

        // Spawning conditions
        int minSpawnLightLevel = 0;                            ///< Minimum light level for spawning
        int maxSpawnLightLevel = 7;                            ///< Maximum light level for spawning
        bool spawnsInDarkness = true;                          ///< Whether mobs spawn in darkness
        bool spawnsInDaylight = false;                         ///< Whether mobs spawn in daylight
        float spawnDensity = 1.0f;                             ///< Overall spawn density multiplier
    };

    /**
     * @struct BiomeDefinition
     * @brief Complete definition of a biome
     */
    struct BiomeDefinition {
        BiomeType type;
        std::string name;
        std::string displayName;
        std::string description;
        BiomeTemperature temperature;
        BiomeHumidity humidity;
        BiomeColors colors;
        BiomeTerrain terrain;
        BiomeFeatures features;
        BiomeSpawning spawning;
        std::vector<BiomeType> similarBiomes;                 ///< Similar biomes for transitions
        std::vector<BiomeType> edgeBiomes;                    ///< Edge biomes for borders
        std::vector<BiomeType> hillBiomes;                    ///< Hill variants
        std::vector<BiomeType> mutatedBiomes;                 ///< Mutated variants
        std::unordered_map<std::string, std::any> customProperties;
    };

    /**
     * @class Biome
     * @brief Base class for all biomes
     */
    class Biome {
    public:
        /**
         * @brief Constructor
         * @param definition Biome definition
         */
        Biome(const BiomeDefinition& definition);

        /**
         * @brief Destructor
         */
        virtual ~Biome() = default;

        /**
         * @brief Get biome type
         * @return Biome type
         */
        BiomeType GetType() const { return m_definition.type; }

        /**
         * @brief Get biome name
         * @return Biome name
         */
        const std::string& GetName() const { return m_definition.name; }

        /**
         * @brief Get biome definition
         * @return Biome definition
         */
        const BiomeDefinition& GetDefinition() const { return m_definition; }

        /**
         * @brief Get biome temperature
         * @return Temperature category
         */
        BiomeTemperature GetTemperature() const { return m_definition.temperature; }

        /**
         * @brief Get biome humidity
         * @return Humidity category
         */
        BiomeHumidity GetHumidity() const { return m_definition.humidity; }

        /**
         * @brief Get biome colors
         * @return Biome colors
         */
        const BiomeColors& GetColors() const { return m_definition.colors; }

        /**
         * @brief Get biome terrain
         * @return Biome terrain parameters
         */
        const BiomeTerrain& GetTerrain() const { return m_definition.terrain; }

        /**
         * @brief Get biome features
         * @return Biome features
         */
        const BiomeFeatures& GetFeatures() const { return m_definition.features; }

        /**
         * @brief Get biome spawning rules
         * @return Biome spawning rules
         */
        const BiomeSpawning& GetSpawning() const { return m_definition.spawning; }

        /**
         * @brief Check if biome can generate at position
         * @param position World position
         * @param world World reference
         * @return true if can generate
         */
        virtual bool CanGenerateAt(const glm::ivec3& position, World* world) const;

        /**
         * @brief Generate terrain at position
         * @param position World position
         * @param world World reference
         * @return Generated height
         */
        virtual int GenerateTerrain(const glm::ivec3& position, World* world);

        /**
         * @brief Generate features at position
         * @param position World position
         * @param world World reference
         */
        virtual void GenerateFeatures(const glm::ivec3& position, World* world);

        /**
         * @brief Generate ores at position
         * @param position World position
         * @param world World reference
         */
        virtual void GenerateOres(const glm::ivec3& position, World* world);

        /**
         * @brief Generate structures at position
         * @param position World position
         * @param world World reference
         */
        virtual void GenerateStructures(const glm::ivec3& position, World* world);

        /**
         * @brief Get spawnable mobs for this biome
         * @param timeOfDay Time of day (0-1, 0=midnight, 0.5=noon)
         * @return Vector of spawnable mob types
         */
        virtual std::vector<MobType> GetSpawnableMobs(float timeOfDay) const;

        /**
         * @brief Get biome ambient color
         * @param timeOfDay Time of day (0-1)
         * @return Ambient color
         */
        virtual uint32_t GetAmbientColor(float timeOfDay) const;

        /**
         * @brief Get biome temperature at position
         * @param position World position
         * @return Temperature (0-1)
         */
        virtual float GetTemperatureAt(const glm::vec3& position) const;

        /**
         * @brief Get biome humidity at position
         * @param position World position
         * @return Humidity (0-1)
         */
        virtual float GetHumidityAt(const glm::vec3& position) const;

        /**
         * @brief Check if position is suitable for specific feature
         * @param position World position
         * @param featureType Feature type
         * @return true if suitable
         */
        virtual bool IsSuitableForFeature(const glm::vec3& position, const std::string& featureType) const;

        /**
         * @brief Get biome-specific block at position
         * @param position World position
         * @param depth Depth from surface
         * @return Block ID
         */
        virtual int GetBlockAt(const glm::vec3& position, int depth) const;

        /**
         * @brief Get biome-specific surface block
         * @return Surface block ID
         */
        virtual int GetSurfaceBlock() const;

        /**
         * @brief Get biome-specific filler block
         * @return Filler block ID
         */
        virtual int GetFillerBlock() const;

        /**
         * @brief Get biome-specific underwater block
         * @return Underwater block ID
         */
        virtual int GetUnderwaterBlock() const;

    protected:
        BiomeDefinition m_definition;

        /**
         * @brief Generate noise-based height
         * @param x X coordinate
         * @param z Z coordinate
         * @param seed World seed
         * @return Height value
         */
        float GenerateNoiseHeight(int x, int z, int seed) const;

        /**
         * @brief Check if position has suitable conditions
         * @param position World position
         * @param world World reference
         * @return true if suitable
         */
        bool HasSuitableConditions(const glm::vec3& position, World* world) const;

        /**
         * @brief Apply biome-specific modifications
         * @param position World position
         * @param world World reference
         */
        virtual void ApplyBiomeModifications(const glm::vec3& position, World* world);
    };

    /**
     * @class PlainsBiome
     * @brief Plains biome implementation
     */
    class PlainsBiome : public Biome {
    public:
        PlainsBiome();
    };

    /**
     * @class ForestBiome
     * @brief Forest biome implementation
     */
    class ForestBiome : public Biome {
    public:
        ForestBiome();
    };

    /**
     * @class DesertBiome
     * @brief Desert biome implementation
     */
    class DesertBiome : public Biome {
    public:
        DesertBiome();
    };

    /**
     * @class OceanBiome
     * @brief Ocean biome implementation
     */
    class OceanBiome : public Biome {
    public:
        OceanBiome();
    };

    /**
     * @class TundraBiome
     * @brief Tundra biome implementation
     */
    class TundraBiome : public Biome {
    public:
        TundraBiome();
    };

    /**
     * @class JungleBiome
     * @brief Jungle biome implementation
     */
    class JungleBiome : public Biome {
    public:
        JungleBiome();
    };

    /**
     * @class SwampBiome
     * @brief Swamp biome implementation
     */
    class SwampBiome : public Biome {
    public:
        SwampBiome();
    };

    /**
     * @class SavannaBiome
     * @brief Savanna biome implementation
     */
    class SavannaBiome : public Biome {
    public:
        SavannaBiome();
    };

    /**
     * @class TaigaBiome
     * @brief Taiga biome implementation
     */
    class TaigaBiome : public Biome {
    public:
        TaigaBiome();
    };

    /**
     * @class MountainBiome
     * @brief Mountain biome implementation
     */
    class MountainBiome : public Biome {
    public:
        MountainBiome();
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_BIOME_BIOME_HPP
