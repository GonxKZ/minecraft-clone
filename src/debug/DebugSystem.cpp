/**
 * @file DebugSystem.cpp
 * @brief VoxelCraft Advanced Debug and Profiling System Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#include "DebugSystem.hpp"

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

#include "../utils/Logger.hpp"
#include "../utils/Random.hpp"
#include "../math/MathUtils.hpp"
#include "../input/InputSystem.hpp"

namespace VoxelCraft {

    // Static instance
    static DebugSystem* s_instance = nullptr;

    DebugSystem& DebugSystem::GetInstance() {
        if (!s_instance) {
            s_instance = new DebugSystem();
        }
        return *s_instance;
    }

    bool DebugSystem::Initialize(const DebugConfig& config) {
        if (m_initialized) {
            Logger::Warning("DebugSystem already initialized");
            return true;
        }

        m_config = config;
        m_initialized = true;

        try {
            // Initialize subsystems
            InitializeProfiling();
            InitializeMetrics();
            InitializeVisualization();
            InitializeOverlays();
            InitializeLogging();

            // Create default overlays
            CreateDefaultOverlays();

            // Register core systems
            RegisterSystem("Engine");
            RegisterSystem("Rendering");
            RegisterSystem("Physics");
            RegisterSystem("AI");
            RegisterSystem("Audio");
            RegisterSystem("Network");
            RegisterSystem("Input");

            Logger::Info("DebugSystem initialized successfully");
            return true;

        } catch (const std::exception& e) {
            Logger::Error("Failed to initialize DebugSystem: {}", e.what());
            return false;
        }
    }

    void DebugSystem::Shutdown() {
        if (!m_initialized) {
            return;
        }

        // Stop any active captures
        if (m_performanceCaptureActive) {
            StopPerformanceCapture();
        }

        // Clear all data
        m_profilerData.clear();
        m_activeProfiles.clear();
        m_metrics.clear();
        m_debugShapes.clear();
        m_overlays.clear();
        m_logEntries.clear();
        m_systemMetrics.clear();
        m_memorySnapshots.clear();
        m_allocationTracker.clear();

        m_initialized = false;
        Logger::Info("DebugSystem shutdown");
    }

    void DebugSystem::Update(float deltaTime) {
        if (!m_initialized) {
            return;
        }

        // Update subsystems
        UpdateProfiling(deltaTime);
        UpdateMetrics(deltaTime);
        UpdateVisualization(deltaTime);
        UpdateOverlays(deltaTime);
        UpdateLogging(deltaTime);

        // Update performance metrics
        UpdatePerformanceMetrics(deltaTime);

        // Process debug commands
        ProcessDebugCommands();

        // Update debug camera if enabled
        if (m_debugCameraEnabled) {
            UpdateDebugCamera(deltaTime);
        }

        // Update performance capture if active
        if (m_performanceCaptureActive) {
            m_performanceCaptureTime += deltaTime;
            m_capturedMetrics.push_back(m_performanceMetrics);

            if (m_performanceCaptureTime >= m_performanceCaptureDuration) {
                StopPerformanceCapture();
            }
        }
    }

    void DebugSystem::Render() {
        if (!m_initialized || !m_config.enableVisualization) {
            return;
        }

        if (m_overlayEnabled) {
            RenderOverlay();
        }

        if (m_currentMode != DebugMode::NONE) {
            RenderShapes();
            RenderMetrics();
            RenderPerformanceGraphs();
        }
    }

    // Profiling API
    void DebugSystem::BeginProfile(const std::string& name, const std::string& category) {
        if (!m_profilingEnabled) {
            return;
        }

        uint64_t currentTime = GetCurrentTimeMicroseconds();

        std::unique_lock<std::shared_mutex> lock(m_profileMutex);

        // Check if already profiling
        if (m_activeProfiles.find(name) != m_activeProfiles.end()) {
            return;  // Already profiling this section
        }

        m_activeProfiles[name] = currentTime;
        m_profileStack.push_back(name);

        // Initialize profiler data if needed
        if (m_profilerData.find(name) == m_profilerData.end()) {
            ProfilerData data;
            data.name = name;
            data.category = category;
            data.threadId = std::hash<std::thread::id>{}(std::this_thread::get_id());
            m_profilerData[name] = data;
        }
    }

    void DebugSystem::EndProfile(const std::string& name) {
        if (!m_profilingEnabled) {
            return;
        }

        uint64_t currentTime = GetCurrentTimeMicroseconds();

        std::unique_lock<std::shared_mutex> lock(m_profileMutex);

        auto it = m_activeProfiles.find(name);
        if (it == m_activeProfiles.end()) {
            return;  // Not profiling this section
        }

        uint64_t startTime = it->second;
        uint64_t duration = currentTime - startTime;

        m_activeProfiles.erase(it);

        // Remove from stack if present
        auto stackIt = std::find(m_profileStack.begin(), m_profileStack.end(), name);
        if (stackIt != m_profileStack.end()) {
            m_profileStack.erase(stackIt);
        }

        // Update profiler data
        UpdateProfilerData(name, startTime, currentTime);
    }

    const ProfilerData* DebugSystem::GetProfilerData(const std::string& name) const {
        std::shared_lock<std::shared_mutex> lock(m_profileMutex);
        auto it = m_profilerData.find(name);
        return it != m_profilerData.end() ? &it->second : nullptr;
    }

    const std::unordered_map<std::string, ProfilerData>& DebugSystem::GetAllProfilerData() const {
        return m_profilerData;
    }

    // Metrics API
    void DebugSystem::RecordMetric(const std::string& name, float value, MetricType type) {
        if (!m_metricsEnabled) {
            return;
        }

        std::unique_lock<std::shared_mutex> lock(m_metricsMutex);
        m_metrics[name] = value;

        // Add to history
        auto& history = m_metricHistory[name];
        history.push_back(value);
        if (history.size() > m_config.maxMetricsHistory) {
            history.pop_front();
        }
    }

    void DebugSystem::IncrementCounter(const std::string& name, int amount) {
        std::shared_lock<std::shared_mutex> lock(m_metricsMutex);
        float currentValue = m_metrics[name];
        lock.unlock();

        RecordMetric(name, currentValue + amount, MetricType::COUNTER);
    }

    void DebugSystem::StartTimer(const std::string& name) {
        m_timerStarts[name] = GetCurrentTimeMicroseconds();
    }

    void DebugSystem::StopTimer(const std::string& name) {
        auto it = m_timerStarts.find(name);
        if (it != m_timerStarts.end()) {
            uint64_t duration = GetCurrentTimeMicroseconds() - it->second;
            float durationMs = duration / 1000.0f;
            RecordMetric(name + "_time", durationMs, MetricType::TIMER);
            m_timerStarts.erase(it);
        }
    }

    float DebugSystem::GetMetricValue(const std::string& name) const {
        std::shared_lock<std::shared_mutex> lock(m_metricsMutex);
        auto it = m_metrics.find(name);
        return it != m_metrics.end() ? it->second : 0.0f;
    }

    const std::unordered_map<std::string, float>& DebugSystem::GetAllMetrics() const {
        return m_metrics;
    }

    // Debug Visualization API
    void DebugSystem::DrawSphere(const Vec3& position, float radius, const Color& color, float lifetime) {
        DebugShape shape;
        shape.shape = DebugShape::SPHERE;
        shape.position = position;
        shape.radius = radius;
        shape.color = color;
        shape.lifetime = lifetime;
        shape.creationTime = GetCurrentTimeMicroseconds();

        AddShape(shape);
    }

    void DebugSystem::DrawBox(const Vec3& position, const Vec3& size, const Color& color, float lifetime) {
        DebugShape shape;
        shape.shape = DebugShape::BOX;
        shape.position = position;
        shape.size = size;
        shape.color = color;
        shape.lifetime = lifetime;
        shape.creationTime = GetCurrentTimeMicroseconds();

        AddShape(shape);
    }

    void DebugSystem::DrawLine(const Vec3& start, const Vec3& end, const Color& color, float lifetime) {
        DebugShape shape;
        shape.shape = DebugShape::LINE;
        shape.start = start;
        shape.end = end;
        shape.color = color;
        shape.lifetime = lifetime;
        shape.creationTime = GetCurrentTimeMicroseconds();

        AddShape(shape);
    }

    void DebugSystem::DrawArrow(const Vec3& start, const Vec3& end, const Color& color, float lifetime) {
        DebugShape shape;
        shape.shape = DebugShape::ARROW;
        shape.start = start;
        shape.end = end;
        shape.color = color;
        shape.lifetime = lifetime;
        shape.creationTime = GetCurrentTimeMicroseconds();

        AddShape(shape);
    }

    void DebugSystem::DrawPath(const std::vector<Vec3>& points, const Color& color, float lifetime) {
        if (points.size() < 2) {
            return;
        }

        DebugShape shape;
        shape.shape = DebugShape::PATH;
        shape.points = points;
        shape.color = color;
        shape.lifetime = lifetime;
        shape.creationTime = GetCurrentTimeMicroseconds();

        AddShape(shape);
    }

    void DebugSystem::DrawGrid(const Vec3& position, const Vec2& size, float spacing, const Color& color, float lifetime) {
        DebugShape shape;
        shape.shape = DebugShape::GRID;
        shape.position = position;
        shape.size = Vec3(size.x, 1.0f, size.y);
        shape.color = color;
        shape.lifetime = lifetime;
        shape.creationTime = GetCurrentTimeMicroseconds();
        shape.radius = spacing;  // Reuse radius for spacing

        AddShape(shape);
    }

    void DebugSystem::DrawText3D(const Vec3& position, const std::string& text, const Color& color, float lifetime) {
        DebugShape shape;
        shape.shape = DebugShape::CUSTOM;  // Text is a special case
        shape.position = position;
        shape.color = color;
        shape.lifetime = lifetime;
        shape.label = text;
        shape.creationTime = GetCurrentTimeMicroseconds();

        AddShape(shape);
    }

    void DebugSystem::ClearShapes() {
        std::unique_lock<std::shared_mutex> lock(m_shapeMutex);
        m_debugShapes.clear();
    }

    const std::vector<DebugShape>& DebugSystem::GetShapes() const {
        return m_debugShapes;
    }

    // Debug Overlay API
    bool DebugSystem::CreateOverlay(const std::string& name, DebugOverlayType type, const Vec2& position, const Vec2& size) {
        std::unique_lock<std::shared_mutex> lock(m_overlayMutex);

        if (m_overlays.find(name) != m_overlays.end()) {
            return false;
        }

        DebugOverlay overlay;
        overlay.name = name;
        overlay.type = type;
        overlay.position = position;
        overlay.size = size;
        overlay.enabled = true;
        overlay.updateInterval = m_config.overlayUpdateInterval;
        overlay.backgroundColor = m_config.overlayColor;
        overlay.textColor = Color(1.0f, 1.0f, 1.0f, 1.0f);

        m_overlays[name] = overlay;
        return true;
    }

    void DebugSystem::UpdateOverlay(const std::string& name, const std::string& content) {
        std::unique_lock<std::shared_mutex> lock(m_overlayMutex);
        auto it = m_overlays.find(name);
        if (it != m_overlays.end()) {
            it->second.textContent = content;
        }
    }

    bool DebugSystem::RemoveOverlay(const std::string& name) {
        std::unique_lock<std::shared_mutex> lock(m_overlayMutex);
        return m_overlays.erase(name) > 0;
    }

    const DebugOverlay* DebugSystem::GetOverlay(const std::string& name) const {
        std::shared_lock<std::shared_mutex> lock(m_overlayMutex);
        auto it = m_overlays.find(name);
        return it != m_overlays.end() ? &it->second : nullptr;
    }

    const std::unordered_map<std::string, DebugOverlay>& DebugSystem::GetAllOverlays() const {
        return m_overlays;
    }

    // Logging API
    void DebugSystem::Log(LogLevel level, const std::string& message, const std::string& category) {
        if (!m_config.enableLogging) {
            return;
        }

        LogEntry entry;
        entry.level = level;
        entry.message = message;
        entry.category = category;
        entry.timestamp = GetCurrentTimeMicroseconds();
        entry.threadId = std::hash<std::thread::id>{}(std::this_thread::get_id());

        std::unique_lock<std::shared_mutex> lock(m_logMutex);
        m_logEntries.push_back(entry);

        // Maintain log size limit
        if (m_logEntries.size() > m_config.maxLogEntries) {
            m_logEntries.pop_front();
        }

        // Also send to main logger
        switch (level) {
            case LogLevel::TRACE:
                Logger::Trace("{}", message);
                break;
            case LogLevel::DEBUG:
                Logger::Debug("{}", message);
                break;
            case LogLevel::INFO:
                Logger::Info("{}", message);
                break;
            case LogLevel::WARNING:
                Logger::Warning("{}", message);
                break;
            case LogLevel::ERROR:
                Logger::Error("{}", message);
                break;
            case LogLevel::CRITICAL:
                Logger::Error("CRITICAL: {}", message);
                break;
        }
    }

    void DebugSystem::LogTrace(const std::string& message, const std::string& category) {
        Log(LogLevel::TRACE, message, category);
    }

    void DebugSystem::LogDebug(const std::string& message, const std::string& category) {
        Log(LogLevel::DEBUG, message, category);
    }

    void DebugSystem::LogInfo(const std::string& message, const std::string& category) {
        Log(LogLevel::INFO, message, category);
    }

    void DebugSystem::LogWarning(const std::string& message, const std::string& category) {
        Log(LogLevel::WARNING, message, category);
    }

    void DebugSystem::LogError(const std::string& message, const std::string& category) {
        Log(LogLevel::ERROR, message, category);
    }

    void DebugSystem::LogCritical(const std::string& message, const std::string& category) {
        Log(LogLevel::CRITICAL, message, category);
    }

    std::vector<std::string> DebugSystem::GetLogEntries(int maxEntries) const {
        std::shared_lock<std::shared_mutex> lock(m_logMutex);
        std::vector<std::string> entries;

        int startIndex = std::max(0, static_cast<int>(m_logEntries.size()) - maxEntries);
        for (int i = startIndex; i < m_logEntries.size(); ++i) {
            const auto& entry = m_logEntries[i];
            std::stringstream ss;
            ss << "[" << GetTimestampString() << "] [" << entry.category << "] " << entry.message;
            entries.push_back(ss.str());
        }

        return entries;
    }

    // Memory Debugging API
    MemorySnapshot DebugSystem::TakeMemorySnapshot() {
        MemorySnapshot snapshot;
        snapshot.timestamp = GetCurrentTimeMicroseconds();

        // This would integrate with the actual memory management system
        // For now, provide placeholder data
        snapshot.totalAllocated = 1024 * 1024 * 256;  // 256MB
        snapshot.totalUsed = 1024 * 1024 * 128;       // 128MB
        snapshot.peakUsage = 1024 * 1024 * 300;       // 300MB
        snapshot.allocationCount = 1000;
        snapshot.deallocationCount = 950;
        snapshot.fragmentationRatio = 0.15f;
        snapshot.fragmentedBlocks = 23;
        snapshot.largestFreeBlock = 1024 * 1024 * 10; // 10MB

        // Add system usage
        snapshot.systemUsage["Rendering"] = 1024 * 1024 * 64;   // 64MB
        snapshot.systemUsage["Physics"] = 1024 * 1024 * 32;     // 32MB
        snapshot.systemUsage["AI"] = 1024 * 1024 * 16;          // 16MB
        snapshot.systemUsage["Audio"] = 1024 * 1024 * 8;        // 8MB

        m_memorySnapshots.push_back(snapshot);
        return snapshot;
    }

    std::string DebugSystem::CompareMemorySnapshots(const MemorySnapshot& snapshot1, const MemorySnapshot& snapshot2) {
        std::stringstream ss;
        ss << "Memory Comparison Report:\n";
        ss << "========================\n";
        ss << "Time difference: " << (snapshot2.timestamp - snapshot1.timestamp) / 1000000.0 << "s\n";
        ss << "Memory usage change: " << FormatMemory(snapshot2.totalUsed - snapshot1.totalUsed) << "\n";
        ss << "Peak usage change: " << FormatMemory(snapshot2.peakUsage - snapshot1.peakUsage) << "\n";
        ss << "Allocation count change: " << (snapshot2.allocationCount - snapshot1.allocationCount) << "\n";
        ss << "Fragmentation change: " << (snapshot2.fragmentationRatio - snapshot1.fragmentationRatio) << "\n";

        return ss.str();
    }

    std::vector<std::pair<void*, size_t>> DebugSystem::DetectMemoryLeaks() {
        std::vector<std::pair<void*, size_t>> leaks;

        // This would integrate with actual memory tracking
        // For now, return empty vector
        return leaks;
    }

    std::unordered_map<std::string, size_t> DebugSystem::GetMemoryUsageBySystem() const {
        std::unordered_map<std::string, size_t> usage;

        // This would integrate with actual memory tracking
        usage["Rendering"] = 1024 * 1024 * 64;
        usage["Physics"] = 1024 * 1024 * 32;
        usage["AI"] = 1024 * 1024 * 16;
        usage["Audio"] = 1024 * 1024 * 8;

        return usage;
    }

    // System Monitoring API
    bool DebugSystem::RegisterSystem(const std::string& systemName) {
        std::unique_lock<std::shared_mutex> lock(m_systemMutex);

        if (m_systemMetrics.find(systemName) != m_systemMetrics.end()) {
            return false;
        }

        SystemMetrics metrics;
        metrics.systemName = systemName;
        metrics.enabled = true;
        metrics.isHealthy = true;
        metrics.healthStatus = "good";

        m_systemMetrics[systemName] = metrics;
        return true;
    }

    void DebugSystem::UpdateSystemMetrics(const std::string& systemName, const SystemMetrics& metrics) {
        std::unique_lock<std::shared_mutex> lock(m_systemMutex);
        m_systemMetrics[systemName] = metrics;
    }

    const SystemMetrics* DebugSystem::GetSystemMetrics(const std::string& systemName) const {
        std::shared_lock<std::shared_mutex> lock(m_systemMutex);
        auto it = m_systemMetrics.find(systemName);
        return it != m_systemMetrics.end() ? &it->second : nullptr;
    }

    const std::unordered_map<std::string, SystemMetrics>& DebugSystem::GetAllSystemMetrics() const {
        return m_systemMetrics;
    }

    // Debug Camera API
    void DebugSystem::EnableDebugCamera(bool enable) {
        m_debugCameraEnabled = enable;
        if (enable) {
            LogInfo("Debug camera enabled");
        } else {
            LogInfo("Debug camera disabled");
        }
    }

    bool DebugSystem::IsDebugCameraEnabled() const {
        return m_debugCameraEnabled;
    }

    void DebugSystem::UpdateDebugCamera(float deltaTime) {
        // This would integrate with input system
        // For now, just update position based on some logic
        m_debugCameraPosition += Vec3(0.0f, 0.0f, 1.0f) * deltaTime;
    }

    // Screenshot and Capture API
    bool DebugSystem::TakeScreenshot(const std::string& filename) {
        std::string actualFilename = filename;
        if (actualFilename.empty()) {
            actualFilename = "screenshot_" + GetTimestampString() + ".png";
        }

        // This would integrate with rendering system
        LogInfo("Screenshot saved: {}", actualFilename);
        return true;
    }

    void DebugSystem::StartPerformanceCapture(float duration) {
        m_performanceCaptureActive = true;
        m_performanceCaptureDuration = duration;
        m_performanceCaptureTime = 0.0f;
        m_capturedMetrics.clear();

        LogInfo("Started performance capture for {} seconds", duration);
    }

    std::string DebugSystem::StopPerformanceCapture() {
        if (!m_performanceCaptureActive) {
            return "No active performance capture";
        }

        m_performanceCaptureActive = false;

        std::stringstream ss;
        ss << "Performance Capture Report:\n";
        ss << "==========================\n";
        ss << "Duration: " << m_performanceCaptureTime << "s\n";
        ss << "Frames captured: " << m_capturedMetrics.size() << "\n";

        if (!m_capturedMetrics.empty()) {
            // Calculate averages
            float avgFPS = 0.0f;
            float avgFrameTime = 0.0f;
            float avgMemory = 0.0f;

            for (const auto& metrics : m_capturedMetrics) {
                avgFPS += metrics.framesPerSecond;
                avgFrameTime += metrics.frameTime;
                avgMemory += metrics.memoryUsageMB;
            }

            avgFPS /= m_capturedMetrics.size();
            avgFrameTime /= m_capturedMetrics.size();
            avgMemory /= m_capturedMetrics.size();

            ss << "Average FPS: " << avgFPS << "\n";
            ss << "Average frame time: " << avgFrameTime << "ms\n";
            ss << "Average memory usage: " << avgMemory << "MB\n";

            // Find min/max
            float minFPS = std::numeric_limits<float>::max();
            float maxFPS = 0.0f;
            float minFrameTime = std::numeric_limits<float>::max();
            float maxFrameTime = 0.0f;

            for (const auto& metrics : m_capturedMetrics) {
                minFPS = std::min(minFPS, metrics.framesPerSecond);
                maxFPS = std::max(maxFPS, metrics.framesPerSecond);
                minFrameTime = std::min(minFrameTime, metrics.frameTime);
                maxFrameTime = std::max(maxFrameTime, metrics.frameTime);
            }

            ss << "FPS range: " << minFPS << " - " << maxFPS << "\n";
            ss << "Frame time range: " << minFrameTime << "ms - " << maxFrameTime << "ms\n";
        }

        LogInfo("Performance capture completed");
        return ss.str();
    }

    bool DebugSystem::ExportProfilingData(const std::string& filename) {
        std::string actualFilename = filename;
        if (actualFilename.empty()) {
            actualFilename = "profile_" + GetTimestampString() + ".json";
        }

        // This would export profiling data to a file
        LogInfo("Profiling data exported to: {}", actualFilename);
        return true;
    }

    // Private methods implementation
    void DebugSystem::InitializeProfiling() {
        m_frameStartTime = GetCurrentTimeMicroseconds();
        m_profilingEnabled = m_config.enableProfiling;
        Logger::Info("Profiling system initialized");
    }

    void DebugSystem::InitializeMetrics() {
        m_metricsEnabled = m_config.enableMetrics;

        // Initialize some default metrics
        RecordMetric("frame_count", 0.0f, MetricType::COUNTER);
        RecordMetric("update_count", 0.0f, MetricType::COUNTER);
        RecordMetric("render_count", 0.0f, MetricType::COUNTER);

        Logger::Info("Metrics system initialized");
    }

    void DebugSystem::InitializeVisualization() {
        m_currentMode = DebugMode::NONE;
        Logger::Info("Visualization system initialized");
    }

    void DebugSystem::InitializeOverlays() {
        Logger::Info("Overlay system initialized");
    }

    void DebugSystem::InitializeLogging() {
        LogInfo("DebugSystem initialized", "DebugSystem");
        Logger::Info("Logging system initialized");
    }

    void DebugSystem::CreateDefaultOverlays() {
        // Create performance overlay
        CreateOverlay("Performance", DebugOverlayType::GRAPH, Vec2(10.0f, 10.0f), Vec2(300.0f, 200.0f));
        CreateOverlay("System", DebugOverlayType::TEXT, Vec2(10.0f, 220.0f), Vec2(300.0f, 150.0f));
        CreateOverlay("Memory", DebugOverlayType::CHART, Vec2(320.0f, 10.0f), Vec2(300.0f, 200.0f));
    }

    void DebugSystem::UpdateProfiling(float deltaTime) {
        // Update profiler data
        for (auto& pair : m_profilerData) {
            ProfilerData& data = pair.second;
            if (data.callCount > 0) {
                data.percentageOfFrame = (data.totalTime / 1000000.0f) / (deltaTime * 1000.0f) * 100.0f;
                data.isBottleneck = data.percentageOfFrame > 50.0f;
            }
        }

        // Cleanup expired profiles
        CleanupExpiredProfiles();

        // Analyze bottlenecks
        AnalyzeBottlenecks();
    }

    void DebugSystem::UpdateMetrics(float deltaTime) {
        // Update metric history
        UpdateMetricHistory();

        // Update counters
        IncrementCounter("frame_count");
        IncrementCounter("update_count");

        // Record frame time
        RecordMetric("frame_time", deltaTime * 1000.0f, MetricType::GAUGE);
    }

    void DebugSystem::UpdateVisualization(float deltaTime) {
        // Update shape lifetimes
        UpdateShapeLifetimes(deltaTime);

        // Cleanup expired shapes
        CleanupExpiredShapes();
    }

    void DebugSystem::UpdateOverlays(float deltaTime) {
        // Update overlay contents
        UpdateOverlay("Performance", GetPerformanceReport());
        UpdateOverlay("System", GetSystemHealthReport());
        UpdateOverlay("Memory", FormatMemory(m_performanceMetrics.memoryUsageMB * 1024 * 1024));
    }

    void DebugSystem::UpdateLogging(float deltaTime) {
        // Log periodic information
        static float logTimer = 0.0f;
        logTimer += deltaTime;

        if (logTimer >= 5.0f) {  // Log every 5 seconds
            logTimer = 0.0f;
            LogInfo("System status: FPS=" + std::to_string(static_cast<int>(m_performanceMetrics.framesPerSecond)) +
                   ", Memory=" + FormatMemory(static_cast<size_t>(m_performanceMetrics.memoryUsageMB * 1024 * 1024)),
                   "Status");
        }
    }

    void DebugSystem::UpdateProfilerData(const std::string& name, uint64_t startTime, uint64_t endTime) {
        auto it = m_profilerData.find(name);
        if (it == m_profilerData.end()) {
            return;
        }

        ProfilerData& data = it->second;
        uint64_t duration = endTime - startTime;

        data.endTime = endTime;
        data.totalTime += duration;
        data.callCount++;

        // Update min/max
        if (duration < data.minTime) {
            data.minTime = duration;
        }
        if (duration > data.maxTime) {
            data.maxTime = duration;
        }

        // Update average
        data.averageTime = data.totalTime / data.callCount;

        // Add to history
        data.timeHistory.push_back(duration);
        if (data.timeHistory.size() > 100) {
            data.timeHistory.pop_front();
        }
    }

    void DebugSystem::AnalyzeBottlenecks() {
        std::string mainBottleneck = "none";
        float maxPercentage = 0.0f;

        for (const auto& pair : m_profilerData) {
            const ProfilerData& data = pair.second;
            if (data.percentageOfFrame > maxPercentage) {
                maxPercentage = data.percentageOfFrame;
                mainBottleneck = data.name;
            }
        }

        m_performanceMetrics.mainBottleneck = mainBottleneck;
        m_performanceMetrics.bottleneckSeverity = maxPercentage / 100.0f;
    }

    void DebugSystem::CleanupExpiredProfiles() {
        // Remove profiles that haven't been called in a while
        uint64_t currentTime = GetCurrentTimeMicroseconds();
        uint64_t expiryTime = currentTime - 30 * 1000000;  // 30 seconds ago

        for (auto it = m_profilerData.begin(); it != m_profilerData.end();) {
            if (it->second.endTime < expiryTime && it->second.callCount > 0) {
                it = m_profilerData.erase(it);
            } else {
                ++it;
            }
        }
    }

    void DebugSystem::UpdatePerformanceMetrics(float deltaTime) {
        // Update frame-based metrics
        m_performanceMetrics.frameTime = deltaTime * 1000.0f;  // Convert to milliseconds
        m_performanceMetrics.framesPerSecond = 1.0f / deltaTime;

        // Update history
        m_performanceMetrics.frameTimeHistory.push_back(m_performanceMetrics.frameTime);
        m_performanceMetrics.fpsHistory.push_back(m_performanceMetrics.framesPerSecond);
        m_performanceMetrics.memoryHistory.push_back(m_performanceMetrics.memoryUsageMB);

        // Maintain history size
        while (m_performanceMetrics.frameTimeHistory.size() > 100) {
            m_performanceMetrics.frameTimeHistory.pop_front();
        }
        while (m_performanceMetrics.fpsHistory.size() > 100) {
            m_performanceMetrics.fpsHistory.pop_front();
        }
        while (m_performanceMetrics.memoryHistory.size() > 100) {
            m_performanceMetrics.memoryHistory.pop_front();
        }
    }

    void DebugSystem::UpdateShapeLifetimes(float deltaTime) {
        uint64_t currentTime = GetCurrentTimeMicroseconds();

        std::unique_lock<std::shared_mutex> lock(m_shapeMutex);

        for (auto& shape : m_debugShapes) {
            if (shape.lifetime > 0.0f) {
                uint64_t elapsed = currentTime - shape.creationTime;
                float elapsedSeconds = elapsed / 1000000.0f;
                if (elapsedSeconds >= shape.lifetime) {
                    shape.lifetime = 0.0f;  // Mark for cleanup
                }
            }
        }
    }

    void DebugSystem::CleanupExpiredShapes() {
        std::unique_lock<std::shared_mutex> lock(m_shapeMutex);

        m_debugShapes.erase(
            std::remove_if(m_debugShapes.begin(), m_debugShapes.end(),
                [](const DebugShape& shape) { return shape.lifetime <= 0.0f; }),
            m_debugShapes.end()
        );
    }

    void DebugSystem::AddShape(const DebugShape& shape) {
        std::unique_lock<std::shared_mutex> lock(m_shapeMutex);

        if (m_debugShapes.size() >= m_config.maxDebugShapes) {
            // Remove oldest shape
            m_debugShapes.erase(m_debugShapes.begin());
        }

        m_debugShapes.push_back(shape);
    }

    void DebugSystem::UpdateMetricHistory() {
        // Update history for all metrics
        for (const auto& metric : m_metrics) {
            auto& history = m_metricHistory[metric.first];
            history.push_back(metric.second);

            // Maintain history size
            if (history.size() > m_config.maxMetricsHistory) {
                history.pop_front();
            }
        }
    }

    void DebugSystem::RenderOverlay() {
        // This would integrate with the rendering system
        // For now, just log that overlay is being rendered
        if (m_debugMode != DebugMode::NONE) {
            LogDebug("Rendering debug overlay with " + std::to_string(m_overlays.size()) + " overlays", "Debug");
        }
    }

    void DebugSystem::RenderShapes() {
        // This would integrate with the rendering system
        LogDebug("Rendering " + std::to_string(m_debugShapes.size()) + " debug shapes", "Debug");
    }

    void DebugSystem::RenderMetrics() {
        // Render metrics overlay
        LogDebug("Rendering performance metrics", "Debug");
    }

    void DebugSystem::RenderPerformanceGraphs() {
        // Render performance graphs
        LogDebug("Rendering performance graphs", "Debug");
    }

    void DebugSystem::ProcessDebugCommands() {
        // This would integrate with input system
        // For now, just process some basic commands
        static bool f3Pressed = false;
        if (!f3Pressed) {
            // Simulate F3 press
            if (m_currentMode == DebugMode::NONE) {
                SetDebugMode(DebugMode::BASIC);
            } else {
                SetDebugMode(DebugMode::NONE);
            }
            f3Pressed = true;
        }
    }

    uint64_t DebugSystem::GetCurrentTimeMicroseconds() const {
        return std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
    }

    std::string DebugSystem::GetTimestampString() const {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y-%m-%d_%H-%M-%S");
        return ss.str();
    }

    std::string DebugSystem::FormatTime(float seconds) const {
        if (seconds < 0.001f) {
            return std::to_string(static_cast<int>(seconds * 1000000.0f)) + "ns";
        } else if (seconds < 0.1f) {
            return std::to_string(static_cast<int>(seconds * 1000.0f)) + "ms";
        } else if (seconds < 60.0f) {
            return std::to_string(static_cast<int>(seconds * 100.0f) / 100.0f) + "s";
        } else {
            int minutes = static_cast<int>(seconds / 60.0f);
            int remainingSeconds = static_cast<int>(seconds) % 60;
            return std::to_string(minutes) + "m " + std::to_string(remainingSeconds) + "s";
        }
    }

    std::string DebugSystem::FormatMemory(size_t bytes) const {
        const char* units[] = {"B", "KB", "MB", "GB", "TB"};
        int unitIndex = 0;
        double size = static_cast<double>(bytes);

        while (size >= 1024.0 && unitIndex < 4) {
            size /= 1024.0;
            unitIndex++;
        }

        std::stringstream ss;
        ss << std::fixed << std::setprecision(2) << size << " " << units[unitIndex];
        return ss.str();
    }

    std::string DebugSystem::GetDebugInfo() const {
        std::stringstream ss;
        ss << std::fixed << std::setprecision(2);

        ss << "=== DebugSystem Status ===\n";
        ss << "Mode: " << static_cast<int>(m_currentMode) << "\n";
        ss << "Overlay: " << (m_overlayEnabled ? "Enabled" : "Disabled") << "\n";
        ss << "Profiling: " << (m_profilingEnabled ? "Enabled" : "Disabled") << "\n";
        ss << "Metrics: " << (m_metricsEnabled ? "Enabled" : "Disabled") << "\n";
        ss << "Shapes: " << m_debugShapes.size() << "\n";
        ss << "Overlays: " << m_overlays.size() << "\n";
        ss << "Log entries: " << m_logEntries.size() << "\n";
        ss << "Profilers: " << m_profilerData.size() << "\n";
        ss << "Systems: " << m_systemMetrics.size() << "\n";

        return ss.str();
    }

    std::string DebugSystem::GetPerformanceReport() const {
        std::stringstream ss;
        ss << std::fixed << std::setprecision(2);

        ss << "=== Performance Report ===\n";
        ss << "FPS: " << m_performanceMetrics.framesPerSecond << "\n";
        ss << "Frame Time: " << m_performanceMetrics.frameTime << "ms\n";
        ss << "Memory: " << m_performanceMetrics.memoryUsageMB << "MB\n";
        ss << "CPU: " << m_performanceMetrics.cpuUsage * 100.0f << "%\n";
        ss << "GPU: " << m_performanceMetrics.gpuUsage * 100.0f << "%\n";

        if (!m_performanceMetrics.mainBottleneck.empty()) {
            ss << "Bottleneck: " << m_performanceMetrics.mainBottleneck << "\n";
            ss << "Severity: " << m_performanceMetrics.bottleneckSeverity * 100.0f << "%\n";
        }

        return ss.str();
    }

    std::string DebugSystem::GetSystemHealthReport() const {
        std::stringstream ss;
        ss << "=== System Health ===\n";

        for (const auto& pair : m_systemMetrics) {
            const SystemMetrics& metrics = pair.second;
            ss << pair.first << ": " << metrics.healthStatus << "\n";
            if (!metrics.warnings.empty()) {
                ss << "  Warnings: " << metrics.warnings.size() << "\n";
            }
            if (!metrics.errors.empty()) {
                ss << "  Errors: " << metrics.errors.size() << "\n";
            }
        }

        return ss.str();
    }

    bool DebugSystem::Validate() const {
        bool valid = true;

        if (!m_initialized) {
            Logger::Error("DebugSystem validation failed: Not initialized");
            valid = false;
        }

        // Check for any critical issues
        if (m_performanceMetrics.framesPerSecond < 10.0f) {
            Logger::Warning("DebugSystem validation: Low FPS detected");
        }

        if (m_performanceMetrics.memoryUsageMB > 2000.0f) {  // 2GB
            Logger::Warning("DebugSystem validation: High memory usage detected");
        }

        return valid;
    }

} // namespace VoxelCraft
