/**
 * @file TextureGenerator.hpp
 * @brief VoxelCraft Procedural Texture Generation System
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_TEXTURES_TEXTURE_GENERATOR_HPP
#define VOXELCRAFT_TEXTURES_TEXTURE_GENERATOR_HPP

#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
#include <atomic>
#include <mutex>
#include <functional>
#include <random>

#include "../math/Vec3.hpp"
#include "../math/Vec2.hpp"
#include "../math/Color.hpp"
#include "../math/Noise.hpp"

namespace VoxelCraft {

    // Forward declarations
    class World;
    class Block;
    struct TextureConfig;
    struct TextureData;
    struct NoiseConfig;
    struct ProceduralTexture;
    struct TextureStats;

    /**
     * @enum TextureType
     * @brief Types of procedural textures
     */
    enum class TextureType {
        BLOCK,              ///< Block textures
        TERRAIN,            ///< Terrain/ground textures
        BIOME,              ///< Biome-specific textures
        SEASONAL,           ///< Seasonal variation textures
        WEATHER,            ///< Weather-affected textures
        DEPTH,              ///< Depth-based textures
        TEMPERATURE,        ///< Temperature-based textures
        CUSTOM              ///< Custom texture type
    };

    /**
     * @enum NoiseType
     * @brief Types of noise functions
     */
    enum class NoiseType {
        PERLIN,             ///< Perlin noise
        SIMPLEX,            ///< Simplex noise
        VALUE,              ///< Value noise
        VORONOI,            ///< Voronoi noise
        WORLEY,             ///< Worley noise
        RIDGED,             ///< Ridged multifractal
        BILLLOW,            ///< Billowy noise
        CUSTOM              ///< Custom noise
    };

    /**
     * @enum TextureFilter
     * @brief Texture filtering modes
     */
    enum class TextureFilter {
        NEAREST,            ///< Nearest neighbor
        BILINEAR,           ///< Bilinear filtering
        TRILINEAR,          ///< Trilinear filtering
        ANISOTROPIC,        ///< Anisotropic filtering
        CUSTOM              ///< Custom filter
    };

    /**
     * @enum TextureFormat
     * @brief Supported texture formats
     */
    enum class TextureFormat {
        RGBA8,              ///< 32-bit RGBA
        RGB8,               ///< 24-bit RGB
        RGBA16,             ///< 64-bit RGBA
        RGB16,              ///< 48-bit RGB
        RGBA32F,            ///< 128-bit float RGBA
        R8,                 ///< 8-bit red channel
        RG8,                ///< 16-bit RG
        RGB5_A1,            ///< 16-bit RGBA (5-5-5-1)
        RGBA4,              ///< 16-bit RGBA (4-4-4-4)
        DXT1,               ///< DXT1 compression
        DXT3,               ///< DXT3 compression
        DXT5,               ///< DXT5 compression
        ETC2,               ///< ETC2 compression
        ASTC,               ///< ASTC compression
        CUSTOM              ///< Custom format
    };

    /**
     * @struct NoiseConfig
     * @brief Configuration for noise generation
     */
    struct NoiseConfig {
        NoiseType type = NoiseType::PERLIN;
        float frequency = 1.0f;
        float amplitude = 1.0f;
        float lacunarity = 2.0f;
        float persistence = 0.5f;
        int octaves = 4;
        float offsetX = 0.0f;
        float offsetY = 0.0f;
        float offsetZ = 0.0f;
        uint32_t seed = 0;

        // Advanced parameters
        float warpStrength = 0.0f;
        float warpFrequency = 1.0f;
        bool useGradient = false;
        Vec3 gradientDirection = Vec3(0.0f, 1.0f, 0.0f);

        // Animation
        bool animated = false;
        float animationSpeed = 1.0f;
        float timeOffset = 0.0f;
    };

    /**
     * @struct ColorConfig
     * @brief Color configuration for texture generation
     */
    struct ColorConfig {
        Color baseColor = Color(1.0f, 1.0f, 1.0f, 1.0f);
        Color secondaryColor = Color(0.5f, 0.5f, 0.5f, 1.0f);
        Color accentColor = Color(0.0f, 0.0f, 0.0f, 1.0f);
        float colorVariation = 0.1f;
        bool useGradient = false;
        Vec3 gradientStart = Vec3(0.0f, 0.0f, 0.0f);
        Vec3 gradientEnd = Vec3(1.0f, 1.0f, 1.0f);
        float gradientStrength = 1.0f;

        // Color mapping
        std::vector<Color> colorPalette;
        bool useColorMap = false;
        float colorMapScale = 1.0f;

        // Advanced color effects
        float saturation = 1.0f;
        float brightness = 1.0f;
        float contrast = 1.0f;
        float hueShift = 0.0f;
    };

    /**
     * @struct PatternConfig
     * @brief Pattern configuration for texture generation
     */
    struct PatternConfig {
        std::string patternType = "noise";
        float patternScale = 1.0f;
        float patternIntensity = 1.0f;
        float patternRotation = 0.0f;
        Vec2 patternOffset = Vec2(0.0f, 0.0f);

        // Pattern variations
        bool useVoronoi = false;
        float voronoiScale = 1.0f;
        bool useWorley = false;
        float worleyScale = 1.0f;
        bool useMarble = false;
        float marbleScale = 1.0f;
        bool useWood = false;
        float woodScale = 1.0f;
        bool useClouds = false;
        float cloudScale = 1.0f;

        // Pattern blending
        std::vector<std::string> blendPatterns;
        std::vector<float> blendWeights;
        std::string blendMode = "overlay";
    };

    /**
     * @struct TextureLayer
     * @brief Single layer of procedural texture
     */
    struct TextureLayer {
        std::string name;
        bool enabled = true;
        float opacity = 1.0f;
        std::string blendMode = "normal";

        // Layer properties
        NoiseConfig noiseConfig;
        ColorConfig colorConfig;
        PatternConfig patternConfig;

        // Layer effects
        float blurRadius = 0.0f;
        float sharpenStrength = 0.0f;
        float normalStrength = 1.0f;
        float displacementStrength = 0.0f;

        // Masking
        bool useMask = false;
        std::shared_ptr<TextureData> maskTexture;
        float maskThreshold = 0.5f;

        // Animation
        bool animated = false;
        float animationSpeed = 1.0f;
        Vec3 animationDirection = Vec3(0.0f, 0.0f, 0.0f);
    };

    /**
     * @struct ProceduralTexture
     * @brief Complete procedural texture definition
     */
    struct ProceduralTexture {
        std::string id;
        std::string name;
        std::string description;
        TextureType type = TextureType::BLOCK;
        TextureFormat format = TextureFormat::RGBA8;

        // Basic properties
        int width = 64;
        int height = 64;
        int depth = 1;  // For 3D textures
        bool is3D = false;
        bool isCubeMap = false;

        // Generation parameters
        std::vector<TextureLayer> layers;
        std::vector<NoiseConfig> noiseSources;
        ColorConfig globalColorConfig;

        // Post-processing
        bool useMipmaps = true;
        TextureFilter minFilter = TextureFilter::LINEAR;
        TextureFilter magFilter = TextureFilter::LINEAR;
        bool useAnisotropy = false;
        float anisotropyLevel = 1.0f;

        // Quality settings
        int qualityLevel = 1;  // 1-10
        bool useCompression = false;
        bool useDithering = false;
        int ditherStrength = 1;

        // Metadata
        std::unordered_map<std::string, std::string> tags;
        std::unordered_map<std::string, float> properties;
        uint64_t creationTime = 0;
        uint32_t version = 1;

        // Performance
        float generationTime = 0.0f;
        size_t memoryUsage = 0;
    };

    /**
     * @struct TextureData
     * @brief Generated texture data
     */
    struct TextureData {
        std::string textureId;
        TextureFormat format = TextureFormat::RGBA8;
        int width = 0;
        int height = 0;
        int depth = 0;
        size_t dataSize = 0;
        std::vector<uint8_t> pixelData;

        // Metadata
        std::string generatorId;
        uint64_t generationTime = 0;
        std::unordered_map<std::string, float> parameters;

        // GPU data
        uint32_t textureHandle = 0;
        bool uploadedToGPU = false;
        uint32_t mipmapsGenerated = 0;

        // Performance metrics
        float loadTime = 0.0f;
        float uploadTime = 0.0f;
        size_t gpuMemoryUsage = 0;
    };

    /**
     * @struct TextureStats
     * @brief Performance statistics for texture system
     */
    struct TextureStats {
        int totalTexturesGenerated = 0;
        int texturesInCache = 0;
        int cacheHits = 0;
        int cacheMisses = 0;
        float averageGenerationTime = 0.0f;
        float averageLoadTime = 0.0f;
        size_t totalMemoryUsage = 0;
        size_t gpuMemoryUsage = 0;
        int activeGenerators = 0;
        int queuedGenerations = 0;
        int failedGenerations = 0;
    };

    /**
     * @class TextureGenerator
     * @brief Advanced procedural texture generation system
     *
     * Features:
     * - 10,000+ procedural texture variations
     * - Multi-layer texture composition
     * - Advanced noise functions (Perlin, Simplex, Voronoi, etc.)
     * - Dynamic texture streaming and caching
     * - Real-time texture animation
     * - GPU-accelerated generation
     * - Texture compression and optimization
     * - Biome and environment-based texturing
     * - Seasonal and weather texture variations
     * - Custom texture creation tools
     * - Texture atlasing and batching
     * - Normal mapping and PBR support
     * - Multi-threading and async generation
     */
    class TextureGenerator {
    public:
        static TextureGenerator& GetInstance();

        /**
         * @brief Initialize the texture generator
         * @param configPath Configuration file path
         * @return true if successful
         */
        bool Initialize(const std::string& configPath = "");

        /**
         * @brief Shutdown the texture generator
         */
        void Shutdown();

        /**
         * @brief Update texture generator (call every frame)
         * @param deltaTime Time since last update
         */
        void Update(float deltaTime);

        // Texture Generation
        /**
         * @brief Generate a procedural texture
         * @param textureDef Texture definition
         * @param seed Random seed
         * @return Generated texture data
         */
        std::shared_ptr<TextureData> GenerateTexture(const ProceduralTexture& textureDef,
                                                   uint32_t seed = 0);

        /**
         * @brief Generate texture asynchronously
         * @param textureDef Texture definition
         * @param seed Random seed
         * @param callback Completion callback
         * @return Generation handle
         */
        uint64_t GenerateTextureAsync(const ProceduralTexture& textureDef,
                                    uint32_t seed,
                                    std::function<void(std::shared_ptr<TextureData>)> callback);

        /**
         * @brief Cancel async texture generation
         * @param handle Generation handle
         * @return true if cancelled
         */
        bool CancelAsyncGeneration(uint64_t handle);

        // Preset Textures
        /**
         * @brief Generate block texture
         * @param blockType Block type identifier
         * @param biome Biome identifier
         * @param quality Quality level (1-10)
         * @return Generated texture data
         */
        std::shared_ptr<TextureData> GenerateBlockTexture(const std::string& blockType,
                                                        const std::string& biome = "",
                                                        int quality = 5);

        /**
         * @brief Generate terrain texture
         * @param position World position
         * @param biome Biome identifier
         * @param height Height value (0-1)
         * @param temperature Temperature (-1 to 1)
         * @param moisture Moisture level (0-1)
         * @return Generated texture data
         */
        std::shared_ptr<TextureData> GenerateTerrainTexture(const Vec3& position,
                                                          const std::string& biome,
                                                          float height,
                                                          float temperature,
                                                          float moisture);

        /**
         * @brief Generate biome-specific texture
         * @param biome Biome identifier
         * @param season Season identifier
         * @param weather Weather condition
         * @return Generated texture data
         */
        std::shared_ptr<TextureData> GenerateBiomeTexture(const std::string& biome,
                                                        const std::string& season = "",
                                                        const std::string& weather = "");

        // Texture Manipulation
        /**
         * @brief Apply effects to existing texture
         * @param texture Input texture
         * @param effects Effect parameters
         * @return Modified texture
         */
        std::shared_ptr<TextureData> ApplyTextureEffects(std::shared_ptr<TextureData> texture,
                                                       const std::unordered_map<std::string, float>& effects);

        /**
         * @brief Blend two textures
         * @param texture1 First texture
         * @param texture2 Second texture
         * @param blendMode Blend mode
         * @param opacity Blend opacity (0-1)
         * @return Blended texture
         */
        std::shared_ptr<TextureData> BlendTextures(std::shared_ptr<TextureData> texture1,
                                                 std::shared_ptr<TextureData> texture2,
                                                 const std::string& blendMode = "normal",
                                                 float opacity = 1.0f);

        /**
         * @brief Generate normal map from height map
         * @param heightMap Height map texture
         * @param strength Normal strength
         * @return Normal map texture
         */
        std::shared_ptr<TextureData> GenerateNormalMap(std::shared_ptr<TextureData> heightMap,
                                                     float strength = 1.0f);

        /**
         * @brief Generate roughness map
         * @param baseTexture Base texture
         * @param roughness Roughness value (0-1)
         * @return Roughness map texture
         */
        std::shared_ptr<TextureData> GenerateRoughnessMap(std::shared_ptr<TextureData> baseTexture,
                                                        float roughness = 0.5f);

        // Caching and Optimization
        /**
         * @brief Cache texture data
         * @param textureId Texture identifier
         * @param textureData Texture data
         * @return true if cached
         */
        bool CacheTexture(const std::string& textureId, std::shared_ptr<TextureData> textureData);

        /**
         * @brief Get cached texture
         * @param textureId Texture identifier
         * @return Cached texture data or nullptr
         */
        std::shared_ptr<TextureData> GetCachedTexture(const std::string& textureId);

        /**
         * @brief Clear texture cache
         * @param olderThan Time threshold (seconds)
         * @return Number of textures cleared
         */
        int ClearCache(float olderThan = 0.0f);

        /**
         * @brief Optimize texture for GPU
         * @param texture Input texture
         * @return Optimized texture
         */
        std::shared_ptr<TextureData> OptimizeForGPU(std::shared_ptr<TextureData> texture);

        // Texture Library
        /**
         * @brief Register texture preset
         * @param presetId Preset identifier
         * @param textureDef Texture definition
         * @return true if registered
         */
        bool RegisterTexturePreset(const std::string& presetId, const ProceduralTexture& textureDef);

        /**
         * @brief Get texture preset
         * @param presetId Preset identifier
         * @return Texture definition or nullptr
         */
        const ProceduralTexture* GetTexturePreset(const std::string& presetId);

        /**
         * @brief List all presets
         * @param type Filter by texture type
         * @return List of preset IDs
         */
        std::vector<std::string> ListTexturePresets(TextureType type = TextureType::CUSTOM);

        // Advanced Generation
        /**
         * @brief Generate texture atlas
         * @param textures List of textures
         * @param maxWidth Maximum atlas width
         * @param maxHeight Maximum atlas height
         * @return Atlas texture and UV coordinates
         */
        std::pair<std::shared_ptr<TextureData>, std::vector<Vec4>> GenerateAtlas(
            const std::vector<std::shared_ptr<TextureData>>& textures,
            int maxWidth = 2048, int maxHeight = 2048);

        /**
         * @brief Generate animated texture
         * @param baseTexture Base texture
         * @param frames Number of frames
         * @param frameRate Frame rate
         * @return Animated texture data
         */
        std::shared_ptr<TextureData> GenerateAnimatedTexture(std::shared_ptr<TextureData> baseTexture,
                                                           int frames = 16, float frameRate = 30.0f);

        /**
         * @brief Generate 3D texture
         * @param textureDef 3D texture definition
         * @param depth 3D depth
         * @return 3D texture data
         */
        std::shared_ptr<TextureData> Generate3DTexture(const ProceduralTexture& textureDef, int depth);

        // Configuration
        /**
         * @brief Set texture generator configuration
         * @param config Configuration data
         */
        void SetConfig(const std::unordered_map<std::string, float>& config);

        /**
         * @brief Get configuration value
         * @param key Configuration key
         * @return Configuration value
         */
        float GetConfigValue(const std::string& key) const;

        // Statistics
        /**
         * @brief Get texture generator statistics
         * @return Current statistics
         */
        const TextureStats& GetStats() const { return m_stats; }

        /**
         * @brief Reset statistics
         */
        void ResetStats();

        // Debug
        /**
         * @brief Enable debug mode
         * @param enable Enable state
         */
        void EnableDebugMode(bool enable) { m_debugMode = enable; }

        /**
         * @brief Get debug information
         * @return Debug info string
         */
        std::string GetDebugInfo() const;

        /**
         * @brief Validate texture generator
         * @return true if valid
         */
        bool Validate() const;

    private:
        TextureGenerator() = default;
        ~TextureGenerator();

        // Prevent copying
        TextureGenerator(const TextureGenerator&) = delete;
        TextureGenerator& operator=(const TextureGenerator&) = delete;

        // Core generation methods
        std::shared_ptr<TextureData> GenerateNoiseTexture(const NoiseConfig& config, int width, int height);
        std::shared_ptr<TextureData> GeneratePatternTexture(const PatternConfig& config, int width, int height);
        std::shared_ptr<TextureData> GenerateColorTexture(const ColorConfig& config, int width, int height);
        std::shared_ptr<TextureData> GenerateLayerTexture(const TextureLayer& layer, int width, int height);

        // Noise generation methods
        float GeneratePerlinNoise(float x, float y, float z, const NoiseConfig& config) const;
        float GenerateSimplexNoise(float x, float y, float z, const NoiseConfig& config) const;
        float GenerateValueNoise(float x, float y, float z, const NoiseConfig& config) const;
        float GenerateVoronoiNoise(float x, float y, float z, const NoiseConfig& config) const;
        float GenerateWorleyNoise(float x, float y, float z, const NoiseConfig& config) const;
        float GenerateRidgedNoise(float x, float y, float z, const NoiseConfig& config) const;
        float GenerateBillowNoise(float x, float y, float z, const NoiseConfig& config) const;

        // Pattern generation methods
        std::shared_ptr<TextureData> GenerateMarblePattern(int width, int height, float scale);
        std::shared_ptr<TextureData> GenerateWoodPattern(int width, int height, float scale);
        std::shared_ptr<TextureData> GenerateCloudPattern(int width, int height, float scale);

        // Texture processing methods
        std::shared_ptr<TextureData> ApplyBlur(std::shared_ptr<TextureData> texture, float radius);
        std::shared_ptr<TextureData> ApplySharpen(std::shared_ptr<TextureData> texture, float strength);
        std::shared_ptr<TextureData> ApplyNormalMap(std::shared_ptr<TextureData> texture, float strength);
        std::shared_ptr<TextureData> ApplyDisplacement(std::shared_ptr<TextureData> texture, float strength);

        // Utility methods
        Color SampleColor(const ColorConfig& config, float x, float y, float noiseValue) const;
        float SampleNoise(const NoiseConfig& config, float x, float y, float z = 0.0f) const;
        Color BlendColors(const Color& color1, const Color& color2, const std::string& blendMode, float opacity) const;
        std::vector<uint8_t> CompressTextureData(const std::vector<uint8_t>& data, TextureFormat format);

        // Random number generation
        float RandomFloat(float min, float max) const;
        int RandomInt(int min, int max) const;
        uint32_t RandomSeed() const;

        // Member variables
        bool m_initialized = false;
        bool m_debugMode = false;

        // Texture library
        std::unordered_map<std::string, ProceduralTexture> m_texturePresets;
        std::unordered_map<std::string, std::shared_ptr<TextureData>> m_textureCache;

        // Generation state
        std::unordered_map<std::string, float> m_config;
        std::mt19937 m_randomEngine;
        mutable std::shared_mutex m_generatorMutex;

        // Async generation
        struct AsyncGeneration {
            uint64_t handle;
            ProceduralTexture textureDef;
            uint32_t seed;
            std::function<void(std::shared_ptr<TextureData>)> callback;
            std::atomic<bool> cancelled;
        };
        std::unordered_map<uint64_t, AsyncGeneration> m_asyncGenerations;
        std::atomic<uint64_t> m_nextHandle;

        // Statistics
        TextureStats m_stats;

        // GPU resources
        uint32_t m_computeShaderProgram = 0;
        uint32_t m_textureComputeBuffer = 0;
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_TEXTURES_TEXTURE_GENERATOR_HPP
