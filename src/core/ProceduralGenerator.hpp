/**
 * @file ProceduralGenerator.hpp
 * @brief VoxelCraft Procedural Generation System - Core Generator
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_CORE_PROCEDURAL_GENERATOR_HPP
#define VOXELCRAFT_CORE_PROCEDURAL_GENERATOR_HPP

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <random>
#include <functional>
#include <chrono>
#include "GameConstants.hpp"
#include "PhysicsUtils.hpp"

namespace VoxelCraft {

    // Forward declarations
    class World;
    class EntityManager;
    class Entity;

    /**
     * @enum GenerationType
     * @brief Types of procedural generation
     */
    enum class GenerationType {
        TERRAIN = 0,      ///< Terrain and landscapes
        BIOMES,           ///< Biome distribution
        STRUCTURES,       ///< Buildings, ruins, etc.
        CAVES,            ///< Cave systems
        ORES,             ///< Mineral deposits
        VEGETATION,       ///< Trees, plants, flowers
        MOBS,             ///< Creatures and enemies
        ITEMS,            ///< Items and tools
        SKINS,            ///< Character and object appearances
        SOUNDS            ///< Audio generation
    };

    /**
     * @struct GenerationParameters
     * @brief Parameters for procedural generation
     */
    struct GenerationParameters {
        uint64_t seed;                    ///< World seed
        int octaves;                      ///< Noise octaves for terrain
        float persistence;                ///< Noise persistence
        float scale;                      ///< Noise scale
        float amplitude;                  ///< Terrain amplitude
        int minHeight;                    ///< Minimum terrain height
        int maxHeight;                    ///< Maximum terrain height
        float caveDensity;                ///< Cave generation density
        float oreDensity;                 ///< Ore distribution density
        float vegetationDensity;          ///< Plant/tree density
        float mobDensity;                 ///< Mob spawn density
        bool generateCaves;               ///< Enable cave generation
        bool generateStructures;          ///< Enable structure generation
        bool generateOres;                ///< Enable ore generation
        bool generateVegetation;          ///< Enable vegetation
        bool generateMobs;                ///< Enable mob generation
        std::string worldName;            ///< World name for generation

        GenerationParameters()
            : seed(12345)
            , octaves(4)
            , persistence(0.5f)
            , scale(0.01f)
            , amplitude(64.0f)
            , minHeight(-64)
            , maxHeight(320)
            , caveDensity(0.3f)
            , oreDensity(0.1f)
            , vegetationDensity(0.7f)
            , mobDensity(0.05f)
            , generateCaves(true)
            , generateStructures(true)
            , generateOres(true)
            , generateVegetation(true)
            , generateMobs(true)
            , worldName("VoxelCraft World")
        {}
    };

    /**
     * @struct ProceduralObject
     * @brief Base structure for procedurally generated objects
     */
    struct ProceduralObject {
        std::string id;                   ///< Unique object identifier
        Vec3 position;                    ///< World position
        Vec3 rotation;                    ///< Object rotation
        Vec3 scale;                       ///< Object scale
        std::string type;                 ///< Object type (block, mob, item, etc.)
        std::unordered_map<std::string, std::string> properties; ///< Object properties
        void* generatedData;              ///< Pointer to generated content (texture, sound, etc.)

        ProceduralObject()
            : id(""), type(""), generatedData(nullptr) {}
    };

    /**
     * @class ProceduralGenerator
     * @brief Core procedural generation system
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
         * @brief Initialize the generator with parameters
         * @param params Generation parameters
         */
        void Initialize(const GenerationParameters& params);

        /**
         * @brief Set world seed
         * @param seed New seed
         */
        void SetSeed(uint64_t seed);

        /**
         * @brief Generate all content for a world position
         * @param position Center position for generation
         * @param radius Generation radius
         * @return Vector of generated objects
         */
        std::vector<ProceduralObject> GenerateAll(const Vec3& position, float radius);

        /**
         * @brief Generate terrain at position
         * @param position Position to generate terrain
         * @return Generated terrain objects
         */
        std::vector<ProceduralObject> GenerateTerrain(const Vec3& position);

        /**
         * @brief Generate biomes at position
         * @param position Position to generate biomes
         * @return Generated biome objects
         */
        std::vector<ProceduralObject> GenerateBiomes(const Vec3& position);

        /**
         * @brief Generate structures at position
         * @param position Position to generate structures
         * @return Generated structure objects
         */
        std::vector<ProceduralObject> GenerateStructures(const Vec3& position);

        /**
         * @brief Generate caves at position
         * @param position Position to generate caves
         * @return Generated cave objects
         */
        std::vector<ProceduralObject> GenerateCaves(const Vec3& position);

        /**
         * @brief Generate ores at position
         * @param position Position to generate ores
         * @return Generated ore objects
         */
        std::vector<ProceduralObject> GenerateOres(const Vec3& position);

        /**
         * @brief Generate vegetation at position
         * @param position Position to generate vegetation
         * @return Generated vegetation objects
         */
        std::vector<ProceduralObject> GenerateVegetation(const Vec3& position);

        /**
         * @brief Generate mobs at position
         * @param position Position to generate mobs
         * @return Generated mob objects
         */
        std::vector<ProceduralObject> GenerateMobs(const Vec3& position);

        /**
         * @brief Generate items at position
         * @param position Position to generate items
         * @return Generated item objects
         */
        std::vector<ProceduralObject> GenerateItems(const Vec3& position);

        /**
         * @brief Generate skin/appearance for object
         * @param objectType Type of object to generate skin for
         * @param seed Seed for generation
         * @return Generated skin data
         */
        void* GenerateSkin(const std::string& objectType, uint64_t seed);

        /**
         * @brief Generate sound for object
         * @param objectType Type of object to generate sound for
         * @param seed Seed for generation
         * @return Generated sound data
         */
        void* GenerateSound(const std::string& objectType, uint64_t seed);

        /**
         * @brief Get generation parameters
         * @return Current parameters
         */
        const GenerationParameters& GetParameters() const { return m_params; }

        /**
         * @brief Set generation parameters
         * @param params New parameters
         */
        void SetParameters(const GenerationParameters& params) { m_params = params; }

        /**
         * @brief Create entities from procedural objects
         * @param objects Vector of procedural objects
         * @param entityManager Entity manager to create entities in
         * @return Vector of created entities
         */
        std::vector<std::shared_ptr<Entity>> CreateEntitiesFromObjects(
            const std::vector<ProceduralObject>& objects,
            EntityManager* entityManager
        );

    private:
        GenerationParameters m_params;
        std::mt19937_64 m_randomEngine;
        std::unordered_map<std::string, std::function<void*(uint64_t)>> m_skinGenerators;
        std::unordered_map<std::string, std::function<void*(uint64_t)>> m_soundGenerators;

        // Noise generators
        std::unique_ptr<PerlinNoise> m_terrainNoise;
        std::unique_ptr<PerlinNoise> m_caveNoise;
        std::unique_ptr<PerlinNoise> m_oreNoise;
        std::unique_ptr<PerlinNoise> m_vegetationNoise;

        /**
         * @brief Initialize noise generators
         */
        void InitializeNoiseGenerators();

        /**
         * @brief Initialize skin generators
         */
        void InitializeSkinGenerators();

        /**
         * @brief Initialize sound generators
         */
        void InitializeSoundGenerators();

        /**
         * @brief Generate terrain height at position
         * @param x X coordinate
         * @param z Z coordinate
         * @return Height value
         */
        float GenerateTerrainHeight(float x, float z);

        /**
         * @brief Generate biome at position
         * @param x X coordinate
         * @param z Z coordinate
         * @return Biome type
         */
        std::string GenerateBiome(float x, float z);

        /**
         * @brief Check if position is inside cave
         * @param x X coordinate
         * @param y Y coordinate
         * @param z Z coordinate
         * @return true if inside cave
         */
        bool IsInCave(float x, float y, float z);

        /**
         * @brief Generate cave at position
         * @param position Starting position
         * @return Cave object
         */
        ProceduralObject GenerateCave(const Vec3& position);

        /**
         * @brief Generate tree at position
         * @param position Tree position
         * @param biome Biome type
         * @return Tree objects
         */
        std::vector<ProceduralObject> GenerateTree(const Vec3& position, const std::string& biome);

        /**
         * @brief Generate ore deposit at position
         * @param position Ore position
         * @return Ore objects
         */
        std::vector<ProceduralObject> GenerateOreDeposit(const Vec3& position);

        /**
         * @brief Generate mob at position
         * @param position Mob position
         * @param biome Biome type
         * @return Mob object
         */
        ProceduralObject GenerateMob(const Vec3& position, const std::string& biome);

        /**
         * @brief Generate item at position
         * @param position Item position
         * @return Item object
         */
        ProceduralObject GenerateItem(const Vec3& position);

        /**
         * @brief Generate structure at position
         * @param position Structure position
         * @param structureType Type of structure
         * @return Structure objects
         */
        std::vector<ProceduralObject> GenerateStructure(const Vec3& position, const std::string& structureType);
    };

    /**
     * @class PerlinNoise
     * @brief Perlin noise generator for procedural content
     */
    class PerlinNoise {
    public:
        /**
         * @brief Constructor
         * @param seed Random seed
         */
        PerlinNoise(uint64_t seed);

        /**
         * @brief Generate 2D noise value
         * @param x X coordinate
         * @param z Z coordinate
         * @return Noise value (-1 to 1)
         */
        float Noise2D(float x, float z);

        /**
         * @brief Generate 3D noise value
         * @param x X coordinate
         * @param y Y coordinate
         * @param z Z coordinate
         * @return Noise value (-1 to 1)
         */
        float Noise3D(float x, float y, float z);

        /**
         * @brief Generate octave noise
         * @param x X coordinate
         * @param z Z coordinate
         * @param octaves Number of octaves
         * @param persistence Persistence value
         * @return Noise value (-1 to 1)
         */
        float OctaveNoise2D(float x, float z, int octaves, float persistence);

    private:
        uint64_t m_seed;
        std::array<int, 256> m_permutation;

        /**
         * @brief Initialize permutation table
         */
        void InitializePermutation();

        /**
         * @brief Fade function for interpolation
         * @param t Input value
         * @return Faded value
         */
        float Fade(float t);

        /**
         * @brief Gradient function
         * @param hash Hash value
         * @param x X coordinate
         * @param z Z coordinate
         * @return Gradient value
         */
        float Gradient(int hash, float x, float z);
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_CORE_PROCEDURAL_GENERATOR_HPP
