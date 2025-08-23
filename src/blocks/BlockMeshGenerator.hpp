/**
 * @file BlockMeshGenerator.hpp
 * @brief VoxelCraft Block Mesh Generator
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_BLOCKS_BLOCK_MESH_GENERATOR_HPP
#define VOXELCRAFT_BLOCKS_BLOCK_MESH_GENERATOR_HPP

#include <memory>
#include <vector>
#include <array>

namespace VoxelCraft {

    // Forward declarations
    class Chunk;
    class Block;

    /**
     * @struct Vertex
     * @brief Vertex structure for block mesh
     */
    struct Vertex {
        float x, y, z;      // Position
        float u, v;         // Texture coordinates
        float nx, ny, nz;   // Normal
        float r, g, b, a;   // Color

        Vertex() = default;
        Vertex(float x, float y, float z, float u, float v, float nx, float ny, float nz, float r, float g, float b, float a)
            : x(x), y(y), z(z), u(u), v(v), nx(nx), ny(nz), nz(nz), r(r), g(g), b(b), a(a) {}
    };

    /**
     * @struct BlockMesh
     * @brief Generated mesh for a block or chunk
     */
    struct BlockMesh {
        std::vector<Vertex> vertices;      ///< Mesh vertices
        std::vector<unsigned int> indices; ///< Mesh indices
        bool isTransparent;               ///< Transparency flag
        bool needsUpdate;                 ///< Update flag

        BlockMesh() : isTransparent(false), needsUpdate(true) {}
    };

    /**
     * @class BlockMeshGenerator
     * @brief Generates 3D meshes for blocks and chunks
     */
    class BlockMeshGenerator {
    public:
        BlockMeshGenerator();
        ~BlockMeshGenerator();

        /**
         * @brief Initialize the mesh generator
         * @return true if successful
         */
        bool Initialize();

        /**
         * @brief Shutdown the mesh generator
         */
        void Shutdown();

        /**
         * @brief Generate mesh for a single block
         * @param block Block to generate mesh for
         * @param x Block X position
         * @param y Block Y position
         * @param z Block Z position
         * @param metadata Block metadata
         * @return Generated mesh
         */
        BlockMesh GenerateBlockMesh(const Block* block, int x, int y, int z, uint16_t metadata);

        /**
         * @brief Generate mesh for a chunk
         * @param chunk Chunk to generate mesh for
         * @return Generated mesh
         */
        BlockMesh GenerateChunkMesh(const Chunk* chunk);

        /**
         * @brief Optimize mesh for rendering
         * @param mesh Mesh to optimize
         * @return true if optimization successful
         */
        bool OptimizeMesh(BlockMesh& mesh);

        /**
         * @brief Clear mesh cache
         */
        void ClearCache();

    private:
        /**
         * @brief Generate cube mesh for solid blocks
         * @param block Block
         * @param x X position
         * @param y Y position
         * @param z Z position
         * @param metadata Metadata
         * @return Cube mesh
         */
        BlockMesh GenerateCubeMesh(const Block* block, int x, int y, int z, uint16_t metadata);

        /**
         * @brief Generate cross mesh for plants
         * @param block Block
         * @param x X position
         * @param y Y position
         * @param z Z position
         * @param metadata Metadata
         * @return Cross mesh
         */
        BlockMesh GenerateCrossMesh(const Block* block, int x, int y, int z, uint16_t metadata);

        /**
         * @brief Add face to mesh
         * @param mesh Mesh to add to
         * @param face Face index (0-5)
         * @param x X position
         * @param y Y position
         * @param z Z position
         * @param textureName Texture name
         */
        void AddFaceToMesh(BlockMesh& mesh, int face, int x, int y, int z, const std::string& textureName);

        bool m_initialized;                                    ///< Initialization flag
        std::unordered_map<uint32_t, BlockMesh> m_meshCache;  ///< Mesh cache
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_BLOCKS_BLOCK_MESH_GENERATOR_HPP