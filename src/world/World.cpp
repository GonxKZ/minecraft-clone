/**
 * @file World.cpp
 * @brief VoxelCraft World System - Main World Coordinator Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#include "World.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>

namespace VoxelCraft {

// Simple vector structures
struct Vec2 {
    float x, y;
    Vec2(float x = 0, float y = 0) : x(x), y(y) {}
};

struct Vec3 {
    float x, y, z;
    Vec3(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {}
};

World::World(const WorldSettings& settings)
    : m_settings(settings)
    , m_state(WorldState::UNINITIALIZED)
    , m_nextCallbackId(1)
    , m_worldThreadRunning(false)
{
    m_stats.loadedChunks = 0;
    m_stats.generatedChunks = 0;
    m_stats.totalChunks = 0;
    m_stats.entities = 0;
    m_stats.players = 0;
    m_stats.chunkLoadTime = 0.0f;
    m_stats.chunkGenTime = 0.0f;
    m_stats.memoryUsage = 0;
    m_stats.chunksInQueue = 0;
}

World::~World() {
    Shutdown();
}

bool World::Initialize() {
    if (m_state != WorldState::UNINITIALIZED) {
        return false;
    }

    m_state = WorldState::INITIALIZING;

    try {
        // Initialize world generator
        // m_worldGenerator = std::make_unique<WorldGenerator>(m_settings.seed);

        // Start world update thread
        m_worldThreadRunning = true;
        m_worldThread = std::thread(&World::WorldUpdateThread, this);

        m_state = WorldState::READY;
        std::cout << "World initialized successfully" << std::endl;
        return true;

    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize world: " << e.what() << std::endl;
        m_state = WorldState::ERROR;
        return false;
    }
}

void World::Shutdown() {
    if (m_state == WorldState::UNINITIALIZED) {
        return;
    }

    // Stop world thread
    m_worldThreadRunning = false;
    if (m_worldThread.joinable()) {
        m_worldThread.join();
    }

    // Save all chunks
    SaveWorld();

    // Clear all data
    {
        std::lock_guard<std::mutex> chunkLock(m_chunkMutex);
        m_loadedChunks.clear();
    }

    {
        std::lock_guard<std::mutex> entityLock(m_entityMutex);
        m_entities.clear();
    }

    m_state = WorldState::UNINITIALIZED;
}

void World::Update(float deltaTime, const Vec3& playerPosition) {
    if (m_state != WorldState::READY) {
        return;
    }

    // Update chunk loading based on player position
    UpdateChunkLoading(playerPosition);

    // Update entities
    UpdateEntities(deltaTime);

    // Update statistics
    UpdateStats();
}

void World::Render(const Vec3& cameraPosition) {
    if (m_state != WorldState::READY) {
        return;
    }

    std::lock_guard<std::mutex> lock(m_chunkMutex);

    // Simple frustum culling and render chunks
    for (const auto& pair : m_loadedChunks) {
        std::shared_ptr<Chunk> chunk = pair.second;
        if (chunk && chunk->IsVisible()) {
            // TODO: Add proper frustum culling
            // For now, render all loaded chunks
            chunk->Render();
        }
    }
}

BlockType World::GetBlock(int worldX, int worldY, int worldZ) const {
    if (!IsValidBlockPosition(worldX, worldY, worldZ)) {
        return BlockType::AIR;
    }

    int chunkX, chunkZ, localX, localZ;
    WorldToChunkCoordinates(worldX, worldZ, chunkX, chunkZ, localX, localZ);

    std::lock_guard<std::mutex> lock(m_chunkMutex);
    auto it = m_loadedChunks.find(ChunkPosition(chunkX, chunkZ));
    if (it != m_loadedChunks.end()) {
        return it->second->GetBlock(localX, worldY, localZ);
    }

    return BlockType::AIR; // Chunk not loaded
}

void World::SetBlock(int worldX, int worldY, int worldZ, BlockType blockType, bool updateNeighbors) {
    if (!IsValidBlockPosition(worldX, worldY, worldZ)) {
        return;
    }

    int chunkX, chunkZ, localX, localZ;
    WorldToChunkCoordinates(worldX, worldZ, chunkX, chunkZ, localX, localZ);

    std::lock_guard<std::mutex> lock(m_chunkMutex);
    auto it = m_loadedChunks.find(ChunkPosition(chunkX, chunkZ));
    if (it != m_loadedChunks.end()) {
        it->second->SetBlock(localX, worldY, localZ, blockType, updateNeighbors);

        // Notify block change
        OnBlockChanged(worldX, worldY, worldZ);
    }
}

bool World::IsValidBlockPosition(int worldX, int worldY, int worldZ) const {
    return worldY >= 0 && worldY < m_settings.worldHeight;
}

std::shared_ptr<Chunk> World::GetChunk(const ChunkPosition& chunkPos) const {
    std::lock_guard<std::mutex> lock(m_chunkMutex);
    auto it = m_loadedChunks.find(chunkPos);
    return (it != m_loadedChunks.end()) ? it->second : nullptr;
}

std::shared_ptr<Chunk> World::GetChunkAt(int worldX, int worldZ) const {
    int chunkX = worldX / CHUNK_SIZE;
    int chunkZ = worldZ / CHUNK_SIZE;
    return GetChunk(ChunkPosition(chunkX, chunkZ));
}

bool World::LoadChunk(const ChunkPosition& chunkPos, bool generateIfMissing) {
    // Check if already loaded
    if (GetChunk(chunkPos) != nullptr) {
        return true;
    }

    // Try to load from disk first
    // TODO: Implement chunk loading from disk

    // Generate if missing and requested
    if (generateIfMissing) {
        return GenerateChunk(chunkPos);
    }

    return false;
}

void World::UnloadChunk(const ChunkPosition& chunkPos) {
    std::lock_guard<std::mutex> lock(m_chunkMutex);
    auto it = m_loadedChunks.find(chunkPos);
    if (it != m_loadedChunks.end()) {
        // Save chunk before unloading
        SaveChunk(chunkPos);
        m_loadedChunks.erase(it);
        m_stats.loadedChunks--;
    }
}

void World::SaveChunk(const ChunkPosition& chunkPos) {
    // TODO: Implement chunk saving to disk
    std::cout << "Saving chunk at " << chunkPos.x << ", " << chunkPos.z << std::endl;
}

bool World::GenerateChunk(const ChunkPosition& chunkPos) {
    auto chunk = std::make_shared<Chunk>(chunkPos.x, chunkPos.z);

    if (chunk->Generate()) {
        std::lock_guard<std::mutex> lock(m_chunkMutex);
        m_loadedChunks[chunkPos] = chunk;
        m_stats.generatedChunks++;
        m_stats.loadedChunks++;
        return true;
    }

    return false;
}

BiomeType World::GetBiomeAt(int worldX, int worldZ) const {
    // Simple biome calculation based on temperature/humidity
    float temperature = std::sin(worldX * 0.01f) * std::cos(worldZ * 0.01f);
    float humidity = std::cos(worldX * 0.005f) * std::sin(worldZ * 0.005f);

    if (temperature > 0.3f && humidity > 0.2f) {
        return BiomeType::JUNGLE;
    } else if (temperature > 0.2f) {
        return BiomeType::PLAINS;
    } else if (temperature < -0.2f) {
        return BiomeType::SNOWY_MOUNTAINS;
    } else {
        return BiomeType::FOREST;
    }
}

int World::GetHeightAt(int worldX, int worldZ) const {
    // Simple height calculation using noise
    float noise1 = std::sin(worldX * 0.01f) * std::cos(worldZ * 0.01f);
    float noise2 = std::sin(worldX * 0.02f) * std::cos(worldZ * 0.02f) * 0.5f;
    float height = (noise1 + noise2) * 20.0f + 63.0f; // Base height around sea level

    return static_cast<int>(std::max(0.0f, std::min(height, static_cast<float>(m_settings.worldHeight - 1))));
}

bool World::IsSolidBlock(int worldX, int worldY, int worldZ) const {
    BlockType block = GetBlock(worldX, worldY, worldZ);
    return block != BlockType::AIR;
}

int World::FindGroundLevel(int worldX, int worldZ, int startY) const {
    for (int y = startY; y >= 0; --y) {
        if (IsSolidBlock(worldX, y, worldZ)) {
            return y;
        }
    }
    return 0;
}

bool World::Raycast(const Vec3& start, const Vec3& direction, float maxDistance,
                   Vec3& hitPosition, Vec3& hitNormal) const {
    // Simple raycast implementation
    Vec3 pos = start;
    Vec3 step = direction * 0.5f; // Step size
    float distance = 0.0f;

    while (distance < maxDistance) {
        int x = static_cast<int>(pos.x);
        int y = static_cast<int>(pos.y);
        int z = static_cast<int>(pos.z);

        if (IsSolidBlock(x, y, z)) {
            hitPosition = pos;
            // Simple normal calculation (could be improved)
            hitNormal = Vec3(0, 1, 0); // Assume top face for now
            return true;
        }

        pos = pos + step;
        distance += 0.5f;
    }

    return false;
}

void World::AddEntity(std::shared_ptr<Entity> entity) {
    std::lock_guard<std::mutex> lock(m_entityMutex);
    m_entities.push_back(entity);
    m_stats.entities++;
}

void World::RemoveEntity(std::shared_ptr<Entity> entity) {
    std::lock_guard<std::mutex> lock(m_entityMutex);
    auto it = std::find(m_entities.begin(), m_entities.end(), entity);
    if (it != m_entities.end()) {
        m_entities.erase(it);
        m_stats.entities--;
    }
}

std::vector<std::shared_ptr<Entity>> World::GetEntitiesInArea(const Vec3& center, float radius) const {
    std::lock_guard<std::mutex> lock(m_entityMutex);
    std::vector<std::shared_ptr<Entity>> result;

    for (const auto& entity : m_entities) {
        Vec3 entityPos = entity->GetPosition();
        Vec3 diff = Vec3(entityPos.x - center.x, entityPos.y - center.y, entityPos.z - center.z);
        if (diff.x * diff.x + diff.y * diff.y + diff.z * diff.z <= radius * radius) {
            result.push_back(entity);
        }
    }

    return result;
}

bool World::SaveWorld() {
    std::lock_guard<std::mutex> lock(m_chunkMutex);
    for (const auto& pair : m_loadedChunks) {
        SaveChunk(pair.first);
    }
    return true;
}

bool World::LoadWorld() {
    // TODO: Implement world loading from disk
    return true;
}

int World::RegisterEventCallback(WorldEventCallback callback) {
    int id = m_nextCallbackId++;
    m_eventCallbacks.push_back(callback);
    return id;
}

void World::UnregisterEventCallback(int callbackId) {
    // Simple implementation - just remove by index
    if (callbackId > 0 && callbackId <= static_cast<int>(m_eventCallbacks.size())) {
        m_eventCallbacks.erase(m_eventCallbacks.begin() + callbackId - 1);
    }
}

// Private methods

void World::WorldUpdateThread() {
    while (m_worldThreadRunning) {
        ProcessChunkQueues();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

void World::ProcessChunkQueues() {
    // Process load queue
    while (!m_chunkLoadQueue.empty()) {
        ChunkPosition pos = m_chunkLoadQueue.front();
        m_chunkLoadQueue.pop();

        if (LoadChunk(pos, true)) {
            m_stats.chunksInQueue--;
        }
    }

    // Process unload queue
    while (!m_chunkUnloadQueue.empty()) {
        ChunkPosition pos = m_chunkUnloadQueue.front();
        m_chunkUnloadQueue.pop();
        UnloadChunk(pos);
    }
}

void World::UpdateChunkLoading(const Vec3& playerPos) {
    int playerChunkX = static_cast<int>(playerPos.x) / CHUNK_SIZE;
    int playerChunkZ = static_cast<int>(playerPos.z) / CHUNK_SIZE;

    // Load chunks in render distance
    for (int x = -m_settings.renderDistance; x <= m_settings.renderDistance; ++x) {
        for (int z = -m_settings.renderDistance; z <= m_settings.renderDistance; ++z) {
            ChunkPosition chunkPos(playerChunkX + x, playerChunkZ + z);

            // Check if in simulation distance (closer chunks)
            int distance = std::max(std::abs(x), std::abs(z));
            if (distance <= m_settings.simulationDistance) {
                if (GetChunk(chunkPos) == nullptr) {
                    m_chunkLoadQueue.push(chunkPos);
                    m_stats.chunksInQueue++;
                }
            }
        }
    }

    // Unload distant chunks
    std::vector<ChunkPosition> chunksToUnload;
    {
        std::lock_guard<std::mutex> lock(m_chunkMutex);
        for (const auto& pair : m_loadedChunks) {
            int distance = pair.first.distance(ChunkPosition(playerChunkX, playerChunkZ));
            if (distance > m_settings.renderDistance + 2) { // Add some hysteresis
                chunksToUnload.push_back(pair.first);
            }
        }
    }

    for (const auto& pos : chunksToUnload) {
        m_chunkUnloadQueue.push(pos);
    }
}

void World::UpdateEntities(float deltaTime) {
    std::lock_guard<std::mutex> lock(m_entityMutex);

    for (auto& entity : m_entities) {
        if (entity) {
            entity->Update(deltaTime);
            CheckEntityCollisions(entity);
        }
    }
}

void World::CheckEntityCollisions(std::shared_ptr<Entity> entity) {
    // Simple collision detection with world blocks
    Vec3 pos = entity->GetPosition();

    // Check block at entity position
    int blockX = static_cast<int>(pos.x);
    int blockY = static_cast<int>(pos.y);
    int blockZ = static_cast<int>(pos.z);

    // Check neighboring blocks for collision
    for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
            for (int dz = -1; dz <= 1; ++dz) {
                if (IsSolidBlock(blockX + dx, blockY + dy, blockZ + dz)) {
                    // Handle collision (simple bounce back)
                    Vec3 newPos = pos;
                    if (dx != 0) newPos.x = blockX + (dx > 0 ? 0.5f : -0.5f);
                    if (dy != 0) newPos.y = blockY + (dy > 0 ? 0.5f : -0.5f);
                    if (dz != 0) newPos.z = blockZ + (dz > 0 ? 0.5f : -0.5f);
                    entity->SetPosition(newPos);
                    return;
                }
            }
        }
    }
}

void World::OnBlockChanged(int worldX, int worldY, int worldZ) {
    // Notify callbacks
    for (const auto& callback : m_eventCallbacks) {
        callback("block_changed", Vec3(static_cast<float>(worldX),
                                      static_cast<float>(worldY),
                                      static_cast<float>(worldZ)));
    }

    // Update lighting if needed
    UpdateLighting();
}

void World::UpdateLighting() {
    // TODO: Implement lighting updates
}

void World::UpdateStats() {
    std::lock_guard<std::mutex> lock(m_chunkMutex);
    m_stats.totalChunks = m_loadedChunks.size();

    // Calculate memory usage (rough estimate)
    m_stats.memoryUsage = m_loadedChunks.size() * sizeof(Chunk) * 16 * 16 * 256; // 16x16x256 blocks per chunk
}

void World::WorldToChunkCoordinates(int worldX, int worldZ,
                                   int& chunkX, int& chunkZ,
                                   int& localX, int& localZ) {
    chunkX = worldX / CHUNK_SIZE;
    chunkZ = worldZ / CHUNK_SIZE;
    localX = worldX % CHUNK_SIZE;
    localZ = worldZ % CHUNK_SIZE;

    // Handle negative coordinates
    if (worldX < 0 && localX != 0) {
        chunkX--;
        localX = CHUNK_SIZE + localX;
    }
    if (worldZ < 0 && localZ != 0) {
        chunkZ--;
        localZ = CHUNK_SIZE + localZ;
    }
}

} // namespace VoxelCraft
