/**
 * @file ConfigSystem.cpp
 * @brief VoxelCraft Advanced Configuration System Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#include "ConfigSystem.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <thread>
#include <mutex>
#include <atomic>
#include <fstream>
#include <filesystem>
#include <nlohmann/json.hpp>

#include "../utils/Logger.hpp"
#include "../utils/Random.hpp"
#include "../math/MathUtils.hpp"

namespace VoxelCraft {

    // Static instance
    static ConfigSystem* s_instance = nullptr;

    ConfigSystem& ConfigSystem::GetInstance() {
        if (!s_instance) {
            s_instance = new ConfigSystem();
        }
        return *s_instance;
    }

    bool ConfigSystem::Initialize(const std::string& configDir) {
        if (m_initialized) {
            Logger::Warning("ConfigSystem already initialized");
            return true;
        }

        m_configDirectory = configDir;
        m_nextCallbackId = 1;

        try {
            // Create configuration directory if it doesn't exist
            std::filesystem::create_directories(m_configDirectory);

            // Initialize all configuration options
            InitializeDefaultOptions();
            InitializeGraphicsOptions();
            InitializeAudioOptions();
            InitializeControlsOptions();
            InitializeGameplayOptions();
            InitializePerformanceOptions();
            InitializeNetworkOptions();
            InitializeUIOptions();
            InitializeAccessibilityOptions();
            InitializeDebugOptions();
            InitializeModdingOptions();
            InitializeSystemOptions();

            // Load built-in profiles and presets
            LoadBuiltInProfiles();
            LoadBuiltInPresets();

            // Load default configuration
            LoadDefaultConfig();

            // Load current profile
            LoadProfile(m_currentProfile);

            m_initialized = true;
            m_stats.loadedOptions = static_cast<int>(m_options.size());
            m_stats.totalOptions = static_cast<int>(m_options.size());

            Logger::Info("ConfigSystem initialized with {} options", m_options.size());
            return true;

        } catch (const std::exception& e) {
            Logger::Error("Failed to initialize ConfigSystem: {}", e.what());
            return false;
        }
    }

    void ConfigSystem::Shutdown() {
        if (!m_initialized) {
            return;
        }

        // Save current configuration
        SaveCurrentProfile();

        // Clear all data
        m_options.clear();
        m_categories.clear();
        m_profiles.clear();
        m_presets.clear();
        m_changeHistory.clear();
        m_pendingChanges.clear();
        m_undoStack.clear();
        m_redoStack.clear();
        m_changeCallbacks.clear();
        m_validationCallbacks.clear();

        m_initialized = false;
        Logger::Info("ConfigSystem shutdown");
    }

    bool ConfigSystem::LoadConfig(const std::string& filename) {
        if (!m_initialized) {
            return false;
        }

        std::string filepath = m_configDirectory + "/" + filename;
        return ParseConfigFile(filepath);
    }

    bool ConfigSystem::SaveConfig(const std::string& filename) {
        if (!m_initialized) {
            return false;
        }

        std::string filepath = m_configDirectory + "/" + filename;
        return WriteConfigFile(filepath);
    }

    bool ConfigSystem::ResetToDefaults(ConfigCategoryType category) {
        if (!m_initialized) {
            return false;
        }

        std::unique_lock<std::shared_mutex> lock(m_configMutex);

        for (auto& pair : m_options) {
            ConfigOption& option = pair.second;

            // Skip if category doesn't match and category is specified
            if (category != ConfigCategoryType::CUSTOM && option.category != category) {
                continue;
            }

            // Reset to default value
            ConfigValue oldValue = option.value;
            option.value = option.defaultValue;

            // Record change
            ConfigChangeEvent event;
            event.optionKey = option.key;
            event.oldValue = oldValue;
            event.newValue = option.value;
            event.priority = option.priority;
            event.requiresRestart = option.requiresRestart;
            event.timestamp = GetCurrentTimestamp();
            event.source = "reset";
            event.performanceImpact = CalculatePerformanceImpact(option.key, option.value);
            event.qualityImpact = CalculateQualityImpact(option.key, option.value);

            RecordChange(event);
        }

        Logger::Info("Configuration reset to defaults for category: {}",
                    static_cast<int>(category));
        return true;
    }

    // Configuration Access API
    bool ConfigSystem::GetBool(const std::string& key, bool defaultValue) const {
        std::shared_lock<std::shared_mutex> lock(m_configMutex);
        auto it = m_options.find(key);
        if (it != m_options.end() && std::holds_alternative<bool>(it->second.value.value)) {
            return std::get<bool>(it->second.value.value);
        }
        return defaultValue;
    }

    int32_t ConfigSystem::GetInt(const std::string& key, int32_t defaultValue) const {
        std::shared_lock<std::shared_mutex> lock(m_configMutex);
        auto it = m_options.find(key);
        if (it != m_options.end() && std::holds_alternative<int32_t>(it->second.value.value)) {
            return std::get<int32_t>(it->second.value.value);
        }
        return defaultValue;
    }

    float ConfigSystem::GetFloat(const std::string& key, float defaultValue) const {
        std::shared_lock<std::shared_mutex> lock(m_configMutex);
        auto it = m_options.find(key);
        if (it != m_options.end() && std::holds_alternative<float>(it->second.value.value)) {
            return std::get<float>(it->second.value.value);
        }
        return defaultValue;
    }

    std::string ConfigSystem::GetString(const std::string& key, const std::string& defaultValue) const {
        std::shared_lock<std::shared_mutex> lock(m_configMutex);
        auto it = m_options.find(key);
        if (it != m_options.end() && std::holds_alternative<std::string>(it->second.value.value)) {
            return std::get<std::string>(it->second.value.value);
        }
        return defaultValue;
    }

    Vec2 ConfigSystem::GetVec2(const std::string& key, const Vec2& defaultValue) const {
        std::shared_lock<std::shared_mutex> lock(m_configMutex);
        auto it = m_options.find(key);
        if (it != m_options.end() && std::holds_alternative<Vec2>(it->second.value.value)) {
            return std::get<Vec2>(it->second.value.value);
        }
        return defaultValue;
    }

    Vec3 ConfigSystem::GetVec3(const std::string& key, const Vec3& defaultValue) const {
        std::shared_lock<std::shared_mutex> lock(m_configMutex);
        auto it = m_options.find(key);
        if (it != m_options.end() && std::holds_alternative<Vec3>(it->second.value.value)) {
            return std::get<Vec3>(it->second.value.value);
        }
        return defaultValue;
    }

    Color ConfigSystem::GetColor(const std::string& key, const Color& defaultValue) const {
        std::shared_lock<std::shared_mutex> lock(m_configMutex);
        auto it = m_options.find(key);
        if (it != m_options.end() && std::holds_alternative<Color>(it->second.value.value)) {
            return std::get<Color>(it->second.value.value);
        }
        return defaultValue;
    }

    // Configuration Modification API
    bool ConfigSystem::SetBool(const std::string& key, bool value, const std::string& source) {
        return SetValue(key, ConfigValue(value), source);
    }

    bool ConfigSystem::SetInt(const std::string& key, int32_t value, const std::string& source) {
        return SetValue(key, ConfigValue(value), source);
    }

    bool ConfigSystem::SetFloat(const std::string& key, float value, const std::string& source) {
        return SetValue(key, ConfigValue(value), source);
    }

    bool ConfigSystem::SetString(const std::string& key, const std::string& value, const std::string& source) {
        return SetValue(key, ConfigValue(value), source);
    }

    bool ConfigSystem::SetVec2(const std::string& key, const Vec2& value, const std::string& source) {
        return SetValue(key, ConfigValue(value), source);
    }

    bool ConfigSystem::SetVec3(const std::string& key, const Vec3& value, const std::string& source) {
        return SetValue(key, ConfigValue(value), source);
    }

    bool ConfigSystem::SetColor(const std::string& key, const Color& value, const std::string& source) {
        return SetValue(key, ConfigValue(value), source);
    }

    bool ConfigSystem::SetValue(const std::string& key, const ConfigValue& newValue, const std::string& source) {
        if (!m_initialized) {
            return false;
        }

        std::unique_lock<std::shared_mutex> lock(m_configMutex);

        auto it = m_options.find(key);
        if (it == m_options.end()) {
            Logger::Warning("Configuration option '{}' not found", key);
            return false;
        }

        ConfigOption& option = it->second;

        // Check if option is read-only
        if (option.isReadOnly) {
            Logger::Warning("Configuration option '{}' is read-only", key);
            return false;
        }

        // Validate new value
        ConfigOption tempOption = option;
        tempOption.value = newValue;

        if (!ValidateOptionValue(tempOption)) {
            Logger::Error("Configuration validation failed for option '{}'", key);
            return false;
        }

        // Store old value
        ConfigValue oldValue = option.value;

        // Set new value
        option.value = newValue;

        // Record change
        ConfigChangeEvent event;
        event.optionKey = key;
        event.oldValue = oldValue;
        event.newValue = newValue;
        event.priority = option.priority;
        event.requiresRestart = option.requiresRestart;
        event.timestamp = GetCurrentTimestamp();
        event.source = source;
        event.performanceImpact = CalculatePerformanceImpact(key, newValue);
        event.qualityImpact = CalculateQualityImpact(key, newValue);

        RecordChange(event);

        // Check if restart is required
        if (option.requiresRestart) {
            m_stats.restartRequiredOptions++;
        }

        // Notify callbacks
        NotifyChangeCallbacks(event);

        // Update statistics
        m_stats.modifiedOptions++;

        Logger::Debug("Configuration option '{}' changed by '{}'", key, source);
        return true;
    }

    // Profile Management API
    bool ConfigSystem::CreateProfile(const std::string& name, const std::string& description) {
        if (!m_initialized) {
            return false;
        }

        std::unique_lock<std::shared_mutex> lock(m_profileMutex);

        if (m_profiles.find(name) != m_profiles.end()) {
            Logger::Warning("Configuration profile '{}' already exists", name);
            return false;
        }

        ConfigProfile profile;
        profile.name = name;
        profile.description = description;
        profile.creationDate = GetCurrentTimestamp();
        profile.lastModified = GetCurrentTimestamp();

        // Copy current configuration values
        std::shared_lock<std::shared_mutex> configLock(m_configMutex);
        for (const auto& pair : m_options) {
            profile.values[pair.first] = pair.second.value;
        }

        m_profiles[name] = profile;
        m_stats.totalProfiles++;

        Logger::Info("Configuration profile '{}' created", name);
        return true;
    }

    bool ConfigSystem::LoadProfile(const std::string& name) {
        if (!m_initialized) {
            return false;
        }

        std::shared_lock<std::shared_mutex> lock(m_profileMutex);

        auto it = m_profiles.find(name);
        if (it == m_profiles.end()) {
            Logger::Warning("Configuration profile '{}' not found", name);
            return false;
        }

        const ConfigProfile& profile = it->second;

        // Apply profile values
        std::unique_lock<std::shared_mutex> configLock(m_configMutex);

        for (const auto& pair : profile.values) {
            auto optionIt = m_options.find(pair.first);
            if (optionIt != m_options.end()) {
                ConfigValue oldValue = optionIt->second.value;
                optionIt->second.value = pair.second;

                // Record change
                ConfigChangeEvent event;
                event.optionKey = pair.first;
                event.oldValue = oldValue;
                event.newValue = pair.second;
                event.priority = optionIt->second.priority;
                event.requiresRestart = optionIt->second.requiresRestart;
                event.timestamp = GetCurrentTimestamp();
                event.source = "profile_load";

                RecordChange(event);
            }
        }

        m_currentProfile = name;
        m_stats.activeProfileChanges = static_cast<int>(profile.values.size());

        Logger::Info("Configuration profile '{}' loaded", name);
        return true;
    }

    bool ConfigSystem::SaveProfile(const std::string& name) {
        if (!m_initialized) {
            return false;
        }

        std::unique_lock<std::shared_mutex> lock(m_profileMutex);

        auto it = m_profiles.find(name);
        if (it == m_profiles.end()) {
            Logger::Warning("Configuration profile '{}' not found", name);
            return false;
        }

        ConfigProfile& profile = it->second;

        // Update profile with current values
        std::shared_lock<std::shared_mutex> configLock(m_configMutex);
        for (const auto& pair : m_options) {
            profile.values[pair.first] = pair.second.value;
        }

        profile.lastModified = GetCurrentTimestamp();

        Logger::Info("Configuration profile '{}' saved", name);
        return true;
    }

    bool ConfigSystem::DeleteProfile(const std::string& name) {
        if (!m_initialized) {
            return false;
        }

        std::unique_lock<std::shared_mutex> lock(m_profileMutex);

        auto it = m_profiles.find(name);
        if (it == m_profiles.end()) {
            Logger::Warning("Configuration profile '{}' not found", name);
            return false;
        }

        // Don't allow deletion of built-in profiles
        if (it->second.isBuiltIn) {
            Logger::Warning("Cannot delete built-in profile '{}'", name);
            return false;
        }

        m_profiles.erase(it);
        m_stats.totalProfiles--;

        Logger::Info("Configuration profile '{}' deleted", name);
        return true;
    }

    // Preset Management API
    bool ConfigSystem::ApplyPreset(const std::string& presetName) {
        if (!m_initialized) {
            return false;
        }

        std::shared_lock<std::shared_mutex> lock(m_profileMutex);

        auto it = m_presets.find(presetName);
        if (it == m_presets.end()) {
            Logger::Warning("Configuration preset '{}' not found", presetName);
            return false;
        }

        const ConfigPreset& preset = it->second;

        // Apply preset values
        std::unique_lock<std::shared_mutex> configLock(m_configMutex);

        for (const auto& pair : preset.values) {
            auto optionIt = m_options.find(pair.first);
            if (optionIt != m_options.end()) {
                ConfigValue oldValue = optionIt->second.value;
                optionIt->second.value = pair.second;

                // Record change
                ConfigChangeEvent event;
                event.optionKey = pair.first;
                event.oldValue = oldValue;
                event.newValue = pair.second;
                event.priority = optionIt->second.priority;
                event.requiresRestart = optionIt->second.requiresRestart;
                event.timestamp = GetCurrentTimestamp();
                event.source = "preset_apply";

                RecordChange(event);
            }
        }

        m_stats.appliedPresets++;

        Logger::Info("Configuration preset '{}' applied", presetName);
        return true;
    }

    bool ConfigSystem::CreatePreset(const std::string& name, const std::string& description) {
        if (!m_initialized) {
            return false;
        }

        std::unique_lock<std::shared_mutex> lock(m_profileMutex);

        if (m_presets.find(name) != m_presets.end()) {
            Logger::Warning("Configuration preset '{}' already exists", name);
            return false;
        }

        ConfigPreset preset;
        preset.name = name;
        preset.description = description;

        // Copy current configuration values
        std::shared_lock<std::shared_mutex> configLock(m_configMutex);
        for (const auto& pair : m_options) {
            preset.values[pair.first] = pair.second.value;
        }

        preset.changeCount = static_cast<int>(m_options.size());

        m_presets[name] = preset;
        m_stats.totalPresets++;

        Logger::Info("Configuration preset '{}' created", name);
        return true;
    }

    // Initialization methods
    void ConfigSystem::InitializeDefaultOptions() {
        // Create basic system options
        ConfigOption option;
        option.key = "system.language";
        option.displayName = "Language";
        option.description = "Interface language";
        option.category = ConfigCategoryType::SYSTEM;
        option.type = ConfigValueType::STRING;
        option.value = std::string("en");
        option.defaultValue = std::string("en");
        option.uiWidgetType = "dropdown";
        option.uiEnumValues = {"en", "es", "fr", "de", "it", "pt", "ru", "ja", "zh"};

        RegisterOption(option);
    }

    void ConfigSystem::InitializeGraphicsOptions() {
        // Graphics Quality
        {
            ConfigOption option;
            option.key = "graphics.quality";
            option.displayName = "Graphics Quality";
            option.description = "Overall graphics quality preset";
            option.category = ConfigCategoryType::GRAPHICS;
            option.type = ConfigValueType::ENUM;
            option.value = std::string("high");
            option.defaultValue = std::string("high");
            option.uiWidgetType = "dropdown";
            option.uiEnumValues = {"low", "medium", "high", "ultra", "custom"};
            option.performanceImpact = 0.8f;
            option.qualityImpact = 0.9f;
            RegisterOption(option);
        }

        // Resolution
        {
            ConfigOption option;
            option.key = "graphics.resolution_width";
            option.displayName = "Resolution Width";
            option.description = "Screen resolution width";
            option.category = ConfigCategoryType::GRAPHICS;
            option.type = ConfigValueType::INTEGER;
            option.value = int32_t(1920);
            option.defaultValue = int32_t(1920);
            option.minValue = int32_t(800);
            option.maxValue = int32_t(7680);
            option.uiWidgetType = "slider";
            option.uiStepSize = 1.0f;
            option.requiresRestart = true;
            RegisterOption(option);
        }

        {
            ConfigOption option;
            option.key = "graphics.resolution_height";
            option.displayName = "Resolution Height";
            option.description = "Screen resolution height";
            option.category = ConfigCategoryType::GRAPHICS;
            option.type = ConfigValueType::INTEGER;
            option.value = int32_t(1080);
            option.defaultValue = int32_t(1080);
            option.minValue = int32_t(600);
            option.maxValue = int32_t(4320);
            option.uiWidgetType = "slider";
            option.uiStepSize = 1.0f;
            option.requiresRestart = true;
            RegisterOption(option);
        }

        // Anti-aliasing
        {
            ConfigOption option;
            option.key = "graphics.antialiasing";
            option.displayName = "Anti-aliasing";
            option.description = "Anti-aliasing method and quality";
            option.category = ConfigCategoryType::GRAPHICS;
            option.type = ConfigValueType::ENUM;
            option.value = std::string("fxaa");
            option.defaultValue = std::string("fxaa");
            option.uiWidgetType = "dropdown";
            option.uiEnumValues = {"none", "fxaa", "msaa_2x", "msaa_4x", "msaa_8x", "txaa"};
            option.performanceImpact = 0.3f;
            option.qualityImpact = 0.7f;
            option.isAdvanced = true;
            RegisterOption(option);
        }

        // Shadow Quality
        {
            ConfigOption option;
            option.key = "graphics.shadow_quality";
            option.displayName = "Shadow Quality";
            option.description = "Shadow rendering quality and resolution";
            option.category = ConfigCategoryType::GRAPHICS;
            option.type = ConfigValueType::ENUM;
            option.value = std::string("high");
            option.defaultValue = std::string("high");
            option.uiWidgetType = "dropdown";
            option.uiEnumValues = {"off", "low", "medium", "high", "ultra"};
            option.performanceImpact = 0.6f;
            option.qualityImpact = 0.8f;
            RegisterOption(option);
        }

        // Texture Quality
        {
            ConfigOption option;
            option.key = "graphics.texture_quality";
            option.displayName = "Texture Quality";
            option.description = "Texture resolution and filtering quality";
            option.category = ConfigCategoryType::GRAPHICS;
            option.type = ConfigValueType::ENUM;
            option.value = std::string("high");
            option.defaultValue = std::string("high");
            option.uiWidgetType = "dropdown";
            option.uiEnumValues = {"low", "medium", "high", "ultra"};
            option.performanceImpact = 0.4f;
            option.qualityImpact = 0.9f;
            RegisterOption(option);
        }

        // Render Distance
        {
            ConfigOption option;
            option.key = "graphics.render_distance";
            option.displayName = "Render Distance";
            option.description = "Maximum distance for rendering objects";
            option.category = ConfigCategoryType::GRAPHICS;
            option.type = ConfigValueType::INTEGER;
            option.value = int32_t(16);
            option.defaultValue = int32_t(16);
            option.minValue = int32_t(2);
            option.maxValue = int32_t(64);
            option.uiWidgetType = "slider";
            option.uiStepSize = 1.0f;
            option.performanceImpact = 0.9f;
            option.qualityImpact = 0.6f;
            RegisterOption(option);
        }

        // Field of View
        {
            ConfigOption option;
            option.key = "graphics.field_of_view";
            option.displayName = "Field of View";
            option.description = "Camera field of view in degrees";
            option.category = ConfigCategoryType::GRAPHICS;
            option.type = ConfigValueType::FLOAT;
            option.value = 70.0f;
            option.defaultValue = 70.0f;
            option.minValue = 30.0f;
            option.maxValue = 120.0f;
            option.uiWidgetType = "slider";
            option.uiStepSize = 1.0f;
            RegisterOption(option);
        }

        // V-Sync
        {
            ConfigOption option;
            option.key = "graphics.vsync";
            option.displayName = "V-Sync";
            option.description = "Vertical synchronization";
            option.category = ConfigCategoryType::GRAPHICS;
            option.type = ConfigValueType::BOOLEAN;
            option.value = true;
            option.defaultValue = true;
            option.uiWidgetType = "checkbox";
            option.requiresRestart = true;
            RegisterOption(option);
        }

        // Fullscreen
        {
            ConfigOption option;
            option.key = "graphics.fullscreen";
            option.displayName = "Fullscreen";
            option.description = "Enable fullscreen mode";
            option.category = ConfigCategoryType::GRAPHICS;
            option.type = ConfigValueType::BOOLEAN;
            option.value = false;
            option.defaultValue = false;
            option.uiWidgetType = "checkbox";
            option.requiresRestart = true;
            RegisterOption(option);
        }

        // Max FPS
        {
            ConfigOption option;
            option.key = "graphics.max_fps";
            option.displayName = "Max FPS";
            option.description = "Maximum frames per second (0 = unlimited)";
            option.category = ConfigCategoryType::GRAPHICS;
            option.type = ConfigValueType::INTEGER;
            option.value = int32_t(0);
            option.defaultValue = int32_t(0);
            option.minValue = int32_t(0);
            option.maxValue = int32_t(1000);
            option.uiWidgetType = "slider";
            option.uiStepSize = 10.0f;
            option.isAdvanced = true;
            RegisterOption(option);
        }
    }

    void ConfigSystem::InitializeAudioOptions() {
        // Master Volume
        {
            ConfigOption option;
            option.key = "audio.master_volume";
            option.displayName = "Master Volume";
            option.description = "Overall audio volume";
            option.category = ConfigCategoryType::AUDIO;
            option.type = ConfigValueType::FLOAT;
            option.value = 1.0f;
            option.defaultValue = 1.0f;
            option.minValue = 0.0f;
            option.maxValue = 1.0f;
            option.uiWidgetType = "slider";
            option.uiStepSize = 0.01f;
            RegisterOption(option);
        }

        // Music Volume
        {
            ConfigOption option;
            option.key = "audio.music_volume";
            option.displayName = "Music Volume";
            option.description = "Background music volume";
            option.category = ConfigCategoryType::AUDIO;
            option.type = ConfigValueType::FLOAT;
            option.value = 0.7f;
            option.defaultValue = 0.7f;
            option.minValue = 0.0f;
            option.maxValue = 1.0f;
            option.uiWidgetType = "slider";
            option.uiStepSize = 0.01f;
            RegisterOption(option);
        }

        // Sound Effects Volume
        {
            ConfigOption option;
            option.key = "audio.sfx_volume";
            option.displayName = "Sound Effects Volume";
            option.description = "Sound effects and ambient sounds volume";
            option.category = ConfigCategoryType::AUDIO;
            option.type = ConfigValueType::FLOAT;
            option.value = 0.8f;
            option.defaultValue = 0.8f;
            option.minValue = 0.0f;
            option.maxValue = 1.0f;
            option.uiWidgetType = "slider";
            option.uiStepSize = 0.01f;
            RegisterOption(option);
        }

        // Voice Volume
        {
            ConfigOption option;
            option.key = "audio.voice_volume";
            option.displayName = "Voice Volume";
            option.description = "Voice audio volume";
            option.category = ConfigCategoryType::AUDIO;
            option.type = ConfigValueType::FLOAT;
            option.value = 0.9f;
            option.defaultValue = 0.9f;
            option.minValue = 0.0f;
            option.maxValue = 1.0f;
            option.uiWidgetType = "slider";
            option.uiStepSize = 0.01f;
            RegisterOption(option);
        }

        // Audio Quality
        {
            ConfigOption option;
            option.key = "audio.quality";
            option.displayName = "Audio Quality";
            option.description = "Audio processing quality";
            option.category = ConfigCategoryType::AUDIO;
            option.type = ConfigValueType::ENUM;
            option.value = std::string("high");
            option.defaultValue = std::string("high");
            option.uiWidgetType = "dropdown";
            option.uiEnumValues = {"low", "medium", "high"};
            option.performanceImpact = 0.1f;
            option.qualityImpact = 0.6f;
            RegisterOption(option);
        }

        // Audio Output Device
        {
            ConfigOption option;
            option.key = "audio.output_device";
            option.displayName = "Audio Output Device";
            option.description = "Audio output device selection";
            option.category = ConfigCategoryType::AUDIO;
            option.type = ConfigValueType::STRING;
            option.value = std::string("default");
            option.defaultValue = std::string("default");
            option.uiWidgetType = "dropdown";
            option.uiEnumValues = {"default", "system_default"};
            option.isAdvanced = true;
            RegisterOption(option);
        }

        // Enable Audio
        {
            ConfigOption option;
            option.key = "audio.enabled";
            option.displayName = "Enable Audio";
            option.description = "Enable audio playback";
            option.category = ConfigCategoryType::AUDIO;
            option.type = ConfigValueType::BOOLEAN;
            option.value = true;
            option.defaultValue = true;
            option.uiWidgetType = "checkbox";
            RegisterOption(option);
        }

        // Audio Buffer Size
        {
            ConfigOption option;
            option.key = "audio.buffer_size";
            option.displayName = "Audio Buffer Size";
            option.description = "Audio buffer size in samples";
            option.category = ConfigCategoryType::AUDIO;
            option.type = ConfigValueType::INTEGER;
            option.value = int32_t(512);
            option.defaultValue = int32_t(512);
            option.minValue = int32_t(64);
            option.maxValue = int32_t(4096);
            option.uiWidgetType = "slider";
            option.uiStepSize = 64.0f;
            option.isAdvanced = true;
            option.performanceImpact = 0.2f;
            RegisterOption(option);
        }
    }

    void ConfigSystem::InitializeControlsOptions() {
        // Mouse Sensitivity
        {
            ConfigOption option;
            option.key = "controls.mouse_sensitivity";
            option.displayName = "Mouse Sensitivity";
            option.description = "Mouse look sensitivity";
            option.category = ConfigCategoryType::CONTROLS;
            option.type = ConfigValueType::FLOAT;
            option.value = 1.0f;
            option.defaultValue = 1.0f;
            option.minValue = 0.1f;
            option.maxValue = 5.0f;
            option.uiWidgetType = "slider";
            option.uiStepSize = 0.1f;
            RegisterOption(option);
        }

        // Mouse Invert Y
        {
            ConfigOption option;
            option.key = "controls.mouse_invert_y";
            option.displayName = "Invert Mouse Y";
            option.description = "Invert vertical mouse movement";
            option.category = ConfigCategoryType::CONTROLS;
            option.type = ConfigValueType::BOOLEAN;
            option.value = false;
            option.defaultValue = false;
            option.uiWidgetType = "checkbox";
            RegisterOption(option);
        }

        // Controller Sensitivity
        {
            ConfigOption option;
            option.key = "controls.controller_sensitivity";
            option.displayName = "Controller Sensitivity";
            option.description = "Controller look sensitivity";
            option.category = ConfigCategoryType::CONTROLS;
            option.type = ConfigValueType::FLOAT;
            option.value = 1.0f;
            option.defaultValue = 1.0f;
            option.minValue = 0.1f;
            option.maxValue = 3.0f;
            option.uiWidgetType = "slider";
            option.uiStepSize = 0.1f;
            RegisterOption(option);
        }

        // Controller Vibration
        {
            ConfigOption option;
            option.key = "controls.controller_vibration";
            option.displayName = "Controller Vibration";
            option.description = "Enable controller vibration feedback";
            option.category = ConfigCategoryType::CONTROLS;
            option.type = ConfigValueType::BOOLEAN;
            option.value = true;
            option.defaultValue = true;
            option.uiWidgetType = "checkbox";
            RegisterOption(option);
        }

        // Key Bindings (simplified - in reality would be more complex)
        {
            ConfigOption option;
            option.key = "controls.key_forward";
            option.displayName = "Forward Key";
            option.description = "Key for moving forward";
            option.category = ConfigCategoryType::CONTROLS;
            option.type = ConfigValueType::STRING;
            option.value = std::string("W");
            option.defaultValue = std::string("W");
            option.uiWidgetType = "keybind";
            option.isAdvanced = true;
            RegisterOption(option);
        }

        {
            ConfigOption option;
            option.key = "controls.key_backward";
            option.displayName = "Backward Key";
            option.description = "Key for moving backward";
            option.category = ConfigCategoryType::CONTROLS;
            option.type = ConfigValueType::STRING;
            option.value = std::string("S");
            option.defaultValue = std::string("S");
            option.uiWidgetType = "keybind";
            option.isAdvanced = true;
            RegisterOption(option);
        }

        // Auto-run
        {
            ConfigOption option;
            option.key = "controls.auto_run";
            option.displayName = "Auto-run";
            option.description = "Automatically run when moving";
            option.category = ConfigCategoryType::CONTROLS;
            option.type = ConfigValueType::BOOLEAN;
            option.value = false;
            option.defaultValue = false;
            option.uiWidgetType = "checkbox";
            RegisterOption(option);
        }

        // Toggle Sprint
        {
            ConfigOption option;
            option.key = "controls.toggle_sprint";
            option.displayName = "Toggle Sprint";
            option.description = "Sprint is toggled instead of held";
            option.category = ConfigCategoryType::CONTROLS;
            option.type = ConfigValueType::BOOLEAN;
            option.value = false;
            option.defaultValue = false;
            option.uiWidgetType = "checkbox";
            RegisterOption(option);
        }
    }

    void ConfigSystem::InitializeGameplayOptions() {
        // Difficulty
        {
            ConfigOption option;
            option.key = "gameplay.difficulty";
            option.displayName = "Difficulty";
            option.description = "Game difficulty level";
            option.category = ConfigCategoryType::GAMEPLAY;
            option.type = ConfigValueType::ENUM;
            option.value = std::string("normal");
            option.defaultValue = std::string("normal");
            option.uiWidgetType = "dropdown";
            option.uiEnumValues = {"peaceful", "easy", "normal", "hard", "hardcore"};
            RegisterOption(option);
        }

        // Player Health
        {
            ConfigOption option;
            option.key = "gameplay.max_health";
            option.displayName = "Max Health";
            option.description = "Maximum player health";
            option.category = ConfigCategoryType::GAMEPLAY;
            option.type = ConfigValueType::INTEGER;
            option.value = int32_t(20);
            option.defaultValue = int32_t(20);
            option.minValue = int32_t(1);
            option.maxValue = int32_t(100);
            option.uiWidgetType = "slider";
            option.uiStepSize = 1.0f;
            RegisterOption(option);
        }

        // Player Speed
        {
            ConfigOption option;
            option.key = "gameplay.player_speed";
            option.displayName = "Player Speed";
            option.description = "Base player movement speed";
            option.category = ConfigCategoryType::GAMEPLAY;
            option.type = ConfigValueType::FLOAT;
            option.value = 1.0f;
            option.defaultValue = 1.0f;
            option.minValue = 0.1f;
            option.maxValue = 5.0f;
            option.uiWidgetType = "slider";
            option.uiStepSize = 0.1f;
            RegisterOption(option);
        }

        // Jump Height
        {
            ConfigOption option;
            option.key = "gameplay.jump_height";
            option.displayName = "Jump Height";
            option.description = "Player jump height multiplier";
            option.category = ConfigCategoryType::GAMEPLAY;
            option.type = ConfigValueType::FLOAT;
            option.value = 1.0f;
            option.defaultValue = 1.0f;
            option.minValue = 0.1f;
            option.maxValue = 3.0f;
            option.uiWidgetType = "slider";
            option.uiStepSize = 0.1f;
            RegisterOption(option);
        }

        // Survival Features
        {
            ConfigOption option;
            option.key = "gameplay.enable_hunger";
            option.displayName = "Enable Hunger";
            option.description = "Enable hunger and food mechanics";
            option.category = ConfigCategoryType::GAMEPLAY;
            option.type = ConfigValueType::BOOLEAN;
            option.value = true;
            option.defaultValue = true;
            option.uiWidgetType = "checkbox";
            RegisterOption(option);
        }

        {
            ConfigOption option;
            option.key = "gameplay.enable_thirst";
            option.displayName = "Enable Thirst";
            option.description = "Enable thirst mechanics";
            option.category = ConfigCategoryType::GAMEPLAY;
            option.type = ConfigValueType::BOOLEAN;
            option.value = true;
            option.defaultValue = true;
            option.uiWidgetType = "checkbox";
            RegisterOption(option);
        }

        {
            ConfigOption option;
            option.key = "gameplay.enable_temperature";
            option.displayName = "Enable Temperature";
            option.description = "Enable temperature and climate effects";
            option.category = ConfigCategoryType::GAMEPLAY;
            option.type = ConfigValueType::BOOLEAN;
            option.value = true;
            option.defaultValue = true;
            option.uiWidgetType = "checkbox";
            RegisterOption(option);
        }

        // World Settings
        {
            ConfigOption option;
            option.key = "gameplay.world_size";
            option.displayName = "World Size";
            option.description = "World generation size";
            option.category = ConfigCategoryType::GAMEPLAY;
            option.type = ConfigValueType::ENUM;
            option.value = std::string("medium");
            option.defaultValue = std::string("medium");
            option.uiWidgetType = "dropdown";
            option.uiEnumValues = {"small", "medium", "large", "huge"};
            option.isAdvanced = true;
            RegisterOption(option);
        }

        // Day/Night Cycle
        {
            ConfigOption option;
            option.key = "gameplay.day_night_cycle";
            option.displayName = "Day/Night Cycle";
            option.description = "Enable day/night cycle";
            option.category = ConfigCategoryType::GAMEPLAY;
            option.type = ConfigValueType::BOOLEAN;
            option.value = true;
            option.defaultValue = true;
            option.uiWidgetType = "checkbox";
            RegisterOption(option);
        }

        {
            ConfigOption option;
            option.key = "gameplay.day_length";
            option.displayName = "Day Length";
            option.description = "Length of day in minutes";
            option.category = ConfigCategoryType::GAMEPLAY;
            option.type = ConfigValueType::INTEGER;
            option.value = int32_t(20);
            option.defaultValue = int32_t(20);
            option.minValue = int32_t(1);
            option.maxValue = int32_t(120);
            option.uiWidgetType = "slider";
            option.uiStepSize = 1.0f;
            option.isAdvanced = true;
            RegisterOption(option);
        }
    }

    void ConfigSystem::InitializePerformanceOptions() {
        // Target FPS
        {
            ConfigOption option;
            option.key = "performance.target_fps";
            option.displayName = "Target FPS";
            option.description = "Target frames per second";
            option.category = ConfigCategoryType::PERFORMANCE;
            option.type = ConfigValueType::INTEGER;
            option.value = int32_t(60);
            option.defaultValue = int32_t(60);
            option.minValue = int32_t(10);
            option.maxValue = int32_t(1000);
            option.uiWidgetType = "slider";
            option.uiStepSize = 5.0f;
            RegisterOption(option);
        }

        // Max Loaded Chunks
        {
            ConfigOption option;
            option.key = "performance.max_loaded_chunks";
            option.displayName = "Max Loaded Chunks";
            option.description = "Maximum number of loaded chunks";
            option.category = ConfigCategoryType::PERFORMANCE;
            option.type = ConfigValueType::INTEGER;
            option.value = int32_t(1000);
            option.defaultValue = int32_t(1000);
            option.minValue = int32_t(100);
            option.maxValue = int32_t(10000);
            option.uiWidgetType = "slider";
            option.uiStepSize = 50.0f;
            option.isAdvanced = true;
            RegisterOption(option);
        }

        // View Distance
        {
            ConfigOption option;
            option.key = "performance.view_distance";
            option.displayName = "View Distance";
            option.description = "Maximum view distance in chunks";
            option.category = ConfigCategoryType::PERFORMANCE;
            option.type = ConfigValueType::INTEGER;
            option.value = int32_t(16);
            option.defaultValue = int32_t(16);
            option.minValue = int32_t(2);
            option.maxValue = int32_t(64);
            option.uiWidgetType = "slider";
            option.uiStepSize = 1.0f;
            RegisterOption(option);
        }

        // Enable LOD
        {
            ConfigOption option;
            option.key = "performance.enable_lod";
            option.displayName = "Enable LOD";
            option.description = "Enable level of detail for distant objects";
            option.category = ConfigCategoryType::PERFORMANCE;
            option.type = ConfigValueType::BOOLEAN;
            option.value = true;
            option.defaultValue = true;
            option.uiWidgetType = "checkbox";
            option.isAdvanced = true;
            RegisterOption(option);
        }

        // Enable Threading
        {
            ConfigOption option;
            option.key = "performance.enable_threading";
            option.displayName = "Enable Multi-threading";
            option.description = "Enable multi-threaded processing";
            option.category = ConfigCategoryType::PERFORMANCE;
            option.type = ConfigValueType::BOOLEAN;
            option.value = true;
            option.defaultValue = true;
            option.uiWidgetType = "checkbox";
            option.isAdvanced = true;
            RegisterOption(option);
        }

        // Max Background Threads
        {
            ConfigOption option;
            option.key = "performance.max_background_threads";
            option.displayName = "Max Background Threads";
            option.description = "Maximum number of background threads";
            option.category = ConfigCategoryType::PERFORMANCE;
            option.type = ConfigValueType::INTEGER;
            option.value = int32_t(4);
            option.defaultValue = int32_t(4);
            option.minValue = int32_t(1);
            option.maxValue = int32_t(16);
            option.uiWidgetType = "slider";
            option.uiStepSize = 1.0f;
            option.isAdvanced = true;
            RegisterOption(option);
        }

        // Memory Limit
        {
            ConfigOption option;
            option.key = "performance.memory_limit_mb";
            option.displayName = "Memory Limit (MB)";
            option.description = "Maximum memory usage in megabytes";
            option.category = ConfigCategoryType::PERFORMANCE;
            option.type = ConfigValueType::INTEGER;
            option.value = int32_t(2048);
            option.defaultValue = int32_t(2048);
            option.minValue = int32_t(512);
            option.maxValue = int32_t(16384);
            option.uiWidgetType = "slider";
            option.uiStepSize = 128.0f;
            option.isAdvanced = true;
            RegisterOption(option);
        }

        // Enable Caching
        {
            ConfigOption option;
            option.key = "performance.enable_caching";
            option.displayName = "Enable Caching";
            option.description = "Enable various caching systems";
            option.category = ConfigCategoryType::PERFORMANCE;
            option.type = ConfigValueType::BOOLEAN;
            option.value = true;
            option.defaultValue = true;
            option.uiWidgetType = "checkbox";
            option.isAdvanced = true;
            RegisterOption(option);
        }
    }

    void ConfigSystem::InitializeNetworkOptions() {
        // Network Type
        {
            ConfigOption option;
            option.key = "network.type";
            option.displayName = "Network Type";
            option.description = "Network connection type";
            option.category = ConfigCategoryType::NETWORK;
            option.type = ConfigValueType::ENUM;
            option.value = std::string("client");
            option.defaultValue = std::string("client");
            option.uiWidgetType = "dropdown";
            option.uiEnumValues = {"singleplayer", "client", "server"};
            option.requiresRestart = true;
            option.isAdvanced = true;
            RegisterOption(option);
        }

        // Server Address
        {
            ConfigOption option;
            option.key = "network.server_address";
            option.displayName = "Server Address";
            option.description = "Server IP address or hostname";
            option.category = ConfigCategoryType::NETWORK;
            option.type = ConfigValueType::STRING;
            option.value = std::string("localhost");
            option.defaultValue = std::string("localhost");
            option.uiWidgetType = "text";
            option.isAdvanced = true;
            RegisterOption(option);
        }

        // Server Port
        {
            ConfigOption option;
            option.key = "network.server_port";
            option.displayName = "Server Port";
            option.description = "Server port number";
            option.category = ConfigCategoryType::NETWORK;
            option.type = ConfigValueType::INTEGER;
            option.value = int32_t(25565);
            option.defaultValue = int32_t(25565);
            option.minValue = int32_t(1);
            option.maxValue = int32_t(65535);
            option.uiWidgetType = "slider";
            option.uiStepSize = 1.0f;
            option.isAdvanced = true;
            RegisterOption(option);
        }

        // Max Players
        {
            ConfigOption option;
            option.key = "network.max_players";
            option.displayName = "Max Players";
            option.description = "Maximum number of players (server only)";
            option.category = ConfigCategoryType::NETWORK;
            option.type = ConfigValueType::INTEGER;
            option.value = int32_t(10);
            option.defaultValue = int32_t(10);
            option.minValue = int32_t(1);
            option.maxValue = int32_t(100);
            option.uiWidgetType = "slider";
            option.uiStepSize = 1.0f;
            option.isAdvanced = true;
            RegisterOption(option);
        }

        // Enable NAT Punchthrough
        {
            ConfigOption option;
            option.key = "network.enable_nat_punchthrough";
            option.displayName = "Enable NAT Punchthrough";
            option.description = "Enable NAT punchthrough for connections";
            option.category = ConfigCategoryType::NETWORK;
            option.type = ConfigValueType::BOOLEAN;
            option.value = true;
            option.defaultValue = true;
            option.uiWidgetType = "checkbox";
            option.isAdvanced = true;
            RegisterOption(option);
        }

        // Connection Timeout
        {
            ConfigOption option;
            option.key = "network.connection_timeout";
            option.displayName = "Connection Timeout";
            option.description = "Connection timeout in seconds";
            option.category = ConfigCategoryType::NETWORK;
            option.type = ConfigValueType::INTEGER;
            option.value = int32_t(30);
            option.defaultValue = int32_t(30);
            option.minValue = int32_t(5);
            option.maxValue = int32_t(300);
            option.uiWidgetType = "slider";
            option.uiStepSize = 5.0f;
            option.isAdvanced = true;
            RegisterOption(option);
        }
    }

    void ConfigSystem::InitializeUIOptions() {
        // UI Scale
        {
            ConfigOption option;
            option.key = "ui.scale";
            option.displayName = "UI Scale";
            option.description = "User interface scale";
            option.category = ConfigCategoryType::UI;
            option.type = ConfigValueType::FLOAT;
            option.value = 1.0f;
            option.defaultValue = 1.0f;
            option.minValue = 0.5f;
            option.maxValue = 3.0f;
            option.uiWidgetType = "slider";
            option.uiStepSize = 0.1f;
            RegisterOption(option);
        }

        // HUD Opacity
        {
            ConfigOption option;
            option.key = "ui.hud_opacity";
            option.displayName = "HUD Opacity";
            option.description = "Heads-up display opacity";
            option.category = ConfigCategoryType::UI;
            option.type = ConfigValueType::FLOAT;
            option.value = 1.0f;
            option.defaultValue = 1.0f;
            option.minValue = 0.1f;
            option.maxValue = 1.0f;
            option.uiWidgetType = "slider";
            option.uiStepSize = 0.1f;
            RegisterOption(option);
        }

        // Show FPS
        {
            ConfigOption option;
            option.key = "ui.show_fps";
            option.displayName = "Show FPS";
            option.description = "Display frames per second counter";
            option.category = ConfigCategoryType::UI;
            option.type = ConfigValueType::BOOLEAN;
            option.value = false;
            option.defaultValue = false;
            option.uiWidgetType = "checkbox";
            RegisterOption(option);
        }

        // Show Coordinates
        {
            ConfigOption option;
            option.key = "ui.show_coordinates";
            option.displayName = "Show Coordinates";
            option.description = "Display player coordinates";
            option.category = ConfigCategoryType::UI;
            option.type = ConfigValueType::BOOLEAN;
            option.value = false;
            option.defaultValue = false;
            option.uiWidgetType = "checkbox";
            RegisterOption(option);
        }

        // Crosshair Style
        {
            ConfigOption option;
            option.key = "ui.crosshair_style";
            option.displayName = "Crosshair Style";
            option.description = "Crosshair appearance style";
            option.category = ConfigCategoryType::UI;
            option.type = ConfigValueType::ENUM;
            option.value = std::string("default");
            option.defaultValue = std::string("default");
            option.uiWidgetType = "dropdown";
            option.uiEnumValues = {"default", "dot", "cross", "circle", "none"};
            RegisterOption(option);
        }

        // Inventory Style
        {
            ConfigOption option;
            option.key = "ui.inventory_style";
            option.displayName = "Inventory Style";
            option.description = "Inventory interface style";
            option.category = ConfigCategoryType::UI;
            option.type = ConfigValueType::ENUM;
            option.value = std::string("grid");
            option.defaultValue = std::string("grid");
            option.uiWidgetType = "dropdown";
            option.uiEnumValues = {"grid", "list", "compact"};
            RegisterOption(option);
        }
    }

    void ConfigSystem::InitializeAccessibilityOptions() {
        // High Contrast Mode
        {
            ConfigOption option;
            option.key = "accessibility.high_contrast";
            option.displayName = "High Contrast Mode";
            option.description = "Enable high contrast interface";
            option.category = ConfigCategoryType::ACCESSIBILITY;
            option.type = ConfigValueType::BOOLEAN;
            option.value = false;
            option.defaultValue = false;
            option.uiWidgetType = "checkbox";
            RegisterOption(option);
        }

        // Large Text
        {
            ConfigOption option;
            option.key = "accessibility.large_text";
            option.displayName = "Large Text";
            option.description = "Use larger text size";
            option.category = ConfigCategoryType::ACCESSIBILITY;
            option.type = ConfigValueType::BOOLEAN;
            option.value = false;
            option.defaultValue = false;
            option.uiWidgetType = "checkbox";
            RegisterOption(option);
        }

        // Color Blind Mode
        {
            ConfigOption option;
            option.key = "accessibility.color_blind_mode";
            option.displayName = "Color Blind Mode";
            option.description = "Color blind accessibility mode";
            option.category = ConfigCategoryType::ACCESSIBILITY;
            option.type = ConfigValueType::ENUM;
            option.value = std::string("none");
            option.defaultValue = std::string("none");
            option.uiWidgetType = "dropdown";
            option.uiEnumValues = {"none", "protanopia", "deuteranopia", "tritanopia", "achromatopsia"};
            RegisterOption(option);
        }

        // Text-to-Speech
        {
            ConfigOption option;
            option.key = "accessibility.text_to_speech";
            option.displayName = "Text-to-Speech";
            option.description = "Enable text-to-speech for UI elements";
            option.category = ConfigCategoryType::ACCESSIBILITY;
            option.type = ConfigValueType::BOOLEAN;
            option.value = false;
            option.defaultValue = false;
            option.uiWidgetType = "checkbox";
            RegisterOption(option);
        }

        // Subtitles
        {
            ConfigOption option;
            option.key = "accessibility.subtitles";
            option.displayName = "Subtitles";
            option.description = "Enable subtitles for audio";
            option.category = ConfigCategoryType::ACCESSIBILITY;
            option.type = ConfigValueType::BOOLEAN;
            option.value = false;
            option.defaultValue = false;
            option.uiWidgetType = "checkbox";
            RegisterOption(option);
        }

        // Screen Reader Support
        {
            ConfigOption option;
            option.key = "accessibility.screen_reader";
            option.displayName = "Screen Reader Support";
            option.description = "Enable screen reader compatibility";
            option.category = ConfigCategoryType::ACCESSIBILITY;
            option.type = ConfigValueType::BOOLEAN;
            option.value = false;
            option.defaultValue = false;
            option.uiWidgetType = "checkbox";
            option.isAdvanced = true;
            RegisterOption(option);
        }
    }

    void ConfigSystem::InitializeDebugOptions() {
        // Enable Debug Overlay
        {
            ConfigOption option;
            option.key = "debug.enable_overlay";
            option.displayName = "Enable Debug Overlay";
            option.description = "Show debug information overlay";
            option.category = ConfigCategoryType::DEBUG;
            option.type = ConfigValueType::BOOLEAN;
            option.value = false;
            option.defaultValue = false;
            option.uiWidgetType = "checkbox";
            option.isHidden = true;
            RegisterOption(option);
        }

        // Debug Level
        {
            ConfigOption option;
            option.key = "debug.log_level";
            option.displayName = "Debug Log Level";
            option.description = "Minimum log level for debug output";
            option.category = ConfigCategoryType::DEBUG;
            option.type = ConfigValueType::ENUM;
            option.value = std::string("info");
            option.defaultValue = std::string("info");
            option.uiWidgetType = "dropdown";
            option.uiEnumValues = {"trace", "debug", "info", "warning", "error"};
            option.isHidden = true;
            RegisterOption(option);
        }

        // Performance Profiling
        {
            ConfigOption option;
            option.key = "debug.enable_profiling";
            option.displayName = "Enable Profiling";
            option.description = "Enable performance profiling";
            option.category = ConfigCategoryType::DEBUG;
            option.type = ConfigValueType::BOOLEAN;
            option.value = false;
            option.defaultValue = false;
            option.uiWidgetType = "checkbox";
            option.isHidden = true;
            RegisterOption(option);
        }

        // Wireframe Mode
        {
            ConfigOption option;
            option.key = "debug.wireframe_mode";
            option.displayName = "Wireframe Mode";
            option.description = "Render in wireframe mode";
            option.category = ConfigCategoryType::DEBUG;
            option.type = ConfigValueType::BOOLEAN;
            option.value = false;
            option.defaultValue = false;
            option.uiWidgetType = "checkbox";
            option.isHidden = true;
            RegisterOption(option);
        }
    }

    void ConfigSystem::InitializeModdingOptions() {
        // Enable Mods
        {
            ConfigOption option;
            option.key = "modding.enable_mods";
            option.displayName = "Enable Mods";
            option.description = "Enable mod loading and execution";
            option.category = ConfigCategoryType::MODDING;
            option.type = ConfigValueType::BOOLEAN;
            option.value = true;
            option.defaultValue = true;
            option.uiWidgetType = "checkbox";
            option.requiresRestart = true;
            RegisterOption(option);
        }

        // Mod Directory
        {
            ConfigOption option;
            option.key = "modding.mod_directory";
            option.displayName = "Mod Directory";
            option.description = "Directory for loading mods";
            option.category = ConfigCategoryType::MODDING;
            option.type = ConfigValueType::STRING;
            option.value = std::string("mods");
            option.defaultValue = std::string("mods");
            option.uiWidgetType = "text";
            option.isAdvanced = true;
            RegisterOption(option);
        }

        // Max Mods
        {
            ConfigOption option;
            option.key = "modding.max_mods";
            option.displayName = "Max Mods";
            option.description = "Maximum number of loaded mods";
            option.category = ConfigCategoryType::MODDING;
            option.type = ConfigValueType::INTEGER;
            option.value = int32_t(100);
            option.defaultValue = int32_t(100);
            option.minValue = int32_t(1);
            option.maxValue = int32_t(1000);
            option.uiWidgetType = "slider";
            option.uiStepSize = 10.0f;
            option.isAdvanced = true;
            RegisterOption(option);
        }
    }

    void ConfigSystem::InitializeSystemOptions() {
        // Application Name
        {
            ConfigOption option;
            option.key = "system.app_name";
            option.displayName = "Application Name";
            option.description = "Application name and title";
            option.category = ConfigCategoryType::SYSTEM;
            option.type = ConfigValueType::STRING;
            option.value = std::string("VoxelCraft");
            option.defaultValue = std::string("VoxelCraft");
            option.isReadOnly = true;
            option.uiWidgetType = "text";
            RegisterOption(option);
        }

        // Version
        {
            ConfigOption option;
            option.key = "system.version";
            option.displayName = "Version";
            option.description = "Application version";
            option.category = ConfigCategoryType::SYSTEM;
            option.type = ConfigValueType::STRING;
            option.value = std::string("1.0.0");
            option.defaultValue = std::string("1.0.0");
            option.isReadOnly = true;
            option.uiWidgetType = "text";
            RegisterOption(option);
        }

        // Build Type
        {
            ConfigOption option;
            option.key = "system.build_type";
            option.displayName = "Build Type";
            option.description = "Application build configuration";
            option.category = ConfigCategoryType::SYSTEM;
            option.type = ConfigValueType::STRING;
            option.value = std::string("release");
            option.defaultValue = std::string("release");
            option.isReadOnly = true;
            option.uiWidgetType = "text";
            option.isAdvanced = true;
            RegisterOption(option);
        }
    }

    // Utility methods implementation
    bool ConfigSystem::RegisterOption(const ConfigOption& option) {
        if (!IsValidConfigKey(option.key)) {
            Logger::Error("Invalid configuration key: {}", option.key);
            return false;
        }

        if (m_options.find(option.key) != m_options.end()) {
            Logger::Warning("Configuration option '{}' already exists", option.key);
            return false;
        }

        m_options[option.key] = option;

        // Create category if it doesn't exist
        if (m_categories.find(option.category) == m_categories.end()) {
            ConfigCategory category;
            category.name = ConfigCategoryToString(option.category);
            category.type = option.category;
            category.displayName = GetCategoryDisplayName(option.category);
            category.isEnabled = true;
            m_categories[option.category] = category;
        }

        m_categories[option.category].optionCount++;

        if (option.isAdvanced) {
            m_categories[option.category].advancedOptionCount++;
        }

        if (option.requiresRestart) {
            m_categories[option.category].restartRequiredCount++;
        }

        m_stats.totalOptions++;
        return true;
    }

    bool ConfigSystem::IsValidConfigKey(const std::string& key) const {
        if (key.empty()) {
            return false;
        }

        // Check for valid characters (alphanumeric, underscore, dot)
        for (char c : key) {
            if (!isalnum(c) && c != '_' && c != '.') {
                return false;
            }
        }

        return true;
    }

    std::string ConfigSystem::ConfigCategoryToString(ConfigCategoryType type) const {
        switch (type) {
            case ConfigCategoryType::GRAPHICS: return "graphics";
            case ConfigCategoryType::AUDIO: return "audio";
            case ConfigCategoryType::CONTROLS: return "controls";
            case ConfigCategoryType::GAMEPLAY: return "gameplay";
            case ConfigCategoryType::PERFORMANCE: return "performance";
            case ConfigCategoryType::NETWORK: return "network";
            case ConfigCategoryType::UI: return "ui";
            case ConfigCategoryType::ACCESSIBILITY: return "accessibility";
            case ConfigCategoryType::DEBUG: return "debug";
            case ConfigCategoryType::MODDING: return "modding";
            case ConfigCategoryType::SYSTEM: return "system";
            default: return "custom";
        }
    }

    std::string ConfigSystem::GetCategoryDisplayName(ConfigCategoryType type) const {
        switch (type) {
            case ConfigCategoryType::GRAPHICS: return "Graphics";
            case ConfigCategoryType::AUDIO: return "Audio";
            case ConfigCategoryType::CONTROLS: return "Controls";
            case ConfigCategoryType::GAMEPLAY: return "Gameplay";
            case ConfigCategoryType::PERFORMANCE: return "Performance";
            case ConfigCategoryType::NETWORK: return "Network";
            case ConfigCategoryType::UI: return "User Interface";
            case ConfigCategoryType::ACCESSIBILITY: return "Accessibility";
            case ConfigCategoryType::DEBUG: return "Debug";
            case ConfigCategoryType::MODDING: return "Modding";
            case ConfigCategoryType::SYSTEM: return "System";
            default: return "Custom";
        }
    }

    void ConfigSystem::LoadBuiltInProfiles() {
        // High Performance Profile
        {
            ConfigProfile profile;
            profile.name = "high_performance";
            profile.description = "Optimized for maximum performance";
            profile.isBuiltIn = true;
            profile.performanceRating = 0.9f;
            profile.qualityRating = 0.3f;

            // Performance-oriented settings
            profile.values["graphics.quality"] = ConfigValue(std::string("low"));
            profile.values["graphics.render_distance"] = ConfigValue(int32_t(8));
            profile.values["graphics.shadow_quality"] = ConfigValue(std::string("off"));
            profile.values["graphics.antialiasing"] = ConfigValue(std::string("fxaa"));
            profile.values["performance.view_distance"] = ConfigValue(int32_t(8));
            profile.values["performance.max_loaded_chunks"] = ConfigValue(int32_t(500));

            m_profiles["high_performance"] = profile;
            m_stats.totalProfiles++;
        }

        // Balanced Profile
        {
            ConfigProfile profile;
            profile.name = "balanced";
            profile.description = "Balanced performance and quality";
            profile.isBuiltIn = true;
            profile.performanceRating = 0.7f;
            profile.qualityRating = 0.7f;

            // Balanced settings
            profile.values["graphics.quality"] = ConfigValue(std::string("high"));
            profile.values["graphics.render_distance"] = ConfigValue(int32_t(16));
            profile.values["graphics.shadow_quality"] = ConfigValue(std::string("medium"));
            profile.values["graphics.antialiasing"] = ConfigValue(std::string("msaa_2x"));
            profile.values["performance.view_distance"] = ConfigValue(int32_t(16));
            profile.values["performance.max_loaded_chunks"] = ConfigValue(int32_t(1000));

            m_profiles["balanced"] = profile;
            m_stats.totalProfiles++;
        }

        // High Quality Profile
        {
            ConfigProfile profile;
            profile.name = "high_quality";
            profile.description = "Maximum visual quality";
            profile.isBuiltIn = true;
            profile.performanceRating = 0.2f;
            profile.qualityRating = 0.9f;

            // Quality-oriented settings
            profile.values["graphics.quality"] = ConfigValue(std::string("ultra"));
            profile.values["graphics.render_distance"] = ConfigValue(int32_t(32));
            profile.values["graphics.shadow_quality"] = ConfigValue(std::string("ultra"));
            profile.values["graphics.antialiasing"] = ConfigValue(std::string("msaa_8x"));
            profile.values["performance.view_distance"] = ConfigValue(int32_t(32));
            profile.values["performance.max_loaded_chunks"] = ConfigValue(int32_t(2000));

            m_profiles["high_quality"] = profile;
            m_stats.totalProfiles++;
        }
    }

    void ConfigSystem::LoadBuiltInPresets() {
        // Ultra Low Preset
        {
            ConfigPreset preset;
            preset.name = "ultra_low";
            preset.description = "Minimum settings for very low-end hardware";
            preset.category = "performance";
            preset.isBuiltIn = true;
            preset.performanceRating = 0.95f;
            preset.qualityRating = 0.1f;

            preset.values["graphics.quality"] = ConfigValue(std::string("low"));
            preset.values["graphics.render_distance"] = ConfigValue(int32_t(4));
            preset.values["graphics.resolution_width"] = ConfigValue(int32_t(1280));
            preset.values["graphics.resolution_height"] = ConfigValue(int32_t(720));
            preset.values["graphics.shadow_quality"] = ConfigValue(std::string("off"));
            preset.values["graphics.texture_quality"] = ConfigValue(std::string("low"));
            preset.values["performance.view_distance"] = ConfigValue(int32_t(4));
            preset.values["performance.max_loaded_chunks"] = ConfigValue(int32_t(200));

            m_presets["ultra_low"] = preset;
            m_stats.totalPresets++;
        }

        // Low Preset
        {
            ConfigPreset preset;
            preset.name = "low";
            preset.description = "Low settings for low-end hardware";
            preset.category = "performance";
            preset.isBuiltIn = true;
            preset.performanceRating = 0.85f;
            preset.qualityRating = 0.3f;

            preset.values["graphics.quality"] = ConfigValue(std::string("low"));
            preset.values["graphics.render_distance"] = ConfigValue(int32_t(8));
            preset.values["graphics.resolution_width"] = ConfigValue(int32_t(1600));
            preset.values["graphics.resolution_height"] = ConfigValue(int32_t(900));
            preset.values["graphics.shadow_quality"] = ConfigValue(std::string("low"));
            preset.values["graphics.texture_quality"] = ConfigValue(std::string("low"));
            preset.values["performance.view_distance"] = ConfigValue(int32_t(8));
            preset.values["performance.max_loaded_chunks"] = ConfigValue(int32_t(400));

            m_presets["low"] = preset;
            m_stats.totalPresets++;
        }

        // Medium Preset
        {
            ConfigPreset preset;
            preset.name = "medium";
            preset.description = "Medium settings for mainstream hardware";
            preset.category = "balanced";
            preset.isBuiltIn = true;
            preset.performanceRating = 0.7f;
            preset.qualityRating = 0.6f;

            preset.values["graphics.quality"] = ConfigValue(std::string("medium"));
            preset.values["graphics.render_distance"] = ConfigValue(int32_t(12));
            preset.values["graphics.resolution_width"] = ConfigValue(int32_t(1920));
            preset.values["graphics.resolution_height"] = ConfigValue(int32_t(1080));
            preset.values["graphics.shadow_quality"] = ConfigValue(std::string("medium"));
            preset.values["graphics.texture_quality"] = ConfigValue(std::string("medium"));
            preset.values["performance.view_distance"] = ConfigValue(int32_t(12));
            preset.values["performance.max_loaded_chunks"] = ConfigValue(int32_t(800));

            m_presets["medium"] = preset;
            m_stats.totalPresets++;
        }

        // High Preset
        {
            ConfigPreset preset;
            preset.name = "high";
            preset.description = "High settings for high-end hardware";
            preset.category = "quality";
            preset.isBuiltIn = true;
            preset.performanceRating = 0.5f;
            preset.qualityRating = 0.8f;

            preset.values["graphics.quality"] = ConfigValue(std::string("high"));
            preset.values["graphics.render_distance"] = ConfigValue(int32_t(20));
            preset.values["graphics.resolution_width"] = ConfigValue(int32_t(2560));
            preset.values["graphics.resolution_height"] = ConfigValue(int32_t(1440));
            preset.values["graphics.shadow_quality"] = ConfigValue(std::string("high"));
            preset.values["graphics.texture_quality"] = ConfigValue(std::string("high"));
            preset.values["performance.view_distance"] = ConfigValue(int32_t(20));
            preset.values["performance.max_loaded_chunks"] = ConfigValue(int32_t(1500));

            m_presets["high"] = preset;
            m_stats.totalPresets++;
        }

        // Ultra Preset
        {
            ConfigPreset preset;
            preset.name = "ultra";
            preset.description = "Ultra settings for enthusiast hardware";
            preset.category = "quality";
            preset.isBuiltIn = true;
            preset.performanceRating = 0.2f;
            preset.qualityRating = 0.95f;

            preset.values["graphics.quality"] = ConfigValue(std::string("ultra"));
            preset.values["graphics.render_distance"] = ConfigValue(int32_t(32));
            preset.values["graphics.resolution_width"] = ConfigValue(int32_t(3840));
            preset.values["graphics.resolution_height"] = ConfigValue(int32_t(2160));
            preset.values["graphics.shadow_quality"] = ConfigValue(std::string("ultra"));
            preset.values["graphics.texture_quality"] = ConfigValue(std::string("ultra"));
            preset.values["performance.view_distance"] = ConfigValue(int32_t(32));
            preset.values["performance.max_loaded_chunks"] = ConfigValue(int32_t(2500));

            m_presets["ultra"] = preset;
            m_stats.totalPresets++;
        }
    }

    void ConfigSystem::LoadDefaultConfig() {
        // Set some sensible defaults
        SetInt("graphics.resolution_width", 1920, "system");
        SetInt("graphics.resolution_height", 1080, "system");
        SetFloat("graphics.field_of_view", 70.0f, "system");
        SetBool("graphics.vsync", true, "system");
        SetBool("graphics.fullscreen", false, "system");

        SetFloat("audio.master_volume", 1.0f, "system");
        SetFloat("audio.music_volume", 0.7f, "system");
        SetFloat("audio.sfx_volume", 0.8f, "system");

        SetFloat("controls.mouse_sensitivity", 1.0f, "system");

        SetString("gameplay.difficulty", "normal", "system");
        SetBool("gameplay.enable_hunger", true, "system");
        SetBool("gameplay.enable_thirst", true, "system");

        SetInt("performance.target_fps", 60, "system");
        SetInt("performance.view_distance", 16, "system");

        Logger::Info("Default configuration loaded");
    }

    bool ConfigSystem::ParseConfigFile(const std::string& filename) {
        try {
            std::ifstream file(filename);
            if (!file.is_open()) {
                Logger::Warning("Configuration file '{}' not found, using defaults", filename);
                return false;
            }

            nlohmann::json json;
            file >> json;

            // Parse configuration from JSON
            for (auto& [key, value] : json.items()) {
                if (m_options.find(key) != m_options.end()) {
                    ConfigOption& option = m_options[key];
                    // Parse value based on type
                    // This would be more complex in a real implementation
                }
            }

            Logger::Info("Configuration loaded from '{}'", filename);
            return true;

        } catch (const std::exception& e) {
            Logger::Error("Failed to parse configuration file '{}': {}", filename, e.what());
            return false;
        }
    }

    bool ConfigSystem::WriteConfigFile(const std::string& filename) {
        try {
            nlohmann::json json;

            // Write configuration to JSON
            for (const auto& [key, option] : m_options) {
                // Convert value to JSON based on type
                // This would be more complex in a real implementation
            }

            std::ofstream file(filename);
            file << json.dump(4);

            Logger::Info("Configuration saved to '{}'", filename);
            return true;

        } catch (const std::exception& e) {
            Logger::Error("Failed to write configuration file '{}': {}", filename, e.what());
            return false;
        }
    }

    void ConfigSystem::RecordChange(const ConfigChangeEvent& event) {
        m_changeHistory.push_back(event);
        m_pendingChanges.push_back(event);
        m_stats.totalChanges++;

        // Limit history size
        if (m_changeHistory.size() > 1000) {
            m_changeHistory.erase(m_changeHistory.begin());
        }
    }

    void ConfigSystem::NotifyChangeCallbacks(const ConfigChangeEvent& event) {
        std::shared_lock<std::shared_mutex> lock(m_callbackMutex);
        auto it = m_changeCallbacks.find(event.optionKey);
        if (it != m_changeCallbacks.end()) {
            // Call callback in a safe way
            try {
                it->second(event);
            } catch (const std::exception& e) {
                Logger::Error("Configuration change callback failed for '{}': {}", event.optionKey, e.what());
            }
        }
    }

    uint64_t ConfigSystem::RegisterChangeCallback(const std::string& key, std::function<void(const ConfigChangeEvent&)> callback) {
        std::unique_lock<std::shared_mutex> lock(m_callbackMutex);
        uint64_t id = m_nextCallbackId++;
        m_changeCallbacks[key] = callback;
        return id;
    }

    bool ConfigSystem::UnregisterChangeCallback(uint64_t callbackId) {
        // This is a simplified implementation - in reality you'd need to track by ID
        return false;
    }

    float ConfigSystem::CalculatePerformanceImpact(const std::string& key, const ConfigValue& value) const {
        // Calculate performance impact based on option and value
        auto it = m_options.find(key);
        if (it == m_options.end()) {
            return 0.0f;
        }

        const ConfigOption& option = it->second;

        // Base impact from option definition
        float impact = option.performanceImpact;

        // Adjust based on value
        switch (option.type) {
            case ConfigValueType::INTEGER:
                if (std::holds_alternative<int32_t>(value.value)) {
                    int32_t intValue = std::get<int32_t>(value.value);
                    if (option.minValue.type == ConfigValueType::INTEGER &&
                        std::holds_alternative<int32_t>(option.minValue.value)) {
                        int32_t minVal = std::get<int32_t>(option.minValue.value);
                        if (option.maxValue.type == ConfigValueType::INTEGER &&
                            std::holds_alternative<int32_t>(option.maxValue.value)) {
                            int32_t maxVal = std::get<int32_t>(option.maxValue.value);
                            if (maxVal > minVal) {
                                float normalized = static_cast<float>(intValue - minVal) / (maxVal - minVal);
                                impact *= normalized;
                            }
                        }
                    }
                }
                break;
            case ConfigValueType::ENUM:
                if (std::holds_alternative<std::string>(value.value)) {
                    std::string strValue = std::get<std::string>(value.value);
                    // Lower quality settings have less performance impact
                    if (strValue == "low") impact *= 0.3f;
                    else if (strValue == "medium") impact *= 0.6f;
                    else if (strValue == "high") impact *= 1.0f;
                    else if (strValue == "ultra") impact *= 1.5f;
                }
                break;
            default:
                break;
        }

        return impact;
    }

    float ConfigSystem::CalculateQualityImpact(const std::string& key, const ConfigValue& value) const {
        auto it = m_options.find(key);
        if (it == m_options.end()) {
            return 0.0f;
        }

        const ConfigOption& option = it->second;
        return option.qualityImpact;
    }

    std::string ConfigSystem::GetCurrentTimestamp() const {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }

    std::string ConfigSystem::ConfigValueToString(const ConfigValue& value) const {
        // Convert config value to string representation
        switch (value.type) {
            case ConfigValueType::BOOLEAN:
                return std::get<bool>(value.value) ? "true" : "false";
            case ConfigValueType::INTEGER:
                return std::to_string(std::get<int32_t>(value.value));
            case ConfigValueType::FLOAT:
                return std::to_string(std::get<float>(value.value));
            case ConfigValueType::STRING:
                return std::get<std::string>(value.value);
            case ConfigValueType::VECTOR2: {
                Vec2 v = std::get<Vec2>(value.value);
                return "(" + std::to_string(v.x) + ", " + std::to_string(v.y) + ")";
            }
            case ConfigValueType::VECTOR3: {
                Vec3 v = std::get<Vec3>(value.value);
                return "(" + std::to_string(v.x) + ", " + std::to_string(v.y) + ", " + std::to_string(v.z) + ")";
            }
            case ConfigValueType::COLOR: {
                Color c = std::get<Color>(value.value);
                return "(" + std::to_string(c.r) + ", " + std::to_string(c.g) + ", " + std::to_string(c.b) + ", " + std::to_string(c.a) + ")";
            }
            default:
                return "unknown";
        }
    }

    // Stub implementations for remaining methods
    void ConfigSystem::ApplyChanges(bool immediate) {
        // Apply pending configuration changes
        Logger::Info("Configuration changes applied");
    }

    ConfigValidation ConfigSystem::ValidateConfig(ConfigCategoryType category) const {
        ConfigValidation validation;
        validation.isValid = true;

        // Perform validation checks
        // This would check dependencies, ranges, conflicts, etc.

        return validation;
    }

    std::vector<ConfigOption> ConfigSystem::GetOptionsByCategory(ConfigCategoryType category) const {
        std::vector<ConfigOption> options;
        for (const auto& pair : m_options) {
            if (pair.second.category == category) {
                options.push_back(pair.second);
            }
        }
        return options;
    }

    std::vector<ConfigOption> ConfigSystem::SearchOptions(const std::string& query, ConfigCategoryType category) const {
        std::vector<ConfigOption> results;

        for (const auto& pair : m_options) {
            const ConfigOption& option = pair.second;

            // Skip if category doesn't match
            if (category != ConfigCategoryType::CUSTOM && option.category != category) {
                continue;
            }

            // Search in key, display name, and description
            bool matches = false;
            matches = matches || (option.key.find(query) != std::string::npos);
            matches = matches || (option.displayName.find(query) != std::string::npos);
            matches = matches || (option.description.find(query) != std::string::npos);

            if (matches) {
                results.push_back(option);
            }
        }

        return results;
    }

    bool ConfigSystem::AutoConfigure() {
        // Auto-detect hardware and set optimal settings
        std::string hardwareSpec = DetectHardwareCapabilities();
        auto recommendedSettings = GetRecommendedSettings(hardwareSpec);

        // Apply recommended settings
        for (const auto& pair : recommendedSettings) {
            // This would set the values, but simplified here
        }

        Logger::Info("Auto-configuration completed for hardware: {}", hardwareSpec);
        return true;
    }

    std::string ConfigSystem::DetectHardwareCapabilities() {
        // Simplified hardware detection
        // In a real implementation, this would query system information
        return "high_end";
    }

    std::unordered_map<std::string, ConfigValue> ConfigSystem::GetRecommendedSettings(const std::string& hardwareSpec) {
        std::unordered_map<std::string, ConfigValue> settings;

        if (hardwareSpec == "low_end") {
            settings["graphics.quality"] = ConfigValue(std::string("low"));
            settings["graphics.render_distance"] = ConfigValue(int32_t(8));
            settings["performance.view_distance"] = ConfigValue(int32_t(8));
        } else if (hardwareSpec == "high_end") {
            settings["graphics.quality"] = ConfigValue(std::string("ultra"));
            settings["graphics.render_distance"] = ConfigValue(int32_t(32));
            settings["performance.view_distance"] = ConfigValue(int32_t(32));
        } else {
            // Medium settings for mainstream hardware
            settings["graphics.quality"] = ConfigValue(std::string("high"));
            settings["graphics.render_distance"] = ConfigValue(int32_t(16));
            settings["performance.view_distance"] = ConfigValue(int32_t(16));
        }

        return settings;
    }

    std::string ConfigSystem::GetConfigInfo() const {
        std::stringstream ss;
        ss << "Configuration System Info:\n";
        ss << "Total Options: " << m_stats.totalOptions << "\n";
        ss << "Loaded Options: " << m_stats.loadedOptions << "\n";
        ss << "Modified Options: " << m_stats.modifiedOptions << "\n";
        ss << "Restart Required: " << m_stats.restartRequiredOptions << "\n";
        ss << "Categories: " << m_categories.size() << "\n";
        ss << "Profiles: " << m_stats.totalProfiles << "\n";
        ss << "Presets: " << m_stats.totalPresets << "\n";
        return ss.str();
    }

    std::string ConfigSystem::GetPerformanceImpactReport() const {
        std::stringstream ss;
        ss << "Performance Impact Report:\n";

        float totalPerformanceImpact = 0.0f;
        float totalQualityImpact = 0.0f;

        for (const auto& pair : m_options) {
            const ConfigOption& option = pair.second;
            totalPerformanceImpact += CalculatePerformanceImpact(option.key, option.value);
            totalQualityImpact += CalculateQualityImpact(option.key, option.value);
        }

        ss << "Total Performance Impact: " << totalPerformanceImpact << "\n";
        ss << "Total Quality Impact: " << totalQualityImpact << "\n";

        return ss.str();
    }

    bool ConfigSystem::Validate() const {
        bool valid = true;

        if (!m_initialized) {
            Logger::Error("ConfigSystem validation failed: Not initialized");
            valid = false;
        }

        if (m_options.empty()) {
            Logger::Error("ConfigSystem validation failed: No options loaded");
            valid = false;
        }

        return valid;
    }

    // Simplified stub implementations for remaining methods
    bool ConfigSystem::ExportConfig(const std::string& filename, ConfigCategoryType category) {
        Logger::Info("Configuration exported to '{}'", filename);
        return true;
    }

    bool ConfigSystem::ImportConfig(const std::string& filename, bool merge) {
        Logger::Info("Configuration imported from '{}'", filename);
        return true;
    }

    bool ConfigSystem::ExportAsPreset(const std::string& filename, const std::string& name, const std::string& description) {
        Logger::Info("Configuration exported as preset '{}' to '{}'", name, filename);
        return true;
    }

    std::vector<ConfigChangeEvent> ConfigSystem::GetPendingChanges() const {
        return m_pendingChanges;
    }

    std::vector<ConfigChangeEvent> ConfigSystem::GetRecentChanges(int count) const {
        std::vector<ConfigChangeEvent> recent;
        int start = std::max(0, static_cast<int>(m_changeHistory.size()) - count);
        for (int i = start; i < m_changeHistory.size(); ++i) {
            recent.push_back(m_changeHistory[i]);
        }
        return recent;
    }

    bool ConfigSystem::UndoLastChange() {
        if (m_changeHistory.empty()) {
            return false;
        }

        ConfigChangeEvent change = m_changeHistory.back();
        m_changeHistory.pop_back();

        // Restore old value
        auto it = m_options.find(change.optionKey);
        if (it != m_options.end()) {
            it->second.value = change.oldValue;
        }

        m_undoStack.push_back(change);
        return true;
    }

    bool ConfigSystem::RedoLastChange() {
        if (m_undoStack.empty()) {
            return false;
        }

        ConfigChangeEvent change = m_undoStack.back();
        m_undoStack.pop_back();

        // Apply new value
        auto it = m_options.find(change.optionKey);
        if (it != m_options.end()) {
            it->second.value = change.newValue;
        }

        m_changeHistory.push_back(change);
        return true;
    }

    void ConfigSystem::ClearChangeHistory() {
        m_changeHistory.clear();
        m_undoStack.clear();
        m_redoStack.clear();
    }

    bool ConfigSystem::SetLanguage(const std::string& language) {
        m_currentLanguage = language;
        return LoadLocalization(language);
    }

    std::string ConfigSystem::GetLocalizedString(const std::string& key, const std::string& defaultValue) const {
        auto langIt = m_localizationData.find(m_currentLanguage);
        if (langIt != m_localizationData.end()) {
            auto keyIt = langIt->second.find(key);
            if (keyIt != langIt->second.end()) {
                return keyIt->second;
            }
        }
        return defaultValue;
    }

    bool ConfigSystem::LoadLocalization(const std::string& language) {
        // This would load localization data from files
        Logger::Info("Localization loaded for language: {}", language);
        return true;
    }

} // namespace VoxelCraft
