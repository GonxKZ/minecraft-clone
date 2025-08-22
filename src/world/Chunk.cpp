/**
 * @file Chunk.cpp
 * @brief VoxelCraft World System - Chunk Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#include "Chunk.hpp"
#include "../blocks/Block.hpp"
#include "../entities/RenderComponent.hpp"
#include "../entities/TransformComponent.hpp"
#include "../entities/Entity.hpp"
#include "Logger.hpp"

#include <algorithm>
#include <random>
#include <cmath>

namespace VoxelCraft {

    Chunk::Chunk(const ChunkPosition& position)
        : m_position(position)
        , m_state(ChunkState::EMPTY)
        , m_isModified(false)
        , m_solidBlockCount(0)
    {
        // Initialize all blocks to air
        m_blocks.fill(BlockType::AIR);
        m_lightLevels.fill(15); // Full sky light initially
        m_biomes.fill("plains"); // Default biome

        VOXELCRAFT_TRACE("Chunk created at position ({}, {})", position.x, position.z);
    }

    Chunk::~Chunk() {
        VOXELCRAFT_TRACE("Chunk destroyed at position ({}, {})", m_position.x, m_position.z);
    }

    BlockType Chunk::GetBlock(const BlockPosition& pos) const {
        if (!pos.isValid()) {
            return BlockType::AIR;
        }

        std::lock_guard<std::mutex> lock(m_chunkMutex);
        return m_blocks[pos.toIndex()];
    }

    bool Chunk::SetBlock(const BlockPosition& pos, BlockType type) {
        if (!pos.isValid()) {
            return false;
        }

        std::lock_guard<std::mutex> lock(m_chunkMutex);

        size_t index = pos.toIndex();
        BlockType oldType = m_blocks[index];

        // Update solid block count
        bool wasSolid = Block::CreateBlock(oldType)->IsSolid();
        bool isSolid = Block::CreateBlock(type)->IsSolid();

        if (wasSolid && !isSolid) {
            m_solidBlockCount--;
        } else if (!wasSolid && isSolid) {
            m_solidBlockCount++;
        }

        m_blocks[index] = type;
        m_isModified = true;

        return true;
    }

    BlockType Chunk::GetBlockAt(int worldX, int worldY, int worldZ) const {
        return GetBlock(WorldToLocal(worldX, worldY, worldZ));
    }

    bool Chunk::SetBlockAt(int worldX, int worldY, int worldZ, BlockType type) {
        return SetBlock(WorldToLocal(worldX, worldY, worldZ), type);
    }

    BlockPosition Chunk::WorldToLocal(int worldX, int worldY, int worldZ) const {
        // Handle negative coordinates correctly
        int localX = (worldX % CHUNK_SIZE + CHUNK_SIZE) % CHUNK_SIZE;
        int localY = (worldY % CHUNK_HEIGHT + CHUNK_HEIGHT) % CHUNK_HEIGHT;
        int localZ = (worldZ % CHUNK_SIZE + CHUNK_SIZE) % CHUNK_SIZE;

        return BlockPosition(
            static_cast<uint8_t>(localX),
            static_cast<uint8_t>(localY),
            static_cast<uint8_t>(localZ)
        );
    }

    bool Chunk::ContainsWorldPosition(int worldX, int worldY, int worldZ) const {
        int chunkWorldX = m_position.x * CHUNK_SIZE;
        int chunkWorldZ = m_position.z * CHUNK_SIZE;

        return worldX >= chunkWorldX && worldX < chunkWorldX + CHUNK_SIZE &&
               worldY >= 0 && worldY < CHUNK_HEIGHT &&
               worldZ >= chunkWorldZ && worldZ < chunkWorldZ + CHUNK_SIZE;
    }

    bool Chunk::GenerateTerrain(int seed) {
        if (GetState() != ChunkState::LOADED) {
            return false;
        }

        SetState(ChunkState::GENERATING);

        VOXELCRAFT_INFO("Generating terrain for chunk ({}, {}) with seed {}",
                       m_position.x, m_position.z, seed);

        // Generate basic terrain
        GenerateBasicTerrain(seed);

        // Generate caves
        GenerateCaves(seed);

        // Generate ores
        GenerateOres(seed);

        // Place vegetation
        PlaceVegetation(seed);

        SetState(ChunkState::GENERATED);
        UpdateSolidBlockCount();

        VOXELCRAFT_INFO("Terrain generated for chunk ({}, {}), {} solid blocks",
                       m_position.x, m_position.z, m_solidBlockCount);

        return true;
    }

    bool Chunk::PopulateChunk(int seed) {
        if (GetState() != ChunkState::GENERATED) {
            return false;
        }

        SetState(ChunkState::POPULATING);

        // Population logic would go here
        // - Generate trees
        // - Place structures
        // - Spawn features

        SetState(ChunkState::POPULATED);

        VOXELCRAFT_TRACE("Chunk ({}, {}) populated", m_position.x, m_position.z);
        return true;
    }

    bool Chunk::CalculateLighting() {
        if (GetState() != ChunkState::POPULATED) {
            return false;
        }

        SetState(ChunkState::LIGHTING);

        // Calculate sky light
        CalculateSkyLight();

        // Calculate block light
        CalculateBlockLight();

        // Propagate light
        PropagateLight();

        SetState(ChunkState::LIGHTED);

        VOXELCRAFT_TRACE("Lighting calculated for chunk ({}, {})", m_position.x, m_position.z);
        return true;
    }

    uint8_t Chunk::GetLightLevel(const BlockPosition& pos) const {
        if (!pos.isValid()) {
            return 15; // Full light outside chunk
        }

        std::lock_guard<std::mutex> lock(m_chunkMutex);
        return m_lightLevels[pos.toIndex()];
    }

    void Chunk::SetLightLevel(const BlockPosition& pos, uint8_t level) {
        if (!pos.isValid()) {
            return;
        }

        std::lock_guard<std::mutex> lock(m_chunkMutex);
        m_lightLevels[pos.toIndex()] = level;
    }

    std::string Chunk::GetBiome(const BlockPosition& pos) const {
        if (pos.x >= CHUNK_SIZE || pos.z >= CHUNK_SIZE) {
            return "plains";
        }

        std::lock_guard<std::mutex> lock(m_chunkMutex);
        return m_biomes[pos.x + pos.z * CHUNK_SIZE];
    }

    void Chunk::SetBiome(const BlockPosition& pos, const std::string& biome) {
        if (pos.x >= CHUNK_SIZE || pos.z >= CHUNK_SIZE) {
            return;
        }

        std::lock_guard<std::mutex> lock(m_chunkMutex);
        m_biomes[pos.x + pos.z * CHUNK_SIZE] = biome;
    }

    void Chunk::Clear() {
        std::lock_guard<std::mutex> lock(m_chunkMutex);
        m_blocks.fill(BlockType::AIR);
        m_lightLevels.fill(15);
        m_isModified = false;
        m_solidBlockCount = 0;
    }

    size_t Chunk::GetMemoryUsage() const {
        return sizeof(Chunk) +
               m_blocks.size() * sizeof(BlockType) +
               m_lightLevels.size() * sizeof(uint8_t) +
               m_biomes.size() * sizeof(std::string);
    }

    std::shared_ptr<RenderComponent> Chunk::CreateRenderComponent() {
        auto renderComponent = std::make_shared<RenderComponent>(nullptr); // No owner entity

        // Create multiple render instances for different block types
        // In a real implementation, this would create optimized mesh data
        // For now, we'll create a simple placeholder

        RenderInstance instance;
        instance.mesh = std::make_shared<Mesh>("chunk_mesh");
        instance.material = std::make_shared<Material>("chunk_material");
        instance.visible = true;

        renderComponent->AddInstance(instance);

        return renderComponent;
    }

    void Chunk::UpdateRenderComponent(std::shared_ptr<RenderComponent> renderComponent) {
        // Update render component with current block data
        // This would rebuild the mesh based on current block states
        // For now, it's a placeholder
    }

    void Chunk::GenerateBasicTerrain(int seed) {
        std::mt19937 rng(seed + m_position.x * 73856093 + m_position.z * 19349669);

        for (uint8_t x = 0; x < CHUNK_SIZE; ++x) {
            for (uint8_t z = 0; z < CHUNK_SIZE; ++z) {
                // Generate height using multiple octaves of noise
                float height = 0.0f;
                float amplitude = 1.0f;
                float frequency = 0.01f;

                for (int octave = 0; octave < 4; ++octave) {
                    float noiseX = (m_position.x * CHUNK_SIZE + x) * frequency;
                    float noiseZ = (m_position.z * CHUNK_SIZE + z) * frequency;

                    // Simple noise function
                    int n = static_cast<int>(noiseX * 1000) + static_cast<int>(noiseZ * 1000);
                    n = (n << 13) ^ n;
                    height += (1.0f - ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f) * amplitude;

                    amplitude *= 0.5f;
                    frequency *= 2.0f;
                }

                height = (height + 1.0f) * 0.5f; // Normalize to 0-1
                int terrainHeight = static_cast<int>(height * 12) + 4; // 4-16 range

                // Set biome
                std::string biome = "plains";
                if (height > 0.7f) {
                    biome = "mountains";
                } else if (height > 0.5f) {
                    biome = "forest";
                } else if (height > 0.3f) {
                    biome = "plains";
                } else {
                    biome = "desert";
                }
                SetBiome(BlockPosition(x, 0, z), biome);

                // Generate terrain
                for (uint8_t y = 0; y < CHUNK_HEIGHT; ++y) {
                    BlockType blockType = BlockType::AIR;

                    if (y < terrainHeight - 3) {
                        blockType = BlockType::STONE;
                    } else if (y < terrainHeight) {
                        if (y == terrainHeight - 1) {
                            if (biome == "desert") {
                                blockType = BlockType::SAND;
                            } else {
                                blockType = BlockType::GRASS_BLOCK;
                            }
                        } else {
                            blockType = BlockType::DIRT;
                        }
                    } else if (y < SEA_LEVEL) {
                        blockType = BlockType::WATER;
                    }

                    SetBlock(BlockPosition(x, y, z), blockType);
                }
            }
        }
    }

    void Chunk::GenerateCaves(int seed) {
        std::mt19937 rng(seed + m_position.x * 49569293 + m_position.z * 82645937);

        // Simple cave generation
        for (int i = 0; i < 5; ++i) { // Generate a few cave systems per chunk
            // Start cave at random position
            uint8_t startX = rng() % CHUNK_SIZE;
            uint8_t startY = rng() % (CHUNK_HEIGHT - 5) + 5; // Avoid surface
            uint8_t startZ = rng() % CHUNK_SIZE;

            // Generate cave tunnel
            uint8_t x = startX, y = startY, z = startZ;
            for (int length = 0; length < 50; ++length) {
                // Carve out blocks
                for (int dx = -1; dx <= 1; ++dx) {
                    for (int dy = -1; dy <= 1; ++dy) {
                        for (int dz = -1; dz <= 1; ++dz) {
                            BlockPosition pos(
                                static_cast<uint8_t>(std::max(0, std::min(15, x + dx))),
                                static_cast<uint8_t>(std::max(0, std::min(15, y + dy))),
                                static_cast<uint8_t>(std::max(0, std::min(15, z + dz)))
                            );

                            BlockType currentBlock = GetBlock(pos);
                            if (currentBlock == BlockType::STONE) {
                                SetBlock(pos, BlockType::AIR);
                            }
                        }
                    }
                }

                // Move in random direction
                x = static_cast<uint8_t>(std::max(0, std::min(15, x + (rng() % 3 - 1))));
                y = static_cast<uint8_t>(std::max(1, std::min(14, y + (rng() % 3 - 1))));
                z = static_cast<uint8_t>(std::max(0, std::min(15, z + (rng() % 3 - 1))));
            }
        }
    }

    void Chunk::GenerateOres(int seed) {
        std::mt19937 rng(seed + m_position.x * 58394723 + m_position.z * 28475923);

        // Generate different ore types with different frequencies
        std::vector<std::pair<BlockType, float>> ores = {
            {BlockType::COAL_ORE, 0.2f},
            {BlockType::IRON_ORE, 0.1f},
            {BlockType::GOLD_ORE, 0.05f},
            {BlockType::DIAMOND_ORE, 0.02f}
        };

        for (const auto& [oreType, frequency] : ores) {
            int numVeins = static_cast<int>(frequency * 100);
            for (int vein = 0; vein < numVeins; ++vein) {
                // Generate ore vein
                uint8_t centerX = rng() % CHUNK_SIZE;
                uint8_t centerY = rng() % (CHUNK_HEIGHT - 10) + 5; // 5-10 range
                uint8_t centerZ = rng() % CHUNK_SIZE;

                int veinSize = rng() % 5 + 3; // 3-7 blocks

                for (int i = 0; i < veinSize; ++i) {
                    uint8_t x = static_cast<uint8_t>(std::max(0, std::min(15, centerX + (rng() % 3 - 1))));
                    uint8_t y = static_cast<uint8_t>(std::max(0, std::min(15, centerY + (rng() % 3 - 1))));
                    uint8_t z = static_cast<uint8_t>(std::max(0, std::min(15, centerZ + (rng() % 3 - 1))));

                    BlockPosition pos(x, y, z);
                    if (GetBlock(pos) == BlockType::STONE) {
                        SetBlock(pos, oreType);
                    }
                }
            }
        }
    }

    void Chunk::PlaceVegetation(int seed) {
        std::mt19937 rng(seed + m_position.x * 17483947 + m_position.z * 98347219);

        for (uint8_t x = 0; x < CHUNK_SIZE; ++x) {
            for (uint8_t z = 0; z < CHUNK_SIZE; ++z) {
                // Find surface level
                uint8_t surfaceY = 0;
                for (uint8_t y = CHUNK_HEIGHT - 1; y > 0; --y) {
                    BlockType block = GetBlock(BlockPosition(x, y, z));
                    if (block != BlockType::AIR && block != BlockType::WATER) {
                        surfaceY = y;
                        break;
                    }
                }

                if (surfaceY > 0 && surfaceY < CHUNK_HEIGHT - 1) {
                    std::string biome = GetBiome(BlockPosition(x, surfaceY, z));

                    // Place vegetation based on biome
                    if (biome == "forest") {
                        if (rng() % 100 < 10) { // 10% chance
                            // Place tree
                            for (uint8_t y = 1; y <= 4; ++y) { // Tree trunk
                                SetBlock(BlockPosition(x, surfaceY + y, z), BlockType::OAK_LOG);
                            }

                            // Place leaves
                            for (int dx = -2; dx <= 2; ++dx) {
                                for (int dz = -2; dz <= 2; ++dz) {
                                    for (int dy = 3; dy <= 5; ++dy) {
                                        uint8_t leafX = static_cast<uint8_t>(std::max(0, std::min(15, x + dx)));
                                        uint8_t leafZ = static_cast<uint8_t>(std::max(0, std::min(15, z + dz)));
                                        uint8_t leafY = static_cast<uint8_t>(std::max(0, std::min(15, surfaceY + dy)));

                                        if (GetBlock(BlockPosition(leafX, leafY, leafZ)) == BlockType::AIR) {
                                            SetBlock(BlockPosition(leafX, leafY, leafZ), BlockType::OAK_LEAVES);
                                        }
                                    }
                                }
                            }
                        }
                    } else if (biome == "plains") {
                        if (rng() % 100 < 5) { // 5% chance
                            // Place grass
                            if (GetBlock(BlockPosition(x, surfaceY + 1, z)) == BlockType::AIR) {
                                // In a real implementation, this would be a separate grass plant block
                                // For now, we'll skip it to keep it simple
                            }
                        }
                    }
                }
            }
        }
    }

    void Chunk::CalculateSkyLight() {
        for (uint8_t x = 0; x < CHUNK_SIZE; ++x) {
            for (uint8_t z = 0; z < CHUNK_SIZE; ++z) {
                uint8_t lightLevel = 15; // Full sky light

                // Find first solid block from top
                for (uint8_t y = CHUNK_HEIGHT - 1; y > 0; --y) {
                    BlockType block = GetBlock(BlockPosition(x, y, z));
                    auto blockPtr = Block::CreateBlock(block);

                    if (blockPtr->IsSolid()) {
                        break; // Block blocks light
                    }

                    SetLightLevel(BlockPosition(x, y, z), lightLevel);
                }

                // Set light level for the blocking block (if any)
                for (uint8_t y = CHUNK_HEIGHT - 1; y > 0; --y) {
                    BlockType block = GetBlock(BlockPosition(x, y, z));
                    auto blockPtr = Block::CreateBlock(block);

                    if (blockPtr->IsSolid()) {
                        SetLightLevel(BlockPosition(x, y, z), 0); // No light inside solid blocks
                        break;
                    }
                }
            }
        }
    }

    void Chunk::CalculateBlockLight() {
        // Find light sources and set their light levels
        for (uint8_t x = 0; x < CHUNK_SIZE; ++x) {
            for (uint8_t y = 0; y < CHUNK_HEIGHT; ++y) {
                for (uint8_t z = 0; z < CHUNK_SIZE; ++z) {
                    BlockType block = GetBlock(BlockPosition(x, y, z));
                    auto blockPtr = Block::CreateBlock(block);

                    if (blockPtr->GetLightLevel() > 0) {
                        SetLightLevel(BlockPosition(x, y, z), blockPtr->GetLightLevel());
                    }
                }
            }
        }
    }

    void Chunk::PropagateLight() {
        // Simple light propagation (could be optimized with flood fill)
        bool changed = true;
        int iterations = 0;
        const int maxIterations = 15; // Max light level

        while (changed && iterations < maxIterations) {
            changed = false;
            iterations++;

            for (uint8_t x = 0; x < CHUNK_SIZE; ++x) {
                for (uint8_t y = 0; y < CHUNK_HEIGHT; ++y) {
                    for (uint8_t z = 0; z < CHUNK_SIZE; ++z) {
                        BlockPosition pos(x, y, z);
                        uint8_t currentLight = GetLightLevel(pos);

                        if (currentLight <= 1) {
                            continue; // Can't propagate further
                        }

                        auto blockPtr = Block::CreateBlock(GetBlock(pos));
                        uint8_t lightReduction = static_cast<uint8_t>(blockPtr->GetLightOpacity());

                        // Check neighbors
                        std::vector<BlockPosition> neighbors = {
                            BlockPosition(std::max(0, x-1), y, z),
                            BlockPosition(std::min(15, x+1), y, z),
                            BlockPosition(x, std::max(0, y-1), z),
                            BlockPosition(x, std::min(15, y+1), z),
                            BlockPosition(x, y, std::max(0, z-1)),
                            BlockPosition(x, y, std::min(15, z+1))
                        };

                        for (const auto& neighbor : neighbors) {
                            uint8_t neighborLight = GetLightLevel(neighbor);
                            uint8_t newLight = currentLight - lightReduction;

                            if (newLight > neighborLight) {
                                SetLightLevel(neighbor, newLight);
                                changed = true;
                            }
                        }
                    }
                }
            }
        }
    }

    void Chunk::UpdateSolidBlockCount() {
        m_solidBlockCount = 0;
        for (const auto& block : m_blocks) {
            auto blockPtr = Block::CreateBlock(block);
            if (blockPtr->IsSolid()) {
                m_solidBlockCount++;
            }
        }
    }

} // namespace VoxelCraft

