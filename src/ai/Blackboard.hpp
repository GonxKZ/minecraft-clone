/**
 * @file Blackboard.hpp
 * @brief VoxelCraft AI Blackboard System
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines the Blackboard system that provides a centralized
 * communication and data-sharing mechanism for AI systems, allowing
 * different AI components to share information and coordinate behavior.
 */

#ifndef VOXELCRAFT_AI_BLACKBOARD_HPP
#define VOXELCRAFT_AI_BLACKBOARD_HPP

#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <any>
#include <chrono>
#include <variant>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../core/Config.hpp"

namespace VoxelCraft {

    // Forward declarations
    class Mob;

    /**
     * @enum BlackboardValueType
     * @brief Types of values that can be stored in the blackboard
     */
    enum class BlackboardValueType {
        Integer,
        Float,
        Boolean,
        String,
        Vector3,
        Quaternion,
        Entity,
        Custom
    };

    /**
     * @enum BlackboardEntryFlags
     * @brief Flags for blackboard entries
     */
    enum class BlackboardEntryFlags : uint32_t {
        NONE                    = 0x0000,
        PERSISTENT              = 0x0001,      ///< Entry persists across AI updates
        VOLATILE                = 0x0002,      ///< Entry is cleared after use
        READ_ONLY               = 0x0004,      ///< Entry can only be read, not modified
        NOTIFY_ON_CHANGE        = 0x0008,      ///< Notify listeners when value changes
        AUTO_EXPIRE             = 0x0010,      ///< Entry expires automatically
        SHARED                  = 0x0020,      ///< Entry is shared across multiple AIs
        DEBUG_VISIBLE           = 0x0040       ///< Entry is visible in debug mode
    };

    /**
     * @struct BlackboardEntry
     * @brief A single entry in the blackboard
     */
    struct BlackboardEntry {
        std::string key;                       ///< Entry key
        std::any value;                        ///< Entry value
        BlackboardValueType type;              ///< Value type
        uint32_t flags;                        ///< Entry flags
        double timestamp;                       ///< Creation timestamp
        double expirationTime;                 ///< Expiration timestamp
        std::string description;               ///< Entry description
        std::unordered_map<std::string, std::any> metadata; ///< Additional metadata

        BlackboardEntry()
            : flags(0)
            , timestamp(0.0)
            , expirationTime(0.0)
        {}
    };

    /**
     * @struct BlackboardEvent
     * @brief Event triggered by blackboard changes
     */
    struct BlackboardEvent {
        std::string eventType;                 ///< Type of event
        std::string key;                       ///< Key that triggered the event
        std::any oldValue;                     ///< Previous value
        std::any newValue;                     ///< New value
        double timestamp;                      ///< Event timestamp
        std::string source;                    ///< Event source
        std::unordered_map<std::string, std::any> eventData; ///< Additional event data
    };

    /**
     * @struct BlackboardConfig
     * @brief Configuration for blackboard system
     */
    struct BlackboardConfig {
        // Basic settings
        size_t maxEntries;                     ///< Maximum number of entries
        double defaultExpirationTime;          ///< Default expiration time (seconds)
        bool enableEvents;                     ///< Enable event system
        bool enableTypeChecking;               ///< Enable strict type checking
        bool enableCompression;                ///< Enable data compression
        bool enableSerialization;              ///< Enable data serialization

        // Performance settings
        size_t initialCapacity;                ///< Initial capacity for hash map
        float loadFactor;                      ///< Load factor for hash map
        size_t maxEventQueueSize;              ///< Maximum event queue size

        // Memory settings
        size_t maxMemoryUsage;                 ///< Maximum memory usage (bytes)
        double cleanupInterval;                ///< Cleanup interval (seconds)
        bool enableMemoryTracking;             ///< Enable memory usage tracking

        // Debug settings
        bool enableDebugLogging;               ///< Enable debug logging
        bool enablePerformanceLogging;         ///< Enable performance logging
        std::string logPrefix;                 ///< Log message prefix
    };

    /**
     * @struct BlackboardMetrics
     * @brief Performance metrics for blackboard system
     */
    struct BlackboardMetrics {
        // Performance metrics
        uint64_t operationsCount;              ///< Total operations performed
        double totalOperationTime;             ///< Total operation time (ms)
        double averageOperationTime;           ///< Average operation time (ms)
        double maxOperationTime;               ///< Maximum operation time (ms)

        // Entry metrics
        uint32_t currentEntries;               ///< Current number of entries
        uint32_t maxEntries;                   ///< Maximum entries reached
        uint64_t entriesCreated;               ///< Total entries created
        uint64_t entriesRemoved;               ///< Total entries removed
        uint64_t entriesExpired;               ///< Total entries expired

        // Event metrics
        uint64_t eventsGenerated;              ///< Total events generated
        uint64_t eventsProcessed;              ///< Total events processed
        uint64_t eventQueueSize;               ///< Current event queue size
        uint64_t maxEventQueueSize;            ///< Maximum event queue size

        // Memory metrics
        size_t currentMemoryUsage;             ///< Current memory usage
        size_t peakMemoryUsage;                ///< Peak memory usage
        uint64_t allocations;                  ///< Memory allocations
        uint64_t deallocations;                ///< Memory deallocations

        // Cache metrics
        uint64_t cacheHits;                    ///< Cache hits
        uint64_t cacheMisses;                  ///< Cache misses
        float cacheHitRate;                    ///< Cache hit rate (0.0 - 1.0)
        uint32_t cachedEntries;                ///< Number of cached entries

        // Error metrics
        uint64_t typeErrors;                   ///< Type checking errors
        uint64_t accessErrors;                 ///< Access errors
        uint64_t serializationErrors;          ///< Serialization errors
        uint64_t memoryErrors;                 ///< Memory-related errors
    };

    /**
     * @class Blackboard
     * @brief Centralized data sharing system for AI components
     *
     * The Blackboard class provides a centralized communication and data-sharing
     * mechanism for AI systems. It allows different AI components (behavior trees,
     * sensors, decision systems, etc.) to share information and coordinate behavior
     * in a structured and efficient manner.
     *
     * Key Features:
     * - Type-safe data storage and retrieval
     * - Automatic data expiration and cleanup
     * - Event-driven notifications for data changes
     * - Memory management and optimization
     * - Serialization and persistence support
     * - Debug visualization and monitoring
     * - Thread-safe operations
     *
     * The blackboard follows a key-value store pattern where keys are strings
     * and values can be of any type, with optional type checking and metadata.
     */
    class Blackboard {
    public:
        /**
         * @brief Constructor
         * @param config Blackboard configuration
         */
        explicit Blackboard(const BlackboardConfig& config);

        /**
         * @brief Destructor
         */
        ~Blackboard();

        /**
         * @brief Deleted copy constructor
         */
        Blackboard(const Blackboard&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        Blackboard& operator=(const Blackboard&) = delete;

        // Blackboard lifecycle

        /**
         * @brief Initialize blackboard
         * @param owner Owning entity
         * @return true if successful, false otherwise
         */
        bool Initialize(Mob* owner = nullptr);

        /**
         * @brief Shutdown blackboard
         */
        void Shutdown();

        /**
         * @brief Update blackboard
         * @param deltaTime Time elapsed since last update
         */
        void Update(double deltaTime);

        /**
         * @brief Clear all entries
         */
        void Clear();

        // Data access methods

        /**
         * @brief Set a value in the blackboard
         * @tparam T Value type
         * @param key Entry key
         * @param value Value to set
         * @param flags Entry flags
         * @param expirationTime Expiration time (0.0 for no expiration)
         * @return true if set successfully, false otherwise
         */
        template<typename T>
        bool SetValue(const std::string& key, const T& value,
                     uint32_t flags = 0, double expirationTime = 0.0);

        /**
         * @brief Get a value from the blackboard
         * @tparam T Expected value type
         * @param key Entry key
         * @param defaultValue Default value if key not found
         * @return Value or default value
         */
        template<typename T>
        T GetValue(const std::string& key, const T& defaultValue = T{}) const;

        /**
         * @brief Check if a key exists in the blackboard
         * @param key Entry key
         * @return true if exists, false otherwise
         */
        bool HasValue(const std::string& key) const;

        /**
         * @brief Remove a value from the blackboard
         * @param key Entry key
         * @return true if removed, false otherwise
         */
        bool RemoveValue(const std::string& key);

        /**
         * @brief Get entry information
         * @param key Entry key
         * @return Entry information or nullptr if not found
         */
        const BlackboardEntry* GetEntry(const std::string& key) const;

        // Bulk operations

        /**
         * @brief Set multiple values at once
         * @param values Map of key-value pairs
         * @return Number of values set successfully
         */
        size_t SetValues(const std::unordered_map<std::string, std::any>& values);

        /**
         * @brief Get multiple values at once
         * @param keys Vector of keys to retrieve
         * @return Map of found values
         */
        std::unordered_map<std::string, std::any> GetValues(const std::vector<std::string>& keys) const;

        /**
         * @brief Remove multiple values at once
         * @param keys Vector of keys to remove
         * @return Number of values removed
         */
        size_t RemoveValues(const std::vector<std::string>& keys);

        // Type-specific access methods

        /**
         * @brief Set integer value
         * @param key Entry key
         * @param value Integer value
         * @param flags Entry flags
         * @return true if set successfully, false otherwise
         */
        bool SetInt(const std::string& key, int32_t value, uint32_t flags = 0);

        /**
         * @brief Get integer value
         * @param key Entry key
         * @param defaultValue Default value
         * @return Integer value or default
         */
        int32_t GetInt(const std::string& key, int32_t defaultValue = 0) const;

        /**
         * @brief Set float value
         * @param key Entry key
         * @param value Float value
         * @param flags Entry flags
         * @return true if set successfully, false otherwise
         */
        bool SetFloat(const std::string& key, float value, uint32_t flags = 0);

        /**
         * @brief Get float value
         * @param key Entry key
         * @param defaultValue Default value
         * @return Float value or default
         */
        float GetFloat(const std::string& key, float defaultValue = 0.0f) const;

        /**
         * @brief Set boolean value
         * @param key Entry key
         * @param value Boolean value
         * @param flags Entry flags
         * @return true if set successfully, false otherwise
         */
        bool SetBool(const std::string& key, bool value, uint32_t flags = 0);

        /**
         * @brief Get boolean value
         * @param key Entry key
         * @param defaultValue Default value
         * @return Boolean value or default
         */
        bool GetBool(const std::string& key, bool defaultValue = false) const;

        /**
         * @brief Set string value
         * @param key Entry key
         * @param value String value
         * @param flags Entry flags
         * @return true if set successfully, false otherwise
         */
        bool SetString(const std::string& key, const std::string& value, uint32_t flags = 0);

        /**
         * @brief Get string value
         * @param key Entry key
         * @param defaultValue Default value
         * @return String value or default
         */
        std::string GetString(const std::string& key, const std::string& defaultValue = "") const;

        /**
         * @brief Set vector3 value
         * @param key Entry key
         * @param value Vector3 value
         * @param flags Entry flags
         * @return true if set successfully, false otherwise
         */
        bool SetVector3(const std::string& key, const glm::vec3& value, uint32_t flags = 0);

        /**
         * @brief Get vector3 value
         * @param key Entry key
         * @param defaultValue Default value
         * @return Vector3 value or default
         */
        glm::vec3 GetVector3(const std::string& key, const glm::vec3& defaultValue = glm::vec3(0.0f)) const;

        // Entry management

        /**
         * @brief Get all entry keys
         * @return Vector of all keys
         */
        std::vector<std::string> GetAllKeys() const;

        /**
         * @brief Get all entries
         * @return Vector of all entries
         */
        std::vector<BlackboardEntry> GetAllEntries() const;

        /**
         * @brief Get entries by type
         * @param type Value type
         * @return Vector of entries of specified type
         */
        std::vector<BlackboardEntry> GetEntriesByType(BlackboardValueType type) const;

        /**
         * @brief Get entries by flags
         * @param flags Entry flags to match
         * @return Vector of entries with matching flags
         */
        std::vector<BlackboardEntry> GetEntriesByFlags(uint32_t flags) const;

        /**
         * @brief Expire entries based on time
         * @param currentTime Current time
         * @return Number of entries expired
         */
        size_t ExpireEntries(double currentTime);

        // Event system

        /**
         * @brief Add change listener
         * @param key Entry key to listen for
         * @param listener Listener function
         * @return Listener ID
         */
        uint32_t AddChangeListener(const std::string& key,
                                  std::function<void(const BlackboardEvent&)> listener);

        /**
         * @brief Add global change listener
         * @param listener Listener function
         * @return Listener ID
         */
        uint32_t AddGlobalChangeListener(std::function<void(const BlackboardEvent&)> listener);

        /**
         * @brief Remove change listener
         * @param listenerId Listener ID
         * @return true if removed, false otherwise
         */
        bool RemoveChangeListener(uint32_t listenerId);

        /**
         * @brief Get pending events
         * @return Vector of pending events
         */
        std::vector<BlackboardEvent> GetPendingEvents();

        /**
         * @brief Clear event queue
         */
        void ClearEventQueue();

        // Configuration

        /**
         * @brief Get blackboard configuration
         * @return Current configuration
         */
        const BlackboardConfig& GetConfig() const { return m_config; }

        /**
         * @brief Set blackboard configuration
         * @param config New configuration
         */
        void SetConfig(const BlackboardConfig& config);

        // Metrics and monitoring

        /**
         * @brief Get blackboard metrics
         * @return Performance metrics
         */
        const BlackboardMetrics& GetMetrics() const { return m_metrics; }

        /**
         * @brief Reset metrics
         */
        void ResetMetrics();

        /**
         * @brief Get memory usage
         * @return Memory usage in bytes
         */
        size_t GetMemoryUsage() const;

        // Serialization

        /**
         * @brief Serialize blackboard to string
         * @return Serialized data
         */
        std::string Serialize() const;

        /**
         * @brief Deserialize blackboard from string
         * @param data Serialized data
         * @return true if successful, false otherwise
         */
        bool Deserialize(const std::string& data);

        /**
         * @brief Export entries to JSON-like format
         * @return Formatted data
         */
        std::string ExportToString() const;

        // Utility functions

        /**
         * @brief Validate blackboard state
         * @return true if valid, false otherwise
         */
        bool Validate() const;

        /**
         * @brief Get blackboard status report
         * @return Detailed status report
         */
        std::string GetStatusReport() const;

        /**
         * @brief Optimize blackboard performance
         * @return Optimization results
         */
        std::unordered_map<std::string, size_t> Optimize();

        /**
         * @brief Get blackboard owner
         * @return Owner mob
         */
        Mob* GetOwner() const { return m_owner; }

        /**
         * @brief Set blackboard owner
         * @param owner New owner
         */
        void SetOwner(Mob* owner) { m_owner = owner; }

    private:
        /**
         * @brief Create blackboard entry
         * @tparam T Value type
         * @param key Entry key
         * @param value Entry value
         * @param flags Entry flags
         * @param expirationTime Expiration time
         * @return Created entry
         */
        template<typename T>
        BlackboardEntry CreateEntry(const std::string& key, const T& value,
                                   uint32_t flags, double expirationTime) const;

        /**
         * @brief Get value type for type T
         * @tparam T Type to check
         * @return Blackboard value type
         */
        template<typename T>
        BlackboardValueType GetValueType() const;

        /**
         * @brief Notify listeners of value change
         * @param key Changed key
         * @param oldValue Previous value
         * @param newValue New value
         */
        void NotifyListeners(const std::string& key, const std::any& oldValue, const std::any& newValue);

        /**
         * @brief Add event to queue
         * @param event Event to add
         */
        void AddEvent(const BlackboardEvent& event);

        /**
         * @brief Process event queue
         */
        void ProcessEventQueue();

        /**
         * @brief Cleanup expired entries
         * @param currentTime Current time
         */
        void CleanupExpiredEntries(double currentTime);

        /**
         * @brief Update metrics
         * @param operationTime Time taken for operation
         */
        void UpdateMetrics(double operationTime);

        /**
         * @brief Handle blackboard errors
         * @param error Error message
         */
        void HandleError(const std::string& error);

        // Blackboard data
        BlackboardConfig m_config;                     ///< Configuration
        BlackboardMetrics m_metrics;                   ///< Performance metrics

        // Data storage
        std::unordered_map<std::string, BlackboardEntry> m_entries; ///< Blackboard entries
        mutable std::shared_mutex m_entriesMutex;      ///< Entries synchronization

        // Event system
        std::vector<BlackboardEvent> m_eventQueue;     ///< Event queue
        std::unordered_map<uint32_t, std::function<void(const BlackboardEvent&)>> m_changeListeners; ///< Change listeners
        std::vector<uint32_t> m_globalListeners;       ///< Global listener IDs
        mutable std::shared_mutex m_eventMutex;        ///< Event synchronization

        // State
        Mob* m_owner;                                  ///< Blackboard owner
        bool m_isInitialized;                          ///< Blackboard is initialized
        double m_lastCleanupTime;                      ///< Last cleanup time
        double m_lastUpdateTime;                       ///< Last update time

        static std::atomic<uint32_t> s_nextListenerId; ///< Next listener ID counter
    };

    // Template implementations

    template<typename T>
    bool Blackboard::SetValue(const std::string& key, const T& value,
                             uint32_t flags, double expirationTime) {
        if (!m_isInitialized) {
            return false;
        }

        try {
            double currentTime = std::chrono::duration<double>(
                std::chrono::high_resolution_clock::now().time_since_epoch()).count();

            auto startTime = std::chrono::high_resolution_clock::now();

            std::unique_lock<std::shared_mutex> lock(m_entriesMutex);

            // Check if entry exists
            auto it = m_entries.find(key);
            std::any oldValue;

            if (it != m_entries.end()) {
                oldValue = it->second.value;

                // Check if entry is read-only
                if (it->second.flags & static_cast<uint32_t>(BlackboardEntryFlags::READ_ONLY)) {
                    HandleError("Attempted to modify read-only entry: " + key);
                    return false;
                }

                // Update existing entry
                it->second.value = value;
                it->second.timestamp = currentTime;
                it->second.expirationTime = expirationTime > 0.0 ?
                    currentTime + expirationTime : 0.0;
                it->second.flags = flags;
            } else {
                // Create new entry
                BlackboardEntry entry = CreateEntry(key, value, flags, expirationTime);
                entry.timestamp = currentTime;
                entry.expirationTime = expirationTime > 0.0 ?
                    currentTime + expirationTime : 0.0;

                m_entries[key] = entry;
                ++m_metrics.entriesCreated;
            }

            auto endTime = std::chrono::high_resolution_clock::now();
            double operationTime = std::chrono::duration<double>(endTime - startTime).count() * 1000.0;

            UpdateMetrics(operationTime);

            // Notify listeners
            lock.unlock();
            NotifyListeners(key, oldValue, value);

            return true;

        } catch (const std::exception& e) {
            HandleError("Exception setting value for key '" + key + "': " + e.what());
            return false;
        }
    }

    template<typename T>
    T Blackboard::GetValue(const std::string& key, const T& defaultValue) const {
        if (!m_isInitialized) {
            return defaultValue;
        }

        try {
            auto startTime = std::chrono::high_resolution_clock::now();

            std::shared_lock<std::shared_mutex> lock(m_entriesMutex);

            auto it = m_entries.find(key);
            if (it == m_entries.end()) {
                ++m_metrics.cacheMisses;
                return defaultValue;
            }

            // Check expiration
            double currentTime = std::chrono::duration<double>(
                std::chrono::high_resolution_clock::now().time_since_epoch()).count();

            if (it->second.expirationTime > 0.0 && currentTime > it->second.expirationTime) {
                ++m_metrics.entriesExpired;
                return defaultValue;
            }

            // Type checking
            if (m_config.enableTypeChecking) {
                BlackboardValueType expectedType = GetValueType<T>();
                if (it->second.type != expectedType) {
                    ++m_metrics.typeErrors;
                    return defaultValue;
                }
            }

            // Get value
            T result = defaultValue;
            try {
                result = std::any_cast<T>(it->second.value);
                ++m_metrics.cacheHits;
            } catch (const std::bad_any_cast&) {
                ++m_metrics.typeErrors;
                return defaultValue;
            }

            auto endTime = std::chrono::high_resolution_clock::now();
            double operationTime = std::chrono::duration<double>(endTime - startTime).count() * 1000.0;

            const_cast<Blackboard*>(this)->UpdateMetrics(operationTime);

            return result;

        } catch (const std::exception& e) {
            const_cast<Blackboard*>(this)->HandleError("Exception getting value for key '" + key + "': " + e.what());
            return defaultValue;
        }
    }

    template<typename T>
    BlackboardEntry Blackboard::CreateEntry(const std::string& key, const T& value,
                                           uint32_t flags, double expirationTime) const {
        BlackboardEntry entry;
        entry.key = key;
        entry.value = value;
        entry.type = GetValueType<T>();
        entry.flags = flags;
        entry.expirationTime = expirationTime;
        entry.timestamp = std::chrono::duration<double>(
            std::chrono::high_resolution_clock::now().time_since_epoch()).count();

        return entry;
    }

    template<typename T>
    BlackboardValueType Blackboard::GetValueType() const {
        if constexpr (std::is_same_v<T, int32_t>) {
            return BlackboardValueType::Integer;
        } else if constexpr (std::is_same_v<T, float>) {
            return BlackboardValueType::Float;
        } else if constexpr (std::is_same_v<T, bool>) {
            return BlackboardValueType::Boolean;
        } else if constexpr (std::is_same_v<T, std::string>) {
            return BlackboardValueType::String;
        } else if constexpr (std::is_same_v<T, glm::vec3>) {
            return BlackboardValueType::Vector3;
        } else if constexpr (std::is_same_v<T, glm::quat>) {
            return BlackboardValueType::Quaternion;
        } else {
            return BlackboardValueType::Custom;
        }
    }

} // namespace VoxelCraft

#endif // VOXELCRAFT_AI_BLACKBOARD_HPP
