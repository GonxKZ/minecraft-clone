/**
 * @file Config.cpp
 * @brief VoxelCraft Engine Configuration Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file implements the configuration management system
 * for the VoxelCraft engine with support for multiple formats
 * and thread-safe operations.
 */

#include "Config.hpp"
#include "Logger.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <regex>
#include <chrono>
#include <stdexcept>

namespace VoxelCraft {

    // Config implementation

    Config::Config()
        : m_currentProfile("default")
        , m_profileDirectory("config/profiles")
        , m_loadCount(0)
        , m_saveCount(0)
        , m_changeCount(0)
        , m_autoSave(false)
        , m_validateOnLoad(true)
        , m_notifyOnLoad(false)
        , m_nextCallbackId(1)
    {
        VOXELCRAFT_TRACE("Config instance created");
    }

    Config::~Config() {
        VOXELCRAFT_TRACE("Config instance destroyed");
    }

    bool Config::LoadFromFile(const std::string& filename) {
        std::lock_guard<std::mutex> lock(m_mutex);

        try {
            std::ifstream file(filename);
            if (!file.is_open()) {
                VOXELCRAFT_ERROR("Failed to open configuration file: {}", filename);
                return false;
            }

            std::string content((std::istreambuf_iterator<char>(file)),
                               std::istreambuf_iterator<char>());
            file.close();

            std::string extension = filename.substr(filename.find_last_of('.') + 1);
            bool success = false;

            if (extension == "toml") {
                success = ParseTOML(content);
            } else if (extension == "json") {
                success = ParseJSON(content);
            } else if (extension == "ini") {
                success = ParseINI(content);
            } else {
                // Try to detect format from content
                if (content.find("[[") != std::string::npos || content.find('[') == 0) {
                    success = ParseTOML(content);
                } else if (content.find('{') != std::string::npos) {
                    success = ParseJSON(content);
                } else {
                    success = ParseINI(content);
                }
            }

            if (success) {
                m_loadCount++;
                VOXELCRAFT_INFO("Configuration loaded from file: {}", filename);
            }

            return success;

        } catch (const std::exception& e) {
            VOXELCRAFT_ERROR("Exception loading configuration from file {}: {}", filename, e.what());
            return false;
        }
    }

    bool Config::LoadFromString(const std::string& content, const std::string& format) {
        std::lock_guard<std::mutex> lock(m_mutex);

        try {
            bool success = false;

            if (format == "toml") {
                success = ParseTOML(content);
            } else if (format == "json") {
                success = ParseJSON(content);
            } else if (format == "ini") {
                success = ParseINI(content);
            } else {
                // Auto-detect format
                if (content.find("[[") != std::string::npos || content.find('[') == 0) {
                    success = ParseTOML(content);
                } else if (content.find('{') != std::string::npos) {
                    success = ParseJSON(content);
                } else {
                    success = ParseINI(content);
                }
            }

            if (success) {
                m_loadCount++;
                VOXELCRAFT_INFO("Configuration loaded from string (format: {})", format);
            }

            return success;

        } catch (const std::exception& e) {
            VOXELCRAFT_ERROR("Exception loading configuration from string: {}", e.what());
            return false;
        }
    }

    bool Config::LoadFromEnvironment(const std::string& prefix) {
        std::lock_guard<std::mutex> lock(m_mutex);

        try {
            // Simple environment variable loading
            // In a real implementation, you'd iterate through all env vars
            // For now, we'll load some common ones
            if (const char* value = std::getenv((prefix + "DEBUG").c_str())) {
                Set("engine.debug_mode", std::string(value) == "true");
            }
            if (const char* value = std::getenv((prefix + "LOG_LEVEL").c_str())) {
                Set("logging.level", std::string(value));
            }
            if (const char* value = std::getenv((prefix + "SERVER_PORT").c_str())) {
                Set("network.server_port", std::stoi(value));
            }

            VOXELCRAFT_INFO("Configuration loaded from environment variables (prefix: {})", prefix);
            return true;

        } catch (const std::exception& e) {
            VOXELCRAFT_ERROR("Exception loading configuration from environment: {}", e.what());
            return false;
        }
    }

    bool Config::LoadFromCommandLine(int argc, char* argv[]) {
        std::lock_guard<std::mutex> lock(m_mutex);

        try {
            // Simple command line argument parsing
            for (int i = 1; i < argc; ++i) {
                std::string arg = argv[i];
                if (arg.find("--config.") == 0) {
                    std::string key = arg.substr(9);
                    if (i + 1 < argc) {
                        std::string value = argv[++i];
                        Set(key, value, "command_line");
                    }
                }
            }

            VOXELCRAFT_INFO("Configuration loaded from command line arguments");
            return true;

        } catch (const std::exception& e) {
            VOXELCRAFT_ERROR("Exception loading configuration from command line: {}", e.what());
            return false;
        }
    }

    bool Config::SaveToFile(const std::string& filename, const std::string& format) {
        std::lock_guard<std::mutex> lock(m_mutex);

        try {
            std::string content = SaveToString(format);

            std::ofstream file(filename);
            if (!file.is_open()) {
                VOXELCRAFT_ERROR("Failed to open file for writing: {}", filename);
                return false;
            }

            file << content;
            file.close();

            m_saveCount++;
            VOXELCRAFT_INFO("Configuration saved to file: {}", filename);
            return true;

        } catch (const std::exception& e) {
            VOXELCRAFT_ERROR("Exception saving configuration to file {}: {}", filename, e.what());
            return false;
        }
    }

    std::string Config::SaveToString(const std::string& format) {
        std::lock_guard<std::mutex> lock(m_mutex);

        try {
            if (format == "toml") {
                return GenerateTOML();
            } else if (format == "json") {
                return GenerateJSON();
            } else if (format == "ini") {
                return GenerateINI();
            } else {
                return GenerateTOML(); // Default to TOML
            }

        } catch (const std::exception& e) {
            VOXELCRAFT_ERROR("Exception generating configuration string: {}", e.what());
            return "";
        }
    }

    bool Config::Has(const std::string& key) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::string normalizedKey = NormalizeKey(key);
        return m_values.count(normalizedKey) > 0;
    }

    void Config::Remove(const std::string& key, const std::string& source) {
        std::lock_guard<std::mutex> lock(m_mutex);

        std::string normalizedKey = NormalizeKey(key);
        auto it = m_values.find(normalizedKey);
        if (it != m_values.end()) {
            ConfigValue oldValue = it->second;
            m_values.erase(it);
            m_changeCount++;

            ConfigChangeEvent event{
                normalizedKey,
                oldValue,
                std::string{},
                source,
                static_cast<double>(std::chrono::system_clock::now().time_since_epoch().count()) / 1e9
            };
            NotifyCallbacks(event);
        }
    }

    void Config::Clear(const std::string& source) {
        std::lock_guard<std::mutex> lock(m_mutex);

        std::unordered_map<std::string, ConfigValue> oldValues = m_values;
        m_values.clear();
        m_changeCount++;

        for (const auto& pair : oldValues) {
            ConfigChangeEvent event{
                pair.first,
                pair.second,
                std::string{},
                source,
                static_cast<double>(std::chrono::system_clock::now().time_since_epoch().count()) / 1e9
            };
            NotifyCallbacks(event);
        }
    }

    std::vector<std::string> Config::GetKeys(const std::string& pattern) const {
        std::lock_guard<std::mutex> lock(m_mutex);

        std::vector<std::string> keys;
        std::regex regexPattern(pattern);

        for (const auto& pair : m_values) {
            if (std::regex_match(pair.first, regexPattern)) {
                keys.push_back(pair.first);
            }
        }

        return keys;
    }

    std::unique_ptr<Config> Config::GetSubtree(const std::string& prefix) const {
        std::lock_guard<std::mutex> lock(m_mutex);

        auto subtree = std::make_unique<Config>();
        std::string normalizedPrefix = NormalizeKey(prefix);

        for (const auto& pair : m_values) {
            if (pair.first.find(normalizedPrefix) == 0) {
                std::string subKey = pair.first.substr(normalizedPrefix.length());
                if (!subKey.empty() && subKey[0] == '.') {
                    subKey = subKey.substr(1);
                }
                subtree->m_values[subKey] = pair.second;
            }
        }

        return subtree;
    }

    void Config::Merge(const Config& other, const std::string& source) {
        std::lock_guard<std::mutex> lock(m_mutex);

        for (const auto& pair : other.m_values) {
            Set(pair.first, pair.second, source);
        }
    }

    std::vector<std::string> Config::Validate(const Config& schema) const {
        std::lock_guard<std::mutex> lock(m_mutex);

        std::vector<std::string> errors;
        // Simple validation - check if required keys exist
        for (const auto& pair : schema.m_values) {
            if (!Has(pair.first)) {
                errors.push_back("Missing required configuration key: " + pair.first);
            }
        }

        return errors;
    }

    uint64_t Config::RegisterCallback(ConfigChangeCallback callback,
                                    const std::vector<std::string>& keys) {
        std::lock_guard<std::mutex> lock(m_mutex);

        CallbackInfo info{callback, keys, m_nextCallbackId++};
        m_callbacks.push_back(info);

        return info.id;
    }

    void Config::UnregisterCallback(uint64_t callbackId) {
        std::lock_guard<std::mutex> lock(m_mutex);

        m_callbacks.erase(
            std::remove_if(m_callbacks.begin(), m_callbacks.end(),
                [callbackId](const CallbackInfo& info) {
                    return info.id == callbackId;
                }
            ),
            m_callbacks.end()
        );
    }

    bool Config::LoadProfile(const std::string& profile) {
        std::lock_guard<std::mutex> lock(m_mutex);

        std::string filename = m_profileDirectory.string() + "/" + profile + ".toml";
        if (LoadFromFile(filename)) {
            m_currentProfile = profile;
            return true;
        }
        return false;
    }

    bool Config::SaveProfile(const std::string& profile) {
        std::lock_guard<std::mutex> lock(m_mutex);

        std::filesystem::create_directories(m_profileDirectory);
        std::string filename = m_profileDirectory.string() + "/" + profile + ".toml";

        if (SaveToFile(filename)) {
            m_currentProfile = profile;
            return true;
        }
        return false;
    }

    std::vector<std::string> Config::ListProfiles() const {
        std::lock_guard<std::mutex> lock(m_mutex);

        std::vector<std::string> profiles;

        if (std::filesystem::exists(m_profileDirectory)) {
            for (const auto& entry : std::filesystem::directory_iterator(m_profileDirectory)) {
                if (entry.path().extension() == ".toml") {
                    profiles.push_back(entry.path().stem().string());
                }
            }
        }

        return profiles;
    }

    std::optional<ConfigValueType> Config::GetType(const std::string& key) const {
        std::lock_guard<std::mutex> lock(m_mutex);

        std::string normalizedKey = NormalizeKey(key);
        auto it = m_values.find(normalizedKey);

        if (it == m_values.end()) {
            return std::nullopt;
        }

        if (std::holds_alternative<std::string>(it->second)) {
            return ConfigValueType::String;
        } else if (std::holds_alternative<int64_t>(it->second)) {
            return ConfigValueType::Integer;
        } else if (std::holds_alternative<double>(it->second)) {
            return ConfigValueType::Float;
        } else if (std::holds_alternative<bool>(it->second)) {
            return ConfigValueType::Boolean;
        } else if (std::holds_alternative<std::vector<std::string>>(it->second)) {
            return ConfigValueType::Array;
        } else if (std::holds_alternative<std::unordered_map<std::string, std::string>>(it->second)) {
            return ConfigValueType::Object;
        }

        return std::nullopt;
    }

    size_t Config::Size() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_values.size();
    }

    bool Config::Empty() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_values.empty();
    }

    std::unordered_map<std::string, size_t> Config::GetStatistics() const {
        std::lock_guard<std::mutex> lock(m_mutex);

        return {
            {"total_keys", m_values.size()},
            {"load_count", m_loadCount},
            {"save_count", m_saveCount},
            {"change_count", m_changeCount},
            {"callback_count", m_callbacks.size()}
        };
    }

    // Parsing methods

    bool Config::ParseTOML(const std::string& content) {
        // Simple TOML parser - in a real implementation you'd use a proper TOML library
        std::istringstream stream(content);
        std::string line;
        std::string currentSection;

        while (std::getline(stream, line)) {
            // Remove comments
            size_t commentPos = line.find('#');
            if (commentPos != std::string::npos) {
                line = line.substr(0, commentPos);
            }

            // Trim whitespace
            line.erase(line.begin(), std::find_if(line.begin(), line.end(), [](char c) {
                return !std::isspace(c);
            }));
            line.erase(std::find_if(line.rbegin(), line.rend(), [](char c) {
                return !std::isspace(c);
            }).base(), line.end());

            if (line.empty()) continue;

            // Section header
            if (line[0] == '[' && line.back() == ']') {
                currentSection = line.substr(1, line.length() - 2);
                continue;
            }

            // Key-value pair
            size_t equalsPos = line.find('=');
            if (equalsPos != std::string::npos) {
                std::string key = line.substr(0, equalsPos);
                std::string value = line.substr(equalsPos + 1);

                // Trim whitespace
                key.erase(key.begin(), std::find_if(key.begin(), key.end(), [](char c) {
                    return !std::isspace(c);
                }));
                key.erase(std::find_if(key.rbegin(), key.rend(), [](char c) {
                    return !std::isspace(c);
                }).base(), key.end());

                value.erase(value.begin(), std::find_if(value.begin(), value.end(), [](char c) {
                    return !std::isspace(c);
                }));
                value.erase(std::find_if(value.rbegin(), value.rend(), [](char c) {
                    return !std::isspace(c);
                }).base(), value.end());

                // Remove quotes from value
                if (!value.empty() && value[0] == '"' && value.back() == '"') {
                    value = value.substr(1, value.length() - 2);
                }

                // Add section prefix to key
                if (!currentSection.empty()) {
                    key = currentSection + "." + key;
                }

                // Parse value
                if (value == "true" || value == "false") {
                    Set(key, value == "true");
                } else if (value.find('.') != std::string::npos) {
                    try {
                        Set(key, std::stod(value));
                    } catch (...) {
                        Set(key, value);
                    }
                } else {
                    try {
                        Set(key, std::stoll(value));
                    } catch (...) {
                        Set(key, value);
                    }
                }
            }
        }

        return true;
    }

    bool Config::ParseJSON(const std::string& content) {
        // Simple JSON parser - in a real implementation you'd use a proper JSON library
        // For now, just return true as if it parsed successfully
        VOXELCRAFT_WARN("JSON parsing not fully implemented yet");
        return true;
    }

    bool Config::ParseINI(const std::string& content) {
        // Simple INI parser - similar to TOML parser but without sections
        std::istringstream stream(content);
        std::string line;

        while (std::getline(stream, line)) {
            // Remove comments
            size_t commentPos = line.find(';');
            if (commentPos != std::string::npos) {
                line = line.substr(0, commentPos);
            }

            // Trim whitespace
            line.erase(line.begin(), std::find_if(line.begin(), line.end(), [](char c) {
                return !std::isspace(c);
            }));
            line.erase(std::find_if(line.rbegin(), line.rend(), [](char c) {
                return !std::isspace(c);
            }).base(), line.end());

            if (line.empty()) continue;

            // Key-value pair
            size_t equalsPos = line.find('=');
            if (equalsPos != std::string::npos) {
                std::string key = line.substr(0, equalsPos);
                std::string value = line.substr(equalsPos + 1);

                // Trim whitespace
                key.erase(key.begin(), std::find_if(key.begin(), key.end(), [](char c) {
                    return !std::isspace(c);
                }));
                key.erase(std::find_if(key.rbegin(), key.rend(), [](char c) {
                    return !std::isspace(c);
                }).base(), key.end());

                value.erase(value.begin(), std::find_if(value.begin(), value.end(), [](char c) {
                    return !std::isspace(c);
                }));
                value.erase(std::find_if(value.rbegin(), value.rend(), [](char c) {
                    return !std::isspace(c);
                }).base(), value.end());

                // Parse value similar to TOML
                if (value == "true" || value == "false") {
                    Set(key, value == "true");
                } else if (value.find('.') != std::string::npos) {
                    try {
                        Set(key, std::stod(value));
                    } catch (...) {
                        Set(key, value);
                    }
                } else {
                    try {
                        Set(key, std::stoll(value));
                    } catch (...) {
                        Set(key, value);
                    }
                }
            }
        }

        return true;
    }

    std::string Config::GenerateTOML() const {
        std::stringstream ss;
        std::map<std::string, std::string> sections;

        // Group keys by section
        for (const auto& pair : m_values) {
            size_t dotPos = pair.first.find('.');
            std::string section = (dotPos != std::string::npos) ?
                pair.first.substr(0, dotPos) : "general";
            std::string key = (dotPos != std::string::npos) ?
                pair.first.substr(dotPos + 1) : pair.first;

            std::string valueStr;
            if (std::holds_alternative<bool>(pair.second)) {
                valueStr = std::get<bool>(pair.second) ? "true" : "false";
            } else if (std::holds_alternative<int64_t>(pair.second)) {
                valueStr = std::to_string(std::get<int64_t>(pair.second));
            } else if (std::holds_alternative<double>(pair.second)) {
                valueStr = std::to_string(std::get<double>(pair.second));
            } else if (std::holds_alternative<std::string>(pair.second)) {
                valueStr = "\"" + std::get<std::string>(pair.second) + "\"";
            } else {
                valueStr = "\"<complex_value>\"";
            }

            sections[section] += key + " = " + valueStr + "\n";
        }

        // Generate TOML content
        for (const auto& section : sections) {
            if (section.first != "general") {
                ss << "[" << section.first << "]\n";
            }
            ss << section.second << "\n";
        }

        return ss.str();
    }

    std::string Config::GenerateJSON() const {
        std::stringstream ss;
        ss << "{\n";

        bool first = true;
        for (const auto& pair : m_values) {
            if (!first) ss << ",\n";
            first = false;

            ss << "  \"" << pair.first << "\": ";

            if (std::holds_alternative<bool>(pair.second)) {
                ss << (std::get<bool>(pair.second) ? "true" : "false");
            } else if (std::holds_alternative<int64_t>(pair.second)) {
                ss << std::get<int64_t>(pair.second);
            } else if (std::holds_alternative<double>(pair.second)) {
                ss << std::get<double>(pair.second);
            } else if (std::holds_alternative<std::string>(pair.second)) {
                ss << "\"" << std::get<std::string>(pair.second) << "\"";
            } else {
                ss << "\"<complex_value>\"";
            }
        }

        ss << "\n}";
        return ss.str();
    }

    std::string Config::GenerateINI() const {
        std::stringstream ss;

        for (const auto& pair : m_values) {
            ss << pair.first << " = ";

            if (std::holds_alternative<bool>(pair.second)) {
                ss << (std::get<bool>(pair.second) ? "true" : "false");
            } else if (std::holds_alternative<int64_t>(pair.second)) {
                ss << std::get<int64_t>(pair.second);
            } else if (std::holds_alternative<double>(pair.second)) {
                ss << std::get<double>(pair.second);
            } else if (std::holds_alternative<std::string>(pair.second)) {
                ss << std::get<std::string>(pair.second);
            } else {
                ss << "<complex_value>";
            }

            ss << "\n";
        }

        return ss.str();
    }

    void Config::NotifyCallbacks(const ConfigChangeEvent& event) {
        for (const auto& callbackInfo : m_callbacks) {
            if (callbackInfo.keys.empty()) {
                // Callback for all keys
                try {
                    callbackInfo.callback(event);
                } catch (const std::exception& e) {
                    VOXELCRAFT_ERROR("Exception in configuration callback: {}", e.what());
                }
            } else {
                // Callback for specific keys
                for (const auto& key : callbackInfo.keys) {
                    if (key == event.key) {
                        try {
                            callbackInfo.callback(event);
                        } catch (const std::exception& e) {
                            VOXELCRAFT_ERROR("Exception in configuration callback: {}", e.what());
                        }
                        break;
                    }
                }
            }
        }
    }

    std::string Config::NormalizeKey(const std::string& key) const {
        std::string normalized = key;
        std::transform(normalized.begin(), normalized.end(), normalized.begin(), ::tolower);
        return normalized;
    }

    bool Config::ValidateKey(const std::string& key) const {
        // Basic key validation - no empty keys, no special characters except dots and underscores
        if (key.empty()) return false;

        for (char c : key) {
            if (!std::isalnum(c) && c != '.' && c != '_' && c != '-') {
                return false;
            }
        }

        return true;
    }

    // Utility functions

    std::unique_ptr<Config> CreateDefaultConfig() {
        auto config = std::make_unique<Config>();

        // Engine settings
        config->Set("engine.target_fps", 60.0);
        config->Set("engine.fixed_timestep", true);
        config->Set("engine.max_frame_skip", 5);
        config->Set("engine.debug_mode", false);
        config->Set("engine.vsync", true);

        // Graphics settings
        config->Set("graphics.width", 1920);
        config->Set("graphics.height", 1080);
        config->Set("graphics.fullscreen", false);
        config->Set("graphics.fov", 90.0);
        config->Set("graphics.near_plane", 0.1);
        config->Set("graphics.far_plane", 1000.0);

        // World settings
        config->Set("world.seed", 12345);
        config->Set("world.name", "world");
        config->Set("world.chunk_size", 16);
        config->Set("world.render_distance", 8);

        // Player settings
        config->Set("player.movement_speed", 5.0);
        config->Set("player.jump_height", 1.5);
        config->Set("player.gravity", -9.81);

        // Network settings
        config->Set("network.server_mode", false);
        config->Set("network.client_mode", false);
        config->Set("network.server_port", 25565);
        config->Set("network.max_players", 10);

        // Audio settings
        config->Set("audio.master_volume", 1.0);
        config->Set("audio.music_volume", 0.7);
        config->Set("audio.sfx_volume", 0.8);

        // Debug settings
        config->Set("debug.show_fps", false);
        config->Set("debug.enable_debug_renderer", false);
        config->Set("debug.log_level", "info");

        return config;
    }

    std::unique_ptr<Config> LoadConfigWithFallback(
        const std::string& primaryPath,
        const std::vector<std::string>& fallbackPaths
    ) {
        auto config = std::make_unique<Config>();

        // Try primary path first
        if (config->LoadFromFile(primaryPath)) {
            return config;
        }

        // Try fallback paths
        for (const auto& path : fallbackPaths) {
            if (config->LoadFromFile(path)) {
                VOXELCRAFT_INFO("Loaded configuration from fallback path: {}", path);
                return config;
            }
        }

        // Load defaults if nothing works
        VOXELCRAFT_WARN("No configuration files found, using defaults");
        return CreateDefaultConfig();
    }

} // namespace VoxelCraft
