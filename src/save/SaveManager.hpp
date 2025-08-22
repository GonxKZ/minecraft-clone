/**
 * @file SaveManager.hpp
 * @brief VoxelCraft Advanced Save Management System
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines the SaveManager class that provides comprehensive save/load
 * functionality for the VoxelCraft game engine, including binary serialization,
 * data compression, automatic backups, version migration, and performance optimization
 * for large-scale game worlds and complex game states.
 */

#ifndef VOXELCRAFT_SAVE_SAVE_MANAGER_HPP
#define VOXELCRAFT_SAVE_SAVE_MANAGER_HPP

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
#include <queue>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../core/Config.hpp"
#include "../core/System.hpp"

namespace VoxelCraft {

    // Forward declarations
    class World;
    class Player;
    class EntityManager;
    class Serializer;
    class Compressor;
    class BackupSystem;
    class VersionManager;
    class SaveFile;

    /**
     * @enum SaveFormat
     * @brief Supported save file formats
     */
    enum class SaveFormat {
        Binary,                 ///< Custom binary format (fastest)
        CompressedBinary,       ///< Compressed binary with LZ4
        JSON,                   ///< JSON format (human readable)
        MessagePack,            ///< MessagePack format
        Custom                  ///< Custom format
    };

    /**
     * @enum SaveType
     * @brief Types of save operations
     */
    enum class SaveType {
        QuickSave,              ///< Quick save (current state)
        FullSave,               ///< Full save (complete game state)
        IncrementalSave,        ///< Incremental save (changes only)
        EmergencySave,          ///< Emergency save (critical state)
        AutoSave,               ///< Automatic periodic save
        ManualSave              ///< Manual user save
    };

    /**
     * @enum LoadMode
     * @brief Save file loading modes
     */
    enum class LoadMode {
        FullLoad,               ///< Load complete game state
        PartialLoad,            ///< Load specific components
        StreamingLoad,          ///< Stream data as needed
        AsyncLoad               ///< Asynchronous loading
    };

    /**
     * @enum CompressionType
     * @brief Data compression algorithms
     */
    enum class CompressionType {
        None,                   ///< No compression
        LZ4,                    ///< LZ4 (fast compression/decompression)
        Zstandard,              ///< Zstandard (high compression ratio)
        Gzip,                   ///< Gzip (good compression)
        Brotli,                 ///< Brotli (very high compression)
        Custom                  ///< Custom compression
    };

    /**
     * @struct SaveConfig
     * @brief Save system configuration
     */
    struct SaveConfig {
        // Basic settings
        std::string saveDirectory;                  ///< Directory for save files
        SaveFormat defaultFormat;                   ///< Default save format
        CompressionType defaultCompression;         ///< Default compression type
        int compressionLevel;                       ///< Compression level (0-9)

        // Performance settings
        size_t maxSaveSize;                         ///< Maximum save file size (MB)
        int saveThreads;                            ///< Number of save threads
        int loadThreads;                            ///< Number of load threads
        bool enableAsyncSaving;                     ///< Enable asynchronous saving
        bool enableAsyncLoading;                    ///< Enable asynchronous loading

        // Auto-save settings
        bool enableAutoSave;                        ///< Enable automatic saving
        float autoSaveInterval;                     ///< Auto-save interval (minutes)
        int maxAutoSaves;                           ///< Maximum auto-save files
        SaveType autoSaveType;                      ///< Auto-save type

        // Backup settings
        bool enableBackup;                          ///< Enable backup system
        int maxBackups;                             ///< Maximum backup files
        float backupInterval;                       ///< Backup interval (minutes)
        bool backupOnSave;                          ///< Create backup on each save

        // Version management
        bool enableVersionMigration;                ///< Enable version migration
        bool enableCompatibilityMode;               ///< Enable compatibility mode
        std::string versionFile;                    ///< Version tracking file

        // Performance optimization
        bool enableSaveOptimization;                ///< Enable save optimization
        bool enableCompressionOptimization;         ///< Enable compression optimization
        size_t memoryBufferSize;                    ///< Memory buffer size (MB)
        bool enableStreaming;                       ///< Enable data streaming

        // Error handling
        bool enableSaveValidation;                  ///< Enable save file validation
        bool enableCorruptionDetection;             ///< Enable corruption detection
        bool enableAutoRepair;                      ///< Enable automatic repair
        std::string errorLogFile;                   ///< Error log file

        SaveConfig()
            : saveDirectory("saves")
            , defaultFormat(SaveFormat::CompressedBinary)
            , defaultCompression(CompressionType::LZ4)
            , compressionLevel(6)
            , maxSaveSize(1024)
            , saveThreads(4)
            , loadThreads(4)
            , enableAsyncSaving(true)
            , enableAsyncLoading(true)
            , enableAutoSave(true)
            , autoSaveInterval(5.0f)
            , maxAutoSaves(10)
            , autoSaveType(SaveType::IncrementalSave)
            , enableBackup(true)
            , maxBackups(20)
            , backupInterval(30.0f)
            , backupOnSave(true)
            , enableVersionMigration(true)
            , enableCompatibilityMode(true)
            , versionFile("version.json")
            , enableSaveOptimization(true)
            , enableCompressionOptimization(true)
            , memoryBufferSize(256)
            , enableStreaming(true)
            , enableSaveValidation(true)
            , enableCorruptionDetection(true)
            , enableAutoRepair(true)
            , errorLogFile("save_errors.log")
        {}
    };

    /**
     * @struct SaveMetadata
     * @brief Save file metadata
     */
    struct SaveMetadata {
        std::string saveName;                        ///< Save file name
        std::string description;                     ///< Save description
        std::string timestamp;                       ///< Save timestamp
        std::string gameVersion;                     ///< Game version
        std::string engineVersion;                   ///< Engine version
        uint32_t saveFormatVersion;                  ///< Save format version
        SaveType saveType;                           ///< Type of save
        CompressionType compression;                 ///< Compression used

        // Game state info
        std::string worldName;                       ///< World name
        std::string playerName;                      ///< Player name
        int playTime;                                ///< Total play time (minutes)
        int playerLevel;                             ///< Player level
        glm::vec3 playerPosition;                    ///< Player position

        // File info
        size_t uncompressedSize;                     ///< Uncompressed data size
        size_t compressedSize;                       ///< Compressed data size
        float compressionRatio;                      ///< Compression ratio
        std::string checksum;                        ///< File checksum
        bool isCorrupted;                            ///< Corruption flag

        // Performance info
        float saveTime;                              ///< Time to create save (seconds)
        float loadTime;                              ///< Time to load save (seconds)
        std::string savePath;                        ///< File system path
        std::string thumbnailPath;                   ///< Screenshot thumbnail path

        SaveMetadata()
            : saveFormatVersion(1)
            , saveType(SaveType::FullSave)
            , compression(CompressionType::LZ4)
            , playTime(0)
            , playerLevel(1)
            , playerPosition(0.0f, 0.0f, 0.0f)
            , uncompressedSize(0)
            , compressedSize(0)
            , compressionRatio(1.0f)
            , isCorrupted(false)
            , saveTime(0.0f)
            , loadTime(0.0f)
        {}
    };

    /**
     * @struct SaveStats
     * @brief Save system performance statistics
     */
    struct SaveStats {
        // Operation statistics
        uint64_t totalSaves;                         ///< Total save operations
        uint64_t totalLoads;                         ///< Total load operations
        uint64_t successfulSaves;                    ///< Successful saves
        uint64_t successfulLoads;                    ///< Successful loads
        uint64_t failedSaves;                        ///< Failed saves
        uint64_t failedLoads;                        ///< Failed loads

        // Performance metrics
        double averageSaveTime;                      ///< Average save time (ms)
        double averageLoadTime;                      ///< Average load time (ms)
        double minSaveTime;                          ///< Minimum save time (ms)
        double maxSaveTime;                          ///< Maximum save time (ms)
        double minLoadTime;                          ///< Minimum load time (ms)
        double maxLoadTime;                          ///< Maximum load time (ms)

        // Data metrics
        uint64_t totalDataSaved;                     ///< Total data saved (bytes)
        uint64_t totalDataLoaded;                    ///< Total data loaded (bytes)
        uint64_t totalCompressedData;                ///< Total compressed data (bytes)
        float averageCompressionRatio;               ///< Average compression ratio

        // Auto-save statistics
        uint64_t autoSaves;                          ///< Number of auto-saves
        uint64_t manualSaves;                        ///< Number of manual saves
        uint64_t emergencySaves;                     ///< Number of emergency saves

        // Error statistics
        uint64_t corruptionDetected;                 ///< Corruption detections
        uint64_t autoRepairs;                        ///< Automatic repairs
        uint64_t versionMigrations;                  ///< Version migrations
        uint64_t compatibilityIssues;                ///< Compatibility issues resolved

        // Threading statistics
        int activeSaveThreads;                       ///< Active save threads
        int activeLoadThreads;                       ///< Active load threads
        int queuedSaveOperations;                    ///< Queued save operations
        int queuedLoadOperations;                    ///< Queued load operations
    };

    /**
     * @struct SaveOperation
     * @brief Save operation descriptor
     */
    struct SaveOperation {
        uint32_t operationId;                        ///< Unique operation ID
        SaveType type;                               ///< Operation type
        std::string saveName;                        ///< Save file name
        std::string description;                     ///< Operation description
        double startTime;                            ///< Operation start time
        double endTime;                              ///< Operation end time
        bool completed;                              ///< Operation completed
        bool successful;                             ///< Operation successful
        std::string errorMessage;                    ///< Error message if failed

        // Progress tracking
        float progress;                              ///< Operation progress (0.0 - 1.0)
        std::string currentTask;                     ///< Current task description
        size_t processedData;                        ///< Processed data size
        size_t totalData;                            ///< Total data size

        // Callbacks
        std::function<void(float)> progressCallback; ///< Progress callback
        std::function<void(bool, const std::string&)> completionCallback; ///< Completion callback

        SaveOperation()
            : operationId(0)
            , startTime(0.0)
            , endTime(0.0)
            , completed(false)
            , successful(false)
            , progress(0.0f)
            , processedData(0)
            , totalData(0)
        {}
    };

    /**
     * @class SaveManager
     * @brief Advanced save management system
     *
     * The SaveManager class provides comprehensive save/load functionality for the
     * VoxelCraft game engine, featuring binary serialization, data compression,
     * automatic backups, version migration, and performance optimization for
     * large-scale game worlds and complex game states with enterprise-grade
     * reliability and performance.
     *
     * Key Features:
     * - Multiple save formats (Binary, Compressed, JSON, MessagePack)
     * - Advanced compression algorithms (LZ4, Zstd, Brotli)
     * - Asynchronous save/load operations with threading
     * - Automatic backup system with rotation
     * - Version migration and compatibility system
     * - Corruption detection and automatic repair
     * - Incremental and differential saves
     * - Save validation and integrity checking
     * - Performance monitoring and optimization
     * - Streaming save/load for large worlds
     * - Emergency save system
     * - Cross-platform save file compatibility
     *
     * The save system is designed to handle massive game worlds efficiently
     * while maintaining data integrity and providing fast save/load times.
     */
    class SaveManager : public System {
    public:
        /**
         * @brief Constructor
         * @param config Save system configuration
         */
        explicit SaveManager(const SaveConfig& config);

        /**
         * @brief Destructor
         */
        ~SaveManager();

        /**
         * @brief Deleted copy constructor
         */
        SaveManager(const SaveManager&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        SaveManager& operator=(const SaveManager&) = delete;

        // Save manager lifecycle

        /**
         * @brief Initialize save manager
         * @return true if successful, false otherwise
         */
        bool Initialize();

        /**
         * @brief Shutdown save manager
         */
        void Shutdown();

        /**
         * @brief Update save manager
         * @param deltaTime Time elapsed since last update
         */
        void Update(double deltaTime);

        /**
         * @brief Get save configuration
         * @return Current configuration
         */
        const SaveConfig& GetConfig() const { return m_config; }

        /**
         * @brief Set save configuration
         * @param config New configuration
         * @return true if successful, false otherwise
         */
        bool SetConfig(const SaveConfig& config);

        /**
         * @brief Get save statistics
         * @return Current statistics
         */
        const SaveStats& GetStats() const { return m_stats; }

        // Save operations

        /**
         * @brief Create save operation
         * @param saveName Save file name
         * @param type Save operation type
         * @param description Save description
         * @return Operation ID or 0 if failed
         */
        uint32_t CreateSaveOperation(const std::string& saveName,
                                   SaveType type = SaveType::FullSave,
                                   const std::string& description = "");

        /**
         * @brief Execute save operation
         * @param operationId Operation ID
         * @return true if successful, false otherwise
         */
        bool ExecuteSaveOperation(uint32_t operationId);

        /**
         * @brief Cancel save operation
         * @param operationId Operation ID
         * @return true if successful, false otherwise
         */
        bool CancelSaveOperation(uint32_t operationId);

        /**
         * @brief Quick save current game state
         * @param saveName Optional save name
         * @return true if successful, false otherwise
         */
        bool QuickSave(const std::string& saveName = "");

        /**
         * @brief Full save complete game state
         * @param saveName Save file name
         * @param description Save description
         * @return true if successful, false otherwise
         */
        bool FullSave(const std::string& saveName, const std::string& description = "");

        /**
         * @brief Incremental save (changes only)
         * @param saveName Save file name
         * @return true if successful, false otherwise
         */
        bool IncrementalSave(const std::string& saveName);

        /**
         * @brief Emergency save (critical state)
         * @param saveName Save file name
         * @return true if successful, false otherwise
         */
        bool EmergencySave(const std::string& saveName);

        // Load operations

        /**
         * @brief Load save file
         * @param saveName Save file name
         * @param mode Load mode
         * @return true if successful, false otherwise
         */
        bool LoadSave(const std::string& saveName, LoadMode mode = LoadMode::FullLoad);

        /**
         * @brief Quick load last save
         * @return true if successful, false otherwise
         */
        bool QuickLoad();

        /**
         * @brief Load save with progress callback
         * @param saveName Save file name
         * @param progressCallback Progress callback function
         * @return true if successful, false otherwise
         */
        bool LoadSaveWithProgress(const std::string& saveName,
                                std::function<void(float, const std::string&)> progressCallback);

        /**
         * @brief Check if save file exists
         * @param saveName Save file name
         * @return true if exists, false otherwise
         */
        bool SaveExists(const std::string& saveName) const;

        /**
         * @brief Delete save file
         * @param saveName Save file name
         * @return true if successful, false otherwise
         */
        bool DeleteSave(const std::string& saveName);

        // Save file management

        /**
         * @brief Get list of save files
         * @return Vector of save metadata
         */
        std::vector<SaveMetadata> GetSaveList() const;

        /**
         * @brief Get save metadata
         * @param saveName Save file name
         * @return Save metadata or nullopt if not found
         */
        std::optional<SaveMetadata> GetSaveMetadata(const std::string& saveName) const;

        /**
         * @brief Validate save file integrity
         * @param saveName Save file name
         * @return true if valid, false otherwise
         */
        bool ValidateSaveFile(const std::string& saveName);

        /**
         * @brief Repair corrupted save file
         * @param saveName Save file name
         * @return true if successful, false otherwise
         */
        bool RepairSaveFile(const std::string& saveName);

        // Auto-save system

        /**
         * @brief Enable auto-save
         * @param enabled Enable state
         * @return true if successful, false otherwise
         */
        bool EnableAutoSave(bool enabled);

        /**
         * @brief Set auto-save interval
         * @param interval Interval in minutes
         * @return true if successful, false otherwise
         */
        bool SetAutoSaveInterval(float interval);

        /**
         * @brief Force auto-save now
         * @return true if successful, false otherwise
         */
        bool ForceAutoSave();

        // Backup system

        /**
         * @brief Create backup of save file
         * @param saveName Save file name
         * @return true if successful, false otherwise
         */
        bool CreateBackup(const std::string& saveName);

        /**
         * @brief Restore from backup
         * @param saveName Save file name
         * @param backupIndex Backup index
         * @return true if successful, false otherwise
         */
        bool RestoreFromBackup(const std::string& saveName, int backupIndex);

        /**
         * @brief Get backup list for save file
         * @param saveName Save file name
         * @return Vector of backup timestamps
         */
        std::vector<std::string> GetBackupList(const std::string& saveName) const;

        /**
         * @brief Cleanup old backups
         * @param maxBackups Maximum number of backups to keep
         * @return Number of backups removed
         */
        int CleanupBackups(int maxBackups = -1);

        // Version management

        /**
         * @brief Check save file compatibility
         * @param saveName Save file name
         * @return Compatibility status
         */
        std::string CheckCompatibility(const std::string& saveName) const;

        /**
         * @brief Migrate save file to current version
         * @param saveName Save file name
         * @return true if successful, false otherwise
         */
        bool MigrateSaveFile(const std::string& saveName);

        /**
         * @brief Get current save format version
         * @return Version number
         */
        uint32_t GetCurrentSaveVersion() const { return m_currentSaveVersion; }

        /**
         * @brief Set save format version
         * @param version Version number
         * @return true if successful, false otherwise
         */
        bool SetSaveFormatVersion(uint32_t version);

        // Performance and optimization

        /**
         * @brief Optimize save system for current hardware
         * @return true if successful, false otherwise
         */
        bool OptimizeForHardware();

        /**
         * @brief Set compression level
         * @param level Compression level (0-9)
         * @return true if successful, false otherwise
         */
        bool SetCompressionLevel(int level);

        /**
         * @brief Enable data streaming
         * @param enabled Enable state
         * @return true if successful, false otherwise
         */
        bool EnableStreaming(bool enabled);

        /**
         * @brief Set memory buffer size
         * @param size Buffer size in MB
         * @return true if successful, false otherwise
         */
        bool SetMemoryBufferSize(size_t size);

        // Game state management

        /**
         * @brief Register game object for saving
         * @param objectId Object ID
         * @param saveCallback Save callback function
         * @param loadCallback Load callback function
         * @return true if successful, false otherwise
         */
        bool RegisterGameObject(uint32_t objectId,
                              std::function<bool(Serializer&)> saveCallback,
                              std::function<bool(Serializer&)> loadCallback);

        /**
         * @brief Unregister game object
         * @param objectId Object ID
         * @return true if successful, false otherwise
         */
        bool UnregisterGameObject(uint32_t objectId);

        /**
         * @brief Set world instance for saving
         * @param world World instance
         */
        void SetWorld(World* world);

        /**
         * @brief Set player instance for saving
         * @param player Player instance
         */
        void SetPlayer(Player* player);

        /**
         * @brief Set entity manager for saving
         * @param entityManager Entity manager instance
         */
        void SetEntityManager(EntityManager* entityManager);

        // Utility functions

        /**
         * @brief Get save file path
         * @param saveName Save file name
         * @return Full file path
         */
        std::string GetSaveFilePath(const std::string& saveName) const;

        /**
         * @brief Get save directory path
         * @return Save directory path
         */
        const std::string& GetSaveDirectory() const { return m_config.saveDirectory; }

        /**
         * @brief Create save directory
         * @return true if successful, false otherwise
         */
        bool CreateSaveDirectory();

        /**
         * @brief Get disk space information
         * @return Disk space info (free, total, used)
         */
        std::tuple<size_t, size_t, size_t> GetDiskSpaceInfo() const;

        /**
         * @brief Validate save manager state
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
         * @brief Optimize save manager
         * @return Optimization results
         */
        std::unordered_map<std::string, size_t> Optimize();

    private:
        /**
         * @brief Initialize save subsystems
         * @return true if successful, false otherwise
         */
        bool InitializeSubsystems();

        /**
         * @brief Save game state to file
         * @param operation Save operation
         * @return true if successful, false otherwise
         */
        bool SaveGameState(SaveOperation& operation);

        /**
         * @brief Load game state from file
         * @param saveName Save file name
         * @param mode Load mode
         * @return true if successful, false otherwise
         */
        bool LoadGameState(const std::string& saveName, LoadMode mode);

        /**
         * @brief Compress save data
         * @param data Input data
         * @param compressedData Output compressed data
         * @param compressionType Compression type
         * @return true if successful, false otherwise
         */
        bool CompressSaveData(const std::vector<uint8_t>& data,
                            std::vector<uint8_t>& compressedData,
                            CompressionType compressionType);

        /**
         * @brief Decompress save data
         * @param compressedData Input compressed data
         * @param data Output decompressed data
         * @param compressionType Compression type
         * @return true if successful, false otherwise
         */
        bool DecompressSaveData(const std::vector<uint8_t>& compressedData,
                              std::vector<uint8_t>& data,
                              CompressionType compressionType);

        /**
         * @brief Calculate data checksum
         * @param data Input data
         * @return Checksum string
         */
        std::string CalculateChecksum(const std::vector<uint8_t>& data) const;

        /**
         * @brief Validate data checksum
         * @param data Input data
         * @param expectedChecksum Expected checksum
         * @return true if valid, false otherwise
         */
        bool ValidateChecksum(const std::vector<uint8_t>& data, const std::string& expectedChecksum) const;

        /**
         * @brief Update auto-save timer
         * @param deltaTime Time elapsed
         */
        void UpdateAutoSave(double deltaTime);

        /**
         * @brief Clean up completed operations
         */
        void CleanupCompletedOperations();

        /**
         * @brief Handle save errors
         * @param error Error message
         */
        void HandleError(const std::string& error);

        /**
         * @brief Update performance statistics
         * @param deltaTime Time elapsed
         */
        void UpdateStats(double deltaTime);

        // Save manager data
        SaveConfig m_config;                         ///< Save system configuration
        SaveStats m_stats;                           ///< Performance statistics

        // Core systems
        World* m_world;                              ///< Game world instance
        Player* m_player;                            ///< Player instance
        EntityManager* m_entityManager;              ///< Entity manager instance

        // Save subsystems
        std::unique_ptr<Serializer> m_serializer;    ///< Data serializer
        std::unique_ptr<Compressor> m_compressor;    ///< Data compressor
        std::unique_ptr<BackupSystem> m_backupSystem; ///< Backup system
        std::unique_ptr<VersionManager> m_versionManager; ///< Version manager

        // Save operations
        std::unordered_map<uint32_t, SaveOperation> m_saveOperations; ///< Active operations
        std::queue<uint32_t> m_operationQueue;       ///< Operation queue
        mutable std::shared_mutex m_operationsMutex; ///< Operations synchronization

        // Game object callbacks
        std::unordered_map<uint32_t, std::pair<std::function<bool(Serializer&)>, std::function<bool(Serializer&)>>> m_gameObjects; ///< Registered game objects
        mutable std::shared_mutex m_gameObjectsMutex; ///< Game objects synchronization

        // Auto-save system
        double m_lastAutoSaveTime;                   ///< Last auto-save time
        double m_autoSaveTimer;                      ///< Auto-save timer
        bool m_autoSaveEnabled;                      ///< Auto-save enabled

        // State tracking
        bool m_isInitialized;                        ///< System is initialized
        double m_lastUpdateTime;                     ///< Last update time
        uint32_t m_currentSaveVersion;               ///< Current save format version
        std::string m_lastError;                     ///< Last error message

        static std::atomic<uint32_t> s_nextOperationId; ///< Next operation ID counter
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_SAVE_SAVE_MANAGER_HPP
