/**
 * @file BlockMeshGenerator.hpp
 * @brief VoxelCraft Block Mesh Generation System
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines the BlockMeshGenerator class that creates optimized
 * 3D geometry for voxel blocks with advanced features like greedy meshing,
 * ambient occlusion, and texture atlas integration.
 */

#ifndef VOXELCRAFT_BLOCKS_BLOCK_MESH_GENERATOR_HPP
#define VOXELCRAFT_BLOCKS_BLOCK_MESH_GENERATOR_HPP

#include <memory>
#include <vector>
#include <array>
#include <unordered_map>
#include <unordered_set>
#include <deque>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <atomic>

#include "../core/Config.hpp"
#include "Block.hpp"
#include "BlockRegistry.hpp"
#include "../world/Chunk.hpp"

namespace VoxelCraft {

    // Forward declarations
    class World;
    class Chunk;

    /**
     * @enum MeshGenerationStrategy
     * @brief Strategy for mesh generation
     */
    enum class MeshGenerationStrategy {
        Simple,              ///< Simple per-block mesh generation
        Greedy,              ///< Greedy meshing for optimal geometry
        AdvancedGreedy,      ///< Advanced greedy with occlusion culling
        Hybrid               ///< Hybrid approach based on block type
    };

    /**
     * @enum MeshOptimizationLevel
     * @brief Level of mesh optimization
     */
    enum class MeshOptimizationLevel {
        None,                ///< No optimization
        Basic,               ///< Basic optimizations
        Advanced,            ///< Advanced optimizations
        Maximum              ///< Maximum optimization (slowest)
    };

    /**
     * @enum VertexFormat
     * @brief Vertex attribute formats
     */
    enum class VertexFormat {
        PositionOnly,        ///< Position only (for shadows)
        Basic,               ///< Position, normal, UV
        Extended,            ///< Position, normal, UV, tangent, color
        Full                ///< All attributes including custom data
    };

    /**
     * @struct MeshVertex
     * @brief Vertex structure for block meshes
     */
    struct MeshVertex {
        glm::vec3 position;       ///< Vertex position
        glm::vec3 normal;         ///< Vertex normal
        glm::vec2 uv;            ///< Texture coordinates
        glm::vec3 tangent;       ///< Tangent vector
        glm::vec3 color;         ///< Vertex color
        uint32_t blockData;      ///< Block-specific data

        MeshVertex() = default;

        MeshVertex(const glm::vec3& pos, const glm::vec3& norm,
                  const glm::vec2& texCoord, const glm::vec3& tan = glm::vec3(0.0f),
                  const glm::vec3& col = glm::vec3(1.0f), uint32_t data = 0)
            : position(pos), normal(norm), uv(texCoord),
              tangent(tan), color(col), blockData(data) {}
    };

    /**
     * @struct MeshFace
     * @brief Face information for mesh generation
     */
    struct MeshFace {
        BlockFace face;           ///< Face direction
        int x, y, z;             ///< Block coordinates
        int width, height;       ///< Face dimensions (for greedy meshing)
        BlockID blockId;         ///< Block ID
        BlockMetadata metadata;  ///< Block metadata
        uint8_t lightLevel;      ///< Light level
        uint8_t skyLightLevel;   ///< Sky light level
        bool isVisible;          ///< Face visibility
        bool isOccluded;         ///< Face occlusion
    };

    /**
     * @struct MeshSection
     * @brief Section of a mesh with specific material properties
     */
    struct MeshSection {
        std::string materialName;     ///< Material name
        std::vector<MeshVertex> vertices;    ///< Vertices in this section
        std::vector<uint32_t> indices;       ///< Indices for this section
        bool usesTransparency;      ///< Section uses transparency
        bool usesAnimation;         ///< Section uses texture animation
        uint32_t textureIndex;      ///< Texture atlas index
        size_t vertexOffset;        ///< Offset in vertex buffer
        size_t indexOffset;         ///< Offset in index buffer
    };

    /**
     * @struct GeneratedMesh
     * @brief Complete generated mesh for a chunk
     */
    struct GeneratedMesh {
        std::vector<MeshSection> sections;      ///< Mesh sections by material
        std::vector<MeshVertex> allVertices;    ///< All vertices (for simple rendering)
        std::vector<uint32_t> allIndices;       ///< All indices (for simple rendering)
        size_t totalVertices;       ///< Total vertex count
        size_t totalIndices;        ///< Total index count
        size_t totalTriangles;      ///< Total triangle count
        bool isEmpty;              ///< Mesh is empty
        double generationTime;      ///< Time taken to generate
        size_t memoryUsage;         ///< Memory usage estimate
        std::unordered_map<std::string, std::any> metadata; ///< Generation metadata
    };

    /**
     * @struct MeshGenerationConfig
     * @brief Configuration for mesh generation
     */
    struct MeshGenerationConfig {
        // Basic settings
        MeshGenerationStrategy strategy;        ///< Generation strategy
        MeshOptimizationLevel optimization;      ///< Optimization level
        VertexFormat vertexFormat;              ///< Vertex format
        bool enableAmbientOcclusion;            ///< Enable ambient occlusion
        bool enableSmoothLighting;              ///< Enable smooth lighting
        bool enableGreedyMeshing;               ///< Enable greedy meshing

        // Performance settings
        int maxVerticesPerSection;              ///< Maximum vertices per section
        int maxIndicesPerSection;               ///< Maximum indices per section
        size_t maxMeshSize;                     ///< Maximum mesh size (bytes)
        bool useMultithreading;                 ///< Enable multithreaded generation

        // Quality settings
        int ambientOcclusionLevel;              ///< AO quality level (1-3)
        float aoStrength;                       ///< Ambient occlusion strength
        bool enableNormalMapping;               ///< Enable normal mapping
        bool enableParallaxMapping;             ///< Enable parallax mapping

        // Advanced settings
        bool enableMeshCompression;             ///< Enable mesh compression
        bool enableLevelOfDetail;               ///< Enable LOD generation
        int lodLevels;                          ///< Number of LOD levels
        bool enableInstancing;                  ///< Enable geometry instancing
        bool enableFrustumCulling;              ///< Enable frustum culling
    };

    /**
     * @struct MeshGenerationMetrics
     * @brief Performance metrics for mesh generation
     */
    struct MeshGenerationMetrics {
        uint64_t totalMeshesGenerated;           ///< Total meshes generated
        double totalGenerationTime;              ///< Total generation time
        double averageGenerationTime;            ///< Average generation time
        double maxGenerationTime;                ///< Maximum generation time
        double minGenerationTime;                ///< Minimum generation time
        size_t totalVerticesGenerated;            ///< Total vertices generated
        size_t totalIndicesGenerated;             ///< Total indices generated
        size_t totalTrianglesGenerated;           ///< Total triangles generated
        uint64_t greedyMerges;                   ///< Number of greedy merges
        uint64_t occlusionCulls;                 ///< Number of occlusion culls
        uint64_t aoCalculations;                 ///< Number of AO calculations
        double averageVerticesPerSecond;         ///< Generation throughput
        double meshCompressionRatio;             ///< Mesh compression ratio
        size_t peakMemoryUsage;                  ///< Peak memory usage
    };

    /**
     * @class GreedyMesher
     * @brief Implements greedy mesh optimization algorithm
     */
    class GreedyMesher {
    public:
        /**
         * @brief Generate greedy mesh for a face
         * @param chunk Chunk to process
         * @param face Face direction
         * @param vertices Output vertices
         * @param indices Output indices
         * @return Number of quads generated
         */
        static size_t GenerateFaceMesh(Chunk* chunk, BlockFace face,
                                     std::vector<MeshVertex>& vertices,
                                     std::vector<uint32_t>& indices);

        /**
         * @brief Check if two adjacent faces can be merged
         * @param chunk Chunk containing the faces
         * @param face1 First face
         * @param face2 Second face
         * @return true if can be merged, false otherwise
         */
        static bool CanMergeFaces(Chunk* chunk, const MeshFace& face1, const MeshFace& face2);

        /**
         * @brief Merge two faces into one
         * @param face1 First face (will be modified)
         * @param face2 Second face
         */
        static void MergeFaces(MeshFace& face1, const MeshFace& face2);

    private:
        /**
         * @brief Check face visibility
         * @param chunk Chunk to check
         * @param x Block X coordinate
         * @param y Block Y coordinate
         * @param z Block Z coordinate
         * @param face Face direction
         * @return true if visible, false otherwise
         */
        static bool IsFaceVisible(Chunk* chunk, int x, int y, int z, BlockFace face);

        /**
         * @brief Check face occlusion
         * @param chunk Chunk to check
         * @param x Block X coordinate
         * @param y Block Y coordinate
         * @param z Block Z coordinate
         * @param face Face direction
         * @return true if occluded, false otherwise
         */
        static bool IsFaceOccluded(Chunk* chunk, int x, int y, int z, BlockFace face);
    };

    /**
     * @class AmbientOcclusionCalculator
     * @brief Calculates ambient occlusion for block vertices
     */
    class AmbientOcclusionCalculator {
    public:
        /**
         * @brief Calculate ambient occlusion for a face
         * @param chunk Chunk containing the face
         * @param x Block X coordinate
         * @param y Block Y coordinate
         * @param z Block Z coordinate
         * @param face Face direction
         * @param aoLevel Ambient occlusion level (1-3)
         * @return Array of 4 AO values for face vertices
         */
        static std::array<float, 4> CalculateFaceAO(Chunk* chunk, int x, int y, int z,
                                                   BlockFace face, int aoLevel = 2);

        /**
         * @brief Calculate single vertex AO
         * @param chunk Chunk to check
         * @param x Block X coordinate
         * @param y Block Y coordinate
         * @param z Block Z coordinate
         * @param face Face direction
         * @param corner Corner index (0-3)
         * @param aoLevel Ambient occlusion level
         * @return AO value (0.0 - 1.0)
         */
        static float CalculateVertexAO(Chunk* chunk, int x, int y, int z,
                                      BlockFace face, int corner, int aoLevel = 2);

        /**
         * @brief Check if position blocks light
         * @param chunk Chunk to check
         * @param x Block X coordinate
         * @param y Block Y coordinate
         * @param z Block Z coordinate
         * @return true if blocks light, false otherwise
         */
        static bool BlocksLight(Chunk* chunk, int x, int y, int z);

    private:
        /**
         * @brief Get corner directions for AO calculation
         * @param face Face direction
         * @param corner Corner index (0-3)
         * @return Array of 3 direction vectors
         */
        static std::array<glm::vec3, 3> GetCornerDirections(BlockFace face, int corner);
    };

    /**
     * @class BlockMeshGenerator
     * @brief Main mesh generator for voxel blocks
     *
     * The BlockMeshGenerator creates optimized 3D geometry for chunks by:
     * - Using greedy meshing to reduce vertex count
     * - Calculating ambient occlusion for realistic lighting
     * - Implementing smooth lighting between blocks
     * - Supporting multiple vertex formats
     * - Generating LOD (Level of Detail) meshes
     * - Optimizing for texture atlases
     *
     * Key Features:
     * - Multiple generation strategies
     * - Advanced optimization techniques
     * - Memory-efficient mesh storage
     * - Multi-threaded generation
     * - Real-time mesh updates
     * - Support for transparent and animated blocks
     */
    class BlockMeshGenerator {
    public:
        /**
         * @brief Constructor
         * @param config Mesh generation configuration
         */
        explicit BlockMeshGenerator(const MeshGenerationConfig& config);

        /**
         * @brief Destructor
         */
        ~BlockMeshGenerator();

        /**
         * @brief Deleted copy constructor
         */
        BlockMeshGenerator(const BlockMeshGenerator&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        BlockMeshGenerator& operator=(const BlockMeshGenerator&) = delete;

        // Mesh generation

        /**
         * @brief Generate mesh for entire chunk
         * @param chunk Chunk to generate mesh for
         * @return Generated mesh
         */
        std::unique_ptr<GeneratedMesh> GenerateChunkMesh(Chunk* chunk);

        /**
         * @brief Generate mesh for specific block
         * @param chunk Chunk containing the block
         * @param x Block X coordinate (local)
         * @param y Block Y coordinate (local)
         * @param z Block Z coordinate (local)
         * @param vertices Output vertices
         * @param indices Output indices
         * @return Number of vertices generated
         */
        size_t GenerateBlockMesh(Chunk* chunk, int x, int y, int z,
                               std::vector<MeshVertex>& vertices,
                               std::vector<uint32_t>& indices);

        /**
         * @brief Generate LOD mesh for chunk
         * @param chunk Chunk to generate LOD for
         * @param lodLevel Level of detail (0 = highest)
         * @return Generated LOD mesh
         */
        std::unique_ptr<GeneratedMesh> GenerateChunkLOD(Chunk* chunk, int lodLevel);

        /**
         * @brief Update mesh for modified block
         * @param chunk Chunk containing the block
         * @param x Block X coordinate (local)
         * @param y Block Y coordinate (local)
         * @param z Block Z coordinate (local)
         * @param mesh Existing mesh to update
         * @return true if mesh was updated, false otherwise
         */
        bool UpdateBlockInMesh(Chunk* chunk, int x, int y, int z, GeneratedMesh* mesh);

        // Configuration

        /**
         * @brief Get mesh generation configuration
         * @return Current configuration
         */
        const MeshGenerationConfig& GetConfig() const { return m_config; }

        /**
         * @brief Set mesh generation configuration
         * @param config New configuration
         */
        void SetConfig(const MeshGenerationConfig& config);

        /**
         * @brief Get mesh generation metrics
         * @return Performance metrics
         */
        const MeshGenerationMetrics& GetMetrics() const { return m_metrics; }

        /**
         * @brief Reset metrics
         */
        void ResetMetrics();

        // Optimization methods

        /**
         * @brief Optimize existing mesh
         * @param mesh Mesh to optimize
         * @return Optimization results
         */
        std::unordered_map<std::string, size_t> OptimizeMesh(GeneratedMesh* mesh);

        /**
         * @brief Compress mesh data
         * @param mesh Mesh to compress
         * @return Compression ratio achieved
         */
        float CompressMesh(GeneratedMesh* mesh);

        /**
         * @brief Decompress mesh data
         * @param mesh Mesh to decompress
         * @return true if successful, false otherwise
         */
        bool DecompressMesh(GeneratedMesh* mesh);

        // Utility functions

        /**
         * @brief Calculate face visibility mask
         * @param chunk Chunk to check
         * @param x Block X coordinate
         * @param y Block Y coordinate
         * @param z Block Z coordinate
         * @return Face visibility mask (6 bits, one per face)
         */
        uint8_t CalculateFaceVisibilityMask(Chunk* chunk, int x, int y, int z);

        /**
         * @brief Get texture coordinates for block face
         * @param block Block to get texture for
         * @param face Face direction
         * @param metadata Block metadata
         * @return Texture coordinates
         */
        BlockTextureCoords GetFaceTextureCoords(const Block* block, BlockFace face, BlockMetadata metadata);

        /**
         * @brief Calculate lighting color for vertex
         * @param chunk Chunk containing the vertex
         * @param x Block X coordinate
         * @param y Block Y coordinate
         * @param z Block Z coordinate
         * @param face Face direction
         * @param ao Ambient occlusion value
         * @return Final vertex color
         */
        glm::vec3 CalculateVertexLighting(Chunk* chunk, int x, int y, int z,
                                        BlockFace face, float ao);

        // Face generation

        /**
         * @brief Generate vertices for a single face
         * @param chunk Chunk containing the face
         * @param x Block X coordinate
         * @param y Block Y coordinate
         * @param z Block Z coordinate
         * @param face Face direction
         * @param width Face width (for greedy meshing)
         * @param height Face height (for greedy meshing)
         * @param vertices Output vertices
         * @param indices Output indices
         * @param vertexOffset Current vertex offset
         * @return Number of vertices generated
         */
        size_t GenerateFaceVertices(Chunk* chunk, int x, int y, int z,
                                  BlockFace face, int width, int height,
                                  std::vector<MeshVertex>& vertices,
                                  std::vector<uint32_t>& indices,
                                  size_t vertexOffset);

        // Memory management

        /**
         * @brief Get total memory usage
         * @return Memory usage in bytes
         */
        size_t GetMemoryUsage() const;

        /**
         * @brief Clear mesh cache
         */
        void ClearCache();

        /**
         * @brief Get cache statistics
         * @return Cache statistics
         */
        std::unordered_map<std::string, size_t> GetCacheStatistics() const;

    private:
        /**
         * @brief Initialize mesh generation
         */
        void Initialize();

        /**
         * @brief Update generation metrics
         * @param mesh Generated mesh
         * @param generationTime Time taken to generate
         */
        void UpdateMetrics(const GeneratedMesh* mesh, double generationTime);

        /**
         * @brief Create mesh section for material
         * @param materialName Material name
         * @return New mesh section
         */
        MeshSection& GetOrCreateSection(std::vector<MeshSection>& sections, const std::string& materialName);

        /**
         * @brief Finalize mesh sections
         * @param sections Mesh sections to finalize
         * @param mesh Generated mesh
         */
        void FinalizeMeshSections(std::vector<MeshSection>& sections, GeneratedMesh* mesh);

        /**
         * @brief Add vertex to mesh with optimization
         * @param vertices Vertex list
         * @param vertex Vertex to add
         * @return Index of added vertex
         */
        size_t AddOptimizedVertex(std::vector<MeshVertex>& vertices, const MeshVertex& vertex);

        // Configuration
        MeshGenerationConfig m_config;

        // Metrics
        MeshGenerationMetrics m_metrics;
        mutable std::shared_mutex m_metricsMutex;

        // Cache for generated block meshes
        std::unordered_map<uint64_t, std::pair<std::vector<MeshVertex>, std::vector<uint32_t>>> m_blockMeshCache;
        mutable std::shared_mutex m_cacheMutex;

        // Temporary buffers for generation
        std::vector<MeshVertex> m_tempVertices;
        std::vector<uint32_t> m_tempIndices;
        std::vector<bool> m_faceVisibilityBuffer; // Reuse buffer for face visibility

        // Constants for optimization
        static constexpr size_t MAX_VERTICES_PER_MESH = 1000000; // 1M vertices
        static constexpr size_t MAX_INDICES_PER_MESH = 2000000;  // 2M indices
        static constexpr float AO_STRENGTH_DEFAULT = 0.25f;      // Default AO strength
    };

    /**
     * @class MeshGeneratorFactory
     * @brief Factory for creating mesh generators with different configurations
     */
    class MeshGeneratorFactory {
    public:
        /**
         * @brief Create default mesh generator
         * @return Default mesh generator
         */
        static std::unique_ptr<BlockMeshGenerator> CreateDefaultGenerator();

        /**
         * @brief Create high-quality mesh generator
         * @return High-quality mesh generator
         */
        static std::unique_ptr<BlockMeshGenerator> CreateHighQualityGenerator();

        /**
         * @brief Create fast mesh generator
         * @return Fast mesh generator
         */
        static std::unique_ptr<BlockMeshGenerator> CreateFastGenerator();

        /**
         * @brief Create custom mesh generator
         * @param config Custom configuration
         * @return Custom mesh generator
         */
        static std::unique_ptr<BlockMeshGenerator> CreateCustomGenerator(const MeshGenerationConfig& config);

        /**
         * @brief Get default configuration
         * @return Default configuration
         */
        static MeshGenerationConfig GetDefaultConfig();

        /**
         * @brief Get high-quality configuration
         * @return High-quality configuration
         */
        static MeshGenerationConfig GetHighQualityConfig();

        /**
         * @brief Get fast configuration
         * @return Fast configuration
         */
        static MeshGenerationConfig GetFastConfig();
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_BLOCKS_BLOCK_MESH_GENERATOR_HPP
