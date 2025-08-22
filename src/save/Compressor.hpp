/**
 * @file Compressor.hpp
 * @brief VoxelCraft Advanced Data Compression System
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines the Compressor class that provides comprehensive data
 * compression for the VoxelCraft game engine, including multiple compression
 * algorithms, adaptive compression, hybrid compression strategies, and
 * performance optimization for save files and network transmission.
 */

#ifndef VOXELCRAFT_SAVE_COMPRESSOR_HPP
#define VOXELCRAFT_SAVE_COMPRESSOR_HPP

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
#include <deque>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../core/Config.hpp"

namespace VoxelCraft {

    // Forward declarations
    class SaveManager;

    /**
     * @enum CompressionLevel
     * @brief Compression level presets
     */
    enum class CompressionLevel {
        Fastest,                ///< Fastest compression (larger files)
        Fast,                   ///< Fast compression
        Balanced,               ///< Balanced speed/size ratio
        Good,                   ///< Good compression
        Best,                   ///< Best compression (slowest)
        Custom                  ///< Custom compression settings
    };

    /**
     * @enum CompressionStrategy
     * @brief Compression strategy types
     */
    enum class CompressionStrategy {
        SingleAlgorithm,        ///< Use single compression algorithm
        Hybrid,                 ///< Use hybrid compression strategies
        Adaptive,               ///< Adaptive compression based on data
        MultiPass,              ///< Multi-pass compression
        DictionaryBased,        ///< Dictionary-based compression
        Custom                  ///< Custom compression strategy
    };

    /**
     * @enum DataTypeHint
     * @brief Data type hints for compression optimization
     */
    enum class DataTypeHint {
        Unknown,                ///< Unknown data type
        Text,                   ///< Text data
        Binary,                 ///< Binary data
        Image,                  ///< Image data
        Audio,                  ///< Audio data
        Mesh,                   ///< 3D mesh data
        Terrain,                ///< Terrain/heightmap data
        Vector,                 ///< Vector/mathematical data
        Repetitive,             ///< Repetitive data (good for RLE)
        Sparse,                 ///< Sparse data (many zeros)
        Custom                  ///< Custom data type
    };

    /**
     * @struct CompressionConfig
     * @brief Compression configuration settings
     */
    struct CompressionConfig {
        CompressionLevel level;                     ///< Compression level
        CompressionStrategy strategy;               ///< Compression strategy
        int quality;                                ///< Compression quality (0-100)
        bool enableMultiThreading;                  ///< Enable multi-threading
        int threadCount;                            ///< Number of threads
        size_t blockSize;                           ///< Compression block size
        bool enableAdaptive;                        ///< Enable adaptive compression
        bool enablePreprocessing;                   ///< Enable data preprocessing

        // Algorithm-specific settings
        bool enableLZ4;                             ///< Enable LZ4 compression
        bool enableZstandard;                       ///< Enable Zstandard compression
        bool enableBrotli;                          ///< Enable Brotli compression
        bool enableRunLength;                       ///< Enable run-length encoding
        bool enableDelta;                           ///< Enable delta encoding
        bool enableQuantization;                    ///< Enable quantization

        // Performance settings
        size_t maxMemoryUsage;                      ///< Maximum memory usage (MB)
        float maxCompressionTime;                   ///< Maximum compression time (seconds)
        bool enableProgressCallback;                ///< Enable progress callbacks
        bool enableVerification;                    ///< Enable compression verification

        CompressionConfig()
            : level(CompressionLevel::Balanced)
            , strategy(CompressionStrategy::Adaptive)
            , quality(75)
            , enableMultiThreading(true)
            , threadCount(4)
            , blockSize(65536)
            , enableAdaptive(true)
            , enablePreprocessing(true)
            , enableLZ4(true)
            , enableZstandard(true)
            , enableBrotli(false)
            , enableRunLength(true)
            , enableDelta(true)
            , enableQuantization(true)
            , maxMemoryUsage(512)
            , maxCompressionTime(30.0f)
            , enableProgressCallback(true)
            , enableVerification(true)
        {}
    };

    /**
     * @struct CompressionResult
     * @brief Result of compression operation
     */
    struct CompressionResult {
        bool success;                               ///< Operation success
        std::vector<uint8_t> compressedData;        ///< Compressed data
        size_t originalSize;                        ///< Original data size
        size_t compressedSize;                      ///< Compressed data size
        float compressionRatio;                     ///< Compression ratio
        double compressionTime;                     ///< Compression time (ms)
        double decompressionTime;                   ///< Decompression time (ms)
        std::string algorithmUsed;                  ///< Algorithm used
        std::string errorMessage;                   ///< Error message if failed

        CompressionResult()
            : success(false)
            , originalSize(0)
            , compressedSize(0)
            , compressionRatio(1.0f)
            , compressionTime(0.0)
            , decompressionTime(0.0)
        {}
    };

    /**
     * @struct CompressionStats
     * @brief Compression performance statistics
     */
    struct CompressionStats {
        // Operation statistics
        uint64_t totalCompressions;                 ///< Total compression operations
        uint64_t totalDecompressions;               ///< Total decompression operations
        uint64_t successfulCompressions;            ///< Successful compressions
        uint64_t successfulDecompressions;          ///< Successful decompressions
        uint64_t failedCompressions;                ///< Failed compressions
        uint64_t failedDecompressions;              ///< Failed decompressions

        // Performance metrics
        double averageCompressionTime;              ///< Average compression time (ms)
        double averageDecompressionTime;            ///< Average decompression time (ms)
        double minCompressionTime;                  ///< Minimum compression time (ms)
        double maxCompressionTime;                  ///< Maximum compression time (ms)
        double minDecompressionTime;                ///< Minimum decompression time (ms)
        double maxDecompressionTime;                ///< Maximum decompression time (ms)

        // Size metrics
        uint64_t totalOriginalBytes;                ///< Total original bytes processed
        uint64_t totalCompressedBytes;              ///< Total compressed bytes produced
        float averageCompressionRatio;              ///< Average compression ratio
        float bestCompressionRatio;                 ///< Best compression ratio achieved
        float worstCompressionRatio;                ///< Worst compression ratio achieved

        // Algorithm usage
        std::unordered_map<std::string, uint64_t> algorithmUsage; ///< Algorithm usage count
        std::unordered_map<std::string, double> algorithmTime; ///< Algorithm time usage
        std::unordered_map<std::string, float> algorithmRatio; ///< Algorithm compression ratios

        // Memory usage
        size_t currentMemoryUsage;                  ///< Current memory usage
        size_t peakMemoryUsage;                     ///< Peak memory usage
        uint32_t memoryAllocations;                 ///< Number of memory allocations
    };

    /**
     * @struct CompressionProfile
     * @brief Compression profile for specific data types
     */
    struct CompressionProfile {
        std::string profileName;                    ///< Profile name
        DataTypeHint dataType;                      ///< Data type hint
        CompressionLevel level;                     ///< Compression level
        std::vector<std::string> preferredAlgorithms; ///< Preferred algorithms
        std::unordered_map<std::string, int> algorithmSettings; ///< Algorithm settings
        bool enablePreprocessing;                   ///< Enable preprocessing
        std::vector<std::string> preprocessingSteps; ///< Preprocessing steps

        CompressionProfile()
            : dataType(DataTypeHint::Unknown)
            , level(CompressionLevel::Balanced)
            , enablePreprocessing(true)
        {}
    };

    /**
     * @class Compressor
     * @brief Advanced data compression system
     *
     * The Compressor class provides comprehensive data compression for the
     * VoxelCraft game engine, featuring multiple compression algorithms,
     * adaptive compression strategies, hybrid compression approaches, and
     * performance optimization for save files and network transmission with
     * enterprise-grade compression ratios and speed.
     *
     * Key Features:
     * - Multiple compression algorithms (LZ4, Zstd, Brotli)
     * - Adaptive compression based on data type
     * - Hybrid compression strategies
     * - Multi-threaded compression
     * - Data preprocessing and optimization
     * - Compression profiles for different data types
     * - Real-time compression monitoring
     * - Memory usage optimization
     * - Compression verification and validation
     * - Streaming compression support
     * - Performance benchmarking
     *
     * The compressor is designed to achieve optimal compression ratios
     * while maintaining acceptable compression and decompression speeds
     * for game save files and network transmission.
     */
    class Compressor {
    public:
        /**
         * @brief Constructor
         * @param config Compression configuration
         */
        explicit Compressor(const CompressionConfig& config);

        /**
         * @brief Destructor
         */
        ~Compressor();

        /**
         * @brief Deleted copy constructor
         */
        Compressor(const Compressor&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        Compressor& operator=(const Compressor&) = delete;

        // Compressor lifecycle

        /**
         * @brief Initialize compressor
         * @return true if successful, false otherwise
         */
        bool Initialize();

        /**
         * @brief Shutdown compressor
         */
        void Shutdown();

        /**
         * @brief Get compression configuration
         * @return Current configuration
         */
        const CompressionConfig& GetConfig() const { return m_config; }

        /**
         * @brief Set compression configuration
         * @param config New configuration
         * @return true if successful, false otherwise
         */
        bool SetConfig(const CompressionConfig& config);

        /**
         * @brief Get compression statistics
         * @return Current statistics
         */
        const CompressionStats& GetStats() const { return m_stats; }

        // Compression operations

        /**
         * @brief Compress data
         * @param input Input data
         * @param dataTypeHint Data type hint for optimization
         * @return Compression result
         */
        CompressionResult Compress(const std::vector<uint8_t>& input,
                                 DataTypeHint dataTypeHint = DataTypeHint::Unknown);

        /**
         * @brief Decompress data
         * @param input Compressed input data
         * @param expectedSize Expected decompressed size (0 for auto-detect)
         * @return Decompression result
         */
        CompressionResult Decompress(const std::vector<uint8_t>& input,
                                   size_t expectedSize = 0);

        /**
         * @brief Compress data with progress callback
         * @param input Input data
         * @param progressCallback Progress callback function
         * @param dataTypeHint Data type hint
         * @return Compression result
         */
        CompressionResult CompressWithProgress(const std::vector<uint8_t>& input,
                                             std::function<void(float, const std::string&)> progressCallback,
                                             DataTypeHint dataTypeHint = DataTypeHint::Unknown);

        /**
         * @brief Streaming compression
         * @param input Input data chunk
         * @param isFinalChunk Whether this is the final chunk
         * @return Compressed chunk
         */
        std::vector<uint8_t> CompressStream(const std::vector<uint8_t>& input, bool isFinalChunk = false);

        /**
         * @brief Streaming decompression
         * @param input Compressed input chunk
         * @param isFinalChunk Whether this is the final chunk
         * @return Decompressed chunk
         */
        std::vector<uint8_t> DecompressStream(const std::vector<uint8_t>& input, bool isFinalChunk = false);

        // Compression profiles

        /**
         * @brief Create compression profile
         * @param profileName Profile name
         * @param dataType Data type hint
         * @param level Compression level
         * @return Profile ID or 0 if failed
         */
        uint32_t CreateCompressionProfile(const std::string& profileName,
                                        DataTypeHint dataType,
                                        CompressionLevel level = CompressionLevel::Balanced);

        /**
         * @brief Get compression profile
         * @param profileId Profile ID
         * @return Compression profile or nullopt if not found
         */
        std::optional<CompressionProfile> GetCompressionProfile(uint32_t profileId) const;

        /**
         * @brief Set profile algorithm settings
         * @param profileId Profile ID
         * @param algorithm Algorithm name
         * @param setting Setting value
         * @return true if successful, false otherwise
         */
        bool SetProfileAlgorithmSetting(uint32_t profileId, const std::string& algorithm, int setting);

        /**
         * @brief Compress data using specific profile
         * @param input Input data
         * @param profileId Profile ID
         * @return Compression result
         */
        CompressionResult CompressWithProfile(const std::vector<uint8_t>& input, uint32_t profileId);

        // Algorithm-specific compression

        /**
         * @brief Compress with LZ4
         * @param input Input data
         * @param compressionLevel LZ4 compression level
         * @return Compression result
         */
        CompressionResult CompressLZ4(const std::vector<uint8_t>& input, int compressionLevel = 1);

        /**
         * @brief Compress with Zstandard
         * @param input Input data
         * @param compressionLevel Zstd compression level
         * @return Compression result
         */
        CompressionResult CompressZstandard(const std::vector<uint8_t>& input, int compressionLevel = 3);

        /**
         * @brief Compress with Brotli
         * @param input Input data
         * @param quality Brotli quality level
         * @return Compression result
         */
        CompressionResult CompressBrotli(const std::vector<uint8_t>& input, int quality = 6);

        /**
         * @brief Compress with run-length encoding
         * @param input Input data
         * @return Compression result
         */
        CompressionResult CompressRunLength(const std::vector<uint8_t>& input);

        /**
         * @brief Compress with delta encoding
         * @param input Input data
         * @return Compression result
         */
        CompressionResult CompressDelta(const std::vector<uint8_t>& input);

        // Hybrid compression

        /**
         * @brief Compress with hybrid strategy
         * @param input Input data
         * @param strategies Compression strategies to try
         * @return Best compression result
         */
        CompressionResult CompressHybrid(const std::vector<uint8_t>& input,
                                       const std::vector<std::string>& strategies);

        /**
         * @brief Compress with adaptive strategy
         * @param input Input data
         * @param dataTypeHint Data type hint
         * @return Adaptive compression result
         */
        CompressionResult CompressAdaptive(const std::vector<uint8_t>& input, DataTypeHint dataTypeHint);

        // Data preprocessing

        /**
         * @brief Preprocess data before compression
         * @param input Input data
         * @param preprocessingSteps Preprocessing steps
         * @return Preprocessed data
         */
        std::vector<uint8_t> PreprocessData(const std::vector<uint8_t>& input,
                                          const std::vector<std::string>& preprocessingSteps);

        /**
         * @brief Postprocess data after decompression
         * @param input Input data
         * @param preprocessingSteps Preprocessing steps to reverse
         * @return Postprocessed data
         */
        std::vector<uint8_t> PostprocessData(const std::vector<uint8_t>& input,
                                           const std::vector<std::string>& preprocessingSteps);

        // Performance and benchmarking

        /**
         * @brief Benchmark compression algorithms
         * @param testData Test data for benchmarking
         * @param algorithms Algorithms to test
         * @return Benchmark results
         */
        std::unordered_map<std::string, CompressionResult> BenchmarkCompression(
            const std::vector<uint8_t>& testData,
            const std::vector<std::string>& algorithms);

        /**
         * @brief Get recommended algorithm for data
         * @param data Sample data
         * @param dataTypeHint Data type hint
         * @return Recommended algorithm name
         */
        std::string GetRecommendedAlgorithm(const std::vector<uint8_t>& data, DataTypeHint dataTypeHint);

        /**
         * @brief Get algorithm performance rating
         * @param algorithm Algorithm name
         * @return Performance rating (0.0 - 1.0)
         */
        float GetAlgorithmPerformanceRating(const std::string& algorithm) const;

        // Memory management

        /**
         * @brief Set memory usage limit
         * @param limit Memory limit in MB
         * @return true if successful, false otherwise
         */
        bool SetMemoryLimit(size_t limit);

        /**
         * @brief Get current memory usage
         * @return Memory usage in bytes
         */
        size_t GetCurrentMemoryUsage() const;

        /**
         * @brief Clear memory caches
         * @return Freed memory in bytes
         */
        size_t ClearMemoryCaches();

        // Utility functions

        /**
         * @brief Get available compression algorithms
         * @return Vector of algorithm names
         */
        std::vector<std::string> GetAvailableAlgorithms() const;

        /**
         * @brief Check if algorithm is available
         * @param algorithm Algorithm name
         * @return true if available, false otherwise
         */
        bool IsAlgorithmAvailable(const std::string& algorithm) const;

        /**
         * @brief Get algorithm description
         * @param algorithm Algorithm name
         * @return Algorithm description
         */
        std::string GetAlgorithmDescription(const std::string& algorithm) const;

        /**
         * @brief Calculate compression ratio
         * @param originalSize Original data size
         * @param compressedSize Compressed data size
         * @return Compression ratio
         */
        static float CalculateCompressionRatio(size_t originalSize, size_t compressedSize);

        /**
         * @brief Validate compressed data
         * @param originalData Original data
         * @param compressedData Compressed data
         * @param algorithm Algorithm used
         * @return true if validation passes, false otherwise
         */
        bool ValidateCompression(const std::vector<uint8_t>& originalData,
                               const std::vector<uint8_t>& compressedData,
                               const std::string& algorithm);

        /**
         * @brief Validate compressor state
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
         * @brief Optimize compressor
         * @return Optimization results
         */
        std::unordered_map<std::string, size_t> Optimize();

    private:
        /**
         * @brief Initialize compression algorithms
         * @return true if successful, false otherwise
         */
        bool InitializeAlgorithms();

        /**
         * @brief Initialize compression profiles
         */
        void InitializeDefaultProfiles();

        /**
         * @brief Select optimal compression algorithm
         * @param data Sample data
         * @param dataTypeHint Data type hint
         * @return Selected algorithm name
         */
        std::string SelectOptimalAlgorithm(const std::vector<uint8_t>& data, DataTypeHint dataTypeHint);

        /**
         * @brief Apply data preprocessing
         * @param data Input data
         * @param preprocessingSteps Preprocessing steps
         * @return Preprocessed data
         */
        std::vector<uint8_t> ApplyPreprocessing(const std::vector<uint8_t>& data,
                                              const std::vector<std::string>& preprocessingSteps);

        /**
         * @brief Apply data postprocessing
         * @param data Input data
         * @param preprocessingSteps Preprocessing steps to reverse
         * @return Postprocessed data
         */
        std::vector<uint8_t> ApplyPostprocessing(const std::vector<uint8_t>& data,
                                               const std::vector<std::string>& preprocessingSteps);

        /**
         * @brief Compress data using specific algorithm
         * @param input Input data
         * @param algorithm Algorithm name
         * @param level Compression level
         * @return Compression result
         */
        CompressionResult CompressWithAlgorithm(const std::vector<uint8_t>& input,
                                              const std::string& algorithm,
                                              int level = -1);

        /**
         * @brief Decompress data using specific algorithm
         * @param input Compressed input data
         * @param algorithm Algorithm used for compression
         * @param expectedSize Expected decompressed size
         * @return Compression result
         */
        CompressionResult DecompressWithAlgorithm(const std::vector<uint8_t>& input,
                                                const std::string& algorithm,
                                                size_t expectedSize = 0);

        /**
         * @brief Update performance statistics
         * @param result Compression result
         * @param operation Operation type ("compress" or "decompress")
         */
        void UpdateStats(const CompressionResult& result, const std::string& operation);

        /**
         * @brief Handle compression errors
         * @param error Error message
         */
        void HandleError(const std::string& error);

        // Compressor data
        CompressionConfig m_config;                 ///< Compression configuration
        CompressionStats m_stats;                   ///< Performance statistics

        // Compression profiles
        std::unordered_map<uint32_t, CompressionProfile> m_profiles; ///< Compression profiles
        mutable std::shared_mutex m_profilesMutex;   ///< Profile synchronization

        // Algorithm availability
        std::unordered_map<std::string, bool> m_availableAlgorithms; ///< Available algorithms
        std::unordered_map<std::string, std::string> m_algorithmDescriptions; ///< Algorithm descriptions

        // Performance tracking
        std::unordered_map<std::string, std::deque<double>> m_algorithmTimes; ///< Algorithm timing history
        std::unordered_map<std::string, std::deque<float>> m_algorithmRatios; ///< Algorithm ratio history

        // Memory management
        size_t m_currentMemoryUsage;                ///< Current memory usage
        size_t m_memoryLimit;                       ///< Memory usage limit

        // State tracking
        bool m_isInitialized;                       ///< Compressor is initialized
        std::string m_lastError;                    ///< Last error message

        static std::atomic<uint32_t> s_nextProfileId; ///< Next profile ID counter
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_SAVE_COMPRESSOR_HPP
