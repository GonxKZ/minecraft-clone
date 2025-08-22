/**
 * @file AutoSave.hpp
 * @brief VoxelCraft Automatic Save System
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines the AutoSave class that provides intelligent automatic
 * save functionality for the VoxelCraft game engine, including smart timing,
 * change detection, performance optimization, and seamless integration with
 * the save system for continuous game state preservation.
 */

#ifndef VOXELCRAFT_SAVE_AUTO_SAVE_HPP
#define VOXELCRAFT_SAVE_AUTO_SAVE_HPP

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
#include <queue>
#include <deque>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../core/Config.hpp"
#include "../core/System.hpp"

namespace VoxelCraft {

    // Forward declarations
    class SaveManager;

    /**
     * @enum AutoSaveTrigger
     * @brief Triggers for automatic saves
     */
    enum class AutoSaveTrigger {
        TimeInterval,           ///< Time-based trigger
        PlayerAction,           ///< Player action trigger
        WorldChange,            ///< World change trigger
        PerformanceThreshold,   ///< Performance threshold trigger
        MemoryUsage,            ///< Memory usage trigger
        DistanceTraveled,       ///< Distance traveled trigger
        StoryProgress,          ///< Story progress trigger
        Custom                  ///< Custom trigger
    };

    /**
     * @enum AutoSavePriority
     * @brief Auto-save operation priorities
     */
    enum class AutoSavePriority {
        Low,                    ///< Low priority (can be skipped)
        Normal,                 ///< Normal priority
        High,                   ///< High priority (should not be skipped)
        Critical                ///< Critical priority (must execute)
    };

    /**
     * @enum AutoSaveCondition
     * @brief Conditions that must be met for auto-save
     */
    enum class AutoSaveCondition {
        PlayerIdle,             ///< Player must be idle
        PlayerSafe,             ///< Player must be in safe location
        NoCombat,               ///< No active combat
        LowPerformanceImpact,   ///< Low performance impact
        SufficientDiskSpace,    ///< Sufficient disk space available
        Custom                  ///< Custom condition
    };

    /**
     * @struct AutoSaveRule
     * @brief Auto-save rule definition
     */
    struct AutoSaveRule {
        uint32_t ruleId;                           ///< Unique rule ID
        std::string ruleName;                      ///< Rule name
        AutoSaveTrigger trigger;                   ///< Trigger type
        AutoSavePriority priority;                 ///< Rule priority

        // Timing settings
        float triggerInterval;                     ///< Trigger interval (seconds/minutes)
        float minInterval;                         ///< Minimum interval between saves
        float maxInterval;                         ///< Maximum interval between saves

        // Conditions
        std::vector<AutoSaveCondition> conditions; ///< Required conditions
        std::vector<std::function<bool()>> customConditions; ///< Custom condition functions

        // Save settings
        SaveType saveType;                         ///< Type of save to perform
        std::string saveNameTemplate;              ///< Save name template
        bool includeTimestamp;                     ///< Include timestamp in name
        bool createBackup;                         ///< Create backup after save

        // Performance settings
        float maxSaveTime;                         ///< Maximum save time (seconds)
        bool allowSkipping;                        ///< Allow skipping if conditions not met
        bool notifyPlayer;                         ///< Notify player about auto-save

        AutoSaveRule()
            : ruleId(0)
            , trigger(AutoSaveTrigger::TimeInterval)
            , priority(AutoSavePriority::Normal)
            , triggerInterval(300.0f)  // 5 minutes
            , minInterval(60.0f)       // 1 minute
            , maxInterval(1800.0f)     // 30 minutes
            , saveType(SaveType::IncrementalSave)
            , includeTimestamp(true)
            , createBackup(true)
            , maxSaveTime(30.0f)
            , allowSkipping(true)
            , notifyPlayer(false)
        {}
    };

    /**
     * @struct AutoSaveContext
     * @brief Context information for auto-save decisions
     */
    struct AutoSaveContext {
        // Game state
        bool isPlayerActive;                       ///< Player is actively playing
        bool isPlayerInCombat;                     ///< Player is in combat
        bool isPlayerInDanger;                     ///< Player is in danger
        bool isPlayerInSafeZone;                   ///< Player is in safe zone

        // Performance state
        float currentFPS;                          ///< Current frames per second
        float averageFPS;                          ///< Average FPS over time
        size_t memoryUsage;                        ///< Current memory usage
        float cpuUsage;                            ///< Current CPU usage

        // World state
        uint32_t activeChunks;                     ///< Number of active chunks
        uint32_t loadedEntities;                   ///< Number of loaded entities
        uint32_t unsavedChanges;                   ///< Number of unsaved changes
        float worldChangeRate;                     ///< Rate of world changes

        // Player state
        glm::vec3 playerPosition;                  ///< Current player position
        float playerVelocity;                      ///< Player velocity
        float distanceSinceLastSave;               ///< Distance traveled since last save
        uint32_t playerActionsSinceLastSave;       ///< Player actions since last save

        // System state
        size_t availableDiskSpace;                 ///< Available disk space
        float diskWriteSpeed;                      ///< Disk write speed
        bool isSystemUnderLoad;                    ///< System is under load

        AutoSaveContext()
            : isPlayerActive(true)
            , isPlayerInCombat(false)
            , isPlayerInDanger(false)
            , isPlayerInSafeZone(false)
            , currentFPS(60.0f)
            , averageFPS(60.0f)
            , memoryUsage(0)
            , cpuUsage(0.0f)
            , activeChunks(0)
            , loadedEntities(0)
            , unsavedChanges(0)
            , worldChangeRate(0.0f)
            , playerPosition(0.0f, 0.0f, 0.0f)
            , playerVelocity(0.0f)
            , distanceSinceLastSave(0.0f)
            , playerActionsSinceLastSave(0)
            , availableDiskSpace(0)
            , diskWriteSpeed(0.0f)
            , isSystemUnderLoad(false)
        {}
    };

    /**
     * @struct AutoSaveOperation
     * @brief Auto-save operation descriptor
     */
    struct AutoSaveOperation {
        uint32_t operationId;                      ///< Unique operation ID
        uint32_t ruleId;                           ///< Triggering rule ID
        std::string saveName;                      ///< Generated save name
        AutoSavePriority priority;                 ///< Operation priority

        // Timing
        double triggerTime;                        ///< Time when triggered
        double startTime;                          ///< Operation start time
        double endTime;                            ///< Operation end time
        float estimatedDuration;                   ///< Estimated duration

        // Context
        AutoSaveContext context;                   ///< Context when triggered
        std::vector<AutoSaveCondition> conditionsMet; ///< Conditions that were met
        std::vector<AutoSaveCondition> conditionsFailed; ///< Conditions that failed

        // Progress tracking
        float progress;                            ///< Operation progress (0.0 - 1.0)
        std::string currentTask;                   ///< Current task description
        bool isRunning;                            ///< Operation is running
        bool wasSkipped;                           ///< Operation was skipped

        // Results
        bool success;                              ///< Operation success
        std::string errorMessage;                  ///< Error message if failed
        double actualDuration;                     ///< Actual duration
        size_t dataSaved;                          ///< Data saved (bytes)

        AutoSaveOperation()
            : operationId(0)
            , ruleId(0)
            , priority(AutoSavePriority::Normal)
            , triggerTime(0.0)
            , startTime(0.0)
            , endTime(0.0)
            , estimatedDuration(0.0f)
            , progress(0.0f)
            , isRunning(false)
            , wasSkipped(false)
            , success(false)
            , actualDuration(0.0)
            , dataSaved(0)
        {}
    };

    /**
     * @struct AutoSaveStats
     * @brief Auto-save system performance statistics
     */
    struct AutoSaveStats {
        // Operation statistics
        uint64_t totalAutoSaves;                   ///< Total auto-save operations
        uint64_t successfulAutoSaves;              ///< Successful auto-saves
        uint64_t failedAutoSaves;                  ///< Failed auto-saves
        uint64_t skippedAutoSaves;                 ///< Skipped auto-saves

        // Performance metrics
        double averageSaveTime;                    ///< Average save time (ms)
        double minSaveTime;                        ///< Minimum save time (ms)
        double maxSaveTime;                        ///< Maximum save time (ms)
        double averageInterval;                    ///< Average interval between saves

        // Trigger statistics
        uint64_t timeBasedTriggers;                ///< Time-based triggers
        uint64_t actionBasedTriggers;              ///< Action-based triggers
        uint64_t performanceBasedTriggers;         ///< Performance-based triggers
        uint64_t customTriggers;                   ///< Custom triggers

        // Condition statistics
        uint64_t conditionsChecked;                ///< Conditions checked
        uint64_t conditionsPassed;                 ///< Conditions passed
        uint64_t conditionsFailed;                 ///< Conditions failed
        uint64_t ruleActivations;                  ///< Rule activations

        // Impact statistics
        double averagePerformanceImpact;           ///< Average performance impact
        uint64_t savesDuringCombat;                ///< Saves during combat
        uint64_t savesDuringHighLoad;              ///< Saves during high system load
        uint64_t playerNotifications;              ///< Player notifications sent

        // Storage statistics
        uint64_t totalDataAutoSaved;               ///< Total data auto-saved (bytes)
        uint64_t averageSaveSize;                  ///< Average save size (bytes)
        uint32_t autoSaveFilesCreated;             ///< Auto-save files created
    };

    /**
     * @class AutoSave
     * @brief Intelligent automatic save system
     *
     * The AutoSave class provides intelligent automatic save functionality for the
     * VoxelCraft game engine, including smart timing, change detection, performance
     * optimization, and seamless integration with the save system for continuous
     * game state preservation with minimal impact on player experience.
     *
     * Key Features:
     * - Intelligent timing based on game state and player activity
     * - Multiple trigger types (time, action, performance, distance)
     * - Smart condition checking to avoid saves at bad times
     * - Change detection to only save when necessary
     * - Performance monitoring to minimize game impact
     * - Adaptive intervals based on game intensity
     * - Player awareness and notification system
     * - Emergency save system for critical situations
     * - Integration with backup and version management
     * - Performance optimization and resource management
     *
     * The auto-save system ensures continuous progress preservation
     * while maintaining optimal game performance and player experience.
     */
    class AutoSave : public System {
    public:
        /**
         * @brief Constructor
         * @param saveManager Save manager instance
         */
        explicit AutoSave(SaveManager* saveManager);

        /**
         * @brief Destructor
         */
        ~AutoSave();

        /**
         * @brief Deleted copy constructor
         */
        AutoSave(const AutoSave&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        AutoSave& operator=(const AutoSave&) = delete;

        // Auto-save lifecycle

        /**
         * @brief Initialize auto-save system
         * @return true if successful, false otherwise
         */
        bool Initialize();

        /**
         * @brief Shutdown auto-save system
         */
        void Shutdown();

        /**
         * @brief Update auto-save system
         * @param deltaTime Time elapsed since last update
         */
        void Update(double deltaTime);

        /**
         * @brief Get auto-save statistics
         * @return Current statistics
         */
        const AutoSaveStats& GetStats() const { return m_stats; }

        // Auto-save rules management

        /**
         * @brief Add auto-save rule
         * @param rule Auto-save rule
         * @return Rule ID or 0 if failed
         */
        uint32_t AddAutoSaveRule(const AutoSaveRule& rule);

        /**
         * @brief Remove auto-save rule
         * @param ruleId Rule ID
         * @return true if successful, false otherwise
         */
        bool RemoveAutoSaveRule(uint32_t ruleId);

        /**
         * @brief Get auto-save rule
         * @param ruleId Rule ID
         * @return Auto-save rule or nullopt if not found
         */
        std::optional<AutoSaveRule> GetAutoSaveRule(uint32_t ruleId) const;

        /**
         * @brief Enable auto-save rule
         * @param ruleId Rule ID
         * @param enabled Enable state
         * @return true if successful, false otherwise
         */
        bool EnableAutoSaveRule(uint32_t ruleId, bool enabled);

        /**
         * @brief Set rule trigger interval
         * @param ruleId Rule ID
         * @param interval Interval in seconds
         * @return true if successful, false otherwise
         */
        bool SetRuleTriggerInterval(uint32_t ruleId, float interval);

        // Context and condition management

        /**
         * @brief Update auto-save context
         * @param context New context information
         */
        void UpdateContext(const AutoSaveContext& context);

        /**
         * @brief Get current context
         * @return Current auto-save context
         */
        const AutoSaveContext& GetContext() const { return m_context; }

        /**
         * @brief Check auto-save conditions
         * @param conditions Conditions to check
         * @return true if all conditions met, false otherwise
         */
        bool CheckConditions(const std::vector<AutoSaveCondition>& conditions) const;

        /**
         * @brief Add custom condition function
         * @param name Condition name
         * @param condition Condition function
         * @return true if successful, false otherwise
         */
        bool AddCustomCondition(const std::string& name, std::function<bool()> condition);

        /**
         * @brief Remove custom condition
         * @param name Condition name
         * @return true if successful, false otherwise
         */
        bool RemoveCustomCondition(const std::string& name);

        // Auto-save operations

        /**
         * @brief Trigger auto-save
         * @param ruleId Rule ID that triggered the save
         * @return Operation ID or 0 if failed
         */
        uint32_t TriggerAutoSave(uint32_t ruleId);

        /**
         * @brief Force immediate auto-save
         * @param reason Reason for forced save
         * @return Operation ID or 0 if failed
         */
        uint32_t ForceAutoSave(const std::string& reason = "");

        /**
         * @brief Cancel auto-save operation
         * @param operationId Operation ID
         * @return true if successful, false otherwise
         */
        bool CancelAutoSaveOperation(uint32_t operationId);

        /**
         * @brief Get auto-save operation
         * @param operationId Operation ID
         * @return Auto-save operation or nullopt if not found
         */
        std::optional<AutoSaveOperation> GetAutoSaveOperation(uint32_t operationId) const;

        /**
         * @brief Get pending auto-save operations
         * @return Vector of pending operation IDs
         */
        std::vector<uint32_t> GetPendingOperations() const;

        /**
         * @brief Get active auto-save operations
         * @return Vector of active operation IDs
         */
        std::vector<uint32_t> GetActiveOperations() const;

        // Trigger management

        /**
         * @brief Register trigger callback
         * @param trigger Trigger type
         * @param callback Callback function
         * @return true if successful, false otherwise
         */
        bool RegisterTriggerCallback(AutoSaveTrigger trigger,
                                   std::function<void(uint32_t)> callback);

        /**
         * @brief Unregister trigger callback
         * @param trigger Trigger type
         * @return true if successful, false otherwise
         */
        bool UnregisterTriggerCallback(AutoSaveTrigger trigger);

        /**
         * @brief Trigger custom event
         * @param eventName Event name
         * @param eventData Event data
         * @return true if successful, false otherwise
         */
        bool TriggerCustomEvent(const std::string& eventName,
                              const std::unordered_map<std::string, std::any>& eventData = {});

        // Performance and optimization

        /**
         * @brief Set performance threshold
         * @param threshold Performance threshold (0.0 - 1.0)
         * @return true if successful, false otherwise
         */
        bool SetPerformanceThreshold(float threshold);

        /**
         * @brief Set memory threshold
         * @param threshold Memory threshold in MB
         * @return true if successful, false otherwise
         */
        bool SetMemoryThreshold(size_t threshold);

        /**
         * @brief Enable adaptive timing
         * @param enabled Enable state
         * @return true if successful, false otherwise
         */
        bool EnableAdaptiveTiming(bool enabled);

        /**
         * @brief Set maximum save frequency
         * @param frequency Maximum saves per minute
         * @return true if successful, false otherwise
         */
        bool SetMaximumSaveFrequency(float frequency);

        // Player interaction

        /**
         * @brief Notify player about auto-save
         * @param message Notification message
         * @param duration Display duration (seconds)
         * @return true if successful, false otherwise
         */
        bool NotifyPlayer(const std::string& message, float duration = 3.0f);

        /**
         * @brief Enable player notifications
         * @param enabled Enable state
         * @return true if successful, false otherwise
         */
        bool EnablePlayerNotifications(bool enabled);

        /**
         * @brief Set notification preferences
         * @param showProgress Show progress notifications
         * @param showCompletion Show completion notifications
         * @param showErrors Show error notifications
         * @return true if successful, false otherwise
         */
        bool SetNotificationPreferences(bool showProgress, bool showCompletion, bool showErrors);

        // Emergency save system

        /**
         * @brief Enable emergency save system
         * @param enabled Enable state
         * @return true if successful, false otherwise
         */
        bool EnableEmergencySave(bool enabled);

        /**
         * @brief Trigger emergency save
         * @param reason Emergency reason
         * @return true if successful, false otherwise
         */
        bool TriggerEmergencySave(const std::string& reason);

        /**
         * @brief Set emergency save conditions
         * @param conditions Emergency conditions
         * @return true if successful, false otherwise
         */
        bool SetEmergencySaveConditions(const std::vector<std::string>& conditions);

        // Utility functions

        /**
         * @brief Get auto-save rule names
         * @return Vector of rule names
         */
        std::vector<std::string> GetAutoSaveRuleNames() const;

        /**
         * @brief Get rule statistics
         * @param ruleId Rule ID
         * @return Rule statistics
         */
        std::unordered_map<std::string, uint64_t> GetRuleStatistics(uint32_t ruleId) const;

        /**
         * @brief Export auto-save configuration
         * @param filePath Export file path
         * @return true if successful, false otherwise
         */
        bool ExportConfiguration(const std::string& filePath) const;

        /**
         * @brief Import auto-save configuration
         * @param filePath Import file path
         * @return true if successful, false otherwise
         */
        bool ImportConfiguration(const std::string& filePath);

        /**
         * @brief Reset auto-save statistics
         * @return true if successful, false otherwise
         */
        bool ResetStatistics();

        /**
         * @brief Validate auto-save system
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
         * @brief Optimize auto-save system
         * @return Optimization results
         */
        std::unordered_map<std::string, size_t> Optimize();

    private:
        /**
         * @brief Initialize default auto-save rules
         * @return true if successful, false otherwise
         */
        bool InitializeDefaultRules();

        /**
         * @brief Update rule timers
         * @param deltaTime Time elapsed
         */
        void UpdateRuleTimers(double deltaTime);

        /**
         * @brief Check rule conditions
         * @param rule Rule to check
         * @return true if conditions met, false otherwise
         */
        bool CheckRuleConditions(const AutoSaveRule& rule) const;

        /**
         * @brief Execute auto-save rule
         * @param rule Rule to execute
         * @return Operation ID or 0 if failed
         */
        uint32_t ExecuteAutoSaveRule(const AutoSaveRule& rule);

        /**
         * @brief Generate save name from template
         * @param templateStr Template string
         * @param rule Rule information
         * @return Generated save name
         */
        std::string GenerateSaveName(const std::string& templateStr, const AutoSaveRule& rule) const;

        /**
         * @brief Check system conditions
         * @param conditions Conditions to check
         * @return Vector of failed conditions
         */
        std::vector<AutoSaveCondition> CheckSystemConditions(const std::vector<AutoSaveCondition>& conditions) const;

        /**
         * @brief Update auto-save operation progress
         * @param operationId Operation ID
         * @param progress Progress (0.0 - 1.0)
         * @param task Current task
         */
        void UpdateOperationProgress(uint32_t operationId, float progress, const std::string& task);

        /**
         * @brief Complete auto-save operation
         * @param operationId Operation ID
         * @param success Operation success
         * @param errorMessage Error message if failed
         */
        void CompleteAutoSaveOperation(uint32_t operationId, bool success, const std::string& errorMessage = "");

        /**
         * @brief Clean up completed operations
         */
        void CleanupCompletedOperations();

        /**
         * @brief Check emergency conditions
         * @return true if emergency conditions met, false otherwise
         */
        bool CheckEmergencyConditions() const;

        /**
         * @brief Update performance statistics
         * @param operationTime Operation time in milliseconds
         */
        void UpdateStats(double operationTime);

        /**
         * @brief Handle auto-save errors
         * @param error Error message
         */
        void HandleError(const std::string& error);

        // Auto-save data
        SaveManager* m_saveManager;                 ///< Save manager instance
        AutoSaveStats m_stats;                      ///< Performance statistics

        // Context and state
        AutoSaveContext m_context;                  ///< Current context
        mutable std::shared_mutex m_contextMutex;   ///< Context synchronization

        // Rules and operations
        std::unordered_map<uint32_t, AutoSaveRule> m_autoSaveRules; ///< Auto-save rules
        std::unordered_map<uint32_t, AutoSaveOperation> m_operations; ///< Active operations
        mutable std::shared_mutex m_rulesMutex;     ///< Rules synchronization

        // Timers and triggers
        std::unordered_map<uint32_t, double> m_ruleTimers; ///< Rule timers
        std::unordered_map<AutoSaveTrigger, std::vector<std::function<void(uint32_t)>>> m_triggerCallbacks; ///< Trigger callbacks
        mutable std::shared_mutex m_timersMutex;    ///< Timers synchronization

        // Custom conditions
        std::unordered_map<std::string, std::function<bool()>> m_customConditions; ///< Custom condition functions
        mutable std::shared_mutex m_conditionsMutex; ///< Conditions synchronization

        // Performance tracking
        double m_lastContextUpdate;                 ///< Last context update
        double m_lastEmergencyCheck;                ///< Last emergency check
        float m_performanceThreshold;               ///< Performance threshold
        size_t m_memoryThreshold;                   ///< Memory threshold
        bool m_adaptiveTimingEnabled;               ///< Adaptive timing enabled
        float m_maxSaveFrequency;                   ///< Maximum save frequency

        // Emergency system
        bool m_emergencySaveEnabled;                ///< Emergency save enabled
        std::vector<std::string> m_emergencyConditions; ///< Emergency conditions
        double m_lastEmergencySave;                 ///< Last emergency save time

        // Player interaction
        bool m_playerNotificationsEnabled;          ///< Player notifications enabled
        bool m_showProgressNotifications;           ///< Show progress notifications
        bool m_showCompletionNotifications;        ///< Show completion notifications
        bool m_showErrorNotifications;              ///< Show error notifications

        // State tracking
        bool m_isInitialized;                       ///< System is initialized
        double m_lastUpdateTime;                    ///< Last update time
        std::string m_lastError;                    ///< Last error message

        static std::atomic<uint32_t> s_nextRuleId;  ///< Next rule ID counter
        static std::atomic<uint32_t> s_nextOperationId; ///< Next operation ID counter
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_SAVE_AUTO_SAVE_HPP
