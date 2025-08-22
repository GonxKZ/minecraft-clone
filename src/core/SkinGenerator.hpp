/**
 * @file SkinGenerator.hpp
 * @brief VoxelCraft Procedural Skin & Appearance Generator
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_CORE_SKIN_GENERATOR_HPP
#define VOXELCRAFT_CORE_SKIN_GENERATOR_HPP

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <random>
#include <functional>
#include "ProceduralGenerator.hpp"

namespace VoxelCraft {

    // Forward declarations
    struct Vec3;

    /**
     * @enum SkinType
     * @brief Types of skins that can be generated
     */
    enum class SkinType {
        PLAYER = 0,      ///< Player character skins
        MOB,             ///< Mob/creature skins
        ITEM,            ///< Item appearances
        BLOCK,           ///< Block textures
        ENVIRONMENT,     ///< Environmental elements
        CUSTOM           ///< Custom skin types
    };

    /**
     * @enum SkinStyle
     * @brief Visual styles for skin generation
     */
    enum class SkinStyle {
        REALISTIC = 0,   ///< Realistic appearance
        CARTOON,         ///< Cartoon/stylized
        FANTASY,         ///< Fantasy/medieval
        SCIFI,           ///< Sci-fi/futuristic
        RETRO,           ///< Retro/pixel art
        ABSTRACT,        ///< Abstract/modern
        CUSTOM           ///< Custom style
    };

    /**
     * @enum ColorPalette
     * @brief Color schemes for skin generation
     */
    enum class ColorPalette {
        EARTHY = 0,      ///< Earth tones (browns, greens)
        OCEANIC,         ///< Ocean tones (blues, cyans)
        FIERY,           ///< Fire tones (reds, oranges)
        ICY,             ///< Ice tones (whites, blues)
        MAGICAL,         ///< Magical tones (purples, pinks)
        METALLIC,        ///< Metal tones (silvers, golds)
        PASTEL,          ///< Pastel colors
        NEON,            ///< Neon/bright colors
        MONOCHROME,      ///< Black and white
        CUSTOM           ///< Custom palette
    };

    /**
     * @struct SkinParameters
     * @brief Parameters for skin generation
     */
    struct SkinParameters {
        SkinType type;                    ///< Type of skin to generate
        SkinStyle style;                  ///< Visual style
        ColorPalette palette;             ///< Color palette
        uint64_t seed;                    ///< Generation seed
        int complexity;                   ///< Detail complexity (1-10)
        int symmetry;                     ///< Symmetry level (0-10)
        bool allowPatterns;               ///< Allow pattern generation
        bool allowGlow;                   ///< Allow glowing effects
        bool allowMetallic;               ///< Allow metallic effects
        bool allowTransparency;           ///< Allow transparent areas
        std::vector<std::string> features; ///< Specific features to include
        std::unordered_map<std::string, std::string> customParams; ///< Custom parameters
    };

    /**
     * @struct GeneratedSkin
     * @brief Complete skin data generated procedurally
     */
    struct GeneratedSkin {
        std::string id;                   ///< Unique skin identifier
        std::string name;                 ///< Skin name
        SkinType type;                    ///< Skin type
        SkinStyle style;                  ///< Visual style
        void* textureData;                ///< Generated texture data
        void* normalMapData;              ///< Generated normal map
        void* roughnessMapData;           ///< Generated roughness map
        void* metallicMapData;            ///< Generated metallic map
        void* emissiveMapData;            ///< Generated emissive map
        std::vector<std::string> features; ///< Generated features
        std::vector<std::string> patterns; ///< Generated patterns
        Vec3 primaryColor;                ///< Primary color
        Vec3 secondaryColor;              ///< Secondary color
        Vec3 accentColor;                 ///< Accent color
        Vec3 glowColor;                   ///< Glow color
        float glowIntensity;              ///< Glow intensity
        int textureWidth;                 ///< Texture width
        int textureHeight;                ///< Texture height
        bool hasTransparency;             ///< Has transparent areas
        bool hasGlow;                     ///< Has glowing effects
        bool hasMetallic;                 ///< Has metallic effects
        bool isAnimated;                  ///< Has animation frames
        int animationFrames;              ///< Number of animation frames
        std::unordered_map<std::string, std::string> metadata; ///< Additional metadata
    };

    /**
     * @struct SkinLayer
     * @brief Individual layer of a skin
     */
    struct SkinLayer {
        std::string name;                 ///< Layer name
        Vec3 color;                       ///< Layer color
        float opacity;                    ///< Layer opacity
        int blendMode;                    ///< Blend mode (0=normal, 1=additive, 2=multiply)
        std::vector<unsigned char> data;  ///< Layer texture data
        int width;                        ///< Layer width
        int height;                       ///< Layer height
        bool isPattern;                   ///< Is this a pattern layer
        bool isGlow;                      ///< Is this a glow layer
    };

    /**
     * @struct SkinPattern
     * @brief Pattern definition for skins
     */
    struct SkinPattern {
        std::string name;                 ///< Pattern name
        std::string category;             ///< Pattern category
        int complexity;                   ///< Pattern complexity
        Vec3 primaryColor;                ///< Primary pattern color
        Vec3 secondaryColor;              ///< Secondary pattern color
        std::function<void(std::vector<unsigned char>&, int, int, uint64_t)> generator; ///< Pattern generator function
    };

    /**
     * @class SkinGenerator
     * @brief Procedural skin and appearance generator
     */
    class SkinGenerator {
    public:
        /**
         * @brief Constructor
         */
        SkinGenerator();

        /**
         * @brief Destructor
         */
        ~SkinGenerator();

        /**
         * @brief Initialize the skin generator
         * @param seed Random seed for generation
         */
        void Initialize(uint64_t seed);

        /**
         * @brief Generate a skin with specified parameters
         * @param params Skin generation parameters
         * @return Generated skin data
         */
        GeneratedSkin GenerateSkin(const SkinParameters& params);

        /**
         * @brief Generate a player skin
         * @param seed Generation seed
         * @param style Visual style
         * @return Generated player skin
         */
        GeneratedSkin GeneratePlayerSkin(uint64_t seed, SkinStyle style = SkinStyle::REALISTIC);

        /**
         * @brief Generate a mob skin
         * @param mobType Type of mob
         * @param seed Generation seed
         * @param style Visual style
         * @return Generated mob skin
         */
        GeneratedSkin GenerateMobSkin(const std::string& mobType, uint64_t seed, SkinStyle style = SkinStyle::REALISTIC);

        /**
         * @brief Generate an item skin
         * @param itemType Type of item
         * @param seed Generation seed
         * @param style Visual style
         * @return Generated item skin
         */
        GeneratedSkin GenerateItemSkin(const std::string& itemType, uint64_t seed, SkinStyle style = SkinStyle::REALISTIC);

        /**
         * @brief Generate a block skin/texture
         * @param blockType Type of block
         * @param seed Generation seed
         * @param style Visual style
         * @return Generated block skin
         */
        GeneratedSkin GenerateBlockSkin(const std::string& blockType, uint64_t seed, SkinStyle style = SkinStyle::REALISTIC);

        /**
         * @brief Generate a custom skin
         * @param customType Custom skin type
         * @param seed Generation seed
         * @param params Custom parameters
         * @return Generated custom skin
         */
        GeneratedSkin GenerateCustomSkin(const std::string& customType, uint64_t seed, const std::unordered_map<std::string, std::string>& params);

        /**
         * @brief Combine multiple skin layers
         * @param layers Vector of skin layers
         * @param width Output texture width
         * @param height Output texture height
         * @return Combined texture data
         */
        std::vector<unsigned char> CombineLayers(const std::vector<SkinLayer>& layers, int width, int height);

        /**
         * @brief Apply color palette to skin
         * @param skin Skin to modify
         * @param palette Color palette to apply
         */
        void ApplyColorPalette(GeneratedSkin& skin, ColorPalette palette);

        /**
         * @brief Add pattern to skin
         * @param skin Skin to modify
         * @param patternName Pattern name
         * @param seed Generation seed
         */
        void AddPattern(GeneratedSkin& skin, const std::string& patternName, uint64_t seed);

        /**
         * @brief Generate normal map from texture
         * @param textureData Source texture data
         * @param width Texture width
         * @param height Texture height
         * @param strength Normal map strength
         * @return Generated normal map data
         */
        std::vector<unsigned char> GenerateNormalMap(const std::vector<unsigned char>& textureData, int width, int height, float strength = 1.0f);

        /**
         * @brief Generate roughness map
         * @param textureData Source texture data
         * @param width Texture width
         * @param height Texture height
         * @return Generated roughness map data
         */
        std::vector<unsigned char> GenerateRoughnessMap(const std::vector<unsigned char>& textureData, int width, int height);

        /**
         * @brief Generate metallic map
         * @param textureData Source texture data
         * @param width Texture width
         * @param height Texture height
         * @param metallicIntensity Metallic intensity (0-1)
         * @return Generated metallic map data
         */
        std::vector<unsigned char> GenerateMetallicMap(const std::vector<unsigned char>& textureData, int width, int height, float metallicIntensity = 0.0f);

        /**
         * @brief Generate emissive map
         * @param textureData Source texture data
         * @param width Texture width
         * @param height Texture height
         * @param glowColor Glow color
         * @return Generated emissive map data
         */
        std::vector<unsigned char> GenerateEmissiveMap(const std::vector<unsigned char>& textureData, int width, int height, const Vec3& glowColor);

        /**
         * @brief Save skin to file
         * @param skin Skin to save
         * @param filename Output filename
         * @return true if saved successfully
         */
        bool SaveSkinToFile(const GeneratedSkin& skin, const std::string& filename);

        /**
         * @brief Load skin from file
         * @param filename Input filename
         * @return Loaded skin data
         */
        GeneratedSkin LoadSkinFromFile(const std::string& filename);

    private:
        uint64_t m_seed;
        std::mt19937_64 m_randomEngine;

        // Pattern definitions
        std::unordered_map<std::string, SkinPattern> m_patterns;

        // Style generators
        std::unordered_map<SkinStyle, std::function<GeneratedSkin(const SkinParameters&)>> m_styleGenerators;

        // Color palettes
        std::unordered_map<ColorPalette, std::vector<Vec3>> m_colorPalettes;

        /**
         * @brief Initialize pattern definitions
         */
        void InitializePatterns();

        /**
         * @brief Initialize style generators
         */
        void InitializeStyleGenerators();

        /**
         * @brief Initialize color palettes
         */
        void InitializeColorPalettes();

        /**
         * @brief Generate base texture layer
         * @param width Texture width
         * @param height Texture height
         * @param baseColor Base color
         * @param seed Generation seed
         * @return Generated texture data
         */
        std::vector<unsigned char> GenerateBaseLayer(int width, int height, const Vec3& baseColor, uint64_t seed);

        /**
         * @brief Apply noise to texture
         * @param textureData Texture data to modify
         * @param width Texture width
         * @param height Texture height
         * @param noiseScale Noise scale
         * @param intensity Noise intensity
         * @param seed Generation seed
         */
        void ApplyNoise(std::vector<unsigned char>& textureData, int width, int height, float noiseScale, float intensity, uint64_t seed);

        /**
         * @brief Apply gradient to texture
         * @param textureData Texture data to modify
         * @param width Texture width
         * @param height Texture height
         * @param startColor Start color
         * @param endColor End color
         * @param direction Gradient direction (0=horizontal, 1=vertical, 2=radial)
         */
        void ApplyGradient(std::vector<unsigned char>& textureData, int width, int height, const Vec3& startColor, const Vec3& endColor, int direction);

        /**
         * @brief Apply pattern to texture
         * @param textureData Texture data to modify
         * @param width Texture width
         * @param height Texture height
         * @param patternName Pattern name
         * @param color Pattern color
         * @param seed Generation seed
         */
        void ApplyPattern(std::vector<unsigned char>& textureData, int width, int height, const std::string& patternName, const Vec3& color, uint64_t seed);

        /**
         * @brief Apply glow effect to texture
         * @param textureData Texture data to modify
         * @param width Texture width
         * @param height Texture height
         * @param glowColor Glow color
         * @param intensity Glow intensity
         */
        void ApplyGlow(std::vector<unsigned char>& textureData, int width, int height, const Vec3& glowColor, float intensity);

        /**
         * @brief Apply metallic effect to texture
         * @param textureData Texture data to modify
         * @param width Texture width
         * @param height Texture height
         * @param metallicColor Metallic color
         * @param intensity Metallic intensity
         */
        void ApplyMetallic(std::vector<unsigned char>& textureData, int width, int height, const Vec3& metallicColor, float intensity);

        /**
         * @brief Generate realistic style skin
         * @param params Skin parameters
         * @return Generated skin
         */
        GeneratedSkin GenerateRealisticSkin(const SkinParameters& params);

        /**
         * @brief Generate cartoon style skin
         * @param params Skin parameters
         * @return Generated skin
         */
        GeneratedSkin GenerateCartoonSkin(const SkinParameters& params);

        /**
         * @brief Generate fantasy style skin
         * @param params Skin parameters
         * @return Generated skin
         */
        GeneratedSkin GenerateFantasySkin(const SkinParameters& params);

        /**
         * @brief Generate sci-fi style skin
         * @param params Skin parameters
         * @return Generated skin
         */
        GeneratedSkin GenerateSciFiSkin(const SkinParameters& params);

        /**
         * @brief Generate retro style skin
         * @param params Skin parameters
         * @return Generated skin
         */
        GeneratedSkin GenerateRetroSkin(const SkinParameters& params);

        /**
         * @brief Generate abstract style skin
         * @param params Skin parameters
         * @return Generated skin
         */
        GeneratedSkin GenerateAbstractSkin(const SkinParameters& params);

        // Pattern generation methods
        void GenerateStripesPattern(std::vector<unsigned char>& data, int width, int height, uint64_t seed);
        void GenerateSpotsPattern(std::vector<unsigned char>& data, int width, int height, uint64_t seed);
        void GenerateMarblePattern(std::vector<unsigned char>& data, int width, int height, uint64_t seed);
        void GenerateWoodGrainPattern(std::vector<unsigned char>& data, int width, int height, uint64_t seed);
        void GenerateCrystalPattern(std::vector<unsigned char>& data, int width, int height, uint64_t seed);
        void GenerateCircuitPattern(std::vector<unsigned char>& data, int width, int height, uint64_t seed);
        void GenerateScalePattern(std::vector<unsigned char>& data, int width, int height, uint64_t seed);
        void GenerateFurPattern(std::vector<unsigned char>& data, int width, int height, uint64_t seed);
        void GenerateCamouflagePattern(std::vector<unsigned char>& data, int width, int height, uint64_t seed);
        void GenerateTribalPattern(std::vector<unsigned char>& data, int width, int height, uint64_t seed);

        // Specific skin generators
        GeneratedSkin GenerateHumanSkin(uint64_t seed, SkinStyle style);
        GeneratedSkin GenerateAnimalSkin(const std::string& animalType, uint64_t seed, SkinStyle style);
        GeneratedSkin GenerateMonsterSkin(const std::string& monsterType, uint64_t seed, SkinStyle style);
        GeneratedSkin GenerateRobotSkin(uint64_t seed, SkinStyle style);
        GeneratedSkin GenerateAlienSkin(uint64_t seed, SkinStyle style);
        GeneratedSkin GenerateUndeadSkin(const std::string& undeadType, uint64_t seed, SkinStyle style);
        GeneratedSkin GenerateElementalSkin(const std::string& elementType, uint64_t seed, SkinStyle style);
        GeneratedSkin GenerateMythicalSkin(const std::string& mythicalType, uint64_t seed, SkinStyle style);

        // Item skin generators
        GeneratedSkin GenerateWeaponSkin(const std::string& weaponType, uint64_t seed, SkinStyle style);
        GeneratedSkin GenerateArmorSkin(const std::string& armorType, uint64_t seed, SkinStyle style);
        GeneratedSkin GenerateToolSkin(const std::string& toolType, uint64_t seed, SkinStyle style);
        GeneratedSkin GenerateConsumableSkin(const std::string& consumableType, uint64_t seed, SkinStyle style);
        GeneratedSkin GenerateMagicalItemSkin(const std::string& itemType, uint64_t seed, SkinStyle style);

        // Block skin generators
        GeneratedSkin GenerateStoneBlockSkin(uint64_t seed, SkinStyle style);
        GeneratedSkin GenerateWoodBlockSkin(uint64_t seed, SkinStyle style);
        GeneratedSkin GenerateMetalBlockSkin(uint64_t seed, SkinStyle style);
        GeneratedSkin GenerateCrystalBlockSkin(uint64_t seed, SkinStyle style);
        GeneratedSkin GenerateOrganicBlockSkin(uint64_t seed, SkinStyle style);
        GeneratedSkin GenerateFluidBlockSkin(uint64_t seed, SkinStyle style);
    };

    /**
     * @struct SkinAnimation
     * @brief Animation data for animated skins
     */
    struct SkinAnimation {
        std::string name;                 ///< Animation name
        int frameCount;                   ///< Number of frames
        float frameDuration;              ///< Duration per frame (seconds)
        std::vector<void*> frames;        ///< Animation frames
        bool loop;                        ///< Should animation loop
        std::string trigger;              ///< Animation trigger condition
    };

    /**
     * @struct SkinVariation
     * @brief Variation data for skin variants
     */
    struct SkinVariation {
        std::string name;                 ///< Variation name
        std::vector<std::string> features; ///< Features in this variation
        Vec3 primaryColor;                ///< Primary color override
        Vec3 secondaryColor;              ///< Secondary color override
        int spawnWeight;                  ///< Spawn weight for this variation
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_CORE_SKIN_GENERATOR_HPP
