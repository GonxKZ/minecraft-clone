/**
 * @file ProceduralModelGenerator.hpp
 * @brief VoxelCraft Procedural 3D Model Generation System
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines the ProceduralModelGenerator class that provides comprehensive
 * procedural 3D model generation for the VoxelCraft game engine, including L-systems
 * for plants, fractal geometry for terrain, cellular automata for caves, and advanced
 * mesh generation with real-time generation capabilities for infinite content variety.
 */

#ifndef VOXELCRAFT_PROCEDURAL_PROCEDURAL_MODEL_GENERATOR_HPP
#define VOXELCRAFT_PROCEDURAL_PROCEDURAL_MODEL_GENERATOR_HPP

#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <chrono>
#include <optional>
#include <queue>
#include <deque>
#include <stack>
#include <array>
#include <random>
#include <algorithm>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/noise.hpp>

#include "../core/Config.hpp"

namespace VoxelCraft {

    // Forward declarations
    class ProceduralAssetGenerator;
    struct Vertex;
    struct Triangle;
    struct MeshData;

    /**
     * @enum ModelType
     * @brief Types of procedural models
     */
    enum class ModelType {
        Plant,                      ///< Plants and vegetation
        Tree,                       ///< Trees and large vegetation
        Rock,                       ///< Rocks and boulders
        Terrain,                    ///< Terrain features
        Cave,                       ///< Cave systems
        Crystal,                    ///< Crystals and minerals
        Structure,                  ///< Buildings and structures
        Creature,                   ///< Simple creatures
        Organic,                    ///< Organic shapes
        Geometric,                  ///< Geometric shapes
        Custom                      ///< Custom model types
    };

    /**
     * @enum GenerationAlgorithm
     * @brief Model generation algorithms
     */
    enum class GenerationAlgorithm {
        LSystem,                    ///< L-System for plants/trees
        Fractal,                    ///< Fractal subdivision
        CellularAutomata,           ///< Cellular automata
        MarchingCubes,              ///< Marching cubes for terrain
        NoiseDisplace,              ///< Noise displacement
        GrammarBased,               ///< Grammar-based generation
        TemplateBased,              ///< Template with variations
        Hybrid,                     ///< Hybrid algorithms
        Custom                      ///< Custom algorithm
    };

    /**
     * @enum ModelQuality
     * @brief Model generation quality levels
     */
    enum class ModelQuality {
        Low,                        ///< Low quality (fast generation)
        Medium,                     ///< Medium quality
        High,                       ///< High quality (detailed)
        Ultra,                      ///< Ultra quality (maximum detail)
        Custom                      ///< Custom quality settings
    };

    /**
     * @struct Vertex
     * @brief 3D vertex structure
     */
    struct Vertex {
        glm::vec3 position;                         ///< Vertex position
        glm::vec3 normal;                           ///< Vertex normal
        glm::vec2 texCoord;                         ///< Texture coordinates
        glm::vec3 tangent;                          ///< Tangent vector
        glm::vec4 color;                            ///< Vertex color

        Vertex()
            : position(0.0f, 0.0f, 0.0f)
            , normal(0.0f, 1.0f, 0.0f)
            , texCoord(0.0f, 0.0f)
            , tangent(1.0f, 0.0f, 0.0f)
            , color(1.0f, 1.0f, 1.0f, 1.0f)
        {}
    };

    /**
     * @struct Triangle
     * @brief Triangle structure
     */
    struct Triangle {
        uint32_t indices[3];                        ///< Vertex indices
        glm::vec3 normal;                           ///< Face normal
        uint32_t materialId;                        ///< Material ID

        Triangle()
            : indices{0, 0, 0}
            , normal(0.0f, 1.0f, 0.0f)
            , materialId(0)
        {}
    };

    /**
     * @struct MeshData
     * @brief Generated mesh data
     */
    struct MeshData {
        std::string meshId;                         ///< Unique mesh identifier
        std::vector<Vertex> vertices;               ///< Vertex data
        std::vector<Triangle> triangles;            ///< Triangle data
        std::vector<uint32_t> indices;              ///< Index buffer (optional)
        std::unordered_map<std::string, std::any> metadata; ///< Mesh metadata

        // Bounding volumes
        glm::vec3 boundingMin;                      ///< Bounding box minimum
        glm::vec3 boundingMax;                      ///< Bounding box maximum
        float boundingRadius;                       ///< Bounding sphere radius

        // Generation info
        ModelType modelType;                        ///< Type of generated model
        GenerationAlgorithm algorithm;              ///< Algorithm used
        double generationTime;                      ///< Time taken to generate
        size_t memoryUsage;                         ///< Memory usage

        MeshData()
            : boundingMin(0.0f, 0.0f, 0.0f)
            , boundingMax(0.0f, 0.0f, 0.0f)
            , boundingRadius(0.0f)
            , modelType(ModelType::Plant)
            , algorithm(GenerationAlgorithm::LSystem)
            , generationTime(0.0)
            , memoryUsage(0)
        {}
    };

    /**
     * @struct ModelGenerationParams
     * @brief Parameters for procedural model generation
     */
    struct ModelGenerationParams {
        // Basic parameters
        ModelType modelType;                        ///< Type of model to generate
        GenerationAlgorithm algorithm;              ///< Generation algorithm
        ModelQuality quality;                       ///< Generation quality

        // Size and scale
        glm::vec3 scale;                            ///< Model scale
        float size;                                 ///< Base size
        float detail;                               ///< Detail level (0.0 - 1.0)

        // Generation parameters
        uint32_t seed;                              ///< Random seed
        int iterations;                             ///< Number of iterations/growth cycles
        float complexity;                           ///< Model complexity
        float branching;                            ///< Branching factor (for plants/trees)

        // Algorithm-specific parameters
        std::unordered_map<std::string, float> algorithmParams; ///< Algorithm parameters

        // L-System specific
        std::string axiom;                          ///< L-System axiom
        std::vector<std::string> rules;             ///< L-System production rules
        float angle;                                ///< L-System rotation angle
        float segmentLength;                        ///< L-System segment length

        // Fractal specific
        int fractalDepth;                           ///< Fractal recursion depth
        float fractalScale;                         ///< Fractal scaling factor

        // Noise specific
        float noiseScale;                           ///< Noise scale
        int noiseOctaves;                           ///< Noise octaves
        float noiseStrength;                        ///< Noise strength

        // Post-processing
        bool enableSmoothing;                       ///< Enable mesh smoothing
        bool enableOptimization;                    ///< Enable mesh optimization
        bool calculateNormals;                      ///< Calculate vertex normals
        bool calculateTangents;                     ///< Calculate tangent vectors

        ModelGenerationParams()
            : modelType(ModelType::Plant)
            , algorithm(GenerationAlgorithm::LSystem)
            , quality(ModelQuality::High)
            , scale(1.0f, 1.0f, 1.0f)
            , size(1.0f)
            , detail(0.5f)
            , seed(0)
            , iterations(4)
            , complexity(0.5f)
            , branching(0.7f)
            , angle(25.0f)
            , segmentLength(1.0f)
            , fractalDepth(3)
            , fractalScale(0.5f)
            , noiseScale(1.0f)
            , noiseOctaves(4)
            , noiseStrength(1.0f)
            , enableSmoothing(true)
            , enableOptimization(true)
            , calculateNormals(true)
            , calculateTangents(false)
        {}
    };

    /**
     * @struct ModelGenerationStats
     * @brief Model generation performance statistics
     */
    struct ModelGenerationStats {
        // Generation statistics
        uint64_t modelsGenerated;                   ///< Total models generated
        uint64_t generationAttempts;                ///< Total generation attempts
        uint64_t successfulGenerations;             ///< Successful generations
        uint64_t failedGenerations;                 ///< Failed generations

        // Performance metrics
        double averageGenerationTime;               ///< Average generation time (ms)
        double minGenerationTime;                   ///< Minimum generation time (ms)
        double maxGenerationTime;                   ///< Maximum generation time (ms)

        // Quality metrics
        uint64_t totalVertices;                     ///< Total vertices generated
        uint64_t totalTriangles;                    ///< Total triangles generated
        float averageDetailLevel;                   ///< Average detail level
        float averageComplexity;                    ///< Average model complexity

        // Memory metrics
        size_t totalMemoryUsed;                     ///< Total memory used
        size_t peakMemoryUsed;                      ///< Peak memory usage
        uint32_t activeModels;                      ///< Number of active models

        // Algorithm usage
        std::unordered_map<GenerationAlgorithm, uint64_t> algorithmUsage; ///< Algorithm usage count
        std::unordered_map<ModelType, uint64_t> modelTypeUsage; ///< Model type usage
    };

    /**
     * @class ProceduralModelGenerator
     * @brief Advanced procedural 3D model generation system
     *
     * The ProceduralModelGenerator class provides comprehensive procedural 3D model
     * generation for the VoxelCraft game engine, featuring L-systems for realistic
     * vegetation, fractal algorithms for natural formations, cellular automata for
     * cave systems, and advanced mesh generation with real-time capabilities for
     * infinite content variety and dynamic world building.
     *
     * Key Features:
     * - L-System generation for realistic plants and trees
     * - Fractal algorithms for terrain and rock formations
     * - Cellular automata for cave and tunnel systems
     * - Marching cubes for smooth terrain surfaces
     * - Grammar-based generation for complex structures
     * - Real-time model generation with GPU acceleration
     * - Advanced mesh optimization and LOD generation
     * - Memory-efficient model caching and instancing
     * - Cross-platform model format compatibility
     * - Integration with physics and collision systems
     *
     * The model generator creates infinite variety of 3D models for vegetation,
     * terrain features, structures, and environmental objects while maintaining
     * high performance and visual quality.
     */
    class ProceduralModelGenerator {
    public:
        /**
         * @brief Constructor
         * @param assetGenerator Parent asset generator instance
         */
        explicit ProceduralModelGenerator(ProceduralAssetGenerator* assetGenerator);

        /**
         * @brief Destructor
         */
        ~ProceduralModelGenerator();

        /**
         * @brief Deleted copy constructor
         */
        ProceduralModelGenerator(const ProceduralModelGenerator&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        ProceduralModelGenerator& operator=(const ProceduralModelGenerator&) = delete;

        // Model generation lifecycle

        /**
         * @brief Initialize model generator
         * @return true if successful, false otherwise
         */
        bool Initialize();

        /**
         * @brief Shutdown model generator
         */
        void Shutdown();

        /**
         * @brief Update model generator
         * @param deltaTime Time elapsed since last update
         */
        void Update(double deltaTime);

        /**
         * @brief Get model generation statistics
         * @return Current statistics
         */
        const ModelGenerationStats& GetStats() const { return m_stats; }

        // Model generation methods

        /**
         * @brief Generate 3D model
         * @param params Generation parameters
         * @return Generated mesh data or nullopt if failed
         */
        std::optional<MeshData> GenerateModel(const ModelGenerationParams& params);

        /**
         * @brief Generate model asynchronously
         * @param params Generation parameters
         * @return Future to the generated mesh
         */
        std::future<std::optional<MeshData>> GenerateModelAsync(const ModelGenerationParams& params);

        /**
         * @brief Generate model with progress callback
         * @param params Generation parameters
         * @param progressCallback Progress callback function
         * @return Generated mesh data or nullopt if failed
         */
        std::optional<MeshData> GenerateModelWithProgress(
            const ModelGenerationParams& params,
            std::function<void(float, const std::string&)> progressCallback);

        // Specific model generation methods

        /**
         * @brief Generate tree using L-System
         * @param params Generation parameters
         * @return Generated tree mesh
         */
        MeshData GenerateTree(const ModelGenerationParams& params);

        /**
         * @brief Generate plant using L-System
         * @param params Generation parameters
         * @return Generated plant mesh
         */
        MeshData GeneratePlant(const ModelGenerationParams& params);

        /**
         * @brief Generate rock using fractal subdivision
         * @param params Generation parameters
         * @return Generated rock mesh
         */
        MeshData GenerateRock(const ModelGenerationParams& params);

        /**
         * @brief Generate crystal using geometric patterns
         * @param params Generation parameters
         * @return Generated crystal mesh
         */
        MeshData GenerateCrystal(const ModelGenerationParams& params);

        /**
         * @brief Generate cave using cellular automata
         * @param params Generation parameters
         * @return Generated cave mesh
         */
        MeshData GenerateCave(const ModelGenerationParams& params);

        /**
         * @brief Generate terrain using noise displacement
         * @param params Generation parameters
         * @return Generated terrain mesh
         */
        MeshData GenerateTerrain(const ModelGenerationParams& params);

        // Advanced generation algorithms

        /**
         * @brief Generate model using L-System
         * @param params L-System parameters
         * @return Generated mesh
         */
        MeshData GenerateLSystemModel(const ModelGenerationParams& params);

        /**
         * @brief Generate fractal model
         * @param params Fractal parameters
         * @return Generated mesh
         */
        MeshData GenerateFractalModel(const ModelGenerationParams& params);

        /**
         * @brief Generate cellular automata model
         * @param params Cellular automata parameters
         * @return Generated mesh
         */
        MeshData GenerateCellularModel(const ModelGenerationParams& params);

        /**
         * @brief Generate marching cubes model
         * @param params Marching cubes parameters
         * @return Generated mesh
         */
        MeshData GenerateMarchingCubesModel(const ModelGenerationParams& params);

        // Model processing and optimization

        /**
         * @brief Optimize mesh data
         * @param mesh Mesh to optimize
         * @return Optimized mesh
         */
        MeshData OptimizeMesh(MeshData mesh);

        /**
         * @brief Smooth mesh surfaces
         * @param mesh Mesh to smooth
         * @param iterations Number of smoothing iterations
         * @return Smoothed mesh
         */
        MeshData SmoothMesh(MeshData mesh, int iterations = 1);

        /**
         * @brief Calculate mesh normals
         * @param mesh Mesh to process
         * @return Mesh with calculated normals
         */
        MeshData CalculateNormals(MeshData mesh);

        /**
         * @brief Calculate mesh tangents
         * @param mesh Mesh to process
         * @return Mesh with calculated tangents
         */
        MeshData CalculateTangents(MeshData mesh);

        /**
         * @brief Generate LOD levels for mesh
         * @param mesh Base mesh
         * @param lodLevels Number of LOD levels to generate
         * @return Vector of LOD meshes
         */
        std::vector<MeshData> GenerateLODs(const MeshData& mesh, int lodLevels = 3);

        /**
         * @brief Merge multiple meshes
         * @param meshes Meshes to merge
         * @return Combined mesh
         */
        MeshData MergeMeshes(const std::vector<MeshData>& meshes);

        // Utility functions

        /**
         * @brief Get supported model types
         * @return Vector of supported model types
         */
        std::vector<ModelType> GetSupportedModelTypes() const;

        /**
         * @brief Get supported generation algorithms
         * @return Vector of supported algorithms
         */
        std::vector<GenerationAlgorithm> GetSupportedAlgorithms() const;

        /**
         * @brief Get algorithm description
         * @param algorithm Algorithm to describe
         * @return Algorithm description
         */
        std::string GetAlgorithmDescription(GenerationAlgorithm algorithm) const;

        /**
         * @brief Get model type description
         * @param modelType Model type to describe
         * @return Model type description
         */
        std::string GetModelTypeDescription(ModelType modelType) const;

        /**
         * @brief Calculate mesh bounding volumes
         * @param mesh Mesh to process
         * @return Mesh with calculated bounding volumes
         */
        MeshData CalculateBoundingVolumes(MeshData mesh);

        /**
         * @brief Validate mesh data
         * @param mesh Mesh to validate
         * @return true if valid, false otherwise
         */
        bool ValidateMesh(const MeshData& mesh) const;

        /**
         * @brief Get mesh memory usage
         * @param mesh Mesh to measure
         * @return Memory usage in bytes
         */
        size_t GetMeshMemoryUsage(const MeshData& mesh) const;

        /**
         * @brief Get mesh vertex count
         * @param mesh Mesh to count
         * @return Number of vertices
         */
        uint32_t GetMeshVertexCount(const MeshData& mesh) const;

        /**
         * @brief Get mesh triangle count
         * @param mesh Mesh to count
         * @return Number of triangles
         */
        uint32_t GetMeshTriangleCount(const MeshData& mesh) const;

        // Preset model generation

        /**
         * @brief Generate oak tree
         * @param seed Random seed
         * @param height Tree height
         * @return Generated oak tree mesh
         */
        MeshData GenerateOakTree(uint32_t seed = 0, float height = 8.0f);

        /**
         * @brief Generate pine tree
         * @param seed Random seed
         * @param height Tree height
         * @return Generated pine tree mesh
         */
        MeshData GeneratePineTree(uint32_t seed = 0, float height = 10.0f);

        /**
         * @brief Generate bush/plant
         * @param seed Random seed
         * @param size Plant size
         * @return Generated bush mesh
         */
        MeshData GenerateBush(uint32_t seed = 0, float size = 2.0f);

        /**
         * @brief Generate rock formation
         * @param seed Random seed
         * @param size Rock size
         * @return Generated rock mesh
         */
        MeshData GenerateRockFormation(uint32_t seed = 0, float size = 3.0f);

        /**
         * @brief Generate crystal cluster
         * @param seed Random seed
         * @param size Crystal size
         * @return Generated crystal mesh
         */
        MeshData GenerateCrystalCluster(uint32_t seed = 0, float size = 1.5f);

        /**
         * @brief Generate cave system
         * @param seed Random seed
         * @param size Cave size
         * @return Generated cave mesh
         */
        MeshData GenerateCaveSystem(uint32_t seed = 0, float size = 20.0f);

        /**
         * @brief Validate model generator state
         * @return true if valid, false otherwise
         */
        bool Validate() const;

        /**
         * @brief Get status report
         * @return Detailed status report
         */
        std::string GetStatusReport() const;

        /**
         * @brief Get performance report
         * @return Detailed performance report
         */
        std::string GetPerformanceReport() const;

        /**
         * @brief Optimize model generator
         * @return Optimization results
         */
        std::unordered_map<std::string, size_t> Optimize();

    private:
        /**
         * @brief Initialize L-System rules
         */
        void InitializeLSystemRules();

        /**
         * @brief Initialize fractal templates
         */
        void InitializeFractalTemplates();

        /**
         * @brief Generate L-System string
         * @param axiom Starting axiom
         * @param rules Production rules
         * @param iterations Number of iterations
         * @return Generated L-System string
         */
        std::string GenerateLSystemString(const std::string& axiom,
                                        const std::vector<std::string>& rules,
                                        int iterations);

        /**
         * @brief Interpret L-System string into 3D mesh
         * @param lsystemString L-System string
         * @param params Generation parameters
         * @return Generated mesh
         */
        MeshData InterpretLSystem(const std::string& lsystemString, const ModelGenerationParams& params);

        /**
         * @brief Generate fractal mesh using subdivision
         * @param params Generation parameters
         * @return Generated fractal mesh
         */
        MeshData GenerateFractalMesh(const ModelGenerationParams& params);

        /**
         * @brief Generate cellular automata mesh
         * @param params Generation parameters
         * @return Generated cellular mesh
         */
        MeshData GenerateCellularAutomataMesh(const ModelGenerationParams& params);

        /**
         * @brief Apply noise displacement to mesh
         * @param mesh Base mesh
         * @param params Noise parameters
         * @return Displaced mesh
         */
        MeshData ApplyNoiseDisplacement(MeshData mesh, const ModelGenerationParams& params);

        /**
         * @brief Optimize mesh topology
         * @param mesh Mesh to optimize
         * @return Optimized mesh
         */
        MeshData OptimizeMeshTopology(MeshData mesh);

        /**
         * @brief Update performance statistics
         * @param generationTime Generation time in milliseconds
         * @param vertexCount Number of vertices generated
         * @param triangleCount Number of triangles generated
         */
        void UpdateStats(double generationTime, uint32_t vertexCount, uint32_t triangleCount);

        /**
         * @brief Handle model generation errors
         * @param error Error message
         */
        void HandleError(const std::string& error);

        // Model generator data
        ProceduralAssetGenerator* m_assetGenerator;  ///< Parent asset generator
        ModelGenerationStats m_stats;                ///< Performance statistics

        // L-System rules database
        std::unordered_map<ModelType, std::unordered_map<std::string, std::any>> m_lSystemRules; ///< L-System rules
        std::unordered_map<ModelType, std::vector<std::string>> m_lSystemTemplates; ///< L-System templates

        // Fractal templates
        std::unordered_map<ModelType, std::vector<glm::vec3>> m_fractalTemplates; ///< Fractal vertex templates

        // Random number generation
        std::mt19937 m_randomEngine;                 ///< Random number generator
        mutable std::shared_mutex m_generatorMutex;  ///< Generator synchronization

        // State tracking
        bool m_isInitialized;                        ///< Generator is initialized
        double m_lastUpdateTime;                     ///< Last update time
        std::string m_lastError;                     ///< Last error message

        static std::atomic<uint32_t> s_nextModelId;  ///< Next model ID counter
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_PROCEDURAL_PROCEDURAL_MODEL_GENERATOR_HPP
