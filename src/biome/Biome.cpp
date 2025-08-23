/**
 * @file Biome.cpp
 * @brief VoxelCraft Biome System Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#include "Biome.hpp"
#include "../world/World.hpp"
#include "../player/Player.hpp"
#include <algorithm>
#include <random>
#include <cmath>
#include <noise/noise.h> // For noise generation

namespace VoxelCraft {

// Biome base implementation
Biome::Biome(const BiomeDefinition& definition)
    : m_definition(definition) {
}

bool Biome::CanGenerateAt(const glm::ivec3& position, World* world) const {
    if (!world) return false;

    // Check if position is within biome generation bounds
    float temperature = GetTemperatureAt(glm::vec3(position));
    float humidity = GetHumidityAt(glm::vec3(position));

    // Check temperature and humidity compatibility
    switch (m_definition.temperature) {
        case BiomeTemperature::ICY:
            if (temperature > 0.1f) return false;
            break;
        case BiomeTemperature::COLD:
            if (temperature < 0.0f || temperature > 0.3f) return false;
            break;
        case BiomeTemperature::NEUTRAL:
            if (temperature < 0.2f || temperature > 1.0f) return false;
            break;
        case BiomeTemperature::WARM:
            if (temperature < 0.8f || temperature > 1.5f) return false;
            break;
        case BiomeTemperature::HOT:
            if (temperature < 1.5f) return false;
            break;
        default:
            break;
    }

    switch (m_definition.humidity) {
        case BiomeHumidity::ARID:
            if (humidity > 0.2f) return false;
            break;
        case BiomeHumidity::DRY:
            if (humidity < 0.0f || humidity > 0.5f) return false;
            break;
        case BiomeHumidity::NEUTRAL:
            if (humidity < 0.3f || humidity > 0.8f) return false;
            break;
        case BiomeHumidity::WET:
            if (humidity < 0.7f || humidity > 1.0f) return false;
            break;
        case BiomeHumidity::HUMID:
            if (humidity < 0.9f) return false;
            break;
    }

    return HasSuitableConditions(glm::vec3(position), world);
}

int Biome::GenerateTerrain(const glm::ivec3& position, World* world) {
    if (!world) return 63; // Default sea level

    // Generate base height using noise
    float noiseHeight = GenerateNoiseHeight(position.x, position.z, world->GetSeed());
    float baseHeight = m_definition.terrain.baseHeight * 128.0f; // Base height in blocks
    float heightVariation = m_definition.terrain.heightVariation * 64.0f; // Height variation

    int height = static_cast<int>(baseHeight + noiseHeight * heightVariation);

    // Apply terrain modifications
    ApplyBiomeModifications(glm::vec3(position.x, height, position.z), world);

    // Ensure height is within valid bounds
    height = std::max(0, std::min(255, height));

    return height;
}

void Biome::GenerateFeatures(const glm::ivec3& position, World* world) {
    if (!world) return;

    // Generate trees
    for (const auto& tree : m_definition.features.trees) {
        if (GenerateNoiseHeight(position.x, position.z, world->GetSeed() + 1000) < tree.second) {
            // Generate tree at this position
            // This would call the tree generation system
        }
    }

    // Generate plants
    for (const auto& plant : m_definition.features.plants) {
        if (GenerateNoiseHeight(position.x, position.z, world->GetSeed() + 2000) < plant.second) {
            // Generate plant at this position
            // This would place plant blocks
        }
    }

    // Generate flowers
    for (const auto& flower : m_definition.features.flowers) {
        if (GenerateNoiseHeight(position.x, position.z, world->GetSeed() + 3000) < flower.second) {
            // Generate flower at this position
            // This would place flower blocks
        }
    }
}

void Biome::GenerateOres(const glm::ivec3& position, World* world) {
    if (!world) return;

    // Generate ores based on biome-specific distribution
    for (const auto& ore : m_definition.features.ores) {
        if (GenerateNoiseHeight(position.x, position.z, world->GetSeed() + 4000) < ore.second) {
            // Generate ore vein at this position
            // This would call the ore generation system
        }
    }
}

void Biome::GenerateStructures(const glm::ivec3& position, World* world) {
    if (!world) return;

    // Generate structures based on biome
    for (const auto& structure : m_definition.features.structures) {
        if (GenerateNoiseHeight(position.x, position.z, world->GetSeed() + 5000) < structure.second) {
            // Generate structure at this position
            // This would call the structure generation system
        }
    }
}

std::vector<MobType> Biome::GetSpawnableMobs(float timeOfDay) const {
    std::vector<MobType> spawnableMobs;

    // Determine if it's day or night
    bool isDay = (timeOfDay > 0.25f && timeOfDay < 0.75f);

    // Add passive mobs (always spawn during day)
    if (isDay || m_definition.spawning.spawnsInDaylight) {
        for (const auto& mob : m_definition.spawning.passiveMobs) {
            spawnableMobs.push_back(mob.first);
        }
    }

    // Add hostile mobs (spawn at night or in darkness)
    if (!isDay || m_definition.spawning.spawnsInDarkness) {
        for (const auto& mob : m_definition.spawning.hostileMobs) {
            spawnableMobs.push_back(mob.first);
        }
    }

    // Add neutral mobs
    for (const auto& mob : m_definition.spawning.neutralMobs) {
        spawnableMobs.push_back(mob.first);
    }

    // Add water mobs if in water
    for (const auto& mob : m_definition.spawning.waterMobs) {
        spawnableMobs.push_back(mob.first);
    }

    // Add ambient mobs
    for (const auto& mob : m_definition.spawning.ambientMobs) {
        spawnableMobs.push_back(mob.first);
    }

    return spawnableMobs;
}

uint32_t Biome::GetAmbientColor(float timeOfDay) const {
    // Calculate ambient color based on time of day
    float dayFactor = std::sin(timeOfDay * 2.0f * 3.14159f);

    if (dayFactor > 0) {
        // Daytime - bright colors
        return m_definition.colors.skyColor;
    } else {
        // Nighttime - dark colors
        return (m_definition.colors.skyColor & 0xFF000000) |
               ((m_definition.colors.skyColor & 0x00FFFFFF) >> 1);
    }
}

float Biome::GetTemperatureAt(const glm::vec3& position) const {
    // Calculate temperature based on position and biome characteristics
    float baseTemp = m_definition.terrain.temperature;

    // Add height-based temperature variation
    float heightFactor = 1.0f - (position.y / 128.0f); // Higher = colder
    baseTemp *= heightFactor;

    // Add noise-based variation
    float noise = GenerateNoiseHeight(static_cast<int>(position.x), static_cast<int>(position.z), 0);
    baseTemp += noise * 0.1f;

    return std::max(0.0f, std::min(2.0f, baseTemp));
}

float Biome::GetHumidityAt(const glm::vec3& position) const {
    // Calculate humidity based on position and biome characteristics
    float baseHumidity = m_definition.terrain.humidity;

    // Add noise-based variation
    float noise = GenerateNoiseHeight(static_cast<int>(position.x), static_cast<int>(position.z), 1000);
    baseHumidity += noise * 0.2f;

    return std::max(0.0f, std::min(1.0f, baseHumidity));
}

bool Biome::IsSuitableForFeature(const glm::vec3& position, const std::string& featureType) const {
    // Check if this biome supports the given feature type
    if (featureType == "tree") {
        return !m_definition.features.trees.empty();
    } else if (featureType == "plant") {
        return !m_definition.features.plants.empty();
    } else if (featureType == "flower") {
        return !m_definition.features.flowers.empty();
    } else if (featureType == "structure") {
        return !m_definition.features.structures.empty();
    } else if (featureType == "cave") {
        return !m_definition.features.caves.empty();
    }

    return false;
}

int Biome::GetBlockAt(const glm::vec3& position, int depth) const {
    if (depth == 0) {
        return GetSurfaceBlock();
    } else if (depth < 4) {
        return GetFillerBlock();
    } else {
        return 1; // Stone
    }
}

int Biome::GetSurfaceBlock() const {
    // Default surface blocks by biome
    switch (m_definition.type) {
        case BiomeType::DESERT:
        case BiomeType::DESERT_HILLS:
            return 12; // Sand
        case BiomeType::BEACH:
            return 12; // Sand
        case BiomeType::SNOWY_BEACH:
            return 12; // Sand
        case BiomeType::BADLANDS:
        case BiomeType::BADLANDS_PLATEAU:
            return 12; // Red sand
        default:
            return 2; // Grass
    }
}

int Biome::GetFillerBlock() const {
    // Default filler blocks by biome
    switch (m_definition.type) {
        case BiomeType::DESERT:
        case BiomeType::DESERT_HILLS:
            return 12; // Sand
        case BiomeType::BADLANDS:
        case BiomeType::BADLANDS_PLATEAU:
            return 1; // Stone (actually red sandstone)
        default:
            return 3; // Dirt
    }
}

int Biome::GetUnderwaterBlock() const {
    return 13; // Gravel for most biomes
}

float Biome::GenerateNoiseHeight(int x, int z, int seed) const {
    // Use Perlin noise for height generation
    noise::module::Perlin perlinNoise;
    perlinNoise.SetSeed(seed);
    perlinNoise.SetFrequency(0.01);
    perlinNoise.SetOctaveCount(4);
    perlinNoise.SetPersistence(0.5);

    return static_cast<float>(perlinNoise.GetValue(x * 0.01, 0, z * 0.01));
}

bool Biome::HasSuitableConditions(const glm::vec3& position, World* world) const {
    // Check for biome-specific conditions
    switch (m_definition.type) {
        case BiomeType::OCEAN:
        case BiomeType::DEEP_OCEAN:
            return position.y < 63; // Must be below sea level
        case BiomeType::MOUNTAIN_EDGE:
        case BiomeType::SNOWY_MOUNTAINS:
            return position.y > 80; // Must be at higher elevations
        case BiomeType::RIVER:
            return position.y <= 63 && position.y >= 60; // Near sea level
        default:
            return true; // No special conditions
    }
}

void Biome::ApplyBiomeModifications(const glm::vec3& position, World* world) {
    // Apply biome-specific terrain modifications
    switch (m_definition.type) {
        case BiomeType::DESERT:
            // Add sand dunes
            break;
        case BiomeType::MOUNTAIN_EDGE:
            // Add mountain ridges
            break;
        case BiomeType::SWAMP:
            // Add marshy terrain
            break;
        default:
            break;
    }
}

// PlainsBiome implementation
PlainsBiome::PlainsBiome()
    : Biome(BiomeDefinition{
        BiomeType::PLAINS,
        "plains",
        "Plains",
        "Flat, grassy areas with few trees",
        BiomeTemperature::NEUTRAL,
        BiomeHumidity::NEUTRAL,
        {
            0x91BD59, // Grass color
            0x77AB2F, // Foliage color
            0x3F76E4, // Water color
            0x78A7FF, // Sky color
            0xC0D8FF, // Fog color
            15.0f,
            1.0f
        },
        {
            0.1f, // baseHeight
            0.2f, // heightVariation
            0.8f, // temperature
            0.4f, // humidity
            0.3f, // roughness
            0.2f, // hilliness
            0.0f, // mountainness
            0.0f, // canyonDepth
            0.6f, // erosion
            0.4f, // deposition
            63,   // seaLevel
            true, // hasRivers
            true  // hasLakes
        },
        {
            // Trees: Oak trees, rare
            {{"oak", 0.02f}},
            // Plants: Tall grass
            {{"tall_grass", 0.3f}},
            // Flowers: Various flowers
            {{"dandelion", 0.1f}, {"poppy", 0.1f}},
            // Mushrooms: None
            {},
            // Crops: Wheat (if farmed)
            {},
            // Rocks: None
            {},
            // Ores: Standard distribution
            {{"coal", 0.5f}, {"iron", 0.3f}, {"gold", 0.1f}},
            // Structures: Villages
            {{"village", 0.01f}},
            // Caves: Standard caves
            {{"cave", 1.0f}},
            0.3f, // rainFrequency
            0.0f, // snowFrequency
            0.1f, // stormFrequency
            0.0f  // fogDensity
        },
        {
            // Passive mobs
            {{MobType::CHICKEN, 0.4f}, {MobType::COW, 0.3f}, {MobType::PIG, 0.3f}},
            10,
            // Hostile mobs
            {{MobType::ZOMBIE, 0.5f}, {MobType::SKELETON, 0.3f}, {MobType::CREEPER, 0.2f}},
            70,
            // Neutral mobs
            {},
            5,
            // Water mobs
            {},
            5,
            // Ambient mobs
            {{MobType::BAT, 0.1f}},
            15,
            0, 7, true, false, 1.0f
        }
    }) {
}

// ForestBiome implementation
ForestBiome::ForestBiome()
    : Biome(BiomeDefinition{
        BiomeType::FOREST,
        "forest",
        "Forest",
        "Dense woodlands with many trees",
        BiomeTemperature::NEUTRAL,
        BiomeHumidity::NEUTRAL,
        {
            0x79C05A, // Grass color
            0x59AE30, // Foliage color
            0x3F76E4, // Water color
            0x78A7FF, // Sky color
            0xC0D8FF, // Fog color
            15.0f,
            1.0f
        },
        {
            0.1f, // baseHeight
            0.3f, // heightVariation
            0.7f, // temperature
            0.8f, // humidity
            0.4f, // roughness
            0.3f, // hilliness
            0.0f, // mountainness
            0.0f, // canyonDepth
            0.5f, // erosion
            0.5f, // deposition
            63,   // seaLevel
            true, // hasRivers
            true  // hasLakes
        },
        {
            // Trees: Oak and birch trees, common
            {{"oak", 0.7f}, {"birch", 0.3f}},
            // Plants: Ferns and grass
            {{"fern", 0.2f}, {"tall_grass", 0.4f}},
            // Flowers: Various flowers
            {{"dandelion", 0.1f}, {"poppy", 0.1f}, {"blue_orchid", 0.05f}},
            // Mushrooms: Common
            {{"red_mushroom", 0.1f}, {"brown_mushroom", 0.1f}},
            // Crops: None wild
            {},
            // Rocks: None
            {},
            // Ores: Standard distribution
            {{"coal", 0.5f}, {"iron", 0.3f}, {"gold", 0.1f}},
            // Structures: None specific
            {},
            // Caves: Standard caves
            {{"cave", 1.0f}},
            0.4f, // rainFrequency
            0.0f, // snowFrequency
            0.15f, // stormFrequency
            0.1f  // fogDensity
        },
        {
            // Passive mobs
            {{MobType::CHICKEN, 0.3f}, {MobType::COW, 0.2f}, {MobType::PIG, 0.2f}, {MobType::SHEEP, 0.3f}},
            8,
            // Hostile mobs
            {{MobType::ZOMBIE, 0.4f}, {MobType::SKELETON, 0.3f}, {MobType::CREEPER, 0.2f}, {MobType::SPIDER, 0.1f}},
            70,
            // Neutral mobs
            {{MobType::WOLF, 0.05f}},
            5,
            // Water mobs
            {},
            5,
            // Ambient mobs
            {{MobType::BAT, 0.1f}},
            15,
            0, 7, true, false, 1.0f
        }
    }) {
}

// DesertBiome implementation
DesertBiome::DesertBiome()
    : Biome(BiomeDefinition{
        BiomeType::DESERT,
        "desert",
        "Desert",
        "Arid, sandy areas with little vegetation",
        BiomeTemperature::HOT,
        BiomeHumidity::ARID,
        {
            0xBFB755, // Grass color (rare)
            0xAEA42A, // Foliage color (rare)
            0x3F76E4, // Water color
            0x78A7FF, // Sky color
            0xC0D8FF, // Fog color
            15.0f,
            0.8f
        },
        {
            0.1f, // baseHeight
            0.2f, // heightVariation
            2.0f, // temperature
            0.0f, // humidity
            0.2f, // roughness
            0.1f, // hilliness
            0.0f, // mountainness
            0.0f, // canyonDepth
            0.8f, // erosion
            0.2f, // deposition
            63,   // seaLevel
            false, // hasRivers
            false  // hasLakes
        },
        {
            // Trees: Cactus only
            {{"cactus", 0.05f}},
            // Plants: Dead bush
            {{"dead_bush", 0.1f}},
            // Flowers: None
            {},
            // Mushrooms: None
            {},
            // Crops: None
            {},
            // Rocks: None
            {},
            // Ores: Standard distribution
            {{"coal", 0.5f}, {"iron", 0.3f}, {"gold", 0.1f}},
            // Structures: Desert temples, villages
            {{"desert_temple", 0.008f}, {"desert_village", 0.01f}},
            // Caves: Standard caves
            {{"cave", 1.0f}},
            0.0f, // rainFrequency
            0.0f, // snowFrequency
            0.05f, // stormFrequency
            0.0f  // fogDensity
        },
        {
            // Passive mobs
            {},
            5,
            // Hostile mobs
            {{MobType::ZOMBIE, 0.5f}, {MobType::SKELETON, 0.3f}, {MobType::CREEPER, 0.2f}},
            70,
            // Neutral mobs
            {},
            5,
            // Water mobs
            {},
            5,
            // Ambient mobs
            {},
            15,
            0, 7, true, false, 1.0f
        }
    }) {
}

// OceanBiome implementation
OceanBiome::OceanBiome()
    : Biome(BiomeDefinition{
        BiomeType::OCEAN,
        "ocean",
        "Ocean",
        "Vast bodies of water",
        BiomeTemperature::OCEANIC,
        BiomeHumidity::HUMID,
        {
            0x91BD59, // Grass color (not used)
            0x77AB2F, // Foliage color (not used)
            0x3F76E4, // Water color
            0x78A7FF, // Sky color
            0xC0D8FF, // Fog color
            15.0f,
            1.0f
        },
        {
            -1.0f, // baseHeight (below sea level)
            0.1f,  // heightVariation
            0.5f,  // temperature
            0.5f,  // humidity
            0.5f,  // roughness
            0.0f,  // hilliness
            0.0f,  // mountainness
            0.0f,  // canyonDepth
            0.5f,  // erosion
            0.5f,  // deposition
            63,    // seaLevel
            false, // hasRivers
            false  // hasLakes
        },
        {
            // Trees: Kelp forests
            {{"kelp", 0.3f}},
            // Plants: Seagrass
            {{"seagrass", 0.4f}},
            // Flowers: None
            {},
            // Mushrooms: None
            {},
            // Crops: None
            {},
            // Rocks: None
            {},
            // Ores: Underwater ores
            {{"coal", 0.5f}, {"iron", 0.3f}, {"gold", 0.2f}},
            // Structures: Ocean monuments, shipwrecks
            {{"ocean_monument", 0.001f}, {"shipwreck", 0.01f}},
            // Caves: Underwater caves
            {{"underwater_cave", 1.0f}},
            0.2f, // rainFrequency
            0.0f, // snowFrequency
            0.1f, // stormFrequency
            0.3f  // fogDensity
        },
        {
            // Passive mobs
            {{MobType::COD, 0.3f}, {MobType::SALMON, 0.2f}},
            5,
            // Hostile mobs
            {{MobType::DROWNED, 0.4f}},
            70,
            // Neutral mobs
            {},
            5,
            // Water mobs
            {{MobType::SQUID, 0.5f}, {MobType::TURTLE, 0.1f}},
            15,
            // Ambient mobs
            {},
            15,
            0, 7, true, false, 1.0f
        }
    }) {
}

// TundraBiome implementation
TundraBiome::TundraBiome()
    : Biome(BiomeDefinition{
        BiomeType::SNOWY_TUNDRA,
        "snowy_tundra",
        "Snowy Tundra",
        "Frozen plains covered in snow",
        BiomeTemperature::ICY,
        BiomeHumidity::ARID,
        {
            0x80B497, // Grass color (blue-tinted)
            0x60A17B, // Foliage color (blue-tinted)
            0x3938C9, // Water color (icy)
            0x78A7FF, // Sky color
            0xC0D8FF, // Fog color
            15.0f,
            1.0f
        },
        {
            0.125f, // baseHeight
            0.05f,  // heightVariation
            -0.5f,  // temperature
            0.0f,   // humidity
            0.3f,   // roughness
            0.1f,   // hilliness
            0.0f,   // mountainness
            0.0f,   // canyonDepth
            0.4f,   // erosion
            0.6f,   // deposition
            63,     // seaLevel
            true,   // hasRivers
            true    // hasLakes
        },
        {
            // Trees: None
            {},
            // Plants: None
            {},
            // Flowers: None
            {},
            // Mushrooms: None
            {},
            // Crops: None
            {},
            // Rocks: None
            {},
            // Ores: Standard distribution
            {{"coal", 0.5f}, {"iron", 0.3f}, {"gold", 0.1f}},
            // Structures: Igloos
            {{"igloo", 0.01f}},
            // Caves: Standard caves
            {{"cave", 1.0f}},
            0.1f, // rainFrequency
            0.9f, // snowFrequency
            0.05f, // stormFrequency
            0.0f  // fogDensity
        },
        {
            // Passive mobs
            {{MobType::RABBIT, 0.4f}, {MobType::POLAR_BEAR, 0.05f}},
            10,
            // Hostile mobs
            {{MobType::STRAY, 0.5f}},
            70,
            // Neutral mobs
            {},
            5,
            // Water mobs
            {},
            5,
            // Ambient mobs
            {},
            15,
            0, 7, true, false, 1.0f
        }
    }) {
}

// JungleBiome implementation
JungleBiome::JungleBiome()
    : Biome(BiomeDefinition{
        BiomeType::JUNGLE,
        "jungle",
        "Jungle",
        "Dense tropical forests with diverse wildlife",
        BiomeTemperature::WARM,
        BiomeHumidity::HUMID,
        {
            0x64C73F, // Grass color (bright green)
            0x3EB80F, // Foliage color (bright green)
            0x3F76E4, // Water color
            0x78A7FF, // Sky color
            0xC0D8FF, // Fog color
            15.0f,
            1.0f
        },
        {
            0.1f, // baseHeight
            0.3f, // heightVariation
            0.95f, // temperature
            0.9f,  // humidity
            0.5f,  // roughness
            0.4f,  // hilliness
            0.0f,  // mountainness
            0.0f,  // canyonDepth
            0.4f,  // erosion
            0.6f,  // deposition
            63,    // seaLevel
            true,  // hasRivers
            true   // hasLakes
        },
        {
            // Trees: Jungle trees, very common
            {{"jungle_tree", 0.9f}, {"mega_jungle_tree", 0.1f}},
            // Plants: Various jungle plants
            {{"fern", 0.3f}, {"tall_grass", 0.2f}},
            // Flowers: Various tropical flowers
            {{"orange_tulip", 0.05f}, {"red_tulip", 0.05f}, {"white_tulip", 0.05f}},
            // Mushrooms: None
            {},
            // Crops: Melon, pumpkin
            {{"melon", 0.01f}, {"pumpkin", 0.01f}},
            // Rocks: None
            {},
            // Ores: Standard distribution
            {{"coal", 0.5f}, {"iron", 0.3f}, {"gold", 0.1f}},
            // Structures: Jungle temples
            {{"jungle_temple", 0.008f}},
            // Caves: Standard caves
            {{"cave", 1.0f}},
            0.8f, // rainFrequency
            0.0f, // snowFrequency
            0.3f, // stormFrequency
            0.2f  // fogDensity
        },
        {
            // Passive mobs
            {{MobType::CHICKEN, 0.2f}, {MobType::COW, 0.1f}, {MobType::PIG, 0.2f}, {MobType::SHEEP, 0.1f}},
            8,
            // Hostile mobs
            {{MobType::ZOMBIE, 0.3f}, {MobType::SKELETON, 0.2f}, {MobType::CREEPER, 0.2f}, {MobType::SPIDER, 0.2f}},
            70,
            // Neutral mobs
            {{MobType::OCELOT, 0.2f}, {MobType::PARROT, 0.1f}},
            10,
            // Water mobs
            {},
            5,
            // Ambient mobs
            {{MobType::BAT, 0.1f}},
            15,
            0, 7, true, false, 1.5f
        }
    }) {
}

// SwampBiome implementation
SwampBiome::SwampBiome()
    : Biome(BiomeDefinition{
        BiomeType::SWAMP,
        "swamp",
        "Swamp",
        "Wet, marshy areas with water pools",
        BiomeTemperature::NEUTRAL,
        BiomeHumidity::WET,
        {
            0x6A7039, // Grass color (swamp green)
            0x4C763C, // Foliage color (swamp green)
            0x617B64, // Water color (swampy)
            0x78A7FF, // Sky color
            0xC0D8FF, // Fog color
            15.0f,
            1.0f
        },
        {
            -0.2f, // baseHeight (slightly below sea level)
            0.1f,  // heightVariation
            0.8f,  // temperature
            0.9f,  // humidity
            0.4f,  // roughness
            0.2f,  // hilliness
            0.0f,  // mountainness
            0.0f,  // canyonDepth
            0.3f,  // erosion
            0.7f,  // deposition
            63,    // seaLevel
            true,  // hasRivers
            true   // hasLakes
        },
        {
            // Trees: Oak trees, less common
            {{"oak", 0.2f}},
            // Plants: Reeds, lily pads
            {{"sugar_cane", 0.3f}, {"lily_pad", 0.2f}},
            // Flowers: Blue orchids
            {{"blue_orchid", 0.1f}},
            // Mushrooms: Common
            {{"red_mushroom", 0.2f}, {"brown_mushroom", 0.2f}},
            // Crops: None
            {},
            // Rocks: None
            {},
            // Ores: Standard distribution
            {{"coal", 0.5f}, {"iron", 0.3f}, {"gold", 0.1f}},
            // Structures: Swamp huts
            {{"swamp_hut", 0.01f}},
            // Caves: Standard caves
            {{"cave", 1.0f}},
            0.6f, // rainFrequency
            0.0f, // snowFrequency
            0.2f, // stormFrequency
            0.4f  // fogDensity
        },
        {
            // Passive mobs
            {},
            5,
            // Hostile mobs
            {{MobType::ZOMBIE, 0.3f}, {MobType::SKELETON, 0.2f}, {MobType::SPIDER, 0.3f}},
            70,
            // Neutral mobs
            {},
            5,
            // Water mobs
            {{MobType::SLIME, 0.5f}},
            10,
            // Ambient mobs
            {{MobType::BAT, 0.2f}},
            15,
            0, 7, true, false, 1.2f
        }
    }) {
}

// SavannaBiome implementation
SavannaBiome::SavannaBiome()
    : Biome(BiomeDefinition{
        BiomeType::SAVANNA,
        "savanna",
        "Savanna",
        "Tropical grasslands with acacia trees",
        BiomeTemperature::WARM,
        BiomeHumidity::DRY,
        {
            0xBFB755, // Grass color (yellowish)
            0xAEA42A, // Foliage color (yellowish)
            0x3F76E4, // Water color
            0x78A7FF, // Sky color
            0xC0D8FF, // Fog color
            15.0f,
            1.0f
        },
        {
            0.125f, // baseHeight
            0.05f,  // heightVariation
            1.2f,   // temperature
            0.0f,   // humidity
            0.4f,   // roughness
            0.2f,   // hilliness
            0.0f,   // mountainness
            0.0f,   // canyonDepth
            0.6f,   // erosion
            0.4f,   // deposition
            63,     // seaLevel
            true,   // hasRivers
            false   // hasLakes
        },
        {
            // Trees: Acacia trees, sparse
            {{"acacia", 0.1f}},
            // Plants: Tall grass
            {{"tall_grass", 0.5f}},
            // Flowers: None specific
            {},
            // Mushrooms: None
            {},
            // Crops: None
            {},
            // Rocks: None
            {},
            // Ores: Standard distribution
            {{"coal", 0.5f}, {"iron", 0.3f}, {"gold", 0.1f}},
            // Structures: None specific
            {},
            // Caves: Standard caves
            {{"cave", 1.0f}},
            0.0f, // rainFrequency
            0.0f, // snowFrequency
            0.1f, // stormFrequency
            0.0f  // fogDensity
        },
        {
            // Passive mobs
            {{MobType::HORSE, 0.3f}, {MobType::LLAMA, 0.2f}, {MobType::COW, 0.2f}},
            10,
            // Hostile mobs
            {{MobType::ZOMBIE, 0.4f}, {MobType::SKELETON, 0.3f}, {MobType::CREEPER, 0.3f}},
            70,
            // Neutral mobs
            {},
            5,
            // Water mobs
            {},
            5,
            // Ambient mobs
            {},
            15,
            0, 7, true, false, 1.0f
        }
    }) {
}

// TaigaBiome implementation
TaigaBiome::TaigaBiome()
    : Biome(BiomeDefinition{
        BiomeType::TAIGA,
        "taiga",
        "Taiga",
        "Snowy coniferous forests",
        BiomeTemperature::COLD,
        BiomeHumidity::NEUTRAL,
        {
            0x7BA331, // Grass color (pine green)
            0x68A55F, // Foliage color (pine green)
            0x3F76E4, // Water color
            0x78A7FF, // Sky color
            0xC0D8FF, // Fog color
            15.0f,
            1.0f
        },
        {
            0.2f, // baseHeight
            0.2f, // heightVariation
            0.25f, // temperature
            0.8f,  // humidity
            0.4f,  // roughness
            0.3f,  // hilliness
            0.0f,  // mountainness
            0.0f,  // canyonDepth
            0.5f,  // erosion
            0.5f,  // deposition
            63,    // seaLevel
            true,  // hasRivers
            true   // hasLakes
        },
        {
            // Trees: Spruce trees, common
            {{"spruce", 0.8f}},
            // Plants: Ferns
            {{"fern", 0.3f}},
            // Flowers: None
            {},
            // Mushrooms: Common
            {{"red_mushroom", 0.1f}, {"brown_mushroom", 0.1f}},
            // Crops: None
            {},
            // Rocks: None
            {},
            // Ores: Standard distribution
            {{"coal", 0.5f}, {"iron", 0.3f}, {"gold", 0.1f}},
            // Structures: None specific
            {},
            // Caves: Standard caves
            {{"cave", 1.0f}},
            0.3f, // rainFrequency
            0.4f, // snowFrequency
            0.1f, // stormFrequency
            0.1f  // fogDensity
        },
        {
            // Passive mobs
            {{MobType::CHICKEN, 0.2f}, {MobType::COW, 0.1f}, {MobType::PIG, 0.2f}, {MobType::SHEEP, 0.2f}},
            8,
            // Hostile mobs
            {{MobType::ZOMBIE, 0.4f}, {MobType::SKELETON, 0.3f}, {MobType::CREEPER, 0.2f}, {MobType::SPIDER, 0.1f}},
            70,
            // Neutral mobs
            {{MobType::WOLF, 0.1f}},
            5,
            // Water mobs
            {},
            5,
            // Ambient mobs
            {{MobType::BAT, 0.1f}},
            15,
            0, 7, true, false, 1.0f
        }
    }) {
}

// MountainBiome implementation
MountainBiome::MountainBiome()
    : Biome(BiomeDefinition{
        BiomeType::SNOWY_MOUNTAINS,
        "snowy_mountains",
        "Snowy Mountains",
        "High mountain peaks covered in snow",
        BiomeTemperature::ICY,
        BiomeHumidity::NEUTRAL,
        {
            0x80B497, // Grass color (blue-tinted)
            0x60A17B, // Foliage color (blue-tinted)
            0x3938C9, // Water color (icy)
            0x78A7FF, // Sky color
            0xC0D8FF, // Fog color
            15.0f,
            1.0f
        },
        {
            1.0f, // baseHeight
            1.2f, // heightVariation
            -0.3f, // temperature
            0.4f,  // humidity
            0.8f,  // roughness
            0.8f,  // hilliness
            0.8f,  // mountainness
            0.0f,  // canyonDepth
            0.2f,  // erosion
            0.8f,  // deposition
            63,    // seaLevel
            true,  // hasRivers
            true   // hasLakes
        },
        {
            // Trees: Spruce trees at lower levels
            {{"spruce", 0.3f}},
            // Plants: None
            {},
            // Flowers: None
            {},
            // Mushrooms: None
            {},
            // Crops: None
            {},
            // Rocks: Stone formations
            {{"stone", 0.2f}},
            // Ores: Rich ore distribution
            {{"coal", 0.6f}, {"iron", 0.4f}, {"gold", 0.2f}, {"diamond", 0.1f}},
            // Structures: None specific
            {},
            // Caves: Mountain caves
            {{"cave", 1.0f}},
            0.2f, // rainFrequency
            0.8f, // snowFrequency
            0.2f, // stormFrequency
            0.1f  // fogDensity
        },
        {
            // Passive mobs
            {{MobType::GOAT, 0.2f}},
            5,
            // Hostile mobs
            {{MobType::STRAY, 0.6f}},
            70,
            // Neutral mobs
            {},
            5,
            // Water mobs
            {},
            5,
            // Ambient mobs
            {},
            15,
            0, 7, true, false, 0.8f
        }
    }) {
}

} // namespace VoxelCraft
