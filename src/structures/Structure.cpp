/**
 * @file Structure.cpp
 * @brief VoxelCraft Structure System Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#include "Structure.hpp"
#include "../world/World.hpp"
#include "../player/Player.hpp"
#include <algorithm>
#include <random>
#include <cmath>
#include <noise/noise.h>

namespace VoxelCraft {

// Structure base implementation
Structure::Structure(const StructureDefinition& definition)
    : m_definition(definition) {
    m_generationRules.respectTerrain = true;
    m_generationRules.clearTrees = true;
    m_generationRules.clearVegetation = true;
    m_generationRules.preserveWater = true;
    m_generationRules.avoidLakes = true;
    m_generationRules.avoidRivers = false;
    m_generationRules.avoidOceans = false;
    m_generationRules.minDistanceFromSpawn = 1000;
    m_generationRules.maxDistanceFromSpawn = 100000;
    m_generationRules.minDistanceFromPlayers = 50;
    m_generationRules.maxInstancesPerBiome = 5;
    m_generationRules.minInstancesPerBiome = 1;
    m_generationRules.terrainModificationChance = 0.3f;

    LoadTemplates();
}

bool Structure::CanGenerateAt(const glm::ivec3& position, World* world) const {
    if (!world) return false;

    // Check Y level
    if (position.y < m_definition.minY || position.y > m_definition.maxY) {
        return false;
    }

    // Check biome compatibility
    auto biome = world->GetBiomeAt(glm::vec3(position));
    if (!CanSpawnInBiome(biome)) {
        return false;
    }

    // Check distance from spawn
    float distanceFromSpawn = glm::distance(glm::vec3(position), glm::vec3(0, position.y, 0));
    if (distanceFromSpawn < m_generationRules.minDistanceFromSpawn ||
        distanceFromSpawn > m_generationRules.maxDistanceFromSpawn) {
        return false;
    }

    // Check terrain compatibility
    if (m_generationRules.respectTerrain) {
        // Check if terrain is suitable for this structure type
        // This would check for flat areas, suitable terrain, etc.
    }

    // Check for water if needed
    if (!m_definition.canSpawnInWater) {
        // Check if position is in water
        // This would check world water blocks
    }

    // Check for conflicts with other structures
    // This would check for nearby structures of the same or conflicting types

    return ValidatePlacement(position, GetSize(), world);
}

StructureInstance Structure::GenerateAt(const glm::ivec3& position, World* world) {
    StructureInstance instance;
    instance.type = m_definition.type;
    instance.position = position;
    instance.size = GetSize();
    instance.isGenerated = false;
    instance.isValid = true;

    if (!CanGenerateAt(position, world)) {
        instance.isValid = false;
        return instance;
    }

    // Clear area if needed
    if (m_generationRules.clearTrees || m_generationRules.clearVegetation) {
        ClearArea(position, instance.size, world);
    }

    // Generate structure pieces
    auto pieces = GeneratePieces(position);

    // Place pieces in world
    for (const auto& piece : pieces) {
        glm::ivec3 worldPos = position + piece.relativePosition;
        if (piece.CanPlaceAt(worldPos)) {
            PlacePiece(piece, worldPos, world);
        }
    }

    // Generate loot
    GenerateLoot(instance, world);

    // Spawn mobs
    SpawnMobs(instance, world);

    instance.isGenerated = true;
    instance.generatedTime = std::chrono::steady_clock::now();

    return instance;
}

bool Structure::CanSpawnInBiome(BiomeType biome) const {
    // Check if biome is in valid list
    if (!m_definition.validBiomes.empty()) {
        if (std::find(m_definition.validBiomes.begin(), m_definition.validBiomes.end(), biome) ==
            m_definition.validBiomes.end()) {
            return false;
        }
    }

    // Check if biome is in invalid list
    if (!m_definition.invalidBiomes.empty()) {
        if (std::find(m_definition.invalidBiomes.begin(), m_definition.invalidBiomes.end(), biome) !=
            m_definition.invalidBiomes.end()) {
            return false;
        }
    }

    return true;
}

StructureGenerationRules Structure::GetGenerationRules() const {
    return m_generationRules;
}

bool Structure::PlacePiece(const StructurePiece& piece, const glm::ivec3& worldPos, World* world) {
    if (!world) return false;

    const auto& templateData = piece.templateData;

    // Apply rotation to piece
    // This would rotate the template based on piece.rotation

    // Place blocks from template
    for (int y = 0; y < templateData.height; ++y) {
        for (int z = 0; z < templateData.length; ++z) {
            for (int x = 0; x < templateData.width; ++x) {
                int blockID = templateData.GetBlock(x, y, z);
                if (blockID != 0) { // Don't place air blocks
                    glm::ivec3 blockPos = worldPos + glm::ivec3(x, y, z);
                    world->SetBlock(blockPos.x, blockPos.y, blockPos.z, blockID);
                }
            }
        }
    }

    // Handle special blocks
    for (const auto& special : templateData.specialBlocks) {
        glm::ivec3 specialPos = worldPos + special.first;
        const std::string& blockType = special.second;

        if (blockType == "chest") {
            // Create chest with loot
            world->SetBlock(specialPos.x, specialPos.y, specialPos.z, 54); // Chest block ID
        } else if (blockType == "spawner") {
            // Create mob spawner
            world->SetBlock(specialPos.x, specialPos.y, specialPos.z, 52); // Spawner block ID
        } else if (blockType == "portal") {
            // Create portal
            // This would create a nether portal
        }
    }

    return true;
}

void Structure::ClearArea(const glm::ivec3& position, const glm::ivec3& size, World* world) {
    if (!world) return;

    glm::ivec3 halfSize = size / 2;
    glm::ivec3 min = position - halfSize;
    glm::ivec3 max = position + halfSize;

    for (int x = min.x; x <= max.x; ++x) {
        for (int z = min.z; z <= max.z; ++z) {
            for (int y = min.y; y <= max.y; ++y) {
                int currentBlock = world->GetBlock(x, y, z);

                // Clear trees if needed
                if (m_generationRules.clearTrees) {
                    if (currentBlock == 17 || currentBlock == 162) { // Log blocks
                        world->SetBlock(x, y, z, 0); // Air
                    }
                }

                // Clear vegetation if needed
                if (m_generationRules.clearVegetation) {
                    if (currentBlock >= 18 && currentBlock <= 31) { // Leaves and plants
                        world->SetBlock(x, y, z, 0); // Air
                    }
                }
            }
        }
    }
}

void Structure::PlaceFoundation(const glm::ivec3& position, const glm::ivec3& size, World* world) {
    if (!world) return;

    glm::ivec3 halfSize = size / 2;
    glm::ivec3 min = position - halfSize;
    glm::ivec3 max = position + halfSize;

    // Place foundation blocks
    for (int x = min.x; x <= max.x; ++x) {
        for (int z = min.z; z <= max.z; ++z) {
            // Find ground level
            int groundY = position.y;
            for (int y = position.y; y > 0; --y) {
                if (world->GetBlock(x, y, z) != 0) {
                    groundY = y + 1;
                    break;
                }
            }

            // Place foundation if needed
            if (groundY < position.y) {
                world->SetBlock(x, groundY, z, 1); // Stone foundation
            }
        }
    }
}

void Structure::GenerateLoot(StructureInstance& instance, World* world) {
    // Default loot generation - specific structures will override this
    for (const auto& chestPos : instance.chestPositions) {
        // Generate chest loot based on structure type and chest position
        // This would use the loot table system
    }
}

void Structure::SpawnMobs(StructureInstance& instance, World* world) {
    // Default mob spawning - specific structures will override this
    for (const auto& spawnerPos : instance.spawnerPositions) {
        // Create mob spawners for structure-specific mobs
        // This would place spawner blocks and configure them
    }
}

bool Structure::ValidatePlacement(const glm::ivec3& position, const glm::ivec3& size, World* world) const {
    if (!world) return false;

    glm::ivec3 halfSize = size / 2;
    glm::ivec3 min = position - halfSize;
    glm::ivec3 max = position + halfSize;

    // Check if area is clear enough for structure
    int solidBlocks = 0;
    int totalBlocks = size.x * size.y * size.z;

    for (int x = min.x; x <= max.x; ++x) {
        for (int y = min.y; y <= max.y; ++y) {
            for (int z = min.z; z <= max.z; ++z) {
                int blockID = world->GetBlock(x, y, z);
                if (blockID != 0) {
                    solidBlocks++;
                }
            }
        }
    }

    // Allow some solid blocks (for terrain) but not too many
    float solidRatio = static_cast<float>(solidBlocks) / totalBlocks;
    return solidRatio < 0.7f; // Less than 70% solid blocks
}

// VillageStructure implementation
VillageStructure::VillageStructure()
    : Structure(StructureDefinition{
        StructureType::VILLAGE,
        "village",
        "Village",
        "A settlement with houses, farms, and villagers",
        StructureSize::LARGE,
        StructureRarity::COMMON,
        60, 120, // minY, maxY
        32, 16,  // spacing, separation
        0.01f,   // spawnChance
        false,   // canSpawnInWater
        false,   // canSpawnUnderground
        true,    // canSpawnAboveGround
        {BiomeType::PLAINS, BiomeType::DESERT, BiomeType::SAVANNA, BiomeType::TAIGA}, // validBiomes
        {BiomeType::OCEAN, BiomeType::RIVER}, // invalidBiomes
        {MobType::VILLAGER}, // spawnsMobs
        {264, 265, 266, 388, 392, 393, 394, 395, 396, 397, 398, 399, 400, 401, 402} // chestLoot
    }) {
}

glm::ivec3 VillageStructure::GetSize() const {
    return glm::ivec3(80, 20, 80); // 80x20x80 blocks
}

void VillageStructure::LoadTemplates() {
    // Load village-specific templates
    // This would load house templates, farm templates, etc.
}

std::vector<StructurePiece> VillageStructure::GeneratePieces(const glm::ivec3& position) {
    std::vector<StructurePiece> pieces;

    // Generate village center
    StructurePiece center;
    center.name = "village_center";
    center.relativePosition = glm::ivec3(0, 0, 0);
    center.rotation = 0;
    pieces.push_back(center);

    // Generate houses around center
    for (int i = 0; i < 8; ++i) {
        float angle = (i / 8.0f) * 2.0f * 3.14159f;
        float distance = 20.0f + (rand() % 10);

        StructurePiece house;
        house.name = "village_house";
        house.relativePosition = glm::ivec3(
            static_cast<int>(cos(angle) * distance),
            0,
            static_cast<int>(sin(angle) * distance)
        );
        house.rotation = (rand() % 4) * 90;
        pieces.push_back(house);
    }

    // Generate farms
    for (int i = 0; i < 4; ++i) {
        float angle = (i / 4.0f) * 2.0f * 3.14159f;
        float distance = 35.0f;

        StructurePiece farm;
        farm.name = "village_farm";
        farm.relativePosition = glm::ivec3(
            static_cast<int>(cos(angle) * distance),
            0,
            static_cast<int>(sin(angle) * distance)
        );
        farm.rotation = (rand() % 4) * 90;
        pieces.push_back(farm);
    }

    return pieces;
}

void VillageStructure::GenerateLoot(StructureInstance& instance, World* world) {
    // Generate village-specific loot
    for (const auto& chestPos : instance.chestPositions) {
        // Village loot: bread, carrots, potatoes, emeralds, etc.
        std::vector<int> villageLoot = {297, 391, 392, 388, 264, 265, 266};
        // This would fill the chest with village-appropriate items
    }
}

void VillageStructure::SpawnMobs(StructureInstance& instance, World* world) {
    // Spawn villagers in houses
    // This would place villagers in appropriate locations
}

// DungeonStructure implementation
DungeonStructure::DungeonStructure()
    : Structure(StructureDefinition{
        StructureType::DUNGEON,
        "dungeon",
        "Dungeon",
        "An underground chamber with a monster spawner",
        StructureSize::SMALL,
        StructureRarity::COMMON,
        1, 40,   // minY, maxY
        8, 4,    // spacing, separation
        0.05f,   // spawnChance
        false,   // canSpawnInWater
        true,    // canSpawnUnderground
        false,   // canSpawnAboveGround
        {},      // validBiomes (any biome)
        {BiomeType::OCEAN}, // invalidBiomes
        {MobType::ZOMBIE, MobType::SKELETON, MobType::SPIDER, MobType::CREEPER}, // spawnsMobs
        {264, 265, 266, 331, 332, 341, 344, 348, 349, 350, 352, 353, 354, 355, 356} // chestLoot
    }) {
}

glm::ivec3 DungeonStructure::GetSize() const {
    return glm::ivec3(16, 8, 16); // 16x8x16 blocks
}

void DungeonStructure::LoadTemplates() {
    // Load dungeon templates
    // This would load different dungeon room templates
}

std::vector<StructurePiece> DungeonStructure::GeneratePieces(const glm::ivec3& position) {
    std::vector<StructurePiece> pieces;

    // Main dungeon chamber
    StructurePiece chamber;
    chamber.name = "dungeon_chamber";
    chamber.relativePosition = glm::ivec3(0, 0, 0);
    chamber.rotation = 0;
    pieces.push_back(chamber);

    // Add cobblestone walls and mossy cobblestone floor
    // The actual block placement is handled in the template

    return pieces;
}

void DungeonStructure::GenerateLoot(StructureInstance& instance, World* world) {
    // Generate dungeon loot
    for (const auto& chestPos : instance.chestPositions) {
        // Dungeon loot: bones, rotten flesh, string, gunpowder, etc.
        std::vector<int> dungeonLoot = {352, 367, 287, 289, 318, 319, 320, 321};
        // This would fill the chest with dungeon-appropriate items
    }
}

void DungeonStructure::SpawnMobs(StructureInstance& instance, World* world) {
    // Create mob spawner in dungeon
    for (const auto& spawnerPos : instance.spawnerPositions) {
        // Create spawner with random mob type from dungeon mobs
        // This would configure the spawner block
    }
}

// TempleStructure implementation
TempleStructure::TempleStructure()
    : Structure(StructureDefinition{
        StructureType::DESERT_TEMPLE,
        "desert_temple",
        "Desert Temple",
        "A desert pyramid with hidden treasures",
        StructureSize::MEDIUM,
        StructureRarity::UNCOMMON,
        60, 120, // minY, maxY
        16, 8,   // spacing, separation
        0.008f,  // spawnChance
        false,   // canSpawnInWater
        false,   // canSpawnUnderground
        true,    // canSpawnAboveGround
        {BiomeType::DESERT, BiomeType::DESERT_HILLS}, // validBiomes
        {},      // invalidBiomes
        {},      // spawnsMobs
        {264, 265, 266, 331, 341, 388, 399} // chestLoot
    }) {
}

glm::ivec3 TempleStructure::GetSize() const {
    return glm::ivec3(21, 11, 21); // 21x11x21 blocks
}

void TempleStructure::LoadTemplates() {
    // Load temple templates
    // This would load pyramid structure templates
}

std::vector<StructurePiece> TempleStructure::GeneratePieces(const glm::ivec3& position) {
    std::vector<StructurePiece> pieces;

    // Main pyramid structure
    StructurePiece pyramid;
    pyramid.name = "desert_pyramid";
    pyramid.relativePosition = glm::ivec3(0, 0, 0);
    pyramid.rotation = 0;
    pieces.push_back(pyramid);

    return pieces;
}

void TempleStructure::GenerateLoot(StructureInstance& instance, World* world) {
    // Generate temple loot
    for (const auto& chestPos : instance.chestPositions) {
        // Temple loot: emeralds, diamonds, gold, bones, rotten flesh
        std::vector<int> templeLoot = {264, 265, 266, 388, 264, 264, 264, 264};
        // This would fill the chest with temple-appropriate items
    }
}

void TempleStructure::SpawnMobs(StructureInstance& instance, World* world) {
    // Temples don't typically have spawners, but may have traps
    // This could create arrow traps or other mechanisms
}

// FortressStructure implementation
FortressStructure::FortressStructure()
    : Structure(StructureDefinition{
        StructureType::FORTRESS,
        "fortress",
        "Nether Fortress",
        "A fortress in the Nether made of nether bricks",
        StructureSize::HUGE,
        StructureRarity::UNCOMMON,
        30, 100, // minY, maxY
        30, 15,  // spacing, separation
        0.01f,   // spawnChance
        false,   // canSpawnInWater
        true,    // canSpawnUnderground (Nether)
        false,   // canSpawnAboveGround
        {BiomeType::NETHER}, // validBiomes
        {},      // invalidBiomes
        {MobType::BLAZE, MobType::WITHER_SKELETON, MobType::SKELETON}, // spawnsMobs
        {331, 406, 407, 452, 371, 372, 373, 377, 388, 399} // chestLoot
    }) {
}

glm::ivec3 FortressStructure::GetSize() const {
    return glm::ivec3(128, 32, 128); // Large fortress
}

void FortressStructure::LoadTemplates() {
    // Load fortress templates
    // This would load different fortress section templates
}

std::vector<StructurePiece> FortressStructure::GeneratePieces(const glm::ivec3& position) {
    std::vector<StructurePiece> pieces;

    // Generate fortress corridors and rooms
    // This is a complex structure with multiple interconnected pieces

    // Main corridor
    StructurePiece corridor;
    corridor.name = "fortress_corridor";
    corridor.relativePosition = glm::ivec3(0, 0, 0);
    corridor.rotation = 0;
    pieces.push_back(corridor);

    // Add rooms along the corridor
    for (int i = 0; i < 10; ++i) {
        StructurePiece room;
        room.name = (rand() % 3 == 0) ? "fortress_blaze_room" : "fortress_nether_wart_room";
        room.relativePosition = glm::ivec3(i * 16, 0, 0);
        room.rotation = 0;
        pieces.push_back(room);
    }

    return pieces;
}

void FortressStructure::GenerateLoot(StructureInstance& instance, World* world) {
    // Generate fortress loot
    for (const auto& chestPos : instance.chestPositions) {
        // Fortress loot: nether wart, blaze rods, gold, etc.
        std::vector<int> fortressLoot = {372, 369, 370, 371, 406, 407, 452, 388, 399};
        // This would fill the chest with fortress-appropriate items
    }
}

void FortressStructure::SpawnMobs(StructureInstance& instance, World* world) {
    // Spawn fortress mobs
    for (const auto& spawnerPos : instance.spawnerPositions) {
        // Create spawners for blazes and wither skeletons
        // This would configure the spawner blocks
    }
}

} // namespace VoxelCraft
