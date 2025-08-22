/**
 * @file NoiseGenerator.hpp
 * @brief VoxelCraft Advanced Noise Generation System
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines multiple noise generation algorithms for procedural content
 * creation including Perlin, Simplex, Worley, Value, and custom noise functions.
 */

#ifndef VOXELCRAFT_WORLD_NOISE_GENERATOR_HPP
#define VOXELCRAFT_WORLD_NOISE_GENERATOR_HPP

#include <memory>
#include <vector>
#include <array>
#include <random>
#include <functional>
#include <unordered_map>
#include <mutex>
#include <shared_mutex>

#include "../core/Config.hpp"

namespace VoxelCraft {

    /**
     * @enum NoiseType
     * @brief Type of noise generation algorithm
     */
    enum class NoiseType {
        Perlin,         ///< Classic Perlin noise
        Simplex,        ///< Improved Simplex noise
        Value,          ///< Value noise
        Worley,         ///< Worley/Voronoi noise
        OpenSimplex,    ///< OpenSimplex noise
        Gradient,       ///< Gradient noise
        White,          ///< White noise (random)
        Fractal,        ///< Fractal noise (multiple octaves)
        Ridged,         ///< Ridged multifractal noise
        Billow,         ///< Billow noise
        Hybrid          ///< Hybrid noise algorithm
    };

    /**
     * @enum NoiseQuality
     * @brief Quality level for noise generation
     */
    enum class NoiseQuality {
        Low,            ///< Low quality, fast generation
        Medium,         ///< Medium quality, balanced performance
        High,           ///< High quality, slower generation
        Ultra           ///< Ultra quality, slowest generation
    };

    /**
     * @struct NoiseConfig
     * @brief Configuration for noise generation
     */
    struct NoiseConfig {
        NoiseType type;                     ///< Noise algorithm type
        int seed;                           ///< Random seed
        float frequency;                    ///< Base frequency
        float amplitude;                    ///< Base amplitude
        int octaves;                        ///< Number of octaves
        float persistence;                  ///< Persistence factor
        float lacunarity;                   ///< Lacunarity factor
        float scale;                        ///< Scale factor
        NoiseQuality quality;               ///< Generation quality

        // Fractal-specific settings
        float fractalBounding;              ///< Fractal bounding factor
        float gain;                         ///< Gain factor for ridged noise
        float weightedStrength;             ///< Weighted strength for hybrid noise
    };

    /**
     * @struct NoiseMetrics
     * @brief Performance metrics for noise generation
     */
    struct NoiseMetrics {
        uint64_t totalCalls;                ///< Total noise function calls
        double totalTime;                   ///< Total time spent generating noise
        double averageTime;                 ///< Average time per call
        double maxTime;                     ///< Maximum time for single call
        double minTime;                     ///< Minimum time for single call
        uint64_t cacheHits;                 ///< Number of cache hits
        uint64_t cacheMisses;               ///< Number of cache misses
        double cacheHitRate;                ///< Cache hit rate (0.0 - 1.0)
    };

    /**
     * @class NoiseGenerator
     * @brief Advanced noise generation system
     *
     * Supports multiple noise algorithms:
     * - Perlin Noise: Classic gradient noise
     * - Simplex Noise: Improved performance over Perlin
     * - Value Noise: Simple interpolation noise
     * - Worley Noise: Cellular/Voronoi noise
     * - Fractal Noise: Multi-octave noise for complex patterns
     * - Ridged Noise: Terrain-like ridged multifractal
     * - Hybrid algorithms combining multiple techniques
     *
     * Features:
     * - Configurable parameters (frequency, amplitude, octaves)
     * - Multiple quality levels
     * - Caching system for performance
     * - Thread-safe operations
     * - 2D and 3D noise generation
     */
    class NoiseGenerator {
    public:
        /**
         * @brief Constructor
         * @param config Noise configuration
         */
        explicit NoiseGenerator(const NoiseConfig& config);

        /**
         * @brief Destructor
         */
        ~NoiseGenerator();

        /**
         * @brief Deleted copy constructor
         */
        NoiseGenerator(const NoiseGenerator&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        NoiseGenerator& operator=(const NoiseGenerator&) = delete;

        // 2D Noise Functions

        /**
         * @brief Generate 2D noise value
         * @param x X coordinate
         * @param y Y coordinate
         * @return Noise value (-1.0 to 1.0)
         */
        float GetNoise(float x, float y);

        /**
         * @brief Generate 2D noise value at integer coordinates
         * @param x X coordinate
         * @param y Y coordinate
         * @return Noise value (-1.0 to 1.0)
         */
        float GetNoise(int x, int y);

        /**
         * @brief Generate 2D fractal noise
         * @param x X coordinate
         * @param y Y coordinate
         * @return Fractal noise value (-1.0 to 1.0)
         */
        float GetFractal(float x, float y);

        // 3D Noise Functions

        /**
         * @brief Generate 3D noise value
         * @param x X coordinate
         * @param y Y coordinate
         * @param z Z coordinate
         * @return Noise value (-1.0 to 1.0)
         */
        float GetNoise(float x, float y, float z);

        /**
         * @brief Generate 3D noise value at integer coordinates
         * @param x X coordinate
         * @param y Y coordinate
         * @param z Z coordinate
         * @return Noise value (-1.0 to 1.0)
         */
        float GetNoise(int x, int y, int z);

        /**
         * @brief Generate 3D fractal noise
         * @param x X coordinate
         * @param y Y coordinate
         * @param z Z coordinate
         * @return Fractal noise value (-1.0 to 1.0)
         */
        float GetFractal(float x, float y, float z);

        // Specialized Noise Functions

        /**
         * @brief Generate terrain-like ridged noise
         * @param x X coordinate
         * @param y Y coordinate
         * @return Ridged noise value (0.0 to 1.0)
         */
        float GetRidged(float x, float y);

        /**
         * @brief Generate billow noise (cloud-like)
         * @param x X coordinate
         * @param y Y coordinate
         * @return Billow noise value (-1.0 to 1.0)
         */
        float GetBillow(float x, float y);

        /**
         * @brief Generate cellular/Worley noise
         * @param x X coordinate
         * @param y Y coordinate
         * @return Cellular noise value (0.0 to 1.0)
         */
        float GetCellular(float x, float y);

        /**
         * @brief Generate hybrid noise
         * @param x X coordinate
         * @param y Y coordinate
         * @return Hybrid noise value (-1.0 to 1.0)
         */
        float GetHybrid(float x, float y);

        // Configuration

        /**
         * @brief Get noise configuration
         * @return Current configuration
         */
        const NoiseConfig& GetConfig() const { return m_config; }

        /**
         * @brief Set noise configuration
         * @param config New configuration
         */
        void SetConfig(const NoiseConfig& config);

        /**
         * @brief Get noise type
         * @return Noise algorithm type
         */
        NoiseType GetType() const { return m_config.type; }

        /**
         * @brief Get noise metrics
         * @return Performance metrics
         */
        const NoiseMetrics& GetMetrics() const { return m_metrics; }

        /**
         * @brief Reset metrics
         */
        void ResetMetrics();

        // Caching system

        /**
         * @brief Enable/disable caching
         * @param enabled Enable state
         */
        void SetCachingEnabled(bool enabled);

        /**
         * @brief Check if caching is enabled
         * @return true if enabled, false otherwise
         */
        bool IsCachingEnabled() const { return m_cachingEnabled; }

        /**
         * @brief Clear noise cache
         */
        void ClearCache();

        /**
         * @brief Get cache size
         * @return Number of cached values
         */
        size_t GetCacheSize() const;

        /**
         * @brief Get cache hit rate
         * @return Cache hit rate (0.0 - 1.0)
         */
        double GetCacheHitRate() const;

    private:
        /**
         * @brief Initialize noise generator
         */
        void Initialize();

        /**
         * @brief Generate permutation table for noise
         */
        void GeneratePermutationTable();

        /**
         * @brief Update metrics after noise generation
         * @param time Time taken for generation
         * @param cacheHit Whether result was cached
         */
        void UpdateMetrics(double time, bool cacheHit);

        // Core noise functions
        float Perlin2D(float x, float y);
        float Simplex2D(float x, float y);
        float Value2D(float x, float y);
        float Worley2D(float x, float y);
        float OpenSimplex2D(float x, float y);

        float Perlin3D(float x, float y, float z);
        float Simplex3D(float x, float y, float z);
        float Value3D(float x, float y, float z);
        float Worley3D(float x, float y, float z);
        float OpenSimplex3D(float x, float y, float z);

        // Fractal noise functions
        float Fractal2D(float x, float y);
        float Fractal3D(float x, float y, float z);
        float RidgedFractal2D(float x, float y);
        float BillowFractal2D(float x, float y);
        float HybridFractal2D(float x, float y);

        // Helper functions
        float Fade(float t) const;
        float Lerp(float a, float b, float t) const;
        float Grad(int hash, float x, float y, float z) const;
        float Grad(int hash, float x, float y) const;
        int FastFloor(float x) const;

        // Configuration
        NoiseConfig m_config;

        // Permutation tables for noise generation
        std::array<int, 512> m_permutationTable;
        std::array<int, 256> m_gradientTable;

        // Random number generation
        std::mt19937 m_randomEngine;
        std::uniform_int_distribution<int> m_randomInt;

        // Caching system
        bool m_cachingEnabled;
        std::unordered_map<uint64_t, float> m_cache2D;
        std::unordered_map<uint64_t, float> m_cache3D;
        mutable std::shared_mutex m_cacheMutex;
        size_t m_maxCacheSize;

        // Metrics
        NoiseMetrics m_metrics;
        mutable std::shared_mutex m_metricsMutex;

        // Cache key generation
        uint64_t MakeCacheKey2D(int x, int y) const;
        uint64_t MakeCacheKey2D(float x, float y) const;
        uint64_t MakeCacheKey3D(int x, int y, int z) const;
        uint64_t MakeCacheKey3D(float x, float y, float z) const;
    };

    /**
     * @class NoiseGeneratorFactory
     * @brief Factory for creating noise generators
     */
    class NoiseGeneratorFactory {
    public:
        /**
         * @brief Create noise generator
         * @param config Noise configuration
         * @return Noise generator instance
         */
        static std::unique_ptr<NoiseGenerator> CreateGenerator(const NoiseConfig& config);

        /**
         * @brief Create Perlin noise generator
         * @param seed Random seed
         * @param frequency Base frequency
         * @param amplitude Base amplitude
         * @return Perlin noise generator
         */
        static std::unique_ptr<NoiseGenerator> CreatePerlinGenerator(
            int seed, float frequency = 0.01f, float amplitude = 1.0f);

        /**
         * @brief Create Simplex noise generator
         * @param seed Random seed
         * @param frequency Base frequency
         * @param amplitude Base amplitude
         * @return Simplex noise generator
         */
        static std::unique_ptr<NoiseGenerator> CreateSimplexGenerator(
            int seed, float frequency = 0.01f, float amplitude = 1.0f);

        /**
         * @brief Create fractal noise generator
         * @param seed Random seed
         * @param octaves Number of octaves
         * @param persistence Persistence factor
         * @param lacunarity Lacunarity factor
         * @return Fractal noise generator
         */
        static std::unique_ptr<NoiseGenerator> CreateFractalGenerator(
            int seed, int octaves = 6, float persistence = 0.5f, float lacunarity = 2.0f);

        /**
         * @brief Create terrain noise generator
         * @param seed Random seed
         * @return Terrain-optimized noise generator
         */
        static std::unique_ptr<NoiseGenerator> CreateTerrainGenerator(int seed);

        /**
         * @brief Create cave noise generator
         * @param seed Random seed
         * @return Cave-optimized noise generator
         */
        static std::unique_ptr<NoiseGenerator> CreateCaveGenerator(int seed);

        /**
         * @brief Get default configuration for noise type
         * @param type Noise algorithm type
         * @param seed Random seed
         * @return Default configuration
         */
        static NoiseConfig GetDefaultConfig(NoiseType type, int seed);
    };

    /**
     * @class NoiseUtils
     * @brief Utility functions for noise generation
     */
    class NoiseUtils {
    public:
        /**
         * @brief Normalize noise value to 0.0 - 1.0 range
         * @param noise Input noise value
         * @return Normalized value
         */
        static float Normalize(float noise);

        /**
         * @brief Clamp noise value to range
         * @param noise Input noise value
         * @param min Minimum value
         * @param max Maximum value
         * @return Clamped value
         */
        static float Clamp(float noise, float min, float max);

        /**
         * @brief Apply curve to noise value
         * @param noise Input noise value
         * @param curve Curve function
         * @return Curved noise value
         */
        static float ApplyCurve(float noise, std::function<float(float)> curve);

        /**
         * @brief Combine two noise values
         * @param noise1 First noise value
         * @param noise2 Second noise value
         * @param operation Combination operation
         * @return Combined noise value
         */
        static float Combine(float noise1, float noise2, std::function<float(float, float)> operation);

        /**
         * @brief Generate seed from string
         * @param seedString Seed string
         * @return Numeric seed
         */
        static int SeedFromString(const std::string& seedString);

        /**
         * @brief Generate random seed
         * @return Random seed
         */
        static int GenerateRandomSeed();

        /**
         * @brief Mix multiple noise values
         * @param noises Vector of noise values
         * @param weights Vector of weights
         * @return Mixed noise value
         */
        static float MixNoises(const std::vector<float>& noises, const std::vector<float>& weights);

        /**
         * @brief Apply turbulence to noise
         * @param noise Input noise value
         * @param turbulence Turbulence factor
         * @return Turbulent noise value
         */
        static float ApplyTurbulence(float noise, float turbulence);

        /**
         * @brief Generate noise gradient
         * @param noiseGenerator Noise generator
         * @param x X coordinate
         * @param y Y coordinate
         * @param radius Gradient radius
         * @return Gradient vector (x, y)
         */
        static std::pair<float, float> GenerateGradient(
            NoiseGenerator* noiseGenerator, float x, float y, float radius);
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_WORLD_NOISE_GENERATOR_HPP
