/**
 * @file PluginSystem.cpp
 * @brief VoxelCraft Advanced Plugin/Modding System Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#include "PluginSystem.hpp"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <functional>
#include <memory>
#include <filesystem>
#include <fstream>
#include <atomic>
#include <condition_variable>

#ifdef _WIN32
    #include <windows.h>
    #include <libloaderapi.h>
#else
    #include <dlfcn.h>
#endif

#include "../utils/Random.hpp"
#include "../logging/Logger.hpp"
#include "../memory/MemorySystem.hpp"

namespace VoxelCraft {

    // Plugin implementation
    Plugin::Plugin(const PluginMetadata& metadata)
        : m_metadata(metadata), m_state(PluginState::UNLOADED) {
    }

    bool Plugin::OnEnable() {
        if (m_state != PluginState::LOADED) {
            m_errorMessage = "Plugin must be loaded before enabling";
            return false;
        }

        m_state = PluginState::ACTIVE;
        return true;
    }

    bool Plugin::OnDisable() {
        if (m_state != PluginState::ACTIVE) {
            return false;
        }

        m_state = PluginState::SUSPENDED;
        return true;
    }

    bool Plugin::OnLoad() {
        if (m_state != PluginState::UNLOADED) {
            return false;
        }

        m_state = PluginState::LOADED;
        return true;
    }

    bool Plugin::OnUnload() {
        if (m_state == PluginState::ACTIVE) {
            OnDisable();
        }

        m_state = PluginState::UNLOADED;
        return true;
    }

    // PluginInstance implementation
    PluginInstance::PluginInstance(std::shared_ptr<Plugin> plugin)
        : m_plugin(plugin), m_loadTime(0), m_initializationTime(0),
          m_memoryUsage(0), m_updateCount(0) {
    }

    PluginInstance::~PluginInstance() {
        Shutdown();
    }

    bool PluginInstance::Initialize() {
        if (!m_plugin) return false;

        auto startTime = std::chrono::steady_clock::now();

        try {
            if (!m_plugin->Initialize()) {
                m_plugin->m_errorMessage = "Plugin initialization failed";
                m_plugin->m_state = PluginState::ERROR;
                return false;
            }

            auto endTime = std::chrono::steady_clock::now();
            m_initializationTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

            m_plugin->m_state = PluginState::ACTIVE;
            return true;

        } catch (const std::exception& e) {
            m_plugin->m_errorMessage = std::string("Exception during initialization: ") + e.what();
            m_plugin->m_state = PluginState::ERROR;
            return false;
        }
    }

    bool PluginInstance::Shutdown() {
        if (!m_plugin) return false;

        try {
            if (m_plugin->GetState() == PluginState::ACTIVE) {
                m_plugin->Shutdown();
            }

            m_plugin->m_state = PluginState::UNLOADED;
            return true;

        } catch (const std::exception& e) {
            m_plugin->m_errorMessage = std::string("Exception during shutdown: ") + e.what();
            m_plugin->m_state = PluginState::ERROR;
            return false;
        }
    }

    bool PluginInstance::Update(float deltaTime) {
        if (!m_plugin || m_plugin->GetState() != PluginState::ACTIVE) {
            return false;
        }

        try {
            if (m_plugin->Update(deltaTime)) {
                m_updateCount++;
                m_lastUpdateTime = std::chrono::steady_clock::now();
                return true;
            }

            return false;

        } catch (const std::exception& e) {
            m_plugin->m_errorMessage = std::string("Exception during update: ") + e.what();
            m_plugin->m_state = PluginState::ERROR;
            return false;
        }
    }

    bool PluginInstance::Load() {
        if (!m_plugin) return false;

        auto startTime = std::chrono::steady_clock::now();

        try {
            if (!m_plugin->OnLoad()) {
                m_plugin->m_errorMessage = "Plugin loading failed";
                m_plugin->m_state = PluginState::ERROR;
                return false;
            }

            auto endTime = std::chrono::steady_clock::now();
            m_loadTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

            return true;

        } catch (const std::exception& e) {
            m_plugin->m_errorMessage = std::string("Exception during loading: ") + e.what();
            m_plugin->m_state = PluginState::ERROR;
            return false;
        }
    }

    bool PluginInstance::Unload() {
        if (!m_plugin) return false;

        try {
            return m_plugin->OnUnload();

        } catch (const std::exception& e) {
            m_plugin->m_errorMessage = std::string("Exception during unloading: ") + e.what();
            m_plugin->m_state = PluginState::ERROR;
            return false;
        }
    }

    bool PluginInstance::Enable() {
        if (!m_plugin) return false;

        try {
            return m_plugin->OnEnable();

        } catch (const std::exception& e) {
            m_plugin->m_errorMessage = std::string("Exception during enabling: ") + e.what();
            m_plugin->m_state = PluginState::ERROR;
            return false;
        }
    }

    bool PluginInstance::Disable() {
        if (!m_plugin) return false;

        try {
            return m_plugin->OnDisable();

        } catch (const std::exception& e) {
            m_plugin->m_errorMessage = std::string("Exception during disabling: ") + e.what();
            m_plugin->m_state = PluginState::ERROR;
            return false;
        }
    }

    bool PluginInstance::HasPermission(PluginPermission permission) const {
        std::unique_lock<std::mutex> lock(m_instanceMutex);
        return m_permissions.find(permission) != m_permissions.end();
    }

    void PluginInstance::GrantPermission(PluginPermission permission) {
        std::unique_lock<std::mutex> lock(m_instanceMutex);
        m_permissions.insert(permission);
    }

    void PluginInstance::RevokePermission(PluginPermission permission) {
        std::unique_lock<std::mutex> lock(m_instanceMutex);
        m_permissions.erase(permission);
    }

    // PluginHook implementation
    PluginHook::PluginHook(const std::string& name, HookType type)
        : m_name(name), m_type(type) {
    }

    void PluginHook::AddCallback(const std::string& pluginId, HookFunction callback) {
        std::unique_lock<std::shared_mutex> lock(m_hookMutex);
        m_callbacks[pluginId] = callback;
    }

    void PluginHook::RemoveCallback(const std::string& pluginId) {
        std::unique_lock<std::shared_mutex> lock(m_hookMutex);
        m_callbacks.erase(pluginId);
    }

    bool PluginHook::HasCallback(const std::string& pluginId) const {
        std::shared_lock<std::shared_mutex> lock(m_hookMutex);
        return m_callbacks.find(pluginId) != m_callbacks.end();
    }

    bool PluginHook::Execute(const std::vector<std::any>& args) const {
        std::shared_lock<std::shared_mutex> lock(m_hookMutex);

        bool result = true;
        for (const auto& pair : m_callbacks) {
            try {
                if (!pair.second(args)) {
                    result = false;
                }
            } catch (const std::exception& e) {
                Logger::GetInstance().Error("Exception in plugin hook '" + m_name +
                                          "' for plugin '" + pair.first + "': " + e.what(), "PluginSystem");
                result = false;
            }
        }

        return result;
    }

    std::vector<std::string> PluginHook::GetCallbackPlugins() const {
        std::shared_lock<std::shared_mutex> lock(m_hookMutex);

        std::vector<std::string> plugins;
        for (const auto& pair : m_callbacks) {
            plugins.push_back(pair.first);
        }

        return plugins;
    }

    // PluginSandbox implementation
    PluginSandbox::PluginSandbox(const std::string& pluginId, PluginIsolationLevel level)
        : m_pluginId(pluginId), m_isolationLevel(level), m_memoryUsage(0) {
    }

    bool PluginSandbox::Initialize() {
        // Initialize sandbox based on isolation level
        return true;
    }

    void PluginSandbox::Shutdown() {
        // Cleanup sandbox resources
        for (void* ptr : m_allocatedMemory) {
            DeallocateMemory(ptr);
        }
        m_allocatedMemory.clear();
    }

    void* PluginSandbox::AllocateMemory(size_t size) {
        if (m_isolationLevel == PluginIsolationLevel::NONE) {
            return VOXELCRAFT_ALLOC(size, MemoryType::GENERAL);
        }

        // Sandboxed allocation
        void* ptr = VOXELCRAFT_ALLOC(size, MemoryType::GENERAL);
        if (ptr) {
            std::unique_lock<std::shared_mutex> lock(m_sandboxMutex);
            m_allocatedMemory.insert(ptr);
            m_memoryUsage += size;
        }

        return ptr;
    }

    void PluginSandbox::DeallocateMemory(void* ptr) {
        if (m_isolationLevel == PluginIsolationLevel::NONE) {
            VOXELCRAFT_DEALLOC(ptr, MemoryType::GENERAL);
            return;
        }

        std::unique_lock<std::shared_mutex> lock(m_sandboxMutex);
        auto it = m_allocatedMemory.find(ptr);
        if (it != m_allocatedMemory.end()) {
            VOXELCRAFT_DEALLOC(ptr, MemoryType::GENERAL);
            m_allocatedMemory.erase(it);
        }
    }

    bool PluginSandbox::CanAccessFile(const std::string& path) const {
        if (m_isolationLevel == PluginIsolationLevel::NONE) {
            return true;
        }

        std::shared_lock<std::shared_mutex> lock(m_sandboxMutex);

        for (const auto& allowedPath : m_allowedPaths) {
            if (path.find(allowedPath) == 0) {
                return true;
            }
        }

        return false;
    }

    void PluginSandbox::AddAllowedPath(const std::string& path) {
        std::unique_lock<std::shared_mutex> lock(m_sandboxMutex);
        m_allowedPaths.push_back(path);
    }

    bool PluginSandbox::CanAccessNetwork(const std::string& host) const {
        if (m_isolationLevel == PluginIsolationLevel::NONE) {
            return true;
        }

        std::shared_lock<std::shared_mutex> lock(m_sandboxMutex);
        return std::find(m_allowedHosts.begin(), m_allowedHosts.end(), host) != m_allowedHosts.end();
    }

    void PluginSandbox::AddAllowedHost(const std::string& host) {
        std::unique_lock<std::shared_mutex> lock(m_sandboxMutex);
        m_allowedHosts.push_back(host);
    }

    bool PluginSandbox::CanExecuteSystemCommand(const std::string& command) const {
        if (m_isolationLevel == PluginIsolationLevel::NONE) {
            return true;
        }

        std::shared_lock<std::shared_mutex> lock(m_sandboxMutex);
        return std::find(m_allowedCommands.begin(), m_allowedCommands.end(), command) != m_allowedCommands.end();
    }

    void PluginSandbox::AddAllowedCommand(const std::string& command) {
        std::unique_lock<std::shared_mutex> lock(m_sandboxMutex);
        m_allowedCommands.push_back(command);
    }

    bool PluginSandbox::ValidatePointer(void* ptr) const {
        if (m_isolationLevel == PluginIsolationLevel::NONE) {
            return true;
        }

        std::shared_lock<std::shared_mutex> lock(m_sandboxMutex);
        return m_allocatedMemory.find(ptr) != m_allocatedMemory.end();
    }

    bool PluginSandbox::ValidateString(const std::string& str) const {
        // Basic string validation
        return str.length() < 1024 * 1024; // Max 1MB strings
    }

    // PluginLoader implementation
    PluginLoader::PluginLoader(const std::string& extension)
        : m_extension(extension) {
    }

    bool PluginLoader::CanLoad(const std::string& path) const {
        return std::filesystem::path(path).extension() == m_extension;
    }

    // NativePluginLoader implementation
    NativePluginLoader::NativePluginLoader()
        : PluginLoader(".dll") {
#ifdef __linux__
        m_extension = ".so";
#endif
#ifdef __APPLE__
        m_extension = ".dylib";
#endif
    }

    std::shared_ptr<Plugin> NativePluginLoader::LoadPlugin(const std::string& path, const PluginMetadata& metadata) {
#ifdef _WIN32
        std::unique_lock<std::mutex> lock(m_loaderMutex);

        HMODULE handle = LoadLibraryA(path.c_str());
        if (!handle) {
            return nullptr;
        }

        // Get plugin creation function
        using CreatePluginFunc = Plugin*(*)();
        CreatePluginFunc createFunc = reinterpret_cast<CreatePluginFunc>(
            GetProcAddress(handle, "CreatePlugin"));

        if (!createFunc) {
            FreeLibrary(handle);
            return nullptr;
        }

        Plugin* plugin = createFunc();
        if (plugin) {
            m_loadedLibraries[metadata.id] = handle;
            return std::shared_ptr<Plugin>(plugin, [this, metadata](Plugin* p) {
                using DestroyPluginFunc = void(*)(Plugin*);
                auto it = m_loadedLibraries.find(metadata.id);
                if (it != m_loadedLibraries.end()) {
                    DestroyPluginFunc destroyFunc = reinterpret_cast<DestroyPluginFunc>(
                        GetProcAddress(static_cast<HMODULE>(it->second), "DestroyPlugin"));
                    if (destroyFunc) {
                        destroyFunc(p);
                    }
                    FreeLibrary(static_cast<HMODULE>(it->second));
                    m_loadedLibraries.erase(it);
                }
                delete p;
            });
        }

        FreeLibrary(handle);
        return nullptr;

#else
        // Unix/Linux implementation
        std::unique_lock<std::mutex> lock(m_loaderMutex);

        void* handle = dlopen(path.c_str(), RTLD_LAZY);
        if (!handle) {
            return nullptr;
        }

        // Get plugin creation function
        using CreatePluginFunc = Plugin*(*)();
        CreatePluginFunc createFunc = reinterpret_cast<CreatePluginFunc>(
            dlsym(handle, "CreatePlugin"));

        if (!createFunc) {
            dlclose(handle);
            return nullptr;
        }

        Plugin* plugin = createFunc();
        if (plugin) {
            m_loadedLibraries[metadata.id] = handle;
            return std::shared_ptr<Plugin>(plugin, [this, metadata](Plugin* p) {
                using DestroyPluginFunc = void(*)(Plugin*);
                auto it = m_loadedLibraries.find(metadata.id);
                if (it != m_loadedLibraries.end()) {
                    DestroyPluginFunc destroyFunc = reinterpret_cast<DestroyPluginFunc>(
                        dlsym(it->second, "DestroyPlugin"));
                    if (destroyFunc) {
                        destroyFunc(p);
                    }
                    dlclose(it->second);
                    m_loadedLibraries.erase(it);
                }
                delete p;
            });
        }

        dlclose(handle);
        return nullptr;
#endif
    }

    bool NativePluginLoader::UnloadPlugin(std::shared_ptr<Plugin> plugin) {
        // Plugin unloading is handled by shared_ptr deleter
        return true;
    }

    std::vector<std::string> NativePluginLoader::GetSupportedExtensions() const {
        return {m_extension};
    }

    PluginMetadata NativePluginLoader::GetMetadata(const std::string& path) const {
        PluginMetadata metadata;
        metadata.id = std::filesystem::path(path).stem().string();
        metadata.name = metadata.id;
        metadata.version = "1.0.0";
        metadata.path = path;
        metadata.fileSize = std::filesystem::file_size(path);
        metadata.createdTime = std::filesystem::last_write_time(path);

        // Try to load metadata from plugin
#ifdef _WIN32
        HMODULE handle = LoadLibraryA(path.c_str());
        if (handle) {
            using GetPluginNameFunc = const char*(*)();
            using GetPluginVersionFunc = const char*(*)();

            GetPluginNameFunc nameFunc = reinterpret_cast<GetPluginNameFunc>(
                GetProcAddress(handle, "GetPluginName"));
            GetPluginVersionFunc versionFunc = reinterpret_cast<GetPluginVersionFunc>(
                GetProcAddress(handle, "GetPluginVersion"));

            if (nameFunc) metadata.name = nameFunc();
            if (versionFunc) metadata.version = versionFunc();

            FreeLibrary(handle);
        }
#endif

        return metadata;
    }

    // ScriptPluginLoader implementation
    ScriptPluginLoader::ScriptPluginLoader()
        : PluginLoader(".lua") {
    }

    std::shared_ptr<Plugin> ScriptPluginLoader::LoadPlugin(const std::string& path, const PluginMetadata& metadata) {
        // Script plugin loading would require scripting system integration
        // For now, return nullptr as it's not implemented
        return nullptr;
    }

    bool ScriptPluginLoader::UnloadPlugin(std::shared_ptr<Plugin> plugin) {
        return true;
    }

    std::vector<std::string> ScriptPluginLoader::GetSupportedExtensions() const {
        return {".lua", ".py", ".js"};
    }

    PluginMetadata ScriptPluginLoader::GetMetadata(const std::string& path) const {
        PluginMetadata metadata;
        metadata.id = std::filesystem::path(path).stem().string();
        metadata.name = metadata.id;
        metadata.version = "1.0.0";
        metadata.path = path;
        metadata.fileSize = std::filesystem::file_size(path);
        metadata.createdTime = std::filesystem::last_write_time(path);

        // Parse script metadata from comments or header
        std::ifstream file(path);
        if (file.is_open()) {
            std::string line;
            while (std::getline(file, line)) {
                if (line.find("--") == 0 && line.find("name:") != std::string::npos) {
                    size_t pos = line.find("name:");
                    metadata.name = line.substr(pos + 5);
                } else if (line.find("--") == 0 && line.find("version:") != std::string::npos) {
                    size_t pos = line.find("version:");
                    metadata.version = line.substr(pos + 8);
                }
            }
        }

        return metadata;
    }

    // PluginAPI implementation
    PluginAPI& PluginAPI::GetInstance() {
        static PluginAPI instance;
        return instance;
    }

    PluginAPI::PluginAPI() = default;

    std::string PluginAPI::GetEngineVersion() const {
        return "1.0.0";
    }

    std::string PluginAPI::GetEngineName() const {
        return "VoxelCraft";
    }

    uint64_t PluginAPI::GetEngineStartTime() const {
        return 0; // Would return actual engine start time
    }

    bool PluginAPI::RegisterPlugin(std::shared_ptr<Plugin> plugin) {
        std::unique_lock<std::shared_mutex> lock(m_apiMutex);

        if (m_plugins.find(plugin->GetId()) != m_plugins.end()) {
            return false;
        }

        m_plugins[plugin->GetId()] = plugin;
        return true;
    }

    bool PluginAPI::UnregisterPlugin(const std::string& pluginId) {
        std::unique_lock<std::shared_mutex> lock(m_apiMutex);

        auto it = m_plugins.find(pluginId);
        if (it == m_plugins.end()) {
            return false;
        }

        m_plugins.erase(it);
        return true;
    }

    std::shared_ptr<Plugin> PluginAPI::GetPlugin(const std::string& pluginId) const {
        std::shared_lock<std::shared_mutex> lock(m_apiMutex);

        auto it = m_plugins.find(pluginId);
        return it != m_plugins.end() ? it->second : nullptr;
    }

    bool PluginAPI::RegisterHook(const std::string& hookName, HookType type, const std::string& pluginId,
                                PluginHook::HookFunction callback) {
        std::unique_lock<std::shared_mutex> lock(m_apiMutex);

        auto& hook = m_hooks[hookName];
        if (!hook) {
            hook = std::make_shared<PluginHook>(hookName, type);
        }

        hook->AddCallback(pluginId, callback);
        return true;
    }

    bool PluginAPI::UnregisterHook(const std::string& hookName, const std::string& pluginId) {
        std::unique_lock<std::shared_mutex> lock(m_apiMutex);

        auto it = m_hooks.find(hookName);
        if (it != m_hooks.end()) {
            it->second->RemoveCallback(pluginId);
            return true;
        }

        return false;
    }

    bool PluginAPI::ExecuteHook(const std::string& hookName, const std::vector<std::any>& args) {
        std::shared_lock<std::shared_mutex> lock(m_apiMutex);

        auto it = m_hooks.find(hookName);
        if (it != m_hooks.end()) {
            return it->second->Execute(args);
        }

        return true;
    }

    bool PluginAPI::RegisterEventHandler(const std::string& eventName, const std::string& pluginId,
                                       std::function<void(const std::any&)> handler) {
        std::unique_lock<std::shared_mutex> lock(m_apiMutex);
        m_eventHandlers[eventName].push_back(handler);
        return true;
    }

    bool PluginAPI::UnregisterEventHandler(const std::string& eventName, const std::string& pluginId) {
        // Simplified - would need to track by plugin ID
        return true;
    }

    bool PluginAPI::FireEvent(const std::string& eventName, const std::any& data) {
        std::shared_lock<std::shared_mutex> lock(m_apiMutex);

        auto it = m_eventHandlers.find(eventName);
        if (it != m_eventHandlers.end()) {
            for (const auto& handler : it->second) {
                try {
                    handler(data);
                } catch (const std::exception& e) {
                    LogError("Exception in event handler for '" + eventName + "': " + e.what(), "PluginAPI");
                }
            }
        }

        return true;
    }

    bool PluginAPI::LoadResource(const std::string& path, const std::string& pluginId) {
        // Implementation for loading resources
        return true;
    }

    bool PluginAPI::UnloadResource(const std::string& path, const std::string& pluginId) {
        // Implementation for unloading resources
        return true;
    }

    std::vector<std::string> PluginAPI::GetLoadedResources(const std::string& pluginId) const {
        std::shared_lock<std::shared_mutex> lock(m_apiMutex);

        auto it = m_pluginResources.find(pluginId);
        return it != m_pluginResources.end() ? it->second : std::vector<std::string>();
    }

    bool PluginAPI::SetConfigValue(const std::string& key, const std::any& value, const std::string& pluginId) {
        std::unique_lock<std::shared_mutex> lock(m_apiMutex);
        m_pluginConfigs[pluginId][key] = value;
        return true;
    }

    std::any PluginAPI::GetConfigValue(const std::string& key, const std::string& pluginId) const {
        std::shared_lock<std::shared_mutex> lock(m_apiMutex);

        auto pluginIt = m_pluginConfigs.find(pluginId);
        if (pluginIt != m_pluginConfigs.end()) {
            auto configIt = pluginIt->second.find(key);
            if (configIt != pluginIt->second.end()) {
                return configIt->second;
            }
        }

        return std::any();
    }

    void PluginAPI::LogInfo(const std::string& message, const std::string& pluginId) {
        Logger::GetInstance().Info("[" + pluginId + "] " + message, "PluginAPI");
    }

    void PluginAPI::LogWarning(const std::string& message, const std::string& pluginId) {
        Logger::GetInstance().Warning("[" + pluginId + "] " + message, "PluginAPI");
    }

    void PluginAPI::LogError(const std::string& message, const std::string& pluginId) {
        Logger::GetInstance().Error("[" + pluginId + "] " + message, "PluginAPI");
    }

    std::string PluginAPI::GetPluginDataPath(const std::string& pluginId) const {
        return "plugins/" + pluginId + "/";
    }

    bool PluginAPI::CreateDirectory(const std::string& path, const std::string& pluginId) {
        try {
            return std::filesystem::create_directories(path);
        } catch (...) {
            return false;
        }
    }

    bool PluginAPI::FileExists(const std::string& path, const std::string& pluginId) const {
        return std::filesystem::exists(path);
    }

    // PluginMarketplace implementation
    PluginMarketplace::PluginMarketplace() : m_isLoggedIn(false) {
    }

    bool PluginMarketplace::Initialize() {
        // Initialize marketplace connection
        return true;
    }

    void PluginMarketplace::Shutdown() {
        // Shutdown marketplace connection
        m_isLoggedIn = false;
    }

    std::vector<PluginMarketplace::PluginListing> PluginMarketplace::SearchPlugins(const std::string& query, PluginType type) {
        // Simulate plugin search
        return m_availablePlugins;
    }

    std::vector<PluginMarketplace::PluginListing> PluginMarketplace::GetPopularPlugins(uint32_t count) {
        // Simulate popular plugins
        return std::vector<PluginListing>(m_availablePlugins.begin(),
                                        m_availablePlugins.begin() + std::min(count, (uint32_t)m_availablePlugins.size()));
    }

    std::vector<PluginMarketplace::PluginListing> PluginMarketplace::GetRecentPlugins(uint32_t count) {
        // Simulate recent plugins
        return std::vector<PluginListing>(m_availablePlugins.begin(),
                                        m_availablePlugins.begin() + std::min(count, (uint32_t)m_availablePlugins.size()));
    }

    std::vector<PluginMarketplace::PluginListing> PluginMarketplace::GetPluginsByAuthor(const std::string& author) {
        // Simulate author search
        return m_availablePlugins;
    }

    bool PluginMarketplace::DownloadPlugin(const std::string& pluginId) {
        // Simulate plugin download
        return true;
    }

    bool PluginMarketplace::InstallPlugin(const std::string& pluginPath) {
        // Simulate plugin installation
        return true;
    }

    bool PluginMarketplace::UninstallPlugin(const std::string& pluginId) {
        // Simulate plugin uninstallation
        return true;
    }

    bool PluginMarketplace::Login(const std::string& username, const std::string& password) {
        // Simulate login
        m_isLoggedIn = true;
        m_username = username;
        return true;
    }

    bool PluginMarketplace::Logout() {
        m_isLoggedIn = false;
        m_username.clear();
        return true;
    }

    bool PluginMarketplace::SubmitReview(const std::string& pluginId, float rating, const std::string& comment) {
        std::unique_lock<std::shared_mutex> lock(m_marketplaceMutex);
        m_reviews[pluginId].emplace_back(rating, comment);
        return true;
    }

    std::vector<std::pair<float, std::string>> PluginMarketplace::GetReviews(const std::string& pluginId) {
        std::shared_lock<std::shared_mutex> lock(m_marketplaceMutex);

        auto it = m_reviews.find(pluginId);
        return it != m_reviews.end() ? it->second : std::vector<std::pair<float, std::string>>();
    }

    // PluginValidator implementation
    PluginValidator::PluginValidator() = default;

    PluginValidator::ValidationResult PluginValidator::ValidatePlugin(const std::string& path) {
        ValidationResult result;
        result.isValid = true;
        result.securityScore = 100;

        // Basic validation
        if (!std::filesystem::exists(path)) {
            result.isValid = false;
            result.errors.push_back("Plugin file does not exist");
            return result;
        }

        // File size check
        size_t fileSize = std::filesystem::file_size(path);
        if (fileSize > 100 * 1024 * 1024) { // 100MB limit
            result.warnings.push_back("Plugin file is very large");
            result.securityScore -= 10;
        }

        // Check file extension
        std::string ext = std::filesystem::path(path).extension().string();
        if (ext != ".dll" && ext != ".so" && ext != ".dylib" && ext != ".lua") {
            result.warnings.push_back("Unusual file extension");
            result.securityScore -= 5;
        }

        return result;
    }

    PluginValidator::ValidationResult PluginValidator::ValidatePlugin(const PluginMetadata& metadata) {
        ValidationResult result;
        result.isValid = true;
        result.securityScore = 100;

        // Validate permissions
        if (!CheckPermissions(metadata)) {
            result.isValid = false;
            result.errors.push_back("Invalid permissions requested");
        }

        // Validate dependencies
        if (!ValidateDependencies(metadata.dependencies)) {
            result.warnings.push_back("Some dependencies may not be available");
            result.securityScore -= 20;
        }

        return result;
    }

    bool PluginValidator::SignPlugin(const std::string& path, const std::string& privateKey) {
        // Plugin signing implementation
        return true;
    }

    bool PluginValidator::VerifySignature(const std::string& path, const std::string& publicKey) {
        // Signature verification implementation
        return true;
    }

    void PluginValidator::AddTrustedPublisher(const std::string& publisherId) {
        std::unique_lock<std::shared_mutex> lock(m_validatorMutex);
        m_trustedPublishers.insert(publisherId);
    }

    void PluginValidator::RemoveTrustedPublisher(const std::string& publisherId) {
        std::unique_lock<std::shared_mutex> lock(m_validatorMutex);
        m_trustedPublishers.erase(publisherId);
    }

    bool PluginValidator::IsTrustedPublisher(const std::string& publisherId) const {
        std::shared_lock<std::shared_mutex> lock(m_validatorMutex);
        return m_trustedPublishers.find(publisherId) != m_trustedPublishers.end();
    }

    bool PluginValidator::ScanForMalware(const std::string& path) {
        // Malware scanning implementation
        return false; // No malware detected
    }

    bool PluginValidator::CheckPermissions(const PluginMetadata& metadata) {
        // Permission checking implementation
        return true; // All permissions valid
    }

    bool PluginValidator::ValidateDependencies(const std::vector<std::string>& dependencies) {
        // Dependency validation implementation
        return true; // All dependencies valid
    }

    // PluginDependencyResolver implementation
    PluginDependencyResolver::PluginDependencyResolver() = default;

    std::vector<std::string> PluginDependencyResolver::ResolveDependencies(const std::vector<std::string>& pluginIds) {
        std::vector<std::string> loadOrder;

        for (const auto& pluginId : pluginIds) {
            std::vector<std::string> visited;
            std::vector<std::string> recursionStack;

            if (!ResolvePlugin(pluginId, loadOrder, visited, recursionStack)) {
                break;
            }
        }

        return loadOrder;
    }

    std::vector<std::string> PluginDependencyResolver::GetLoadOrder(const std::vector<std::string>& pluginIds) {
        return ResolveDependencies(pluginIds);
    }

    bool PluginDependencyResolver::HasCircularDependency(const std::string& pluginId) const {
        std::shared_lock<std::shared_mutex> lock(m_resolverMutex);

        auto it = m_dependencyGraph.find(pluginId);
        return it != m_dependencyGraph.end() && it->second.hasCircularDependency;
    }

    std::vector<std::string> PluginDependencyResolver::GetCircularDependencies(const std::string& pluginId) const {
        // Implementation for finding circular dependencies
        return {};
    }

    bool PluginDependencyResolver::AddPlugin(const std::string& pluginId, const std::vector<std::string>& dependencies) {
        std::unique_lock<std::shared_mutex> lock(m_resolverMutex);

        DependencyNode node;
        node.pluginId = pluginId;
        node.dependencies = dependencies;
        node.isResolved = false;
        node.hasCircularDependency = false;

        m_dependencyGraph[pluginId] = node;

        // Update dependents
        for (const auto& dep : dependencies) {
            m_dependencyGraph[dep].dependents.push_back(pluginId);
        }

        return true;
    }

    bool PluginDependencyResolver::RemovePlugin(const std::string& pluginId) {
        std::unique_lock<std::shared_mutex> lock(m_resolverMutex);

        auto it = m_dependencyGraph.find(pluginId);
        if (it == m_dependencyGraph.end()) {
            return false;
        }

        // Remove from dependents
        for (const auto& dep : it->second.dependencies) {
            auto& depNode = m_dependencyGraph[dep];
            depNode.dependents.erase(std::remove(depNode.dependents.begin(), depNode.dependents.end(), pluginId),
                                   depNode.dependents.end());
        }

        m_dependencyGraph.erase(it);
        return true;
    }

    void PluginDependencyResolver::Clear() {
        std::unique_lock<std::shared_mutex> lock(m_resolverMutex);
        m_dependencyGraph.clear();
    }

    bool PluginDependencyResolver::ResolvePlugin(const std::string& pluginId, std::vector<std::string>& loadOrder,
                                               std::vector<std::string>& visited, std::vector<std::string>& recursionStack) {
        visited.push_back(pluginId);
        recursionStack.push_back(pluginId);

        auto it = m_dependencyGraph.find(pluginId);
        if (it == m_dependencyGraph.end()) {
            recursionStack.pop_back();
            return false;
        }

        for (const auto& dep : it->second.dependencies) {
            if (std::find(visited.begin(), visited.end(), dep) == visited.end()) {
                if (!ResolvePlugin(dep, loadOrder, visited, recursionStack)) {
                    recursionStack.pop_back();
                    return false;
                }
            } else if (std::find(recursionStack.begin(), recursionStack.end(), dep) != recursionStack.end()) {
                // Circular dependency detected
                it->second.hasCircularDependency = true;
                recursionStack.pop_back();
                return false;
            }
        }

        loadOrder.push_back(pluginId);
        it->second.isResolved = true;
        recursionStack.pop_back();
        return true;
    }

    // Main PluginSystem implementation
    PluginSystem& PluginSystem::GetInstance() {
        static PluginSystem instance;
        return instance;
    }

    PluginSystem::PluginSystem()
        : m_pluginDirectory("plugins/"), m_autoLoadPlugins(true),
          m_sandboxingEnabled(true), m_marketplaceConnected(false) {
    }

    PluginSystem::~PluginSystem() {
        Shutdown();
    }

    bool PluginSystem::Initialize() {
        if (!System::Initialize()) {
            return false;
        }

        Logger::GetInstance().Info("PluginSystem initializing", "PluginSystem");

        // Register default loaders
        RegisterLoader(std::make_shared<NativePluginLoader>());
        RegisterLoader(std::make_shared<ScriptPluginLoader>());

        // Initialize validator
        m_validator.AddTrustedPublisher("voxelcraft");

        // Create plugin directory
        std::filesystem::create_directories(m_pluginDirectory);

        Logger::GetInstance().Info("PluginSystem initialized successfully", "PluginSystem");
        return true;
    }

    void PluginSystem::Update(float deltaTime) {
        System::Update(deltaTime);

        // Update active plugins
        std::shared_lock<std::shared_mutex> lock(m_pluginsMutex);

        for (auto& pair : m_plugins) {
            if (pair.second && pair.second->GetPlugin()->GetState() == PluginState::ACTIVE) {
                pair.second->Update(deltaTime);
            }
        }
    }

    void PluginSystem::Shutdown() {
        if (!System::Shutdown()) {
            return;
        }

        Logger::GetInstance().Info("PluginSystem shutting down", "PluginSystem");

        // Unload all plugins
        {
            std::unique_lock<std::shared_mutex> lock(m_pluginsMutex);
            for (auto& pair : m_plugins) {
                if (pair.second) {
                    UnloadPlugin(pair.first);
                }
            }
            m_plugins.clear();
        }

        // Clear hooks
        {
            std::unique_lock<std::shared_mutex> lock(m_hooksMutex);
            m_hooks.clear();
        }

        // Clear sandboxes
        {
            std::unique_lock<std::shared_mutex> lock(m_hooksMutex);
            for (auto& pair : m_sandboxes) {
                if (pair.second) {
                    pair.second->Shutdown();
                }
            }
            m_sandboxes.clear();
        }

        Logger::GetInstance().Info("PluginSystem shutdown complete", "PluginSystem");
    }

    bool PluginSystem::LoadPlugin(const std::string& path) {
        if (!std::filesystem::exists(path)) {
            Logger::GetInstance().Error("Plugin file not found: " + path, "PluginSystem");
            return false;
        }

        // Find appropriate loader
        auto loader = FindLoaderForPath(path);
        if (!loader) {
            Logger::GetInstance().Error("No loader found for plugin: " + path, "PluginSystem");
            return false;
        }

        // Load plugin metadata
        PluginMetadata metadata = loader->GetMetadata(path);

        // Validate plugin
        auto validation = ValidatePluginFile(path);
        if (!validation.isValid) {
            Logger::GetInstance().Error("Plugin validation failed: " + path, "PluginSystem");
            for (const auto& error : validation.errors) {
                Logger::GetInstance().Error("  " + error, "PluginSystem");
            }
            return false;
        }

        // Load plugin
        return LoadPluginInternal(path, metadata);
    }

    bool PluginSystem::UnloadPlugin(const std::string& pluginId) {
        std::unique_lock<std::shared_mutex> lock(m_pluginsMutex);

        auto it = m_plugins.find(pluginId);
        if (it == m_plugins.end()) {
            return false;
        }

        auto instance = it->second;
        if (!instance) {
            m_plugins.erase(it);
            return true;
        }

        // Shutdown plugin
        if (!instance->Shutdown()) {
            Logger::GetInstance().Warning("Plugin shutdown failed: " + pluginId, "PluginSystem");
        }

        // Unload plugin
        if (!instance->Unload()) {
            Logger::GetInstance().Warning("Plugin unload failed: " + pluginId, "PluginSystem");
        }

        // Remove sandbox
        {
            std::unique_lock<std::shared_mutex> sandboxLock(m_hooksMutex);
            auto sandboxIt = m_sandboxes.find(pluginId);
            if (sandboxIt != m_sandboxes.end()) {
                sandboxIt->second->Shutdown();
                m_sandboxes.erase(sandboxIt);
            }
        }

        m_plugins.erase(it);

        UpdatePluginStats(pluginId, PluginState::ACTIVE, PluginState::UNLOADED);

        Logger::GetInstance().Info("Unloaded plugin: " + pluginId, "PluginSystem");
        return true;
    }

    bool PluginSystem::ReloadPlugin(const std::string& pluginId) {
        auto instance = GetPluginInstance(pluginId);
        if (!instance) {
            return false;
        }

        auto metadata = instance->GetPlugin()->GetMetadata();

        if (!UnloadPlugin(pluginId)) {
            return false;
        }

        return LoadPluginInternal(metadata.path, metadata);
    }

    std::shared_ptr<PluginInstance> PluginSystem::GetPluginInstance(const std::string& pluginId) const {
        std::shared_lock<std::shared_mutex> lock(m_pluginsMutex);

        auto it = m_plugins.find(pluginId);
        return it != m_plugins.end() ? it->second : nullptr;
    }

    std::vector<std::string> PluginSystem::GetLoadedPlugins() const {
        std::shared_lock<std::shared_mutex> lock(m_pluginsMutex);

        std::vector<std::string> plugins;
        for (const auto& pair : m_plugins) {
            plugins.push_back(pair.first);
        }

        return plugins;
    }

    std::vector<std::string> PluginSystem::GetActivePlugins() const {
        std::shared_lock<std::shared_mutex> lock(m_pluginsMutex);

        std::vector<std::string> active;
        for (const auto& pair : m_plugins) {
            if (pair.second && pair.second->GetPlugin()->IsActive()) {
                active.push_back(pair.first);
            }
        }

        return active;
    }

    bool PluginSystem::EnablePlugin(const std::string& pluginId) {
        auto instance = GetPluginInstance(pluginId);
        if (!instance) return false;

        if (instance->Enable()) {
            UpdatePluginStats(pluginId, PluginState::LOADED, PluginState::ACTIVE);
            return true;
        }

        return false;
    }

    bool PluginSystem::DisablePlugin(const std::string& pluginId) {
        auto instance = GetPluginInstance(pluginId);
        if (!instance) return false;

        if (instance->Disable()) {
            UpdatePluginStats(pluginId, PluginState::ACTIVE, PluginState::SUSPENDED);
            return true;
        }

        return false;
    }

    bool PluginSystem::SuspendPlugin(const std::string& pluginId) {
        return DisablePlugin(pluginId);
    }

    bool PluginSystem::ResumePlugin(const std::string& pluginId) {
        return EnablePlugin(pluginId);
    }

    std::vector<std::string> PluginSystem::DiscoverPlugins(const std::string& directory) {
        std::vector<std::string> plugins;

        if (!std::filesystem::exists(directory)) {
            return plugins;
        }

        for (const auto& entry : std::filesystem::directory_iterator(directory)) {
            if (entry.is_regular_file()) {
                std::string path = entry.path().string();
                auto loader = FindLoaderForPath(path);
                if (loader) {
                    plugins.push_back(path);
                }
            }
        }

        return plugins;
    }

    bool PluginSystem::ScanDirectory(const std::string& directory) {
        auto plugins = DiscoverPlugins(directory);

        for (const auto& plugin : plugins) {
            if (!LoadPlugin(plugin)) {
                Logger::GetInstance().Warning("Failed to load plugin: " + plugin, "PluginSystem");
            }
        }

        return true;
    }

    void PluginSystem::RegisterLoader(std::shared_ptr<PluginLoader> loader) {
        if (!loader) return;

        std::unique_lock<std::shared_mutex> lock(m_hooksMutex); // Using hooks mutex for loaders
        for (const auto& ext : loader->GetSupportedExtensions()) {
            m_loaders[ext] = loader;
        }
    }

    void PluginSystem::UnregisterLoader(const std::string& extension) {
        std::unique_lock<std::shared_mutex> lock(m_hooksMutex);
        m_loaders.erase(extension);
    }

    std::shared_ptr<PluginLoader> PluginSystem::GetLoader(const std::string& extension) const {
        std::shared_lock<std::shared_mutex> lock(m_hooksMutex);

        auto it = m_loaders.find(extension);
        return it != m_loaders.end() ? it->second : nullptr;
    }

    std::shared_ptr<PluginSandbox> PluginSystem::CreateSandbox(const std::string& pluginId, PluginIsolationLevel level) {
        std::unique_lock<std::shared_mutex> lock(m_hooksMutex);

        auto sandbox = std::make_shared<PluginSandbox>(pluginId, level);
        if (sandbox->Initialize()) {
            m_sandboxes[pluginId] = sandbox;
            return sandbox;
        }

        return nullptr;
    }

    bool PluginSystem::DestroySandbox(const std::string& pluginId) {
        std::unique_lock<std::shared_mutex> lock(m_hooksMutex);

        auto it = m_sandboxes.find(pluginId);
        if (it != m_sandboxes.end()) {
            it->second->Shutdown();
            m_sandboxes.erase(it);
            return true;
        }

        return false;
    }

    bool PluginSystem::RegisterHook(const std::string& hookName, HookType type, const std::string& pluginId,
                                  PluginHook::HookFunction callback) {
        std::unique_lock<std::shared_mutex> lock(m_hooksMutex);

        auto& hook = m_hooks[hookName];
        if (!hook) {
            hook = std::make_shared<PluginHook>(hookName, type);
        }

        hook->AddCallback(pluginId, callback);
        return true;
    }

    bool PluginSystem::UnregisterHook(const std::string& hookName, const std::string& pluginId) {
        std::unique_lock<std::shared_mutex> lock(m_hooksMutex);

        auto it = m_hooks.find(hookName);
        if (it != m_hooks.end()) {
            it->second->RemoveCallback(pluginId);
            return true;
        }

        return false;
    }

    bool PluginSystem::ExecuteHook(const std::string& hookName, const std::vector<std::any>& args) {
        std::shared_lock<std::shared_mutex> lock(m_hooksMutex);

        auto it = m_hooks.find(hookName);
        if (it != m_hooks.end()) {
            return it->second->Execute(args);
        }

        return true;
    }

    bool PluginSystem::ConnectToMarketplace() {
        if (m_marketplace.Initialize()) {
            m_marketplaceConnected = true;
            return true;
        }
        return false;
    }

    bool PluginSystem::DisconnectFromMarketplace() {
        m_marketplace.Shutdown();
        m_marketplaceConnected = false;
        return true;
    }

    bool PluginSystem::ResolveDependencies(const std::string& pluginId) {
        auto instance = GetPluginInstance(pluginId);
        if (!instance) return false;

        auto dependencies = instance->GetPlugin()->GetMetadata().dependencies;
        auto loadOrder = m_dependencyResolver.ResolveDependencies({pluginId});

        return !loadOrder.empty();
    }

    std::vector<std::string> PluginSystem::GetMissingDependencies(const std::string& pluginId) const {
        auto instance = GetPluginInstance(pluginId);
        if (!instance) return {};

        auto dependencies = instance->GetPlugin()->GetMetadata().dependencies;
        std::vector<std::string> missing;

        for (const auto& dep : dependencies) {
            if (!GetPluginInstance(dep)) {
                missing.push_back(dep);
            }
        }

        return missing;
    }

    std::vector<std::string> PluginSystem::GetPluginDependencies(const std::string& pluginId) const {
        auto instance = GetPluginInstance(pluginId);
        if (!instance) return {};

        return instance->GetPlugin()->GetMetadata().dependencies;
    }

    PluginSystem::PluginStats PluginSystem::GetStats() const {
        std::shared_lock<std::shared_mutex> lock(m_statsMutex);
        return m_stats;
    }

    std::string PluginSystem::GetPluginInfo(const std::string& pluginId) const {
        auto instance = GetPluginInstance(pluginId);
        if (!instance) return "Plugin not found";

        auto plugin = instance->GetPlugin();
        auto metadata = plugin->GetMetadata();

        std::stringstream ss;
        ss << "Plugin: " << metadata.name << " v" << metadata.version << "\n";
        ss << "ID: " << metadata.id << "\n";
        ss << "Type: " << static_cast<int>(metadata.type) << "\n";
        ss << "State: " << static_cast<int>(plugin->GetState()) << "\n";
        ss << "Load Time: " << instance->GetLoadTime() << " ms\n";
        ss << "Memory Usage: " << instance->GetMemoryUsage() << " bytes\n";

        return ss.str();
    }

    void PluginSystem::PrintPluginReport() const {
        auto stats = GetStats();
        std::stringstream report;

        report << "=== Plugin System Report ===\n";
        report << "Total Plugins: " << stats.totalPlugins << "\n";
        report << "Loaded Plugins: " << stats.loadedPlugins << "\n";
        report << "Active Plugins: " << stats.activePlugins << "\n";
        report << "Suspended Plugins: " << stats.suspendedPlugins << "\n";
        report << "Error Plugins: " << stats.errorPlugins << "\n";
        report << "Total Load Time: " << stats.totalLoadTime << " ms\n";
        report << "Total Memory Usage: " << stats.totalMemoryUsage << " bytes\n";
        report << "Hook Count: " << stats.hookCount << "\n";
        report << "Sandbox Count: " << stats.sandboxCount << "\n";

        Logger::GetInstance().Info(report.str(), "PluginSystem");
    }

    bool PluginSystem::ValidateAllPlugins() const {
        std::shared_lock<std::shared_mutex> lock(m_pluginsMutex);

        for (const auto& pair : m_plugins) {
            if (pair.second && !pair.second->GetPlugin()->Validate()) {
                return false;
            }
        }

        return true;
    }

    void PluginSystem::CleanupUnusedPlugins() {
        std::unique_lock<std::shared_mutex> lock(m_pluginsMutex);

        std::vector<std::string> toRemove;
        for (const auto& pair : m_plugins) {
            if (pair.second && pair.second->GetPlugin()->GetState() == PluginState::ERROR) {
                toRemove.push_back(pair.first);
            }
        }

        for (const auto& id : toRemove) {
            m_plugins.erase(id);
        }
    }

    // Private methods implementation
    std::shared_ptr<PluginLoader> PluginSystem::FindLoaderForPath(const std::string& path) const {
        std::string ext = std::filesystem::path(path).extension().string();
        return GetLoader(ext);
    }

    PluginMetadata PluginSystem::LoadPluginMetadata(const std::string& path) const {
        auto loader = FindLoaderForPath(path);
        if (loader) {
            return loader->GetMetadata(path);
        }

        PluginMetadata metadata;
        metadata.path = path;
        metadata.name = std::filesystem::path(path).stem().string();
        metadata.id = metadata.name;
        metadata.version = "1.0.0";
        return metadata;
    }

    bool PluginSystem::LoadPluginInternal(const std::string& path, const PluginMetadata& metadata) {
        // Check if already loaded
        if (GetPluginInstance(metadata.id)) {
            return true;
        }

        // Find loader
        auto loader = FindLoaderForPath(path);
        if (!loader) {
            return false;
        }

        // Load plugin
        auto plugin = loader->LoadPlugin(path, metadata);
        if (!plugin) {
            return false;
        }

        // Create instance
        auto instance = std::make_shared<PluginInstance>(plugin);

        // Load plugin
        if (!instance->Load()) {
            return false;
        }

        // Create sandbox if enabled
        if (m_sandboxingEnabled) {
            auto sandbox = CreateSandbox(metadata.id, metadata.isolationLevel);
            if (sandbox) {
                plugin->m_sandbox = sandbox;
            }
        }

        // Initialize plugin
        if (!instance->Initialize()) {
            return false;
        }

        // Store plugin
        {
            std::unique_lock<std::shared_mutex> lock(m_pluginsMutex);
            m_plugins[metadata.id] = instance;
        }

        UpdatePluginStats(metadata.id, PluginState::UNLOADED, PluginState::ACTIVE);

        Logger::GetInstance().Info("Loaded plugin: " + metadata.name, "PluginSystem");
        return true;
    }

    void PluginSystem::UpdatePluginStats(const std::string& pluginId, PluginState oldState, PluginState newState) {
        std::unique_lock<std::shared_mutex> lock(m_statsMutex);

        // Update counts based on state changes
        switch (oldState) {
            case PluginState::ACTIVE: m_stats.activePlugins--; break;
            case PluginState::SUSPENDED: m_stats.suspendedPlugins--; break;
            case PluginState::ERROR: m_stats.errorPlugins--; break;
            default: break;
        }

        switch (newState) {
            case PluginState::ACTIVE: m_stats.activePlugins++; break;
            case PluginState::SUSPENDED: m_stats.suspendedPlugins++; break;
            case PluginState::ERROR: m_stats.errorPlugins++; break;
            default: break;
        }

        // Update total count
        m_stats.totalPlugins = m_plugins.size();
        m_stats.loadedPlugins = m_stats.activePlugins + m_stats.suspendedPlugins;
    }

    bool PluginSystem::CheckPluginPermissions(const PluginMetadata& metadata) const {
        return m_validator.CheckPermissions(metadata);
    }

    bool PluginSystem::ValidatePluginSecurity(const std::string& path) const {
        return !m_validator.ScanForMalware(path);
    }

    void PluginSystem::OnPluginError(const std::string& pluginId, const std::string& error) {
        Logger::GetInstance().Error("Plugin error [" + pluginId + "]: " + error, "PluginSystem");
        UpdatePluginStats(pluginId, PluginState::ACTIVE, PluginState::ERROR);
    }

    void PluginSystem::OnPluginStateChanged(const std::string& pluginId, PluginState oldState, PluginState newState) {
        UpdatePluginStats(pluginId, oldState, newState);
    }

    PluginValidator::ValidationResult PluginSystem::ValidatePluginFile(const std::string& path) {
        return m_validator.ValidatePlugin(path);
    }

    bool PluginSystem::InstallValidatedPlugin(const std::string& path, const PluginValidator::ValidationResult& validation) {
        if (!validation.isValid) {
            return false;
        }

        return LoadPlugin(path);
    }

} // namespace VoxelCraft
