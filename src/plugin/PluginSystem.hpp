/**
 * @file PluginSystem.hpp
 * @brief VoxelCraft Advanced Plugin/Modding System Header
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_PLUGIN_PLUGIN_SYSTEM_HPP
#define VOXELCRAFT_PLUGIN_PLUGIN_SYSTEM_HPP

#include <memory>
#include <atomic>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <functional>
#include <queue>
#include <chrono>
#include <thread>
#include <condition_variable>
#include <variant>
#include <any>
#include <filesystem>

#include "../core/System.hpp"
#include "../memory/MemorySystem.hpp"
#include "../logging/Logger.hpp"

namespace VoxelCraft {

    // Forward declarations
    class Plugin;
    class PluginInstance;
    class PluginManager;
    class PluginLoader;
    class PluginSandbox;
    class PluginHook;
    class PluginEvent;
    class PluginAPI;
    class PluginMarketplace;
    class PluginValidator;
    class PluginDependencyResolver;

    /**
     * @brief Plugin states
     */
    enum class PluginState {
        UNLOADED,       // Not loaded
        LOADING,        // Currently loading
        LOADED,         // Loaded but not initialized
        INITIALIZING,   // Currently initializing
        ACTIVE,         // Active and running
        SUSPENDED,      // Temporarily suspended
        ERROR,          // Error state
        UNLOADING       // Being unloaded
    };

    /**
     * @brief Plugin load modes
     */
    enum class PluginLoadMode {
        EAGER,          // Load immediately
        LAZY,           // Load on demand
        BACKGROUND,     // Load in background
        MANUAL          // Manual loading only
    };

    /**
     * @brief Plugin isolation levels
     */
    enum class PluginIsolationLevel {
        NONE,           // No isolation
        LIGHT,          // Memory isolation only
        FULL,           // Full sandboxing
        SECURE          // Maximum security
    };

    /**
     * @brief Plugin types
     */
    enum class PluginType {
        MOD,            // Game modification
        UI_MOD,         // UI modification
        CONTENT,        // New content (items, blocks, etc.)
        LIBRARY,        // Shared library
        TOOL,           // Development tool
        THEME,          // Visual theme
        LANGUAGE,       // Language pack
        SERVER,         // Server-side plugin
        CLIENT,         // Client-side plugin
        HYBRID,         // Both client and server
        SYSTEM          // System-level plugin
    };

    /**
     * @brief Plugin permissions
     */
    enum class PluginPermission {
        FILE_READ,
        FILE_WRITE,
        NETWORK_ACCESS,
        SYSTEM_INFO,
        MEMORY_MANAGEMENT,
        ENTITY_MANAGEMENT,
        WORLD_MODIFICATION,
        UI_MODIFICATION,
        AUDIO_CONTROL,
        INPUT_INTERCEPTION,
        SCRIPT_EXECUTION,
        PLUGIN_MANAGEMENT,
        FULL_ACCESS
    };

    /**
     * @brief Hook types for plugin integration
     */
    enum class HookType {
        PRE_HOOK,       // Before original function
        POST_HOOK,      // After original function
        REPLACE_HOOK,   // Replace original function
        WRAP_HOOK       // Wrap original function
    };

    /**
     * @brief Plugin metadata
     */
    struct PluginMetadata {
        std::string id;
        std::string name;
        std::string version;
        std::string description;
        std::string author;
        std::string website;
        PluginType type;
        PluginLoadMode loadMode;
        PluginIsolationLevel isolationLevel;
        std::vector<std::string> dependencies;
        std::vector<std::string> conflicts;
        std::vector<PluginPermission> permissions;
        std::unordered_map<std::string, std::string> properties;
        uint64_t fileSize;
        std::chrono::system_clock::time_point createdTime;
        std::chrono::system_clock::time_point modifiedTime;
        bool isSigned;
        std::string signature;
        std::string hash;
    };

    /**
     * @brief Plugin interface
     */
    class Plugin {
    public:
        Plugin(const PluginMetadata& metadata);
        virtual ~Plugin() = default;

        const std::string& GetId() const { return m_metadata.id; }
        const std::string& GetName() const { return m_metadata.name; }
        const std::string& GetVersion() const { return m_metadata.version; }
        PluginType GetType() const { return m_metadata.type; }
        PluginState GetState() const { return m_state; }
        const PluginMetadata& GetMetadata() const { return m_metadata; }

        bool IsActive() const { return m_state == PluginState::ACTIVE; }
        bool HasError() const { return m_state == PluginState::ERROR; }
        const std::string& GetErrorMessage() const { return m_errorMessage; }

        virtual bool Initialize() = 0;
        virtual bool Shutdown() = 0;
        virtual bool Update(float deltaTime) = 0;

        virtual bool OnEnable();
        virtual bool OnDisable();

        virtual bool OnLoad();
        virtual bool OnUnload();

        // Plugin-specific methods
        virtual std::string GetPluginInfo() const = 0;
        virtual std::vector<std::string> GetCommands() const { return {}; }
        virtual bool ExecuteCommand(const std::string& command, const std::vector<std::string>& args) { return false; }

    protected:
        PluginMetadata m_metadata;
        PluginState m_state;
        std::string m_errorMessage;
        std::shared_ptr<PluginSandbox> m_sandbox;
        mutable std::shared_mutex m_pluginMutex;
    };

    /**
     * @brief Plugin instance wrapper
     */
    class PluginInstance {
    public:
        PluginInstance(std::shared_ptr<Plugin> plugin);
        ~PluginInstance();

        const std::string& GetId() const { return m_plugin->GetId(); }
        PluginState GetState() const { return m_plugin->GetState(); }
        std::shared_ptr<Plugin> GetPlugin() const { return m_plugin; }

        bool Initialize();
        bool Shutdown();
        bool Update(float deltaTime);

        bool Load();
        bool Unload();

        bool Enable();
        bool Disable();

        bool HasPermission(PluginPermission permission) const;
        void GrantPermission(PluginPermission permission);
        void RevokePermission(PluginPermission permission);

        // Statistics
        uint64_t GetLoadTime() const { return m_loadTime; }
        uint64_t GetInitializationTime() const { return m_initializationTime; }
        size_t GetMemoryUsage() const { return m_memoryUsage; }
        uint32_t GetUpdateCount() const { return m_updateCount; }

    private:
        std::shared_ptr<Plugin> m_plugin;
        std::unordered_set<PluginPermission> m_permissions;
        uint64_t m_loadTime;
        uint64_t m_initializationTime;
        size_t m_memoryUsage;
        uint32_t m_updateCount;
        std::chrono::steady_clock::time_point m_lastUpdateTime;

        mutable std::mutex m_instanceMutex;
    };

    /**
     * @brief Plugin hook for intercepting engine functions
     */
    class PluginHook {
    public:
        using HookFunction = std::function<bool(const std::vector<std::any>&)>;

        PluginHook(const std::string& name, HookType type);
        ~PluginHook() = default;

        const std::string& GetName() const { return m_name; }
        HookType GetType() const { return m_type; }

        void AddCallback(const std::string& pluginId, HookFunction callback);
        void RemoveCallback(const std::string& pluginId);
        bool HasCallback(const std::string& pluginId) const;

        bool Execute(const std::vector<std::any>& args = {}) const;

        size_t GetCallbackCount() const { return m_callbacks.size(); }
        std::vector<std::string> GetCallbackPlugins() const;

    private:
        std::string m_name;
        HookType m_type;
        std::unordered_map<std::string, HookFunction> m_callbacks;
        mutable std::shared_mutex m_hookMutex;
    };

    /**
     * @brief Plugin sandbox for isolation
     */
    class PluginSandbox {
    public:
        PluginSandbox(const std::string& pluginId, PluginIsolationLevel level);
        ~PluginSandbox() = default;

        const std::string& GetPluginId() const { return m_pluginId; }
        PluginIsolationLevel GetIsolationLevel() const { return m_isolationLevel; }

        bool Initialize();
        void Shutdown();

        // Memory management
        void* AllocateMemory(size_t size);
        void DeallocateMemory(void* ptr);
        size_t GetMemoryUsage() const { return m_memoryUsage; }

        // File system access
        bool CanAccessFile(const std::string& path) const;
        std::vector<std::string> GetAllowedPaths() const { return m_allowedPaths; }
        void AddAllowedPath(const std::string& path);

        // Network access
        bool CanAccessNetwork(const std::string& host) const;
        std::vector<std::string> GetAllowedHosts() const { return m_allowedHosts; }
        void AddAllowedHost(const std::string& host);

        // System access
        bool CanExecuteSystemCommand(const std::string& command) const;
        std::vector<std::string> GetAllowedCommands() const { return m_allowedCommands; }
        void AddAllowedCommand(const std::string& command);

        // Validation
        bool ValidatePointer(void* ptr) const;
        bool ValidateString(const std::string& str) const;

    private:
        std::string m_pluginId;
        PluginIsolationLevel m_isolationLevel;
        size_t m_memoryUsage;
        std::vector<std::string> m_allowedPaths;
        std::vector<std::string> m_allowedHosts;
        std::vector<std::string> m_allowedCommands;
        std::unordered_set<void*> m_allocatedMemory;
        mutable std::shared_mutex m_sandboxMutex;
    };

    /**
     * @brief Plugin loader interface
     */
    class PluginLoader {
    public:
        PluginLoader(const std::string& extension);
        virtual ~PluginLoader() = default;

        const std::string& GetExtension() const { return m_extension; }

        virtual bool CanLoad(const std::string& path) const;
        virtual std::shared_ptr<Plugin> LoadPlugin(const std::string& path, const PluginMetadata& metadata) = 0;
        virtual bool UnloadPlugin(std::shared_ptr<Plugin> plugin) = 0;

        virtual std::vector<std::string> GetSupportedExtensions() const = 0;
        virtual PluginMetadata GetMetadata(const std::string& path) const = 0;

    protected:
        std::string m_extension;
    };

    /**
     * @brief Native plugin loader (DLL/SO)
     */
    class NativePluginLoader : public PluginLoader {
    public:
        NativePluginLoader();
        ~NativePluginLoader() override = default;

        std::shared_ptr<Plugin> LoadPlugin(const std::string& path, const PluginMetadata& metadata) override;
        bool UnloadPlugin(std::shared_ptr<Plugin> plugin) override;

        std::vector<std::string> GetSupportedExtensions() const override;
        PluginMetadata GetMetadata(const std::string& path) const override;

    private:
        std::unordered_map<std::string, void*> m_loadedLibraries;
        mutable std::mutex m_loaderMutex;
    };

    /**
     * @brief Script plugin loader
     */
    class ScriptPluginLoader : public PluginLoader {
    public:
        ScriptPluginLoader();
        ~ScriptPluginLoader() override = default;

        std::shared_ptr<Plugin> LoadPlugin(const std::string& path, const PluginMetadata& metadata) override;
        bool UnloadPlugin(std::shared_ptr<Plugin> plugin) override;

        std::vector<std::string> GetSupportedExtensions() const override;
        PluginMetadata GetMetadata(const std::string& path) const override;
    };

    /**
     * @brief Plugin API for plugins to interact with the engine
     */
    class PluginAPI {
    public:
        static PluginAPI& GetInstance();

        PluginAPI();
        ~PluginAPI() = default;

        // Engine information
        std::string GetEngineVersion() const;
        std::string GetEngineName() const;
        uint64_t GetEngineStartTime() const;

        // Plugin management
        bool RegisterPlugin(std::shared_ptr<Plugin> plugin);
        bool UnregisterPlugin(const std::string& pluginId);
        std::shared_ptr<Plugin> GetPlugin(const std::string& pluginId) const;

        // Hook system
        bool RegisterHook(const std::string& hookName, HookType type, const std::string& pluginId,
                         PluginHook::HookFunction callback);
        bool UnregisterHook(const std::string& hookName, const std::string& pluginId);
        bool ExecuteHook(const std::string& hookName, const std::vector<std::any>& args = {});

        // Event system
        bool RegisterEventHandler(const std::string& eventName, const std::string& pluginId,
                                 std::function<void(const std::any&)> handler);
        bool UnregisterEventHandler(const std::string& eventName, const std::string& pluginId);
        bool FireEvent(const std::string& eventName, const std::any& data);

        // Resource management
        bool LoadResource(const std::string& path, const std::string& pluginId);
        bool UnloadResource(const std::string& path, const std::string& pluginId);
        std::vector<std::string> GetLoadedResources(const std::string& pluginId) const;

        // Configuration
        bool SetConfigValue(const std::string& key, const std::any& value, const std::string& pluginId);
        std::any GetConfigValue(const std::string& key, const std::string& pluginId) const;

        // Logging
        void LogInfo(const std::string& message, const std::string& pluginId);
        void LogWarning(const std::string& message, const std::string& pluginId);
        void LogError(const std::string& message, const std::string& pluginId);

        // Utility functions
        std::string GetPluginDataPath(const std::string& pluginId) const;
        bool CreateDirectory(const std::string& path, const std::string& pluginId);
        bool FileExists(const std::string& path, const std::string& pluginId) const;

    private:
        std::unordered_map<std::string, std::shared_ptr<Plugin>> m_plugins;
        std::unordered_map<std::string, std::shared_ptr<PluginHook>> m_hooks;
        std::unordered_map<std::string, std::vector<std::function<void(const std::any&)>>> m_eventHandlers;
        std::unordered_map<std::string, std::unordered_map<std::string, std::any>> m_pluginConfigs;
        std::unordered_map<std::string, std::vector<std::string>> m_pluginResources;

        mutable std::shared_mutex m_apiMutex;
    };

    /**
     * @brief Plugin marketplace (simulated)
     */
    class PluginMarketplace {
    public:
        struct PluginListing {
            std::string id;
            std::string name;
            std::string description;
            std::string version;
            std::string author;
            float price;
            uint32_t downloads;
            float rating;
            std::string downloadUrl;
            std::chrono::system_clock::time_point releaseDate;
            std::vector<std::string> screenshots;
            std::vector<std::string> tags;
            bool isPaid;
            bool isVerified;
        };

        PluginMarketplace();
        ~PluginMarketplace() = default;

        bool Initialize();
        void Shutdown();

        // Plugin discovery
        std::vector<PluginListing> SearchPlugins(const std::string& query, PluginType type = PluginType::MOD);
        std::vector<PluginListing> GetPopularPlugins(uint32_t count = 10);
        std::vector<PluginListing> GetRecentPlugins(uint32_t count = 10);
        std::vector<PluginListing> GetPluginsByAuthor(const std::string& author);

        // Plugin management
        bool DownloadPlugin(const std::string& pluginId);
        bool InstallPlugin(const std::string& pluginPath);
        bool UninstallPlugin(const std::string& pluginId);

        // User management
        bool Login(const std::string& username, const std::string& password);
        bool Logout();
        bool IsLoggedIn() const { return m_isLoggedIn; }
        const std::string& GetUsername() const { return m_username; }

        // Reviews and ratings
        bool SubmitReview(const std::string& pluginId, float rating, const std::string& comment);
        std::vector<std::pair<float, std::string>> GetReviews(const std::string& pluginId);

    private:
        bool m_isLoggedIn;
        std::string m_username;
        std::vector<PluginListing> m_availablePlugins;
        std::unordered_map<std::string, std::vector<std::pair<float, std::string>>> m_reviews;

        mutable std::shared_mutex m_marketplaceMutex;
    };

    /**
     * @brief Plugin validator for security
     */
    class PluginValidator {
    public:
        PluginValidator();
        ~PluginValidator() = default;

        struct ValidationResult {
            bool isValid;
            bool isSigned;
            bool isTrusted;
            std::string signatureStatus;
            std::vector<std::string> warnings;
            std::vector<std::string> errors;
            uint32_t securityScore;
        };

        ValidationResult ValidatePlugin(const std::string& path);
        ValidationResult ValidatePlugin(const PluginMetadata& metadata);

        bool SignPlugin(const std::string& path, const std::string& privateKey);
        bool VerifySignature(const std::string& path, const std::string& publicKey);

        void AddTrustedPublisher(const std::string& publisherId);
        void RemoveTrustedPublisher(const std::string& publisherId);
        bool IsTrustedPublisher(const std::string& publisherId) const;

        // Security scanning
        bool ScanForMalware(const std::string& path);
        bool CheckPermissions(const PluginMetadata& metadata);
        bool ValidateDependencies(const std::vector<std::string>& dependencies);

    private:
        std::unordered_set<std::string> m_trustedPublishers;
        mutable std::shared_mutex m_validatorMutex;
    };

    /**
     * @brief Plugin dependency resolver
     */
    class PluginDependencyResolver {
    public:
        PluginDependencyResolver();
        ~PluginDependencyResolver() = default;

        struct DependencyNode {
            std::string pluginId;
            std::vector<std::string> dependencies;
            std::vector<std::string> dependents;
            bool isResolved;
            bool hasCircularDependency;
        };

        std::vector<std::string> ResolveDependencies(const std::vector<std::string>& pluginIds);
        std::vector<std::string> GetLoadOrder(const std::vector<std::string>& pluginIds);

        bool HasCircularDependency(const std::string& pluginId) const;
        std::vector<std::string> GetCircularDependencies(const std::string& pluginId) const;

        bool AddPlugin(const std::string& pluginId, const std::vector<std::string>& dependencies);
        bool RemovePlugin(const std::string& pluginId);

        void Clear();

    private:
        std::unordered_map<std::string, DependencyNode> m_dependencyGraph;

        bool DetectCircularDependency(const std::string& pluginId, std::vector<std::string>& visited,
                                    std::vector<std::string>& recursionStack);
        void BuildDependencyGraph();

        mutable std::shared_mutex m_resolverMutex;
    };

    /**
     * @brief Main plugin system
     */
    class PluginSystem : public System {
    public:
        static PluginSystem& GetInstance();

        PluginSystem();
        ~PluginSystem() override;

        // System interface
        bool Initialize() override;
        void Update(float deltaTime) override;
        void Shutdown() override;
        std::string GetName() const override { return "PluginSystem"; }
        System::Type GetType() const override { return System::Type::PLUGIN; }

        // Plugin loading and management
        bool LoadPlugin(const std::string& path);
        bool UnloadPlugin(const std::string& pluginId);
        bool ReloadPlugin(const std::string& pluginId);

        std::shared_ptr<PluginInstance> GetPluginInstance(const std::string& pluginId) const;
        std::vector<std::string> GetLoadedPlugins() const;
        std::vector<std::string> GetActivePlugins() const;

        // Plugin control
        bool EnablePlugin(const std::string& pluginId);
        bool DisablePlugin(const std::string& pluginId);
        bool SuspendPlugin(const std::string& pluginId);
        bool ResumePlugin(const std::string& pluginId);

        // Plugin discovery
        std::vector<std::string> DiscoverPlugins(const std::string& directory);
        bool ScanDirectory(const std::string& directory);

        // Plugin loader management
        void RegisterLoader(std::shared_ptr<PluginLoader> loader);
        void UnregisterLoader(const std::string& extension);
        std::shared_ptr<PluginLoader> GetLoader(const std::string& extension) const;

        // Plugin sandboxing
        std::shared_ptr<PluginSandbox> CreateSandbox(const std::string& pluginId, PluginIsolationLevel level);
        bool DestroySandbox(const std::string& pluginId);

        // Hook system
        bool RegisterHook(const std::string& hookName, HookType type, const std::string& pluginId,
                         PluginHook::HookFunction callback);
        bool UnregisterHook(const std::string& hookName, const std::string& pluginId);
        bool ExecuteHook(const std::string& hookName, const std::vector<std::any>& args = {});

        // Plugin marketplace
        bool ConnectToMarketplace();
        bool DisconnectFromMarketplace();
        bool IsConnectedToMarketplace() const { return m_marketplaceConnected; }

        // Plugin validation
        PluginValidator::ValidationResult ValidatePluginFile(const std::string& path);
        bool InstallValidatedPlugin(const std::string& path, const PluginValidator::ValidationResult& validation);

        // Plugin dependencies
        bool ResolveDependencies(const std::string& pluginId);
        std::vector<std::string> GetMissingDependencies(const std::string& pluginId) const;
        std::vector<std::string> GetPluginDependencies(const std::string& pluginId) const;

        // Configuration
        void SetPluginDirectory(const std::string& directory) { m_pluginDirectory = directory; }
        const std::string& GetPluginDirectory() const { return m_pluginDirectory; }

        void SetAutoLoadPlugins(bool enable) { m_autoLoadPlugins = enable; }
        bool IsAutoLoadEnabled() const { return m_autoLoadPlugins; }

        void SetSandboxingEnabled(bool enable) { m_sandboxingEnabled = enable; }
        bool IsSandboxingEnabled() const { return m_sandboxingEnabled; }

        // Statistics
        struct PluginStats {
            uint32_t totalPlugins;
            uint32_t loadedPlugins;
            uint32_t activePlugins;
            uint32_t suspendedPlugins;
            uint32_t errorPlugins;
            uint64_t totalLoadTime;
            uint64_t totalMemoryUsage;
            uint32_t hookCount;
            uint32_t sandboxCount;
        };

        PluginStats GetStats() const;

        // Utility functions
        std::string GetPluginInfo(const std::string& pluginId) const;
        void PrintPluginReport() const;

        bool ExportPluginList(const std::string& filename) const;
        bool ImportPluginList(const std::string& filename);

        void CleanupUnusedPlugins();
        bool ValidateAllPlugins() const;

    private:
        // Core components
        mutable std::shared_mutex m_pluginsMutex;
        std::unordered_map<std::string, std::shared_ptr<PluginInstance>> m_plugins;
        std::unordered_map<std::string, std::shared_ptr<PluginLoader>> m_loaders;
        std::unordered_map<std::string, std::shared_ptr<PluginSandbox>> m_sandboxes;
        std::unordered_map<std::string, std::shared_ptr<PluginHook>> m_hooks;

        // Plugin systems
        PluginAPI m_api;
        PluginMarketplace m_marketplace;
        PluginValidator m_validator;
        PluginDependencyResolver m_dependencyResolver;

        // Configuration
        std::string m_pluginDirectory;
        bool m_autoLoadPlugins;
        bool m_sandboxingEnabled;
        bool m_marketplaceConnected;

        // Statistics
        mutable std::shared_mutex m_statsMutex;
        PluginStats m_stats;

        // Internal methods
        std::shared_ptr<PluginLoader> FindLoaderForPath(const std::string& path) const;
        PluginMetadata LoadPluginMetadata(const std::string& path) const;

        bool LoadPluginInternal(const std::string& path, const PluginMetadata& metadata);
        bool InitializePlugin(std::shared_ptr<PluginInstance> instance);
        bool ShutdownPlugin(std::shared_ptr<PluginInstance> instance);

        void UpdatePluginStats(const std::string& pluginId, PluginState oldState, PluginState newState);

        bool CheckPluginPermissions(const PluginMetadata& metadata) const;
        bool ValidatePluginSecurity(const std::string& path) const;

        void OnPluginError(const std::string& pluginId, const std::string& error);
        void OnPluginStateChanged(const std::string& pluginId, PluginState oldState, PluginState newState);
    };

    // Plugin system utility macros
    #define VOXELCRAFT_LOAD_PLUGIN(path) \
        PluginSystem::GetInstance().LoadPlugin(path)

    #define VOXELCRAFT_UNLOAD_PLUGIN(id) \
        PluginSystem::GetInstance().UnloadPlugin(id)

    #define VOXELCRAFT_GET_PLUGIN(id) \
        PluginSystem::GetInstance().GetPluginInstance(id)

    #define VOXELCRAFT_REGISTER_HOOK(hook, type, callback) \
        PluginSystem::GetInstance().RegisterHook(hook, type, PLUGIN_ID, callback)

    // Plugin development macros
    #define VOXELCRAFT_DECLARE_PLUGIN(className, pluginName, version) \
        extern "C" { \
            VOXELCRAFT_PLUGIN_EXPORT Plugin* CreatePlugin() { \
                return new className(); \
            } \
            VOXELCRAFT_PLUGIN_EXPORT void DestroyPlugin(Plugin* plugin) { \
                delete plugin; \
            } \
            VOXELCRAFT_PLUGIN_EXPORT const char* GetPluginName() { \
                return pluginName; \
            } \
            VOXELCRAFT_PLUGIN_EXPORT const char* GetPluginVersion() { \
                return version; \
            } \
        }

    #define VOXELCRAFT_PLUGIN_EXPORT __declspec(dllexport)

} // namespace VoxelCraft

#endif // VOXELCRAFT_PLUGIN_PLUGIN_SYSTEM_HPP
