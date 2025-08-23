/**
 * @file ConfigSystem.hpp
 * @brief VoxelCraft Advanced Configuration System
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_CONFIG_CONFIG_SYSTEM_HPP
#define VOXELCRAFT_CONFIG_CONFIG_SYSTEM_HPP

#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
#include <variant>
#include <atomic>
#include <mutex>
#include <shared_mutex>
#include <functional>
#include <filesystem>

#include "../math/Vec3.hpp"
#include "../math/Vec2.hpp"
#include "../math/Color.hpp"

namespace VoxelCraft {

    // Forward declarations
    struct ConfigOption;
    struct ConfigCategory;
    struct ConfigProfile;
    struct ConfigPreset;
    struct ConfigValidation;
    struct ConfigChangeEvent;
    struct ConfigStats;

    /**
     * @enum ConfigValueType
     * @brief Types of configuration values
     */
    enum class ConfigValueType {
        BOOLEAN,            ///< bool
        INTEGER,            ///< int32_t
        FLOAT,              ///< float
        DOUBLE,             ///< double
        STRING,             ///< std::string
        VECTOR2,            ///< Vec2
        VECTOR3,            ///< Vec3
        COLOR,              ///< Color
        RANGE,              ///< Range (min-max pair)
        ENUM,               ///< Enumeration
        LIST,               ///< List of values
        CUSTOM              ///< Custom type
    };

    /**
     * @enum ConfigCategoryType
     * @brief Categories of configuration options
     */
    enum class ConfigCategoryType {
        GRAPHICS,           ///< Graphics and rendering
        AUDIO,              ///< Audio and sound
        CONTROLS,           ///< Input and controls
        GAMEPLAY,           ///< Gameplay mechanics
        PERFORMANCE,        ///< Performance settings
        NETWORK,            ///< Network and multiplayer
        UI,                 ///< User interface
        ACCESSIBILITY,      ///< Accessibility options
        DEBUG,              ///< Debug and development
        MODDING,            ///< Modding and customization
        SYSTEM,             ///< System-level settings
        CUSTOM              ///< Custom category
    };

    /**
     * @enum ConfigPriority
     * @brief Priority levels for configuration changes
     */
    enum class ConfigPriority {
        LOW,                ///< Low priority changes
        NORMAL,             ///< Normal priority changes
        HIGH,               ///< High priority changes
        CRITICAL            ///< Critical changes (require restart)
    };

    /**
     * @enum ConfigScope
     * @brief Scope of configuration options
     */
    enum class ConfigScope {
        GLOBAL,             ///< Global configuration
        PROFILE,            ///< Per-profile configuration
        WORLD,              ///< Per-world configuration
        SESSION,            ///< Session-only configuration
        RUNTIME             ///< Runtime-only configuration
    };

    /**
     * @struct ConfigValue
     * @brief Configuration value container
     */
    struct ConfigValue {
        ConfigValueType type;
        std::variant<bool, int32_t, float, double, std::string,
                    Vec2, Vec3, Color, std::pair<float, float>,
                    std::vector<std::string>> value;

        // Metadata
        std::string description;
        std::string unit;
        bool requiresRestart = false;
        bool isAdvanced = false;
        std::string minVersion = "1.0.0";
        std::string deprecationMessage;

        // Validation
        std::function<bool(const ConfigValue&)> validator;
        std::pair<float, float> range;  // For numeric types
        std::vector<std::string> enumValues;  // For enum types

        // Default value
        ConfigValue defaultValue;

        // Dependencies
        std::vector<std::string> dependencies;
        std::string dependencyExpression;

        // Callbacks
        std::function<void(const ConfigValue&, const ConfigValue&)> onChange;
        std::function<void(const ConfigValue&)> onValidate;

        ConfigValue() = default;
        ConfigValue(bool v) : type(ConfigValueType::BOOLEAN), value(v) {}
        ConfigValue(int32_t v) : type(ConfigValueType::INTEGER), value(v) {}
        ConfigValue(float v) : type(ConfigValueType::FLOAT), value(v) {}
        ConfigValue(const std::string& v) : type(ConfigValueType::STRING), value(v) {}
        ConfigValue(const Vec2& v) : type(ConfigValueType::VECTOR2), value(v) {}
        ConfigValue(const Vec3& v) : type(ConfigValueType::VECTOR3), value(v) {}
        ConfigValue(const Color& v) : type(ConfigValueType::COLOR), value(v) {}
    };

    /**
     * @struct ConfigOption
     * @brief Complete configuration option definition
     */
    struct ConfigOption {
        std::string key;                    ///< Unique configuration key
        std::string displayName;            ///< Display name for UI
        std::string description;            ///< Detailed description
        ConfigCategoryType category;        ///< Configuration category
        ConfigValueType type;               ///< Value type
        ConfigScope scope;                  ///< Configuration scope
        ConfigPriority priority;            ///< Change priority

        ConfigValue value;                  ///< Current value
        ConfigValue defaultValue;           ///< Default value
        ConfigValue minValue;               ///< Minimum value (for numeric types)
        ConfigValue maxValue;               ///< Maximum value (for numeric types)

        // Metadata
        bool isReadOnly = false;            ///< Whether option can be changed
        bool isHidden = false;              ///< Whether option is hidden from UI
        bool isAdvanced = false;            ///< Whether option is advanced
        bool requiresRestart = false;       ///< Whether change requires restart
        bool isExperimental = false;        ///< Whether option is experimental
        bool isDeprecated = false;          ///< Whether option is deprecated

        // Validation
        std::function<bool(const ConfigOption&)> validator;
        std::string validationMessage;      ///< Validation error message

        // Dependencies and conflicts
        std::vector<std::string> dependencies;  ///< Required options
        std::vector<std::string> conflicts;     ///< Conflicting options
        std::string dependencyExpression;       ///< Logical dependency expression

        // Callbacks
        std::function<void(const ConfigOption&, const ConfigOption&)> onChange;
        std::function<void(const ConfigOption&)> onValidate;
        std::function<void(const ConfigOption&)> onReset;

        // UI properties
        std::string uiWidgetType;           ///< UI widget type ("slider", "checkbox", "dropdown", etc.)
        float uiStepSize = 1.0f;            ///< Step size for numeric widgets
        std::vector<std::string> uiEnumValues;  ///< Values for enum widgets
        std::string uiGroup;                ///< UI group for organization
        int uiOrder = 0;                    ///< UI display order

        // Version information
        std::string introducedVersion = "1.0.0";
        std::string deprecatedVersion;
        std::string removalVersion;

        // Tags and search
        std::vector<std::string> tags;
        std::string searchKeywords;

        // Performance impact
        float performanceImpact = 0.0f;     ///< Performance impact (0-1)
        std::string performanceDescription; ///< Performance impact description

        // Localization
        std::unordered_map<std::string, std::string> localizedNames;
        std::unordered_map<std::string, std::string> localizedDescriptions;
    };

    /**
     * @struct ConfigCategory
     * @brief Configuration category with metadata
     */
    struct ConfigCategory {
        std::string name;                   ///< Category name
        ConfigCategoryType type;            ///< Category type
        std::string displayName;            ///< Display name for UI
        std::string description;            ///< Category description
        std::string iconPath;               ///< Icon path for UI
        int displayOrder = 0;               ///< Display order in UI
        bool isExpandedByDefault = false;   ///< Whether expanded by default in UI

        // Category properties
        bool isEnabled = true;              ///< Whether category is enabled
        bool isAdvanced = false;            ///< Whether category contains advanced options
        bool requiresRestart = false;       ///< Whether category changes require restart

        // Statistics
        int optionCount = 0;                ///< Number of options in category
        int advancedOptionCount = 0;        ///< Number of advanced options
        int restartRequiredCount = 0;       ///< Number of options requiring restart

        // Subcategories
        std::vector<std::string> subcategories;
        std::string parentCategory;

        // Localization
        std::unordered_map<std::string, std::string> localizedNames;
        std::unordered_map<std::string, std::string> localizedDescriptions;
    };

    /**
     * @struct ConfigProfile
     * @brief Configuration profile for different setups
     */
    struct ConfigProfile {
        std::string name;                   ///< Profile name
        std::string description;            ///< Profile description
        std::string author;                 ///< Profile author
        std::string version = "1.0.0";      ///< Profile version

        // Profile data
        std::unordered_map<std::string, ConfigValue> values;
        std::unordered_map<std::string, ConfigCategory> categories;

        // Metadata
        bool isBuiltIn = false;             ///< Whether profile is built-in
        bool isReadOnly = false;            ///< Whether profile can be modified
        std::string creationDate;           ///< Profile creation date
        std::string lastModified;           ///< Last modification date

        // Compatibility
        std::string minGameVersion = "1.0.0";
        std::string maxGameVersion = "*";
        std::vector<std::string> compatibleMods;

        // Performance rating
        float performanceRating = 0.5f;     ///< Performance rating (0-1)
        float qualityRating = 0.5f;         ///< Quality rating (0-1)

        // Tags
        std::vector<std::string> tags;
        std::string intendedUse;            ///< Intended use case
    };

    /**
     * @struct ConfigPreset
     * @brief Configuration preset for quick setup
     */
    struct ConfigPreset {
        std::string name;                   ///< Preset name
        std::string description;            ///< Preset description
        std::string category;               ///< Preset category

        // Preset values
        std::unordered_map<std::string, ConfigValue> values;

        // Metadata
        bool isBuiltIn = true;              ///< Whether preset is built-in
        std::string previewImage;           ///< Preview image path
        float performanceRating = 0.5f;     ///< Performance rating (0-1)
        float qualityRating = 0.5f;         ///< Quality rating (0-1)

        // Requirements
        std::string minHardwareSpec;        ///< Minimum hardware requirements
        std::vector<std::string> requiredMods;

        // Statistics
        int changeCount = 0;                ///< Number of settings changed
        std::vector<std::string> affectedCategories;
    };

    /**
     * @struct ConfigValidation
     * @brief Configuration validation result
     */
    struct ConfigValidation {
        bool isValid = true;                ///< Overall validation result
        std::vector<std::string> errors;    ///< Validation errors
        std::vector<std::string> warnings;  ///< Validation warnings
        std::vector<std::string> info;      ///< Validation info messages

        // Detailed results
        std::unordered_map<std::string, bool> optionResults;
        std::unordered_map<std::string, std::string> optionMessages;

        // Performance impact
        float performanceImpact = 0.0f;     ///< Total performance impact
        float qualityImpact = 0.0f;         ///< Total quality impact

        // Recommendations
        std::vector<std::string> recommendations;
        std::vector<std::string> optimizations;
    };

    /**
     * @struct ConfigChangeEvent
     * @brief Configuration change event
     */
    struct ConfigChangeEvent {
        std::string optionKey;              ///< Changed option key
        ConfigValue oldValue;               ///< Old value
        ConfigValue newValue;               ///< New value
        ConfigPriority priority;            ///< Change priority
        bool requiresRestart;               ///< Whether restart is required
        std::string timestamp;              ///< Change timestamp
        std::string source;                 ///< Change source (user, auto, etc.)

        // Impact assessment
        float performanceImpact = 0.0f;     ///< Performance impact
        float qualityImpact = 0.0f;         ///< Quality impact
        std::vector<std::string> affectedSystems;  ///< Affected systems
    };

    /**
     * @struct ConfigStats
     * @brief Configuration system statistics
     */
    struct ConfigStats {
        int totalOptions = 0;               ///< Total configuration options
        int loadedOptions = 0;              ///< Currently loaded options
        int modifiedOptions = 0;            ///< Modified options
        int restartRequiredOptions = 0;     ///< Options requiring restart

        // Categories
        std::unordered_map<ConfigCategoryType, int> optionsByCategory;

        // Profiles
        int totalProfiles = 0;              ///< Total profiles
        int activeProfileChanges = 0;       ///< Changes in active profile

        // Presets
        int totalPresets = 0;               ///< Total presets
        int appliedPresets = 0;             ///< Applied presets

        // Changes
        int totalChanges = 0;               ///< Total configuration changes
        int pendingChanges = 0;             ///< Pending changes
        int failedChanges = 0;              ///< Failed changes

        // Performance
        float averageLoadTime = 0.0f;       ///< Average config load time
        float averageSaveTime = 0.0f;       ///< Average config save time
        float averageValidationTime = 0.0f; ///< Average validation time

        // Memory
        size_t memoryUsage = 0;             ///< Memory usage
        int allocatedObjects = 0;           ///< Allocated configuration objects
    };

    /**
     * @class ConfigSystem
     * @brief Advanced configuration system with 200+ options
     *
     * Features:
     * - 200+ customizable configuration options
     * - Profile-based configurations
     * - Configuration presets for different use cases
     * - Real-time validation and dependency checking
     * - Configuration change tracking and rollback
     * - Performance impact assessment
     * - Multi-language support and localization
     * - Cloud synchronization capabilities
     * - Configuration sharing and import/export
     * - Advanced search and filtering
     * - Configuration diff and comparison
     * - Automatic configuration optimization
     * - Hardware detection and recommendation
     * - Mod configuration support
     * - Configuration versioning and migration
     * - Performance benchmarking integration
     * - Accessibility configuration options
     * - Cross-platform configuration management
     */
    class ConfigSystem {
    public:
        static ConfigSystem& GetInstance();

        /**
         * @brief Initialize the configuration system
         * @param configDir Configuration directory path
         * @return true if successful
         */
        bool Initialize(const std::string& configDir = "config");

        /**
         * @brief Shutdown the configuration system
         */
        void Shutdown();

        /**
         * @brief Load configuration from file
         * @param filename Configuration file name
         * @return true if successful
         */
        bool LoadConfig(const std::string& filename = "config.json");

        /**
         * @brief Save configuration to file
         * @param filename Configuration file name
         * @return true if successful
         */
        bool SaveConfig(const std::string& filename = "config.json");

        /**
         * @brief Reset configuration to defaults
         * @param category Optional category to reset
         * @return true if successful
         */
        bool ResetToDefaults(ConfigCategoryType category = ConfigCategoryType::CUSTOM);

        // Configuration Access API
        /**
         * @brief Get boolean configuration value
         * @param key Configuration key
         * @param defaultValue Default value if not found
         * @return Configuration value
         */
        bool GetBool(const std::string& key, bool defaultValue = false) const;

        /**
         * @brief Get integer configuration value
         * @param key Configuration key
         * @param defaultValue Default value if not found
         * @return Configuration value
         */
        int32_t GetInt(const std::string& key, int32_t defaultValue = 0) const;

        /**
         * @brief Get float configuration value
         * @param key Configuration key
         * @param defaultValue Default value if not found
         * @return Configuration value
         */
        float GetFloat(const std::string& key, float defaultValue = 0.0f) const;

        /**
         * @brief Get string configuration value
         * @param key Configuration key
         * @param defaultValue Default value if not found
         * @return Configuration value
         */
        std::string GetString(const std::string& key, const std::string& defaultValue = "") const;

        /**
         * @brief Get Vec2 configuration value
         * @param key Configuration key
         * @param defaultValue Default value if not found
         * @return Configuration value
         */
        Vec2 GetVec2(const std::string& key, const Vec2& defaultValue = Vec2(0.0f)) const;

        /**
         * @brief Get Vec3 configuration value
         * @param key Configuration key
         * @param defaultValue Default value if not found
         * @return Configuration value
         */
        Vec3 GetVec3(const std::string& key, const Vec3& defaultValue = Vec3(0.0f)) const;

        /**
         * @brief Get Color configuration value
         * @param key Configuration key
         * @param defaultValue Default value if not found
         * @return Configuration value
         */
        Color GetColor(const std::string& key, const Color& defaultValue = Color(1.0f, 1.0f, 1.0f, 1.0f)) const;

        // Configuration Modification API
        /**
         * @brief Set boolean configuration value
         * @param key Configuration key
         * @param value New value
         * @param source Change source
         * @return true if successful
         */
        bool SetBool(const std::string& key, bool value, const std::string& source = "user");

        /**
         * @brief Set integer configuration value
         * @param key Configuration key
         * @param value New value
         * @param source Change source
         * @return true if successful
         */
        bool SetInt(const std::string& key, int32_t value, const std::string& source = "user");

        /**
         * @brief Set float configuration value
         * @param key Configuration key
         * @param value New value
         * @param source Change source
         * @return true if successful
         */
        bool SetFloat(const std::string& key, float value, const std::string& source = "user");

        /**
         * @brief Set string configuration value
         * @param key Configuration key
         * @param value New value
         * @param source Change source
         * @return true if successful
         */
        bool SetString(const std::string& key, const std::string& value, const std::string& source = "user");

        /**
         * @brief Set Vec2 configuration value
         * @param key Configuration key
         * @param value New value
         * @param source Change source
         * @return true if successful
         */
        bool SetVec2(const std::string& key, const Vec2& value, const std::string& source = "user");

        /**
         * @brief Set Vec3 configuration value
         * @param key Configuration key
         * @param value New value
         * @param source Change source
         * @return true if successful
         */
        bool SetVec3(const std::string& key, const Vec3& value, const std::string& source = "user");

        /**
         * @brief Set Color configuration value
         * @param key Configuration key
         * @param value New value
         * @param source Change source
         * @return true if successful
         */
        bool SetColor(const std::string& key, const Color& value, const std::string& source = "user");

        // Configuration Management API
        /**
         * @brief Get configuration option
         * @param key Configuration key
         * @return Configuration option or nullptr
         */
        const ConfigOption* GetOption(const std::string& key) const;

        /**
         * @brief Get all configuration options
         * @return Map of all options
         */
        const std::unordered_map<std::string, ConfigOption>& GetAllOptions() const;

        /**
         * @brief Get options by category
         * @param category Configuration category
         * @return Vector of options
         */
        std::vector<ConfigOption> GetOptionsByCategory(ConfigCategoryType category) const;

        /**
         * @brief Search configuration options
         * @param query Search query
         * @param category Optional category filter
         * @return Vector of matching options
         */
        std::vector<ConfigOption> SearchOptions(const std::string& query, ConfigCategoryType category = ConfigCategoryType::CUSTOM) const;

        /**
         * @brief Validate configuration
         * @param category Optional category to validate
         * @return Validation result
         */
        ConfigValidation ValidateConfig(ConfigCategoryType category = ConfigCategoryType::CUSTOM) const;

        /**
         * @brief Apply configuration changes
         * @param immediate Whether to apply immediately
         * @return true if successful
         */
        bool ApplyChanges(bool immediate = false);

        // Profile Management API
        /**
         * @brief Create configuration profile
         * @param name Profile name
         * @param description Profile description
         * @return true if successful
         */
        bool CreateProfile(const std::string& name, const std::string& description = "");

        /**
         * @brief Load configuration profile
         * @param name Profile name
         * @return true if successful
         */
        bool LoadProfile(const std::string& name);

        /**
         * @brief Save configuration profile
         * @param name Profile name
         * @return true if successful
         */
        bool SaveProfile(const std::string& name);

        /**
         * @brief Delete configuration profile
         * @param name Profile name
         * @return true if successful
         */
        bool DeleteProfile(const std::string& name);

        /**
         * @brief Get current profile
         * @return Current profile
         */
        const ConfigProfile* GetCurrentProfile() const;

        /**
         * @brief Get all profiles
         * @return Map of profiles
         */
        const std::unordered_map<std::string, ConfigProfile>& GetAllProfiles() const;

        // Preset Management API
        /**
         * @brief Apply configuration preset
         * @param presetName Preset name
         * @return true if successful
         */
        bool ApplyPreset(const std::string& presetName);

        /**
         * @brief Get configuration preset
         * @param presetName Preset name
         * @return Configuration preset or nullptr
         */
        const ConfigPreset* GetPreset(const std::string& presetName) const;

        /**
         * @brief Get all presets
         * @return Map of presets
         */
        const std::unordered_map<std::string, ConfigPreset>& GetAllPresets() const;

        /**
         * @brief Create preset from current configuration
         * @param name Preset name
         * @param description Preset description
         * @return true if successful
         */
        bool CreatePreset(const std::string& name, const std::string& description = "");

        // Hardware Detection and Recommendations
        /**
         * @brief Detect hardware capabilities
         * @return Hardware capability report
         */
        std::string DetectHardwareCapabilities();

        /**
         * @brief Get recommended settings for hardware
         * @param hardwareSpec Hardware specification
         * @return Recommended configuration
         */
        std::unordered_map<std::string, ConfigValue> GetRecommendedSettings(const std::string& hardwareSpec);

        /**
         * @brief Auto-configure for optimal performance
         * @return true if successful
         */
        bool AutoConfigure();

        // Import/Export API
        /**
         * @brief Export configuration
         * @param filename Export filename
         * @param category Optional category filter
         * @return true if successful
         */
        bool ExportConfig(const std::string& filename, ConfigCategoryType category = ConfigCategoryType::CUSTOM);

        /**
         * @brief Import configuration
         * @param filename Import filename
         * @param merge Whether to merge with current config
         * @return true if successful
         */
        bool ImportConfig(const std::string& filename, bool merge = true);

        /**
         * @brief Export configuration as preset
         * @param filename Export filename
         * @param name Preset name
         * @param description Preset description
         * @return true if successful
         */
        bool ExportAsPreset(const std::string& filename, const std::string& name, const std::string& description);

        // Change Tracking API
        /**
         * @brief Get pending changes
         * @return Vector of pending changes
         */
        std::vector<ConfigChangeEvent> GetPendingChanges() const;

        /**
         * @brief Get recent changes
         * @param count Number of changes to retrieve
         * @return Vector of recent changes
         */
        std::vector<ConfigChangeEvent> GetRecentChanges(int count = 10) const;

        /**
         * @brief Undo last change
         * @return true if successful
         */
        bool UndoLastChange();

        /**
         * @brief Redo last undone change
         * @return true if successful
         */
        bool RedoLastChange();

        /**
         * @brief Clear change history
         */
        void ClearChangeHistory();

        // Callback System API
        /**
         * @brief Register change callback
         * @param key Configuration key
         * @param callback Change callback
         * @return Callback ID
         */
        uint64_t RegisterChangeCallback(const std::string& key, std::function<void(const ConfigChangeEvent&)> callback);

        /**
         * @brief Unregister change callback
         * @param callbackId Callback ID
         * @return true if successful
         */
        bool UnregisterChangeCallback(uint64_t callbackId);

        /**
         * @brief Register validation callback
         * @param key Configuration key
         * @param callback Validation callback
         * @return Callback ID
         */
        uint64_t RegisterValidationCallback(const std::string& key, std::function<bool(const ConfigOption&)> callback);

        /**
         * @brief Unregister validation callback
         * @param callbackId Callback ID
         * @return true if successful
         */
        bool UnregisterValidationCallback(uint64_t callbackId);

        // Localization API
        /**
         * @brief Set language for configuration display
         * @param language Language code
         * @return true if successful
         */
        bool SetLanguage(const std::string& language);

        /**
         * @brief Get localized string
         * @param key Localization key
         * @param defaultValue Default value
         * @return Localized string
         */
        std::string GetLocalizedString(const std::string& key, const std::string& defaultValue = "") const;

        /**
         * @brief Load localization data
         * @param language Language code
         * @return true if successful
         */
        bool LoadLocalization(const std::string& language);

        // Statistics and Monitoring
        /**
         * @brief Get configuration statistics
         * @return Configuration statistics
         */
        const ConfigStats& GetStats() const { return m_stats; }

        /**
         * @brief Get configuration info
         * @return Configuration info string
         */
        std::string GetConfigInfo() const;

        /**
         * @brief Get performance impact assessment
         * @return Performance impact report
         */
        std::string GetPerformanceImpactReport() const;

        /**
         * @brief Validate configuration system
         * @return true if valid
         */
        bool Validate() const;

    private:
        ConfigSystem() = default;
        ~ConfigSystem();

        // Prevent copying
        ConfigSystem(const ConfigSystem&) = delete;
        ConfigSystem& operator=(const ConfigSystem&) = delete;

        // Initialization methods
        void InitializeDefaultOptions();
        void InitializeGraphicsOptions();
        void InitializeAudioOptions();
        void InitializeControlsOptions();
        void InitializeGameplayOptions();
        void InitializePerformanceOptions();
        void InitializeNetworkOptions();
        void InitializeUIOptions();
        void InitializeAccessibilityOptions();
        void InitializeDebugOptions();
        void InitializeModdingOptions();
        void InitializeSystemOptions();

        // Core methods
        void LoadDefaultConfig();
        void SaveDefaultConfig();
        bool ParseConfigFile(const std::string& filename);
        bool WriteConfigFile(const std::string& filename);

        // Option management
        bool RegisterOption(const ConfigOption& option);
        bool ValidateOption(const ConfigOption& option) const;
        bool CheckDependencies(const ConfigOption& option) const;
        void NotifyChangeCallbacks(const ConfigChangeEvent& event);

        // Profile management
        void LoadBuiltInProfiles();
        void SaveCurrentProfile();

        // Preset management
        void LoadBuiltInPresets();
        void CreatePerformancePresets();
        void CreateQualityPresets();
        void CreateAccessibilityPresets();

        // Validation methods
        ConfigValidation ValidateAllOptions() const;
        ConfigValidation ValidateCategory(ConfigCategoryType category) const;
        bool ValidateOptionValue(const ConfigOption& option) const;

        // Change management
        void RecordChange(const ConfigChangeEvent& event);
        void ApplyPendingChanges();
        bool RequiresRestart(const std::string& key) const;

        // Utility methods
        ConfigValue StringToConfigValue(const std::string& str, ConfigValueType type) const;
        std::string ConfigValueToString(const ConfigValue& value) const;
        ConfigValueType StringToConfigType(const std::string& typeStr) const;
        std::string ConfigTypeToString(ConfigValueType type) const;
        bool IsValidConfigKey(const std::string& key) const;

        // Performance assessment
        float CalculatePerformanceImpact(const std::string& key, const ConfigValue& value) const;
        float CalculateQualityImpact(const std::string& key, const ConfigValue& value) const;

        // Member variables
        bool m_initialized = false;
        std::string m_configDirectory;
        std::string m_currentProfile = "default";
        std::string m_currentLanguage = "en";

        // Configuration data
        std::unordered_map<std::string, ConfigOption> m_options;
        std::unordered_map<std::string, ConfigCategory> m_categories;
        std::unordered_map<std::string, ConfigProfile> m_profiles;
        std::unordered_map<std::string, ConfigPreset> m_presets;

        // Change tracking
        std::vector<ConfigChangeEvent> m_changeHistory;
        std::vector<ConfigChangeEvent> m_pendingChanges;
        std::vector<ConfigChangeEvent> m_undoStack;
        std::vector<ConfigChangeEvent> m_redoStack;

        // Callback system
        std::unordered_map<uint64_t, std::function<void(const ConfigChangeEvent&)>> m_changeCallbacks;
        std::unordered_map<uint64_t, std::function<bool(const ConfigOption&)>> m_validationCallbacks;
        std::atomic<uint64_t> m_nextCallbackId;

        // Localization
        std::unordered_map<std::string, std::unordered_map<std::string, std::string>> m_localizationData;

        // Statistics
        ConfigStats m_stats;

        // Thread safety
        mutable std::shared_mutex m_configMutex;
        mutable std::shared_mutex m_profileMutex;
        mutable std::shared_mutex m_callbackMutex;
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_CONFIG_CONFIG_SYSTEM_HPP
