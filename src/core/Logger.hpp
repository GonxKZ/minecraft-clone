/**
 * @file Logger.hpp
 * @brief VoxelCraft Engine Advanced Logging System
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file provides a comprehensive, high-performance logging system
 * for the VoxelCraft engine with support for multiple sinks, formatting,
 * filtering, and performance monitoring.
 */

#ifndef VOXELCRAFT_CORE_LOGGER_HPP
#define VOXELCRAFT_CORE_LOGGER_HPP

#include <string>
#include <memory>
#include <vector>
#include <mutex>
#include <atomic>
#include <chrono>
#include <sstream>
#include <filesystem>
#include <unordered_map>
#include <functional>
#include <optional>

namespace VoxelCraft {

    /**
     * @enum LogLevel
     * @brief Log message severity levels
     */
    enum class LogLevel {
        Trace = 0,    ///< Most verbose level, used for detailed tracing
        Debug,        ///< Debug information for developers
        Info,         ///< General information messages
        Warning,      ///< Warning messages that don't stop execution
        Error,        ///< Error messages that indicate problems
        Fatal,        ///< Fatal errors that require immediate attention
        Off           ///< Disable all logging
    };

    /**
     * @struct LogMessage
     * @brief Structure representing a log message
     */
    struct LogMessage {
        LogLevel level;                           ///< Message severity level
        std::string logger;                       ///< Logger name
        std::string message;                      ///< Log message content
        std::string file;                         ///< Source file name
        uint32_t line;                            ///< Source line number
        std::string function;                     ///< Source function name
        std::chrono::system_clock::time_point timestamp; ///< Message timestamp
        std::thread::id threadId;                 ///< Thread ID
        uint32_t sequenceNumber;                  ///< Global sequence number
    };

    /**
     * @struct LogStatistics
     * @brief Logging system performance statistics
     */
    struct LogStatistics {
        uint64_t totalMessages;                   ///< Total messages logged
        uint64_t messagesPerSecond;               ///< Current messages per second
        uint64_t droppedMessages;                 ///< Messages dropped due to filtering
        double averageProcessingTime;             ///< Average time to process message (μs)
        size_t memoryUsage;                       ///< Current memory usage
        size_t peakMemoryUsage;                   ///< Peak memory usage
        uint32_t activeLoggers;                   ///< Number of active loggers
    };

    /**
     * @class LogSink
     * @brief Abstract base class for log output destinations
     */
    class LogSink {
    public:
        /**
         * @brief Constructor
         */
        LogSink() = default;

        /**
         * @brief Destructor
         */
        virtual ~LogSink() = default;

        /**
         * @brief Deleted copy constructor
         */
        LogSink(const LogSink&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        LogSink& operator=(const LogSink&) = delete;

        /**
         * @brief Write log message to sink
         * @param message Log message to write
         */
        virtual void Write(const LogMessage& message) = 0;

        /**
         * @brief Flush any buffered output
         */
        virtual void Flush() = 0;

        /**
         * @brief Get sink name
         * @return Sink name
         */
        virtual std::string GetName() const = 0;

        /**
         * @brief Check if sink should handle message level
         * @param level Log level to check
         * @return true if should handle, false otherwise
         */
        virtual bool ShouldHandle(LogLevel level) const { return true; }
    };

    /**
     * @class ConsoleSink
     * @brief Console output sink with color support
     */
    class ConsoleSink : public LogSink {
    public:
        /**
         * @brief Constructor
         * @param useColors Enable colored output
         * @param useStderr Use stderr for error messages
         */
        explicit ConsoleSink(bool useColors = true, bool useStderr = true);

        /**
         * @brief Write log message to console
         * @param message Log message to write
         */
        void Write(const LogMessage& message) override;

        /**
         * @brief Flush console output
         */
        void Flush() override;

        /**
         * @brief Get sink name
         * @return Sink name
         */
        std::string GetName() const override { return "Console"; }

        /**
         * @brief Check if sink should handle message level
         * @param level Log level to check
         * @return true if should handle, false otherwise
         */
        bool ShouldHandle(LogLevel level) const override;

    private:
        /**
         * @brief Get ANSI color code for log level
         * @param level Log level
         * @return Color code string
         */
        std::string GetColorCode(LogLevel level) const;

        /**
         * @brief Reset color to default
         * @return Reset code string
         */
        std::string ResetColor() const;

        bool m_useColors;                    ///< Use colored output
        bool m_useStderr;                    ///< Use stderr for errors
    };

    /**
     * @class FileSink
     * @brief File output sink with rotation and compression
     */
    class FileSink : public LogSink {
    public:
        /**
         * @brief Constructor
         * @param filename Log file path
         * @param maxSize Maximum file size before rotation
         * @param maxFiles Maximum number of rotated files
         * @param compressOldFiles Compress old log files
         */
        explicit FileSink(const std::string& filename,
                         size_t maxSize = 10 * 1024 * 1024, // 10MB
                         uint32_t maxFiles = 5,
                         bool compressOldFiles = true);

        /**
         * @brief Destructor
         */
        ~FileSink() override;

        /**
         * @brief Write log message to file
         * @param message Log message to write
         */
        void Write(const LogMessage& message) override;

        /**
         * @brief Flush file output
         */
        void Flush() override;

        /**
         * @brief Get sink name
         * @return Sink name
         */
        std::string GetName() const override { return "File"; }

        /**
         * @brief Get current file size
         * @return File size in bytes
         */
        size_t GetCurrentFileSize() const { return m_currentSize; }

        /**
         * @brief Get number of rotated files
         * @return Number of rotated files
         */
        uint32_t GetRotatedFileCount() const { return m_rotatedFiles; }

    private:
        /**
         * @brief Rotate log file if needed
         */
        void RotateFile();

        /**
         * @brief Compress old log file
         * @param filename File to compress
         */
        void CompressFile(const std::string& filename);

        /**
         * @brief Generate rotated filename
         * @param index Rotation index
         * @return Rotated filename
         */
        std::string GetRotatedFilename(uint32_t index) const;

        std::string m_filename;               ///< Base log filename
        std::filesystem::path m_filepath;     ///< Full log file path
        std::unique_ptr<std::ofstream> m_file; ///< File stream
        size_t m_maxSize;                     ///< Maximum file size
        uint32_t m_maxFiles;                  ///< Maximum rotated files
        uint32_t m_rotatedFiles;              ///< Current rotated files count
        size_t m_currentSize;                 ///< Current file size
        bool m_compressOldFiles;              ///< Compress old files
    };

    /**
     * @class NetworkSink
     * @brief Network output sink for remote logging
     */
    class NetworkSink : public LogSink {
    public:
        /**
         * @brief Constructor
         * @param host Remote host address
         * @param port Remote port
         * @param reconnectInterval Reconnection interval in seconds
         */
        explicit NetworkSink(const std::string& host,
                           uint16_t port,
                           double reconnectInterval = 5.0);

        /**
         * @brief Destructor
         */
        ~NetworkSink() override;

        /**
         * @brief Write log message to network
         * @param message Log message to write
         */
        void Write(const LogMessage& message) override;

        /**
         * @brief Flush network output
         */
        void Flush() override;

        /**
         * @brief Get sink name
         * @return Sink name
         */
        std::string GetName() const override { return "Network"; }

        /**
         * @brief Check connection status
         * @return true if connected, false otherwise
         */
        bool IsConnected() const { return m_connected; }

    private:
        /**
         * @brief Connect to remote server
         */
        void Connect();

        /**
         * @brief Disconnect from remote server
         */
        void Disconnect();

        /**
         * @brief Reconnection thread function
         */
        void ReconnectionThread();

        std::string m_host;                   ///< Remote host
        uint16_t m_port;                      ///< Remote port
        double m_reconnectInterval;           ///< Reconnection interval
        std::unique_ptr<std::thread> m_reconnectThread; ///< Reconnection thread
        std::atomic<bool> m_connected;        ///< Connection status
        std::atomic<bool> m_running;          ///< Thread running flag
        mutable std::mutex m_connectionMutex; ///< Connection synchronization
        // Network socket would be here in full implementation
    };

    /**
     * @class Logger
     * @brief Main logger class with filtering and formatting
     */
    class Logger {
    public:
        /**
         * @brief Constructor
         * @param name Logger name
         * @param level Default log level
         */
        explicit Logger(const std::string& name, LogLevel level = LogLevel::Info);

        /**
         * @brief Destructor
         */
        ~Logger();

        /**
         * @brief Deleted copy constructor
         */
        Logger(const Logger&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        Logger& operator=(const Logger&) = delete;

        // Logging functions

        /**
         * @brief Log trace message
         * @param message Log message
         * @param file Source file (auto-filled)
         * @param line Source line (auto-filled)
         * @param function Source function (auto-filled)
         */
        void Trace(const std::string& message,
                  const char* file = __FILE__,
                  uint32_t line = __LINE__,
                  const char* function = __FUNCTION__);

        /**
         * @brief Log debug message
         * @param message Log message
         * @param file Source file (auto-filled)
         * @param line Source line (auto-filled)
         * @param function Source function (auto-filled)
         */
        void Debug(const std::string& message,
                  const char* file = __FILE__,
                  uint32_t line = __LINE__,
                  const char* function = __FUNCTION__);

        /**
         * @brief Log info message
         * @param message Log message
         * @param file Source file (auto-filled)
         * @param line Source line (auto-filled)
         * @param function Source function (auto-filled)
         */
        void Info(const std::string& message,
                 const char* file = __FILE__,
                 uint32_t line = __LINE__,
                 const char* function = __FUNCTION__);

        /**
         * @brief Log warning message
         * @param message Log message
         * @param file Source file (auto-filled)
         * @param line Source line (auto-filled)
         * @param function Source function (auto-filled)
         */
        void Warning(const std::string& message,
                    const char* file = __FILE__,
                    uint32_t line = __LINE__,
                    const char* function = __FUNCTION__);

        /**
         * @brief Log error message
         * @param message Log message
         * @param file Source file (auto-filled)
         * @param line Source line (auto-filled)
         * @param function Source function (auto-filled)
         */
        void Error(const std::string& message,
                  const char* file = __FILE__,
                  uint32_t line = __LINE__,
                  const char* function = __FUNCTION__);

        /**
         * @brief Log fatal message
         * @param message Log message
         * @param file Source file (auto-filled)
         * @param line Source line (auto-filled)
         * @param function Source function (auto-filled)
         */
        void Fatal(const std::string& message,
                  const char* file = __FILE__,
                  uint32_t line = __LINE__,
                  const char* function = __FUNCTION__);

        // Configuration

        /**
         * @brief Set log level
         * @param level New log level
         */
        void SetLevel(LogLevel level) { m_level = level; }

        /**
         * @brief Get current log level
         * @return Current log level
         */
        LogLevel GetLevel() const { return m_level; }

        /**
         * @brief Get logger name
         * @return Logger name
         */
        const std::string& GetName() const { return m_name; }

        /**
         * @brief Add log sink
         * @param sink Sink to add
         */
        void AddSink(std::shared_ptr<LogSink> sink);

        /**
         * @brief Remove log sink
         * @param sinkName Name of sink to remove
         */
        void RemoveSink(const std::string& sinkName);

        /**
         * @brief Clear all sinks
         */
        void ClearSinks();

        /**
         * @brief Enable/disable logger
         * @param enabled Enable state
         */
        void SetEnabled(bool enabled) { m_enabled = enabled; }

        /**
         * @brief Check if logger is enabled
         * @return true if enabled, false otherwise
         */
        bool IsEnabled() const { return m_enabled; }

        // Statistics

        /**
         * @brief Get logger statistics
         * @return Logger statistics
         */
        LogStatistics GetStatistics() const;

        /**
         * @brief Reset statistics
         */
        void ResetStatistics();

    private:
        /**
         * @brief Log message with specified level
         * @param level Log level
         * @param message Log message
         * @param file Source file
         * @param line Source line
         * @param function Source function
         */
        void Log(LogLevel level,
                const std::string& message,
                const char* file,
                uint32_t line,
                const char* function);

        /**
         * @brief Format log message
         * @param message Log message data
         * @return Formatted message string
         */
        std::string FormatMessage(const LogMessage& message) const;

        /**
         * @brief Check if message should be logged
         * @param level Message level
         * @return true if should log, false otherwise
         */
        bool ShouldLog(LogLevel level) const;

        /**
         * @brief Write message to all sinks
         * @param message Log message
         */
        void WriteToSinks(const LogMessage& message);

        std::string m_name;                                    ///< Logger name
        LogLevel m_level;                                      ///< Current log level
        bool m_enabled;                                        ///< Logger enabled flag

        std::vector<std::shared_ptr<LogSink>> m_sinks;         ///< Log sinks
        mutable std::mutex m_sinksMutex;                       ///< Sinks synchronization

        // Statistics
        mutable std::atomic<uint64_t> m_messageCount;          ///< Message counter
        mutable std::atomic<uint64_t> m_droppedCount;          ///< Dropped message counter
        mutable std::chrono::steady_clock::time_point m_startTime; ///< Start time for rate calculation

        // Formatting options
        std::string m_formatString;                            ///< Message format string
        bool m_includeTimestamp;                               ///< Include timestamp
        bool m_includeThreadId;                                ///< Include thread ID
        bool m_includeLoggerName;                              ///< Include logger name
        bool m_includeSourceLocation;                          ///< Include source location
    };

    /**
     * @class LogManager
     * @brief Global logging system manager
     */
    class LogManager {
    public:
        /**
         * @brief Initialize logging system
         * @param configPath Configuration file path
         * @return true if successful, false otherwise
         */
        static bool Initialize(const std::string& configPath = "");

        /**
         * @brief Shutdown logging system
         */
        static void Shutdown();

        /**
         * @brief Get logger instance
         * @param name Logger name
         * @return Logger instance
         */
        static std::shared_ptr<Logger> GetLogger(const std::string& name);

        /**
         * @brief Create logger with specific configuration
         * @param name Logger name
         * @param level Default log level
         * @param sinks List of sinks to attach
         * @return Logger instance
         */
        static std::shared_ptr<Logger> CreateLogger(
            const std::string& name,
            LogLevel level = LogLevel::Info,
            const std::vector<std::shared_ptr<LogSink>>& sinks = {}
        );

        /**
         * @brief Set global log level
         * @param level Global log level
         */
        static void SetGlobalLevel(LogLevel level);

        /**
         * @brief Get global log level
         * @return Global log level
         */
        static LogLevel GetGlobalLevel();

        /**
         * @brief Get system statistics
         * @return System statistics
         */
        static LogStatistics GetStatistics();

        /**
         * @brief Flush all loggers
         */
        static void FlushAll();

    private:
        /**
         * @brief Constructor (private for singleton)
         */
        LogManager() = default;

        /**
         * @brief Destructor
         */
        ~LogManager() = default;

        /**
         * @brief Get singleton instance
         * @return LogManager instance
         */
        static LogManager& GetInstance();

        std::unordered_map<std::string, std::shared_ptr<Logger>> m_loggers;
        std::vector<std::shared_ptr<LogSink>> m_globalSinks;
        LogLevel m_globalLevel;
        mutable std::mutex m_mutex;
    };

    // Global logger macros

    /**
     * @def VOXELCRAFT_TRACE(message)
     * @brief Log trace message to default logger
     */
    #define VOXELCRAFT_TRACE(message) \
        VoxelCraft::LogManager::GetLogger("VoxelCraft")->Trace(message, __FILE__, __LINE__, __FUNCTION__)

    /**
     * @def VOXELCRAFT_DEBUG(message)
     * @brief Log debug message to default logger
     */
    #define VOXELCRAFT_DEBUG(message) \
        VoxelCraft::LogManager::GetLogger("VoxelCraft")->Debug(message, __FILE__, __LINE__, __FUNCTION__)

    /**
     * @def VOXELCRAFT_INFO(message)
     * @brief Log info message to default logger
     */
    #define VOXELCRAFT_INFO(message) \
        VoxelCraft::LogManager::GetLogger("VoxelCraft")->Info(message, __FILE__, __LINE__, __FUNCTION__)

    /**
     * @def VOXELCRAFT_WARNING(message)
     * @brief Log warning message to default logger
     */
    #define VOXELCRAFT_WARNING(message) \
        VoxelCraft::LogManager::GetLogger("VoxelCraft")->Warning(message, __FILE__, __LINE__, __FUNCTION__)

    /**
     * @def VOXELCRAFT_ERROR(message)
     * @brief Log error message to default logger
     */
    #define VOXELCRAFT_ERROR(message) \
        VoxelCraft::LogManager::GetLogger("VoxelCraft")->Error(message, __FILE__, __LINE__, __FUNCTION__)

    /**
     * @def VOXELCRAFT_FATAL(message)
     * @brief Log fatal message to default logger
     */
    #define VOXELCRAFT_FATAL(message) \
        VoxelCraft::LogManager::GetLogger("VoxelCraft")->Fatal(message, __FILE__, __LINE__, __FUNCTION__)

} // namespace VoxelCraft

#endif // VOXELCRAFT_CORE_LOGGER_HPP
