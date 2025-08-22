/**
 * @file SaveManager.hpp
 * @brief VoxelCraft Save System Manager
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_CORE_SAVE_MANAGER_HPP
#define VOXELCRAFT_CORE_SAVE_MANAGER_HPP

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <atomic>
#include <thread>
#include <chrono>
#include <filesystem>

namespace VoxelCraft {

// Forward declarations
class Config;
class World;
class Player;
struct Vec3;

/**
 * @enum SaveState
 * @brief Current save operation state
 */
enum class SaveState {
    IDLE = 0,       ///< Not currently saving
    SAVING,         ///< Currently saving data
    LOADING,        ///< Currently loading data
    ERROR           ///< Save/load error occurred
};

/**
 * @enum SaveType
 * @brief Types of save operations
 */
enum class SaveType {
    AUTO_SAVE = 0,  ///< Automatic periodic save
    MANUAL_SAVE,    ///< Manual user-initiated save
    QUICK_SAVE,     ///< Quick save (less data)
    FULL_SAVE       ///< Full comprehensive save
};

/**
 * @struct SaveOperation
 * @brief Information about a save operation
 */
struct SaveOperation {
    SaveType type;                      ///< Type of save operation
    std::string filename;               ///< Save file name
    std::chrono::steady_clock::time_point startTime; ///< Operation start time
    std::chrono::steady_clock::time_point endTime;   ///< Operation end time
    size_t dataSize;                    ///< Size of saved data
    bool success;                       ///< Whether operation succeeded
    std::string errorMessage;           ///< Error message if failed
};

/**
 * @struct SaveMetrics
 * @brief Save system performance metrics
 */
struct SaveMetrics {
    uint64_t totalSaves;               ///< Total number of saves performed
    uint64_t totalLoads;               ///< Total number of loads performed
    uint64_t failedSaves;              ///< Number of failed saves
    uint64_t failedLoads;              ///< Number of failed loads
    double averageSaveTime;            ///< Average save time (seconds)
    double averageLoadTime;            ///< Average load time (seconds)
    size_t totalDataSaved;             ///< Total data saved (bytes)
    size_t totalDataLoaded;            ///< Total data loaded (bytes)
    uint32_t autoSaveCount;            ///< Number of auto-saves performed
    uint32_t manualSaveCount;          ///< Number of manual saves performed
};

/**
 * @struct WorldSaveData
 * @brief World save data structure
 */
struct WorldSaveData {
    std::string worldName;             ///< World name
    uint64_t seed;                     ///< World seed
    uint32_t version;                  ///< Save format version
    std::chrono::system_clock::time_point created; ///< World creation time
    std::chrono::system_clock::time_point lastPlayed; ///< Last played time
    double playTime;                   ///< Total play time (seconds)
    Vec3 spawnPoint;                   ///< World spawn point
    std::unordered_map<std::string, std::string> metadata; ///< Additional metadata
};

/**
 * @struct PlayerSaveData
 * @brief Player save data structure
 */
struct PlayerSaveData {
    std::string playerName;            ///< Player name
    Vec3 position;                     ///< Player position
    Vec3 rotation;                     ///< Player rotation (pitch, yaw)
    float health;                      ///< Player health
    float food;                        ///< Player food level
    int32_t experience;                ///< Player experience points
    int32_t level;                     ///< Player level
    std::vector<std::string> inventory; ///< Player inventory items
    std::unordered_map<std::string, std::string> metadata; ///< Additional metadata
};

/**
 * @class SaveManager
 * @brief Manages game saving and loading operations
 *
 * The SaveManager handles all save/load operations including:
 * - World data persistence
 * - Player data persistence
 * - Automatic and manual saves
 * - Save file compression and encryption
 * - Backup and recovery
 * - Performance monitoring
 */
class SaveManager {
public:
    /**
     * @brief Constructor
     */
    SaveManager();

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

    /**
     * @brief Initialize the save manager
     * @param config Save configuration
     * @return true if successful, false otherwise
     */
    bool Initialize(const Config& config);

    /**
     * @brief Shutdown the save manager
     */
    void Shutdown();

    /**
     * @brief Update save manager state
     * @param deltaTime Time elapsed since last update
     */
    void Update(double deltaTime);

    /**
     * @brief Get current save state
     * @return Current save state
     */
    SaveState GetState() const { return m_state; }

    /**
     * @brief Get save metrics
     * @return Save performance metrics
     */
    const SaveMetrics& GetMetrics() const { return m_metrics; }

    /**
     * @brief Save world data
     * @param world World to save
     * @param saveType Type of save operation
     * @return true if successful, false otherwise
     */
    bool SaveWorld(const World* world, SaveType saveType = SaveType::FULL_SAVE);

    /**
     * @brief Load world data
     * @param worldName Name of world to load
     * @param world World object to load into
     * @return true if successful, false otherwise
     */
    bool LoadWorld(const std::string& worldName, World* world);

    /**
     * @brief Save player data
     * @param player Player to save
     * @param saveType Type of save operation
     * @return true if successful, false otherwise
     */
    bool SavePlayer(const Player* player, SaveType saveType = SaveType::FULL_SAVE);

    /**
     * @brief Load player data
     * @param playerName Name of player to load
     * @param player Player object to load into
     * @return true if successful, false otherwise
     */
    bool LoadPlayer(const std::string& playerName, Player* player);

    /**
     * @brief Perform auto-save if needed
     * @return true if auto-save was performed, false otherwise
     */
    bool AutoSave();

    /**
     * @brief Create backup of save files
     * @param worldName Name of world to backup
     * @return true if successful, false otherwise
     */
    bool CreateBackup(const std::string& worldName);

    /**
     * @brief Restore from backup
     * @param worldName Name of world to restore
     * @param backupName Name of backup to restore from
     * @return true if successful, false otherwise
     */
    bool RestoreFromBackup(const std::string& worldName, const std::string& backupName);

    /**
     * @brief List available worlds
     * @return Vector of world names
     */
    std::vector<std::string> ListWorlds() const;

    /**
     * @brief List available backups for world
     * @param worldName Name of world
     * @return Vector of backup names
     */
    std::vector<std::string> ListBackups(const std::string& worldName) const;

    /**
     * @brief Delete world save
     * @param worldName Name of world to delete
     * @return true if successful, false otherwise
     */
    bool DeleteWorld(const std::string& worldName);

    /**
     * @brief Get world save data
     * @param worldName Name of world
     * @return World save data, empty if not found
     */
    WorldSaveData GetWorldData(const std::string& worldName) const;

    /**
     * @brief Get player save data
     * @param playerName Name of player
     * @return Player save data, empty if not found
     */
    PlayerSaveData GetPlayerData(const std::string& playerName) const;

    /**
     * @brief Set auto-save interval
     * @param intervalSeconds Auto-save interval in seconds
     */
    void SetAutoSaveInterval(double intervalSeconds) { m_autoSaveInterval = intervalSeconds; }

    /**
     * @brief Get auto-save interval
     * @return Auto-save interval in seconds
     */
    double GetAutoSaveInterval() const { return m_autoSaveInterval; }

    /**
     * @brief Enable/disable compression
     * @param enabled Enable compression
     */
    void SetCompressionEnabled(bool enabled) { m_compressionEnabled = enabled; }

    /**
     * @brief Check if compression is enabled
     * @return true if compression is enabled
     */
    bool IsCompressionEnabled() const { return m_compressionEnabled; }

    /**
     * @brief Enable/disable encryption
     * @param enabled Enable encryption
     */
    void SetEncryptionEnabled(bool enabled) { m_encryptionEnabled = enabled; }

    /**
     * @brief Check if encryption is enabled
     * @return true if encryption is enabled
     */
    bool IsEncryptionEnabled() const { return m_encryptionEnabled; }

private:
    // Save configuration
    std::string m_saveDirectory;        ///< Save files directory
    double m_autoSaveInterval;          ///< Auto-save interval (seconds)
    bool m_compressionEnabled;          ///< Enable save file compression
    bool m_encryptionEnabled;           ///< Enable save file encryption
    uint32_t m_maxBackups;              ///< Maximum number of backups to keep

    // Save state
    std::atomic<SaveState> m_state;     ///< Current save state
    std::chrono::steady_clock::time_point m_lastAutoSave; ///< Last auto-save time
    std::chrono::steady_clock::time_point m_lastSaveTime; ///< Last save operation time

    // Metrics
    SaveMetrics m_metrics;              ///< Performance metrics
    mutable std::mutex m_metricsMutex;  ///< Metrics synchronization

    // Threading
    std::unique_ptr<std::thread> m_saveThread; ///< Save operation thread
    std::atomic<bool> m_saveThreadRunning; ///< Save thread running flag
    mutable std::mutex m_saveMutex;      ///< Save operation synchronization

    // Current operation
    SaveOperation m_currentOperation;    ///< Current save operation

    /**
     * @brief Save thread function
     */
    void SaveThread();

    /**
     * @brief Perform save operation
     * @param operation Save operation to perform
     * @return true if successful
     */
    bool PerformSave(const SaveOperation& operation);

    /**
     * @brief Perform load operation
     * @param filename File to load
     * @param data Output data buffer
     * @return true if successful
     */
    bool PerformLoad(const std::string& filename, std::vector<uint8_t>& data);

    /**
     * @brief Compress data
     * @param input Input data
     * @param output Output compressed data
     * @return true if successful
     */
    bool CompressData(const std::vector<uint8_t>& input, std::vector<uint8_t>& output);

    /**
     * @brief Decompress data
     * @param input Input compressed data
     * @param output Output decompressed data
     * @return true if successful
     */
    bool DecompressData(const std::vector<uint8_t>& input, std::vector<uint8_t>& output);

    /**
     * @brief Encrypt data
     * @param input Input data
     * @param output Output encrypted data
     * @return true if successful
     */
    bool EncryptData(const std::vector<uint8_t>& input, std::vector<uint8_t>& output);

    /**
     * @brief Decrypt data
     * @param input Input encrypted data
     * @param output Output decrypted data
     * @return true if successful
     */
    bool DecryptData(const std::vector<uint8_t>& input, std::vector<uint8_t>& output);

    /**
     * @brief Generate save filename
     * @param baseName Base name for save
     * @param saveType Type of save
     * @return Generated filename
     */
    std::string GenerateSaveFilename(const std::string& baseName, SaveType saveType);

    /**
     * @brief Generate backup filename
     * @param baseName Base name for backup
     * @param timestamp Timestamp for backup
     * @return Generated backup filename
     */
    std::string GenerateBackupFilename(const std::string& baseName, const std::chrono::system_clock::time_point& timestamp);

    /**
     * @brief Clean old backups
     * @param worldName Name of world
     */
    void CleanOldBackups(const std::string& worldName);

    /**
     * @brief Update save metrics
     * @param operation Completed operation
     */
    void UpdateMetrics(const SaveOperation& operation);

    /**
     * @brief Get world save directory
     * @param worldName Name of world
     * @return World save directory path
     */
    std::filesystem::path GetWorldSaveDirectory(const std::string& worldName) const;

    /**
     * @brief Get player save directory
     * @param playerName Name of player
     * @return Player save directory path
     */
    std::filesystem::path GetPlayerSaveDirectory(const std::string& playerName) const;
};

} // namespace VoxelCraft

#endif // VOXELCRAFT_CORE_SAVE_MANAGER_HPP
