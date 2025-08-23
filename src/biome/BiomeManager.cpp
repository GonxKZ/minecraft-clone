/**
 * @file BiomeManager.cpp
 * @brief VoxelCraft Biome Manager Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#include "BiomeManager.hpp"
#include "../world/World.hpp"
#include "../player/Player.hpp"
#include <algorithm>
#include <random>
#include <cmath>
#include <noise/noise.h>

namespace VoxelCraft {

// BiomeManager implementation
BiomeManager& BiomeManager::GetInstance() {
    static BiomeManager instance;
    return instance;
}

bool BiomeManager::Initialize(World* world) {
    if (!world) return false;

    m_world = world;
    m_initialized = true;

    // Initialize random engine
    std::random_device rd;
    m_randomEngine.seed(rd());

    // Initialize default configuration
    m_config.mode = BiomeGenerationMode::VANILLA;
    m_config.biomeSize = 4; // 4 chunks per biome
    m_config.regionSize = 6; // 6x6 biome regions
    m_config.temperatureNoiseScale = 0.01f;
    m_config.humidityNoiseScale = 0.01f;
    m_config.continentalnessScale = 0.001f;
    m_config.erosionScale = 0.01f;
    m_config.weirdnessScale = 0.01f;
    m_config.enableTransitions = true;
    m_config.enableRivers = true;
    m_config.enableBeaches = true;
    m_config.enableOceans = true;
    m_config.seaLevel = 63;
    m_config.riverThreshold = 0.02f;
    m_config.beachThreshold = 0.1f;
    m_config.minOceanSize = 100;
    m_config.maxOceanSize = 1000;

    // Initialize cache
    m_cache.cacheRadius = 8; // 8 chunk radius

    // Initialize components
    InitializeDefaultBiomes();
    InitializeDefaultTransitions();
    InitializeNoiseLayers();
    GenerateBiomeRegions();

    return true;
}

void BiomeManager::Shutdown() {
    m_biomes.clear();
    m_transitions.clear();
    m_regions.clear();
    m_noiseLayers.clear();
    m_cache.Clear();
    m_world = nullptr;
    m_initialized = false;
}

void BiomeManager::Update(float deltaTime) {
    if (!m_initialized || !m_world) return;

    // Update biome cache around players
    UpdateBiomeCache();

    // Update statistics
    m_stats.cachedChunks = m_cache.biomeMap.size();
}

std::shared_ptr<Biome> BiomeManager::GetBiomeAt(const glm::vec3& position) const {
    BiomeType type = GetBiomeTypeAt(position);
    return GetBiome(type);
}

BiomeType BiomeManager::GetBiomeTypeAt(const glm::vec3& position) const {
    glm::ivec2 chunkPos(
        static_cast<int>(position.x) >> 4,
        static_cast<int>(position.z) >> 4
    );

    // Check cache first
    auto it = m_cache.biomeMap.find(chunkPos);
    if (it != m_cache.biomeMap.end()) {
        return it->second;
    }

    // Generate biome if not cached
    return GenerateBiomeAtPosition(position);
}

std::vector<std::shared_ptr<Biome>> BiomeManager::GetAllBiomes() const {
    std::vector<std::shared_ptr<Biome>> biomes;
    biomes.reserve(m_biomes.size());

    for (const auto& pair : m_biomes) {
        biomes.push_back(pair.second);
    }

    return biomes;
}

std::shared_ptr<Biome> BiomeManager::GetBiome(BiomeType type) const {
    auto it = m_biomes.find(type);
    return (it != m_biomes.end()) ? it->second : nullptr;
}

int BiomeManager::GenerateTerrain(const glm::vec3& position) {
    auto biome = GetBiomeAt(position);
    if (!biome) return m_config.seaLevel;

    return biome->GenerateTerrain(glm::ivec3(position), m_world);
}

std::unordered_map<glm::ivec2, BiomeType> BiomeManager::GenerateChunkBiomes(int chunkX, int chunkZ) {
    std::unordered_map<glm::ivec2, BiomeType> chunkBiomes;

    // Generate biomes for the chunk and surrounding area
    for (int x = -1; x <= 1; ++x) {
        for (int z = -1; z <= 1; ++z) {
            glm::ivec2 pos(chunkX + x, chunkZ + z);
            glm::vec3 worldPos(
                static_cast<float>(pos.x * 16 + 8),
                0.0f,
                static_cast<float>(pos.y * 16 + 8)
            );

            BiomeType biome = GenerateBiomeAtPosition(worldPos);

            // Apply transitions if enabled
            if (m_config.enableTransitions) {
                biome = ApplyBiomeTransitions(worldPos, biome);
            }

            chunkBiomes[pos] = biome;

            // Cache the result
            CacheBiomeData(pos, biome, GenerateTerrain(worldPos));
        }
    }

    m_stats.generatedChunks++;
    return chunkBiomes;
}

void BiomeManager::GenerateBiomeFeatures(const glm::vec3& position, std::shared_ptr<Biome> biome) {
    if (!biome) return;

    biome->GenerateFeatures(glm::ivec3(position), m_world);
}

void BiomeManager::GenerateBiomeStructures(const glm::vec3& position, std::shared_ptr<Biome> biome) {
    if (!biome) return;

    biome->GenerateStructures(glm::ivec3(position), m_world);
}

float BiomeManager::GetTemperatureAt(const glm::vec3& position) const {
    glm::ivec2 chunkPos(
        static_cast<int>(position.x) >> 4,
        static_cast<int>(position.z) >> 4
    );

    // Check cache first
    auto it = m_cache.temperatureMap.find(chunkPos);
    if (it != m_cache.temperatureMap.end()) {
        return it->second;
    }

    // Generate temperature using noise
    float noise = GenerateNoise(m_noiseLayers[0], position.x, position.z);
    float heightFactor = 1.0f - (position.y / 128.0f); // Higher = colder

    return std::max(0.0f, std::min(1.0f, (noise + 0.5f) * heightFactor));
}

float BiomeManager::GetHumidityAt(const glm::vec3& position) const {
    glm::ivec2 chunkPos(
        static_cast<int>(position.x) >> 4,
        static_cast<int>(position.z) >> 4
    );

    // Check cache first
    auto it = m_cache.humidityMap.find(chunkPos);
    if (it != m_cache.humidityMap.end()) {
        return it->second;
    }

    // Generate humidity using noise
    float noise = GenerateNoise(m_noiseLayers[1], position.x, position.z);
    return std::max(0.0f, std::min(1.0f, noise + 0.5f));
}

BiomeColors BiomeManager::GetBiomeColorsAt(const glm::vec3& position) const {
    auto biome = GetBiomeAt(position);
    if (!biome) return BiomeColors();

    return biome->GetColors();
}

BiomeTransitionType BiomeManager::GetBiomeTransitionAt(const glm::vec3& position) const {
    // Check for biome transitions at this position
    for (const auto& transition : m_transitions) {
        // This would check if position is in a transition zone
        // For now, return NONE
    }

    return BiomeTransitionType::NONE;
}

std::vector<MobType> BiomeManager::GetSpawnableMobsAt(const glm::vec3& position, float timeOfDay) const {
    auto biome = GetBiomeAt(position);
    if (!biome) return {};

    return biome->GetSpawnableMobs(timeOfDay);
}

void BiomeManager::SetGenerationMode(BiomeGenerationMode mode) {
    m_config.mode = mode;

    // Clear cache when changing modes
    ClearBiomeCache();

    // Regenerate regions
    GenerateBiomeRegions();
}

bool BiomeManager::AddCustomBiome(std::shared_ptr<Biome> biome) {
    if (!biome) return false;

    BiomeType type = biome->GetType();
    if (m_biomes.find(type) != m_biomes.end()) return false;

    m_biomes[type] = biome;
    m_stats.totalBiomes++;

    return true;
}

bool BiomeManager::RemoveCustomBiome(BiomeType type) {
    auto it = m_biomes.find(type);
    if (it == m_biomes.end()) return false;

    m_biomes.erase(it);
    m_stats.totalBiomes--;

    return true;
}

bool BiomeManager::AddBiomeTransition(const BiomeTransition& transition) {
    // Check if transition already exists
    for (const auto& existing : m_transitions) {
        if (existing.fromBiome == transition.fromBiome &&
            existing.toBiome == transition.toBiome) {
            return false;
        }
    }

    m_transitions.push_back(transition);
    return true;
}

void BiomeManager::ClearBiomeCache() {
    m_cache.Clear();
}

void BiomeManager::InitializeDefaultBiomes() {
    // Add all default biomes
    AddCustomBiome(std::make_shared<PlainsBiome>());
    AddCustomBiome(std::make_shared<ForestBiome>());
    AddCustomBiome(std::make_shared<DesertBiome>());
    AddCustomBiome(std::make_shared<OceanBiome>());
    AddCustomBiome(std::make_shared<TundraBiome>());
    AddCustomBiome(std::make_shared<JungleBiome>());
    AddCustomBiome(std::make_shared<SwampBiome>());
    AddCustomBiome(std::make_shared<SavannaBiome>());
    AddCustomBiome(std::make_shared<TaigaBiome>());
    AddCustomBiome(std::make_shared<MountainBiome>());
}

void BiomeManager::InitializeDefaultTransitions() {
    // Add common biome transitions
    m_transitions.push_back({
        BiomeType::PLAINS,
        BiomeType::FOREST,
        BiomeTransitionType::SOFT,
        16.0f,
        [](float t) { return t * t * (3.0f - 2.0f * t); }, // Smoothstep
        {}
    });

    m_transitions.push_back({
        BiomeType::PLAINS,
        BiomeType::DESERT,
        BiomeTransitionType::HARD,
        8.0f,
        [](float t) { return t; }, // Linear
        {}
    });

    m_transitions.push_back({
        BiomeType::FOREST,
        BiomeType::TAIGA,
        BiomeTransitionType::BLENDED,
        32.0f,
        [](float t) { return t * t * (3.0f - 2.0f * t); },
        {}
    });
}

void BiomeManager::InitializeNoiseLayers() {
    // Temperature noise layer
    m_noiseLayers.push_back({
        "temperature",
        0.01f,     // frequency
        1.0f,      // amplitude
        2.0f,      // lacunarity
        0.5f,      // persistence
        4,         // octaves
        1000,      // seed
        nullptr    // modifier
    });

    // Humidity noise layer
    m_noiseLayers.push_back({
        "humidity",
        0.01f,
        1.0f,
        2.0f,
        0.5f,
        4,
        2000,
        nullptr
    });

    // Continentalness noise layer
    m_noiseLayers.push_back({
        "continentalness",
        0.001f,
        1.0f,
        2.0f,
        0.5f,
        4,
        3000,
        nullptr
    });

    // Erosion noise layer
    m_noiseLayers.push_back({
        "erosion",
        0.01f,
        1.0f,
        2.0f,
        0.5f,
        4,
        4000,
        nullptr
    });

    // Weirdness noise layer
    m_noiseLayers.push_back({
        "weirdness",
        0.01f,
        1.0f,
        2.0f,
        0.5f,
        4,
        5000,
        nullptr
    });
}

void BiomeManager::GenerateBiomeRegions() {
    m_regions.clear();

    // Generate biome regions based on generation mode
    switch (m_config.mode) {
        case BiomeGenerationMode::VANILLA:
            GenerateVanillaRegions();
            break;
        case BiomeGenerationMode::REALISTIC:
            GenerateRealisticRegions();
            break;
        case BiomeGenerationMode::ISLAND:
            GenerateIslandRegions();
            break;
        case BiomeGenerationMode::CONTINENTAL:
            GenerateContinentalRegions();
            break;
        default:
            GenerateVanillaRegions();
            break;
    }
}

void BiomeManager::GenerateVanillaRegions() {
    // Create regions with vanilla-style biome distribution
    // This would create regions based on Minecraft's biome distribution patterns
}

void BiomeManager::GenerateRealisticRegions() {
    // Create regions with more realistic biome distribution
    // This would follow real-world climate patterns
}

void BiomeManager::GenerateIslandRegions() {
    // Create island-style biome distribution
}

void BiomeManager::GenerateContinentalRegions() {
    // Create large continental biomes
}

BiomeType BiomeManager::GenerateBiomeAtPosition(const glm::vec3& position) const {
    // Use noise to determine biome type
    float temperature = GenerateNoise(m_noiseLayers[0], position.x, position.z);
    float humidity = GenerateNoise(m_noiseLayers[1], position.x, position.z);
    float continentalness = GenerateContinentalness(position.x, position.z);
    float erosion = GenerateErosion(position.x, position.z);
    float weirdness = GenerateWeirdness(position.x, position.z);

    // Apply height-based modifications
    float heightFactor = position.y / 128.0f;
    temperature -= heightFactor * 0.5f;
    humidity += heightFactor * 0.2f;

    // Determine biome based on parameters
    if (continentalness < 0.3f) {
        // Ocean
        if (temperature < 0.2f) {
            return BiomeType::FROZEN_OCEAN;
        } else if (temperature > 0.8f) {
            return BiomeType::WARM_OCEAN;
        } else {
            return BiomeType::OCEAN;
        }
    } else if (ShouldBeRiver(position)) {
        return BiomeType::RIVER;
    } else if (ShouldBeBeach(position, GenerateBiomeAtPosition(glm::vec3(position.x, 0, position.z)))) {
        return BiomeType::BEACH;
    } else {
        // Land biomes
        if (temperature < 0.1f) {
            // Icy biomes
            if (humidity > 0.7f) {
                return BiomeType::ICE_SPIKES;
            } else {
                return BiomeType::SNOWY_TUNDRA;
            }
        } else if (temperature < 0.3f) {
            // Cold biomes
            if (humidity > 0.6f) {
                return BiomeType::TAIGA;
            } else if (humidity < 0.3f) {
                return BiomeType::SNOWY_TUNDRA;
            } else {
                return BiomeType::SNOWY_TAIGA;
            }
        } else if (temperature < 0.7f) {
            // Temperate biomes
            if (humidity > 0.7f) {
                return BiomeType::FOREST;
            } else if (humidity < 0.3f) {
                return BiomeType::PLAINS;
            } else {
                return BiomeType::BIRCH_FOREST;
            }
        } else if (temperature < 0.9f) {
            // Warm biomes
            if (humidity > 0.6f) {
                return BiomeType::JUNGLE;
            } else if (humidity < 0.3f) {
                return BiomeType::SAVANNA;
            } else {
                return BiomeType::SWAMP;
            }
        } else {
            // Hot biomes
            if (humidity < 0.2f) {
                return BiomeType::DESERT;
            } else if (humidity > 0.8f) {
                return BiomeType::JUNGLE;
            } else {
                return BiomeType::SAVANNA;
            }
        }
    }
}

std::unordered_map<BiomeType, float> BiomeManager::CalculateBiomeWeights(const glm::vec3& position) const {
    std::unordered_map<BiomeType, float> weights;

    // Calculate weights for nearby biomes
    for (const auto& pair : m_biomes) {
        float weight = 1.0f;
        weights[pair.first] = weight;
    }

    return weights;
}

BiomeType BiomeManager::SelectBiomeFromWeights(const std::unordered_map<BiomeType, float>& weights) const {
    if (weights.empty()) return BiomeType::PLAINS;

    float totalWeight = 0.0f;
    for (const auto& pair : weights) {
        totalWeight += pair.second;
    }

    std::uniform_real_distribution<float> dist(0.0f, totalWeight);
    float randomValue = dist(m_randomEngine);

    float currentWeight = 0.0f;
    for (const auto& pair : weights) {
        currentWeight += pair.second;
        if (randomValue <= currentWeight) {
            return pair.first;
        }
    }

    return BiomeType::PLAINS; // Fallback
}

BiomeType BiomeManager::ApplyBiomeTransitions(const glm::vec3& position, BiomeType biome) const {
    // Apply biome transitions if enabled
    if (!m_config.enableTransitions) return biome;

    for (const auto& transition : m_transitions) {
        if (transition.fromBiome == biome || transition.toBiome == biome) {
            // Check if position is in transition zone
            // This would calculate transition based on position and biome boundaries
        }
    }

    return biome;
}

float BiomeManager::GenerateNoise(const BiomeNoiseLayer& layer, float x, float z) const {
    // Use Perlin noise for generation
    noise::module::Perlin perlinNoise;
    perlinNoise.SetSeed(static_cast<int>(layer.seed));
    perlinNoise.SetFrequency(layer.frequency);
    perlinNoise.SetOctaveCount(layer.octaves);
    perlinNoise.SetPersistence(layer.persistence);
    perlinNoise.SetLacunarity(layer.lacunarity);

    return static_cast<float>(perlinNoise.GetValue(x * layer.frequency, 0, z * layer.frequency));
}

float BiomeManager::GenerateContinentalness(float x, float z) const {
    // Continentalness determines land vs ocean
    return GenerateNoise(m_noiseLayers[2], x, z);
}

float BiomeManager::GenerateErosion(float x, float z) const {
    // Erosion affects terrain shape
    return GenerateNoise(m_noiseLayers[3], x, z);
}

float BiomeManager::GenerateWeirdness(float x, float z) const {
    // Weirdness adds unusual features
    return GenerateNoise(m_noiseLayers[4], x, z);
}

bool BiomeManager::ShouldBeOcean(const glm::vec3& position) const {
    float continentalness = GenerateContinentalness(position.x, position.z);
    return continentalness < m_config.riverThreshold;
}

bool BiomeManager::ShouldBeRiver(const glm::vec3& position) const {
    if (!m_config.enableRivers) return false;

    float continentalness = GenerateContinentalness(position.x, position.z);
    float erosion = GenerateErosion(position.x, position.z);

    return continentalness > m_config.riverThreshold &&
           erosion > 0.1f &&
           std::abs(erosion) < m_config.beachThreshold;
}

bool BiomeManager::ShouldBeBeach(const glm::vec3& position, BiomeType biome) const {
    if (!m_config.enableBeaches) return false;

    // Check if position is near ocean and not too high
    bool nearOcean = false; // This would check nearby biomes
    bool lowElevation = position.y < m_config.seaLevel + 5;

    return nearOcean && lowElevation;
}

void BiomeManager::CacheBiomeData(const glm::ivec2& position, BiomeType biome, int height) {
    m_cache.biomeMap[position] = biome;
    m_cache.heightMap[position] = height;
    m_cache.temperatureMap[position] = GetTemperatureAt(glm::vec3(position.x * 16, 0, position.y * 16));
    m_cache.humidityMap[position] = GetHumidityAt(glm::vec3(position.x * 16, 0, position.y * 16));
    m_cache.lastUpdate = std::chrono::steady_clock::now();
}

void BiomeManager::UpdateBiomeCache() {
    // Update cache around player positions
    // This would get player positions and cache biomes around them
}

BiomeType BiomeManager::GenerateSmoothTransition(BiomeType fromBiome, BiomeType toBiome,
                                               float distance, float maxDistance) const {
    float t = distance / maxDistance;

    // Use transition blend function
    for (const auto& transition : m_transitions) {
        if (transition.fromBiome == fromBiome && transition.toBiome == toBiome) {
            t = transition.blendFunction(t);

            if (t < 0.5f) {
                return fromBiome;
            } else {
                return toBiome;
            }
        }
    }

    // Default linear transition
    return (t < 0.5f) ? fromBiome : toBiome;
}

} // namespace VoxelCraft
