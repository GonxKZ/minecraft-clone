/**
 * @file Chunk.hpp
 * @brief VoxelCraft Chunk System
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines the Chunk class that represents a 16x16x16 section of the world,
 * including block data, lighting, entities, and optimization features.
 */

#ifndef VOXELCRAFT_WORLD_CHUNK_HPP
#define VOXELCRAFT_WORLD_CHUNK_HPP

#include <memory>
#include <vector>
#include <array>
#include <unordered_set>
#include <atomic>
#include <mutex>
#include <shared_mutex>
#include <optional>
#include <deque>

#include "../core/Config.hpp"
#include "World.hpp"

namespace VoxelCraft {

    // Forward declarations
    class Entity;
    class BlockRegistry;

    /**
     * @enum ChunkState
     * @brief Current state of a chunk
     */
    enum class ChunkState {
        Unloaded,       ///< Chunk not loaded
        Loading,        ///< Chunk is being loaded
        Loaded,         ///< Chunk is loaded and ready
        Generating,     ///< Chunk is generating terrain
        Generated,      ///< Chunk terrain is generated
        Lighting,       ///< Chunk is calculating lighting
        Lit,            ///< Chunk lighting is calculated
        Rendering,      ///< Chunk is preparing for rendering
        Rendered,       ///< Chunk is ready for rendering
        Unloading,      ///< Chunk is being unloaded
        Error           ///< Chunk encountered an error
    };

    /**
     * @enum ChunkLOD
     * @brief Level of Detail for chunk rendering
     */
    enum class ChunkLOD {
        Full,           ///< Full detail (16x16x16)
        Half,           ///< Half detail (8x8x8)
        Quarter,        ///< Quarter detail (4x4x4)
        Eighth,         ///< Eighth detail (2x2x2)
        Minimum         ///< Minimum detail (1x1x1)
    };

    /**
     * @struct ChunkInfo
     * @brief Information about a chunk
     */
    struct ChunkInfo {
        ChunkCoordinate coord;                  ///< Chunk coordinates
        ChunkState state;                       ///< Current chunk state
        ChunkLOD lod;                           ///< Current level of detail
        bool modified;                          ///< Chunk has been modified
        bool needsLightUpdate;                  ///< Chunk needs lighting update
        bool needsMeshUpdate;                   ///< Chunk needs mesh regeneration
        double lastAccessed;                    ///< Last access timestamp
        double lastModified;                    ///< Last modification timestamp
        size_t memoryUsage;                     ///< Memory usage in bytes
        uint32_t blockUpdates;                  ///< Number of block updates
        uint32_t entityCount;                   ///< Number of entities in chunk
        uint8_t maxHeight;                      ///< Maximum height in chunk
        uint8_t minHeight;                      ///< Minimum height in chunk
    };

    /**
     * @class Chunk
     * @brief Represents a 16x16x16 section of the world
     *
     * A chunk contains:
     * - Block data (16x16x16 = 4096 blocks)
     * - Light data (sky light and block light)
     * - Entity references
     * - Mesh data for rendering
     * - Optimization flags
     * - State management
     *
     * Features:
     * - Memory-efficient storage
     * - Fast block access
     * - Lighting calculations
     * - Mesh generation
     * - Level of Detail support
     * - Compression support
     * - Thread-safe operations
     */
    class Chunk {
    public:
        // Constants
        static constexpr int CHUNK_SIZE = 16;           ///< Size of chunk (16x16x16)
        static constexpr int CHUNK_VOLUME = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE; ///< Total blocks
        static constexpr int CHUNK_HEIGHT = 256;        ///< World height (for indexing)

        /**
         * @brief Constructor
         * @param x Chunk X coordinate
         * @param z Chunk Z coordinate
         * @param world Parent world
         */
        Chunk(int x, int z, World* world);

        /**
         * @brief Destructor
         */
        ~Chunk();

        /**
         * @brief Deleted copy constructor
         */
        Chunk(const Chunk&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        Chunk& operator=(const Chunk&) = delete;

        // Chunk identification

        /**
         * @brief Get chunk coordinates
         * @return Chunk coordinate
         */
        ChunkCoordinate GetCoordinate() const { return m_coordinate; }

        /**
         * @brief Get chunk X coordinate
         * @return X coordinate
         */
        int GetX() const { return m_coordinate.x; }

        /**
         * @brief Get chunk Z coordinate
         * @return Z coordinate
         */
        int GetZ() const { return m_coordinate.z; }

        /**
         * @brief Get center position of chunk
         * @return Center position as 3D vector
         */
        glm::vec3 GetCenter() const;

        /**
         * @brief Get bounds of chunk
         * @return AABB bounds (minX, minY, minZ, maxX, maxY, maxZ)
         */
        std::tuple<int, int, int, int, int, int> GetBounds() const;

        // Chunk state

        /**
         * @brief Get chunk state
         * @return Current chunk state
         */
        ChunkState GetState() const { return m_state; }

        /**
         * @brief Set chunk state
         * @param state New chunk state
         */
        void SetState(ChunkState state);

        /**
         * @brief Get chunk LOD
         * @return Current LOD level
         */
        ChunkLOD GetLOD() const { return m_lod; }

        /**
         * @brief Set chunk LOD
         * @param lod New LOD level
         */
        void SetLOD(ChunkLOD lod);

        /**
         * @brief Check if chunk is loaded
         * @return true if loaded, false otherwise
         */
        bool IsLoaded() const { return m_state >= ChunkState::Loaded; }

        /**
         * @brief Check if chunk is ready for rendering
         * @return true if ready, false otherwise
         */
        bool IsReady() const { return m_state >= ChunkState::Rendered; }

        // Block access

        /**
         * @brief Get block at local chunk coordinates
         * @param x Local X coordinate (0-15)
         * @param y Local Y coordinate (0-255)
         * @param z Local Z coordinate (0-15)
         * @return Block ID
         */
        uint32_t GetBlock(int x, int y, int z) const;

        /**
         * @brief Set block at local chunk coordinates
         * @param x Local X coordinate (0-15)
         * @param y Local Y coordinate (0-255)
         * @param z Local Z coordinate (0-15)
         * @param blockId Block ID to set
         * @return true if successful, false otherwise
         */
        bool SetBlock(int x, int y, int z, uint32_t blockId);

        /**
         * @brief Get block at world coordinates (if within chunk)
         * @param worldX World X coordinate
         * @param worldY World Y coordinate
         * @param worldZ World Z coordinate
         * @return Block ID or empty optional if outside chunk
         */
        std::optional<uint32_t> GetBlockAtWorld(int worldX, int worldY, int worldZ) const;

        /**
         * @brief Set block at world coordinates (if within chunk)
         * @param worldX World X coordinate
         * @param worldY World Y coordinate
         * @param worldZ World Z coordinate
         * @param blockId Block ID to set
         * @return true if successful, false if outside chunk
         */
        bool SetBlockAtWorld(int worldX, int worldY, int worldZ, uint32_t blockId);

        /**
         * @brief Get block at block coordinate (if within chunk)
         * @param coord Block coordinate
         * @return Block ID or empty optional if outside chunk
         */
        std::optional<uint32_t> GetBlock(const BlockCoordinate& coord) const;

        /**
         * @brief Set block at block coordinate (if within chunk)
         * @param coord Block coordinate
         * @param blockId Block ID to set
         * @return true if successful, false if outside chunk
         */
        bool SetBlock(const BlockCoordinate& coord, uint32_t blockId);

        // Lighting

        /**
         * @brief Get sky light level at local coordinates
         * @param x Local X coordinate (0-15)
         * @param y Local Y coordinate (0-255)
         * @param z Local Z coordinate (0-15)
         * @return Sky light level (0-15)
         */
        uint8_t GetSkyLight(int x, int y, int z) const;

        /**
         * @brief Set sky light level at local coordinates
         * @param x Local X coordinate (0-15)
         * @param y Local Y coordinate (0-255)
         * @param z Local Z coordinate (0-15)
         * @param level Sky light level (0-15)
         */
        void SetSkyLight(int x, int y, int z, uint8_t level);

        /**
         * @brief Get block light level at local coordinates
         * @param x Local X coordinate (0-15)
         * @param y Local Y coordinate (0-255)
         * @param z Local Z coordinate (0-15)
         * @return Block light level (0-15)
         */
        uint8_t GetBlockLight(int x, int y, int z) const;

        /**
         * @brief Set block light level at local coordinates
         * @param x Local X coordinate (0-15)
         * @param y Local Y coordinate (0-255)
         * @param z Local Z coordinate (0-15)
         * @param level Block light level (0-15)
         */
        void SetBlockLight(int x, int y, int z, uint8_t level);

        /**
         * @brief Get combined light level at local coordinates
         * @param x Local X coordinate (0-15)
         * @param y Local Y coordinate (0-255)
         * @param z Local Z coordinate (0-15)
         * @return Combined light level (0-15)
         */
        uint8_t GetLightLevel(int x, int y, int z) const;

        /**
         * @brief Update lighting for entire chunk
         */
        void UpdateLighting();

        /**
         * @brief Propagate light changes
         * @param x Local X coordinate
         * @param y Local Y coordinate
         * @param z Local Z coordinate
         */
        void PropagateLight(int x, int y, int z);

        // Entity management

        /**
         * @brief Add entity to chunk
         * @param entity Entity to add
         * @return true if added, false otherwise
         */
        bool AddEntity(Entity* entity);

        /**
         * @brief Remove entity from chunk
         * @param entity Entity to remove
         * @return true if removed, false otherwise
         */
        bool RemoveEntity(Entity* entity);

        /**
         * @brief Get entities in chunk
         * @return Vector of entities in chunk
         */
        std::vector<Entity*> GetEntities() const;

        /**
         * @brief Get entities in area
         * @param minX Minimum local X
         * @param minY Minimum local Y
         * @param minZ Minimum local Z
         * @param maxX Maximum local X
         * @param maxY Maximum local Y
         * @param maxZ Maximum local Z
         * @return Vector of entities in area
         */
        std::vector<Entity*> GetEntitiesInArea(int minX, int minY, int minZ,
                                               int maxX, int maxY, int maxZ) const;

        // Chunk operations

        /**
         * @brief Generate terrain for chunk
         * @return true if generation started, false otherwise
         */
        bool GenerateTerrain();

        /**
         * @brief Generate mesh for chunk
         * @return true if generation started, false otherwise
         */
        bool GenerateMesh();

        /**
         * @brief Compress chunk data
         * @return Compression ratio achieved
         */
        float Compress();

        /**
         * @brief Decompress chunk data
         * @return true if successful, false otherwise
         */
        bool Decompress();

        /**
         * @brief Clear chunk data
         */
        void Clear();

        /**
         * @brief Check if chunk is empty (all air blocks)
         * @return true if empty, false otherwise
         */
        bool IsEmpty() const;

        /**
         * @brief Check if chunk is solid (all solid blocks)
         * @return true if solid, false otherwise
         */
        bool IsSolid() const;

        // Serialization

        /**
         * @brief Serialize chunk to data stream
         * @param stream Output stream
         * @return true if successful, false otherwise
         */
        bool Serialize(std::ostream& stream) const;

        /**
         * @brief Deserialize chunk from data stream
         * @param stream Input stream
         * @return true if successful, false otherwise
         */
        bool Deserialize(std::istream& stream);

        // Statistics and info

        /**
         * @brief Get chunk information
         * @return Chunk info
         */
        ChunkInfo GetInfo() const;

        /**
         * @brief Get memory usage
         * @return Memory usage in bytes
         */
        size_t GetMemoryUsage() const;

        /**
         * @brief Get chunk statistics
         * @return Statistics as key-value pairs
         */
        std::unordered_map<std::string, uint64_t> GetStatistics() const;

        // Utility functions

        /**
         * @brief Convert local coordinates to index
         * @param x Local X coordinate (0-15)
         * @param y Local Y coordinate (0-255)
         * @param z Local Z coordinate (0-15)
         * @return Linear index
         */
        static size_t LocalToIndex(int x, int y, int z) {
            return (y * CHUNK_SIZE * CHUNK_SIZE) + (z * CHUNK_SIZE) + x;
        }

        /**
         * @brief Convert index to local coordinates
         * @param index Linear index
         * @return Local coordinates (x, y, z)
         */
        static std::tuple<int, int, int> IndexToLocal(size_t index) {
            int y = index / (CHUNK_SIZE * CHUNK_SIZE);
            int remaining = index % (CHUNK_SIZE * CHUNK_SIZE);
            int z = remaining / CHUNK_SIZE;
            int x = remaining % CHUNK_SIZE;
            return {x, y, z};
        }

        /**
         * @brief Check if coordinates are within chunk bounds
         * @param x Local X coordinate
         * @param y Local Y coordinate
         * @param z Local Z coordinate
         * @return true if within bounds, false otherwise
         */
        static bool IsValidLocalCoordinate(int x, int y, int z) {
            return x >= 0 && x < CHUNK_SIZE &&
                   y >= 0 && y < CHUNK_HEIGHT &&
                   z >= 0 && z < CHUNK_SIZE;
        }

    private:
        /**
         * @brief Initialize chunk data structures
         */
        void Initialize();

        /**
         * @brief Cleanup chunk resources
         */
        void Cleanup();

        /**
         * @brief Allocate block data
         */
        void AllocateBlockData();

        /**
         * @brief Deallocate block data
         */
        void DeallocateBlockData();

        /**
         * @brief Allocate lighting data
         */
        void AllocateLightingData();

        /**
         * @brief Deallocate lighting data
         */
        void DeallocateLightingData();

        /**
         * @brief Mark chunk as modified
         */
        void MarkModified();

        /**
         * @brief Update chunk bounds
         */
        void UpdateBounds();

        /**
         * @brief Compress block data using run-length encoding
         */
        void CompressBlockData();

        /**
         * @brief Decompress block data
         */
        void DecompressBlockData();

        // Chunk data
        ChunkCoordinate m_coordinate;                     ///< Chunk coordinates
        World* m_world;                                   ///< Parent world
        ChunkState m_state;                               ///< Current chunk state
        ChunkLOD m_lod;                                   ///< Current LOD level

        // Block data
        std::unique_ptr<uint32_t[]> m_blocks;             ///< Block data array
        bool m_compressed;                                ///< Data is compressed
        std::vector<uint8_t> m_compressedData;            ///< Compressed block data

        // Lighting data
        std::unique_ptr<uint8_t[]> m_skyLight;            ///< Sky light data
        std::unique_ptr<uint8_t[]> m_blockLight;          ///< Block light data

        // Entity management
        std::vector<Entity*> m_entities;                  ///< Entities in chunk
        mutable std::shared_mutex m_entityMutex;          ///< Entity synchronization

        // Mesh data (for rendering)
        std::vector<float> m_vertexData;                  ///< Vertex data for rendering
        std::vector<uint32_t> m_indexData;                ///< Index data for rendering
        bool m_meshGenerated;                             ///< Mesh has been generated
        bool m_meshDirty;                                 ///< Mesh needs regeneration

        // Chunk properties
        bool m_modified;                                  ///< Chunk has been modified
        bool m_needsLightUpdate;                          ///< Lighting needs update
        bool m_empty;                                     ///< Chunk is empty (all air)
        bool m_solid;                                     ///< Chunk is solid (no air)

        // Bounds
        uint8_t m_maxHeight;                              ///< Maximum height in chunk
        uint8_t m_minHeight;                              ///< Minimum height in chunk

        // Statistics
        uint32_t m_blockUpdates;                          ///< Number of block updates
        double m_lastAccessed;                            ///< Last access timestamp
        double m_lastModified;                            ///< Last modification timestamp
        double m_generationTime;                          ///< Time spent generating terrain
        double m_lightingTime;                            ///< Time spent on lighting

        // Threading
        mutable std::shared_mutex m_dataMutex;            ///< Data access synchronization
        mutable std::shared_mutex m_lightMutex;           ///< Lighting synchronization
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_WORLD_CHUNK_HPP
