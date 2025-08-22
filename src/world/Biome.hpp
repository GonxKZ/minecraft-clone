/**
 * @file Biome.hpp
 * @brief VoxelCraft Advanced Biome System
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines the biome system that creates diverse environments
 * with unique terrain, vegetation, structures, and atmospheric conditions.
 */

#ifndef VOXELCRAFT_WORLD_BIOME_HPP
#define VOXELCRAFT_WORLD_BIOME_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>
#include <optional>

#include "../core/Config.hpp"

namespace VoxelCraft {

    /**
     * @enum BiomeType
     * @brief Type of biome for classification
     */
    enum class BiomeType {
        // Ocean biomes
        Ocean,              ///< Standard ocean
        DeepOcean,          ///< Deep ocean
        WarmOcean,          ///< Warm ocean
        LukewarmOcean,      ///< Lukewarm ocean
        ColdOcean,          ///< Cold ocean
        FrozenOcean,        ///< Frozen ocean

        // Beach biomes
        Beach,              ///< Standard beach
        StoneBeach,         ///< Stone beach

        // Desert biomes
        Desert,             ///< Standard desert
        DesertHills,        ///< Desert hills

        // Savanna biomes
        Savanna,            ///< Standard savanna
        SavannaPlateau,     ///< Savanna plateau
        ShatteredSavanna,   ///< Shattered savanna
        ShatteredSavannaPlateau, ///< Shattered savanna plateau

        // Plains biomes
        Plains,             ///< Standard plains
        SunflowerPlains,    ///< Sunflower plains

        // Forest biomes
        Forest,             ///< Standard forest
        ForestHills,        ///< Forest hills
        FlowerForest,       ///< Flower forest
        BirchForest,        ///< Birch forest
        BirchForestHills,   ///< Birch forest hills
        DarkForest,         ///< Dark forest
        DarkForestHills,    ///< Dark forest hills

        // Taiga biomes
        Taiga,              ///< Standard taiga
        TaigaHills,         ///< Taiga hills
        TaigaMountains,     ///< Taiga mountains
        SnowyTaiga,         ///< Snowy taiga
        SnowyTaigaHills,    ///< Snowy taiga hills
        SnowyTaigaMountains, ///< Snowy taiga mountains

        // Swamp biomes
        Swamp,              ///< Standard swamp
        SwampHills,         ///< Swamp hills

        // Jungle biomes
        Jungle,             ///< Standard jungle
        JungleHills,        ///< Jungle hills
        JungleEdge,         ///< Jungle edge
        ModifiedJungle,     ///< Modified jungle
        ModifiedJungleEdge, ///< Modified jungle edge

        // Mountain biomes
        Mountains,          ///< Standard mountains
        MountainEdge,       ///< Mountain edge
        GravellyMountains,  ///< Gravelly mountains
        ModifiedGravellyMountains, ///< Modified gravelly mountains

        // Snowy biomes
        SnowyTundra,        ///< Snowy tundra
        SnowyMountains,     ///< Snowy mountains
        IceSpikes,          ///< Ice spikes

        // River biomes
        River,              ///< Standard river
        FrozenRiver,        ///< Frozen river

        // Mushroom biomes
        MushroomFields,     ///< Mushroom fields
        MushroomFieldShore, ///< Mushroom field shore

        // Nether biomes
        Nether,             ///< Standard nether
        NetherWastes,       ///< Nether wastes
        SoulSandValley,     ///< Soul sand valley
        CrimsonForest,      ///< Crimson forest
        WarpedForest,       ///< Warped forest
        BasaltDeltas,       ///< Basalt deltas

        // End biomes
        End,                ///< Standard end
        EndHighlands,       ///< End highlands
        EndMidlands,        ///< End midlands
        EndBarrens,         ///< End barrens
        SmallEndIslands,    ///< Small end islands
        EndIslands,         ///< End islands

        // Custom biomes
        Custom,             ///< Custom biome type
        Count               ///< Number of biome types
    };

    /**
     * @struct BiomeColor
     * @brief Color information for biome rendering
     */
    struct BiomeColor {
        uint32_t grassColor;        ///< Grass color
        uint32_t foliageColor;      ///< Foliage color
        uint32_t waterColor;        ///< Water color
        uint32_t skyColor;          ///< Sky color
        uint32_t fogColor;          ///< Fog color
        float temperature;          ///< Biome temperature (0.0 - 1.0)
        float downfall;             ///< Biome rainfall (0.0 - 1.0)
    };

    /**
     * @struct BiomeTerrain
     * @brief Terrain generation parameters for biome
     */
    struct BiomeTerrain {
        float baseHeight;           ///< Base terrain height
        float heightVariation;      ///< Height variation
        float roughness;            ///< Terrain roughness
        float hilliness;            ///< Hill formation
        float terrainScale;         ///< Terrain scale factor
        float detailScale;          ///< Detail scale factor
        int minHeight;              ///< Minimum height
        int maxHeight;              ///< Maximum height
        bool hasRivers;             ///< Has rivers
        bool hasCaves;              ///< Has caves
        float caveDensity;          ///< Cave generation density
    };

    /**
     * @struct BiomeFeatures
     * @brief Feature generation parameters for biome
     */
    struct BiomeFeatures {
        // Vegetation
        float treeDensity;          ///< Tree generation density
        float flowerDensity;        ///< Flower generation density
        float grassDensity;         ///< Grass generation density
        float bushDensity;          ///< Bush generation density
        float reedDensity;          ///< Reed generation density
        float cactusDensity;        ///< Cactus generation density

        // Ores
        float coalDensity;          ///< Coal ore density
        float ironDensity;          ///< Iron ore density
        float goldDensity;          ///< Gold ore density
        float diamondDensity;       ///< Diamond ore density
        float redstoneDensity;      ///< Redstone ore density
        float lapisDensity;         ///< Lapis ore density

        // Structures
        float villageDensity;       ///< Village generation density
        float dungeonDensity;       ///< Dungeon generation density
        float templeDensity;        ///< Temple generation density
        float strongholdDensity;    ///< Stronghold generation density
        float mansionDensity;       ///< Mansion generation density
    };

    /**
     * @struct BiomeClimate
     * @brief Climate parameters for biome
     */
    struct BiomeClimate {
        float temperature;          ///< Base temperature (0.0 - 1.0)
        float temperatureVariation; ///< Temperature variation
        float humidity;             ///< Base humidity (0.0 - 1.0)
        float humidityVariation;    ///< Humidity variation
        float precipitation;        ///< Precipitation amount
        bool canSnow;               ///< Can snow in this biome
        bool canRain;               ///< Can rain in this biome
        bool isHumid;               ///< Is humid biome
        bool isDry;                 ///< Is dry biome
        bool isCold;                ///< Is cold biome
        bool isHot;                 ///< Is hot biome
    };

    /**
     * @struct BiomeSpawning
     * @brief Mob spawning parameters for biome
     */
    struct BiomeSpawning {
        struct SpawnEntry {
            std::string entityType;     ///< Entity type to spawn
            int minCount;              ///< Minimum spawn count
            int maxCount;              ///< Maximum spawn count
            int weight;                ///< Spawn weight
            int minY;                  ///< Minimum Y level
            int maxY;                  ///< Maximum Y level
        };

        std::vector<SpawnEntry> passiveMobs;    ///< Passive mob spawns
        std::vector<SpawnEntry> hostileMobs;    ///< Hostile mob spawns
        std::vector<SpawnEntry> ambientMobs;    ///< Ambient mob spawns
        std::vector<SpawnEntry> waterMobs;      ///< Water mob spawns

        float spawnRate;              ///< Overall spawn rate multiplier
        bool canSpawnAnimals;         ///< Can spawn animals
        bool canSpawnMonsters;        ///< Can spawn monsters
        bool canSpawnAmbient;         ///< Can spawn ambient creatures
    };

    /**
     * @struct BiomeMusic
     * @brief Music and audio parameters for biome
     */
    struct BiomeMusic {
        std::vector<std::string> musicTracks;    ///< Available music tracks
        std::vector<std::string> ambientSounds;  ///< Ambient sound effects
        std::vector<std::string> moodSounds;     ///< Mood sound effects
        float musicVolume;         ///< Music volume multiplier
        float ambientVolume;       ///< Ambient volume multiplier
        float moodVolume;          ///< Mood volume multiplier
        float musicInterval;       ///< Music interval (seconds)
        float moodInterval;        ///< Mood interval (seconds)
    };

    /**
     * @class Biome
     * @brief Represents a biome with all its characteristics
     *
     * A biome defines the environment, terrain generation, vegetation,
     * structures, and atmosphere of a specific region in the world.
     *
     * Features:
     * - Unique terrain generation parameters
     * - Custom vegetation and feature placement
     * - Specific mob spawning rules
     * - Unique atmospheric conditions
     * - Custom music and ambient sounds
     * - Color variations for rendering
     */
    class Biome {
    public:
        /**
         * @brief Constructor
         * @param name Biome name
         * @param type Biome type
         */
        Biome(const std::string& name, BiomeType type);

        /**
         * @brief Destructor
         */
        ~Biome();

        /**
         * @brief Deleted copy constructor
         */
        Biome(const Biome&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        Biome& operator=(const Biome&) = delete;

        // Biome identification

        /**
         * @brief Get biome name
         * @return Biome name
         */
        const std::string& GetName() const { return m_name; }

        /**
         * @brief Get biome type
         * @return Biome type
         */
        BiomeType GetType() const { return m_type; }

        /**
         * @brief Get biome ID
         * @return Unique biome ID
         */
        uint32_t GetID() const { return m_id; }

        // Biome properties

        /**
         * @brief Get biome color information
         * @return Biome colors
         */
        const BiomeColor& GetColor() const { return m_color; }

        /**
         * @brief Set biome color
         * @param color New color information
         */
        void SetColor(const BiomeColor& color) { m_color = color; }

        /**
         * @brief Get terrain parameters
         * @return Terrain parameters
         */
        const BiomeTerrain& GetTerrain() const { return m_terrain; }

        /**
         * @brief Set terrain parameters
         * @param terrain New terrain parameters
         */
        void SetTerrain(const BiomeTerrain& terrain) { m_terrain = terrain; }

        /**
         * @brief Get feature parameters
         * @return Feature parameters
         */
        const BiomeFeatures& GetFeatures() const { return m_features; }

        /**
         * @brief Set feature parameters
         * @param features New feature parameters
         */
        void SetFeatures(const BiomeFeatures& features) { m_features = features; }

        /**
         * @brief Get climate parameters
         * @return Climate parameters
         */
        const BiomeClimate& GetClimate() const { return m_climate; }

        /**
         * @brief Set climate parameters
         * @param climate New climate parameters
         */
        void SetClimate(const BiomeClimate& climate) { m_climate = climate; }

        /**
         * @brief Get spawning parameters
         * @return Spawning parameters
         */
        const BiomeSpawning& GetSpawning() const { return m_spawning; }

        /**
         * @brief Set spawning parameters
         * @param spawning New spawning parameters
         */
        void SetSpawning(const BiomeSpawning& spawning) { m_spawning = spawning; }

        /**
         * @brief Get music parameters
         * @return Music parameters
         */
        const BiomeMusic& GetMusic() const { return m_music; }

        /**
         * @brief Set music parameters
         * @param music New music parameters
         */
        void SetMusic(const BiomeMusic& music) { m_music = music; }

        // Biome queries

        /**
         * @brief Get temperature at specific location
         * @param x World X coordinate
         * @param z World Z coordinate
         * @return Temperature (0.0 - 1.0)
         */
        float GetTemperature(int x, int z) const;

        /**
         * @brief Get humidity at specific location
         * @param x World X coordinate
         * @param z World Z coordinate
         * @return Humidity (0.0 - 1.0)
         */
        float GetHumidity(int x, int z) const;

        /**
         * @brief Check if position is suitable for tree generation
         * @param x World X coordinate
         * @param y World Y coordinate
         * @param z World Z coordinate
         * @return true if suitable, false otherwise
         */
        bool IsSuitableForTree(int x, int y, int z) const;

        /**
         * @brief Check if position is suitable for flower generation
         * @param x World X coordinate
         * @param y World Y coordinate
         * @param z World Z coordinate
         * @return true if suitable, false otherwise
         */
        bool IsSuitableForFlower(int x, int y, int z) const;

        /**
         * @brief Get grass color at position
         * @param x World X coordinate
         * @param z World Z coordinate
         * @return Grass color
         */
        uint32_t GetGrassColor(int x, int z) const;

        /**
         * @brief Get foliage color at position
         * @param x World X coordinate
         * @param z World Z coordinate
         * @return Foliage color
         */
        uint32_t GetFoliageColor(int x, int z) const;

        /**
         * @brief Get water color at position
         * @param x World X coordinate
         * @param z World Z coordinate
         * @return Water color
         */
        uint32_t GetWaterColor(int x, int z) const;

        // Biome customization

        /**
         * @brief Add custom property
         * @param key Property key
         * @param value Property value
         */
        template<typename T>
        void SetProperty(const std::string& key, const T& value);

        /**
         * @brief Get custom property
         * @param key Property key
         * @param defaultValue Default value
         * @return Property value or default
         */
        template<typename T>
        T GetProperty(const std::string& key, const T& defaultValue = T{}) const;

        /**
         * @brief Check if biome has property
         * @param key Property key
         * @return true if has property, false otherwise
         */
        bool HasProperty(const std::string& key) const;

        /**
         * @brief Remove custom property
         * @param key Property key
         */
        void RemoveProperty(const std::string& key);

    private:
        /**
         * @brief Initialize biome with default values
         */
        void InitializeDefaults();

        /**
         * @brief Generate unique biome ID
         * @return Unique ID
         */
        static uint32_t GenerateID();

        std::string m_name;                         ///< Biome name
        BiomeType m_type;                           ///< Biome type
        uint32_t m_id;                              ///< Unique biome ID

        BiomeColor m_color;                         ///< Color information
        BiomeTerrain m_terrain;                     ///< Terrain parameters
        BiomeFeatures m_features;                   ///< Feature parameters
        BiomeClimate m_climate;                     ///< Climate parameters
        BiomeSpawning m_spawning;                   ///< Spawning parameters
        BiomeMusic m_music;                         ///< Music parameters

        std::unordered_map<std::string, std::any> m_properties; ///< Custom properties

        static uint32_t s_nextID;                   ///< Next biome ID
    };

    // Template implementations

    template<typename T>
    void Biome::SetProperty(const std::string& key, const T& value) {
        m_properties[key] = value;
    }

    template<typename T>
    T Biome::GetProperty(const std::string& key, const T& defaultValue) const {
        auto it = m_properties.find(key);
        if (it != m_properties.end()) {
            try {
                return std::any_cast<T>(it->second);
            } catch (const std::bad_any_cast&) {
                return defaultValue;
            }
        }
        return defaultValue;
    }

    /**
     * @class BiomeRegistry
     * @brief Registry for managing biomes
     */
    class BiomeRegistry {
    public:
        /**
         * @brief Register a biome
         * @param biome Biome to register
         * @return true if registered, false if name exists
         */
        static bool RegisterBiome(std::unique_ptr<Biome> biome);

        /**
         * @brief Get biome by name
         * @param name Biome name
         * @return Biome pointer or nullptr if not found
         */
        static Biome* GetBiome(const std::string& name);

        /**
         * @brief Get biome by type
         * @param type Biome type
         * @return Biome pointer or nullptr if not found
         */
        static Biome* GetBiome(BiomeType type);

        /**
         * @brief Get biome by ID
         * @param id Biome ID
         * @return Biome pointer or nullptr if not found
         */
        static Biome* GetBiome(uint32_t id);

        /**
         * @brief Get all registered biomes
         * @return Vector of biome pointers
         */
        static std::vector<Biome*> GetAllBiomes();

        /**
         * @brief Get biomes of specific type
         * @param type Biome type
         * @return Vector of biomes of type
         */
        static std::vector<Biome*> GetBiomesOfType(BiomeType type);

        /**
         * @brief Check if biome is registered
         * @param name Biome name
         * @return true if registered, false otherwise
         */
        static bool IsRegistered(const std::string& name);

        /**
         * @brief Unregister biome
         * @param name Biome name
         * @return true if unregistered, false if not found
         */
        static bool UnregisterBiome(const std::string& name);

        /**
         * @brief Clear all registered biomes
         */
        static void ClearRegistry();

        /**
         * @brief Initialize default biomes
         */
        static void InitializeDefaults();

    private:
        static std::unordered_map<std::string, std::unique_ptr<Biome>> s_biomes;
        static std::unordered_map<BiomeType, Biome*> s_typeMap;
        static std::unordered_map<uint32_t, Biome*> s_idMap;
    };

    /**
     * @class BiomeGenerator
     * @brief Generates biomes based on temperature and humidity
     */
    class BiomeGenerator {
    public:
        /**
         * @brief Constructor
         * @param seed Random seed
         */
        explicit BiomeGenerator(int seed);

        /**
         * @brief Get biome at coordinates
         * @param x World X coordinate
         * @param z World Z coordinate
         * @return Biome at coordinates
         */
        Biome GetBiome(int x, int z);

        /**
         * @brief Get temperature at coordinates
         * @param x World X coordinate
         * @param z World Z coordinate
         * @return Temperature (0.0 - 1.0)
         */
        float GetTemperature(int x, int z);

        /**
         * @brief Get humidity at coordinates
         * @param x World X coordinate
         * @param z World Z coordinate
         * @return Humidity (0.0 - 1.0)
         */
        float GetHumidity(int x, int z);

        /**
         * @brief Set temperature scale
         * @param scale Temperature scale factor
         */
        void SetTemperatureScale(float scale) { m_temperatureScale = scale; }

        /**
         * @brief Set humidity scale
         * @param scale Humidity scale factor
         */
        void SetHumidityScale(float scale) { m_humidityScale = scale; }

        /**
         * @brief Set biome scale
         * @param scale Biome scale factor
         */
        void SetBiomeScale(float scale) { m_biomeScale = scale; }

    private:
        /**
         * @brief Determine biome from temperature and humidity
         * @param temperature Temperature (0.0 - 1.0)
         * @param humidity Humidity (0.0 - 1.0)
         * @return Biome type
         */
        BiomeType GetBiomeType(float temperature, float humidity) const;

        /**
         * @brief Get biome color based on temperature and humidity
         * @param temperature Temperature (0.0 - 1.0)
         * @param humidity Humidity (0.0 - 1.0)
         * @return Biome color
         */
        BiomeColor GetBiomeColor(float temperature, float humidity) const;

        int m_seed;                    ///< Random seed
        float m_temperatureScale;      ///< Temperature scale factor
        float m_humidityScale;         ///< Humidity scale factor
        float m_biomeScale;            ///< Biome scale factor
    };

    /**
     * @class BiomeUtils
     * @brief Utility functions for biome operations
     */
    class BiomeUtils {
    public:
        /**
         * @brief Convert biome type to string
         * @param type Biome type
         * @return String representation
         */
        static std::string BiomeTypeToString(BiomeType type);

        /**
         * @brief Convert string to biome type
         * @param str String representation
         * @return Biome type or empty optional if invalid
         */
        static std::optional<BiomeType> StringToBiomeType(const std::string& str);

        /**
         * @brief Get biome display name
         * @param type Biome type
         * @return Display name
         */
        static std::string GetBiomeDisplayName(BiomeType type);

        /**
         * @brief Get biome description
         * @param type Biome type
         * @return Description text
         */
        static std::string GetBiomeDescription(BiomeType type);

        /**
         * @brief Check if biome is ocean type
         * @param type Biome type
         * @return true if ocean, false otherwise
         */
        static bool IsOceanBiome(BiomeType type);

        /**
         * @brief Check if biome is desert type
         * @param type Biome type
         * @return true if desert, false otherwise
         */
        static bool IsDesertBiome(BiomeType type);

        /**
         * @brief Check if biome is forest type
         * @param type Biome type
         * @return true if forest, false otherwise
         */
        static bool IsForestBiome(BiomeType type);

        /**
         * @brief Check if biome is cold type
         * @param type Biome type
         * @return true if cold, false otherwise
         */
        static bool IsColdBiome(BiomeType type);

        /**
         * @brief Check if biome is hot type
         * @param type Biome type
         * @return true if hot, false otherwise
         */
        static bool IsHotBiome(BiomeType type);

        /**
         * @brief Get biome spawn rate multiplier
         * @param type Biome type
         * @return Spawn rate multiplier
         */
        static float GetBiomeSpawnRate(BiomeType type);

        /**
         * @brief Get biome temperature range
         * @param type Biome type
         * @return Temperature range (min, max)
         */
        static std::pair<float, float> GetBiomeTemperatureRange(BiomeType type);

        /**
         * @brief Get biome humidity range
         * @param type Biome type
         * @return Humidity range (min, max)
         */
        static std::pair<float, float> GetBiomeHumidityRange(BiomeType type);
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_WORLD_BIOME_HPP
