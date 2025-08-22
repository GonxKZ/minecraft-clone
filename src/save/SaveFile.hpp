/**
 * @file SaveFile.hpp
 * @brief VoxelCraft Save File Format System
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines the SaveFile class that provides comprehensive save file
 * format handling for the VoxelCraft game engine, including file format
 * specifications, metadata management, file integrity, compression support,
 * and cross-platform compatibility for game save files.
 */

#ifndef VOXELCRAFT_SAVE_SAVE_FILE_HPP
#define VOXELCRAFT_SAVE_SAVE_FILE_HPP

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
#include <filesystem>
#include <fstream>
#include <array>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../core/Config.hpp"

namespace VoxelCraft {

    // Forward declarations
    class SaveManager;
    class Serializer;
    class Compressor;

    /**
     * @enum SaveFileFormat
     * @brief Save file format types
     */
    enum class SaveFileFormat {
        BinaryV1,               ///< Binary format version 1 (legacy)
        BinaryV2,               ///< Binary format version 2
        BinaryV3,               ///< Binary format version 3 (current)
        CompressedBinaryV1,     ///< Compressed binary format v1
        CompressedBinaryV2,     ///< Compressed binary format v2 (current)
        JSON,                   ///< JSON format (human readable)
        MessagePack,            ///< MessagePack format
        Custom                  ///< Custom format
    };

    /**
     * @enum SaveFileSection
     * @brief Save file section types
     */
    enum class SaveFileSection {
        Header,                 ///< File header
        Metadata,               ///< Metadata section
        WorldData,              ///< World data section
        EntityData,             ///< Entity data section
        PlayerData,             ///< Player data section
        InventoryData,          ///< Inventory data section
        GameStateData,          ///< Game state data section
        CustomData,             ///< Custom data section
        Footer                  ///< File footer
    };

    /**
     * @enum FileIntegrityStatus
     * @brief File integrity check results
     */
    enum class FileIntegrityStatus {
        Valid,                  ///< File is valid
        Invalid,                ///< File is invalid
        Corrupted,              ///< File is corrupted
        Incomplete,             ///< File is incomplete
        WrongVersion,           ///< Wrong file version
        Unknown                 ///< Unknown status
    };

    /**
     * @struct SaveFileHeader
     * @brief Save file header structure
     */
    struct SaveFileHeader {
        // Magic number and version
        std::array<char, 8> magicNumber;           ///< File magic number "VOXELSAV"
        uint32_t formatVersion;                    ///< Format version
        uint32_t saveVersion;                      ///< Save data version

        // File information
        uint64_t fileSize;                         ///< Total file size in bytes
        uint64_t headerSize;                       ///< Header size in bytes
        uint32_t sectionCount;                     ///< Number of sections
        uint32_t flags;                            ///< Format flags

        // Timestamps
        uint64_t creationTime;                     ///< File creation timestamp
        uint64_t modificationTime;                  ///< File modification timestamp
        uint64_t saveTime;                          ///< Game time when saved

        // Data integrity
        std::array<uint8_t, 32> checksum;          ///< File checksum (SHA-256)
        std::array<uint8_t, 16> salt;              ///< Random salt for checksum

        // Game version info
        uint32_t gameMajorVersion;                 ///< Game major version
        uint32_t gameMinorVersion;                 ///< Game minor version
        uint32_t gamePatchVersion;                 ///< Game patch version
        uint32_t gameBuildVersion;                 ///< Game build version

        SaveFileHeader()
            : magicNumber({'V','O','X','E','L','S','A','V'})
            , formatVersion(3)
            , saveVersion(1)
            , fileSize(0)
            , headerSize(sizeof(SaveFileHeader))
            , sectionCount(0)
            , flags(0)
            , creationTime(0)
            , modificationTime(0)
            , saveTime(0)
            , gameMajorVersion(1)
            , gameMinorVersion(0)
            , gamePatchVersion(0)
            , gameBuildVersion(0)
        {
            // Initialize salt with random data
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<uint8_t> dis(0, 255);
            for (auto& byte : salt) {
                byte = dis(gen);
            }
        }
    };

    /**
     * @struct SaveFileSectionHeader
     * @brief Save file section header
     */
    struct SaveFileSectionHeader {
        uint32_t sectionType;                       ///< Section type ID
        uint64_t sectionOffset;                     ///< Offset from file start
        uint64_t sectionSize;                       ///< Section size in bytes
        uint32_t compressionType;                   ///< Compression type
        uint64_t uncompressedSize;                  ///< Uncompressed size
        std::array<uint8_t, 32> sectionChecksum;    ///< Section checksum
        uint32_t flags;                             ///< Section flags

        SaveFileSectionHeader()
            : sectionType(0)
            , sectionOffset(0)
            , sectionSize(0)
            , compressionType(0)
            , uncompressedSize(0)
            , flags(0)
        {}
    };

    /**
     * @struct SaveFileMetadata
     * @brief Save file metadata
     */
    struct SaveFileMetadata {
        // Basic information
        std::string saveName;                       ///< Save file name
        std::string description;                     ///< Save description
        std::string author;                          ///< Save author
        std::string creationDate;                    ///< Creation date
        std::string lastPlayedDate;                  ///< Last played date

        // Game state
        std::string worldName;                       ///< World name
        std::string worldSeed;                       ///< World seed
        std::string gameMode;                        ///< Game mode
        std::string difficulty;                      ///< Difficulty level

        // Player information
        std::string playerName;                      ///< Player name
        uint32_t playerLevel;                        ///< Player level
        uint64_t playTime;                           ///< Total play time (seconds)
        glm::vec3 playerPosition;                    ///< Player position
        std::string playerHealth;                    ///< Player health status

        // World information
        uint64_t worldSize;                          ///< World size in blocks
        uint32_t chunkCount;                         ///< Number of chunks
        uint32_t entityCount;                        ///< Number of entities
        uint32_t itemCount;                          ///< Number of items

        // Technical information
        SaveFileFormat format;                       ///< File format
        uint64_t fileSize;                           ///< File size in bytes
        uint64_t compressedSize;                     ///< Compressed size
        float compressionRatio;                      ///< Compression ratio
        std::string checksum;                        ///< File checksum

        // Screenshots and media
        std::string thumbnailPath;                   ///< Thumbnail image path
        std::vector<std::string> screenshotPaths;    ///< Screenshot paths

        // Custom metadata
        std::unordered_map<std::string, std::string> customFields; ///< Custom fields
        std::vector<std::string> tags;               ///< Save file tags

        SaveFileMetadata()
            : playerLevel(1)
            , playTime(0)
            , playerPosition(0.0f, 0.0f, 0.0f)
            , worldSize(0)
            , chunkCount(0)
            , entityCount(0)
            , itemCount(0)
            , format(SaveFileFormat::CompressedBinaryV2)
            , fileSize(0)
            , compressedSize(0)
            , compressionRatio(1.0f)
        {}
    };

    /**
     * @struct SaveFileStats
     * @brief Save file statistics
     */
    struct SaveFileStats {
        // File operations
        uint64_t filesCreated;                       ///< Files created
        uint64_t filesLoaded;                        ///< Files loaded
        uint64_t filesValidated;                     ///< Files validated
        uint64_t filesRepaired;                      ///< Files repaired

        // Performance metrics
        double averageLoadTime;                      ///< Average load time (ms)
        double averageSaveTime;                      ///< Average save time (ms)
        double minLoadTime;                          ///< Minimum load time (ms)
        double maxLoadTime;                          ///< Maximum load time (ms)

        // Size metrics
        uint64_t totalDataSaved;                     ///< Total data saved (bytes)
        uint64_t totalDataLoaded;                    ///< Total data loaded (bytes)
        uint64_t largestFileSize;                    ///< Largest file size
        uint64_t smallestFileSize;                   ///< Smallest file size

        // Compression statistics
        uint64_t totalCompressedSize;                ///< Total compressed size
        uint64_t totalUncompressedSize;              ///< Total uncompressed size
        float averageCompressionRatio;               ///< Average compression ratio
        float bestCompressionRatio;                  ///< Best compression ratio

        // Integrity statistics
        uint64_t integrityChecksPassed;              ///< Integrity checks passed
        uint64_t integrityChecksFailed;              ///< Integrity checks failed
        uint64_t corruptionDetected;                 ///< Corruption detections
        uint64_t autoRepairs;                        ///< Automatic repairs

        // Format statistics
        std::unordered_map<SaveFileFormat, uint64_t> formatUsage; ///< Format usage
        std::unordered_map<std::string, uint64_t> sectionUsage; ///< Section usage
    };

    /**
     * @struct SaveFileReader
     * @brief Save file reader interface
     */
    class SaveFileReader {
    public:
        virtual ~SaveFileReader() = default;

        /**
         * @brief Open save file for reading
         * @param filePath File path
         * @return true if successful, false otherwise
         */
        virtual bool Open(const std::string& filePath) = 0;

        /**
         * @brief Close save file
         */
        virtual void Close() = 0;

        /**
         * @brief Read file header
         * @return File header
         */
        virtual SaveFileHeader ReadHeader() = 0;

        /**
         * @brief Read metadata
         * @return File metadata
         */
        virtual SaveFileMetadata ReadMetadata() = 0;

        /**
         * @brief Read section data
         * @param sectionType Section type
         * @return Section data
         */
        virtual std::vector<uint8_t> ReadSection(SaveFileSection sectionType) = 0;

        /**
         * @brief Check file integrity
         * @return Integrity status
         */
        virtual FileIntegrityStatus CheckIntegrity() = 0;

        /**
         * @brief Get file size
         * @return File size in bytes
         */
        virtual uint64_t GetFileSize() const = 0;

        /**
         * @brief Is file open
         * @return true if open, false otherwise
         */
        virtual bool IsOpen() const = 0;
    };

    /**
     * @struct SaveFileWriter
     * @brief Save file writer interface
     */
    class SaveFileWriter {
    public:
        virtual ~SaveFileWriter() = default;

        /**
         * @brief Create new save file
         * @param filePath File path
         * @param format File format
         * @return true if successful, false otherwise
         */
        virtual bool Create(const std::string& filePath, SaveFileFormat format) = 0;

        /**
         * @brief Close save file
         */
        virtual void Close() = 0;

        /**
         * @brief Write file header
         * @param header File header
         * @return true if successful, false otherwise
         */
        virtual bool WriteHeader(const SaveFileHeader& header) = 0;

        /**
         * @brief Write metadata
         * @param metadata File metadata
         * @return true if successful, false otherwise
         */
        virtual bool WriteMetadata(const SaveFileMetadata& metadata) = 0;

        /**
         * @brief Write section data
         * @param sectionType Section type
         * @param data Section data
         * @return true if successful, false otherwise
         */
        virtual bool WriteSection(SaveFileSection sectionType, const std::vector<uint8_t>& data) = 0;

        /**
         * @brief Finalize file
         * @return true if successful, false otherwise
         */
        virtual bool Finalize() = 0;

        /**
         * @brief Is file open
         * @return true if open, false otherwise
         */
        virtual bool IsOpen() const = 0;
    };

    /**
     * @class SaveFile
     * @brief Advanced save file format system
     *
     * The SaveFile class provides comprehensive save file format handling for the
     * VoxelCraft game engine, including file format specifications, metadata
     * management, file integrity, compression support, and cross-platform
     * compatibility for game save files with enterprise-grade file handling
     * and data integrity features.
     *
     * Key Features:
     * - Multiple file formats (Binary, Compressed, JSON, MessagePack)
     * - Advanced file integrity checking and validation
     * - Metadata management with rich game state information
     * - Section-based file organization for efficient loading
     * - Compression support for all file formats
     * - Cross-platform compatibility and endianness handling
     * - File corruption detection and automatic repair
     * - Streaming support for large save files
     * - Backup integration and version management
     * - Performance monitoring and optimization
     * - Memory-mapped file I/O for large files
     *
     * The save file system ensures data integrity while providing
     * optimal performance for loading and saving game states.
     */
    class SaveFile {
    public:
        /**
         * @brief Constructor
         * @param saveManager Save manager instance
         */
        explicit SaveFile(SaveManager* saveManager);

        /**
         * @brief Destructor
         */
        ~SaveFile();

        /**
         * @brief Deleted copy constructor
         */
        SaveFile(const SaveFile&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        SaveFile& operator=(const SaveFile&) = delete;

        // Save file lifecycle

        /**
         * @brief Initialize save file system
         * @return true if successful, false otherwise
         */
        bool Initialize();

        /**
         * @brief Shutdown save file system
         */
        void Shutdown();

        /**
         * @brief Get save file statistics
         * @return Current statistics
         */
        const SaveFileStats& GetStats() const { return m_stats; }

        // File creation and loading

        /**
         * @brief Create new save file
         * @param filePath File path
         * @param metadata File metadata
         * @param format File format
         * @return true if successful, false otherwise
         */
        bool CreateSaveFile(const std::string& filePath,
                          const SaveFileMetadata& metadata,
                          SaveFileFormat format = SaveFileFormat::CompressedBinaryV2);

        /**
         * @brief Load save file
         * @param filePath File path
         * @return true if successful, false otherwise
         */
        bool LoadSaveFile(const std::string& filePath);

        /**
         * @brief Save data to file
         * @param filePath File path
         * @param metadata File metadata
         * @param sectionData Section data map
         * @return true if successful, false otherwise
         */
        bool SaveDataToFile(const std::string& filePath,
                          const SaveFileMetadata& metadata,
                          const std::unordered_map<SaveFileSection, std::vector<uint8_t>>& sectionData);

        /**
         * @brief Load data from file
         * @param filePath File path
         * @param sectionData Output section data map
         * @return true if successful, false otherwise
         */
        bool LoadDataFromFile(const std::string& filePath,
                            std::unordered_map<SaveFileSection, std::vector<uint8_t>>& sectionData);

        // File validation and integrity

        /**
         * @brief Validate save file
         * @param filePath File path
         * @return true if valid, false otherwise
         */
        bool ValidateSaveFile(const std::string& filePath);

        /**
         * @brief Check file integrity
         * @param filePath File path
         * @return Integrity status
         */
        FileIntegrityStatus CheckFileIntegrity(const std::string& filePath);

        /**
         * @brief Repair corrupted save file
         * @param filePath File path
         * @return true if successful, false otherwise
         */
        bool RepairSaveFile(const std::string& filePath);

        /**
         * @brief Calculate file checksum
         * @param filePath File path
         * @return Checksum string
         */
        std::string CalculateFileChecksum(const std::string& filePath);

        /**
         * @brief Verify file checksum
         * @param filePath File path
         * @param expectedChecksum Expected checksum
         * @return true if matches, false otherwise
         */
        bool VerifyFileChecksum(const std::string& filePath, const std::string& expectedChecksum);

        // Metadata management

        /**
         * @brief Read file metadata
         * @param filePath File path
         * @return File metadata
         */
        SaveFileMetadata ReadFileMetadata(const std::string& filePath);

        /**
         * @brief Write file metadata
         * @param filePath File path
         * @param metadata File metadata
         * @return true if successful, false otherwise
         */
        bool WriteFileMetadata(const std::string& filePath, const SaveFileMetadata& metadata);

        /**
         * @brief Update file metadata
         * @param filePath File path
         * @param updates Metadata updates
         * @return true if successful, false otherwise
         */
        bool UpdateFileMetadata(const std::string& filePath,
                              const std::unordered_map<std::string, std::string>& updates);

        // Section management

        /**
         * @brief Write section to file
         * @param filePath File path
         * @param sectionType Section type
         * @param data Section data
         * @return true if successful, false otherwise
         */
        bool WriteSectionToFile(const std::string& filePath,
                              SaveFileSection sectionType,
                              const std::vector<uint8_t>& data);

        /**
         * @brief Read section from file
         * @param filePath File path
         * @param sectionType Section type
         * @return Section data
         */
        std::vector<uint8_t> ReadSectionFromFile(const std::string& filePath, SaveFileSection sectionType);

        /**
         * @brief Get section list from file
         * @param filePath File path
         * @return Vector of section types
         */
        std::vector<SaveFileSection> GetSectionList(const std::string& filePath);

        /**
         * @brief Get section information
         * @param filePath File path
         * @param sectionType Section type
         * @return Section information
         */
        SaveFileSectionHeader GetSectionInfo(const std::string& filePath, SaveFileSection sectionType);

        // File format conversion

        /**
         * @brief Convert save file format
         * @param inputPath Input file path
         * @param outputPath Output file path
         * @param newFormat New file format
         * @return true if successful, false otherwise
         */
        bool ConvertFileFormat(const std::string& inputPath,
                             const std::string& outputPath,
                             SaveFileFormat newFormat);

        /**
         * @brief Optimize save file
         * @param filePath File path
         * @return true if successful, false otherwise
         */
        bool OptimizeSaveFile(const std::string& filePath);

        /**
         * @brief Defragment save file
         * @param filePath File path
         * @return true if successful, false otherwise
         */
        bool DefragmentSaveFile(const std::string& filePath);

        // Utility functions

        /**
         * @brief Get file format from path
         * @param filePath File path
         * @return File format
         */
        static SaveFileFormat GetFormatFromPath(const std::string& filePath);

        /**
         * @brief Get file extension for format
         * @param format File format
         * @return File extension
         */
        static std::string GetExtensionForFormat(SaveFileFormat format);

        /**
         * @brief Get format description
         * @param format File format
         * @return Format description
         */
        static std::string GetFormatDescription(SaveFileFormat format);

        /**
         * @brief Check if format is compressed
         * @param format File format
         * @return true if compressed, false otherwise
         */
        static bool IsFormatCompressed(SaveFileFormat format);

        /**
         * @brief Get section type name
         * @param sectionType Section type
         * @return Section type name
         */
        static std::string GetSectionTypeName(SaveFileSection sectionType);

        /**
         * @brief Get supported file formats
         * @return Vector of supported formats
         */
        static std::vector<SaveFileFormat> GetSupportedFormats();

        /**
         * @brief Validate file path
         * @param filePath File path
         * @return true if valid, false otherwise
         */
        static bool ValidateFilePath(const std::string& filePath);

        /**
         * @brief Get file size
         * @param filePath File path
         * @return File size in bytes
         */
        static uint64_t GetFileSize(const std::string& filePath);

        /**
         * @brief Get file creation time
         * @param filePath File path
         * @return Creation time as string
         */
        static std::string GetFileCreationTime(const std::string& filePath);

        /**
         * @brief Get file modification time
         * @param filePath File path
         * @return Modification time as string
         */
        static std::string GetFileModificationTime(const std::string& filePath);

        /**
         * @brief Validate save file system
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
         * @brief Optimize save file system
         * @return Optimization results
         */
        std::unordered_map<std::string, size_t> Optimize();

    private:
        /**
         * @brief Create save file reader
         * @param filePath File path
         * @param format File format
         * @return Reader instance or nullptr if failed
         */
        std::unique_ptr<SaveFileReader> CreateReader(const std::string& filePath, SaveFileFormat format);

        /**
         * @brief Create save file writer
         * @param filePath File path
         * @param format File format
         * @return Writer instance or nullptr if failed
         */
        std::unique_ptr<SaveFileWriter> CreateWriter(const std::string& filePath, SaveFileFormat format);

        /**
         * @brief Compress data
         * @param data Input data
         * @param compressionType Compression type
         * @return Compressed data
         */
        std::vector<uint8_t> CompressData(const std::vector<uint8_t>& data, uint32_t compressionType);

        /**
         * @brief Decompress data
         * @param compressedData Input compressed data
         * @param uncompressedSize Expected uncompressed size
         * @param compressionType Compression type
         * @return Decompressed data
         */
        std::vector<uint8_t> DecompressData(const std::vector<uint8_t>& compressedData,
                                          size_t uncompressedSize,
                                          uint32_t compressionType);

        /**
         * @brief Calculate data checksum
         * @param data Input data
         * @return Checksum bytes
         */
        std::array<uint8_t, 32> CalculateChecksum(const std::vector<uint8_t>& data) const;

        /**
         * @brief Verify data checksum
         * @param data Input data
         * @param expectedChecksum Expected checksum
         * @return true if matches, false otherwise
         */
        bool VerifyChecksum(const std::vector<uint8_t>& data, const std::array<uint8_t, 32>& expectedChecksum) const;

        /**
         * @brief Update performance statistics
         * @param operationTime Operation time in milliseconds
         * @param dataSize Data size in bytes
         * @param operation Operation type
         */
        void UpdateStats(double operationTime, size_t dataSize, const std::string& operation);

        /**
         * @brief Handle save file errors
         * @param error Error message
         */
        void HandleError(const std::string& error);

        // Save file data
        SaveManager* m_saveManager;                 ///< Save manager instance
        SaveFileStats m_stats;                      ///< Performance statistics

        // File readers and writers
        std::unordered_map<std::string, std::unique_ptr<SaveFileReader>> m_readers; ///< Active readers
        std::unordered_map<std::string, std::unique_ptr<SaveFileWriter>> m_writers; ///< Active writers
        mutable std::shared_mutex m_fileMutex;      ///< File operations synchronization

        // State tracking
        bool m_isInitialized;                       ///< System is initialized
        double m_lastUpdateTime;                    ///< Last update time
        std::string m_lastError;                    ///< Last error message
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_SAVE_SAVE_FILE_HPP
