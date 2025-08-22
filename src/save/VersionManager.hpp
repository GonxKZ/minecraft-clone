/**
 * @file VersionManager.hpp
 * @brief VoxelCraft Advanced Version Management System
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines the VersionManager class that provides comprehensive version
 * management for the VoxelCraft game engine, including schema evolution, data
 * migration, version compatibility checking, upgrade/downgrade functionality,
 * and seamless version transitions for game save files and game data.
 */

#ifndef VOXELCRAFT_SAVE_VERSION_MANAGER_HPP
#define VOXELCRAFT_SAVE_VERSION_MANAGER_HPP

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
#include <variant>
#include <any>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../core/Config.hpp"

namespace VoxelCraft {

    // Forward declarations
    class SaveManager;
    class Serializer;

    /**
     * @enum VersionComparison
     * @brief Version comparison results
     */
    enum class VersionComparison {
        Older,                  ///< Version is older
        Newer,                  ///< Version is newer
        Equal,                  ///< Versions are equal
        Incompatible,           ///< Versions are incompatible
        Unknown                 ///< Version comparison unknown
    };

    /**
     * @enum MigrationDirection
     * @brief Migration direction
     */
    enum class MigrationDirection {
        Upgrade,                ///< Upgrade to newer version
        Downgrade,              ///< Downgrade to older version
        CrossGrade,             ///< Cross-grade to different branch
        Repair                  ///< Repair current version
    };

    /**
     * @enum MigrationStatus
     * @brief Migration operation status
     */
    enum class MigrationStatus {
        Pending,                ///< Migration is pending
        InProgress,             ///< Migration is in progress
        Completed,              ///< Migration completed successfully
        Failed,                 ///< Migration failed
        Cancelled,              ///< Migration was cancelled
        PartiallyCompleted      ///< Migration partially completed
    };

    /**
     * @enum CompatibilityLevel
     * @brief Version compatibility levels
     */
    enum class CompatibilityLevel {
        Full,                   ///< Full compatibility
        Partial,                ///< Partial compatibility (some features may not work)
        Limited,                ///< Limited compatibility (major features disabled)
        None,                   ///< No compatibility
        Unknown                 ///< Compatibility unknown
    };

    /**
     * @struct VersionInfo
     * @brief Version information
     */
    struct VersionInfo {
        uint32_t major;                            ///< Major version number
        uint32_t minor;                            ///< Minor version number
        uint32_t patch;                            ///< Patch version number
        uint32_t build;                            ///< Build number
        std::string branch;                        ///< Branch name
        std::string commitHash;                    ///< Git commit hash
        std::string buildDate;                     ///< Build date
        std::string buildTime;                     ///< Build time

        // Version metadata
        std::string description;                   ///< Version description
        std::vector<std::string> changes;          ///< List of changes
        std::vector<std::string> breakingChanges;  ///< Breaking changes
        std::unordered_map<std::string, std::string> metadata; ///< Additional metadata

        VersionInfo()
            : major(1)
            , minor(0)
            , patch(0)
            , build(0)
        {}

        /**
         * @brief Convert to string
         * @return Version string (major.minor.patch.build)
         */
        std::string toString() const {
            return std::to_string(major) + "." +
                   std::to_string(minor) + "." +
                   std::to_string(patch) + "." +
                   std::to_string(build);
        }

        /**
         * @brief Convert to comparable integer
         * @return Comparable version number
         */
        uint64_t toInt() const {
            return static_cast<uint64_t>(major) << 48 |
                   static_cast<uint64_t>(minor) << 32 |
                   static_cast<uint64_t>(patch) << 16 |
                   static_cast<uint64_t>(build);
        }
    };

    /**
     * @struct VersionCompatibility
     * @brief Version compatibility information
     */
    struct VersionCompatibility {
        VersionInfo sourceVersion;                 ///< Source version
        VersionInfo targetVersion;                 ///< Target version
        CompatibilityLevel level;                  ///< Compatibility level
        std::vector<std::string> issues;           ///< Compatibility issues
        std::vector<std::string> recommendations;  ///< Compatibility recommendations
        bool requiresMigration;                    ///< Whether migration is required
        bool canAutoMigrate;                       ///< Whether automatic migration is possible

        VersionCompatibility()
            : level(CompatibilityLevel::Unknown)
            , requiresMigration(false)
            , canAutoMigrate(false)
        {}
    };

    /**
     * @struct MigrationRule
     * @brief Data migration rule
     */
    struct MigrationRule {
        uint32_t ruleId;                           ///< Unique rule ID
        VersionInfo fromVersion;                   ///< Source version
        VersionInfo toVersion;                     ///< Target version
        MigrationDirection direction;              ///< Migration direction
        std::string description;                   ///< Rule description

        // Migration logic
        std::function<bool(const std::vector<uint8_t>&, std::vector<uint8_t>&)> migrationFunction; ///< Migration function
        std::function<bool(const std::any&)> validationFunction; ///< Validation function

        // Metadata
        bool isBreaking;                           ///< Whether migration is breaking
        bool requiresUserConfirmation;             ///< Whether user confirmation is required
        std::string userWarning;                   ///< Warning message for user
        std::vector<std::string> dependencies;     ///< Migration dependencies

        MigrationRule()
            : ruleId(0)
            , direction(MigrationDirection::Upgrade)
            , isBreaking(false)
            , requiresUserConfirmation(false)
        {}
    };

    /**
     * @struct MigrationOperation
     * @brief Migration operation descriptor
     */
    struct MigrationOperation {
        uint32_t operationId;                      ///< Unique operation ID
        std::string dataName;                      ///< Name of data being migrated
        VersionInfo sourceVersion;                 ///< Source version
        VersionInfo targetVersion;                 ///< Target version
        MigrationDirection direction;              ///< Migration direction
        MigrationStatus status;                    ///< Operation status

        // Timing
        double startTime;                          ///< Operation start time
        double endTime;                            ///< Operation end time
        double duration;                           ///< Operation duration

        // Progress tracking
        float progress;                            ///< Operation progress (0.0 - 1.0)
        std::string currentTask;                   ///< Current task description
        size_t processedItems;                     ///< Processed items
        size_t totalItems;                         ///< Total items to process

        // Results
        std::vector<std::string> warnings;         ///< Warning messages
        std::vector<std::string> errors;           ///< Error messages
        bool success;                              ///< Operation success
        std::string errorMessage;                  ///< Main error message

        // Callbacks
        std::function<void(float, const std::string&)> progressCallback; ///< Progress callback
        std::function<void(bool, const std::string&)> completionCallback; ///< Completion callback

        MigrationOperation()
            : operationId(0)
            , direction(MigrationDirection::Upgrade)
            , status(MigrationStatus::Pending)
            , startTime(0.0)
            , endTime(0.0)
            , duration(0.0)
            , progress(0.0f)
            , processedItems(0)
            , totalItems(0)
            , success(false)
        {}
    };

    /**
     * @struct VersionStats
     * @brief Version management statistics
     */
    struct VersionStats {
        // Migration statistics
        uint64_t totalMigrations;                  ///< Total migrations performed
        uint64_t successfulMigrations;             ///< Successful migrations
        uint64_t failedMigrations;                 ///< Failed migrations
        uint64_t pendingMigrations;                ///< Pending migrations

        // Version statistics
        uint64_t compatibilityChecks;              ///< Compatibility checks performed
        uint64_t versionComparisons;               ///< Version comparisons made
        uint64_t autoMigrations;                   ///< Automatic migrations
        uint64_t manualMigrations;                 ///< Manual migrations

        // Performance metrics
        double averageMigrationTime;               ///< Average migration time (ms)
        double minMigrationTime;                   ///< Minimum migration time (ms)
        double maxMigrationTime;                   ///< Maximum migration time (ms)
        double totalMigrationTime;                 ///< Total migration time (ms)

        // Error statistics
        uint64_t migrationErrors;                  ///< Migration errors
        uint64_t compatibilityErrors;              ///< Compatibility errors
        uint64_t validationErrors;                 ///< Validation errors
        uint64_t rollbackOperations;               ///< Rollback operations

        // Version distribution
        std::unordered_map<std::string, uint64_t> versionUsage; ///< Version usage count
        std::unordered_map<std::string, uint64_t> migrationPaths; ///< Migration path usage
    };

    /**
     * @class VersionManager
     * @brief Advanced version management system
     *
     * The VersionManager class provides comprehensive version management for the
     * VoxelCraft game engine, featuring schema evolution, data migration, version
     * compatibility checking, upgrade/downgrade functionality, and seamless version
     * transitions for game save files and game data with enterprise-grade version
     * management and migration capabilities.
     *
     * Key Features:
     * - Version comparison and compatibility checking
     * - Automatic data migration between versions
     * - Schema evolution and field transformation
     * - Backward and forward compatibility
     * - Migration rollback and recovery
     * - Version branching and merging
     * - Migration dependency management
     * - User-friendly migration warnings
     * - Performance monitoring and optimization
     * - Cross-platform version compatibility
     *
     * The version manager ensures smooth transitions between game versions
     * while maintaining data integrity and user experience.
     */
    class VersionManager {
    public:
        /**
         * @brief Constructor
         * @param currentVersion Current engine version
         */
        explicit VersionManager(const VersionInfo& currentVersion);

        /**
         * @brief Destructor
         */
        ~VersionManager();

        /**
         * @brief Deleted copy constructor
         */
        VersionManager(const VersionManager&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        VersionManager& operator=(const VersionManager&) = delete;

        // Version manager lifecycle

        /**
         * @brief Initialize version manager
         * @return true if successful, false otherwise
         */
        bool Initialize();

        /**
         * @brief Shutdown version manager
         */
        void Shutdown();

        /**
         * @brief Update version manager
         * @param deltaTime Time elapsed since last update
         */
        void Update(double deltaTime);

        /**
         * @brief Get current version
         * @return Current version information
         */
        const VersionInfo& GetCurrentVersion() const { return m_currentVersion; }

        /**
         * @brief Set current version
         * @param version New version
         */
        void SetCurrentVersion(const VersionInfo& version);

        /**
         * @brief Get version statistics
         * @return Current statistics
         */
        const VersionStats& GetStats() const { return m_stats; }

        // Version comparison and compatibility

        /**
         * @brief Compare two versions
         * @param version1 First version
         * @param version2 Second version
         * @return Version comparison result
         */
        VersionComparison CompareVersions(const VersionInfo& version1, const VersionInfo& version2) const;

        /**
         * @brief Check version compatibility
         * @param sourceVersion Source version
         * @param targetVersion Target version (optional, uses current if not specified)
         * @return Compatibility information
         */
        VersionCompatibility CheckCompatibility(const VersionInfo& sourceVersion,
                                              const VersionInfo& targetVersion = VersionInfo()) const;

        /**
         * @brief Check if version is compatible
         * @param version Version to check
         * @return true if compatible, false otherwise
         */
        bool IsVersionCompatible(const VersionInfo& version) const;

        /**
         * @brief Get compatibility level
         * @param version Version to check
         * @return Compatibility level
         */
        CompatibilityLevel GetCompatibilityLevel(const VersionInfo& version) const;

        // Migration rules management

        /**
         * @brief Register migration rule
         * @param rule Migration rule
         * @return Rule ID or 0 if failed
         */
        uint32_t RegisterMigrationRule(const MigrationRule& rule);

        /**
         * @brief Get migration rule
         * @param ruleId Rule ID
         * @return Migration rule or nullopt if not found
         */
        std::optional<MigrationRule> GetMigrationRule(uint32_t ruleId) const;

        /**
         * @brief Remove migration rule
         * @param ruleId Rule ID
         * @return true if successful, false otherwise
         */
        bool RemoveMigrationRule(uint32_t ruleId);

        /**
         * @brief Find migration path
         * @param fromVersion Source version
         * @param toVersion Target version
         * @return Vector of migration rule IDs
         */
        std::vector<uint32_t> FindMigrationPath(const VersionInfo& fromVersion, const VersionInfo& toVersion) const;

        /**
         * @brief Validate migration path
         * @param path Migration path (rule IDs)
         * @return true if path is valid, false otherwise
         */
        bool ValidateMigrationPath(const std::vector<uint32_t>& path) const;

        // Migration operations

        /**
         * @brief Create migration operation
         * @param dataName Name of data to migrate
         * @param fromVersion Source version
         * @param toVersion Target version
         * @param direction Migration direction
         * @return Operation ID or 0 if failed
         */
        uint32_t CreateMigrationOperation(const std::string& dataName,
                                        const VersionInfo& fromVersion,
                                        const VersionInfo& toVersion,
                                        MigrationDirection direction = MigrationDirection::Upgrade);

        /**
         * @brief Execute migration operation
         * @param operationId Operation ID
         * @param data Input data to migrate
         * @return Migrated data or empty vector if failed
         */
        std::vector<uint8_t> ExecuteMigrationOperation(uint32_t operationId, const std::vector<uint8_t>& data);

        /**
         * @brief Cancel migration operation
         * @param operationId Operation ID
         * @return true if successful, false otherwise
         */
        bool CancelMigrationOperation(uint32_t operationId);

        /**
         * @brief Get migration operation status
         * @param operationId Operation ID
         * @return Migration operation or nullopt if not found
         */
        std::optional<MigrationOperation> GetMigrationOperation(uint32_t operationId) const;

        /**
         * @brief Auto-migrate data to current version
         * @param data Input data
         * @param sourceVersion Source version of the data
         * @param dataName Name of the data (for logging)
         * @return Migrated data or empty vector if failed
         */
        std::vector<uint8_t> AutoMigrateData(const std::vector<uint8_t>& data,
                                           const VersionInfo& sourceVersion,
                                           const std::string& dataName = "");

        /**
         * @brief Auto-migrate data with progress callback
         * @param data Input data
         * @param sourceVersion Source version
         * @param progressCallback Progress callback function
         * @param dataName Name of the data
         * @return Migrated data or empty vector if failed
         */
        std::vector<uint8_t> AutoMigrateDataWithProgress(const std::vector<uint8_t>& data,
                                                       const VersionInfo& sourceVersion,
                                                       std::function<void(float, const std::string&)> progressCallback,
                                                       const std::string& dataName = "");

        // Rollback and recovery

        /**
         * @brief Create migration rollback point
         * @param operationId Operation ID
         * @param data Original data before migration
         * @return true if successful, false otherwise
         */
        bool CreateMigrationRollbackPoint(uint32_t operationId, const std::vector<uint8_t>& data);

        /**
         * @brief Rollback migration operation
         * @param operationId Operation ID
         * @return Original data or empty vector if rollback failed
         */
        std::vector<uint8_t> RollbackMigrationOperation(uint32_t operationId);

        /**
         * @brief Check if rollback is possible
         * @param operationId Operation ID
         * @return true if rollback is possible, false otherwise
         */
        bool CanRollbackOperation(uint32_t operationId) const;

        /**
         * @brief Get rollback information
         * @param operationId Operation ID
         * @return Rollback information
         */
        std::string GetRollbackInfo(uint32_t operationId) const;

        // Version management

        /**
         * @brief Register version
         * @param version Version information
         * @return true if successful, false otherwise
         */
        bool RegisterVersion(const VersionInfo& version);

        /**
         * @brief Get version information
         * @param versionString Version string
         * @return Version information or nullopt if not found
         */
        std::optional<VersionInfo> GetVersionInfo(const std::string& versionString) const;

        /**
         * @brief Get version history
         * @return Vector of registered versions
         */
        std::vector<VersionInfo> GetVersionHistory() const;

        /**
         * @brief Set minimum compatible version
         * @param version Minimum compatible version
         * @return true if successful, false otherwise
         */
        bool SetMinimumCompatibleVersion(const VersionInfo& version);

        /**
         * @brief Get minimum compatible version
         * @return Minimum compatible version
         */
        const VersionInfo& GetMinimumCompatibleVersion() const { return m_minimumCompatibleVersion; }

        // Branch management

        /**
         * @brief Create version branch
         * @param branchName Branch name
         * @param baseVersion Base version for the branch
         * @return true if successful, false otherwise
         */
        bool CreateVersionBranch(const std::string& branchName, const VersionInfo& baseVersion);

        /**
         * @brief Merge version branches
         * @param sourceBranch Source branch name
         * @param targetBranch Target branch name
         * @return true if successful, false otherwise
         */
        bool MergeVersionBranches(const std::string& sourceBranch, const std::string& targetBranch);

        /**
         * @brief Get branch information
         * @param branchName Branch name
         * @return Branch version information
         */
        std::vector<VersionInfo> GetBranchInfo(const std::string& branchName) const;

        // Utility functions

        /**
         * @brief Parse version string
         * @param versionString Version string (e.g., "1.2.3.4")
         * @return Version information
         */
        static VersionInfo ParseVersionString(const std::string& versionString);

        /**
         * @brief Validate version info
         * @param version Version to validate
         * @return true if valid, false otherwise
         */
        static bool ValidateVersionInfo(const VersionInfo& version);

        /**
         * @brief Get version difference
         * @param version1 First version
         * @param version2 Second version
         * @return Version difference description
         */
        static std::string GetVersionDifference(const VersionInfo& version1, const VersionInfo& version2);

        /**
         * @brief Check if version is newer
         * @param version Version to check
         * @param reference Reference version (current if not specified)
         * @return true if newer, false otherwise
         */
        bool IsVersionNewer(const VersionInfo& version, const VersionInfo& reference = VersionInfo()) const;

        /**
         * @brief Get migration warnings
         * @param fromVersion Source version
         * @param toVersion Target version
         * @return Vector of warning messages
         */
        std::vector<std::string> GetMigrationWarnings(const VersionInfo& fromVersion,
                                                    const VersionInfo& toVersion) const;

        /**
         * @brief Validate version manager state
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
         * @brief Optimize version manager
         * @return Optimization results
         */
        std::unordered_map<std::string, size_t> Optimize();

    private:
        /**
         * @brief Initialize migration rules
         * @return true if successful, false otherwise
         */
        bool InitializeMigrationRules();

        /**
         * @brief Execute migration rule
         * @param rule Migration rule
         * @param data Input data
         * @return Migrated data or empty vector if failed
         */
        std::vector<uint8_t> ExecuteMigrationRule(const MigrationRule& rule, const std::vector<uint8_t>& data);

        /**
         * @brief Validate migration rule
         * @param rule Migration rule
         * @return true if valid, false otherwise
         */
        bool ValidateMigrationRule(const MigrationRule& rule) const;

        /**
         * @brief Check migration dependencies
         * @param rule Migration rule
         * @return true if dependencies satisfied, false otherwise
         */
        bool CheckMigrationDependencies(const MigrationRule& rule) const;

        /**
         * @brief Update migration operation progress
         * @param operationId Operation ID
         * @param progress Progress (0.0 - 1.0)
         * @param task Current task description
         */
        void UpdateMigrationProgress(uint32_t operationId, float progress, const std::string& task);

        /**
         * @brief Complete migration operation
         * @param operationId Operation ID
         * @param success Operation success
         * @param errorMessage Error message if failed
         */
        void CompleteMigrationOperation(uint32_t operationId, bool success, const std::string& errorMessage = "");

        /**
         * @brief Clean up completed operations
         */
        void CleanupCompletedOperations();

        /**
         * @brief Update performance statistics
         * @param operationTime Operation time in milliseconds
         */
        void UpdateStats(double operationTime);

        /**
         * @brief Handle version manager errors
         * @param error Error message
         */
        void HandleError(const std::string& error);

        // Version manager data
        VersionInfo m_currentVersion;              ///< Current engine version
        VersionInfo m_minimumCompatibleVersion;    ///< Minimum compatible version
        VersionStats m_stats;                      ///< Performance statistics

        // Migration rules
        std::unordered_map<uint32_t, MigrationRule> m_migrationRules; ///< Migration rules
        mutable std::shared_mutex m_rulesMutex;    ///< Rules synchronization

        // Migration operations
        std::unordered_map<uint32_t, MigrationOperation> m_migrationOperations; ///< Active operations
        std::unordered_map<uint32_t, std::vector<uint8_t>> m_rollbackPoints; ///< Rollback data
        mutable std::shared_mutex m_operationsMutex; ///< Operations synchronization

        // Version registry
        std::unordered_map<std::string, VersionInfo> m_versionRegistry; ///< Version registry
        std::unordered_map<std::string, std::vector<VersionInfo>> m_branchRegistry; ///< Branch registry
        mutable std::shared_mutex m_registryMutex; ///< Registry synchronization

        // State tracking
        bool m_isInitialized;                      ///< System is initialized
        double m_lastUpdateTime;                   ///< Last update time
        std::string m_lastError;                   ///< Last error message

        static std::atomic<uint32_t> s_nextRuleId; ///< Next rule ID counter
        static std::atomic<uint32_t> s_nextOperationId; ///< Next operation ID counter
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_SAVE_VERSION_MANAGER_HPP
