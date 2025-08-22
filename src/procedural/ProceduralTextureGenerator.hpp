/**
 * @file ProceduralTextureGenerator.hpp
 * @brief VoxelCraft Procedural Texture Generation System
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines the ProceduralTextureGenerator class that provides comprehensive
 * procedural texture generation for the VoxelCraft game engine, including 2D/3D textures,
 * noise-based patterns, fractal textures, cellular automata textures, and advanced
 * texture synthesis with real-time generation capabilities for infinite variety.
 */

#ifndef VOXELCRAFT_PROCEDURAL_PROCEDURAL_TEXTURE_GENERATOR_HPP
#define VOXELCRAFT_PROCEDURAL_PROCEDURAL_TEXTURE_GENERATOR_HPP

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
#include <array>
#include <random>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/noise.hpp>

#include "../core/Config.hpp"

namespace VoxelCraft {

    // Forward declarations
    class ProceduralAssetGenerator;
    class NoiseGenerator;

    /**
     * @enum TextureType
     * @brief Types of procedural textures
     */
    enum class TextureType {
        Albedo,                     ///< Color/diffuse texture
        Normal,                     ///< Normal map texture
        Roughness,                  ///< Roughness map texture
        Metallic,                   ///< Metallic map texture
        AO,                         ///< Ambient occlusion texture
        Height,                     ///< Height/displacement map
        Emission,                   ///< Emission/glow texture
        Opacity,                    ///< Opacity/alpha texture
        Custom                      ///< Custom texture type
    };

    /**
     * @enum TexturePattern
     * @brief Procedural texture patterns
     */
    enum class TexturePattern {
        Noise,                      ///< Noise-based patterns
        Fractal,                    ///< Fractal patterns
        Cellular,                   ///< Cellular automata patterns
        Voronoi,                    ///< Voronoi diagram patterns
        Marble,                     ///< Marble/stone patterns
        Wood,                       ///< Wood grain patterns
        Clouds,                     ///< Cloud patterns
        Fire,                       ///< Fire/flame patterns
        Water,                      ///< Water/liquid patterns
        Crystal,                    ///< Crystal/geometric patterns
        Organic,                    ///< Organic/natural patterns
        Terrain,                    ///< Terrain/heightmap patterns
        Custom                      ///< Custom pattern
    };

    /**
     * @enum ColorPalette
     * @brief Color palette presets for textures
     */
    enum class ColorPalette {
        Earth,                      ///< Earth tones (browns, greens)
        Ocean,                      ///< Ocean colors (blues)
        Fire,                       ///< Fire colors (reds, oranges, yellows)
        Ice,                        ///< Ice colors (blues, whites)
        Metal,                      ///< Metal colors (grays, silvers)
        Crystal,                    ///< Crystal colors (purples, blues)
        Forest,                     ///< Forest colors (greens)
        Desert,                     ///< Desert colors (yellows, browns)
        Volcanic,                   ///< Volcanic colors (reds, blacks)
        Alien,                      ///< Alien colors (purples, greens)
        Custom                      ///< Custom color palette
    };

    /**
     * @struct TextureGenerationParams
     * @brief Parameters for procedural texture generation
     */
    struct TextureGenerationParams {
        // Basic parameters
        TextureType textureType;                    ///< Type of texture to generate
        TexturePattern pattern;                     ///< Pattern to use
        ColorPalette colorPalette;                  ///< Color palette to use

        // Resolution and dimensions
        int width;                                  ///< Texture width
        int height;                                 ///< Texture height
        int depth;                                  ///< Texture depth (for 3D textures)
        int channels;                               ///< Number of color channels

        // Generation parameters
        float scale;                                ///< Pattern scale
        float detail;                               ///< Detail level (0.0 - 1.0)
        float contrast;                             ///< Texture contrast
        float brightness;                            ///< Texture brightness
        float saturation;                            ///< Color saturation

        // Noise parameters
        int octaves;                                ///< Number of noise octaves
        float persistence;                          ///< Noise persistence
        float lacunarity;                           ///< Noise lacunarity
        uint32_t seed;                              ///< Random seed

        // Pattern-specific parameters
        std::unordered_map<std::string, float> patternParams; ///< Pattern-specific parameters

        // Post-processing
        bool enablePostProcessing;                  ///< Enable post-processing
        std::vector<std::string> postProcessEffects; ///< Post-processing effects
        float blurRadius;                           ///< Blur radius
        float sharpenAmount;                        ///< Sharpen amount
        bool enableTiling;                          ///< Enable seamless tiling

        TextureGenerationParams()
            : textureType(TextureType::Albedo)
            , pattern(TexturePattern::Noise)
            , colorPalette(ColorPalette::Earth)
            , width(512)
            , height(512)
            , depth(1)
            , channels(4)
            , scale(1.0f)
            , detail(0.5f)
            , contrast(1.0f)
            , brightness(1.0f)
            , saturation(1.0f)
            , octaves(4)
            , persistence(0.5f)
            , lacunarity(2.0f)
            , seed(0)
            , enablePostProcessing(true)
            , blurRadius(0.0f)
            , sharpenAmount(0.0f)
            , enableTiling(true)
        {}
    };

    /**
     * @struct GeneratedTexture
     * @brief Generated texture data
     */
    struct GeneratedTexture {
        std::string textureId;                      ///< Unique texture identifier
        TextureType textureType;                    ///< Type of generated texture
        int width;                                  ///< Texture width
        int height;                                 ///< Texture height
        int channels;                               ///< Number of channels
        std::vector<uint8_t> data;                  ///< Raw texture data (RGBA)

        // Metadata
        TextureGenerationParams params;              ///< Generation parameters used
        double generationTime;                       ///< Time taken to generate
        size_t memoryUsage;                          ///< Memory usage
        std::string checksum;                        ///< Data checksum

        // Quality metrics
        float detailLevel;                           ///< Detail level achieved
        float contrastLevel;                         ///< Contrast level achieved
        float brightnessLevel;                       ///< Brightness level achieved

        GeneratedTexture()
            : width(0)
            , height(0)
            , channels(0)
            , generationTime(0.0)
            , memoryUsage(0)
            , detailLevel(0.0f)
            , contrastLevel(0.0f)
            , brightnessLevel(0.0f)
        {}
    };

    /**
     * @struct TextureGenerationStats
     * @brief Texture generation performance statistics
     */
    struct TextureGenerationStats {
        // Generation statistics
        uint64_t texturesGenerated;                 ///< Total textures generated
        uint64_t generationAttempts;                ///< Total generation attempts
        uint64_t successfulGenerations;             ///< Successful generations
        uint64_t failedGenerations;                 ///< Failed generations

        // Performance metrics
        double averageGenerationTime;               ///< Average generation time (ms)
        double minGenerationTime;                   ///< Minimum generation time (ms)
        double maxGenerationTime;                   ///< Maximum generation time (ms)
        double totalGenerationTime;                 ///< Total generation time (ms)

        // Quality metrics
        float averageDetailLevel;                   ///< Average detail level
        float averageContrast;                      ///< Average contrast
        float averageBrightness;                    ///< Average brightness

        // Memory metrics
        size_t totalMemoryUsed;                     ///< Total memory used
        size_t peakMemoryUsed;                      ///< Peak memory usage
        uint32_t activeTextures;                    ///< Number of active textures

        // Cache metrics
        uint32_t cacheHits;                         ///< Cache hits
        uint32_t cacheMisses;                       ///< Cache misses
        float cacheHitRate;                         ///< Cache hit rate (0-1)
    };

    /**
     * @class ProceduralTextureGenerator
     * @brief Advanced procedural texture generation system
     *
     * The ProceduralTextureGenerator class provides comprehensive procedural texture
     * generation for the VoxelCraft game engine, featuring noise-based patterns,
     * fractal generation, cellular automata, advanced color palettes, and real-time
     * texture synthesis with GPU acceleration support for infinite texture variety
     * and dynamic content generation.
     *
     * Key Features:
     * - Multiple texture patterns (noise, fractal, cellular, voronoi)
     * - Advanced color palette system with 10+ presets
     * - Real-time texture generation with GPU acceleration
     * - Seamless texture tiling and atlasing
     * - Multi-resolution texture generation
     * - Texture post-processing and filtering
     * - Memory-efficient texture caching
     * - Cross-platform texture format support
     * - Performance monitoring and optimization
     * - Integration with material system
     *
     * The texture generator is designed to create infinite variety of textures
     * for blocks, terrain, entities, and UI elements while maintaining
     * high performance and low memory usage.
     */
    class ProceduralTextureGenerator {
    public:
        /**
         * @brief Constructor
         * @param assetGenerator Parent asset generator instance
         */
        explicit ProceduralTextureGenerator(ProceduralAssetGenerator* assetGenerator);

        /**
         * @brief Destructor
         */
        ~ProceduralTextureGenerator();

        /**
         * @brief Deleted copy constructor
         */
        ProceduralTextureGenerator(const ProceduralTextureGenerator&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        ProceduralTextureGenerator& operator=(const ProceduralTextureGenerator&) = delete;

        // Texture generation lifecycle

        /**
         * @brief Initialize texture generator
         * @return true if successful, false otherwise
         */
        bool Initialize();

        /**
         * @brief Shutdown texture generator
         */
        void Shutdown();

        /**
         * @brief Update texture generator
         * @param deltaTime Time elapsed since last update
         */
        void Update(double deltaTime);

        /**
         * @brief Get texture generation statistics
         * @return Current statistics
         */
        const TextureGenerationStats& GetStats() const { return m_stats; }

        // Texture generation methods

        /**
         * @brief Generate texture
         * @param params Generation parameters
         * @return Generated texture or nullopt if failed
         */
        std::optional<GeneratedTexture> GenerateTexture(const TextureGenerationParams& params);

        /**
         * @brief Generate texture asynchronously
         * @param params Generation parameters
         * @return Future to the generated texture
         */
        std::future<std::optional<GeneratedTexture>> GenerateTextureAsync(const TextureGenerationParams& params);

        /**
         * @brief Generate texture with progress callback
         * @param params Generation parameters
         * @param progressCallback Progress callback function
         * @return Generated texture or nullopt if failed
         */
        std::optional<GeneratedTexture> GenerateTextureWithProgress(
            const TextureGenerationParams& params,
            std::function<void(float, const std::string&)> progressCallback);

        // Specific texture generation methods

        /**
         * @brief Generate noise-based texture
         * @param params Generation parameters
         * @return Generated texture
         */
        GeneratedTexture GenerateNoiseTexture(const TextureGenerationParams& params);

        /**
         * @brief Generate fractal texture
         * @param params Generation parameters
         * @return Generated texture
         */
        GeneratedTexture GenerateFractalTexture(const TextureGenerationParams& params);

        /**
         * @brief Generate cellular automata texture
         * @param params Generation parameters
         * @return Generated texture
         */
        GeneratedTexture GenerateCellularTexture(const TextureGenerationParams& params);

        /**
         * @brief Generate marble texture
         * @param params Generation parameters
         * @return Generated texture
         */
        GeneratedTexture GenerateMarbleTexture(const TextureGenerationParams& params);

        /**
         * @brief Generate wood texture
         * @param params Generation parameters
         * @return Generated texture
         */
        GeneratedTexture GenerateWoodTexture(const TextureGenerationParams& params);

        /**
         * @brief Generate cloud texture
         * @param params Generation parameters
         * @return Generated texture
         */
        GeneratedTexture GenerateCloudTexture(const TextureGenerationParams& params);

        /**
         * @brief Generate fire texture
         * @param params Generation parameters
         * @return Generated texture
         */
        GeneratedTexture GenerateFireTexture(const TextureGenerationParams& params);

        /**
         * @brief Generate water texture
         * @param params Generation parameters
         * @return Generated texture
         */
        GeneratedTexture GenerateWaterTexture(const TextureGenerationParams& params);

        /**
         * @brief Generate crystal texture
         * @param params Generation parameters
         * @return Generated texture
         */
        GeneratedTexture GenerateCrystalTexture(const TextureGenerationParams& params);

        // Texture modification and processing

        /**
         * @brief Apply post-processing effects
         * @param texture Texture to process
         * @param effects List of effects to apply
         * @return Processed texture
         */
        GeneratedTexture ApplyPostProcessing(GeneratedTexture texture, const std::vector<std::string>& effects);

        /**
         * @brief Make texture tileable
         * @param texture Texture to make tileable
         * @return Tileable texture
         */
        GeneratedTexture MakeTileable(GeneratedTexture texture);

        /**
         * @brief Resize texture
         * @param texture Texture to resize
         * @param newWidth New width
         * @param newHeight New height
         * @return Resized texture
         */
        GeneratedTexture ResizeTexture(GeneratedTexture texture, int newWidth, int newHeight);

        /**
         * @brief Apply color palette to texture
         * @param texture Texture to colorize
         * @param palette Color palette to apply
         * @return Colorized texture
         */
        GeneratedTexture ApplyColorPalette(GeneratedTexture texture, ColorPalette palette);

        /**
         * @brief Blend two textures
         * @param texture1 First texture
         * @param texture2 Second texture
         * @param blendFactor Blend factor (0.0 - 1.0)
         * @return Blended texture
         */
        GeneratedTexture BlendTextures(GeneratedTexture texture1, GeneratedTexture texture2, float blendFactor);

        // Utility functions

        /**
         * @brief Get supported texture patterns
         * @return Vector of supported patterns
         */
        std::vector<TexturePattern> GetSupportedPatterns() const;

        /**
         * @brief Get supported color palettes
         * @return Vector of supported palettes
         */
        std::vector<ColorPalette> GetSupportedPalettes() const;

        /**
         * @brief Get pattern description
         * @param pattern Pattern to describe
         * @return Pattern description
         */
        std::string GetPatternDescription(TexturePattern pattern) const;

        /**
         * @brief Get palette description
         * @param palette Palette to describe
         * @return Palette description
         */
        std::string GetPaletteDescription(ColorPalette palette) const;

        /**
         * @brief Calculate texture checksum
         * @param texture Texture to checksum
         * @return Checksum string
         */
        std::string CalculateTextureChecksum(const GeneratedTexture& texture) const;

        /**
         * @brief Validate texture data
         * @param texture Texture to validate
         * @return true if valid, false otherwise
         */
        bool ValidateTexture(const GeneratedTexture& texture) const;

        /**
         * @brief Get texture memory usage
         * @param texture Texture to measure
         * @return Memory usage in bytes
         */
        size_t GetTextureMemoryUsage(const GeneratedTexture& texture) const;

        // Preset texture generation

        /**
         * @brief Generate grass texture
         * @param width Texture width
         * @param height Texture height
         * @param seed Random seed
         * @return Generated grass texture
         */
        GeneratedTexture GenerateGrassTexture(int width = 512, int height = 512, uint32_t seed = 0);

        /**
         * @brief Generate stone texture
         * @param width Texture width
         * @param height Texture height
         * @param seed Random seed
         * @return Generated stone texture
         */
        GeneratedTexture GenerateStoneTexture(int width = 512, int height = 512, uint32_t seed = 0);

        /**
         * @brief Generate dirt texture
         * @param width Texture width
         * @param height Texture height
         * @param seed Random seed
         * @return Generated dirt texture
         */
        GeneratedTexture GenerateDirtTexture(int width = 512, int height = 512, uint32_t seed = 0);

        /**
         * @brief Generate sand texture
         * @param width Texture width
         * @param height Texture height
         * @param seed Random seed
         * @return Generated sand texture
         */
        GeneratedTexture GenerateSandTexture(int width = 512, int height = 512, uint32_t seed = 0);

        /**
         * @brief Generate water texture
         * @param width Texture width
         * @param height Texture height
         * @param seed Random seed
         * @return Generated water texture
         */
        GeneratedTexture GenerateWaterSurfaceTexture(int width = 512, int height = 512, uint32_t seed = 0);

        /**
         * @brief Generate lava texture
         * @param width Texture width
         * @param height Texture height
         * @param seed Random seed
         * @return Generated lava texture
         */
        GeneratedTexture GenerateLavaTexture(int width = 512, int height = 512, uint32_t seed = 0);

        /**
         * @brief Generate snow texture
         * @param width Texture width
         * @param height Texture height
         * @param seed Random seed
         * @return Generated snow texture
         */
        GeneratedTexture GenerateSnowTexture(int width = 512, int height = 512, uint32_t seed = 0);

        /**
         * @brief Generate leaf texture
         * @param width Texture width
         * @param height Texture height
         * @param seed Random seed
         * @return Generated leaf texture
         */
        GeneratedTexture GenerateLeafTexture(int width = 512, int height = 512, uint32_t seed = 0);

        /**
         * @brief Generate bark texture
         * @param width Texture width
         * @param height Texture height
         * @param seed Random seed
         * @return Generated bark texture
         */
        GeneratedTexture GenerateBarkTexture(int width = 512, int height = 512, uint32_t seed = 0);

        /**
         * @brief Validate texture generator state
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
         * @brief Optimize texture generator
         * @return Optimization results
         */
        std::unordered_map<std::string, size_t> Optimize();

    private:
        /**
         * @brief Initialize noise generators
         * @return true if successful, false otherwise
         */
        bool InitializeNoiseGenerators();

        /**
         * @brief Initialize color palettes
         */
        void InitializeColorPalettes();

        /**
         * @brief Generate noise value
         * @param x X coordinate
         * @param y Y coordinate
         * @param z Z coordinate (optional)
         * @param params Generation parameters
         * @return Noise value (0.0 - 1.0)
         */
        float GenerateNoise(float x, float y, float z = 0.0f, const TextureGenerationParams& params = TextureGenerationParams()) const;

        /**
         * @brief Generate fractal noise
         * @param x X coordinate
         * @param y Y coordinate
         * @param z Z coordinate (optional)
         * @param params Generation parameters
         * @return Fractal noise value (0.0 - 1.0)
         */
        float GenerateFractalNoise(float x, float y, float z = 0.0f, const TextureGenerationParams& params = TextureGenerationParams()) const;

        /**
         * @brief Apply color palette to grayscale value
         * @param grayscale Grayscale value (0.0 - 1.0)
         * @param palette Color palette to use
         * @return RGBA color
         */
        glm::vec4 ApplyColorPalette(float grayscale, ColorPalette palette) const;

        /**
         * @brief Generate color from palette
         * @param t Interpolation factor (0.0 - 1.0)
         * @param palette Color palette
         * @return RGBA color
         */
        glm::vec4 GeneratePaletteColor(float t, ColorPalette palette) const;

        /**
         * @brief Apply post-processing effect
         * @param data Texture data
         * @param width Texture width
         * @param height Texture height
         * @param effect Effect to apply
         * @return Processed texture data
         */
        std::vector<uint8_t> ApplyEffect(std::vector<uint8_t> data, int width, int height, const std::string& effect);

        /**
         * @brief Blur texture data
         * @param data Texture data
         * @param width Texture width
         * @param height Texture height
         * @param radius Blur radius
         * @return Blurred texture data
         */
        std::vector<uint8_t> ApplyBlur(std::vector<uint8_t> data, int width, int height, float radius);

        /**
         * @brief Sharpen texture data
         * @param data Texture data
         * @param width Texture width
         * @param height Texture height
         * @param amount Sharpen amount
         * @return Sharpened texture data
         */
        std::vector<uint8_t> ApplySharpen(std::vector<uint8_t> data, int width, int height, float amount);

        /**
         * @brief Update performance statistics
         * @param generationTime Generation time in milliseconds
         * @param textureSize Texture size in bytes
         */
        void UpdateStats(double generationTime, size_t textureSize);

        /**
         * @brief Handle texture generation errors
         * @param error Error message
         */
        void HandleError(const std::string& error);

        // Texture generator data
        ProceduralAssetGenerator* m_assetGenerator;  ///< Parent asset generator
        TextureGenerationStats m_stats;              ///< Performance statistics

        // Color palettes
        std::unordered_map<ColorPalette, std::vector<glm::vec4>> m_colorPalettes; ///< Color palette definitions

        // Noise generators
        std::mt19937 m_randomEngine;                 ///< Random number generator
        mutable std::shared_mutex m_generatorMutex;  ///< Generator synchronization

        // State tracking
        bool m_isInitialized;                        ///< Generator is initialized
        double m_lastUpdateTime;                     ///< Last update time
        std::string m_lastError;                     ///< Last error message

        static std::atomic<uint32_t> s_nextTextureId; ///< Next texture ID counter
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_PROCEDURAL_PROCEDURAL_TEXTURE_GENERATOR_HPP
