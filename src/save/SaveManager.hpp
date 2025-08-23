/**
 * @file SaveManager.hpp
 * @brief VoxelCraft Advanced Save/Load System
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_SAVE_SAVE_MANAGER_HPP
#define VOXELCRAFT_SAVE_SAVE_MANAGER_HPP

#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
#include <atomic>
#include <mutex>
#include <functional>

namespace VoxelCraft {

    // Forward declarations
    class World;
    class Player;
    class Inventory;
    class EntityManager;
    struct SaveData;
    struct SaveMetadata;
    struct SaveOptions;

    /**
     * @enum SaveFormat
     * @brief Supported save file formats
     */
    enum class SaveFormat {
        BINARY,           ///< Custom binary format (fastest)
        JSON,             ///< JSON format (human readable)
        XML,              ///< XML format
        CBOR,             ///< CBOR format (binary JSON)
        MESSAGEPACK,      ///< MessagePack format
        SQLITE            ///< SQLite database format
    };

    /**
     * @enum CompressionType
     * @brief Compression algorithms for save files
     */
    enum class CompressionType {
        NONE,             ///< No compression
        GZIP,             ///< GZIP compression
        ZLIB,             ///< ZLIB compression
        LZ4,              ///< LZ4 compression (fastest)
        ZSTD,             ///< ZStandard compression (best ratio)
        SNAPPY,           ///< Snappy compression
        BROTLI            ///< Brotli compression
    };

    /**
     * @enum SaveType
     * @brief Types of save operations
     */
    enum class SaveType {
        MANUAL,           ///< Manual save by user
        AUTO,             ///< Automatic save
        QUICK,            ///< Quick save (minimal data)
        FULL,             ///< Full save (all data)
        BACKUP,           ///< Backup save
        CHECKPOINT        ///< Checkpoint save
    };

    /**
     * @enum SaveStatus
     * @brief Status of save operations
     */
    enum class SaveStatus {
        IDLE,             ///< No operation in progress
        PREPARING,        ///< Preparing data for save
        COMPRESSING,      ///< Compressing data
        WRITING,          ///< Writing to disk
        FINALIZING,       ///< Finalizing save
        COMPLETE,         ///< Save completed successfully
        FAILED,           ///< Save failed
        CORRUPTED         ///< Save file corrupted
    };

    /**
     * @struct SaveOptions
     * @brief Configuration options for save operations
     */
    struct SaveOptions {
        SaveFormat format = SaveFormat::BINARY;
        CompressionType compression = CompressionType::ZSTD;
        bool enableEncryption = false;
        std::string encryptionKey = "";
        bool createBackup = true;
        bool compressWorldData = true;
        bool compressEntityData = true;
        bool compressInventoryData = true;
        int compressionLevel = 6; // 1-22 for ZSTD
        size_t maxChunkSize = 1024 * 1024; // 1MB chunks
        bool enableProgressCallback = true;
        std::function<void(float, const std::string&)> progressCallback;
    };

    /**
     * @struct SaveMetadata
     * @brief Metadata for save files
     */
    struct SaveMetadata {
        std::string worldName;
        std::string playerName;
        uint64_t timestamp;
        uint32_t version;
        SaveFormat format;
        CompressionType compression;
        uint64_t uncompressedSize;
        uint64_t compressedSize;
        double compressionRatio;
        uint32_t chunkCount;
        uint32_t entityCount;
        uint32_t itemCount;
        uint32_t playTimeSeconds;
        std::string gameVersion;
        std::string platform;
        std::unordered_map<std::string, std::string> customData;
    };

    /**
     * @struct SaveData
     * @brief Container for all game data to be saved
     */
    struct SaveData {
        std::unique_ptr<World> world;
        std::unique_ptr<Player> player;
        std::unique_ptr<Inventory> playerInventory;
        std::unique_ptr<EntityManager> entityManager;
        SaveMetadata metadata;
        std::unordered_map<std::string, std::string> customData;
    };

    /**
     * @class SaveManager
     * @brief Advanced save/load system for VoxelCraft
     *
     * Features:
     * - Multiple save formats with automatic format detection
     * - Advanced compression with 6 algorithms (LZ4, ZSTD, GZIP, etc.)
     * - Encryption support for save files
     * - Automatic backup system with rollback
     * - Incremental saves and quick saves
     * - Save file validation and corruption detection
     * - Cross-platform compatibility
     * - Save file management and cleanup
     * - Progress tracking and callbacks
     * - Memory-efficient streaming for large worlds
     * - Multi-threaded save/load operations
     */
    class SaveManager {
    public:
        static SaveManager& GetInstance();

        /**
         * @brief Initialize the save manager
         * @param saveDirectory Base directory for saves
         * @return true if successful
         */
        bool Initialize(const std::string& saveDirectory);

        /**
         * @brief Shutdown the save manager
         */
        void Shutdown();

        // Save Operations
        /**
         * @brief Save game with specified options
         * @param saveData Game data to save
         * @param saveName Name of the save file
         * @param options Save options
         * @return true if successful
         */
        bool SaveGame(const SaveData& saveData, const std::string& saveName, const SaveOptions& options = SaveOptions());

        /**
         * @brief Quick save current game state
         * @return true if successful
         */
        bool QuickSave();

        /**
         * @brief Auto save current game state
         * @return true if successful
         */
        bool AutoSave();

        /**
         * @brief Create backup of current save
         * @param saveName Name of the save to backup
         * @return true if successful
         */
        bool CreateBackup(const std::string& saveName);

        // Load Operations
        /**
         * @brief Load game from save file
         * @param saveName Name of the save file
         * @param saveData Output game data
         * @return true if successful
         */
        bool LoadGame(const std::string& saveName, SaveData& saveData);

        /**
         * @brief Quick load last save
         * @param saveData Output game data
         * @return true if successful
         */
        bool QuickLoad(SaveData& saveData);

        /**
         * @brief Load from backup
         * @param saveName Name of the save
         * @param backupIndex Index of backup to load
         * @param saveData Output game data
         * @return true if successful
         */
        bool LoadFromBackup(const std::string& saveName, int backupIndex, SaveData& saveData);

        // Save Management
        /**
         * @brief Get list of available saves
         * @return Vector of save names with metadata
         */
        std::vector<std::pair<std::string, SaveMetadata>> GetAvailableSaves();

        /**
         * @brief Get save metadata
         * @param saveName Name of the save
         * @return Save metadata
         */
        SaveMetadata GetSaveMetadata(const std::string& saveName);

        /**
         * @brief Delete save file
         * @param saveName Name of the save to delete
         * @return true if successful
         */
        bool DeleteSave(const std::string& saveName);

        /**
         * @brief Rename save file
         * @param oldName Current name
         * @param newName New name
         * @return true if successful
         */
        bool RenameSave(const std::string& oldName, const std::string& newName);

        /**
         * @brief Validate save file integrity
         * @param saveName Name of the save
         * @return true if valid
         */
        bool ValidateSave(const std::string& saveName);

        /**
         * @brief Repair corrupted save file
         * @param saveName Name of the save
         * @return true if repaired
         */
        bool RepairSave(const std::string& saveName);

        // Backup Management
        /**
         * @brief Get list of backups for a save
         * @param saveName Name of the save
         * @return Vector of backup metadata
         */
        std::vector<SaveMetadata> GetBackups(const std::string& saveName);

        /**
         * @brief Delete old backups
         * @param saveName Name of the save
         * @param keepCount Number of backups to keep
         * @return Number of backups deleted
         */
        int CleanupBackups(const std::string& saveName, int keepCount = 10);

        // Status and Progress
        /**
         * @brief Get current save status
         * @return Current status
         */
        SaveStatus GetSaveStatus() const { return m_saveStatus.load(); }

        /**
         * @brief Get save progress (0.0 - 1.0)
         * @return Progress value
         */
        float GetSaveProgress() const { return m_saveProgress.load(); }

        /**
         * @brief Get current save operation message
         * @return Status message
         */
        std::string GetSaveMessage() const;

        /**
         * @brief Cancel current save operation
         * @return true if cancelled
         */
        bool CancelSave();

        // Configuration
        /**
         * @brief Set save options
         * @param options New save options
         */
        void SetSaveOptions(const SaveOptions& options) { m_saveOptions = options; }

        /**
         * @brief Get current save options
         * @return Current options
         */
        const SaveOptions& GetSaveOptions() const { return m_saveOptions; }

        /**
         * @brief Set auto save interval
         * @param minutes Interval in minutes
         */
        void SetAutoSaveInterval(int minutes) { m_autoSaveIntervalMinutes = minutes; }

        // Utility
        /**
         * @brief Get save file path
         * @param saveName Name of the save
         * @return Full file path
         */
        std::string GetSaveFilePath(const std::string& saveName) const;

        /**
         * @brief Get backup file path
         * @param saveName Name of the save
         * @param index Backup index
         * @return Full backup file path
         */
        std::string GetBackupFilePath(const std::string& saveName, int index) const;

        /**
         * @brief Get total save file size
         * @param saveName Name of the save
         * @return Size in bytes
         */
        uint64_t GetSaveFileSize(const std::string& saveName) const;

        /**
         * @brief Get free disk space
         * @param path Path to check
         * @return Free space in bytes
         */
        uint64_t GetFreeDiskSpace(const std::string& path) const;

        /**
         * @brief Check if disk space is sufficient
         * @param requiredBytes Required space
         * @return true if sufficient
         */
        bool CheckDiskSpace(uint64_t requiredBytes) const;

    private:
        SaveManager() = default;
        ~SaveManager();

        // Prevent copying
        SaveManager(const SaveManager&) = delete;
        SaveManager& operator=(const SaveManager&) = delete;

        // Core functionality
        bool SaveWorldData(const SaveData& saveData, const std::string& filePath);
        bool SavePlayerData(const SaveData& saveData, const std::string& filePath);
        bool SaveEntityData(const SaveData& saveData, const std::string& filePath);
        bool SaveInventoryData(const SaveData& saveData, const std::string& filePath);

        bool LoadWorldData(const std::string& filePath, SaveData& saveData);
        bool LoadPlayerData(const std::string& filePath, SaveData& saveData);
        bool LoadEntityData(const std::string& filePath, SaveData& saveData);
        bool LoadInventoryData(const std::string& filePath, SaveData& saveData);

        // Compression/Decompression
        std::vector<uint8_t> CompressData(const std::vector<uint8_t>& data, CompressionType type, int level = 6);
        std::vector<uint8_t> DecompressData(const std::vector<uint8_t>& data, CompressionType type, uint64_t originalSize);
        double CalculateCompressionRatio(uint64_t original, uint64_t compressed) const;

        // Encryption/Decryption
        std::vector<uint8_t> EncryptData(const std::vector<uint8_t>& data, const std::string& key);
        std::vector<uint8_t> DecryptData(const std::vector<uint8_t>& data, const std::string& key);

        // File operations
        bool WriteSaveFile(const std::string& filePath, const std::vector<uint8_t>& data);
        bool ReadSaveFile(const std::string& filePath, std::vector<uint8_t>& data);
        bool WriteMetadata(const std::string& filePath, const SaveMetadata& metadata);
        bool ReadMetadata(const std::string& filePath, SaveMetadata& metadata);

        // Backup operations
        bool CreateBackupInternal(const std::string& saveName, const std::string& backupName);
        bool RestoreFromBackupInternal(const std::string& saveName, const std::string& backupName, SaveData& saveData);

        // Validation
        bool ValidateSaveData(const SaveData& saveData);
        uint32_t CalculateChecksum(const std::vector<uint8_t>& data) const;
        bool VerifyChecksum(const std::vector<uint8_t>& data, uint32_t expectedChecksum) const;

        // Utility
        std::string GenerateTimestamp() const;
        std::string GenerateUniqueSaveName(const std::string& baseName) const;
        std::vector<std::string> GetSaveFiles() const;
        std::vector<std::string> GetBackupFiles(const std::string& saveName) const;
        bool EnsureDirectoryExists(const std::string& path) const;

        // Member variables
        std::string m_saveDirectory;
        SaveOptions m_saveOptions;
        bool m_initialized = false;

        // Status tracking
        std::atomic<SaveStatus> m_saveStatus;
        std::atomic<float> m_saveProgress;
        std::string m_currentSaveMessage;
        mutable std::shared_mutex m_statusMutex;

        // Auto save
        int m_autoSaveIntervalMinutes = 5;
        uint64_t m_lastAutoSaveTime = 0;

        // Current operation tracking
        std::atomic<bool> m_cancelRequested;
        std::string m_currentSaveName;

        // Thread safety
        mutable std::shared_mutex m_saveMutex;
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_SAVE_SAVE_MANAGER_HPP
