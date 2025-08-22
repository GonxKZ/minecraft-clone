/**
 * @file Config.hpp
 * @brief VoxelCraft Engine Configuration Management
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file provides a comprehensive configuration management system
 * for the VoxelCraft engine. It supports loading/saving from multiple
 * formats, runtime configuration changes, and type-safe access.
 */

#ifndef VOXELCRAFT_CORE_CONFIG_HPP
#define VOXELCRAFT_CORE_CONFIG_HPP

#include <string>
#include <unordered_map>
#include <variant>
#include <vector>
#include <memory>
#include <mutex>
#include <filesystem>
#include <functional>
#include <optional>

namespace VoxelCraft {

    /**
     * @enum ConfigValueType
     * @brief Supported configuration value types
     */
    enum class ConfigValueType {
        String,
        Integer,
        Float,
        Boolean,
        Array,
        Object
    };

    /**
     * @typedef ConfigValue
     * @brief Variant type for configuration values
     */
    using ConfigValue = std::variant<
        std::string,
        int64_t,
        double,
        bool,
        std::vector<std::string>,
        std::unordered_map<std::string, std::string>
    >;

    /**
     * @struct ConfigChangeEvent
     * @brief Event data for configuration changes
     */
    struct ConfigChangeEvent {
        std::string key;           ///< Configuration key that changed
        ConfigValue oldValue;      ///< Previous value
        ConfigValue newValue;      ///< New value
        std::string source;        ///< Source of the change (file, runtime, etc.)
        double timestamp;          ///< Timestamp of the change
    };

    /**
     * @typedef ConfigChangeCallback
     * @brief Callback function type for configuration changes
     */
    using ConfigChangeCallback = std::function<void(const ConfigChangeEvent&)>;

    /**
     * @class Config
     * @brief Advanced configuration management system
     *
     * Features:
     * - Type-safe configuration access
     * - Multiple file format support (TOML, JSON, INI)
     * - Runtime configuration changes
     * - Change notifications and callbacks
     * - Configuration validation
     * - Default value management
     * - Environment variable override
     * - Command line argument integration
     * - Thread-safe operations
     * - Configuration profiles
     */
    class Config {
    public:
        /**
         * @brief Constructor
         */
        Config();

        /**
         * @brief Destructor
         */
        ~Config();

        /**
         * @brief Deleted copy constructor
         */
        Config(const Config&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        Config& operator=(const Config&) = delete;

        // Configuration Loading

        /**
         * @brief Load configuration from file
         * @param filename Path to configuration file
         * @return true if successful, false otherwise
         */
        bool LoadFromFile(const std::string& filename);

        /**
         * @brief Load configuration from string
         * @param content Configuration content as string
         * @param format File format (toml, json, ini)
         * @return true if successful, false otherwise
         */
        bool LoadFromString(const std::string& content, const std::string& format = "toml");

        /**
         * @brief Load configuration from environment variables
         * @param prefix Environment variable prefix
         * @return true if successful, false otherwise
         */
        bool LoadFromEnvironment(const std::string& prefix = "VOXELCRAFT_");

        /**
         * @brief Load configuration from command line arguments
         * @param argc Argument count
         * @param argv Argument array
         * @return true if successful, false otherwise
         */
        bool LoadFromCommandLine(int argc, char* argv[]);

        // Configuration Saving

        /**
         * @brief Save configuration to file
         * @param filename Path to save configuration
         * @param format File format (toml, json, ini)
         * @return true if successful, false otherwise
         */
        bool SaveToFile(const std::string& filename, const std::string& format = "toml");

        /**
         * @brief Save configuration to string
         * @param format File format (toml, json, ini)
         * @return Configuration as string
         */
        std::string SaveToString(const std::string& format = "toml");

        // Value Access

        /**
         * @brief Set configuration value
         * @tparam T Value type
         * @param key Configuration key
         * @param value Value to set
         * @param source Source of the change
         */
        template<typename T>
        void Set(const std::string& key, const T& value, const std::string& source = "runtime");

        /**
         * @brief Get configuration value
         * @tparam T Expected value type
         * @param key Configuration key
         * @param defaultValue Default value if key doesn't exist
         * @return Configuration value or default
         */
        template<typename T>
        T Get(const std::string& key, const T& defaultValue = T{}) const;

        /**
         * @brief Check if configuration key exists
         * @param key Configuration key
         * @return true if key exists, false otherwise
         */
        bool Has(const std::string& key) const;

        /**
         * @brief Remove configuration key
         * @param key Configuration key
         * @param source Source of the change
         */
        void Remove(const std::string& key, const std::string& source = "runtime");

        /**
         * @brief Clear all configuration values
         * @param source Source of the change
         */
        void Clear(const std::string& source = "runtime");

        // Advanced Features

        /**
         * @brief Get configuration keys matching pattern
         * @param pattern Pattern to match (supports wildcards)
         * @return Vector of matching keys
         */
        std::vector<std::string> GetKeys(const std::string& pattern = "*") const;

        /**
         * @brief Get configuration subtree
         * @param prefix Key prefix for subtree
         * @return New Config object with subtree
         */
        std::unique_ptr<Config> GetSubtree(const std::string& prefix) const;

        /**
         * @brief Merge another configuration
         * @param other Configuration to merge
         * @param source Source of the merge
         */
        void Merge(const Config& other, const std::string& source = "merge");

        /**
         * @brief Validate configuration against schema
         * @param schema Configuration schema
         * @return Vector of validation errors
         */
        std::vector<std::string> Validate(const Config& schema) const;

        // Change Notifications

        /**
         * @brief Register change callback
         * @param callback Callback function
         * @param keys Specific keys to watch (empty for all)
         * @return Callback ID for removal
         */
        uint64_t RegisterCallback(ConfigChangeCallback callback,
                                 const std::vector<std::string>& keys = {});

        /**
         * @brief Unregister change callback
         * @param callbackId Callback ID returned by RegisterCallback
         */
        void UnregisterCallback(uint64_t callbackId);

        // Configuration Profiles

        /**
         * @brief Load configuration profile
         * @param profile Profile name
         * @return true if successful, false otherwise
         */
        bool LoadProfile(const std::string& profile);

        /**
         * @brief Save configuration profile
         * @param profile Profile name
         * @return true if successful, false otherwise
         */
        bool SaveProfile(const std::string& profile);

        /**
         * @brief List available profiles
         * @return Vector of profile names
         */
        std::vector<std::string> ListProfiles() const;

        // Utility Functions

        /**
         * @brief Get configuration value type
         * @param key Configuration key
         * @return Value type or empty optional if key doesn't exist
         */
        std::optional<ConfigValueType> GetType(const std::string& key) const;

        /**
         * @brief Get number of configuration entries
         * @return Number of entries
         */
        size_t Size() const;

        /**
         * @brief Check if configuration is empty
         * @return true if empty, false otherwise
         */
        bool Empty() const;

        /**
         * @brief Get configuration statistics
         * @return Statistics as key-value pairs
         */
        std::unordered_map<std::string, size_t> GetStatistics() const;

    private:
        /**
         * @brief Parse TOML configuration
         * @param content TOML content
         * @return true if successful, false otherwise
         */
        bool ParseTOML(const std::string& content);

        /**
         * @brief Parse JSON configuration
         * @param content JSON content
         * @return true if successful, false otherwise
         */
        bool ParseJSON(const std::string& content);

        /**
         * @brief Parse INI configuration
         * @param content INI content
         * @return true if successful, false otherwise
         */
        bool ParseINI(const std::string& content);

        /**
         * @brief Generate TOML configuration
         * @return TOML string
         */
        std::string GenerateTOML() const;

        /**
         * @brief Generate JSON configuration
         * @return JSON string
         */
        std::string GenerateJSON() const;

        /**
         * @brief Generate INI configuration
         * @return INI string
         */
        std::string GenerateINI() const;

        /**
         * @brief Notify callbacks about configuration change
         * @param event Change event data
         */
        void NotifyCallbacks(const ConfigChangeEvent& event);

        /**
         * @brief Normalize configuration key
         * @param key Key to normalize
         * @return Normalized key
         */
        std::string NormalizeKey(const std::string& key) const;

        /**
         * @brief Validate configuration key
         * @param key Key to validate
         * @return true if valid, false otherwise
         */
        bool ValidateKey(const std::string& key) const;

        /**
         * @struct CallbackInfo
         * @brief Information about registered callback
         */
        struct CallbackInfo {
            ConfigChangeCallback callback;
            std::vector<std::string> keys;
            uint64_t id;
        };

        // Configuration data
        std::unordered_map<std::string, ConfigValue> m_values;        ///< Configuration values
        std::vector<CallbackInfo> m_callbacks;                       ///< Registered callbacks
        mutable std::mutex m_mutex;                                  ///< Thread synchronization

        // Profile management
        std::string m_currentProfile;                                ///< Current profile name
        std::filesystem::path m_profileDirectory;                   ///< Profile directory

        // Statistics
        size_t m_loadCount;                                          ///< Number of loads
        size_t m_saveCount;                                          ///< Number of saves
        size_t m_changeCount;                                        ///< Number of changes

        // Settings
        bool m_autoSave;                                            ///< Auto-save on changes
        bool m_validateOnLoad;                                       ///< Validate on load
        bool m_notifyOnLoad;                                         ///< Notify callbacks on load

        // Next callback ID
        uint64_t m_nextCallbackId;
    };

    // Template implementations

    template<typename T>
    void Config::Set(const std::string& key, const T& value, const std::string& source) {
        std::lock_guard<std::mutex> lock(m_mutex);

        std::string normalizedKey = NormalizeKey(key);
        if (!ValidateKey(normalizedKey)) {
            throw std::invalid_argument("Invalid configuration key: " + key);
        }

        ConfigValue oldValue;
        bool hadOldValue = m_values.count(normalizedKey) > 0;
        if (hadOldValue) {
            oldValue = m_values[normalizedKey];
        }

        ConfigValue newValue = value;
        m_values[normalizedKey] = newValue;
        m_changeCount++;

        // Notify callbacks
        if (hadOldValue || !source.empty()) {
            ConfigChangeEvent event{
                normalizedKey,
                hadOldValue ? oldValue : ConfigValue{std::string{}},
                newValue,
                source,
                static_cast<double>(std::chrono::system_clock::now().time_since_epoch().count()) / 1e9
            };
            NotifyCallbacks(event);
        }

        // Auto-save if enabled
        if (m_autoSave && !m_currentProfile.empty()) {
            SaveProfile(m_currentProfile);
        }
    }

    template<typename T>
    T Config::Get(const std::string& key, const T& defaultValue) const {
        std::lock_guard<std::mutex> lock(m_mutex);

        std::string normalizedKey = NormalizeKey(key);
        auto it = m_values.find(normalizedKey);

        if (it == m_values.end()) {
            return defaultValue;
        }

        try {
            return std::get<T>(it->second);
        } catch (const std::bad_variant_access&) {
            return defaultValue;
        }
    }

    // Utility functions

    /**
     * @brief Create default configuration
     * @return Default configuration object
     */
    std::unique_ptr<Config> CreateDefaultConfig();

    /**
     * @brief Load configuration with fallback chain
     * @param primaryPath Primary configuration file path
     * @param fallbackPaths Fallback configuration file paths
     * @return Configuration object
     */
    std::unique_ptr<Config> LoadConfigWithFallback(
        const std::string& primaryPath,
        const std::vector<std::string>& fallbackPaths = {}
    );

} // namespace VoxelCraft

#endif // VOXELCRAFT_CORE_CONFIG_HPP
