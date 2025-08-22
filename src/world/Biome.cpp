/**
 * @file Biome.cpp
 * @brief VoxelCraft World System - Biome Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#include "Biome.hpp"
#include <algorithm>
#include <cmath>
#include <random>

namespace VoxelCraft {

    Biome::Biome(BiomeType type, const BiomeProperties& properties)
        : m_type(type)
        , m_properties(properties)
    {
    }

    float Biome::GetTemperature(int x, int z) const {
        // Simple temperature variation based on position
        float baseTemp = m_properties.temperature;

        // Add some noise-based variation
        int noise = (x * 374761393 + z * 668265263) ^ 0x5F2C3B1A;
        noise = (noise << 13) ^ noise;
        float variation = ((noise * (noise * noise * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f;
        variation = (variation + 1.0f) * 0.5f; // 0-1 range

        return baseTemp + (variation - 0.5f) * 0.2f; // ±0.1 variation
    }

    float Biome::GetHumidity(int x, int z) const {
        // Simple humidity variation based on position
        float baseHumidity = m_properties.humidity;

        // Add some noise-based variation
        int noise = (x * 846184263 + z * 284759273) ^ 0x3A7B4C2D;
        noise = (noise << 13) ^ noise;
        float variation = ((noise * (noise * noise * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f;
        variation = (variation + 1.0f) * 0.5f; // 0-1 range

        return baseHumidity + (variation - 0.5f) * 0.1f; // ±0.05 variation
    }

    std::string Biome::GetRandomMob(const std::string& category) const {
        const std::vector<std::string>* mobList = nullptr;

        if (category == "ambient") {
            mobList = &m_properties.ambientMobs;
        } else if (category == "passive") {
            mobList = &m_properties.passiveMobs;
        } else if (category == "hostile") {
            mobList = &m_properties.hostileMobs;
        } else if (category == "water") {
            mobList = &m_properties.waterMobs;
        }

        if (mobList && !mobList->empty()) {
            std::mt19937 rng(std::random_device{}());
            std::uniform_int_distribution<size_t> dist(0, mobList->size() - 1);
            return (*mobList)[dist(rng)];
        }

        return "";
    }

    bool Biome::CanGenerateFeature(const std::string& feature) const {
        if (feature == "trees") return m_properties.canGenerateTrees;
        if (feature == "caves") return m_properties.canGenerateCaves;
        if (feature == "lakes") return m_properties.canGenerateLakes;
        if (feature == "ores") return m_properties.canGenerateOres;
        if (feature == "villages") return m_properties.canGenerateVillages;
        if (feature == "temples") return m_properties.canGenerateTemples;
        return false;
    }

    uint32_t Biome::GetMapColor() const {
        // Return a representative color for each biome
        switch (m_type) {
            case BiomeType::PLAINS: return 0xFF7CBD6B;     // Light green
            case BiomeType::FOREST: return 0xFF056621;     // Dark green
            case BiomeType::DARK_FOREST: return 0xFF2D5016; // Very dark green
            case BiomeType::BIRCH_FOREST: return 0xFF6BA353; // Pale green
            case BiomeType::JUNGLE: return 0xFF537B09;     // Bright green
            case BiomeType::TAIGA: return 0xFF5E5224;      // Brown-green
            case BiomeType::SNOWY_TAIGA: return 0xFF8B8680; // Gray
            case BiomeType::MOUNTAINS: return 0xFF8B8B8B;  // Gray
            case BiomeType::SNOWY_MOUNTAINS: return 0xFFFFFF; // White
            case BiomeType::DESERT: return 0xFFF2B632;     // Yellow
            case BiomeType::SAVANNA: return 0xFF9B7E3A;    // Brown-yellow
            case BiomeType::MESA: return 0xFFD94515;       // Red
            case BiomeType::SWAMP: return 0xFF4C763C;      // Dark green
            case BiomeType::BEACH: return 0xFFF7E9A3;      // Sand yellow
            case BiomeType::SNOWY_BEACH: return 0xFFF0F0F0; // Light gray
            case BiomeType::OCEAN: return 0xFF3F76E4;      // Blue
            case BiomeType::DEEP_OCEAN: return 0xFF2D5AA0; // Dark blue
            case BiomeType::RIVER: return 0xFF4569CC;      // Light blue
            case BiomeType::FROZEN_RIVER: return 0xFF8BB9DD; // Ice blue
            case BiomeType::FROZEN_OCEAN: return 0xFF7DA3C0; // Ice blue
            case BiomeType::NETHER_WASTES: return 0xFF8B1A1A; // Red
            case BiomeType::CRIMSON_FOREST: return 0xFF8B0000; // Dark red
            case BiomeType::WARPED_FOREST: return 0xFF301934; // Purple
            case BiomeType::SOUL_SAND_VALLEY: return 0xFF5C4033; // Brown
            case BiomeType::BASALT_DELTAS: return 0xFF403A3A; // Dark gray
            case BiomeType::END_HIGHLANDS: return 0xFF8080FF; // Purple
            case BiomeType::END_MIDLANDS: return 0xFF7070FF; // Light purple
            case BiomeType::END_BARRENS: return 0xFF6060FF; // Lighter purple
            case BiomeType::SMALL_END_ISLANDS: return 0xFF5050FF; // Even lighter
            case BiomeType::THE_VOID: return 0xFF000000;    // Black
            default: return 0xFF7CBD6B; // Default green
        }
    }

    std::shared_ptr<Biome> Biome::CreateBiome(BiomeType type) {
        BiomeProperties props;

        switch (type) {
            case BiomeType::PLAINS:
                props.name = "Plains";
                props.description = "A flat and grassy biome with rolling hills and occasional trees";
                props.dimension = Dimension::OVERWORLD;
                props.climate = Climate::TEMPERATE;
                props.type = BiomeType::PLAINS;
                props.temperature = 0.8f;
                props.humidity = 0.4f;
                props.downfall = 0.4f;
                props.depth = 0.125f;
                props.scale = 0.05f;
                props.skyColor = 0xFF87CEEB;
                props.fogColor = 0xFFC0D8FF;
                props.waterColor = 0xFF3F76E4;
                props.grassColor = 0xFF7CBD6B;
                props.foliageColor = 0xFF48B518;
                props.surfaceBlock = BlockType::GRASS_BLOCK;
                props.subsurfaceBlock = BlockType::DIRT;
                props.underwaterBlock = BlockType::SAND;
                props.stoneBlock = BlockType::STONE;
                props.treeDensity = 0.02f;
                props.grassDensity = 0.4f;
                props.flowerDensity = 0.1f;
                props.passiveMobs = {"cow", "pig", "chicken", "sheep", "horse"};
                props.hostileMobs = {"zombie", "skeleton", "spider", "creeper"};
                props.canGenerateTrees = true;
                props.canGenerateVillages = true;
                props.musicTrack = "plains";
                props.ambientSounds = {"ambient.cave.cave", "ambient.weather.rain"};
                break;

            case BiomeType::FOREST:
                props.name = "Forest";
                props.description = "A biome densely populated with oak and birch trees";
                props.dimension = Dimension::OVERWORLD;
                props.climate = Climate::TEMPERATE;
                props.type = BiomeType::FOREST;
                props.temperature = 0.7f;
                props.humidity = 0.8f;
                props.downfall = 0.8f;
                props.depth = 0.1f;
                props.scale = 0.2f;
                props.skyColor = 0xFF87CEEB;
                props.fogColor = 0xFFC0D8FF;
                props.waterColor = 0xFF3F76E4;
                props.grassColor = 0xFF056621;
                props.foliageColor = 0xFF2D5016;
                props.surfaceBlock = BlockType::GRASS_BLOCK;
                props.subsurfaceBlock = BlockType::DIRT;
                props.underwaterBlock = BlockType::SAND;
                props.stoneBlock = BlockType::STONE;
                props.treeDensity = 0.4f;
                props.grassDensity = 0.3f;
                props.flowerDensity = 0.05f;
                props.passiveMobs = {"cow", "pig", "chicken", "sheep", "wolf"};
                props.hostileMobs = {"zombie", "skeleton", "spider", "creeper", "witch"};
                props.canGenerateTrees = true;
                props.musicTrack = "forest";
                break;

            case BiomeType::DESERT:
                props.name = "Desert";
                props.description = "A barren and arid biome with sand and cacti";
                props.dimension = Dimension::OVERWORLD;
                props.climate = Climate::WARM;
                props.type = BiomeType::DESERT;
                props.temperature = 2.0f;
                props.humidity = 0.0f;
                props.downfall = 0.0f;
                props.depth = 0.1f;
                props.scale = 0.2f;
                props.skyColor = 0xFF87CEEB;
                props.fogColor = 0xFFC0D8FF;
                props.waterColor = 0xFF3F76E4;
                props.grassColor = 0xFF9B7E3A;
                props.foliageColor = 0xFF9B7E3A;
                props.surfaceBlock = BlockType::SAND;
                props.subsurfaceBlock = BlockType::SAND;
                props.underwaterBlock = BlockType::SAND;
                props.stoneBlock = BlockType::STONE;
                props.treeDensity = 0.001f; // Very few trees
                props.grassDensity = 0.0f;  // No grass
                props.flowerDensity = 0.0f; // No flowers
                props.passiveMobs = {"rabbit"};
                props.hostileMobs = {"husk", "zombie", "skeleton", "creeper", "spider"};
                props.canGenerateTrees = false;
                props.canGenerateTemples = true;
                props.musicTrack = "desert";
                break;

            case BiomeType::MOUNTAINS:
                props.name = "Mountains";
                props.description = "A rugged biome with high peaks and steep terrain";
                props.dimension = Dimension::OVERWORLD;
                props.climate = Climate::COOL;
                props.type = BiomeType::MOUNTAINS;
                props.temperature = 0.2f;
                props.humidity = 0.3f;
                props.downfall = 0.3f;
                props.depth = 1.0f;
                props.scale = 1.2f;
                props.skyColor = 0xFF87CEEB;
                props.fogColor = 0xFFC0D8FF;
                props.waterColor = 0xFF3F76E4;
                props.grassColor = 0xFF8B8B8B;
                props.foliageColor = 0xFF8B8B8B;
                props.surfaceBlock = BlockType::GRASS_BLOCK;
                props.subsurfaceBlock = BlockType::DIRT;
                props.underwaterBlock = BlockType::GRAVEL;
                props.stoneBlock = BlockType::STONE;
                props.treeDensity = 0.1f;
                props.grassDensity = 0.2f;
                props.passiveMobs = {"goat", "sheep", "llama"};
                props.hostileMobs = {"zombie", "skeleton", "spider", "creeper"};
                props.canGenerateTrees = true;
                props.musicTrack = "mountains";
                break;

            case BiomeType::OCEAN:
                props.name = "Ocean";
                props.description = "A vast body of water covering most of the world";
                props.dimension = Dimension::OVERWORLD;
                props.climate = Climate::TEMPERATE;
                props.type = BiomeType::OCEAN;
                props.temperature = 0.5f;
                props.humidity = 0.5f;
                props.downfall = 0.5f;
                props.depth = -1.0f;
                props.scale = 0.1f;
                props.skyColor = 0xFF87CEEB;
                props.fogColor = 0xFFC0D8FF;
                props.waterColor = 0xFF3F76E4;
                props.surfaceBlock = BlockType::SAND;
                props.subsurfaceBlock = BlockType::SAND;
                props.underwaterBlock = BlockType::SAND;
                props.stoneBlock = BlockType::STONE;
                props.treeDensity = 0.0f;
                props.grassDensity = 0.0f;
                props.waterMobs = {"squid", "dolphin", "cod", "salmon"};
                props.canGenerateTrees = false;
                props.musicTrack = "ocean";
                break;

            case BiomeType::NETHER_WASTES:
                props.name = "Nether Wastes";
                props.description = "A hellish biome filled with lava and dangerous mobs";
                props.dimension = Dimension::NETHER;
                props.climate = Climate::HOT;
                props.type = BiomeType::NETHER_WASTES;
                props.temperature = 2.0f;
                props.humidity = 0.0f;
                props.downfall = 0.0f;
                props.depth = 0.1f;
                props.scale = 0.2f;
                props.skyColor = 0xFF8B1A1A;
                props.fogColor = 0xFF8B1A1A;
                props.waterColor = 0xFF8B1A1A;
                props.grassColor = 0xFF8B1A1A;
                props.foliageColor = 0xFF8B1A1A;
                props.surfaceBlock = BlockType::NETHERRACK;
                props.subsurfaceBlock = BlockType::NETHERRACK;
                props.stoneBlock = BlockType::NETHERRACK;
                props.treeDensity = 0.0f;
                props.hostileMobs = {"blaze", "ghast", "wither_skeleton", "piglin", "hoglin"};
                props.canGenerateTrees = false;
                props.canGenerateCaves = false;
                props.musicTrack = "nether_wastes";
                break;

            case BiomeType::END_HIGHLANDS:
                props.name = "End Highlands";
                props.description = "The central biome of the End dimension";
                props.dimension = Dimension::END;
                props.climate = Climate::COLD;
                props.type = BiomeType::END_HIGHLANDS;
                props.temperature = 0.0f;
                props.humidity = 0.0f;
                props.downfall = 0.0f;
                props.depth = 0.1f;
                props.scale = 0.2f;
                props.skyColor = 0xFF000000;
                props.fogColor = 0xFF000000;
                props.waterColor = 0xFF000000;
                props.surfaceBlock = BlockType::END_STONE;
                props.subsurfaceBlock = BlockType::END_STONE;
                props.stoneBlock = BlockType::END_STONE;
                props.treeDensity = 0.0f;
                props.hostileMobs = {"enderman"};
                props.canGenerateTrees = false;
                props.musicTrack = "end";
                break;

            default:
                props.name = "Unknown Biome";
                props.description = "An unknown biome";
                break;
        }

        return std::make_shared<Biome>(type, props);
    }

    // BiomeManager Implementation
    BiomeManager::BiomeManager() {
        InitializeDefaultBiomes();
    }

    void BiomeManager::InitializeDefaultBiomes() {
        // Initialize all default biomes
        std::vector<BiomeType> allBiomes = {
            BiomeType::PLAINS, BiomeType::FOREST, BiomeType::DESERT,
            BiomeType::MOUNTAINS, BiomeType::OCEAN, BiomeType::NETHER_WASTES,
            BiomeType::END_HIGHLANDS, BiomeType::TAIGA, BiomeType::SAVANNA,
            BiomeType::JUNGLE, BiomeType::SWAMP, BiomeType::DARK_FOREST,
            BiomeType::BIRCH_FOREST, BiomeType::SNOWY_TAIGA, BiomeType::SNOWY_MOUNTAINS,
            BiomeType::BEACH, BiomeType::SNOWY_BEACH, BiomeType::RIVER,
            BiomeType::FROZEN_RIVER, BiomeType::DEEP_OCEAN, BiomeType::CRIMSON_FOREST,
            BiomeType::WARPED_FOREST, BiomeType::SOUL_SAND_VALLEY, BiomeType::BASALT_DELTAS
        };

        for (auto biomeType : allBiomes) {
            m_biomes[biomeType] = Biome::CreateBiome(biomeType);
        }
    }

    BiomeType BiomeManager::GetBiomeAt(int x, int z, int seed) const {
        return GenerateBiomeNoise(x, z, seed);
    }

    const BiomeProperties& BiomeManager::GetBiomeProperties(BiomeType type) const {
        auto it = m_biomes.find(type);
        if (it != m_biomes.end()) {
            return it->second->GetProperties();
        }

        // Return default properties if biome not found
        static BiomeProperties defaultProps;
        return defaultProps;
    }

    std::shared_ptr<Biome> BiomeManager::GetBiome(BiomeType type) const {
        auto it = m_biomes.find(type);
        if (it != m_biomes.end()) {
            return it->second;
        }
        return nullptr;
    }

    std::vector<BiomeType> BiomeManager::GetBiomesInDimension(Dimension dimension) const {
        std::vector<BiomeType> result;
        for (const auto& pair : m_biomes) {
            if (pair.second->GetDimension() == dimension) {
                result.push_back(pair.first);
            }
        }
        return result;
    }

    void BiomeManager::RegisterBiome(BiomeType type, std::shared_ptr<Biome> biome) {
        m_biomes[type] = biome;
    }

    std::vector<BiomeType> BiomeManager::GenerateBiomeMap(int chunkX, int chunkZ, int seed) const {
        std::vector<BiomeType> biomeMap(16 * 16);

        for (int z = 0; z < 16; ++z) {
            for (int x = 0; x < 16; ++x) {
                int worldX = chunkX * 16 + x;
                int worldZ = chunkZ * 16 + z;
                biomeMap[z * 16 + x] = GetBiomeAt(worldX, worldZ, seed);
            }
        }

        return biomeMap;
    }

    float BiomeManager::GetBiomeTemperature(BiomeType biome, int x, int z) const {
        auto biomePtr = GetBiome(biome);
        if (biomePtr) {
            return biomePtr->GetTemperature(x, z);
        }
        return 0.8f;
    }

    float BiomeManager::GetBiomeHumidity(BiomeType biome, int x, int z) const {
        auto biomePtr = GetBiome(biome);
        if (biomePtr) {
            return biomePtr->GetHumidity(x, z);
        }
        return 0.4f;
    }

    BiomeType BiomeManager::FindBiomeByClimate(float temperature, float humidity, Dimension dimension) const {
        BiomeType bestMatch = BiomeType::PLAINS;
        float bestScore = 999.0f;

        for (const auto& pair : m_biomes) {
            if (pair.second->GetDimension() == dimension) {
                const auto& props = pair.second->GetProperties();
                float tempDiff = std::abs(props.temperature - temperature);
                float humidDiff = std::abs(props.humidity - humidity);
                float score = tempDiff * tempDiff + humidDiff * humidDiff;

                if (score < bestScore) {
                    bestScore = score;
                    bestMatch = pair.first;
                }
            }
        }

        return bestMatch;
    }

    BiomeType BiomeManager::GenerateBiomeNoise(int x, int z, int seed) const {
        // Simple biome generation based on temperature and humidity noise
        std::string cacheKey = std::to_string(x) + "," + std::to_string(z) + "," + std::to_string(seed);
        auto cacheIt = m_noiseCache.find(cacheKey);
        if (cacheIt != m_noiseCache.end()) {
            return static_cast<BiomeType>(static_cast<int>(cacheIt->second));
        }

        // Temperature noise (0-1)
        int tempNoise = (x * 374761393 + z * 668265263 + seed) ^ 0x5F2C3B1A;
        tempNoise = (tempNoise << 13) ^ tempNoise;
        float temperature = ((tempNoise * (tempNoise * tempNoise * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f;
        temperature = (temperature + 1.0f) * 0.5f; // 0-1 range

        // Humidity noise (0-1)
        int humidNoise = (x * 846184263 + z * 284759273 + seed * 2) ^ 0x3A7B4C2D;
        humidNoise = (humidNoise << 13) ^ humidNoise;
        float humidity = ((humidNoise * (humidNoise * humidNoise * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f;
        humidity = (humidity + 1.0f) * 0.5f; // 0-1 range

        // Cache the noise value
        m_noiseCache[cacheKey] = temperature;

        // Find biome based on climate
        return FindBiomeByClimate(temperature, humidity, Dimension::OVERWORLD);
    }

} // namespace VoxelCraft

