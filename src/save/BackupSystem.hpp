/**
 * @file BackupSystem.hpp
 * @brief VoxelCraft Advanced Backup Management System
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines the BackupSystem class that provides comprehensive backup
 * management for the VoxelCraft game engine, including automatic backups,
 * backup rotation, backup verification, restore functionality, and disaster
 * recovery for game save files and critical game data.
 */

#ifndef VOXELCRAFT_SAVE_BACKUP_SYSTEM_HPP
#define VOXELCRAFT_SAVE_BACKUP_SYSTEM_HPP

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
#include <deque>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../core/Config.hpp"

namespace VoxelCraft {

    // Forward declarations
    class SaveManager;

    /**
     * @enum BackupType
     * @brief Types of backup operations
     */
    enum class BackupType {
        Automatic,              ///< Automatic scheduled backup
        Manual,                 ///< Manual user backup
        Emergency,              ///< Emergency backup (critical state)
        Incremental,            ///< Incremental backup (changes only)
        Differential,           ///< Differential backup (since last full)
        Full,                   ///< Full backup (complete data)
        Snapshot                ///< System snapshot backup
    };

    /**
     * @enum BackupStatus
     * @brief Status of backup operations
     */
    enum class BackupStatus {
        Pending,                ///< Backup is pending
        InProgress,             ///< Backup is in progress
        Completed,              ///< Backup completed successfully
        Failed,                 ///< Backup failed
        Cancelled,              ///< Backup was cancelled
        Corrupted,              ///< Backup is corrupted
        Expired                 ///< Backup has expired
    };

    /**
     * @enum RestoreMode
     * @brief Backup restoration modes
     */
    enum class RestoreMode {
        FullRestore,            ///< Complete restoration
        PartialRestore,         ///< Restore specific components
        PointInTimeRestore,     ///< Restore to specific point in time
        RollbackRestore         ///< Rollback to previous version
    };

    /**
     * @enum BackupStorage
     * @brief Backup storage locations
     */
    enum class BackupStorage {
        Local,                  ///< Local storage
        External,               ///< External drive
        Network,                ///< Network storage
        Cloud,                  ///< Cloud storage
        Hybrid                  ///< Hybrid storage (multiple locations)
    };

    /**
     * @struct BackupConfig
     * @brief Backup system configuration
     */
    struct BackupConfig {
        // Basic settings
        bool enableAutomaticBackups;               ///< Enable automatic backups
        bool enableManualBackups;                  ///< Enable manual backups
        bool enableEmergencyBackups;               ///< Enable emergency backups
        BackupStorage storageType;                 ///< Backup storage type

        // Timing settings
        float automaticBackupInterval;             ///< Auto backup interval (minutes)
        float emergencyBackupInterval;             ///< Emergency backup interval (minutes)
        int maxBackupAge;                          ///< Maximum backup age (days)
        int backupRetentionDays;                   ///< Backup retention period (days)

        // Storage settings
        std::string backupDirectory;               ///< Backup directory path
        std::string externalDrivePath;             ///< External drive path
        std::string networkPath;                   ///< Network storage path
        std::string cloudCredentials;              ///< Cloud storage credentials

        // Performance settings
        bool enableCompression;                    ///< Enable backup compression
        bool enableEncryption;                     ///< Enable backup encryption
        int compressionLevel;                      ///< Backup compression level
        std::string encryptionKey;                 ///< Encryption key
        bool enableVerification;                   ///< Enable backup verification

        // Rotation settings
        int maxAutomaticBackups;                   ///< Maximum automatic backups
        int maxManualBackups;                      ///< Maximum manual backups
        int maxEmergencyBackups;                   ///< Maximum emergency backups
        int maxIncrementalBackups;                 ///< Maximum incremental backups
        bool enableBackupRotation;                 ///< Enable backup rotation

        // Notification settings
        bool enableBackupNotifications;            ///< Enable backup notifications
        bool notifyOnFailure;                      ///< Notify on backup failure
        bool notifyOnSuccess;                      ///< Notify on backup success
        std::string notificationEmail;             ///< Notification email address

        // Recovery settings
        bool enableAutoRecovery;                   ///< Enable automatic recovery
        int recoveryAttempts;                      ///< Maximum recovery attempts
        float recoveryTimeout;                     ///< Recovery timeout (seconds)
        bool enableBackupIntegrityCheck;           ///< Enable integrity checking

        BackupConfig()
            : enableAutomaticBackups(true)
            , enableManualBackups(true)
            , enableEmergencyBackups(true)
            , storageType(BackupStorage::Local)
            , automaticBackupInterval(30.0f)
            , emergencyBackupInterval(5.0f)
            , maxBackupAge(30)
            , backupRetentionDays(90)
            , backupDirectory("backups")
            , enableCompression(true)
            , enableEncryption(false)
            , compressionLevel(6)
            , enableVerification(true)
            , maxAutomaticBackups(20)
            , maxManualBackups(50)
            , maxEmergencyBackups(10)
            , maxIncrementalBackups(100)
            , enableBackupRotation(true)
            , enableBackupNotifications(false)
            , notifyOnFailure(true)
            , notifyOnSuccess(false)
            , enableAutoRecovery(true)
            , recoveryAttempts(3)
            , recoveryTimeout(60.0f)
            , enableBackupIntegrityCheck(true)
        {}
    };

    /**
     * @struct BackupInfo
     * @brief Information about a backup
     */
    struct BackupInfo {
        uint32_t backupId;                         ///< Unique backup ID
        std::string backupName;                    ///< Backup name
        std::string saveFileName;                  ///< Associated save file
        BackupType type;                           ///< Backup type
        BackupStatus status;                       ///< Backup status

        // Timing information
        std::string creationTime;                  ///< Backup creation time
        std::string completionTime;                ///< Backup completion time
        double duration;                           ///< Backup duration (seconds)
        std::string expirationTime;                ///< Backup expiration time

        // File information
        std::string backupPath;                    ///< Backup file path
        size_t originalSize;                       ///< Original file size
        size_t backupSize;                         ///< Backup file size
        float compressionRatio;                    ///< Compression ratio
        std::string checksum;                      ///< Backup checksum

        // Metadata
        std::string description;                   ///< Backup description
        std::unordered_map<std::string, std::string> metadata; ///< Additional metadata
        std::string createdBy;                     ///< User/system that created backup

        // Verification
        bool isVerified;                           ///< Backup verification status
        std::string verificationTime;              ///< Last verification time
        std::vector<std::string> verificationErrors; ///< Verification errors

        BackupInfo()
            : backupId(0)
            , type(BackupType::Automatic)
            , status(BackupStatus::Pending)
            , duration(0.0)
            , originalSize(0)
            , backupSize(0)
            , compressionRatio(1.0f)
            , isVerified(false)
        {}
    };

    /**
     * @struct BackupStats
     * @brief Backup system performance statistics
     */
    struct BackupStats {
        // Operation statistics
        uint64_t totalBackupsCreated;              ///< Total backups created
        uint64_t totalBackupsRestored;             ///< Total backups restored
        uint64_t successfulBackups;                ///< Successful backup operations
        uint64_t failedBackups;                    ///< Failed backup operations
        uint64_t corruptedBackups;                 ///< Corrupted backup files

        // Performance metrics
        double averageBackupTime;                  ///< Average backup time (seconds)
        double averageRestoreTime;                 ///< Average restore time (seconds)
        double minBackupTime;                      ///< Minimum backup time (seconds)
        double maxBackupTime;                      ///< Maximum backup time (seconds)
        double totalBackupSize;                    ///< Total backup data size (bytes)
        double averageCompressionRatio;            ///< Average compression ratio

        // Storage metrics
        size_t localStorageUsed;                   ///< Local storage used (bytes)
        size_t externalStorageUsed;                ///< External storage used (bytes)
        size_t networkStorageUsed;                 ///< Network storage used (bytes)
        size_t cloudStorageUsed;                   ///< Cloud storage used (bytes)

        // Backup type statistics
        uint64_t automaticBackups;                 ///< Number of automatic backups
        uint64_t manualBackups;                    ///< Number of manual backups
        uint64_t emergencyBackups;                 ///< Number of emergency backups
        uint64_t incrementalBackups;               ///< Number of incremental backups

        // Error statistics
        uint64_t storageErrors;                    ///< Storage-related errors
        uint64_t networkErrors;                    ///< Network-related errors
        uint64_t corruptionErrors;                 ///< Corruption-related errors
        uint64_t verificationErrors;               ///< Verification errors

        // Recovery statistics
        uint64_t recoveryAttempts;                 ///< Number of recovery attempts
        uint64_t successfulRecoveries;             ///< Successful recoveries
        uint64_t failedRecoveries;                 ///< Failed recoveries
    };

    /**
     * @struct BackupOperation
     * @brief Backup operation descriptor
     */
    struct BackupOperation {
        uint32_t operationId;                      ///< Unique operation ID
        BackupType type;                           ///< Operation type
        std::string saveFileName;                  ///< Save file to backup
        std::string backupName;                    ///< Backup name
        std::string description;                   ///< Operation description

        // Timing
        double startTime;                          ///< Operation start time
        double endTime;                            ///< Operation end time
        double maxDuration;                        ///< Maximum allowed duration

        // Progress tracking
        float progress;                            ///< Operation progress (0.0 - 1.0)
        std::string currentTask;                   ///< Current task description
        size_t processedData;                      ///< Processed data size
        size_t totalData;                          ///< Total data size

        // Callbacks
        std::function<void(float, const std::string&)> progressCallback; ///< Progress callback
        std::function<void(bool, const std::string&)> completionCallback; ///< Completion callback

        // State
        BackupStatus status;                       ///< Operation status
        std::string errorMessage;                  ///< Error message if failed
        bool canCancel;                            ///< Whether operation can be cancelled

        BackupOperation()
            : operationId(0)
            , type(BackupType::Automatic)
            , startTime(0.0)
            , endTime(0.0)
            , maxDuration(300.0)
            , progress(0.0f)
            , processedData(0)
            , totalData(0)
            , status(BackupStatus::Pending)
            , canCancel(true)
        {}
    };

    /**
     * @class BackupSystem
     * @brief Advanced backup management system
     *
     * The BackupSystem class provides comprehensive backup management for the
     * VoxelCraft game engine, featuring automatic backups, backup rotation,
     * backup verification, restore functionality, and disaster recovery for
     * game save files and critical game data with enterprise-grade reliability
     * and comprehensive backup strategies.
     *
     * Key Features:
     * - Automatic scheduled backups
     * - Multiple backup types (full, incremental, differential)
     * - Backup rotation and retention policies
     * - Backup verification and integrity checking
     * - Multiple storage locations (local, external, network, cloud)
     * - Backup compression and encryption
     * - Disaster recovery and point-in-time restore
     * - Backup monitoring and notifications
     * - Performance optimization and parallel processing
     * - Cross-platform backup compatibility
     *
     * The backup system is designed to ensure data safety and provide
     * comprehensive recovery options for critical game data.
     */
    class BackupSystem {
    public:
        /**
         * @brief Constructor
         * @param config Backup system configuration
         */
        explicit BackupSystem(const BackupConfig& config);

        /**
         * @brief Destructor
         */
        ~BackupSystem();

        /**
         * @brief Deleted copy constructor
         */
        BackupSystem(const BackupSystem&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        BackupSystem& operator=(const BackupSystem&) = delete;

        // Backup system lifecycle

        /**
         * @brief Initialize backup system
         * @return true if successful, false otherwise
         */
        bool Initialize();

        /**
         * @brief Shutdown backup system
         */
        void Shutdown();

        /**
         * @brief Update backup system
         * @param deltaTime Time elapsed since last update
         */
        void Update(double deltaTime);

        /**
         * @brief Get backup configuration
         * @return Current configuration
         */
        const BackupConfig& GetConfig() const { return m_config; }

        /**
         * @brief Set backup configuration
         * @param config New configuration
         * @return true if successful, false otherwise
         */
        bool SetConfig(const BackupConfig& config);

        /**
         * @brief Get backup statistics
         * @return Current statistics
         */
        const BackupStats& GetStats() const { return m_stats; }

        // Backup operations

        /**
         * @brief Create backup operation
         * @param saveFileName Save file to backup
         * @param type Backup type
         * @param description Backup description
         * @return Operation ID or 0 if failed
         */
        uint32_t CreateBackupOperation(const std::string& saveFileName,
                                     BackupType type = BackupType::Automatic,
                                     const std::string& description = "");

        /**
         * @brief Execute backup operation
         * @param operationId Operation ID
         * @return true if successful, false otherwise
         */
        bool ExecuteBackupOperation(uint32_t operationId);

        /**
         * @brief Cancel backup operation
         * @param operationId Operation ID
         * @return true if successful, false otherwise
         */
        bool CancelBackupOperation(uint32_t operationId);

        /**
         * @brief Create automatic backup
         * @param saveFileName Save file to backup
         * @return true if successful, false otherwise
         */
        bool CreateAutomaticBackup(const std::string& saveFileName);

        /**
         * @brief Create manual backup
         * @param saveFileName Save file to backup
         * @param backupName Backup name
         * @param description Backup description
         * @return true if successful, false otherwise
         */
        bool CreateManualBackup(const std::string& saveFileName,
                              const std::string& backupName,
                              const std::string& description = "");

        /**
         * @brief Create emergency backup
         * @param saveFileName Save file to backup
         * @return true if successful, false otherwise
         */
        bool CreateEmergencyBackup(const std::string& saveFileName);

        /**
         * @brief Create incremental backup
         * @param saveFileName Save file to backup
         * @param baseBackupId Base backup ID for incremental
         * @return true if successful, false otherwise
         */
        bool CreateIncrementalBackup(const std::string& saveFileName, uint32_t baseBackupId);

        // Restore operations

        /**
         * @brief Restore from backup
         * @param backupId Backup ID to restore from
         * @param mode Restore mode
         * @return true if successful, false otherwise
         */
        bool RestoreFromBackup(uint32_t backupId, RestoreMode mode = RestoreMode::FullRestore);

        /**
         * @brief Restore to specific point in time
         * @param saveFileName Save file to restore
         * @param timestamp Timestamp to restore to
         * @return true if successful, false otherwise
         */
        bool RestoreToPointInTime(const std::string& saveFileName, const std::string& timestamp);

        /**
         * @brief Rollback to previous version
         * @param saveFileName Save file to rollback
         * @param versions Number of versions to rollback
         * @return true if successful, false otherwise
         */
        bool RollbackToPreviousVersion(const std::string& saveFileName, int versions = 1);

        /**
         * @brief Restore with progress callback
         * @param backupId Backup ID to restore from
         * @param progressCallback Progress callback function
         * @return true if successful, false otherwise
         */
        bool RestoreWithProgress(uint32_t backupId,
                               std::function<void(float, const std::string&)> progressCallback);

        // Backup management

        /**
         * @brief Get list of backups for save file
         * @param saveFileName Save file name
         * @return Vector of backup information
         */
        std::vector<BackupInfo> GetBackupList(const std::string& saveFileName) const;

        /**
         * @brief Get backup information
         * @param backupId Backup ID
         * @return Backup information or nullopt if not found
         */
        std::optional<BackupInfo> GetBackupInfo(uint32_t backupId) const;

        /**
         * @brief Delete backup
         * @param backupId Backup ID
         * @return true if successful, false otherwise
         */
        bool DeleteBackup(uint32_t backupId);

        /**
         * @brief Verify backup integrity
         * @param backupId Backup ID
         * @return true if backup is valid, false otherwise
         */
        bool VerifyBackupIntegrity(uint32_t backupId);

        /**
         * @brief Get backup storage information
         * @param backupId Backup ID
         * @return Storage information
         */
        std::string GetBackupStorageInfo(uint32_t backupId) const;

        // Backup rotation and cleanup

        /**
         * @brief Rotate backups according to policy
         * @param saveFileName Save file name
         * @return Number of backups removed
         */
        int RotateBackups(const std::string& saveFileName);

        /**
         * @brief Cleanup expired backups
         * @return Number of backups removed
         */
        int CleanupExpiredBackups();

        /**
         * @brief Cleanup old backups
         * @param maxBackups Maximum number of backups to keep
         * @return Number of backups removed
         */
        int CleanupOldBackups(int maxBackups = -1);

        /**
         * @brief Get backup retention policy info
         * @param saveFileName Save file name
         * @return Retention policy information
         */
        std::string GetRetentionPolicyInfo(const std::string& saveFileName) const;

        // Storage management

        /**
         * @brief Get storage usage information
         * @return Storage usage by location
         */
        std::unordered_map<BackupStorage, size_t> GetStorageUsage() const;

        /**
         * @brief Optimize backup storage
         * @return Space saved in bytes
         */
        size_t OptimizeBackupStorage();

        /**
         * @brief Check storage availability
         * @param storageType Storage type to check
         * @return Available space in bytes
         */
        size_t CheckStorageAvailability(BackupStorage storageType) const;

        /**
         * @brief Set storage priority
         * @param priorities Storage priority list
         * @return true if successful, false otherwise
         */
        bool SetStoragePriority(const std::vector<BackupStorage>& priorities);

        // Emergency and recovery

        /**
         * @brief Enable emergency backup mode
         * @param enabled Enable state
         * @return true if successful, false otherwise
         */
        bool EnableEmergencyBackupMode(bool enabled);

        /**
         * @brief Trigger emergency backup
         * @param saveFileName Save file to backup
         * @param reason Emergency reason
         * @return true if successful, false otherwise
         */
        bool TriggerEmergencyBackup(const std::string& saveFileName, const std::string& reason);

        /**
         * @brief Start disaster recovery
         * @param saveFileName Save file to recover
         * @return Recovery operation ID or 0 if failed
         */
        uint32_t StartDisasterRecovery(const std::string& saveFileName);

        /**
         * @brief Get recovery status
         * @param operationId Recovery operation ID
         * @return Recovery status information
         */
        std::string GetRecoveryStatus(uint32_t operationId) const;

        // Monitoring and notifications

        /**
         * @brief Enable backup monitoring
         * @param enabled Enable state
         * @return true if successful, false otherwise
         */
        bool EnableBackupMonitoring(bool enabled);

        /**
         * @brief Get backup system health
         * @return Health status report
         */
        std::string GetSystemHealthReport() const;

        /**
         * @brief Get backup failure report
         * @param sinceTime Time since when to report failures
         * @return Failure report
         */
        std::string GetBackupFailureReport(double sinceTime = 86400.0) const;

        // Utility functions

        /**
         * @brief Get backup file path
         * @param backupId Backup ID
         * @return Full backup file path
         */
        std::string GetBackupFilePath(uint32_t backupId) const;

        /**
         * @brief Calculate backup checksum
         * @param backupId Backup ID
         * @return Checksum string
         */
        std::string CalculateBackupChecksum(uint32_t backupId);

        /**
         * @brief Get backup creation time
         * @param backupId Backup ID
         * @return Creation time as string
         */
        std::string GetBackupCreationTime(uint32_t backupId) const;

        /**
         * @brief Check if backup is expired
         * @param backupId Backup ID
         * @return true if expired, false otherwise
         */
        bool IsBackupExpired(uint32_t backupId) const;

        /**
         * @brief Get backup expiration time
         * @param backupId Backup ID
         * @return Expiration time as string
         */
        std::string GetBackupExpirationTime(uint32_t backupId) const;

        /**
         * @brief Validate backup system state
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
         * @brief Optimize backup system
         * @return Optimization results
         */
        std::unordered_map<std::string, size_t> Optimize();

    private:
        /**
         * @brief Initialize backup storage
         * @return true if successful, false otherwise
         */
        bool InitializeBackupStorage();

        /**
         * @brief Initialize backup monitoring
         * @return true if successful, false otherwise
         */
        bool InitializeBackupMonitoring();

        /**
         * @brief Create backup from save file
         * @param operation Backup operation
         * @return true if successful, false otherwise
         */
        bool CreateBackupFromSaveFile(BackupOperation& operation);

        /**
         * @brief Restore save file from backup
         * @param backupId Backup ID
         * @param restorePath Restore path
         * @param mode Restore mode
         * @return true if successful, false otherwise
         */
        bool RestoreSaveFileFromBackup(uint32_t backupId, const std::string& restorePath, RestoreMode mode);

        /**
         * @brief Compress backup data
         * @param data Input data
         * @param compressedData Output compressed data
         * @return true if successful, false otherwise
         */
        bool CompressBackupData(const std::vector<uint8_t>& data, std::vector<uint8_t>& compressedData);

        /**
         * @brief Decompress backup data
         * @param compressedData Input compressed data
         * @param data Output decompressed data
         * @return true if successful, false otherwise
         */
        bool DecompressBackupData(const std::vector<uint8_t>& compressedData, std::vector<uint8_t>& data);

        /**
         * @brief Encrypt backup data
         * @param data Input data
         * @param encryptedData Output encrypted data
         * @return true if successful, false otherwise
         */
        bool EncryptBackupData(const std::vector<uint8_t>& data, std::vector<uint8_t>& encryptedData);

        /**
         * @brief Decrypt backup data
         * @param encryptedData Input encrypted data
         * @param data Output decrypted data
         * @return true if successful, false otherwise
         */
        bool DecryptBackupData(const std::vector<uint8_t>& encryptedData, std::vector<uint8_t>& data);

        /**
         * @brief Update auto-backup timer
         * @param deltaTime Time elapsed
         */
        void UpdateAutoBackupTimer(double deltaTime);

        /**
         * @brief Check backup expiration
         */
        void CheckBackupExpiration();

        /**
         * @brief Clean up completed operations
         */
        void CleanupCompletedOperations();

        /**
         * @brief Update performance statistics
         * @param operationTime Operation time in seconds
         * @param dataSize Data size in bytes
         */
        void UpdateStats(double operationTime, size_t dataSize);

        /**
         * @brief Handle backup errors
         * @param error Error message
         */
        void HandleError(const std::string& error);

        // Backup system data
        BackupConfig m_config;                     ///< Backup system configuration
        BackupStats m_stats;                       ///< Performance statistics

        // Backup operations
        std::unordered_map<uint32_t, BackupOperation> m_backupOperations; ///< Active operations
        std::unordered_map<uint32_t, BackupInfo> m_backupInfo; ///< Backup information
        std::queue<uint32_t> m_operationQueue;     ///< Operation queue
        mutable std::shared_mutex m_operationsMutex; ///< Operations synchronization

        // Auto-backup system
        double m_lastAutoBackupTime;               ///< Last auto-backup time
        double m_autoBackupTimer;                  ///< Auto-backup timer
        bool m_emergencyModeEnabled;               ///< Emergency backup mode
        double m_lastEmergencyBackupTime;          ///< Last emergency backup time

        // Storage management
        std::vector<BackupStorage> m_storagePriority; ///< Storage priority list
        std::unordered_map<BackupStorage, size_t> m_storageUsage; ///< Storage usage by location

        // Monitoring
        bool m_monitoringEnabled;                  ///< Backup monitoring enabled
        std::deque<std::string> m_systemLog;       ///< System log entries
        mutable std::shared_mutex m_monitoringMutex; ///< Monitoring synchronization

        // State tracking
        bool m_isInitialized;                      ///< System is initialized
        double m_lastUpdateTime;                   ///< Last update time
        std::string m_lastError;                   ///< Last error message

        static std::atomic<uint32_t> s_nextOperationId; ///< Next operation ID counter
        static std::atomic<uint32_t> s_nextBackupId; ///< Next backup ID counter
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_SAVE_BACKUP_SYSTEM_HPP
