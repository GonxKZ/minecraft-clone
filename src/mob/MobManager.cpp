/**
 * @file MobManager.cpp
 * @brief VoxelCraft Mob Manager Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#include "MobManager.hpp"
#include "../world/World.hpp"
#include "../player/Player.hpp"
#include <algorithm>
#include <random>
#include <cmath>

namespace VoxelCraft {

// MobManager implementation
MobManager& MobManager::GetInstance() {
    static MobManager instance;
    return instance;
}

bool MobManager::Initialize(World* world) {
    if (!world) return false;

    m_world = world;
    m_nextMobId = 1;
    m_initialized = true;
    m_spawningEnabled = true;
    m_maxMobCount = 100; // Default maximum

    m_naturalSpawnTimer = 0.0f;
    m_spawnerUpdateTimer = 0.0f;
    m_packSpawnTimer = 0.0f;

    RegisterMobFactories();
    InitializeDefaultSpawnRules();
    InitializeDefaultMobPacks();

    return true;
}

void MobManager::Shutdown() {
    ClearAllMobs();
    ClearAllMobSpawners();
    m_spawnRules.clear();
    m_mobPacks.clear();
    m_mobFactories.clear();
    m_world = nullptr;
    m_initialized = false;
}

void MobManager::Update(float deltaTime) {
    if (!m_initialized || !m_world) return;

    // Update all mobs
    for (auto it = m_mobs.begin(); it != m_mobs.end();) {
        if (it->second) {
            it->second->Update(deltaTime);
            ++it;
        } else {
            it = m_mobs.erase(it);
        }
    }

    // Update natural spawning
    if (m_spawningEnabled) {
        UpdateNaturalSpawning(deltaTime);
        UpdateMobSpawners(deltaTime);
        UpdateMobPacks(deltaTime);
    }

    // Update statistics
    m_stats.activeMobs = m_mobs.size();
    m_stats.activeSpawners = 0;
    for (const auto& pair : m_spawners) {
        if (pair.second.isActive) {
            m_stats.activeSpawners++;
        }
    }
}

std::shared_ptr<Mob> MobManager::SpawnMob(MobType type, const glm::vec3& position,
                                        MobSpawnReason reason, Entity* spawner) {
    if (!m_initialized || !m_world) return nullptr;

    // Check if we've reached the mob limit
    if (m_mobs.size() >= m_maxMobCount) {
        return nullptr;
    }

    // Check if position is valid
    if (!IsPositionValid(position)) {
        return nullptr;
    }

    // Check if area is clear
    if (!IsAreaClear(position, 2.0f)) {
        return nullptr;
    }

    // Create mob using factory
    auto factoryIt = m_mobFactories.find(type);
    if (factoryIt == m_mobFactories.end()) {
        return nullptr;
    }

    auto mob = factoryIt->second(position, m_world);
    if (!mob) return nullptr;

    // Assign ID and setup
    uint32_t mobId = GenerateMobId();
    mob->SetId(mobId);

    // Add to active mobs
    m_mobs[mobId] = mob;

    // Update statistics
    m_stats.totalMobsSpawned++;
    m_stats.mobsByType[type]++;
    m_stats.spawnReasons[reason]++;

    return mob;
}

bool MobManager::RemoveMob(std::shared_ptr<Mob> mob) {
    if (!mob) return false;

    return RemoveMob(mob->GetId());
}

bool MobManager::RemoveMob(uint32_t mobId) {
    auto it = m_mobs.find(mobId);
    if (it == m_mobs.end()) return false;

    // Update statistics
    if (it->second) {
        m_stats.totalMobsKilled++;
        m_stats.mobsByType[it->second->GetMobType()]--;
    }

    m_mobs.erase(it);
    return true;
}

std::shared_ptr<Mob> MobManager::GetMob(uint32_t mobId) const {
    auto it = m_mobs.find(mobId);
    return (it != m_mobs.end()) ? it->second : nullptr;
}

std::vector<std::shared_ptr<Mob>> MobManager::GetAllMobs() const {
    std::vector<std::shared_ptr<Mob>> mobs;
    mobs.reserve(m_mobs.size());

    for (const auto& pair : m_mobs) {
        mobs.push_back(pair.second);
    }

    return mobs;
}

std::vector<std::shared_ptr<Mob>> MobManager::GetMobsInArea(const glm::vec3& center, float radius) const {
    std::vector<std::shared_ptr<Mob>> mobsInArea;

    for (const auto& pair : m_mobs) {
        if (pair.second) {
            float distance = glm::distance(center, pair.second->GetPosition());
            if (distance <= radius) {
                mobsInArea.push_back(pair.second);
            }
        }
    }

    return mobsInArea;
}

std::vector<std::shared_ptr<Mob>> MobManager::GetMobsByType(MobType type) const {
    std::vector<std::shared_ptr<Mob>> mobsOfType;

    for (const auto& pair : m_mobs) {
        if (pair.second && pair.second->GetMobType() == type) {
            mobsOfType.push_back(pair.second);
        }
    }

    return mobsOfType;
}

std::shared_ptr<Mob> MobManager::GetNearestMob(const glm::vec3& position, float maxDistance) const {
    std::shared_ptr<Mob> nearest = nullptr;
    float nearestDistance = maxDistance * maxDistance;

    for (const auto& pair : m_mobs) {
        if (pair.second) {
            float distanceSquared = glm::distance2(position, pair.second->GetPosition());
            if (distanceSquared < nearestDistance) {
                nearestDistance = distanceSquared;
                nearest = pair.second;
            }
        }
    }

    return nearest;
}

size_t MobManager::GetMobCount(MobType type) const {
    return GetMobsByType(type).size();
}

size_t MobManager::GetTotalMobCount() const {
    return m_mobs.size();
}

bool MobManager::CanMobSpawnAt(MobType type, const glm::vec3& position) const {
    auto rulesIt = m_spawnRules.find(type);
    if (rulesIt == m_spawnRules.end()) return false;

    const auto& rules = rulesIt->second;

    // Check Y level
    if (position.y < rules.minY || position.y > rules.maxY) return false;

    // Check light level (simplified - would check world light)
    int lightLevel = 15; // This would get actual light level
    if (lightLevel < rules.minLightLevel || lightLevel > rules.maxLightLevel) return false;

    // Check distance from players
    float distanceToPlayer = GetDistanceToNearestPlayer(position);
    if (distanceToPlayer < rules.minDistanceFromPlayer || distanceToPlayer > rules.maxDistanceFromPlayer) {
        return false;
    }

    // Check if position is valid
    if (!IsPositionValid(position)) return false;

    // Check if area is clear
    if (!IsAreaClear(position, 2.0f)) return false;

    return true;
}

const MobSpawnRules& MobManager::GetSpawnRules(MobType type) const {
    static MobSpawnRules defaultRules;
    auto it = m_spawnRules.find(type);
    return (it != m_spawnRules.end()) ? it->second : defaultRules;
}

void MobManager::SetSpawnRules(MobType type, const MobSpawnRules& rules) {
    m_spawnRules[type] = rules;
}

bool MobManager::CreateMobSpawner(const glm::ivec3& position, MobType type) {
    if (m_spawners.find(position) != m_spawners.end()) {
        return false;
    }

    MobSpawner spawner;
    spawner.position = position;
    spawner.spawnType = type;
    spawner.spawnDelay = 200; // 10 seconds
    spawner.minSpawnDelay = 200;
    spawner.maxSpawnDelay = 800;
    spawner.spawnCount = 4;
    spawner.maxNearbyEntities = 6;
    spawner.requiredPlayerRange = 16;
    spawner.spawnRange = 4;
    spawner.isActive = true;
    spawner.lastSpawnTime = std::chrono::steady_clock::now();

    m_spawners[position] = spawner;
    m_stats.totalSpawners++;

    return true;
}

bool MobManager::RemoveMobSpawner(const glm::ivec3& position) {
    auto it = m_spawners.find(position);
    if (it == m_spawners.end()) return false;

    m_spawners.erase(it);
    m_stats.totalSpawners--;

    return true;
}

MobSpawner* MobManager::GetMobSpawner(const glm::ivec3& position) {
    auto it = m_spawners.find(position);
    return (it != m_spawners.end()) ? &it->second : nullptr;
}

std::vector<MobSpawner*> MobManager::GetAllMobSpawners() {
    std::vector<MobSpawner*> spawners;
    spawners.reserve(m_spawners.size());

    for (auto& pair : m_spawners) {
        spawners.push_back(&pair.second);
    }

    return spawners;
}

void MobManager::RegisterMobPack(const MobPack& pack) {
    m_mobPacks.push_back(pack);
}

std::vector<std::shared_ptr<Mob>> MobManager::SpawnMobPack(size_t packType, const glm::vec3& position) {
    std::vector<std::shared_ptr<Mob>> spawnedMobs;

    if (packType >= m_mobPacks.size()) return spawnedMobs;

    const auto& pack = m_mobPacks[packType];

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> sizeDist(pack.minSize, pack.maxSize);
    int packSize = sizeDist(gen);

    // Spawn leader first
    auto leader = SpawnMob(pack.leaderType, position, MobSpawnReason::NATURAL);
    if (leader) {
        spawnedMobs.push_back(leader);
        packSize--;
    }

    // Spawn members around leader
    for (int i = 0; i < packSize && i < static_cast<int>(pack.memberTypes.size()); ++i) {
        std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * 3.14159f);
        std::uniform_real_distribution<float> radiusDist(1.0f, pack.packRadius);

        float angle = angleDist(gen);
        float radius = radiusDist(gen);

        glm::vec3 memberPos = position + glm::vec3(
            radius * cos(angle),
            0.0f,
            radius * sin(angle)
        );

        auto member = SpawnMob(pack.memberTypes[i % pack.memberTypes.size()],
                             memberPos, MobSpawnReason::NATURAL);
        if (member) {
            spawnedMobs.push_back(member);
        }
    }

    return spawnedMobs;
}

void MobManager::ClearAllMobs() {
    m_mobs.clear();
}

void MobManager::ClearAllMobSpawners() {
    m_spawners.clear();
    m_stats.totalSpawners = 0;
}

void MobManager::UpdateNaturalSpawning(float deltaTime) {
    m_naturalSpawnTimer += deltaTime;

    if (m_naturalSpawnTimer >= 1.0f) { // Check every second
        m_naturalSpawnTimer = 0.0f;

        // This would typically check player positions and spawn mobs around them
        // For now, we'll simulate occasional natural spawning

        if (rand() % 100 < 5) { // 5% chance per second
            // Find a random position around a player
            glm::vec3 spawnPos(0.0f, 70.0f, 0.0f); // Simplified - would use actual player position
            AttemptNaturalSpawn(spawnPos);
        }
    }
}

void MobManager::AttemptNaturalSpawn(const glm::vec3& position) {
    if (!ShouldSpawnMob(position)) return;

    MobType mobType = SelectMobTypeForSpawn(position);
    if (mobType == MobType::MAX_MOB_TYPES) return;

    // Check if we can spawn this type here
    if (!CanMobSpawnAt(mobType, position)) return;

    // Get spawn rules for group size
    const auto& rules = GetSpawnRules(mobType);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> groupDist(rules.minGroupSize, rules.maxGroupSize);
    int groupSize = groupDist(gen);

    // Spawn the group
    for (int i = 0; i < groupSize; ++i) {
        std::uniform_real_distribution<float> offsetDist(-5.0f, 5.0f);
        glm::vec3 spawnPos = position + glm::vec3(offsetDist(gen), 0.0f, offsetDist(gen));

        SpawnMob(mobType, spawnPos, MobSpawnReason::NATURAL);
    }
}

bool MobManager::ShouldSpawnMob(const glm::vec3& position) const {
    // Check if there are too many mobs already
    if (m_mobs.size() >= m_maxMobCount) return false;

    // Check distance from players
    float distanceToPlayer = GetDistanceToNearestPlayer(position);
    if (distanceToPlayer < 24.0f || distanceToPlayer > 128.0f) return false;

    return true;
}

MobType MobManager::SelectMobTypeForSpawn(const glm::vec3& position) const {
    // Simplified mob selection based on position
    // In a real implementation, this would consider biome, time of day, etc.

    std::vector<MobType> possibleMobs;

    // Daytime mobs
    if (true) { // Would check time of day
        possibleMobs = {MobType::CHICKEN, MobType::COW, MobType::PIG, MobType::SHEEP};
    } else { // Nighttime mobs
        possibleMobs = {MobType::ZOMBIE, MobType::SKELETON, MobType::CREEPER, MobType::SPIDER};
    }

    if (possibleMobs.empty()) return MobType::MAX_MOB_TYPES;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> dist(0, possibleMobs.size() - 1);

    return possibleMobs[dist(gen)];
}

void MobManager::UpdateMobSpawners(float deltaTime) {
    m_spawnerUpdateTimer += deltaTime;

    if (m_spawnerUpdateTimer >= 1.0f) { // Update every second
        m_spawnerUpdateTimer = 0.0f;

        for (auto& pair : m_spawners) {
            UpdateMobSpawner(pair.second, deltaTime);
        }
    }
}

void MobManager::UpdateMobSpawner(MobSpawner& spawner, float deltaTime) {
    if (!spawner.isActive) return;

    auto now = std::chrono::steady_clock::now();
    auto timeSinceLastSpawn = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - spawner.lastSpawnTime).count();

    if (timeSinceLastSpawn >= spawner.spawnDelay * 50) { // Convert ticks to milliseconds
        if (ShouldSpawnerActivate(spawner)) {
            // Spawn mobs
            for (int i = 0; i < spawner.spawnCount; ++i) {
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_real_distribution<float> offsetDist(-spawner.spawnRange, spawner.spawnRange);

                glm::vec3 spawnPos = glm::vec3(spawner.position) + glm::vec3(
                    offsetDist(gen), 0.0f, offsetDist(gen)
                );

                auto mob = SpawnMob(spawner.spawnType, spawnPos, MobSpawnReason::SPAWNER);
                if (mob) {
                    spawner.spawnedMobs.push_back({
                        spawner.spawnType,
                        spawnPos,
                        MobSpawnReason::SPAWNER,
                        nullptr, // spawner entity
                        static_cast<float>(timeSinceLastSpawn) / 1000.0f
                    });
                }
            }

            // Reset spawn timer
            std::uniform_int_distribution<int> delayDist(spawner.minSpawnDelay, spawner.maxSpawnDelay);
            spawner.spawnDelay = delayDist(gen);
            spawner.lastSpawnTime = now;
        }
    }
}

bool MobManager::ShouldSpawnerActivate(const MobSpawner& spawner) const {
    // Check if players are nearby
    float distanceToPlayer = GetDistanceToNearestPlayer(glm::vec3(spawner.position));
    if (distanceToPlayer > spawner.requiredPlayerRange) return false;

    // Check how many entities are nearby
    auto nearbyMobs = GetMobsInArea(glm::vec3(spawner.position), 8.0f);
    if (nearbyMobs.size() >= static_cast<size_t>(spawner.maxNearbyEntities)) return false;

    return true;
}

void MobManager::UpdateMobPacks(float deltaTime) {
    m_packSpawnTimer += deltaTime;

    if (m_packSpawnTimer >= 60.0f && !m_mobPacks.empty()) { // Check every minute
        m_packSpawnTimer = 0.0f;

        if (rand() % 10 == 0) { // 10% chance
            SpawnRandomPack();
        }
    }
}

void MobManager::SpawnRandomPack() {
    if (m_mobPacks.empty()) return;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> packDist(0, m_mobPacks.size() - 1);
    size_t packIndex = packDist(gen);

    // Find a suitable spawn location
    glm::vec3 spawnPos(0.0f, 70.0f, 0.0f); // Simplified

    SpawnMobPack(packIndex, spawnPos);
}

uint32_t MobManager::GenerateMobId() {
    return m_nextMobId++;
}

bool MobManager::IsPositionValid(const glm::vec3& position) const {
    if (!m_world) return false;

    // Check if position is in world bounds
    // Check if position is not in solid blocks
    // This would do actual collision checks with the world

    return true;
}

bool MobManager::IsAreaClear(const glm::vec3& position, float radius) const {
    // Check if the area around the position is clear of other mobs
    auto nearbyMobs = GetMobsInArea(position, radius);
    return nearbyMobs.empty();
}

float MobManager::GetDistanceToNearestPlayer(const glm::vec3& position) const {
    // This would calculate distance to the nearest player
    // For now, return a default distance
    return 50.0f;
}

void MobManager::RegisterMobFactories() {
    m_mobFactories[MobType::CREEPER] = CreateCreeper;
    m_mobFactories[MobType::ZOMBIE] = CreateZombie;
    m_mobFactories[MobType::SKELETON] = CreateSkeleton;
    m_mobFactories[MobType::SPIDER] = CreateSpider;
    m_mobFactories[MobType::ENDERMAN] = CreateEnderman;
    m_mobFactories[MobType::WOLF] = CreateWolf;
    m_mobFactories[MobType::CHICKEN] = CreateChicken;
    m_mobFactories[MobType::COW] = CreateCow;
    m_mobFactories[MobType::PIG] = CreatePig;
    m_mobFactories[MobType::SHEEP] = CreateSheep;
    m_mobFactories[MobType::VILLAGER] = CreateVillager;
}

void MobManager::InitializeDefaultSpawnRules() {
    // Hostile mobs
    MobSpawnRules zombieRules;
    zombieRules.minLightLevel = 0;
    zombieRules.maxLightLevel = 7;
    zombieRules.minY = 0;
    zombieRules.maxY = 256;
    zombieRules.minGroupSize = 1;
    zombieRules.maxGroupSize = 4;
    zombieRules.spawnWeight = 100.0f;
    zombieRules.needsDarkness = true;
    zombieRules.minDistanceFromPlayer = 24;
    zombieRules.maxDistanceFromPlayer = 128;
    SetSpawnRules(MobType::ZOMBIE, zombieRules);

    MobSpawnRules creeperRules;
    creeperRules.minLightLevel = 0;
    creeperRules.maxLightLevel = 7;
    creeperRules.minY = 0;
    creeperRules.maxY = 256;
    creeperRules.minGroupSize = 1;
    creeperRules.maxGroupSize = 1;
    creeperRules.spawnWeight = 100.0f;
    creeperRules.needsDarkness = true;
    creeperRules.minDistanceFromPlayer = 24;
    creeperRules.maxDistanceFromPlayer = 128;
    SetSpawnRules(MobType::CREEPER, creeperRules);

    // Passive mobs
    MobSpawnRules cowRules;
    cowRules.minLightLevel = 9;
    cowRules.maxLightLevel = 15;
    cowRules.minY = 0;
    cowRules.maxY = 256;
    cowRules.minGroupSize = 1;
    cowRules.maxGroupSize = 4;
    cowRules.spawnWeight = 8.0f;
    cowRules.needsSurface = true;
    cowRules.minDistanceFromPlayer = 24;
    cowRules.maxDistanceFromPlayer = 128;
    SetSpawnRules(MobType::COW, cowRules);

    MobSpawnRules chickenRules;
    chickenRules.minLightLevel = 9;
    chickenRules.maxLightLevel = 15;
    chickenRules.minY = 0;
    chickenRules.maxY = 256;
    chickenRules.minGroupSize = 1;
    chickenRules.maxGroupSize = 4;
    chickenRules.spawnWeight = 10.0f;
    chickenRules.needsSurface = true;
    chickenRules.minDistanceFromPlayer = 24;
    chickenRules.maxDistanceFromPlayer = 128;
    SetSpawnRules(MobType::CHICKEN, chickenRules);
}

void MobManager::InitializeDefaultMobPacks() {
    // Wolf pack
    MobPack wolfPack;
    wolfPack.leaderType = MobType::WOLF;
    wolfPack.memberTypes = {MobType::WOLF, MobType::WOLF, MobType::WOLF};
    wolfPack.minSize = 2;
    wolfPack.maxSize = 4;
    wolfPack.cohesion = 0.8f;
    wolfPack.packRadius = 10.0f;
    wolfPack.packBehavior = MobBehavior::PACK_HUNTING;
    RegisterMobPack(wolfPack);

    // Zombie pack
    MobPack zombiePack;
    zombiePack.leaderType = MobType::ZOMBIE;
    zombiePack.memberTypes = {MobType::ZOMBIE, MobType::ZOMBIE};
    zombiePack.minSize = 2;
    zombiePack.maxSize = 6;
    zombiePack.cohesion = 0.6f;
    zombiePack.packRadius = 15.0f;
    zombiePack.packBehavior = MobBehavior::PACK_HUNTING;
    RegisterMobPack(zombiePack);
}

// Mob factory functions
std::shared_ptr<Mob> MobManager::CreateCreeper(const glm::vec3& position, World* world) {
    return std::make_shared<Creeper>(position, world);
}

std::shared_ptr<Mob> MobManager::CreateZombie(const glm::vec3& position, World* world) {
    return std::make_shared<Zombie>(position, world);
}

std::shared_ptr<Mob> MobManager::CreateSkeleton(const glm::vec3& position, World* world) {
    return std::make_shared<Skeleton>(position, world);
}

std::shared_ptr<Mob> MobManager::CreateSpider(const glm::vec3& position, World* world) {
    return std::make_shared<Spider>(position, world);
}

std::shared_ptr<Mob> MobManager::CreateEnderman(const glm::vec3& position, World* world) {
    return std::make_shared<Enderman>(position, world);
}

std::shared_ptr<Mob> MobManager::CreateWolf(const glm::vec3& position, World* world) {
    return std::make_shared<Wolf>(position, world);
}

std::shared_ptr<Mob> MobManager::CreateChicken(const glm::vec3& position, World* world) {
    return std::make_shared<Chicken>(position, world);
}

std::shared_ptr<Mob> MobManager::CreateCow(const glm::vec3& position, World* world) {
    return std::make_shared<Cow>(position, world);
}

std::shared_ptr<Mob> MobManager::CreatePig(const glm::vec3& position, World* world) {
    return std::make_shared<Pig>(position, world);
}

std::shared_ptr<Mob> MobManager::CreateSheep(const glm::vec3& position, World* world) {
    return std::make_shared<Sheep>(position, world);
}

std::shared_ptr<Mob> MobManager::CreateVillager(const glm::vec3& position, World* world) {
    return std::make_shared<Villager>(position, world);
}

} // namespace VoxelCraft
