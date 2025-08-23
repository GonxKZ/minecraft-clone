/**
 * @file StructureManager.cpp
 * @brief VoxelCraft Structure Manager Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#include "StructureManager.hpp"
#include "../world/World.hpp"
#include "../player/Player.hpp"
#include <algorithm>
#include <random>
#include <cmath>
#include <noise/noise.h>

namespace VoxelCraft {

// StructureManager implementation
StructureManager& StructureManager::GetInstance() {
    static StructureManager instance;
    return instance;
}

bool StructureManager::Initialize(World* world) {
    if (!world) return false;

    m_world = world;
    m_initialized = true;

    // Initialize random engine
    std::random_device rd;
    m_randomEngine.seed(rd());

    // Initialize default configuration
    m_config.mode = StructureGenerationMode::CLASSIC;
    m_config.enableStructureGeneration = true;
    m_config.enableNaturalSpawning = true;
    m_config.enablePlayerTriggered = true;
    m_config.respectBiomeRules = true;
    m_config.respectTerrain = true;
    m_config.allowOverlapping = false;
    m_config.globalSpawnMultiplier = 1.0f;
    m_config.rareStructureMultiplier = 1.0f;
    m_config.epicStructureMultiplier = 1.0f;
    m_config.minDistanceBetweenStructures = 10;
    m_config.maxStructuresPerChunk = 5;
    m_config.structureCheckRadius = 32;
    m_config.generateInCreative = true;
    m_config.generateInSurvival = true;
    m_config.minYLevel = 0;
    m_config.maxYLevel = 255;

    // Initialize cache
    m_cache.cacheRadius = 8; // 8 chunk radius

    // Initialize components
    InitializeDefaultStructures();

    return true;
}

void StructureManager::Shutdown() {
    m_structures.clear();
    m_activeStructures.clear();
    m_cache.Clear();
    m_world = nullptr;
    m_initialized = false;
}

void StructureManager::Update(float deltaTime) {
    if (!m_initialized || !m_world) return;

    // Update cache around players
    // This would get player positions and update cache
    glm::vec3 playerPos(0.0f, 70.0f, 0.0f); // Simplified
    UpdateCacheAroundPlayer(playerPos);

    // Update statistics
    m_stats.cachedStructures = m_cache.GetTotalCount();
}

std::vector<StructureInstance> StructureManager::GenerateChunkStructures(int chunkX, int chunkZ) {
    std::vector<StructureInstance> generatedStructures;

    if (!m_config.enableStructureGeneration) return generatedStructures;

    // Check cache first
    glm::ivec2 chunkPos(chunkX, chunkZ);
    if (m_cache.IsCached(chunkPos)) {
        return m_cache.structureMap[chunkPos];
    }

    // Generate structures for this chunk
    for (const auto& pair : m_structures) {
        StructureType type = pair.first;
        if (ShouldGenerateStructureInChunk(type, chunkX, chunkZ)) {
            GenerateStructureForChunk(chunkX, chunkZ);
        }
    }

    // Cache the results
    std::vector<StructureInstance> chunkStructures;
    for (const auto& structure : m_activeStructures) {
        if (structure.ContainsPosition(glm::vec3(chunkX * 16 + 8, 70, chunkZ * 16 + 8))) {
            chunkStructures.push_back(structure);
        }
    }

    m_cache.structureMap[chunkPos] = chunkStructures;
    m_stats.chunksGenerated++;

    return chunkStructures;
}

StructureInstance* StructureManager::GenerateStructureAt(StructureType type, const glm::ivec3& position) {
    auto structureIt = m_structures.find(type);
    if (structureIt == m_structures.end()) return nullptr;

    auto structure = structureIt->second;
    if (!structure) return nullptr;

    // Check if structure can spawn at position
    if (!structure->CanGenerateAt(position, m_world)) return nullptr;

    // Check for conflicts
    if (!m_config.allowOverlapping && CheckStructureConflicts(position, type)) {
        return nullptr;
    }

    // Generate the structure
    StructureInstance instance = structure->GenerateAt(position, m_world);
    if (!instance.isValid) return nullptr;

    // Add to active structures
    m_activeStructures.push_back(instance);

    // Update cache and statistics
    CacheStructure(instance);
    m_stats.totalStructures++;
    m_stats.structuresByType[type]++;
    m_stats.generatedThisSession++;

    return &m_activeStructures.back();
}

StructureInstance* StructureManager::GetStructureAt(const glm::vec3& position) const {
    auto it = m_cache.positionToStructure.find(glm::ivec3(position));
    if (it != m_cache.positionToStructure.end()) {
        return it->second;
    }

    // Check active structures
    for (auto& structure : m_activeStructures) {
        if (structure.ContainsPosition(position)) {
            return const_cast<StructureInstance*>(&structure);
        }
    }

    return nullptr;
}

std::vector<StructureInstance*> StructureManager::GetStructuresInArea(const glm::vec3& center, float radius) const {
    std::vector<StructureInstance*> structuresInArea;

    for (auto& structure : m_activeStructures) {
        float distance = glm::distance(center, glm::vec3(structure.position));
        if (distance <= radius) {
            structuresInArea.push_back(const_cast<StructureInstance*>(&structure));
        }
    }

    return structuresInArea;
}

std::vector<StructureInstance*> StructureManager::GetStructuresByType(StructureType type) const {
    std::vector<StructureInstance*> structuresOfType;

    for (auto& structure : m_activeStructures) {
        if (structure.type == type) {
            structuresOfType.push_back(const_cast<StructureInstance*>(&structure));
        }
    }

    return structuresOfType;
}

StructureInstance* StructureManager::IsPositionInStructure(const glm::vec3& position) const {
    return GetStructureAt(position);
}

StructureInstance* StructureManager::FindNearestStructure(StructureType type, const glm::vec3& position, float maxDistance) {
    StructureInstance* nearest = nullptr;
    float nearestDistance = maxDistance * maxDistance;

    for (auto& structure : m_activeStructures) {
        if (structure.type == type) {
            float distanceSquared = glm::distance2(position, glm::vec3(structure.position));
            if (distanceSquared < nearestDistance) {
                nearestDistance = distanceSquared;
                nearest = &structure;
            }
        }
    }

    return nearest;
}

bool StructureManager::CanStructureSpawnAt(StructureType type, const glm::vec3& position) const {
    auto structure = GetStructure(type);
    if (!structure) return false;

    return structure->CanGenerateAt(position, m_world);
}

std::shared_ptr<Structure> StructureManager::GetStructure(StructureType type) const {
    auto it = m_structures.find(type);
    return (it != m_structures.end()) ? it->second : nullptr;
}

bool StructureManager::AddCustomStructure(std::shared_ptr<Structure> structure) {
    if (!structure) return false;

    StructureType type = structure->GetType();
    if (m_structures.find(type) != m_structures.end()) return false;

    m_structures[type] = structure;
    m_stats.totalStructures++;

    return true;
}

bool StructureManager::RemoveCustomStructure(StructureType type) {
    auto it = m_structures.find(type);
    if (it == m_structures.end()) return false;

    m_structures.erase(it);
    m_stats.totalStructures--;

    return true;
}

void StructureManager::SetGenerationMode(StructureGenerationMode mode) {
    m_config.mode = mode;
    ClearCache();
}

void StructureManager::ClearCache() {
    m_cache.Clear();
}

StructureInstance* StructureManager::ForceGenerateStructure(StructureType type, const glm::vec3& position) {
    // Bypass normal generation checks for debugging
    auto structureIt = m_structures.find(type);
    if (structureIt == m_structures.end()) return nullptr;

    auto structure = structureIt->second;
    if (!structure) return nullptr;

    StructureInstance instance = structure->GenerateAt(glm::ivec3(position), m_world);
    if (!instance.isValid) return nullptr;

    m_activeStructures.push_back(instance);
    CacheStructure(instance);

    return &m_activeStructures.back();
}

void StructureManager::InitializeDefaultStructures() {
    // Add all default structures
    m_structures[StructureType::VILLAGE] = std::make_shared<VillageStructure>();
    m_structures[StructureType::DUNGEON] = std::make_shared<DungeonStructure>();
    m_structures[StructureType::DESERT_TEMPLE] = std::make_shared<TempleStructure>();
    m_structures[StructureType::FORTRESS] = std::make_shared<FortressStructure>();

    // Add more structures as they are implemented
    // m_structures[StructureType::STRONGHOLD] = std::make_shared<StrongholdStructure>();
    // m_structures[StructureType::MINESHAFT] = std::make_shared<MineshaftStructure>();
    // etc.
}

void StructureManager::GenerateStructureForChunk(int chunkX, int chunkZ) {
    // Find suitable structure type for this chunk
    std::vector<StructureType> possibleStructures;
    for (const auto& pair : m_structures) {
        if (ShouldGenerateStructureInChunk(pair.first, chunkX, chunkZ)) {
            possibleStructures.push_back(pair.first);
        }
    }

    if (possibleStructures.empty()) return;

    // Select random structure type
    std::uniform_int_distribution<size_t> typeDist(0, possibleStructures.size() - 1);
    StructureType selectedType = possibleStructures[typeDist(m_randomEngine)];

    // Find suitable position in chunk
    glm::ivec3 position = FindSuitablePosition(selectedType, chunkX, chunkZ);
    if (position.x == 0 && position.y == 0 && position.z == 0) return; // Invalid position

    // Generate the structure
    GenerateStructureAt(selectedType, position);
}

bool StructureManager::ShouldGenerateStructureInChunk(StructureType type, int chunkX, int chunkZ) const {
    auto structure = GetStructure(type);
    if (!structure) return false;

    const auto& definition = structure->GetDefinition();

    // Check if structure is disabled
    if (std::find(m_config.disabledStructures.begin(), m_config.disabledStructures.end(), type) !=
        m_config.disabledStructures.end()) {
        return false;
    }

    // Check if structure is forced
    if (std::find(m_config.forcedStructures.begin(), m_config.forcedStructures.end(), type) !=
        m_config.forcedStructures.end()) {
        return true;
    }

    // Generate seed for this position
    uint32_t seed = GenerateStructureSeed(chunkX, chunkZ);

    // Use structure's spawn chance
    float spawnChance = definition.spawnChance * ApplyGenerationModifiers(type);

    std::mt19937 gen(seed);
    std::uniform_real_distribution<float> chanceDist(0.0f, 1.0f);

    return chanceDist(gen) <= spawnChance;
}

glm::ivec3 StructureManager::FindSuitablePosition(StructureType type, int chunkX, int chunkZ) const {
    auto structure = GetStructure(type);
    if (!structure) return glm::ivec3(0, 0, 0);

    const auto& definition = structure->GetDefinition();
    glm::ivec3 structureSize = structure->GetSize();

    // Generate random position within chunk
    std::mt19937 gen(GenerateStructureSeed(chunkX, chunkZ));

    std::uniform_int_distribution<int> xDist(chunkX * 16, chunkX * 16 + 15);
    std::uniform_int_distribution<int> zDist(chunkZ * 16, chunkZ * 16 + 15);
    std::uniform_int_distribution<int> yDist(definition.minY, definition.maxY);

    int attempts = 0;
    const int maxAttempts = 50;

    while (attempts < maxAttempts) {
        glm::ivec3 position(xDist(gen), yDist(gen), zDist(gen));

        // Check if position is suitable
        if (ValidateStructurePosition(glm::vec3(position), type)) {
            return position;
        }

        attempts++;
    }

    return glm::ivec3(0, 0, 0); // No suitable position found
}

bool StructureManager::CheckStructureConflicts(const glm::ivec3& position, StructureType type) const {
    auto structure = GetStructure(type);
    if (!structure) return true; // Can't check, assume conflict

    glm::ivec3 size = structure->GetSize();
    glm::ivec3 min = position - size / 2;
    glm::ivec3 max = position + size / 2;

    // Check for conflicts with existing structures
    for (const auto& existing : m_activeStructures) {
        if (existing.type != type) { // Allow same type structures to be close
            auto [existingMin, existingMax] = existing.GetBoundingBox();

            // Check for overlap
            if (min.x <= existingMax.x && max.x >= existingMin.x &&
                min.y <= existingMax.y && max.y >= existingMin.y &&
                min.z <= existingMax.z && max.z >= existingMin.z) {
                return true; // Conflict found
            }
        }
    }

    return false;
}

void StructureManager::CacheStructure(const StructureInstance& structure) {
    // Add to position cache
    glm::ivec3 center = structure.position;
    m_cache.positionToStructure[center] = const_cast<StructureInstance*>(&structure);

    // Add to chunk cache
    int chunkX = center.x >> 4;
    int chunkZ = center.z >> 4;
    glm::ivec2 chunkPos(chunkX, chunkZ);

    m_cache.structureMap[chunkPos].push_back(structure);
    m_cache.structureCounts[structure.type]++;
}

void StructureManager::UpdateCacheAroundPlayer(const glm::vec3& playerPos) {
    // Update cache for chunks around player
    int playerChunkX = static_cast<int>(playerPos.x) >> 4;
    int playerChunkZ = static_cast<int>(playerPos.z) >> 4;

    for (int x = -m_cache.cacheRadius; x <= m_cache.cacheRadius; ++x) {
        for (int z = -m_cache.cacheRadius; z <= m_cache.cacheRadius; ++z) {
            int chunkX = playerChunkX + x;
            int chunkZ = playerChunkZ + z;
            glm::ivec2 chunkPos(chunkX, chunkZ);

            if (!m_cache.IsCached(chunkPos)) {
                // Generate structures for this chunk
                GenerateChunkStructures(chunkX, chunkZ);
            }
        }
    }
}

std::shared_ptr<Structure> StructureManager::CreateStructure(StructureType type) const {
    switch (type) {
        case StructureType::VILLAGE:
            return std::make_shared<VillageStructure>();
        case StructureType::DUNGEON:
            return std::make_shared<DungeonStructure>();
        case StructureType::DESERT_TEMPLE:
            return std::make_shared<TempleStructure>();
        case StructureType::FORTRESS:
            return std::make_shared<FortressStructure>();
        default:
            return nullptr;
    }
}

uint32_t StructureManager::GenerateStructureSeed(int x, int z) const {
    // Simple seed generation based on coordinates
    uint32_t seed = (x * 0x1f1f1f1f) ^ z;
    seed = (seed * 0x9e3779b9) >> 16;
    return seed;
}

bool StructureManager::IsBiomeCompatible(StructureType type, const glm::vec3& position) const {
    auto structure = GetStructure(type);
    if (!structure) return false;

    auto biome = m_world->GetBiomeAt(position);
    return structure->CanSpawnInBiome(static_cast<BiomeType>(biome));
}

float StructureManager::ApplyGenerationModifiers(StructureType type) const {
    float multiplier = m_config.globalSpawnMultiplier;

    auto structure = GetStructure(type);
    if (structure) {
        StructureRarity rarity = structure->GetRarity();
        switch (rarity) {
            case StructureRarity::RARE:
                multiplier *= m_config.rareStructureMultiplier;
                break;
            case StructureRarity::EPIC:
                multiplier *= m_config.epicStructureMultiplier;
                break;
            default:
                break;
        }
    }

    return multiplier;
}

bool StructureManager::ValidateStructurePosition(const glm::vec3& position, StructureType type) const {
    auto structure = GetStructure(type);
    if (!structure) return false;

    // Check Y level bounds
    const auto& definition = structure->GetDefinition();
    if (position.y < definition.minY || position.y > definition.maxY) {
        return false;
    }

    // Check biome compatibility
    if (m_config.respectBiomeRules && !IsBiomeCompatible(type, position)) {
        return false;
    }

    // Check terrain suitability
    if (m_config.respectTerrain) {
        // This would check if the terrain is suitable for the structure type
        // For example, villages need flat terrain, dungeons need underground space
    }

    // Check distance from world spawn
    float distanceFromSpawn = glm::distance(position, glm::vec3(0, position.y, 0));
    if (distanceFromSpawn < m_config.minDistanceBetweenStructures) {
        return false;
    }

    return true;
}

} // namespace VoxelCraft
