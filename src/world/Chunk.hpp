/**
 * @file Chunk.hpp
 * @brief VoxelCraft World System - Chunk Implementation (Minecraft-like)
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * Chunk system for infinite world generation (16x16x16 blocks per chunk)
 */

#ifndef VOXELCRAFT_WORLD_CHUNK_HPP
#define VOXELCRAFT_WORLD_CHUNK_HPP

#include <array>
#include <vector>
#include <memory>
#include <unordered_set>
#include <atomic>
#include <mutex>

#include "../blocks/Block.hpp"

namespace VoxelCraft {

    // Forward declarations
    struct Vec3;
    class EntityManager;
    class RenderComponent;

    /**
     * @struct ChunkPosition
     * @brief Position of a chunk in the world (chunk coordinates, not block coordinates)
     */
    struct ChunkPosition {
        int x, z;  // Chunk coordinates (each chunk is 16x16x16 blocks)

        ChunkPosition() : x(0), z(0) {}
        ChunkPosition(int x_, int z_) : x(x_), z(z_) {}

        bool operator==(const ChunkPosition& other) const {
            return x == other.x && z == other.z;
        }

        bool operator!=(const ChunkPosition& other) const {
            return !(*this == other);
        }

        ChunkPosition operator+(const ChunkPosition& other) const {
            return ChunkPosition(x + other.x, z + other.z);
        }

        ChunkPosition operator-(const ChunkPosition& other) const {
            return ChunkPosition(x - other.x, z - other.z);
        }

        // Hash function for use in unordered containers
        struct Hash {
            size_t operator()(const ChunkPosition& pos) const {
                return std::hash<int>()(pos.x) ^ (std::hash<int>()(pos.z) << 1);
            }
        };

        // Manhattan distance
        int distance(const ChunkPosition& other) const {
            return std::abs(x - other.x) + std::abs(z - other.z);
        }

        // Chebyshev distance (max of x and z difference)
        int chebyshevDistance(const ChunkPosition& other) const {
            return std::max(std::abs(x - other.x), std::abs(z - other.z));
        }
    };

    /**
     * @enum ChunkState
     * @brief Current state of a chunk
     */
    enum class ChunkState {
        EMPTY,           // Chunk not loaded
        LOADING,         // Currently being loaded
        LOADED,          // Loaded but not generated
        GENERATING,      // Terrain being generated
        GENERATED,       // Terrain generated
        POPULATING,      // Being populated with structures/features
        POPULATED,       // Populated with structures
        LIGHTING,        // Light calculations in progress
        LIGHTED,         // Light calculations complete
        RENDERING,       // Render data being prepared
        RENDER_READY,    // Ready for rendering
        UNLOADING        // Being unloaded
    };

    /**
     * @struct BlockPosition
     * @brief Position of a block within a chunk (0-15 for x, y, z)
     */
    struct BlockPosition {
        uint8_t x, y, z;  // Local chunk coordinates (0-15)

        BlockPosition() : x(0), y(0), z(0) {}
        BlockPosition(uint8_t x_, uint8_t y_, uint8_t z_) : x(x_), y(y_), z(z_) {}

        bool operator==(const BlockPosition& other) const {
            return x == other.x && y == other.y && z == other.z;
        }

        bool operator!=(const BlockPosition& other) const {
            return !(*this == other);
        }

        // Convert to world position given chunk position
        Vec3 toWorldPosition(const ChunkPosition& chunkPos) const {
            return Vec3(
                static_cast<float>(chunkPos.x * 16 + x),
                static_cast<float>(y),
                static_cast<float>(chunkPos.z * 16 + z)
            );
        }

        // Check if position is within chunk bounds
        bool isValid() const {
            return x < 16 && y < 16 && z < 16;
        }

        // Linear index for array storage
        size_t toIndex() const {
            return x + (y * 16) + (z * 16 * 16);
        }

        // Create from index
        static BlockPosition fromIndex(size_t index) {
            uint8_t z = index / (16 * 16);
            index %= (16 * 16);
            uint8_t y = index / 16;
            uint8_t x = index % 16;
            return BlockPosition(x, y, z);
        }
    };

    /**
     * @class Chunk
     * @brief 16x16x16 block chunk representing part of the world (Minecraft-like)
     */
    class Chunk {
    public:
        // Constants
        static constexpr int CHUNK_SIZE = 16;
        static constexpr int CHUNK_HEIGHT = 16;
        static constexpr int CHUNK_VOLUME = CHUNK_SIZE * CHUNK_HEIGHT * CHUNK_SIZE;
        static constexpr int SEA_LEVEL = 8;  // Sea level in chunk coordinates

        /**
         * @brief Constructor
         * @param position Chunk position in world
         */
        explicit Chunk(const ChunkPosition& position);

        /**
         * @brief Destructor
         */
        ~Chunk();

        /**
         * @brief Get chunk position
         * @return Chunk position
         */
        const ChunkPosition& GetPosition() const { return m_position; }

        /**
         * @brief Get chunk state
         * @return Current chunk state
         */
        ChunkState GetState() const { return m_state.load(); }

        /**
         * @brief Set chunk state
         * @param state New state
         */
        void SetState(ChunkState state) { m_state.store(state); }

        /**
         * @brief Get block at local position
         * @param pos Local block position (0-15)
         * @return Block type at position
         */
        BlockType GetBlock(const BlockPosition& pos) const;

        /**
         * @brief Set block at local position
         * @param pos Local block position (0-15)
         * @param type New block type
         * @return true if block was set, false if position invalid
         */
        bool SetBlock(const BlockPosition& pos, BlockType type);

        /**
         * @brief Get block at world position
         * @param worldX World X coordinate
         * @param worldY World Y coordinate
         * @param worldZ World Z coordinate
         * @return Block type at position
         */
        BlockType GetBlockAt(int worldX, int worldY, int worldZ) const;

        /**
         * @brief Set block at world position
         * @param worldX World X coordinate
         * @param worldY World Y coordinate
         * @param worldZ World Z coordinate
         * @param type New block type
         * @return true if block was set, false if position invalid
         */
        bool SetBlockAt(int worldX, int worldY, int worldZ, BlockType type);

        /**
         * @brief Convert world position to local chunk position
         * @param worldX World X coordinate
         * @param worldY World Y coordinate
         * @param worldZ World Z coordinate
         * @return Local block position
         */
        BlockPosition WorldToLocal(int worldX, int worldY, int worldZ) const;

        /**
         * @brief Check if chunk contains world position
         * @param worldX World X coordinate
         * @param worldY World Y coordinate
         * @param worldZ World Z coordinate
         * @return true if position is within this chunk
         */
        bool ContainsWorldPosition(int worldX, int worldY, int worldZ) const;

        /**
         * @brief Generate terrain for this chunk
         * @param seed World seed
         * @return true if generation successful
         */
        bool GenerateTerrain(int seed);

        /**
         * @brief Populate chunk with structures and features
         * @param seed World seed
         * @return true if population successful
         */
        bool PopulateChunk(int seed);

        /**
         * @brief Calculate lighting for the chunk
         * @return true if lighting calculation successful
         */
        bool CalculateLighting();

        /**
         * @brief Get light level at position
         * @param pos Local block position
         * @return Light level (0-15)
         */
        uint8_t GetLightLevel(const BlockPosition& pos) const;

        /**
         * @brief Set light level at position
         * @param pos Local block position
         * @param level Light level (0-15)
         */
        void SetLightLevel(const BlockPosition& pos, uint8_t level);

        /**
         * @brief Check if chunk is modified (has changes from generation)
         * @return true if chunk has been modified
         */
        bool IsModified() const { return m_isModified; }

        /**
         * @brief Mark chunk as modified
         */
        void SetModified(bool modified = true) { m_isModified = modified; }

        /**
         * @brief Get number of solid blocks in chunk
         * @return Number of solid blocks
         */
        size_t GetSolidBlockCount() const { return m_solidBlockCount; }

        /**
         * @brief Check if chunk is empty (only air blocks)
         * @return true if chunk is empty
         */
        bool IsEmpty() const { return m_solidBlockCount == 0; }

        /**
         * @brief Get biome at position
         * @param pos Local block position
         * @return Biome identifier
         */
        std::string GetBiome(const BlockPosition& pos) const;

        /**
         * @brief Set biome at position
         * @param pos Local block position
         * @param biome Biome identifier
         */
        void SetBiome(const BlockPosition& pos, const std::string& biome);

        /**
         * @brief Clear all blocks (set to air)
         */
        void Clear();

        /**
         * @brief Get memory usage of chunk
         * @return Memory usage in bytes
         */
        size_t GetMemoryUsage() const;

        /**
         * @brief Create render component for this chunk
         * @return Render component for the chunk
         */
        std::shared_ptr<RenderComponent> CreateRenderComponent();

        /**
         * @brief Update render component with current block data
         * @param renderComponent Render component to update
         */
        void UpdateRenderComponent(std::shared_ptr<RenderComponent> renderComponent);

    private:
        ChunkPosition m_position;                          ///< Chunk position in world
        std::atomic<ChunkState> m_state;                   ///< Current chunk state
        std::array<BlockType, CHUNK_VOLUME> m_blocks;      ///< Block data (16x16x16)
        std::array<uint8_t, CHUNK_VOLUME> m_lightLevels;   ///< Light levels (16x16x16)
        std::array<std::string, CHUNK_SIZE * CHUNK_SIZE> m_biomes; ///< Biome data (16x16)

        mutable std::mutex m_chunkMutex;                   ///< Thread safety for chunk operations
        bool m_isModified;                                 ///< Modification flag
        size_t m_solidBlockCount;                          ///< Number of solid blocks

        /**
         * @brief Generate basic terrain (hills, valleys)
         * @param seed World seed
         */
        void GenerateBasicTerrain(int seed);

        /**
         * @brief Generate caves and underground structures
         * @param seed World seed
         */
        void GenerateCaves(int seed);

        /**
         * @brief Generate ore deposits
         * @param seed World seed
         */
        void GenerateOres(int seed);

        /**
         * @brief Place trees and vegetation
         * @param seed World seed
         */
        void PlaceVegetation(int seed);

        /**
         * @brief Calculate sky light for the chunk
         */
        void CalculateSkyLight();

        /**
         * @brief Calculate block light from light sources
         */
        void CalculateBlockLight();

        /**
         * @brief Propagate light through the chunk
         */
        void PropagateLight();

        /**
         * @brief Update solid block count
         */
        void UpdateSolidBlockCount();
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_WORLD_CHUNK_HPP