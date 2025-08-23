/**
 * @file DebugSystem.hpp
 * @brief VoxelCraft Advanced Debug and Profiling System
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_DEBUG_DEBUG_SYSTEM_HPP
#define VOXELCRAFT_DEBUG_DEBUG_SYSTEM_HPP

#include <memory>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <deque>
#include <atomic>
#include <mutex>
#include <shared_mutex>
#include <functional>
#include <chrono>
#include <string>
#include <sstream>
#include <iomanip>

#include "../math/Vec3.hpp"
#include "../math/Vec2.hpp"
#include "../math/Color.hpp"
#include "../math/Mat4.hpp"

namespace VoxelCraft {

    // Forward declarations
    class Player;
    class Entity;
    class World;
    class Camera;
    struct DebugConfig;
    struct PerformanceMetrics;
    struct DebugOverlay;
    struct ProfilerData;
    struct MemorySnapshot;
    struct SystemMetrics;
    struct DebugCommand;

    /**
     * @enum DebugMode
     * @brief Different debug visualization modes
     */
    enum class DebugMode {
        NONE,                    ///< No debug visualization
        BASIC,                   ///< Basic debug info
        PERFORMANCE,             ///< Performance metrics
        RENDERING,               ///< Rendering debug
        PHYSICS,                 ///< Physics debug
        AI,                      ///< AI debug
        NETWORKING,              ///< Network debug
        MEMORY,                  ///< Memory debug
        AUDIO,                   ///< Audio debug
        INPUT,                   ///< Input debug
        CUSTOM                   ///< Custom debug mode
    };

    /**
     * @enum DebugOverlayType
     * @brief Types of debug overlays
     */
    enum class DebugOverlayType {
        TEXT,                    ///< Text overlay
        GRAPH,                   ///< Graph overlay
        HISTOGRAM,              ///< Histogram overlay
        CHART,                   ///< Chart overlay
        GAUGE,                   ///< Gauge overlay
        RADAR,                   ///< Radar overlay
        HEATMAP,                 ///< Heatmap overlay
        CUSTOM                   ///< Custom overlay
    };

    /**
     * @enum DebugShape
     * @brief Debug visualization shapes
     */
    enum class DebugShape {
        SPHERE,                  ///< Debug sphere
        BOX,                     ///< Debug box
        LINE,                    ///< Debug line
        ARROW,                   ///< Debug arrow
        PLANE,                   ///< Debug plane
        FRUSTUM,                 ///< Debug frustum
        SKELETON,                ///< Debug skeleton
        PATH,                    ///< Debug path
        GRID,                    ///< Debug grid
        CUSTOM                   ///< Custom shape
    };

    /**
     * @enum LogLevel
     * @brief Debug logging levels
     */
    enum class LogLevel {
        TRACE,                   ///< Most verbose
        DEBUG,                   ///< Debug information
        INFO,                    ///< General information
        WARNING,                 ///< Warnings
        ERROR,                   ///< Errors
        CRITICAL                 ///< Critical errors
    };

    /**
     * @enum MetricType
     * @brief Types of performance metrics
     */
    enum class MetricType {
        COUNTER,                 ///< Simple counter
        GAUGE,                   ///< Gauge metric
        HISTOGRAM,              ///< Histogram
        TIMER,                   ///< Timer metric
        METER,                   ///< Meter (rate)
        SET,                     ///< Set of unique values
        CUSTOM                   ///< Custom metric
    };

    /**
     * @struct DebugConfig
     * @brief Configuration for debug system
     */
    struct DebugConfig {
        bool enableDebug = true;
        bool enableProfiling = true;
        bool enableOverlay = true;
        bool enableLogging = true;
        bool enableMetrics = true;
        bool enableVisualization = true;
        bool enableConsole = false;

        // Display settings
        Vec2 overlayPosition = Vec2(10.0f, 10.0f);
        float overlayScale = 1.0f;
        Color overlayColor = Color(1.0f, 1.0f, 1.0f, 0.8f);
        float overlayUpdateInterval = 0.1f;  // seconds

        // Performance settings
        int maxMetricsHistory = 1000;
        float metricsUpdateInterval = 0.1f;
        int maxLogEntries = 10000;
        int maxDebugShapes = 1000;

        // Visualization settings
        float shapeLifetime = 5.0f;  // seconds
        float lineThickness = 2.0f;
        bool enableDepthTesting = true;
        bool enableWireframe = false;

        // Profiling settings
        bool enableCPUProfiling = true;
        bool enableGPUProfiling = false;
        bool enableMemoryProfiling = true;
        float profileSampleInterval = 0.016f;  // ~60fps

        // Debug keys
        int toggleDebugKey = 192;  // ~
        int toggleOverlayKey = 9;   // Tab
        int toggleConsoleKey = 192; // ~
        int screenshotKey = 107;    // +
    };

    /**
     * @struct PerformanceMetrics
     * @brief Real-time performance metrics
     */
    struct PerformanceMetrics {
        // Timing metrics
        float frameTime = 0.0f;
        float updateTime = 0.0f;
        float renderTime = 0.0f;
        float physicsTime = 0.0f;
        float aiTime = 0.0f;
        float networkTime = 0.0f;
        float audioTime = 0.0f;

        // Frame rate metrics
        float framesPerSecond = 0.0f;
        float updatesPerSecond = 0.0f;
        float trianglesPerSecond = 0.0f;
        float verticesPerSecond = 0.0f;
        float drawCallsPerSecond = 0.0f;

        // Memory metrics
        float memoryUsageMB = 0.0f;
        float memoryPeakMB = 0.0f;
        int memoryAllocations = 0;
        int memoryDeallocations = 0;

        // System metrics
        float cpuUsage = 0.0f;
        float gpuUsage = 0.0f;
        float gpuMemoryUsage = 0.0f;
        float diskIO = 0.0f;
        float networkIO = 0.0f;

        // Game metrics
        int activeEntities = 0;
        int visibleChunks = 0;
        int activeParticles = 0;
        int activeSounds = 0;
        int networkPackets = 0;

        // Quality metrics
        float lodQuality = 1.0f;
        float textureQuality = 1.0f;
        float shadowQuality = 1.0f;
        float effectQuality = 1.0f;

        // Performance indicators
        float bottleneckSeverity = 0.0f;
        std::string mainBottleneck = "none";
        int performanceScore = 100;  // 0-100

        // History data
        std::deque<float> frameTimeHistory;
        std::deque<float> fpsHistory;
        std::deque<float> memoryHistory;
        std::deque<float> cpuHistory;
        std::deque<float> gpuHistory;
    };

    /**
     * @struct DebugOverlay
     * @brief Debug overlay configuration
     */
    struct DebugOverlay {
        std::string name;
        DebugOverlayType type;
        Vec2 position;
        Vec2 size;
        bool enabled = true;
        bool autoLayout = true;
        float updateInterval = 0.1f;
        Color backgroundColor = Color(0.0f, 0.0f, 0.0f, 0.7f);
        Color textColor = Color(1.0f, 1.0f, 1.0f, 1.0f);

        // Type-specific data
        std::string textContent;
        std::vector<float> graphData;
        std::vector<std::pair<std::string, float>> chartData;
        float minValue = 0.0f;
        float maxValue = 100.0f;

        // Layout
        int column = 0;
        int row = 0;
        int spanX = 1;
        int spanY = 1;
    };

    /**
     * @struct ProfilerData
     * @brief Profiling data for a function or system
     */
    struct ProfilerData {
        std::string name;
        std::string category;
        uint64_t startTime = 0;
        uint64_t endTime = 0;
        uint64_t totalTime = 0;
        uint64_t callCount = 0;
        uint64_t maxTime = 0;
        uint64_t minTime = UINT64_MAX;
        uint64_t averageTime = 0;

        // Call stack information
        std::vector<std::string> callStack;
        int threadId = 0;
        bool isRecursive = false;

        // Memory usage
        size_t memoryAllocated = 0;
        size_t memoryPeak = 0;

        // Performance indicators
        float percentageOfFrame = 0.0f;
        bool isBottleneck = false;

        // History
        std::deque<uint64_t> timeHistory;
        std::deque<size_t> memoryHistory;
    };

    /**
     * @struct MemorySnapshot
     * @brief Snapshot of memory usage
     */
    struct MemorySnapshot {
        uint64_t timestamp = 0;
        size_t totalAllocated = 0;
        size_t totalUsed = 0;
        size_t peakUsage = 0;
        int allocationCount = 0;
        int deallocationCount = 0;

        // Per-system breakdown
        std::unordered_map<std::string, size_t> systemUsage;
        std::unordered_map<std::string, int> systemAllocations;

        // Memory leaks detection
        std::vector<std::pair<void*, size_t>> activeAllocations;
        std::vector<std::pair<void*, size_t>> potentialLeaks;

        // Fragmentation analysis
        float fragmentationRatio = 0.0f;
        int fragmentedBlocks = 0;
        size_t largestFreeBlock = 0;
    };

    /**
     * @struct SystemMetrics
     * @brief System-wide metrics
     */
    struct SystemMetrics {
        std::string systemName;
        bool enabled = true;
        PerformanceMetrics metrics;

        // System-specific data
        std::unordered_map<std::string, float> customMetrics;
        std::unordered_map<std::string, std::string> customStrings;

        // Health indicators
        bool isHealthy = true;
        std::string healthStatus = "good";
        std::vector<std::string> warnings;
        std::vector<std::string> errors;
    };

    /**
     * @struct DebugShape
     * @brief Debug visualization shape
     */
    struct DebugShape {
        DebugShape shape = DebugShape::SPHERE;
        Vec3 position = Vec3(0.0f);
        Vec3 rotation = Vec3(0.0f);
        Vec3 scale = Vec3(1.0f);
        Color color = Color(1.0f, 1.0f, 1.0f, 1.0f);
        float lifetime = 5.0f;
        float thickness = 2.0f;
        bool depthTest = true;
        bool wireframe = false;

        // Shape-specific data
        float radius = 1.0f;
        Vec3 size = Vec3(1.0f);
        Vec3 start = Vec3(0.0f);
        Vec3 end = Vec3(0.0f);
        std::vector<Vec3> points;
        std::vector<Vec3> vertices;

        // Animation
        bool animated = false;
        float animationSpeed = 1.0f;
        Vec3 animationAxis = Vec3(0.0f, 1.0f, 0.0f);

        // Metadata
        std::string label;
        void* userData = nullptr;
        uint64_t creationTime = 0;
    };

    /**
     * @class DebugSystem
     * @brief Comprehensive debug and profiling system
     *
     * Features:
     * - Real-time performance monitoring with 100+ metrics
     * - Advanced profiling with call stack analysis
     * - Debug visualization with shapes and overlays
     * - Memory leak detection and analysis
     * - System health monitoring
     * - Interactive debug console
     * - Performance bottleneck detection
     * - Custom metrics and telemetry
     * - Debug camera and navigation
     * - Screenshot and video capture
     * - Remote debugging support
     * - Performance regression detection
     * - Multi-threading debugging
     * - GPU debugging and profiling
     * - Network debugging tools
     * - AI behavior debugging
     * - Physics debugging visualization
     */
    class DebugSystem {
    public:
        static DebugSystem& GetInstance();

        /**
         * @brief Initialize the debug system
         * @param config Debug configuration
         * @return true if successful
         */
        bool Initialize(const DebugConfig& config);

        /**
         * @brief Shutdown the debug system
         */
        void Shutdown();

        /**
         * @brief Update debug system (call every frame)
         * @param deltaTime Time since last update
         */
        void Update(float deltaTime);

        /**
         * @brief Render debug information
         */
        void Render();

        // Profiling API
        /**
         * @brief Begin profiling section
         * @param name Section name
         * @param category Section category
         */
        void BeginProfile(const std::string& name, const std::string& category = "");

        /**
         * @brief End profiling section
         * @param name Section name
         */
        void EndProfile(const std::string& name);

        /**
         * @brief Profile a function call
         * @param name Function name
         * @param func Function to profile
         */
        template<typename Func>
        void ProfileFunction(const std::string& name, Func&& func);

        /**
         * @brief Get profiler data
         * @param name Profiler name
         * @return Profiler data
         */
        const ProfilerData* GetProfilerData(const std::string& name) const;

        /**
         * @brief Get all profiler data
         * @return Map of profiler data
         */
        const std::unordered_map<std::string, ProfilerData>& GetAllProfilerData() const;

        // Metrics API
        /**
         * @brief Record metric value
         * @param name Metric name
         * @param value Metric value
         * @param type Metric type
         */
        void RecordMetric(const std::string& name, float value, MetricType type = MetricType::GAUGE);

        /**
         * @brief Increment counter metric
         * @param name Counter name
         * @param amount Amount to increment
         */
        void IncrementCounter(const std::string& name, int amount = 1);

        /**
         * @brief Start timer metric
         * @param name Timer name
         */
        void StartTimer(const std::string& name);

        /**
         * @brief Stop timer metric
         * @param name Timer name
         */
        void StopTimer(const std::string& name);

        /**
         * @brief Get metric value
         * @param name Metric name
         * @return Metric value
         */
        float GetMetricValue(const std::string& name) const;

        /**
         * @brief Get all metrics
         * @return Map of metrics
         */
        const std::unordered_map<std::string, float>& GetAllMetrics() const;

        // Debug Visualization API
        /**
         * @brief Draw debug sphere
         * @param position Sphere position
         * @param radius Sphere radius
         * @param color Sphere color
         * @param lifetime Lifetime in seconds
         */
        void DrawSphere(const Vec3& position, float radius, const Color& color = Color(1.0f, 1.0f, 1.0f, 1.0f), float lifetime = 5.0f);

        /**
         * @brief Draw debug box
         * @param position Box position
         * @param size Box size
         * @param color Box color
         * @param lifetime Lifetime in seconds
         */
        void DrawBox(const Vec3& position, const Vec3& size, const Color& color = Color(1.0f, 1.0f, 1.0f, 1.0f), float lifetime = 5.0f);

        /**
         * @brief Draw debug line
         * @param start Line start position
         * @param end Line end position
         * @param color Line color
         * @param lifetime Lifetime in seconds
         */
        void DrawLine(const Vec3& start, const Vec3& end, const Color& color = Color(1.0f, 1.0f, 1.0f, 1.0f), float lifetime = 5.0f);

        /**
         * @brief Draw debug arrow
         * @param start Arrow start position
         * @param end Arrow end position
         * @param color Arrow color
         * @param lifetime Lifetime in seconds
         */
        void DrawArrow(const Vec3& start, const Vec3& end, const Color& color = Color(1.0f, 1.0f, 1.0f, 1.0f), float lifetime = 5.0f);

        /**
         * @brief Draw debug path
         * @param points Path points
         * @param color Path color
         * @param lifetime Lifetime in seconds
         */
        void DrawPath(const std::vector<Vec3>& points, const Color& color = Color(1.0f, 1.0f, 1.0f, 1.0f), float lifetime = 5.0f);

        /**
         * @brief Draw debug grid
         * @param position Grid position
         * @param size Grid size
         * @param spacing Grid spacing
         * @param color Grid color
         * @param lifetime Lifetime in seconds
         */
        void DrawGrid(const Vec3& position, const Vec2& size, float spacing, const Color& color = Color(0.5f, 0.5f, 0.5f, 1.0f), float lifetime = 5.0f);

        /**
         * @brief Draw debug text in 3D space
         * @param position Text position
         * @param text Text content
         * @param color Text color
         * @param lifetime Lifetime in seconds
         */
        void DrawText3D(const Vec3& position, const std::string& text, const Color& color = Color(1.0f, 1.0f, 1.0f, 1.0f), float lifetime = 5.0f);

        /**
         * @brief Clear all debug shapes
         */
        void ClearShapes();

        /**
         * @brief Get debug shapes
         * @return Vector of debug shapes
         */
        const std::vector<DebugShape>& GetShapes() const;

        // Debug Overlay API
        /**
         * @brief Create debug overlay
         * @param name Overlay name
         * @param type Overlay type
         * @param position Overlay position
         * @param size Overlay size
         * @return true if successful
         */
        bool CreateOverlay(const std::string& name, DebugOverlayType type, const Vec2& position, const Vec2& size);

        /**
         * @brief Update overlay content
         * @param name Overlay name
         * @param content Content to update
         */
        void UpdateOverlay(const std::string& name, const std::string& content);

        /**
         * @brief Remove debug overlay
         * @param name Overlay name
         * @return true if successful
         */
        bool RemoveOverlay(const std::string& name);

        /**
         * @brief Get debug overlay
         * @param name Overlay name
         * @return Debug overlay
         */
        const DebugOverlay* GetOverlay(const std::string& name) const;

        /**
         * @brief Get all overlays
         * @return Map of overlays
         */
        const std::unordered_map<std::string, DebugOverlay>& GetAllOverlays() const;

        // Logging API
        /**
         * @brief Log debug message
         * @param level Log level
         * @param message Log message
         * @param category Log category
         */
        void Log(LogLevel level, const std::string& message, const std::string& category = "");

        /**
         * @brief Log trace message
         * @param message Log message
         * @param category Log category
         */
        void LogTrace(const std::string& message, const std::string& category = "");

        /**
         * @brief Log debug message
         * @param message Log message
         * @param category Log category
         */
        void LogDebug(const std::string& message, const std::string& category = "");

        /**
         * @brief Log info message
         * @param message Log message
         * @param category Log category
         */
        void LogInfo(const std::string& message, const std::string& category = "");

        /**
         * @brief Log warning message
         * @param message Log message
         * @param category Log category
         */
        void LogWarning(const std::string& message, const std::string& category = "");

        /**
         * @brief Log error message
         * @param message Log message
         * @param category Log category
         */
        void LogError(const std::string& message, const std::string& category = "");

        /**
         * @brief Log critical message
         * @param message Log message
         * @param category Log category
         */
        void LogCritical(const std::string& message, const std::string& category = "");

        /**
         * @brief Get log entries
         * @param maxEntries Maximum number of entries
         * @return Vector of log entries
         */
        std::vector<std::string> GetLogEntries(int maxEntries = 100) const;

        // Memory Debugging API
        /**
         * @brief Take memory snapshot
         * @return Memory snapshot
         */
        MemorySnapshot TakeMemorySnapshot();

        /**
         * @brief Compare memory snapshots
         * @param snapshot1 First snapshot
         * @param snapshot2 Second snapshot
         * @return Memory difference report
         */
        std::string CompareMemorySnapshots(const MemorySnapshot& snapshot1, const MemorySnapshot& snapshot2);

        /**
         * @brief Detect memory leaks
         * @return Vector of potential leaks
         */
        std::vector<std::pair<void*, size_t>> DetectMemoryLeaks();

        /**
         * @brief Get memory usage by system
         * @return Map of system memory usage
         */
        std::unordered_map<std::string, size_t> GetMemoryUsageBySystem() const;

        // System Monitoring API
        /**
         * @brief Register system for monitoring
         * @param systemName System name
         * @return true if successful
         */
        bool RegisterSystem(const std::string& systemName);

        /**
         * @brief Update system metrics
         * @param systemName System name
         * @param metrics System metrics
         */
        void UpdateSystemMetrics(const std::string& systemName, const SystemMetrics& metrics);

        /**
         * @brief Get system metrics
         * @param systemName System name
         * @return System metrics
         */
        const SystemMetrics* GetSystemMetrics(const std::string& systemName) const;

        /**
         * @brief Get all system metrics
         * @return Map of system metrics
         */
        const std::unordered_map<std::string, SystemMetrics>& GetAllSystemMetrics() const;

        // Debug Camera API
        /**
         * @brief Enable debug camera
         * @param enable Enable state
         */
        void EnableDebugCamera(bool enable);

        /**
         * @brief Is debug camera enabled
         * @return true if enabled
         */
        bool IsDebugCameraEnabled() const;

        /**
         * @brief Update debug camera
         * @param deltaTime Time since last update
         */
        void UpdateDebugCamera(float deltaTime);

        // Screenshot and Capture API
        /**
         * @brief Take screenshot
         * @param filename Screenshot filename
         * @return true if successful
         */
        bool TakeScreenshot(const std::string& filename = "");

        /**
         * @brief Start performance capture
         * @param duration Capture duration in seconds
         */
        void StartPerformanceCapture(float duration = 10.0f);

        /**
         * @brief Stop performance capture
         * @return Capture data
         */
        std::string StopPerformanceCapture();

        /**
         * @brief Export profiling data
         * @param filename Export filename
         * @return true if successful
         */
        bool ExportProfilingData(const std::string& filename);

        // Configuration
        /**
         * @brief Set debug configuration
         * @param config New configuration
         */
        void SetConfig(const DebugConfig& config) { m_config = config; }

        /**
         * @brief Get current configuration
         * @return Current configuration
         */
        const DebugConfig& GetConfig() const { return m_config; }

        /**
         * @brief Set debug mode
         * @param mode Debug mode
         */
        void SetDebugMode(DebugMode mode) { m_currentMode = mode; }

        /**
         * @brief Get current debug mode
         * @return Current debug mode
         */
        DebugMode GetDebugMode() const { return m_currentMode; }

        /**
         * @brief Toggle debug overlay
         */
        void ToggleOverlay() { m_overlayEnabled = !m_overlayEnabled; }

        /**
         * @brief Is overlay enabled
         * @return true if enabled
         */
        bool IsOverlayEnabled() const { return m_overlayEnabled; }

        // Debug Information
        /**
         * @brief Get debug information string
         * @return Debug info
         */
        std::string GetDebugInfo() const;

        /**
         * @brief Get performance report
         * @return Performance report
         */
        std::string GetPerformanceReport() const;

        /**
         * @brief Get system health report
         * @return System health report
         */
        std::string GetSystemHealthReport() const;

        /**
         * @brief Validate debug system
         * @return true if valid
         */
        bool Validate() const;

    private:
        DebugSystem() = default;
        ~DebugSystem();

        // Prevent copying
        DebugSystem(const DebugSystem&) = delete;
        DebugSystem& operator=(const DebugSystem&) = delete;

        // Core systems
        void InitializeProfiling();
        void InitializeMetrics();
        void InitializeVisualization();
        void InitializeOverlays();
        void InitializeLogging();

        // Update methods
        void UpdateProfiling(float deltaTime);
        void UpdateMetrics(float deltaTime);
        void UpdateVisualization(float deltaTime);
        void UpdateOverlays(float deltaTime);
        void UpdateLogging(float deltaTime);

        // Rendering methods
        void RenderOverlay();
        void RenderShapes();
        void RenderMetrics();
        void RenderPerformanceGraphs();

        // Profiling methods
        void UpdateProfilerData(const std::string& name, uint64_t startTime, uint64_t endTime);
        void AnalyzeBottlenecks();
        void CleanupExpiredProfiles();

        // Metrics methods
        void UpdatePerformanceMetrics(float deltaTime);
        void CollectSystemMetrics();
        void UpdateMetricHistory();

        // Visualization methods
        void UpdateShapeLifetimes(float deltaTime);
        void CleanupExpiredShapes();
        void AddShape(const DebugShape& shape);

        // Utility methods
        uint64_t GetCurrentTimeMicroseconds() const;
        std::string GetTimestampString() const;
        std::string FormatTime(float seconds) const;
        std::string FormatMemory(size_t bytes) const;
        Color GetLogLevelColor(LogLevel level) const;
        std::string GetSystemInfo() const;

        // Debug command processing
        void ProcessDebugCommands();
        void ExecuteCommand(const std::string& command);

        // Member variables
        DebugConfig m_config;
        bool m_initialized = false;

        // Core state
        DebugMode m_currentMode = DebugMode::NONE;
        bool m_overlayEnabled = true;
        bool m_profilingEnabled = true;
        bool m_metricsEnabled = true;

        // Profiling data
        std::unordered_map<std::string, ProfilerData> m_profilerData;
        std::unordered_map<std::string, uint64_t> m_activeProfiles;
        std::vector<std::string> m_profileStack;
        uint64_t m_frameStartTime = 0;

        // Metrics data
        PerformanceMetrics m_performanceMetrics;
        std::unordered_map<std::string, float> m_metrics;
        std::unordered_map<std::string, uint64_t> m_timerStarts;
        std::unordered_map<std::string, std::deque<float>> m_metricHistory;

        // Debug visualization
        std::vector<DebugShape> m_debugShapes;
        std::unordered_map<std::string, DebugOverlay> m_overlays;

        // Logging data
        struct LogEntry {
            LogLevel level;
            std::string message;
            std::string category;
            uint64_t timestamp;
            int threadId;
        };
        std::deque<LogEntry> m_logEntries;
        mutable std::shared_mutex m_logMutex;

        // System monitoring
        std::unordered_map<std::string, SystemMetrics> m_systemMetrics;

        // Memory debugging
        std::vector<MemorySnapshot> m_memorySnapshots;
        std::unordered_map<void*, std::pair<size_t, uint64_t>> m_allocationTracker;

        // Debug camera
        bool m_debugCameraEnabled = false;
        Vec3 m_debugCameraPosition = Vec3(0.0f);
        Vec3 m_debugCameraRotation = Vec3(0.0f);

        // Performance capture
        bool m_performanceCaptureActive = false;
        float m_performanceCaptureDuration = 0.0f;
        float m_performanceCaptureTime = 0.0f;
        std::vector<PerformanceMetrics> m_capturedMetrics;

        // Thread safety
        mutable std::shared_mutex m_systemMutex;
        mutable std::shared_mutex m_profileMutex;
        mutable std::shared_mutex m_metricsMutex;
        mutable std::shared_mutex m_shapeMutex;
        mutable std::shared_mutex m_overlayMutex;
    };

    // Template implementations
    template<typename Func>
    void DebugSystem::ProfileFunction(const std::string& name, Func&& func) {
        BeginProfile(name);
        try {
            func();
        } catch (...) {
            EndProfile(name);
            throw;
        }
        EndProfile(name);
    }

} // namespace VoxelCraft

#endif // VOXELCRAFT_DEBUG_DEBUG_SYSTEM_HPP
