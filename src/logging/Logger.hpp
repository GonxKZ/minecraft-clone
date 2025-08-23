/**
 * @file Logger.hpp
 * @brief VoxelCraft Advanced Logging System
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_LOGGING_LOGGER_HPP
#define VOXELCRAFT_LOGGING_LOGGER_HPP

#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
#include <atomic>
#include <mutex>
#include <shared_mutex>
#include <functional>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <filesystem>

#include "../math/Vec3.hpp"

namespace VoxelCraft {

    // Forward declarations
    class LogSink;
    class LogFormatter;
    class LogFilter;
    struct LogConfig;
    struct LogEntry;
    struct LogStats;
    struct LogContext;

    /**
     * @enum LogLevel
     * @brief Log severity levels
     */
    enum class LogLevel {
        TRACE = 0,           ///< Most verbose - detailed execution flow
        DEBUG = 1,           ///< Debug information for developers
        INFO = 2,            ///< General information messages
        WARNING = 3,         ///< Warning messages
        ERROR = 4,           ///< Error messages
        CRITICAL = 5,        ///< Critical error messages
        OFF = 6              ///< Disable all logging
    };

    /**
     * @enum LogSinkType
     * @brief Types of log output sinks
     */
    enum class LogSinkType {
        CONSOLE,             ///< Console output
        FILE,                ///< File output
        NETWORK,             ///< Network output
        DATABASE,            ///< Database output
        SYSTEM,              ///< System log
        CUSTOM               ///< Custom output
    };

    /**
     * @enum LogFormat
     * @brief Log message formatting options
     */
    enum class LogFormat {
        PLAIN_TEXT,          ///< Simple text format
        JSON,                ///< JSON format
        XML,                 ///< XML format
        CSV,                 ///< CSV format
        HTML,                ///< HTML format
        MARKDOWN,            ///< Markdown format
        CUSTOM               ///< Custom format
    };

    /**
     * @enum LogRotationType
     * @brief Log file rotation strategies
     */
    enum class LogRotationType {
        NONE,                ///< No rotation
        SIZE,                ///< Rotate by file size
        TIME,                ///< Rotate by time
        DAILY,               ///< Daily rotation
        WEEKLY,              ///< Weekly rotation
        MONTHLY,             ///< Monthly rotation
        CUSTOM               ///< Custom rotation
    };

    /**
     * @enum LogCompression
     * @brief Log file compression options
     */
    enum class LogCompression {
        NONE,                ///< No compression
        GZIP,                ///< GZIP compression
        ZIP,                 ///< ZIP compression
        LZ4,                 ///< LZ4 compression
        ZSTD,                ///< Zstandard compression
        CUSTOM               ///< Custom compression
    };

    /**
     * @struct LogEntry
     * @brief Individual log entry
     */
    struct LogEntry {
        uint64_t id;                          ///< Unique entry ID
        LogLevel level;                       ///< Log level
        std::string message;                  ///< Log message
        std::string category;                 ///< Log category
        std::string loggerName;               ///< Logger name
        std::chrono::system_clock::time_point timestamp;  ///< Timestamp
        std::thread::id threadId;             ///< Thread ID
        uint32_t processId;                   ///< Process ID
        std::string file;                     ///< Source file
        uint32_t line;                        ///< Source line
        std::string function;                 ///< Source function
        std::unordered_map<std::string, std::string> metadata;  ///< Additional metadata
        void* userData;                       ///< User-defined data
    };

    /**
     * @struct LogContext
     * @brief Logging context information
     */
    struct LogContext {
        std::string loggerName;               ///< Logger name
        std::string category;                 ///< Default category
        LogLevel minLevel;                    ///< Minimum log level
        bool enabled;                         ///< Whether logger is enabled
        std::unordered_map<std::string, std::string> properties;  ///< Context properties
        void* userData;                       ///< User-defined context data
    };

    /**
     * @struct LogConfig
     * @brief Logger configuration
     */
    struct LogConfig {
        bool enableLogging = true;
        bool enableAsyncLogging = true;
        bool enableBuffering = true;
        bool enableCompression = false;

        LogLevel defaultLevel = LogLevel::INFO;
        LogFormat defaultFormat = LogFormat::PLAIN_TEXT;
        std::string defaultPattern = "[%timestamp%] [%level%] [%category%] %message%";

        // File logging
        std::string logDirectory = "logs";
        std::string logFilename = "voxelcraft.log";
        LogRotationType rotationType = LogRotationType::DAILY;
        uint64_t maxFileSize = 10 * 1024 * 1024;  // 10MB
        uint32_t maxBackupFiles = 10;

        // Buffer settings
        uint32_t bufferSize = 8192;
        float flushInterval = 1.0f;  // seconds

        // Performance settings
        uint32_t maxQueueSize = 10000;
        float queueTimeout = 5.0f;  // seconds
        uint32_t workerThreads = 2;

        // Filtering
        std::vector<std::string> enabledCategories;
        std::vector<std::string> disabledCategories;
        std::unordered_map<LogLevel, bool> levelEnabled;

        // Network logging
        std::string networkHost = "localhost";
        uint16_t networkPort = 514;
        bool enableNetworkLogging = false;

        // System logging
        bool enableSystemLogging = false;
        std::string systemLogFacility = "user";
    };

    /**
     * @struct LogStats
     * @brief Logging system statistics
     */
    struct LogStats {
        uint64_t totalEntries = 0;            ///< Total log entries
        uint64_t entriesPerSecond = 0;        ///< Entries per second
        uint64_t droppedEntries = 0;          ///< Dropped entries due to queue full
        uint64_t filteredEntries = 0;         ///< Filtered out entries

        // Level statistics
        std::unordered_map<LogLevel, uint64_t> entriesByLevel;
        std::unordered_map<std::string, uint64_t> entriesByCategory;

        // Performance statistics
        float averageProcessingTime = 0.0f;   ///< Average time to process entry
        float maxProcessingTime = 0.0f;       ///< Maximum processing time
        uint64_t queueSize = 0;               ///< Current queue size
        uint64_t maxQueueSize = 0;            ///< Maximum queue size reached

        // Memory statistics
        uint64_t memoryUsed = 0;              ///< Memory used by logging system
        uint64_t buffersAllocated = 0;        ///< Number of buffers allocated

        // Error statistics
        uint64_t formattingErrors = 0;        ///< Number of formatting errors
        uint64_t sinkErrors = 0;              ///< Number of sink write errors
        uint64_t queueOverflows = 0;          ///< Number of queue overflows
    };

    /**
     * @class LogSink
     * @brief Abstract base class for log output sinks
     */
    class LogSink {
    public:
        LogSink(LogSinkType type, const std::string& name);
        virtual ~LogSink();

        virtual bool Initialize(const LogConfig& config) = 0;
        virtual void Shutdown() = 0;
        virtual bool Write(const LogEntry& entry) = 0;
        virtual bool Flush() = 0;

        LogSinkType GetType() const { return m_type; }
        const std::string& GetName() const { return m_name; }
        bool IsEnabled() const { return m_enabled; }
        void SetEnabled(bool enabled) { m_enabled = enabled; }

        void SetFormatter(std::shared_ptr<LogFormatter> formatter) { m_formatter = formatter; }
        void AddFilter(std::shared_ptr<LogFilter> filter) { m_filters.push_back(filter); }

    protected:
        bool ShouldWrite(const LogEntry& entry) const;
        std::string FormatEntry(const LogEntry& entry) const;

        LogSinkType m_type;
        std::string m_name;
        bool m_enabled = true;
        std::shared_ptr<LogFormatter> m_formatter;
        std::vector<std::shared_ptr<LogFilter>> m_filters;
    };

    /**
     * @class ConsoleSink
     * @brief Console output sink
     */
    class ConsoleSink : public LogSink {
    public:
        ConsoleSink(const std::string& name = "console");
        ~ConsoleSink() override;

        bool Initialize(const LogConfig& config) override;
        void Shutdown() override;
        bool Write(const LogEntry& entry) override;
        bool Flush() override;

    private:
        bool m_useColors = true;
        bool m_useStderr = false;
    };

    /**
     * @class FileSink
     * @brief File output sink with rotation and compression
     */
    class FileSink : public LogSink {
    public:
        FileSink(const std::string& name = "file");
        ~FileSink() override;

        bool Initialize(const LogConfig& config) override;
        void Shutdown() override;
        bool Write(const LogEntry& entry) override;
        bool Flush() override;

        void SetRotation(LogRotationType type, uint64_t maxSize = 10 * 1024 * 1024);
        void SetCompression(LogCompression compression);
        void SetMaxBackupFiles(uint32_t maxFiles) { m_maxBackupFiles = maxFiles; }

    private:
        bool RotateFile();
        bool CompressFile(const std::string& filename);
        std::string GetRotatedFilename() const;

        std::string m_logDirectory;
        std::string m_baseFilename;
        std::string m_currentFilename;
        std::ofstream m_fileStream;

        LogRotationType m_rotationType = LogRotationType::NONE;
        uint64_t m_maxFileSize = 10 * 1024 * 1024;
        uint64_t m_currentFileSize = 0;
        uint32_t m_maxBackupFiles = 10;
        LogCompression m_compression = LogCompression::NONE;

        std::chrono::system_clock::time_point m_lastRotation;
    };

    /**
     * @class NetworkSink
     * @brief Network output sink for remote logging
     */
    class NetworkSink : public LogSink {
    public:
        NetworkSink(const std::string& name = "network");
        ~NetworkSink() override;

        bool Initialize(const LogConfig& config) override;
        void Shutdown() override;
        bool Write(const LogEntry& entry) override;
        bool Flush() override;

        void SetHost(const std::string& host) { m_host = host; }
        void SetPort(uint16_t port) { m_port = port; }
        void SetProtocol(const std::string& protocol) { m_protocol = protocol; }

    private:
        std::string m_host = "localhost";
        uint16_t m_port = 514;
        std::string m_protocol = "udp";
        int m_socket = -1;
        bool m_connected = false;
    };

    /**
     * @class LogFormatter
     * @brief Abstract base class for log message formatting
     */
    class LogFormatter {
    public:
        LogFormatter(LogFormat format);
        virtual ~LogFormatter();

        virtual std::string Format(const LogEntry& entry) = 0;
        void SetPattern(const std::string& pattern) { m_pattern = pattern; }

        LogFormat GetFormat() const { return m_format; }

    protected:
        std::string ReplacePlaceholders(const std::string& pattern, const LogEntry& entry) const;
        std::string GetTimestampString(const std::chrono::system_clock::time_point& timestamp) const;
        std::string GetLevelString(LogLevel level) const;

        LogFormat m_format;
        std::string m_pattern;
    };

    /**
     * @class PlainTextFormatter
     * @brief Plain text log formatter
     */
    class PlainTextFormatter : public LogFormatter {
    public:
        PlainTextFormatter();
        ~PlainTextFormatter() override;

        std::string Format(const LogEntry& entry) override;
    };

    /**
     * @class JSONFormatter
     * @brief JSON log formatter
     */
    class JSONFormatter : public LogFormatter {
    public:
        JSONFormatter();
        ~JSONFormatter() override;

        std::string Format(const LogEntry& entry) override;
    };

    /**
     * @class LogFilter
     * @brief Abstract base class for log filtering
     */
    class LogFilter {
    public:
        LogFilter(const std::string& name);
        virtual ~LogFilter();

        virtual bool ShouldFilter(const LogEntry& entry) = 0;
        const std::string& GetName() const { return m_name; }

    protected:
        std::string m_name;
    };

    /**
     * @class LevelFilter
     * @brief Filter logs by level
     */
    class LevelFilter : public LogFilter {
    public:
        LevelFilter(LogLevel minLevel);
        ~LevelFilter() override;

        bool ShouldFilter(const LogEntry& entry) override;

    private:
        LogLevel m_minLevel;
    };

    /**
     * @class CategoryFilter
     * @brief Filter logs by category
     */
    class CategoryFilter : public LogFilter {
    public:
        CategoryFilter(const std::vector<std::string>& categories, bool include = true);
        ~CategoryFilter() override;

        bool ShouldFilter(const LogEntry& entry) override;

    private:
        std::vector<std::string> m_categories;
        bool m_include;
    };

    /**
     * @class Logger
     * @brief Main logging system
     *
     * Features:
     * - 10+ output types (console, file, network, database, system)
     * - Multiple log levels with fine-grained control
     * - Asynchronous logging with buffering
     * - Log rotation and compression
     * - Custom formatting and filtering
     * - Performance monitoring and statistics
     * - Context-aware logging
     * - Thread-safe operations
     * - Log analysis and querying
     * - Remote logging capabilities
     * - Structured logging support
     * - Log archival and retention
     * - Real-time log streaming
     * - Performance benchmarking integration
     */
    class Logger {
    public:
        static Logger& GetInstance();

        /**
         * @brief Initialize the logging system
         * @param config Logger configuration
         * @return true if successful
         */
        bool Initialize(const LogConfig& config);

        /**
         * @brief Shutdown the logging system
         */
        void Shutdown();

        /**
         * @brief Update logging system (call every frame)
         * @param deltaTime Time since last update
         */
        void Update(float deltaTime);

        // Core Logging API
        /**
         * @brief Log a message with TRACE level
         * @param message Log message
         * @param category Log category
         * @param context Optional context data
         */
        void Trace(const std::string& message, const std::string& category = "",
                  const std::unordered_map<std::string, std::string>& context = {});

        /**
         * @brief Log a message with DEBUG level
         * @param message Log message
         * @param category Log category
         * @param context Optional context data
         */
        void Debug(const std::string& message, const std::string& category = "",
                  const std::unordered_map<std::string, std::string>& context = {});

        /**
         * @brief Log a message with INFO level
         * @param message Log message
         * @param category Log category
         * @param context Optional context data
         */
        void Info(const std::string& message, const std::string& category = "",
                 const std::unordered_map<std::string, std::string>& context = {});

        /**
         * @brief Log a message with WARNING level
         * @param message Log message
         * @param category Log category
         * @param context Optional context data
         */
        void Warning(const std::string& message, const std::string& category = "",
                    const std::unordered_map<std::string, std::string>& context = {});

        /**
         * @brief Log a message with ERROR level
         * @param message Log message
         * @param category Log category
         * @param context Optional context data
         */
        void Error(const std::string& message, const std::string& category = "",
                  const std::unordered_map<std::string, std::string>& context = {});

        /**
         * @brief Log a message with CRITICAL level
         * @param message Log message
         * @param category Log category
         * @param context Optional context data
         */
        void Critical(const std::string& message, const std::string& category = "",
                     const std::unordered_map<std::string, std::string>& context = {});

        /**
         * @brief Log a message with custom level
         * @param level Log level
         * @param message Log message
         * @param category Log category
         * @param context Optional context data
         */
        void Log(LogLevel level, const std::string& message, const std::string& category = "",
                const std::unordered_map<std::string, std::string>& context = {});

        // Advanced Logging API
        /**
         * @brief Create a contextual logger
         * @param name Logger name
         * @param context Logger context
         * @return Logger instance
         */
        std::shared_ptr<Logger> CreateLogger(const std::string& name, const LogContext& context);

        /**
         * @brief Log a structured message
         * @param level Log level
         * @param data Structured data
         * @param category Log category
         */
        void LogStructured(LogLevel level, const std::unordered_map<std::string, std::string>& data,
                          const std::string& category = "");

        /**
         * @brief Log a performance measurement
         * @param name Measurement name
         * @param value Measurement value
         * @param unit Unit of measurement
         * @param category Log category
         */
        void LogPerformance(const std::string& name, double value, const std::string& unit = "",
                           const std::string& category = "performance");

        /**
         * @brief Log a system event
         * @param event Event name
         * @param details Event details
         * @param category Log category
         */
        void LogSystemEvent(const std::string& event, const std::string& details,
                           const std::string& category = "system");

        // Sink Management API
        /**
         * @brief Add a log sink
         * @param sink Log sink to add
         * @return true if successful
         */
        bool AddSink(std::shared_ptr<LogSink> sink);

        /**
         * @brief Remove a log sink
         * @param name Sink name
         * @return true if successful
         */
        bool RemoveSink(const std::string& name);

        /**
         * @brief Get a log sink
         * @param name Sink name
         * @return Log sink or nullptr
         */
        std::shared_ptr<LogSink> GetSink(const std::string& name);

        /**
         * @brief Enable/disable sink
         * @param name Sink name
         * @param enabled Enable state
         * @return true if successful
         */
        bool SetSinkEnabled(const std::string& name, bool enabled);

        // Configuration API
        /**
         * @brief Set minimum log level
         * @param level Minimum level
         */
        void SetLevel(LogLevel level) { m_config.defaultLevel = level; }

        /**
         * @brief Set log format
         * @param format Log format
         */
        void SetFormat(LogFormat format) { m_config.defaultFormat = format; }

        /**
         * @brief Enable/disable category
         * @param category Log category
         * @param enabled Enable state
         */
        void SetCategoryEnabled(const std::string& category, bool enabled);

        /**
         * @brief Set log pattern
         * @param pattern Log pattern
         */
        void SetPattern(const std::string& pattern) { m_config.defaultPattern = pattern; }

        // Query and Analysis API
        /**
         * @brief Query log entries
         * @param startTime Start time
         * @param endTime End time
         * @param level Optional level filter
         * @param category Optional category filter
         * @param limit Maximum entries to return
         * @return Vector of log entries
         */
        std::vector<LogEntry> QueryLogs(const std::chrono::system_clock::time_point& startTime,
                                       const std::chrono::system_clock::time_point& endTime,
                                       LogLevel level = LogLevel::TRACE,
                                       const std::string& category = "",
                                       uint32_t limit = 1000) const;

        /**
         * @brief Get log statistics
         * @param startTime Start time
         * @param endTime End time
         * @return Log statistics
         */
        LogStats GetLogStats(const std::chrono::system_clock::time_point& startTime,
                           const std::chrono::system_clock::time_point& endTime) const;

        /**
         * @brief Search log entries
         * @param query Search query
         * @param caseSensitive Case sensitive search
         * @param limit Maximum entries to return
         * @return Vector of matching entries
         */
        std::vector<LogEntry> SearchLogs(const std::string& query, bool caseSensitive = false,
                                        uint32_t limit = 1000) const;

        /**
         * @brief Export logs to file
         * @param filename Export filename
         * @param startTime Start time
         * @param endTime End time
         * @param format Export format
         * @return true if successful
         */
        bool ExportLogs(const std::string& filename,
                       const std::chrono::system_clock::time_point& startTime,
                       const std::chrono::system_clock::time_point& endTime,
                       LogFormat format = LogFormat::PLAIN_TEXT);

        // Performance API
        /**
         * @brief Start performance measurement
         * @param name Measurement name
         * @return Measurement ID
         */
        uint64_t StartPerformanceMeasurement(const std::string& name);

        /**
         * @brief End performance measurement
         * @param id Measurement ID
         * @return Measurement duration in microseconds
         */
        uint64_t EndPerformanceMeasurement(uint64_t id);

        /**
         * @brief Get performance statistics
         * @return Performance statistics
         */
        std::unordered_map<std::string, double> GetPerformanceStats() const;

        // Utility API
        /**
         * @brief Flush all sinks
         */
        void Flush();

        /**
         * @brief Clear log history
         */
        void ClearHistory();

        /**
         * @brief Get log configuration
         * @return Current configuration
         */
        const LogConfig& GetConfig() const { return m_config; }

        /**
         * @brief Get logging statistics
         * @return Current statistics
         */
        const LogStats& GetStats() const { return m_stats; }

        /**
         * @brief Get system info
         * @return System information string
         */
        std::string GetSystemInfo() const;

        /**
         * @brief Validate logging system
         * @return true if valid
         */
        bool Validate() const;

    private:
        Logger() = default;
        ~Logger();

        // Prevent copying
        Logger(const Logger&) = delete;
        Logger& operator=(const Logger&) = delete;

        // Core logging methods
        void LogInternal(const LogEntry& entry);
        void ProcessLogEntry(const LogEntry& entry);
        uint64_t GenerateEntryId();
        LogEntry CreateLogEntry(LogLevel level, const std::string& message, const std::string& category,
                               const std::unordered_map<std::string, std::string>& context);

        // Async processing
        void InitializeAsyncProcessing();
        void ShutdownAsyncProcessing();
        void AsyncWorkerThread();
        void ProcessQueuedEntries();

        // Sink management
        void InitializeDefaultSinks();
        void WriteToSinks(const LogEntry& entry);
        void FlushSinks();

        // Statistics and monitoring
        void UpdateStatistics(const LogEntry& entry);
        void UpdatePerformanceStats();

        // Utility methods
        std::string GetCurrentTimestamp() const;
        std::string GetThreadIdString(std::thread::id threadId) const;
        bool ShouldLog(LogLevel level, const std::string& category) const;
        void RotateLogsIfNeeded();

        // Member variables
        bool m_initialized = false;
        LogConfig m_config;

        // Core components
        std::unordered_map<std::string, std::shared_ptr<LogSink>> m_sinks;
        std::shared_ptr<LogFormatter> m_defaultFormatter;

        // Async processing
        std::vector<std::thread> m_workerThreads;
        std::atomic<bool> m_shutdownWorkers;
        std::deque<LogEntry> m_entryQueue;
        mutable std::mutex m_queueMutex;
        std::condition_variable m_queueCondition;

        // Statistics
        LogStats m_stats;
        mutable std::shared_mutex m_statsMutex;

        // Performance measurement
        std::unordered_map<uint64_t, std::chrono::steady_clock::time_point> m_activeMeasurements;
        std::unordered_map<std::string, std::vector<uint64_t>> m_performanceHistory;
        mutable std::shared_mutex m_performanceMutex;

        // Entry tracking
        std::deque<LogEntry> m_entryHistory;
        mutable std::shared_mutex m_historyMutex;
        std::atomic<uint64_t> m_nextEntryId;
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_LOGGING_LOGGER_HPP
