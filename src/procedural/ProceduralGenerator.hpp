/**
 * @file ProceduralGenerator.hpp
 * @brief VoxelCraft Procedural Generation System
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * Procedural generation system for creating worlds, terrain, objects, and content.
 */

#ifndef VOXELCRAFT_PROCEDURAL_PROCEDURAL_GENERATOR_HPP
#define VOXELCRAFT_PROCEDURAL_PROCEDURAL_GENERATOR_HPP

#include <memory>
#include <vector>
#include <unordered_map>
#include <functional>
#include <string>
#include <random>
#include <chrono>

namespace VoxelCraft {

    // Forward declarations
    class Entity;
    class EntityManager;
    struct Vec3;

    /**
     * @enum GenerationType
     * @brief Types of procedural generation
     */
    enum class GenerationType {
        Terrain,        ///< Terrain and landscape generation
        Vegetation,     ///< Trees, plants, and vegetation
        Structures,     ///< Buildings, caves, and man-made structures
        Objects,        ///< Props, items, and environmental objects
        Enemies,        ///< Enemy placement and spawning
        Resources,      ///< Resource distribution and spawning
        Weather,        ///< Weather system generation
        Biome          ///< Biome and ecosystem generation
    };

    /**
     * @struct GenerationParameters
     * @brief Parameters for procedural generation
     */
    struct GenerationParameters {
        int seed;                           ///< Random seed for generation
        Vec3 position;                      ///< Center position for generation
        float radius;                       ///< Generation radius
        int detailLevel;                    ///< Level of detail (0-10)
        bool useBiomes;                     ///< Enable biome-based generation
        bool useHeightmaps;                 ///< Use heightmap-based terrain
        float noiseScale;                   ///< Scale for noise functions
        int octaves;                        ///< Number of noise octaves
        float persistence;                  ///< Noise persistence
        float lacunarity;                   ///< Noise lacunarity

        GenerationParameters()
            : seed(static_cast<int>(std::chrono::system_clock::now().time_since_epoch().count()))
            , position(0.0f, 0.0f, 0.0f)
            , radius(100.0f)
            , detailLevel(5)
            , useBiomes(true)
            , useHeightmaps(true)
            , noiseScale(0.1f)
            , octaves(4)
            , persistence(0.5f)
            , lacunarity(2.0f)
        {}
    };

    /**
     * @struct ProceduralObject
     * @brief Represents a procedurally generated object
     */
    struct ProceduralObject {
        std::string type;                   ///< Object type identifier
        std::string name;                   ///< Object name
        Vec3 position;                      ///< Object position
        Vec3 rotation;                      ///< Object rotation (Euler angles)
        Vec3 scale;                         ///< Object scale
        std::unordered_map<std::string, float> properties; ///< Custom properties
        bool isActive;                      ///< Whether object is active
        float spawnProbability;             ///< Probability of spawning

        ProceduralObject()
            : type("unknown")
            , name("ProceduralObject")
            , position(0.0f, 0.0f, 0.0f)
            , rotation(0.0f, 0.0f, 0.0f)
            , scale(1.0f, 1.0f, 1.0f)
            , isActive(true)
            , spawnProbability(1.0f)
        {}
    };

    /**
     * @class NoiseGenerator
     * @brief Procedural noise generator for terrain and features
     */
    class NoiseGenerator {
    public:
        /**
         * @brief Constructor
         * @param seed Random seed
         */
        explicit NoiseGenerator(int seed = 0);

        /**
         * @brief Generate 2D Perlin noise
         * @param x X coordinate
         * @param y Y coordinate
         * @param scale Noise scale
         * @param octaves Number of octaves
         * @param persistence Persistence factor
         * @param lacunarity Lacunarity factor
         * @return Noise value (-1 to 1)
         */
        float PerlinNoise2D(float x, float y, float scale = 1.0f,
                           int octaves = 4, float persistence = 0.5f, float lacunarity = 2.0f);

        /**
         * @brief Generate 3D Perlin noise
         * @param x X coordinate
         * @param y Y coordinate
         * @param z Z coordinate
         * @param scale Noise scale
         * @param octaves Number of octaves
         * @param persistence Persistence factor
         * @param lacunarity Lacunarity factor
         * @return Noise value (-1 to 1)
         */
        float PerlinNoise3D(float x, float y, float z, float scale = 1.0f,
                           int octaves = 4, float persistence = 0.5f, float lacunarity = 2.0f);

        /**
         * @brief Generate ridged noise (for mountains)
         * @param x X coordinate
         * @param y Y coordinate
         * @param scale Noise scale
         * @param octaves Number of octaves
         * @return Noise value (0 to 1)
         */
        float RidgedNoise(float x, float y, float scale = 1.0f, int octaves = 4);

        /**
         * @brief Generate cellular noise (Worley noise)
         * @param x X coordinate
         * @param y Y coordinate
         * @param scale Noise scale
         * @return Noise value (0 to 1)
         */
        float CellularNoise(float x, float y, float scale = 1.0f);

        /**
         * @brief Set random seed
         * @param seed New seed value
         */
        void SetSeed(int seed);

        /**
         * @brief Get current seed
         * @return Current seed
         */
        int GetSeed() const { return m_seed; }

    private:
        /**
         * @brief Fade function for Perlin noise
         * @param t Input value
         * @return Faded value
         */
        float Fade(float t) const;

        /**
         * @brief Linear interpolation
         * @param a First value
         * @param b Second value
         * @param t Interpolation factor
         * @return Interpolated value
         */
        float Lerp(float a, float b, float t) const;

        /**
         * @brief Gradient function
         * @param hash Hash value
         * @param x X coordinate
         * @param y Y coordinate
         * @return Gradient value
         */
        float Grad(int hash, float x, float y) const;

        /**
         * @brief Gradient function for 3D
         * @param hash Hash value
         * @param x X coordinate
         * @param y Y coordinate
         * @param z Z coordinate
         * @return Gradient value
         */
        float Grad(int hash, float x, float y, float z) const;

        int m_seed;                         ///< Random seed
        std::mt19937 m_rng;                ///< Random number generator
        std::vector<int> m_permutation;    ///< Permutation table for noise
    };

    /**
     * @class ProceduralGenerator
     * @brief Main procedural generation system
     */
    class ProceduralGenerator {
    public:
        /**
         * @brief Constructor
         */
        ProceduralGenerator();

        /**
         * @brief Destructor
         */
        ~ProceduralGenerator();

        /**
         * @brief Initialize the generator
         * @param entityManager Entity manager to use
         * @return true if successful, false otherwise
         */
        bool Initialize(EntityManager* entityManager);

        /**
         * @brief Set generation parameters
         * @param params New generation parameters
         */
        void SetParameters(const GenerationParameters& params);

        /**
         * @brief Get current parameters
         * @return Current generation parameters
         */
        const GenerationParameters& GetParameters() const { return m_parameters; }

        /**
         * @brief Generate terrain at position
         * @param center Center position for generation
         * @param radius Generation radius
         * @return List of generated objects
         */
        std::vector<ProceduralObject> GenerateTerrain(const Vec3& center, float radius);

        /**
         * @brief Generate vegetation in area
         * @param center Center position
         * @param radius Generation radius
         * @return List of generated objects
         */
        std::vector<ProceduralObject> GenerateVegetation(const Vec3& center, float radius);

        /**
         * @brief Generate structures in area
         * @param center Center position
         * @param radius Generation radius
         * @return List of generated objects
         */
        std::vector<ProceduralObject> GenerateStructures(const Vec3& center, float radius);

        /**
         * @brief Generate objects in area
         * @param center Center position
         * @param radius Generation radius
         * @return List of generated objects
         */
        std::vector<ProceduralObject> GenerateObjects(const Vec3& center, float radius);

        /**
         * @brief Generate enemies in area
         * @param center Center position
         * @param radius Generation radius
         * @return List of generated objects
         */
        std::vector<ProceduralObject> GenerateEnemies(const Vec3& center, float radius);

        /**
         * @brief Generate resources in area
         * @param center Center position
         * @param radius Generation radius
         * @return List of generated objects
         */
        std::vector<ProceduralObject> GenerateResources(const Vec3& center, float radius);

        /**
         * @brief Generate everything at once
         * @param center Center position
         * @param radius Generation radius
         * @return List of all generated objects
         */
        std::vector<ProceduralObject> GenerateAll(const Vec3& center, float radius);

        /**
         * @brief Create entities from procedural objects
         * @param objects List of procedural objects
         * @return Number of entities created
         */
        size_t CreateEntitiesFromObjects(const std::vector<ProceduralObject>& objects);

        /**
         * @brief Clear all generated content in area
         * @param center Center position
         * @param radius Clear radius
         * @return Number of objects cleared
         */
        size_t ClearArea(const Vec3& center, float radius);

        /**
         * @brief Get noise generator
         * @return Noise generator instance
         */
        NoiseGenerator& GetNoiseGenerator() { return m_noiseGenerator; }

        /**
         * @brief Register custom generation function
         * @param type Generation type
         * @param function Generation function
         */
        void RegisterGenerator(GenerationType type,
                             std::function<std::vector<ProceduralObject>(const Vec3&, float)> function);

    private:
        /**
         * @brief Generate terrain height at position
         * @param x X coordinate
         * @param z Z coordinate
         * @return Height value
         */
        float GenerateTerrainHeight(float x, float z);

        /**
         * @brief Determine biome at position
         * @param x X coordinate
         * @param z Z coordinate
         * @return Biome type string
         */
        std::string DetermineBiome(float x, float z);

        /**
         * @brief Generate random position in circle
         * @param center Center position
         * @param radius Circle radius
         * @return Random position
         */
        Vec3 GetRandomPositionInCircle(const Vec3& center, float radius);

        EntityManager* m_entityManager;                             ///< Entity manager
        GenerationParameters m_parameters;                         ///< Generation parameters
        NoiseGenerator m_noiseGenerator;                           ///< Noise generator

        std::unordered_map<GenerationType,
            std::function<std::vector<ProceduralObject>(const Vec3&, float)>> m_generators;

        // Generated objects cache
        std::vector<ProceduralObject> m_generatedObjects;

        bool m_initialized;                                        ///< Initialization flag
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_PROCEDURAL_PROCEDURAL_GENERATOR_HPP
