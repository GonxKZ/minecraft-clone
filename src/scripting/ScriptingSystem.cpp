/**
 * @file ScriptingSystem.cpp
 * @brief VoxelCraft Advanced Scripting System Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#include "ScriptingSystem.hpp"

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
#include <queue>
#include <future>
#include <memory>
#include <filesystem>
#include <fstream>
#include <atomic>
#include <condition_variable>

// Lua integration (if available)
#ifdef VOXELCRAFT_LUA_ENABLED
    extern "C" {
        #include <lua.h>
        #include <lauxlib.h>
        #include <lualib.h>
    }
#endif

// Python integration (if available)
#ifdef VOXELCRAFT_PYTHON_ENABLED
    #include <Python.h>
#endif

// JavaScript integration (if available)
#ifdef VOXELCRAFT_JS_ENABLED
    // Include JavaScript engine headers
#endif

#include "../utils/Random.hpp"
#include "../logging/Logger.hpp"
#include "../memory/MemorySystem.hpp"

namespace VoxelCraft {

    // ScriptValue implementation
    ScriptValue::ScriptValue() : m_type(ScriptValueType::NIL) {
    }

    ScriptValue::ScriptValue(bool value) : m_type(ScriptValueType::BOOLEAN), m_value(value) {
    }

    ScriptValue::ScriptValue(int64_t value) : m_type(ScriptValueType::INTEGER), m_value(value) {
    }

    ScriptValue::ScriptValue(double value) : m_type(ScriptValueType::FLOAT), m_value(value) {
    }

    ScriptValue::ScriptValue(const std::string& value) : m_type(ScriptValueType::STRING), m_value(value) {
    }

    ScriptValue::ScriptValue(const glm::vec2& value) : m_type(ScriptValueType::VECTOR2), m_value(value) {
    }

    ScriptValue::ScriptValue(const glm::vec3& value) : m_type(ScriptValueType::VECTOR3), m_value(value) {
    }

    ScriptValue::ScriptValue(const glm::vec4& value) : m_type(ScriptValueType::VECTOR4), m_value(value) {
    }

    ScriptValue::ScriptValue(const glm::mat3& value) : m_type(ScriptValueType::MATRIX3), m_value(value) {
    }

    ScriptValue::ScriptValue(const glm::mat4& value) : m_type(ScriptValueType::MATRIX4), m_value(value) {
    }

    template<typename T>
    T ScriptValue::As() const {
        if (std::holds_alternative<T>(m_value)) {
            return std::get<T>(m_value);
        }
        throw std::bad_variant_access();
    }

    template<typename T>
    bool ScriptValue::Is() const {
        return std::holds_alternative<T>(m_value);
    }

    std::string ScriptValue::ToString() const {
        switch (m_type) {
            case ScriptValueType::NIL: return "nil";
            case ScriptValueType::BOOLEAN: return std::get<bool>(m_value) ? "true" : "false";
            case ScriptValueType::INTEGER: return std::to_string(std::get<int64_t>(m_value));
            case ScriptValueType::FLOAT: return std::to_string(std::get<double>(m_value));
            case ScriptValueType::STRING: return std::get<std::string>(m_value);
            case ScriptValueType::VECTOR2: {
                auto v = std::get<glm::vec2>(m_value);
                return "vec2(" + std::to_string(v.x) + ", " + std::to_string(v.y) + ")";
            }
            case ScriptValueType::VECTOR3: {
                auto v = std::get<glm::vec3>(m_value);
                return "vec3(" + std::to_string(v.x) + ", " + std::to_string(v.y) + ", " + std::to_string(v.z) + ")";
            }
            case ScriptValueType::VECTOR4: {
                auto v = std::get<glm::vec4>(m_value);
                return "vec4(" + std::to_string(v.x) + ", " + std::to_string(v.y) + ", " + std::to_string(v.z) + ", " + std::to_string(v.w) + ")";
            }
            default: return "unknown";
        }
    }

    // Explicit template instantiations
    template bool ScriptValue::As<bool>() const;
    template bool ScriptValue::As<int64_t>() const;
    template bool ScriptValue::As<double>() const;
    template bool ScriptValue::As<std::string>() const;
    template bool ScriptValue::As<glm::vec2>() const;
    template bool ScriptValue::As<glm::vec3>() const;
    template bool ScriptValue::As<glm::vec4>() const;
    template bool ScriptValue::As<glm::mat3>() const;
    template bool ScriptValue::As<glm::mat4>() const;

    template bool ScriptValue::Is<bool>() const;
    template bool ScriptValue::Is<int64_t>() const;
    template bool ScriptValue::Is<double>() const;
    template bool ScriptValue::Is<std::string>() const;
    template bool ScriptValue::Is<glm::vec2>() const;
    template bool ScriptValue::Is<glm::vec3>() const;
    template bool ScriptValue::Is<glm::vec4>() const;
    template bool ScriptValue::Is<glm::mat3>() const;
    template bool ScriptValue::Is<glm::mat4>() const;

    // ScriptContext implementation
    ScriptContext::ScriptContext(const std::string& name, ScriptLanguage language)
        : m_name(name), m_language(language), m_state(ScriptState::UNLOADED),
          m_timeout(5000), m_memoryLimit(1024 * 1024) { // 5 seconds, 1MB
    }

    void ScriptContext::SetGlobal(const std::string& name, const ScriptValue& value) {
        std::unique_lock<std::shared_mutex> lock(m_contextMutex);
        // Implementation depends on the scripting language
    }

    ScriptValue ScriptContext::GetGlobal(const std::string& name) const {
        std::shared_lock<std::shared_mutex> lock(m_contextMutex);
        // Implementation depends on the scripting language
        return ScriptValue();
    }

    ScriptResult ScriptContext::ExecuteString(const std::string& code) {
        std::unique_lock<std::shared_mutex> lock(m_contextMutex);

        if (m_state == ScriptState::ERROR) {
            return ScriptResult::ERROR;
        }

        m_state = ScriptState::EXECUTING;

        ScriptResult result = ScriptResult::SUCCESS;
        auto startTime = std::chrono::steady_clock::now();

        try {
            // Language-specific execution
            switch (m_language) {
                case ScriptLanguage::LUA:
                    result = ExecuteLuaScript(code);
                    break;
                case ScriptLanguage::PYTHON:
                    result = ExecutePythonScript(code);
                    break;
                case ScriptLanguage::JAVASCRIPT:
                    result = ExecuteJavaScriptScript(code);
                    break;
                default:
                    m_errorMessage = "Unsupported scripting language";
                    result = ScriptResult::ERROR;
                    break;
            }
        } catch (const std::exception& e) {
            m_errorMessage = e.what();
            result = ScriptResult::ERROR;
        }

        auto endTime = std::chrono::steady_clock::now();
        auto executionTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

        if (executionTime.count() > static_cast<int64_t>(m_timeout)) {
            m_errorMessage = "Script execution timed out";
            result = ScriptResult::TIMEOUT;
        }

        m_state = (result == ScriptResult::SUCCESS) ? ScriptState::FINISHED : ScriptState::ERROR;

        return result;
    }

    ScriptResult ScriptContext::ExecuteFile(const std::string& filename) {
        if (!std::filesystem::exists(filename)) {
            m_errorMessage = "Script file not found: " + filename;
            m_state = ScriptState::ERROR;
            return ScriptResult::ERROR;
        }

        std::ifstream file(filename);
        if (!file.is_open()) {
            m_errorMessage = "Failed to open script file: " + filename;
            m_state = ScriptState::ERROR;
            return ScriptResult::ERROR;
        }

        std::string code((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();

        return ExecuteString(code);
    }

    ScriptResult ScriptContext::CallFunction(const std::string& name, const std::vector<ScriptValue>& args) {
        std::unique_lock<std::shared_mutex> lock(m_contextMutex);

        if (m_state == ScriptState::ERROR) {
            return ScriptResult::ERROR;
        }

        m_state = ScriptState::EXECUTING;

        // Implementation depends on the scripting language
        // This would call the specified function with the given arguments

        m_state = ScriptState::FINISHED;
        return ScriptResult::SUCCESS;
    }

    void ScriptContext::Pause() {
        if (m_state == ScriptState::EXECUTING) {
            m_state = ScriptState::PAUSED;
        }
    }

    void ScriptContext::Resume() {
        if (m_state == ScriptState::PAUSED) {
            m_state = ScriptState::EXECUTING;
        }
    }

    void ScriptContext::Stop() {
        m_state = ScriptState::FINISHED;
    }

    // Language-specific execution methods
    ScriptResult ScriptContext::ExecuteLuaScript(const std::string& code) {
#ifdef VOXELCRAFT_LUA_ENABLED
        if (!m_languageData.has_value()) {
            // Initialize Lua state
            lua_State* L = luaL_newstate();
            if (!L) {
                m_errorMessage = "Failed to create Lua state";
                return ScriptResult::ERROR;
            }

            luaL_openlibs(L);
            m_languageData = L;
        }

        lua_State* L = std::any_cast<lua_State*>(m_languageData);

        if (luaL_dostring(L, code.c_str()) != LUA_OK) {
            m_errorMessage = lua_tostring(L, -1);
            lua_pop(L, 1);
            return ScriptResult::ERROR;
        }

        return ScriptResult::SUCCESS;
#else
        m_errorMessage = "Lua scripting is not enabled";
        return ScriptResult::ERROR;
#endif
    }

    ScriptResult ScriptContext::ExecutePythonScript(const std::string& code) {
#ifdef VOXELCRAFT_PYTHON_ENABLED
        if (!m_languageData.has_value()) {
            // Initialize Python
            Py_Initialize();
            m_languageData = true;
        }

        // Execute Python code
        if (PyRun_SimpleString(code.c_str()) != 0) {
            m_errorMessage = "Python script execution failed";
            return ScriptResult::ERROR;
        }

        return ScriptResult::SUCCESS;
#else
        m_errorMessage = "Python scripting is not enabled";
        return ScriptResult::ERROR;
#endif
    }

    ScriptResult ScriptContext::ExecuteJavaScriptScript(const std::string& code) {
#ifdef VOXELCRAFT_JS_ENABLED
        // JavaScript execution implementation
        m_errorMessage = "JavaScript scripting not yet implemented";
        return ScriptResult::ERROR;
#else
        m_errorMessage = "JavaScript scripting is not enabled";
        return ScriptResult::ERROR;
#endif
    }

    // ScriptCoroutine implementation
    ScriptCoroutine::ScriptCoroutine(const std::string& name, std::shared_ptr<ScriptContext> context)
        : m_name(name), m_context(context), m_running(false), m_paused(false),
          m_executionTime(0), m_yieldCount(0) {
    }

    ScriptResult ScriptCoroutine::Resume() {
        if (m_paused) {
            m_paused = false;
            m_running = true;
        } else if (!m_running) {
            m_running = true;
        }

        // Resume coroutine execution
        // Implementation depends on the scripting language

        return ScriptResult::SUCCESS;
    }

    ScriptResult ScriptCoroutine::Yield() {
        m_yieldCount++;
        m_paused = true;
        return ScriptResult::YIELD;
    }

    void ScriptCoroutine::Stop() {
        m_running = false;
        m_paused = false;
    }

    // ScriptModule implementation
    ScriptModule::ScriptModule(const std::string& name, const std::string& path)
        : m_name(name), m_path(path), m_loaded(false), m_autoReload(false) {
    }

    bool ScriptModule::Load() {
        std::unique_lock<std::shared_mutex> lock(m_moduleMutex);

        if (m_loaded) {
            return true;
        }

        if (!std::filesystem::exists(m_path)) {
            return false;
        }

        // Load all script files in the module directory
        for (const auto& entry : std::filesystem::directory_iterator(m_path)) {
            if (entry.is_regular_file()) {
                std::string filename = entry.path().string();
                std::string scriptName = entry.path().stem().string();

                // Determine language from file extension
                ScriptLanguage language = ScriptLanguage::LUA;
                std::string ext = entry.path().extension().string();
                if (ext == ".py") language = ScriptLanguage::PYTHON;
                else if (ext == ".js") language = ScriptLanguage::JAVASCRIPT;

                // Create script context
                auto context = std::make_shared<ScriptContext>(scriptName, language);
                if (context->ExecuteFile(filename) == ScriptResult::SUCCESS) {
                    m_scripts[scriptName] = context;
                }
            }
        }

        m_loaded = true;
        return true;
    }

    bool ScriptModule::Unload() {
        std::unique_lock<std::shared_mutex> lock(m_moduleMutex);

        m_scripts.clear();
        m_loaded = false;
        return true;
    }

    bool ScriptModule::Reload() {
        Unload();
        return Load();
    }

    std::shared_ptr<ScriptContext> ScriptModule::GetScript(const std::string& name) const {
        std::shared_lock<std::shared_mutex> lock(m_moduleMutex);

        auto it = m_scripts.find(name);
        return it != m_scripts.end() ? it->second : nullptr;
    }

    std::vector<std::string> ScriptModule::GetScriptNames() const {
        std::shared_lock<std::shared_mutex> lock(m_moduleMutex);

        std::vector<std::string> names;
        for (const auto& pair : m_scripts) {
            names.push_back(pair.first);
        }

        return names;
    }

    // ScriptDebugger implementation
    ScriptDebugger::ScriptDebugger() : m_paused(false) {
    }

    void ScriptDebugger::AttachToContext(std::shared_ptr<ScriptContext> context) {
        std::unique_lock<std::shared_mutex> lock(m_debuggerMutex);
        m_attachedContexts[context->GetName()] = context;
    }

    void ScriptDebugger::DetachFromContext(const std::string& contextName) {
        std::unique_lock<std::shared_mutex> lock(m_debuggerMutex);
        m_attachedContexts.erase(contextName);
    }

    void ScriptDebugger::AddBreakpoint(const Breakpoint& breakpoint) {
        std::unique_lock<std::shared_mutex> lock(m_debuggerMutex);
        m_breakpoints.push_back(breakpoint);
    }

    void ScriptDebugger::RemoveBreakpoint(const std::string& filename, uint32_t line) {
        std::unique_lock<std::shared_mutex> lock(m_debuggerMutex);

        m_breakpoints.erase(std::remove_if(m_breakpoints.begin(), m_breakpoints.end(),
            [&](const Breakpoint& bp) {
                return bp.filename == filename && bp.line == line;
            }), m_breakpoints.end());
    }

    void ScriptDebugger::ClearBreakpoints() {
        std::unique_lock<std::shared_mutex> lock(m_debuggerMutex);
        m_breakpoints.clear();
    }

    void ScriptDebugger::Continue() {
        m_paused = false;
    }

    void ScriptDebugger::StepOver() {
        // Implement step over logic
    }

    void ScriptDebugger::StepInto() {
        // Implement step into logic
    }

    void ScriptDebugger::StepOut() {
        // Implement step out logic
    }

    std::vector<ScriptDebugger::CallStackFrame> ScriptDebugger::GetCallStack() const {
        std::shared_lock<std::shared_mutex> lock(m_debuggerMutex);
        return m_callStack;
    }

    std::vector<ScriptDebugger::Breakpoint> ScriptDebugger::GetBreakpoints() const {
        std::shared_lock<std::shared_mutex> lock(m_debuggerMutex);
        return m_breakpoints;
    }

    void ScriptDebugger::SetVariable(const std::string& name, const ScriptValue& value) {
        // Set variable in the current context
    }

    ScriptValue ScriptDebugger::GetVariable(const std::string& name) const {
        // Get variable from the current context
        return ScriptValue();
    }

    // ScriptProfiler implementation
    ScriptProfiler::ScriptProfiler() : m_profiling(false) {
    }

    void ScriptProfiler::StartProfiling() {
        std::unique_lock<std::shared_mutex> lock(m_profilerMutex);
        m_profiling = true;
        m_profileData.clear();
        m_callStack.clear();
    }

    void ScriptProfiler::StopProfiling() {
        std::unique_lock<std::shared_mutex> lock(m_profilerMutex);
        m_profiling = false;
    }

    void ScriptProfiler::Reset() {
        std::unique_lock<std::shared_mutex> lock(m_profilerMutex);
        m_profileData.clear();
        m_callStack.clear();
    }

    void ScriptProfiler::BeginFunction(const std::string& name, const std::string& filename) {
        if (!m_profiling) return;

        std::unique_lock<std::shared_mutex> lock(m_profilerMutex);
        m_callStack.emplace_back(name, std::chrono::steady_clock::now());
    }

    void ScriptProfiler::EndFunction(const std::string& name) {
        if (!m_profiling || m_callStack.empty()) return;

        std::unique_lock<std::shared_mutex> lock(m_profilerMutex);

        auto endTime = std::chrono::steady_clock::now();
        auto& call = m_callStack.back();

        if (call.first == name) {
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - call.second);

            auto& data = m_profileData[name];
            data.functionName = name;
            data.callCount++;
            data.totalTime += duration.count();
            data.selfTime += duration.count(); // Simplified - should subtract child times
            data.minTime = data.minTime == 0 ? duration.count() : std::min(data.minTime, duration.count());
            data.maxTime = std::max(data.maxTime, duration.count());
            data.avgTime = data.totalTime / data.callCount;

            m_callStack.pop_back();
        }
    }

    std::vector<ScriptProfiler::ProfileData> ScriptProfiler::GetProfileData() const {
        std::shared_lock<std::shared_mutex> lock(m_profilerMutex);

        std::vector<ProfileData> result;
        for (const auto& pair : m_profileData) {
            result.push_back(pair.second);
        }

        return result;
    }

    ScriptProfiler::ProfileData ScriptProfiler::GetFunctionData(const std::string& name) const {
        std::shared_lock<std::shared_mutex> lock(m_profilerMutex);

        auto it = m_profileData.find(name);
        return it != m_profileData.end() ? it->second : ProfileData();
    }

    void ScriptProfiler::GenerateReport(const std::string& filename) const {
        std::shared_lock<std::shared_mutex> lock(m_profilerMutex);

        std::ofstream file(filename);
        if (!file.is_open()) {
            return;
        }

        file << "=== Script Profiling Report ===\n\n";

        for (const auto& pair : m_profileData) {
            const auto& data = pair.second;
            file << "Function: " << data.functionName << "\n";
            file << "  Calls: " << data.callCount << "\n";
            file << "  Total Time: " << data.totalTime << " μs\n";
            file << "  Self Time: " << data.selfTime << " μs\n";
            file << "  Min Time: " << data.minTime << " μs\n";
            file << "  Max Time: " << data.maxTime << " μs\n";
            file << "  Avg Time: " << data.avgTime << " μs\n\n";
        }

        file.close();
    }

    // ScriptBindings implementation
    ScriptBindings::ScriptBindings(ScriptLanguage language)
        : m_language(language) {
    }

    void ScriptBindings::RegisterFunction(const std::string& name,
                                        const std::function<ScriptValue(const std::vector<ScriptValue>&)>& function,
                                        const std::vector<std::string>& parameters,
                                        const std::string& description) {
        std::unique_lock<std::shared_mutex> lock(m_bindingsMutex);

        BindingInfo info;
        info.name = name;
        info.description = description;
        info.parameters = parameters;
        info.returnType = "ScriptValue";
        info.function = function;

        m_functions[name] = info;
    }

    void ScriptBindings::RegisterClass(const std::string& name, const std::vector<BindingInfo>& methods) {
        std::unique_lock<std::shared_mutex> lock(m_bindingsMutex);
        m_classes[name] = methods;
    }

    bool ScriptBindings::GenerateBindings(const std::string& outputPath) {
        // Implementation depends on the target language
        return false;
    }

    bool ScriptBindings::LoadBindings(const std::string& path) {
        // Implementation depends on the target language
        return false;
    }

    std::vector<std::string> ScriptBindings::GetRegisteredFunctions() const {
        std::shared_lock<std::shared_mutex> lock(m_bindingsMutex);

        std::vector<std::string> functions;
        for (const auto& pair : m_functions) {
            functions.push_back(pair.first);
        }

        return functions;
    }

    std::vector<std::string> ScriptBindings::GetRegisteredClasses() const {
        std::shared_lock<std::shared_mutex> lock(m_bindingsMutex);

        std::vector<std::string> classes;
        for (const auto& pair : m_classes) {
            classes.push_back(pair.first);
        }

        return classes;
    }

    ScriptBindings::BindingInfo ScriptBindings::GetFunctionInfo(const std::string& name) const {
        std::shared_lock<std::shared_mutex> lock(m_bindingsMutex);

        auto it = m_functions.find(name);
        return it != m_functions.end() ? it->second : BindingInfo();
    }

    // Main ScriptingSystem implementation
    ScriptingSystem& ScriptingSystem::GetInstance() {
        static ScriptingSystem instance;
        return instance;
    }

    ScriptingSystem::ScriptingSystem()
        : m_maxExecutionTime(5000), m_maxMemoryUsage(10 * 1024 * 1024),
          m_enableJIT(true), m_enableDebugger(false), m_hotReloadEnabled(false) {
    }

    ScriptingSystem::~ScriptingSystem() {
        Shutdown();
    }

    bool ScriptingSystem::Initialize() {
        if (!System::Initialize()) {
            return false;
        }

        Logger::GetInstance().Info("ScriptingSystem initializing", "ScriptingSystem");

        // Initialize scripting engines
        if (!InitializeLuaEngine()) {
            Logger::GetInstance().Warning("Failed to initialize Lua engine", "ScriptingSystem");
        }

        if (!InitializePythonEngine()) {
            Logger::GetInstance().Warning("Failed to initialize Python engine", "ScriptingSystem");
        }

        if (!InitializeJavaScriptEngine()) {
            Logger::GetInstance().Warning("Failed to initialize JavaScript engine", "ScriptingSystem");
        }

        // Start execution threads
        StartExecutionThreads();

        // Register engine APIs
        RegisterEngineAPI();
        RegisterMathAPI();
        RegisterPhysicsAPI();
        RegisterGraphicsAPI();
        RegisterInputAPI();
        RegisterAudioAPI();
        RegisterNetworkAPI();
        RegisterEntityAPI();
        RegisterUIAPI();
        RegisterFileAPI();
        RegisterTimeAPI();

        Logger::GetInstance().Info("ScriptingSystem initialized successfully", "ScriptingSystem");
        return true;
    }

    void ScriptingSystem::Update(float deltaTime) {
        System::Update(deltaTime);

        // Process execution queue
        ProcessExecutionQueue();

        // Update coroutines
        UpdateCoroutines(deltaTime);

        // Check for script changes if hot reload is enabled
        if (m_hotReloadEnabled) {
            CheckForScriptChanges();
        }
    }

    void ScriptingSystem::Shutdown() {
        if (!System::Shutdown()) {
            return;
        }

        Logger::GetInstance().Info("ScriptingSystem shutting down", "ScriptingSystem");

        // Stop execution threads
        StopExecutionThreads();

        // Clear all contexts
        {
            std::unique_lock<std::shared_mutex> lock(m_contextsMutex);
            m_contexts.clear();
        }

        // Clear all modules
        {
            std::unique_lock<std::shared_mutex> lock(m_modulesMutex);
            m_modules.clear();
        }

        // Clear all coroutines
        {
            std::unique_lock<std::shared_mutex> lock(m_coroutinesMutex);
            m_coroutines.clear();
        }

        // Clear execution queue
        {
            std::unique_lock<std::shared_mutex> lock(m_executionMutex);
            while (!m_executionQueue.empty()) {
                m_executionQueue.pop();
            }
            m_activeExecutions.clear();
        }

        // Shutdown scripting engines
#ifdef VOXELCRAFT_LUA_ENABLED
        // Lua cleanup
#endif

#ifdef VOXELCRAFT_PYTHON_ENABLED
        Py_FinalizeEx();
#endif

        Logger::GetInstance().Info("ScriptingSystem shutdown complete", "ScriptingSystem");
    }

    std::shared_ptr<ScriptContext> ScriptingSystem::CreateContext(const std::string& name, ScriptLanguage language) {
        std::unique_lock<std::shared_mutex> lock(m_contextsMutex);

        if (m_contexts.find(name) != m_contexts.end()) {
            return m_contexts[name];
        }

        auto context = std::make_shared<ScriptContext>(name, language);
        m_contexts[name] = context;

        Logger::GetInstance().Debug("Created script context: " + name, "ScriptingSystem");
        return context;
    }

    bool ScriptingSystem::DestroyContext(const std::string& name) {
        std::unique_lock<std::shared_mutex> lock(m_contextsMutex);

        auto it = m_contexts.find(name);
        if (it == m_contexts.end()) {
            return false;
        }

        m_contexts.erase(it);

        Logger::GetInstance().Debug("Destroyed script context: " + name, "ScriptingSystem");
        return true;
    }

    std::shared_ptr<ScriptContext> ScriptingSystem::GetContext(const std::string& name) const {
        std::shared_lock<std::shared_mutex> lock(m_contextsMutex);

        auto it = m_contexts.find(name);
        return it != m_contexts.end() ? it->second : nullptr;
    }

    std::shared_ptr<ScriptModule> ScriptingSystem::LoadModule(const std::string& name, const std::string& path) {
        std::unique_lock<std::shared_mutex> lock(m_modulesMutex);

        if (m_modules.find(name) != m_modules.end()) {
            return m_modules[name];
        }

        auto module = std::make_shared<ScriptModule>(name, path);
        if (module->Load()) {
            m_modules[name] = module;
            Logger::GetInstance().Debug("Loaded script module: " + name, "ScriptingSystem");
            return module;
        }

        return nullptr;
    }

    bool ScriptingSystem::UnloadModule(const std::string& name) {
        std::unique_lock<std::shared_mutex> lock(m_modulesMutex);

        auto it = m_modules.find(name);
        if (it == m_modules.end()) {
            return false;
        }

        it->second->Unload();
        m_modules.erase(it);

        Logger::GetInstance().Debug("Unloaded script module: " + name, "ScriptingSystem");
        return true;
    }

    std::shared_ptr<ScriptModule> ScriptingSystem::GetModule(const std::string& name) const {
        std::shared_lock<std::shared_mutex> lock(m_modulesMutex);

        auto it = m_modules.find(name);
        return it != m_modules.end() ? it->second : nullptr;
    }

    ScriptResult ScriptingSystem::ExecuteString(const std::string& code, const std::string& contextName,
                                              ScriptExecutionMode mode) {
        auto context = GetContext(contextName);
        if (!context) {
            context = GetOrCreateDefaultContext(ScriptLanguage::LUA);
        }

        if (mode == ScriptExecutionMode::ASYNCHRONOUS) {
            // Queue for async execution
            ScriptExecutionTask task;
            task.scriptId = "inline_" + std::to_string(Random::GetInstance().GetUint64());
            task.code = code;
            task.mode = mode;
            task.context = context;
            task.priority = 0;

            std::unique_lock<std::shared_mutex> lock(m_executionMutex);
            m_executionQueue.push(task);
            m_executionCondition.notify_one();

            return ScriptResult::SUCCESS;
        }

        return context->ExecuteString(code);
    }

    ScriptResult ScriptingSystem::ExecuteFile(const std::string& filename, const std::string& contextName,
                                            ScriptExecutionMode mode) {
        auto context = GetContext(contextName);
        if (!context) {
            context = GetOrCreateDefaultContext(ScriptLanguage::LUA);
        }

        if (mode == ScriptExecutionMode::ASYNCHRONOUS) {
            // Queue for async execution
            ScriptExecutionTask task;
            task.scriptId = filename;
            task.filename = filename;
            task.mode = mode;
            task.context = context;
            task.priority = 0;

            std::unique_lock<std::shared_mutex> lock(m_executionMutex);
            m_executionQueue.push(task);
            m_executionCondition.notify_one();

            return ScriptResult::SUCCESS;
        }

        return context->ExecuteFile(filename);
    }

    std::future<ScriptResult> ScriptingSystem::ExecuteAsync(const std::string& code, const std::string& contextName) {
        return std::async(std::launch::async, [this, code, contextName]() {
            return ExecuteString(code, contextName, ScriptExecutionMode::SYNCHRONOUS);
        });
    }

    std::shared_ptr<ScriptCoroutine> ScriptingSystem::CreateCoroutine(const std::string& name, const std::string& contextName) {
        auto context = GetContext(contextName);
        if (!context) {
            return nullptr;
        }

        std::unique_lock<std::shared_mutex> lock(m_coroutinesMutex);

        if (m_coroutines.find(name) != m_coroutines.end()) {
            return m_coroutines[name];
        }

        auto coroutine = std::make_shared<ScriptCoroutine>(name, context);
        m_coroutines[name] = coroutine;

        Logger::GetInstance().Debug("Created script coroutine: " + name, "ScriptingSystem");
        return coroutine;
    }

    bool ScriptingSystem::DestroyCoroutine(const std::string& name) {
        std::unique_lock<std::shared_mutex> lock(m_coroutinesMutex);

        auto it = m_coroutines.find(name);
        if (it == m_coroutines.end()) {
            return false;
        }

        it->second->Stop();
        m_coroutines.erase(it);

        Logger::GetInstance().Debug("Destroyed script coroutine: " + name, "ScriptingSystem");
        return true;
    }

    std::shared_ptr<ScriptCoroutine> ScriptingSystem::GetCoroutine(const std::string& name) const {
        std::shared_lock<std::shared_mutex> lock(m_coroutinesMutex);

        auto it = m_coroutines.find(name);
        return it != m_coroutines.end() ? it->second : nullptr;
    }

    void ScriptingSystem::RegisterGlobalFunction(const std::string& name,
                                                const std::function<ScriptValue(const std::vector<ScriptValue>&)>& function,
                                                const std::vector<std::string>& parameters,
                                                const std::string& description) {
        // Register in all available bindings
        for (auto& pair : m_bindings) {
            if (pair.second) {
                pair.second->RegisterFunction(name, function, parameters, description);
            }
        }
    }

    void ScriptingSystem::RegisterClass(const std::string& name, const std::vector<std::string>& methods) {
        // Implementation for class registration
    }

    bool ScriptingSystem::GenerateBindings(const std::string& language, const std::string& outputPath) {
        ScriptLanguage lang = ScriptLanguage::LUA;
        if (language == "python") lang = ScriptLanguage::PYTHON;
        else if (language == "javascript") lang = ScriptLanguage::JAVASCRIPT;

        auto it = m_bindings.find(lang);
        if (it != m_bindings.end() && it->second) {
            return it->second->GenerateBindings(outputPath);
        }

        return false;
    }

    // API Registration methods
    void ScriptingSystem::RegisterEngineAPI() {
        // Register core engine functions
        RegisterGlobalFunction("log", [](const std::vector<ScriptValue>& args) -> ScriptValue {
            if (!args.empty()) {
                Logger::GetInstance().Info(args[0].ToString(), "Script");
            }
            return ScriptValue();
        }, {"message"}, "Log a message to the console");

        RegisterGlobalFunction("get_delta_time", []() -> ScriptValue {
            return ScriptValue(0.016f); // Simplified
        }, {}, "Get the time elapsed since the last frame");
    }

    void ScriptingSystem::RegisterMathAPI() {
        // Register math functions
        RegisterGlobalFunction("sin", [](const std::vector<ScriptValue>& args) -> ScriptValue {
            if (!args.empty() && args[0].Is<double>()) {
                return ScriptValue(std::sin(args[0].As<double>()));
            }
            return ScriptValue(0.0);
        }, {"angle"}, "Calculate sine of an angle");

        RegisterGlobalFunction("cos", [](const std::vector<ScriptValue>& args) -> ScriptValue {
            if (!args.empty() && args[0].Is<double>()) {
                return ScriptValue(std::cos(args[0].As<double>()));
            }
            return ScriptValue(0.0);
        }, {"angle"}, "Calculate cosine of an angle");

        RegisterGlobalFunction("sqrt", [](const std::vector<ScriptValue>& args) -> ScriptValue {
            if (!args.empty() && args[0].Is<double>()) {
                return ScriptValue(std::sqrt(args[0].As<double>()));
            }
            return ScriptValue(0.0);
        }, {"value"}, "Calculate square root");
    }

    void ScriptingSystem::RegisterPhysicsAPI() {
        // Register physics functions
        RegisterGlobalFunction("apply_force", [](const std::vector<ScriptValue>& args) -> ScriptValue {
            // Implementation for applying force to entities
            return ScriptValue(true);
        }, {"entity", "force"}, "Apply a force to an entity");
    }

    void ScriptingSystem::RegisterGraphicsAPI() {
        // Register graphics functions
        RegisterGlobalFunction("draw_line", [](const std::vector<ScriptValue>& args) -> ScriptValue {
            // Implementation for drawing lines
            return ScriptValue();
        }, {"start", "end", "color"}, "Draw a line in 3D space");
    }

    void ScriptingSystem::RegisterInputAPI() {
        // Register input functions
        RegisterGlobalFunction("is_key_pressed", [](const std::vector<ScriptValue>& args) -> ScriptValue {
            // Implementation for key checking
            return ScriptValue(false);
        }, {"key"}, "Check if a key is pressed");
    }

    void ScriptingSystem::RegisterAudioAPI() {
        // Register audio functions
        RegisterGlobalFunction("play_sound", [](const std::vector<ScriptValue>& args) -> ScriptValue {
            // Implementation for playing sounds
            return ScriptValue();
        }, {"sound_name"}, "Play a sound effect");
    }

    void ScriptingSystem::RegisterNetworkAPI() {
        // Register network functions
        RegisterGlobalFunction("send_message", [](const std::vector<ScriptValue>& args) -> ScriptValue {
            // Implementation for sending network messages
            return ScriptValue();
        }, {"message"}, "Send a network message");
    }

    void ScriptingSystem::RegisterEntityAPI() {
        // Register entity functions
        RegisterGlobalFunction("create_entity", [](const std::vector<ScriptValue>& args) -> ScriptValue {
            // Implementation for creating entities
            return ScriptValue(0);
        }, {"template"}, "Create a new entity");
    }

    void ScriptingSystem::RegisterUIAPI() {
        // Register UI functions
        RegisterGlobalFunction("show_message", [](const std::vector<ScriptValue>& args) -> ScriptValue {
            // Implementation for showing UI messages
            return ScriptValue();
        }, {"message"}, "Show a message to the player");
    }

    void ScriptingSystem::RegisterFileAPI() {
        // Register file functions
        RegisterGlobalFunction("read_file", [](const std::vector<ScriptValue>& args) -> ScriptValue {
            if (!args.empty() && args[0].Is<std::string>()) {
                // Implementation for reading files
                return ScriptValue(std::string(""));
            }
            return ScriptValue();
        }, {"filename"}, "Read a file from disk");
    }

    void ScriptingSystem::RegisterTimeAPI() {
        // Register time functions
        RegisterGlobalFunction("get_time", []() -> ScriptValue {
            auto now = std::chrono::system_clock::now();
            auto time = std::chrono::system_clock::to_time_t(now);
            return ScriptValue(static_cast<int64_t>(time));
        }, {}, "Get the current system time");
    }

    // Private methods implementation
    void ScriptingSystem::StartExecutionThreads() {
        // Start worker threads for async script execution
        for (uint32_t i = 0; i < 4; ++i) {
            m_executionThreads.emplace_back(&ScriptingSystem::ExecutionThreadFunction, this);
        }

        Logger::GetInstance().Info("Started 4 script execution threads", "ScriptingSystem");
    }

    void ScriptingSystem::StopExecutionThreads() {
        // Signal threads to stop
        {
            std::unique_lock<std::shared_mutex> lock(m_executionMutex);
            // Add termination tasks
        }
        m_executionCondition.notify_all();

        // Wait for threads to finish
        for (auto& thread : m_executionThreads) {
            if (thread.joinable()) {
                thread.join();
            }
        }

        m_executionThreads.clear();

        Logger::GetInstance().Info("Stopped script execution threads", "ScriptingSystem");
    }

    void ScriptingSystem::ExecutionThreadFunction() {
        while (true) {
            ScriptExecutionTask task;

            {
                std::unique_lock<std::shared_mutex> lock(m_executionMutex);
                m_executionCondition.wait(lock, [this]() {
                    return !m_executionQueue.empty();
                });

                if (m_executionQueue.empty()) {
                    break; // Shutdown signal
                }

                task = m_executionQueue.top();
                m_executionQueue.pop();
                m_activeExecutions[task.scriptId] = task;
            }

            // Execute the script
            auto result = ExecuteScriptInternal(task);

            {
                std::unique_lock<std::shared_mutex> lock(m_executionMutex);
                m_activeExecutions.erase(task.scriptId);

                if (task.callback) {
                    task.callback(result);
                }
            }
        }
    }

    ScriptResult ScriptingSystem::ExecuteScriptInternal(const ScriptExecutionTask& task) {
        auto startTime = std::chrono::steady_clock::now();

        ScriptResult result = ScriptResult::SUCCESS;
        if (task.context) {
            if (!task.filename.empty()) {
                result = task.context->ExecuteFile(task.filename);
            } else {
                result = task.context->ExecuteString(task.code);
            }
        }

        auto endTime = std::chrono::steady_clock::now();
        uint64_t executionTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

        UpdateContextStats(task.context ? task.context->GetName() : "unknown", result, executionTime);

        return result;
    }

    bool ScriptingSystem::ProcessExecutionQueue() {
        // Process scheduled tasks
        auto now = std::chrono::steady_clock::now();

        std::unique_lock<std::shared_mutex> lock(m_executionMutex);

        // Process scheduled tasks (simplified - would need proper priority queue with time)
        // For now, just process immediate tasks

        return true;
    }

    void ScriptingSystem::UpdateCoroutines(float deltaTime) {
        std::shared_lock<std::shared_mutex> lock(m_coroutinesMutex);

        for (auto& pair : m_coroutines) {
            auto& coroutine = pair.second;
            if (coroutine->IsRunning() && !coroutine->IsPaused()) {
                // Resume coroutine if needed
            }
        }
    }

    void ScriptingSystem::CheckForScriptChanges() {
        // Implementation for hot reload checking
    }

    void ScriptingSystem::UpdateContextStats(const std::string& contextName, ScriptResult result, uint64_t executionTime) {
        std::unique_lock<std::shared_mutex> lock(m_statsMutex);

        if (result == ScriptResult::SUCCESS) {
            m_stats.executingScripts--;
            m_stats.totalExecutionTime += executionTime;
            m_stats.averageExecutionTime = m_stats.totalExecutionTime / std::max(1u, m_stats.functionCalls);
        } else if (result == ScriptResult::ERROR) {
            m_stats.errorScripts++;
        }
    }

    std::shared_ptr<ScriptContext> ScriptingSystem::GetOrCreateDefaultContext(ScriptLanguage language) {
        std::string contextName = "default_" + std::to_string(static_cast<int>(language));
        auto context = GetContext(contextName);
        if (!context) {
            context = CreateContext(contextName, language);
        }
        return context;
    }

    bool ScriptingSystem::InitializeLuaEngine() {
#ifdef VOXELCRAFT_LUA_ENABLED
        // Initialize Lua bindings
        m_bindings[ScriptLanguage::LUA] = std::make_unique<LuaBindings>();
        return true;
#else
        return false;
#endif
    }

    bool ScriptingSystem::InitializePythonEngine() {
#ifdef VOXELCRAFT_PYTHON_ENABLED
        // Initialize Python bindings
        m_bindings[ScriptLanguage::PYTHON] = std::make_unique<PythonBindings>();
        return true;
#else
        return false;
#endif
    }

    bool ScriptingSystem::InitializeJavaScriptEngine() {
#ifdef VOXELCRAFT_JS_ENABLED
        // Initialize JavaScript bindings
        return true;
#else
        return false;
#endif
    }

    // Utility methods
    std::string ScriptingSystem::GetScriptInfo() const {
        std::stringstream ss;
        ss << "Scripting System Info:\n";
        ss << "Contexts: " << m_contexts.size() << "\n";
        ss << "Modules: " << m_modules.size() << "\n";
        ss << "Coroutines: " << m_coroutines.size() << "\n";
        ss << "Active Executions: " << m_activeExecutions.size() << "\n";
        ss << "JIT Enabled: " << (m_enableJIT ? "Yes" : "No") << "\n";
        ss << "Debugger Enabled: " << (m_enableDebugger ? "Yes" : "No") << "\n";
        ss << "Hot Reload: " << (m_hotReloadEnabled ? "Enabled" : "Disabled") << "\n";
        return ss.str();
    }

    void ScriptingSystem::PrintScriptReport() const {
        auto stats = GetStats();
        std::stringstream report;

        report << "=== Scripting System Report ===\n";
        report << "Total Contexts: " << stats.totalContexts << "\n";
        report << "Total Modules: " << stats.totalModules << "\n";
        report << "Total Coroutines: " << stats.totalCoroutines << "\n";
        report << "Executing Scripts: " << stats.executingScripts << "\n";
        report << "Loaded Scripts: " << stats.loadedScripts << "\n";
        report << "Error Scripts: " << stats.errorScripts << "\n";
        report << "Total Execution Time: " << stats.totalExecutionTime << " ms\n";
        report << "Average Execution Time: " << stats.averageExecutionTime << " ms\n";
        report << "Function Calls: " << stats.functionCalls << "\n";
        report << "Memory Usage: " << stats.memoryUsage << " bytes\n";

        Logger::GetInstance().Info(report.str(), "ScriptingSystem");
    }

    ScriptingSystem::ScriptStats ScriptingSystem::GetStats() const {
        std::shared_lock<std::shared_mutex> lock(m_statsMutex);
        return m_stats;
    }

    // LuaBindings implementation
#ifdef VOXELCRAFT_LUA_ENABLED
    LuaBindings::LuaBindings() : ScriptBindings(ScriptLanguage::LUA) {
    }

    bool LuaBindings::GenerateBindings(const std::string& outputPath) {
        // Generate Lua binding code
        return false; // Placeholder
    }

    bool LuaBindings::LoadBindings(const std::string& path) {
        // Load Lua binding code
        return false; // Placeholder
    }

    void LuaBindings::PushValue(lua_State* L, const ScriptValue& value) {
        // Implementation for pushing values to Lua stack
    }

    ScriptValue LuaBindings::GetValue(lua_State* L, int index) {
        // Implementation for getting values from Lua stack
        return ScriptValue();
    }

    int LuaBindings::LuaFunctionWrapper(lua_State* L) {
        // Implementation for Lua function wrapper
        return 0;
    }
#endif

    // PythonBindings implementation
#ifdef VOXELCRAFT_PYTHON_ENABLED
    PythonBindings::PythonBindings() : ScriptBindings(ScriptLanguage::PYTHON) {
    }

    bool PythonBindings::GenerateBindings(const std::string& outputPath) {
        // Generate Python binding code
        return false; // Placeholder
    }

    bool PythonBindings::LoadBindings(const std::string& path) {
        // Load Python binding code
        return false; // Placeholder
    }

    PyObject* PythonBindings::CreatePythonValue(const ScriptValue& value) {
        // Implementation for creating Python objects
        return nullptr;
    }

    ScriptValue PythonBindings::FromPythonValue(PyObject* obj) {
        // Implementation for converting Python objects
        return ScriptValue();
    }

    PyObject* PythonBindings::PythonFunctionWrapper(PyObject* self, PyObject* args) {
        // Implementation for Python function wrapper
        return nullptr;
    }
#endif

} // namespace VoxelCraft
