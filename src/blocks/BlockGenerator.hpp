/**
 * @file BlockGenerator.hpp
 * @brief VoxelCraft Procedural Block Generator - Dynamic Textures and Properties
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_BLOCKS_BLOCK_GENERATOR_HPP
#define VOXELCRAFT_BLOCKS_BLOCK_GENERATOR_HPP

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <random>
#include <functional>
#include "ProceduralGenerator.hpp"

namespace VoxelCraft {

    // Forward declarations
    class EntityManager;
    class Entity;
    struct Vec3;

    /**
     * @enum BlockGenerationType
     * @brief Types of block generation
     */
    enum class BlockGenerationType {
        TERRAIN = 0,        ///< Natural terrain blocks
        ORE,                ///< Mineral deposits
        VEGETATION,         ///< Plant-based blocks
        STRUCTURE,          ///< Building blocks
        MAGICAL,            ///< Magic-related blocks
        CUSTOM              ///< Custom generation
    };

    /**
     * @enum BlockTextureType
     * @brief Types of procedural textures
     */
    enum class BlockTextureType {
        SOLID = 0,          ///< Solid color texture
        NOISE,              ///< Noise-based texture
        GRADIENT,           ///< Gradient texture
        PATTERN,            ///< Pattern-based texture
        CRYSTAL,            ///< Crystal-like texture
        ORGANIC,            ///< Organic/natural texture
        METAL,              ///< Metal texture
        STONE,              ///< Stone/rock texture
        WOOD,               ///< Wood texture
        LEAF,               ///< Leaf texture
        FLUID,              ///< Fluid texture
        CUSTOM              ///< Custom texture type
    };

    /**
     * @struct BlockProperties
     * @brief Dynamic properties of generated blocks
     */
    struct BlockProperties {
        float hardness;                 ///< How hard the block is to break
        float resistance;               ///< Blast resistance
        float lightLevel;               ///< Light emission (0-15)
        float friction;                 ///< Movement friction
        float slipperiness;             ///< How slippery the block is
        bool isSolid;                   ///< Can entities collide with it
        bool isOpaque;                  ///< Blocks light completely
        bool isTransparent;             ///< Allows light through
        bool isFlammable;               ///< Can catch fire
        bool isReplaceable;             ///< Can be replaced by other blocks
        bool requiresTool;              ///< Requires tool to break
        bool hasGravity;                ///< Falls when unsupported
        bool emitsParticles;            ///< Emits particles
        bool hasAnimation;              ///< Has animated texture
        int miningLevel;                ///< Required mining level
        std::string requiredTool;       ///< Required tool type
        std::vector<std::string> drops; ///< Items dropped when broken
        std::vector<std::string> specialEffects; ///< Special block effects
    };

    /**
     * @struct BlockAppearance
     * @brief Visual appearance data for blocks
     */
    struct BlockAppearance {
        BlockTextureType textureType;    ///< Type of procedural texture
        Vec3 primaryColor;               ///< Primary color
        Vec3 secondaryColor;             ///< Secondary color
        Vec3 accentColor;                ///< Accent/detail color
        Vec3 emissiveColor;              ///< Glow color
        float textureScale;              ///< Texture scale factor
        int patternComplexity;           ///< Pattern complexity (0-10)
        int variant;                     ///< Visual variant
        bool hasGlow;                    ///< Has glow effect
        bool hasParticles;               ///< Emits particles
        bool isAnimated;                 ///< Has animation
        std::vector<std::string> visualEffects; ///< Visual effects
        std::string customTextureData;   ///< Custom texture generation data
    };

    /**
     * @struct GeneratedBlock
     * @brief Complete block data generated procedurally
     */
    struct GeneratedBlock {
        std::string id;                  ///< Unique block identifier
        std::string name;                ///< Block name
        std::string displayName;         ///< Display name with formatting
        BlockGenerationType type;        ///< Generation type
        std::string materialType;        ///< Material category
        std::string biomeRestriction;    ///< Biome where block can spawn
        BlockProperties properties;      ///< Block properties
        BlockAppearance appearance;      ///< Visual appearance
        std::vector<std::string> tags;   ///< Block tags
        bool isNatural;                  ///< Naturally occurring
        bool isRare;                     ///< Rare block
        bool isMagical;                  ///< Has magical properties
        void* textureData;               ///< Generated texture data
        void* modelData;                 ///< Generated 3D model data
        void* soundData;                 ///< Generated sound data
        void* particleData;              ///< Generated particle data
    };

    /**
     * @class BlockGenerator
     * @brief Procedural block generator with dynamic textures and properties
     */
    class BlockGenerator {
    public:
        /**
         * @brief Constructor
         */
        BlockGenerator();

        /**
         * @brief Destructor
         */
        ~BlockGenerator();

        /**
         * @brief Initialize the block generator
         * @param seed Random seed for generation
         */
        void Initialize(uint64_t seed);

        /**
         * @brief Generate a terrain block
         * @param biome Biome where block will spawn
         * @param depth Depth from surface (0 = surface)
         * @param temperature Biome temperature
         * @param humidity Biome humidity
         * @return Generated terrain block
         */
        GeneratedBlock GenerateTerrainBlock(const std::string& biome, int depth, float temperature, float humidity);

        /**
         * @brief Generate an ore block
         * @param oreType Type of ore to generate
         * @param biome Biome where ore spawns
         * @param depth Depth from surface
         * @return Generated ore block
         */
        GeneratedBlock GenerateOreBlock(const std::string& oreType, const std::string& biome, int depth);

        /**
         * @brief Generate a vegetation block
         * @param plantType Type of plant/vegetation
         * @param biome Biome where plant grows
         * @param season Current season
         * @return Generated vegetation block
         */
        GeneratedBlock GenerateVegetationBlock(const std::string& plantType, const std::string& biome, const std::string& season);

        /**
         * @brief Generate a structure block
         * @param structureType Type of structure block
         * @param material Material to use
         * @return Generated structure block
         */
        GeneratedBlock GenerateStructureBlock(const std::string& structureType, const std::string& material);

        /**
         * @brief Generate a magical block
         * @param magicType Type of magical block
         * @param powerLevel Magical power level
         * @return Generated magical block
         */
        GeneratedBlock GenerateMagicalBlock(const std::string& magicType, int powerLevel);

        /**
         * @brief Generate a custom block
         * @param customType Custom block type
         * @param parameters Generation parameters
         * @return Generated custom block
         */
        GeneratedBlock GenerateCustomBlock(const std::string& customType, const std::unordered_map<std::string, std::string>& parameters);

        /**
         * @brief Generate random block
         * @param generationType Type of generation
         * @param biome Biome context
         * @return Generated random block
         */
        GeneratedBlock GenerateRandomBlock(BlockGenerationType generationType = BlockGenerationType::TERRAIN, const std::string& biome = "plains");

        /**
         * @brief Generate block texture procedurally
         * @param block Generated block data
         * @return Generated texture data
         */
        void* GenerateBlockTexture(const GeneratedBlock& block);

        /**
         * @brief Generate block model procedurally
         * @param block Generated block data
         * @return Generated 3D model data
         */
        void* GenerateBlockModel(const GeneratedBlock& block);

        /**
         * @brief Generate block sounds procedurally
         * @param block Generated block data
         * @return Generated sound data
         */
        void* GenerateBlockSounds(const GeneratedBlock& block);

        /**
         * @brief Generate block particles procedurally
         * @param block Generated block data
         * @return Generated particle data
         */
        void* GenerateBlockParticles(const GeneratedBlock& block);

        /**
         * @brief Create entity from generated block
         * @param block Generated block data
         * @param position Block position
         * @param entityManager Entity manager to create in
         * @return Created entity
         */
        std::shared_ptr<Entity> CreateBlockEntity(const GeneratedBlock& block, const Vec3& position, EntityManager* entityManager);

        /**
         * @brief Get block generation statistics
         * @return Statistics map
         */
        std::unordered_map<std::string, int> GetGenerationStats() const;

        /**
         * @brief Reset generation statistics
         */
        void ResetGenerationStats();

    private:
        uint64_t m_seed;
        std::mt19937_64 m_randomEngine;

        // Generation statistics
        std::unordered_map<std::string, int> m_generationStats;

        // Block templates for different types
        std::unordered_map<std::string, BlockProperties> m_blockPropertiesTemplates;
        std::unordered_map<std::string, std::function<GeneratedBlock(const std::string&, int, float, float)>> m_terrainGenerators;
        std::unordered_map<std::string, std::function<GeneratedBlock(const std::string&, const std::string&, int)>> m_oreGenerators;
        std::unordered_map<std::string, std::function<GeneratedBlock(const std::string&, const std::string&, const std::string&)>> m_vegetationGenerators;
        std::unordered_map<std::string, std::function<GeneratedBlock(const std::string&, const std::string&)>> m_structureGenerators;
        std::unordered_map<std::string, std::function<GeneratedBlock(const std::string&, int)>> m_magicalGenerators;

        // Texture and model generators
        std::unordered_map<BlockTextureType, std::function<void*(const GeneratedBlock&)>> m_textureGenerators;
        std::unordered_map<std::string, std::function<void*(const GeneratedBlock&)>> m_modelGenerators;
        std::unordered_map<std::string, std::function<void*(const GeneratedBlock&)>> m_soundGenerators;
        std::unordered_map<std::string, std::function<void*(const GeneratedBlock&)>> m_particleGenerators;

        /**
         * @brief Initialize block property templates
         */
        void InitializeBlockTemplates();

        /**
         * @brief Initialize generation functions
         */
        void InitializeGenerators();

        /**
         * @brief Initialize texture generators
         */
        void InitializeTextureGenerators();

        /**
         * @brief Initialize model generators
         */
        void InitializeModelGenerators();

        /**
         * @brief Initialize sound generators
         */
        void InitializeSoundGenerators();

        /**
         * @brief Initialize particle generators
         */
        void InitializeParticleGenerators();

        /**
         * @brief Generate unique block name
         * @param baseName Base block name
         * @param seed Generation seed
         * @return Unique name
         */
        std::string GenerateUniqueBlockName(const std::string& baseName, uint64_t seed);

        /**
         * @brief Apply biome modifications to block
         * @param block Block to modify
         * @param biome Biome type
         */
        void ApplyBiomeModifications(GeneratedBlock& block, const std::string& biome);

        /**
         * @brief Apply depth modifications to block
         * @param block Block to modify
         * @param depth Depth from surface
         */
        void ApplyDepthModifications(GeneratedBlock& block, int depth);

        /**
         * @brief Apply temperature effects to block
         * @param block Block to modify
         * @param temperature Temperature value
         */
        void ApplyTemperatureEffects(GeneratedBlock& block, float temperature);

        /**
         * @brief Generate block tags based on properties
         * @param block Generated block
         * @return Vector of tags
         */
        std::vector<std::string> GenerateBlockTags(const GeneratedBlock& block);

        // Terrain block generation methods
        GeneratedBlock GenerateStoneBlock(const std::string& biome, int depth, float temperature, float humidity);
        GeneratedBlock GenerateDirtBlock(const std::string& biome, int depth, float temperature, float humidity);
        GeneratedBlock GenerateGrassBlock(const std::string& biome, int depth, float temperature, float humidity);
        GeneratedBlock GenerateSandBlock(const std::string& biome, int depth, float temperature, float humidity);
        GeneratedBlock GenerateGravelBlock(const std::string& biome, int depth, float temperature, float humidity);
        GeneratedBlock GenerateClayBlock(const std::string& biome, int depth, float temperature, float humidity);
        GeneratedBlock GenerateSnowBlock(const std::string& biome, int depth, float temperature, float humidity);

        // Ore generation methods
        GeneratedBlock GenerateCoalOreBlock(const std::string& biome, int depth);
        GeneratedBlock GenerateIronOreBlock(const std::string& biome, int depth);
        GeneratedBlock GenerateGoldOreBlock(const std::string& biome, int depth);
        GeneratedBlock GenerateDiamondOreBlock(const std::string& biome, int depth);
        GeneratedBlock GenerateRedstoneOreBlock(const std::string& biome, int depth);
        GeneratedBlock GenerateLapisOreBlock(const std::string& biome, int depth);
        GeneratedBlock GenerateEmeraldOreBlock(const std::string& biome, int depth);
        GeneratedBlock GenerateCustomOreBlock(const std::string& oreType, const std::string& biome, int depth);

        // Vegetation generation methods
        GeneratedBlock GenerateTreeBlock(const std::string& treeType, const std::string& biome, const std::string& season);
        GeneratedBlock GenerateFlowerBlock(const std::string& flowerType, const std::string& biome, const std::string& season);
        GeneratedBlock GenerateGrassPlantBlock(const std::string& biome, const std::string& season);
        GeneratedBlock GenerateCropBlock(const std::string& cropType, const std::string& biome, const std::string& season);
        GeneratedBlock GenerateMushroomBlock(const std::string& mushroomType, const std::string& biome, const std::string& season);

        // Structure generation methods
        GeneratedBlock GenerateWoodBlock(const std::string& woodType, const std::string& material);
        GeneratedBlock GenerateBrickBlock(const std::string& brickType, const std::string& material);
        GeneratedBlock GenerateStoneBrickBlock(const std::string& stoneType, const std::string& material);
        GeneratedBlock GenerateGlassBlock(const std::string& glassType, const std::string& material);
        GeneratedBlock GenerateConcreteBlock(const std::string& concreteType, const std::string& material);

        // Magical generation methods
        GeneratedBlock GenerateCrystalBlock(const std::string& crystalType, int powerLevel);
        GeneratedBlock GenerateMagicalStoneBlock(const std::string& magicType, int powerLevel);
        GeneratedBlock GenerateRunestoneBlock(const std::string& runeType, int powerLevel);
        GeneratedBlock GenerateObsidianBlock(const std::string& obsidianType, int powerLevel);

        // Custom generation methods
        GeneratedBlock GenerateAlienBlock(const std::string& alienType, const std::unordered_map<std::string, std::string>& parameters);
        GeneratedBlock GenerateTechBlock(const std::string& techType, const std::unordered_map<std::string, std::string>& parameters);
        GeneratedBlock GenerateOrganicBlock(const std::string& organicType, const std::unordered_map<std::string, std::string>& parameters);

        // Texture generation methods
        void* GenerateSolidTexture(const GeneratedBlock& block);
        void* GenerateNoiseTexture(const GeneratedBlock& block);
        void* GenerateGradientTexture(const GeneratedBlock& block);
        void* GeneratePatternTexture(const GeneratedBlock& block);
        void* GenerateCrystalTexture(const GeneratedBlock& block);
        void* GenerateOrganicTexture(const GeneratedBlock& block);
        void* GenerateMetalTexture(const GeneratedBlock& block);
        void* GenerateStoneTexture(const GeneratedBlock& block);
        void* GenerateWoodTexture(const GeneratedBlock& block);
        void* GenerateLeafTexture(const GeneratedBlock& block);
        void* GenerateFluidTexture(const GeneratedBlock& block);

        // Model generation methods
        void* GenerateStandardBlockModel(const GeneratedBlock& block);
        void* GenerateComplexBlockModel(const GeneratedBlock& block);
        void* GenerateVegetationModel(const GeneratedBlock& block);
        void* GenerateCrystalModel(const GeneratedBlock& block);
        void* GenerateMagicalModel(const GeneratedBlock& block);

        // Sound generation methods
        void* GenerateStoneBlockSounds(const GeneratedBlock& block);
        void* GenerateWoodBlockSounds(const GeneratedBlock& block);
        void* GenerateMetalBlockSounds(const GeneratedBlock& block);
        void* GenerateOrganicBlockSounds(const GeneratedBlock& block);
        void* GenerateMagicalBlockSounds(const GeneratedBlock& block);

        // Particle generation methods
        void* GenerateStoneParticles(const GeneratedBlock& block);
        void* GenerateWoodParticles(const GeneratedBlock& block);
        void* GenerateLeafParticles(const GeneratedBlock& block);
        void* GenerateCrystalParticles(const GeneratedBlock& block);
        void* GenerateMagicalParticles(const GeneratedBlock& block);
    };

    /**
     * @struct BlockTexture
     * @brief Texture data for blocks
     */
    struct BlockTexture {
        int width;                        ///< Texture width
        int height;                       ///< Texture height
        int depth;                        ///< Texture depth (for 3D textures)
        std::vector<unsigned char> data;  ///< RGBA texture data
        BlockTextureType type;            ///< Texture type
        Vec3 primaryColor;                ///< Primary color
        Vec3 secondaryColor;              ///< Secondary color
        Vec3 accentColor;                 ///< Accent color
        Vec3 emissiveColor;               ///< Glow color
        float emissiveIntensity;          ///< Glow intensity
        bool hasTransparency;             ///< Has transparent areas
        bool hasAnimation;                ///< Has animated frames
        int animationFrames;              ///< Number of animation frames
        std::string customData;           ///< Custom generation data
    };

    /**
     * @struct BlockModel
     * @brief 3D model data for blocks
     */
    struct BlockModel {
        std::vector<float> vertices;      ///< Vertex data
        std::vector<float> normals;       ///< Normal data
        std::vector<float> texCoords;     ///< Texture coordinates
        std::vector<float> colors;        ///< Vertex colors
        std::vector<unsigned int> indices; ///< Index data
        Vec3 boundingBoxMin;              ///< Bounding box minimum
        Vec3 boundingBoxMax;              ///< Bounding box maximum
        float scale;                      ///< Model scale
        bool hasTransparency;             ///< Has transparent parts
        bool isDoubleSided;               ///< Render both sides
        int lodLevels;                    ///< Level of detail levels
        std::vector<std::vector<float>> lodVertices; ///< LOD vertex data
        std::vector<std::vector<unsigned int>> lodIndices; ///< LOD index data
    };

    /**
     * @struct BlockSounds
     * @brief Sound data for blocks
     */
    struct BlockSounds {
        void* placeSound;                 ///< Block placement sound
        void* breakSound;                 ///< Block breaking sound
        void* stepSound;                  ///< Walking on block sound
        void* hitSound;                   ///< Block hit sound
        void* fallSound;                  ///< Falling on block sound
        float volume;                     ///< Base volume
        float pitch;                      ///< Base pitch
        int variants;                     ///< Number of sound variants
        bool hasEcho;                     ///< Has echo effect
        float echoDelay;                  ///< Echo delay
    };

    /**
     * @struct BlockParticles
     * @brief Particle effect data for blocks
     */
    struct BlockParticles {
        std::string particleType;         ///< Type of particles
        Vec3 particleColor;               ///< Particle color
        Vec3 particleVelocity;            ///< Initial particle velocity
        float particleSize;               ///< Particle size
        float particleLifetime;           ///< Particle lifetime
        int particleCount;                ///< Number of particles
        float spawnRate;                  ///< Particles per second
        bool hasGravity;                  ///< Particles affected by gravity
        bool hasLight;                    ///< Particles emit light
        Vec3 lightColor;                  ///< Light color
        float lightIntensity;             ///< Light intensity
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_BLOCKS_BLOCK_GENERATOR_HPP
