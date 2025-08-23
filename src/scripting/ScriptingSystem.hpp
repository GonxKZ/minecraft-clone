/**
 * @file ScriptingSystem.hpp
 * @brief VoxelCraft Advanced Scripting System Header
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_SCRIPTING_SCRIPTING_SYSTEM_HPP
#define VOXELCRAFT_SCRIPTING_SCRIPTING_SYSTEM_HPP

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

#include <glm/glm.hpp>

#include "../core/System.hpp"
#include "../memory/MemorySystem.hpp"

namespace VoxelCraft {

    // Forward declarations
    class Script;
    class ScriptEngine;
    class ScriptContext;
    class ScriptDebugger;
    class ScriptProfiler;
    class ScriptCompiler;
    class ScriptBindings;
    class ScriptCoroutine;
    class ScriptModule;
    class ScriptEnvironment;

    /**
     * @brief Scripting languages supported
     */
    enum class ScriptLanguage {
        LUA,
        PYTHON,
        JAVASCRIPT,
        CSHARP,
        CUSTOM
    };

    /**
     * @brief Script execution modes
     */
    enum class ScriptExecutionMode {
        SYNCHRONOUS,    // Execute immediately
        ASYNCHRONOUS,   // Execute in background
        COROUTINE,      // Execute as coroutine
        SCHEDULED       // Execute at scheduled time
    };

    /**
     * @brief Script states
     */
    enum class ScriptState {
        UNLOADED,       // Not loaded
        LOADING,        // Loading script
        LOADED,         // Script loaded
        COMPILING,      // Compiling script
        COMPILED,       // Script compiled
        EXECUTING,      // Currently executing
        PAUSED,         // Execution paused
        ERROR,          // Execution error
        FINISHED        // Execution finished
    };

    /**
     * @brief Script execution results
     */
    enum class ScriptResult {
        SUCCESS,
        ERROR,
        YIELD,          // Coroutine yielded
        TIMEOUT,        // Execution timed out
        INTERRUPTED     // Execution was interrupted
    };

    /**
     * @brief Script variable types
     */
    enum class ScriptValueType {
        NIL,
        BOOLEAN,
        INTEGER,
        FLOAT,
        STRING,
        TABLE,
        FUNCTION,
        USERDATA,
        THREAD,
        VECTOR2,
        VECTOR3,
        VECTOR4,
        MATRIX3,
        MATRIX4,
        ENTITY,
        COMPONENT
    };

    /**
     * @brief Script value container
     */
    class ScriptValue {
    public:
        ScriptValue();
        ScriptValue(bool value);
        ScriptValue(int64_t value);
        ScriptValue(double value);
        ScriptValue(const std::string& value);
        ScriptValue(const glm::vec2& value);
        ScriptValue(const glm::vec3& value);
        ScriptValue(const glm::vec4& value);
        ScriptValue(const glm::mat3& value);
        ScriptValue(const glm::mat4& value);

        ScriptValueType GetType() const { return m_type; }
        bool IsValid() const { return m_type != ScriptValueType::NIL; }

        template<typename T>
        T As() const;

        template<typename T>
        bool Is() const;

        std::string ToString() const;

    private:
        ScriptValueType m_type;
        std::variant<
            bool,
            int64_t,
            double,
            std::string,
            glm::vec2,
            glm::vec3,
            glm::vec4,
            glm::mat3,
            glm::mat4,
            std::any
        > m_value;
    };

    /**
     * @brief Script execution context
     */
    class ScriptContext {
    public:
        ScriptContext(const std::string& name, ScriptLanguage language);
        ~ScriptContext() = default;

        const std::string& GetName() const { return m_name; }
        ScriptLanguage GetLanguage() const { return m_language; }
        ScriptState GetState() const { return m_state; }

        bool IsExecuting() const { return m_state == ScriptState::EXECUTING; }
        bool HasError() const { return m_state == ScriptState::ERROR; }

        void SetGlobal(const std::string& name, const ScriptValue& value);
        ScriptValue GetGlobal(const std::string& name) const;

        ScriptResult ExecuteString(const std::string& code);
        ScriptResult ExecuteFile(const std::string& filename);
        ScriptResult CallFunction(const std::string& name, const std::vector<ScriptValue>& args = {});

        void SetTimeout(uint64_t milliseconds) { m_timeout = milliseconds; }
        uint64_t GetTimeout() const { return m_timeout; }

        void SetMemoryLimit(size_t bytes) { m_memoryLimit = bytes; }
        size_t GetMemoryLimit() const { return m_memoryLimit; }

        const std::string& GetErrorMessage() const { return m_errorMessage; }
        const std::string& GetStackTrace() const { return m_stackTrace; }

        void Pause();
        void Resume();
        void Stop();

    private:
        std::string m_name;
        ScriptLanguage m_language;
        ScriptState m_state;
        uint64_t m_timeout;
        size_t m_memoryLimit;
        std::string m_errorMessage;
        std::string m_stackTrace;

        // Language-specific data
        std::any m_languageData;

        mutable std::shared_mutex m_contextMutex;
    };

    /**
     * @brief Script coroutine for async execution
     */
    class ScriptCoroutine {
    public:
        ScriptCoroutine(const std::string& name, std::shared_ptr<ScriptContext> context);
        ~ScriptCoroutine() = default;

        const std::string& GetName() const { return m_name; }
        bool IsRunning() const { return m_running; }
        bool IsPaused() const { return m_paused; }

        ScriptResult Resume();
        ScriptResult Yield();
        void Stop();

        void SetData(const std::any& data) { m_userData = data; }
        std::any GetData() const { return m_userData; }

        uint64_t GetExecutionTime() const { return m_executionTime; }
        uint32_t GetYieldCount() const { return m_yieldCount; }

    private:
        std::string m_name;
        std::shared_ptr<ScriptContext> m_context;
        std::atomic<bool> m_running;
        std::atomic<bool> m_paused;
        std::any m_userData;
        uint64_t m_executionTime;
        uint32_t m_yieldCount;

        mutable std::mutex m_coroutineMutex;
    };

    /**
     * @brief Script module for organization
     */
    class ScriptModule {
    public:
        ScriptModule(const std::string& name, const std::string& path);
        ~ScriptModule() = default;

        const std::string& GetName() const { return m_name; }
        const std::string& GetPath() const { return m_path; }

        bool Load();
        bool Unload();
        bool Reload();

        bool IsLoaded() const { return m_loaded; }
        size_t GetScriptCount() const { return m_scripts.size(); }

        std::shared_ptr<ScriptContext> GetScript(const std::string& name) const;
        std::vector<std::string> GetScriptNames() const;

        void SetAutoReload(bool enable) { m_autoReload = enable; }
        bool IsAutoReloadEnabled() const { return m_autoReload; }

    private:
        std::string m_name;
        std::string m_path;
        bool m_loaded;
        bool m_autoReload;
        std::unordered_map<std::string, std::shared_ptr<ScriptContext>> m_scripts;

        mutable std::shared_mutex m_moduleMutex;
    };

    /**
     * @brief Script debugger
     */
    class ScriptDebugger {
    public:
        struct Breakpoint {
            std::string filename;
            uint32_t line;
            std::string condition;
            bool enabled;
        };

        struct CallStackFrame {
            std::string functionName;
            std::string filename;
            uint32_t line;
            std::unordered_map<std::string, ScriptValue> locals;
        };

        ScriptDebugger();
        ~ScriptDebugger() = default;

        void AttachToContext(std::shared_ptr<ScriptContext> context);
        void DetachFromContext(const std::string& contextName);

        void AddBreakpoint(const Breakpoint& breakpoint);
        void RemoveBreakpoint(const std::string& filename, uint32_t line);
        void ClearBreakpoints();

        void Continue();
        void StepOver();
        void StepInto();
        void StepOut();

        bool IsPaused() const { return m_paused; }
        bool IsAttached() const { return !m_attachedContexts.empty(); }

        std::vector<CallStackFrame> GetCallStack() const;
        std::vector<Breakpoint> GetBreakpoints() const;

        void SetVariable(const std::string& name, const ScriptValue& value);
        ScriptValue GetVariable(const std::string& name) const;

    private:
        std::atomic<bool> m_paused;
        std::unordered_map<std::string, std::shared_ptr<ScriptContext>> m_attachedContexts;
        std::vector<Breakpoint> m_breakpoints;
        std::vector<CallStackFrame> m_callStack;

        mutable std::shared_mutex m_debuggerMutex;
    };

    /**
     * @brief Script profiler
     */
    class ScriptProfiler {
    public:
        struct ProfileData {
            std::string functionName;
            std::string filename;
            uint64_t callCount;
            uint64_t totalTime;
            uint64_t selfTime;
            uint64_t minTime;
            uint64_t maxTime;
            uint64_t avgTime;
        };

        ScriptProfiler();
        ~ScriptProfiler() = default;

        void StartProfiling();
        void StopProfiling();
        void Reset();

        bool IsProfiling() const { return m_profiling; }

        void BeginFunction(const std::string& name, const std::string& filename);
        void EndFunction(const std::string& name);

        std::vector<ProfileData> GetProfileData() const;
        ProfileData GetFunctionData(const std::string& name) const;

        void GenerateReport(const std::string& filename) const;

    private:
        std::atomic<bool> m_profiling;
        std::unordered_map<std::string, ProfileData> m_profileData;
        std::vector<std::pair<std::string, std::chrono::steady_clock::time_point>> m_callStack;

        mutable std::shared_mutex m_profilerMutex;
    };

    /**
     * @brief Script bindings generator
     */
    class ScriptBindings {
    public:
        struct BindingInfo {
            std::string name;
            std::string description;
            std::vector<std::string> parameters;
            std::string returnType;
            std::function<ScriptValue(const std::vector<ScriptValue>&)> function;
        };

        ScriptBindings(ScriptLanguage language);
        ~ScriptBindings() = default;

        void RegisterFunction(const std::string& name, const std::function<ScriptValue(const std::vector<ScriptValue>&)>& function,
                            const std::vector<std::string>& parameters = {}, const std::string& description = "");

        void RegisterClass(const std::string& name, const std::vector<BindingInfo>& methods);

        bool GenerateBindings(const std::string& outputPath);
        bool LoadBindings(const std::string& path);

        std::vector<std::string> GetRegisteredFunctions() const;
        std::vector<std::string> GetRegisteredClasses() const;

        BindingInfo GetFunctionInfo(const std::string& name) const;

    private:
        ScriptLanguage m_language;
        std::unordered_map<std::string, BindingInfo> m_functions;
        std::unordered_map<std::string, std::vector<BindingInfo>> m_classes;

        mutable std::shared_mutex m_bindingsMutex;
    };

    /**
     * @brief Script execution task
     */
    struct ScriptExecutionTask {
        std::string scriptId;
        std::string code;
        std::string filename;
        ScriptExecutionMode mode;
        std::shared_ptr<ScriptContext> context;
        std::chrono::steady_clock::time_point scheduledTime;
        std::function<void(ScriptResult)> callback;
        uint32_t priority;

        bool operator<(const ScriptExecutionTask& other) const {
            return priority < other.priority;
        }
    };

    /**
     * @brief Main scripting system
     */
    class ScriptingSystem : public System {
    public:
        static ScriptingSystem& GetInstance();

        ScriptingSystem();
        ~ScriptingSystem() override;

        // System interface
        bool Initialize() override;
        void Update(float deltaTime) override;
        void Shutdown() override;
        std::string GetName() const override { return "ScriptingSystem"; }
        System::Type GetType() const override { return System::Type::SCRIPTING; }

        // Context management
        std::shared_ptr<ScriptContext> CreateContext(const std::string& name, ScriptLanguage language);
        bool DestroyContext(const std::string& name);
        std::shared_ptr<ScriptContext> GetContext(const std::string& name) const;

        // Module management
        std::shared_ptr<ScriptModule> LoadModule(const std::string& name, const std::string& path);
        bool UnloadModule(const std::string& name);
        std::shared_ptr<ScriptModule> GetModule(const std::string& name) const;

        // Script execution
        ScriptResult ExecuteString(const std::string& code, const std::string& contextName = "default",
                                 ScriptExecutionMode mode = ScriptExecutionMode::SYNCHRONOUS);

        ScriptResult ExecuteFile(const std::string& filename, const std::string& contextName = "default",
                               ScriptExecutionMode mode = ScriptExecutionMode::SYNCHRONOUS);

        std::future<ScriptResult> ExecuteAsync(const std::string& code, const std::string& contextName = "default");

        // Coroutine management
        std::shared_ptr<ScriptCoroutine> CreateCoroutine(const std::string& name, const std::string& contextName);
        bool DestroyCoroutine(const std::string& name);
        std::shared_ptr<ScriptCoroutine> GetCoroutine(const std::string& name) const;

        // Debugging
        void AttachDebugger(const std::string& contextName);
        void DetachDebugger(const std::string& contextName);
        bool IsDebuggerAttached(const std::string& contextName) const;

        void AddBreakpoint(const std::string& filename, uint32_t line, const std::string& condition = "");
        void RemoveBreakpoint(const std::string& filename, uint32_t line);

        // Profiling
        void StartProfiling();
        void StopProfiling();
        bool IsProfiling() const;

        // Bindings
        void RegisterGlobalFunction(const std::string& name,
                                   const std::function<ScriptValue(const std::vector<ScriptValue>&)>& function,
                                   const std::vector<std::string>& parameters = {},
                                   const std::string& description = "");

        void RegisterClass(const std::string& name, const std::vector<std::string>& methods);

        bool GenerateBindings(const std::string& language, const std::string& outputPath);

        // Engine integration
        void RegisterEngineAPI();
        void RegisterMathAPI();
        void RegisterPhysicsAPI();
        void RegisterGraphicsAPI();
        void RegisterInputAPI();
        void RegisterAudioAPI();
        void RegisterNetworkAPI();
        void RegisterEntityAPI();
        void RegisterUIAPI();
        void RegisterFileAPI();
        void RegisterTimeAPI();

        // Configuration
        void SetMaxExecutionTime(uint64_t milliseconds);
        uint64_t GetMaxExecutionTime() const;

        void SetMaxMemoryUsage(size_t bytes);
        size_t GetMaxMemoryUsage() const;

        void SetEnableJIT(bool enable);
        bool IsJITEnabled() const;

        void SetEnableDebugger(bool enable);
        bool IsDebuggerEnabled() const;

        // Statistics
        struct ScriptStats {
            uint32_t totalContexts;
            uint32_t totalModules;
            uint32_t totalCoroutines;
            uint32_t executingScripts;
            uint32_t loadedScripts;
            uint32_t errorScripts;
            uint64_t totalExecutionTime;
            uint64_t averageExecutionTime;
            uint32_t functionCalls;
            size_t memoryUsage;
        };

        ScriptStats GetStats() const;

        // Utility functions
        std::string GetScriptInfo() const;
        void PrintScriptReport() const;

        bool ValidateScript(const std::string& code, ScriptLanguage language);
        std::string FormatScriptError(const std::string& error, const std::string& filename = "", uint32_t line = 0);

        // Hot reloading
        bool EnableHotReload(bool enable);
        bool IsHotReloadEnabled() const;

        void CheckForScriptChanges();
        bool ReloadChangedScripts();

        // Serialization
        bool SaveScriptState(const std::string& contextName, const std::string& filename);
        bool LoadScriptState(const std::string& contextName, const std::string& filename);

    private:
        // Core components
        mutable std::shared_mutex m_contextsMutex;
        std::unordered_map<std::string, std::shared_ptr<ScriptContext>> m_contexts;

        mutable std::shared_mutex m_modulesMutex;
        std::unordered_map<std::string, std::shared_ptr<ScriptModule>> m_modules;

        mutable std::shared_mutex m_coroutinesMutex;
        std::unordered_map<std::string, std::shared_ptr<ScriptCoroutine>> m_coroutines;

        // Execution system
        std::priority_queue<ScriptExecutionTask> m_executionQueue;
        std::unordered_map<std::string, ScriptExecutionTask> m_activeExecutions;
        mutable std::shared_mutex m_executionMutex;
        std::condition_variable_any m_executionCondition;
        std::vector<std::thread> m_executionThreads;

        // Debugging and profiling
        ScriptDebugger m_debugger;
        ScriptProfiler m_profiler;
        std::unordered_map<ScriptLanguage, std::unique_ptr<ScriptBindings>> m_bindings;

        // Configuration
        uint64_t m_maxExecutionTime;
        size_t m_maxMemoryUsage;
        bool m_enableJIT;
        bool m_enableDebugger;
        bool m_hotReloadEnabled;

        // Statistics
        mutable std::shared_mutex m_statsMutex;
        ScriptStats m_stats;

        // Internal methods
        void StartExecutionThreads();
        void StopExecutionThreads();
        void ExecutionThreadFunction();

        ScriptResult ExecuteScriptInternal(const ScriptExecutionTask& task);
        bool ProcessExecutionQueue();

        std::shared_ptr<ScriptContext> GetOrCreateDefaultContext(ScriptLanguage language);

        void UpdateContextStats(const std::string& contextName, ScriptResult result, uint64_t executionTime);
        void HandleScriptError(const std::string& contextName, const std::string& error);

        // Language-specific implementations
        bool InitializeLuaEngine();
        bool InitializePythonEngine();
        bool InitializeJavaScriptEngine();

        ScriptResult ExecuteLuaScript(const std::string& code, std::shared_ptr<ScriptContext> context);
        ScriptResult ExecutePythonScript(const std::string& code, std::shared_ptr<ScriptContext> context);
        ScriptResult ExecuteJavaScriptScript(const std::string& code, std::shared_ptr<ScriptContext> context);
    };

    // Lua-specific bindings
    class LuaBindings : public ScriptBindings {
    public:
        LuaBindings();
        ~LuaBindings() override = default;

        bool GenerateBindings(const std::string& outputPath) override;
        bool LoadBindings(const std::string& path) override;

        // Lua-specific functions
        void PushValue(lua_State* L, const ScriptValue& value);
        ScriptValue GetValue(lua_State* L, int index);

        static int LuaFunctionWrapper(lua_State* L);
    };

    // Python-specific bindings
    class PythonBindings : public ScriptBindings {
    public:
        PythonBindings();
        ~PythonBindings() override = default;

        bool GenerateBindings(const std::string& outputPath) override;
        bool LoadBindings(const std::string& path) override;

        // Python-specific functions
        PyObject* CreatePythonValue(const ScriptValue& value);
        ScriptValue FromPythonValue(PyObject* obj);

        static PyObject* PythonFunctionWrapper(PyObject* self, PyObject* args);
    };

    // Utility macros
    #define VOXELCRAFT_REGISTER_SCRIPT_FUNCTION(name, func) \
        ScriptingSystem::GetInstance().RegisterGlobalFunction(#name, func)

    #define VOXELCRAFT_EXECUTE_SCRIPT(code) \
        ScriptingSystem::GetInstance().ExecuteString(code)

    #define VOXELCRAFT_EXECUTE_SCRIPT_FILE(filename) \
        ScriptingSystem::GetInstance().ExecuteFile(filename)

    #define VOXELCRAFT_CREATE_COROUTINE(name, context) \
        ScriptingSystem::GetInstance().CreateCoroutine(name, context)

} // namespace VoxelCraft

#endif // VOXELCRAFT_SCRIPTING_SCRIPTING_SYSTEM_HPP
