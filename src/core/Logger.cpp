/**
 * @file Logger.cpp
 * @brief VoxelCraft Engine Logger Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file implements the logging system for the VoxelCraft engine.
 */

#include "Logger.hpp"

#include <iostream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <thread>
#include <algorithm>
#include <sstream>

namespace VoxelCraft {

    // ConsoleSink implementation

    ConsoleSink::ConsoleSink(bool useColors, bool useStderr)
        : m_useColors(useColors), m_useStderr(useStderr)
    {
    }

    void ConsoleSink::Write(const LogMessage& message) {
        std::ostream& stream = (m_useStderr && message.level >= LogLevel::Error) ? std::cerr : std::cout;

        if (m_useColors) {
            stream << GetColorCode(message.level);
        }

        stream << FormatMessage(message);

        if (m_useColors) {
            stream << ResetColor();
        }

        stream << std::endl;
    }

    void ConsoleSink::Flush() {
        std::cout.flush();
        std::cerr.flush();
    }

    bool ConsoleSink::ShouldHandle(LogLevel level) const {
        return true; // Handle all levels
    }

    std::string ConsoleSink::GetColorCode(LogLevel level) const {
        if (!m_useColors) return "";

        switch (level) {
            case LogLevel::Trace: return "\033[37m";   // White
            case LogLevel::Debug: return "\033[36m";   // Cyan
            case LogLevel::Info: return "\033[32m";    // Green
            case LogLevel::Warning: return "\033[33m"; // Yellow
            case LogLevel::Error: return "\033[31m";   // Red
            case LogLevel::Fatal: return "\033[35m";   // Magenta
            default: return "\033[0m";                 // Reset
        }
    }

    std::string ConsoleSink::ResetColor() const {
        return m_useColors ? "\033[0m" : "";
    }

    // FileSink implementation

    FileSink::FileSink(const std::string& filename, size_t maxSize, uint32_t maxFiles, bool compressOldFiles)
        : m_filename(filename)
        , m_maxSize(maxSize)
        , m_maxFiles(maxFiles)
        , m_rotatedFiles(0)
        , m_currentSize(0)
        , m_compressOldFiles(compressOldFiles)
    {
        m_filepath = std::filesystem::path(filename);
        m_file = std::make_unique<std::ofstream>(filename, std::ios::app);

        if (!m_file->is_open()) {
            throw std::runtime_error("Failed to open log file: " + filename);
        }

        // Calculate current file size
        if (std::filesystem::exists(m_filepath)) {
            m_currentSize = std::filesystem::file_size(m_filepath);
        }
    }

    FileSink::~FileSink() {
        if (m_file && m_file->is_open()) {
            m_file->close();
        }
    }

    void FileSink::Write(const LogMessage& message) {
        if (!m_file || !m_file->is_open()) {
            return;
        }

        std::string formattedMessage = FormatMessage(message) + "\n";
        m_file->write(formattedMessage.c_str(), formattedMessage.length());
        m_currentSize += formattedMessage.length();

        // Check if rotation is needed
        if (m_currentSize >= m_maxSize) {
            RotateFile();
        }
    }

    void FileSink::Flush() {
        if (m_file && m_file->is_open()) {
            m_file->flush();
        }
    }

    void FileSink::RotateFile() {
        if (m_file) {
            m_file->close();
        }

        // Rotate existing files
        for (uint32_t i = m_maxFiles - 1; i > 0; --i) {
            std::string oldName = GetRotatedFilename(i - 1);
            std::string newName = GetRotatedFilename(i);

            if (std::filesystem::exists(oldName)) {
                if (std::filesystem::exists(newName)) {
                    std::filesystem::remove(newName);
                }
                std::filesystem::rename(oldName, newName);
            }
        }

        // Move current file
        if (std::filesystem::exists(m_filename)) {
            std::string rotatedName = GetRotatedFilename(0);
            std::filesystem::rename(m_filename, rotatedName);

            if (m_compressOldFiles) {
                CompressFile(rotatedName);
            }
        }

        // Open new file
        m_file = std::make_unique<std::ofstream>(m_filename, std::ios::app);
        m_currentSize = 0;
    }

    void FileSink::CompressFile(const std::string& filename) {
        // Simple compression - in a real implementation you'd use gzip or similar
        // For now, just rename with .gz extension
        std::string compressedName = filename + ".gz";
        if (std::filesystem::exists(filename)) {
            std::filesystem::rename(filename, compressedName);
        }
    }

    std::string FileSink::GetRotatedFilename(uint32_t index) const {
        return m_filename + "." + std::to_string(index + 1);
    }

    // NetworkSink implementation (simplified)

    NetworkSink::NetworkSink(const std::string& host, uint16_t port, double reconnectInterval)
        : m_host(host)
        , m_port(port)
        , m_reconnectInterval(reconnectInterval)
        , m_connected(false)
        , m_running(false)
    {
        // In a real implementation, you'd initialize network connection here
    }

    NetworkSink::~NetworkSink() {
        Disconnect();
    }

    void NetworkSink::Write(const LogMessage& message) {
        // Simplified - just print to console that we'd send over network
        if (m_connected) {
            std::cout << "[Network] " << FormatMessage(message) << std::endl;
        }
    }

    void NetworkSink::Flush() {
        // Network flush would happen here
    }

    void NetworkSink::Connect() {
        // Network connection logic would go here
        m_connected = true;
    }

    void NetworkSink::Disconnect() {
        m_running = false;
        if (m_reconnectThread && m_reconnectThread->joinable()) {
            m_reconnectThread->join();
        }
        m_connected = false;
    }

    void NetworkSink::ReconnectionThread() {
        while (m_running) {
            if (!m_connected) {
                Connect();
            }
            std::this_thread::sleep_for(std::chrono::duration<double>(m_reconnectInterval));
        }
    }

    // Logger implementation

    Logger::Logger(const std::string& name, LogLevel level)
        : m_name(name)
        , m_level(level)
        , m_enabled(true)
        , m_messageCount(0)
        , m_droppedCount(0)
        , m_formatString("[%Y-%m-%d %H:%M:%S] [%l] [%n] %v")
        , m_includeTimestamp(true)
        , m_includeThreadId(true)
        , m_includeLoggerName(true)
        , m_includeSourceLocation(true)
    {
        m_startTime = std::chrono::steady_clock::now();
    }

    Logger::~Logger() {
        ClearSinks();
    }

    void Logger::Trace(const std::string& message, const char* file, uint32_t line, const char* function) {
        Log(LogLevel::Trace, message, file, line, function);
    }

    void Logger::Debug(const std::string& message, const char* file, uint32_t line, const char* function) {
        Log(LogLevel::Debug, message, file, line, function);
    }

    void Logger::Info(const std::string& message, const char* file, uint32_t line, const char* function) {
        Log(LogLevel::Info, message, file, line, function);
    }

    void Logger::Warning(const std::string& message, const char* file, uint32_t line, const char* function) {
        Log(LogLevel::Warning, message, file, line, function);
    }

    void Logger::Error(const std::string& message, const char* file, uint32_t line, const char* function) {
        Log(LogLevel::Error, message, file, line, function);
    }

    void Logger::Fatal(const std::string& message, const char* file, uint32_t line, const char* function) {
        Log(LogLevel::Fatal, message, file, line, function);
    }

    void Logger::AddSink(std::shared_ptr<LogSink> sink) {
        std::lock_guard<std::mutex> lock(m_sinksMutex);
        m_sinks.push_back(sink);
    }

    void Logger::RemoveSink(const std::string& sinkName) {
        std::lock_guard<std::mutex> lock(m_sinksMutex);
        m_sinks.erase(
            std::remove_if(m_sinks.begin(), m_sinks.end(),
                [&sinkName](const std::shared_ptr<LogSink>& sink) {
                    return sink->GetName() == sinkName;
                }
            ),
            m_sinks.end()
        );
    }

    void Logger::ClearSinks() {
        std::lock_guard<std::mutex> lock(m_sinksMutex);
        for (auto& sink : m_sinks) {
            sink->Flush();
        }
        m_sinks.clear();
    }

    LogStatistics Logger::GetStatistics() const {
        LogStatistics stats;
        stats.totalMessages = m_messageCount;
        stats.droppedMessages = m_droppedCount;

        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - m_startTime).count();
        stats.messagesPerSecond = elapsed > 0 ? stats.totalMessages / elapsed : 0;

        stats.activeLoggers = 1; // This logger
        stats.memoryUsage = sizeof(Logger) + m_sinks.size() * sizeof(std::shared_ptr<LogSink>);

        return stats;
    }

    void Logger::ResetStatistics() {
        m_messageCount = 0;
        m_droppedCount = 0;
        m_startTime = std::chrono::steady_clock::now();
    }

    void Logger::Log(LogLevel level, const std::string& message, const char* file, uint32_t line, const char* function) {
        if (!m_enabled || !ShouldLog(level)) {
            m_droppedCount++;
            return;
        }

        static std::atomic<uint32_t> sequenceNumber{0};

        LogMessage logMessage{
            level,
            m_name,
            message,
            file ? std::string(file) : "",
            line,
            function ? std::string(function) : "",
            std::chrono::system_clock::now(),
            std::this_thread::get_id(),
            sequenceNumber++
        };

        m_messageCount++;
        WriteToSinks(logMessage);
    }

    std::string Logger::FormatMessage(const LogMessage& message) const {
        std::stringstream ss;

        // Add timestamp
        if (m_includeTimestamp) {
            auto time = std::chrono::system_clock::to_time_t(message.timestamp);
            auto tm = *std::localtime(&time);
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                message.timestamp.time_since_epoch()
            ).count() % 1000;

            ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S")
               << "." << std::setfill('0') << std::setw(3) << ms << " ";
        }

        // Add log level
        switch (message.level) {
            case LogLevel::Trace: ss << "[TRACE] "; break;
            case LogLevel::Debug: ss << "[DEBUG] "; break;
            case LogLevel::Info: ss << "[INFO] "; break;
            case LogLevel::Warning: ss << "[WARN] "; break;
            case LogLevel::Error: ss << "[ERROR] "; break;
            case LogLevel::Fatal: ss << "[FATAL] "; break;
            default: ss << "[UNKNOWN] "; break;
        }

        // Add logger name
        if (m_includeLoggerName) {
            ss << "[" << message.logger << "] ";
        }

        // Add thread ID
        if (m_includeThreadId) {
            ss << "[Thread-" << message.threadId << "] ";
        }

        // Add source location
        if (m_includeSourceLocation && !message.file.empty()) {
            ss << "[" << message.file << ":" << message.line << " in " << message.function << "] ";
        }

        // Add message
        ss << message.message;

        return ss.str();
    }

    bool Logger::ShouldLog(LogLevel level) const {
        return static_cast<int>(level) >= static_cast<int>(m_level);
    }

    void Logger::WriteToSinks(const LogMessage& message) {
        std::lock_guard<std::mutex> lock(m_sinksMutex);
        for (auto& sink : m_sinks) {
            if (sink->ShouldHandle(message.level)) {
                sink->Write(message);
            }
        }
    }

    // LogManager implementation

    LogManager& LogManager::GetInstance() {
        static LogManager instance;
        return instance;
    }

    bool LogManager::Initialize(const std::string& configPath) {
        auto& instance = GetInstance();

        // Create default logger
        auto consoleSink = std::make_shared<ConsoleSink>(true, true);
        auto logger = std::make_shared<Logger>("VoxelCraft", LogLevel::Info);
        logger->AddSink(consoleSink);

        instance.m_loggers["VoxelCraft"] = logger;

        // Set up global sinks
        instance.m_globalSinks.push_back(consoleSink);

        return true;
    }

    void LogManager::Shutdown() {
        auto& instance = GetInstance();
        instance.m_loggers.clear();
        instance.m_globalSinks.clear();
    }

    std::shared_ptr<Logger> LogManager::GetLogger(const std::string& name) {
        auto& instance = GetInstance();

        std::lock_guard<std::mutex> lock(instance.m_mutex);
        auto it = instance.m_loggers.find(name);
        if (it != instance.m_loggers.end()) {
            return it->second;
        }

        // Create new logger with default settings
        auto logger = std::make_shared<Logger>(name, instance.m_globalLevel);
        for (auto& sink : instance.m_globalSinks) {
            logger->AddSink(sink);
        }

        instance.m_loggers[name] = logger;
        return logger;
    }

    std::shared_ptr<Logger> LogManager::CreateLogger(
        const std::string& name,
        LogLevel level,
        const std::vector<std::shared_ptr<LogSink>>& sinks
    ) {
        auto& instance = GetInstance();
        std::lock_guard<std::mutex> lock(instance.m_mutex);

        auto logger = std::make_shared<Logger>(name, level);
        for (auto& sink : sinks) {
            logger->AddSink(sink);
        }

        instance.m_loggers[name] = logger;
        return logger;
    }

    void LogManager::SetGlobalLevel(LogLevel level) {
        auto& instance = GetInstance();
        instance.m_globalLevel = level;

        std::lock_guard<std::mutex> lock(instance.m_mutex);
        for (auto& pair : instance.m_loggers) {
            pair.second->SetLevel(level);
        }
    }

    LogLevel LogManager::GetGlobalLevel() {
        return GetInstance().m_globalLevel;
    }

    LogStatistics LogManager::GetStatistics() {
        auto& instance = GetInstance();
        std::lock_guard<std::mutex> lock(instance.m_mutex);

        LogStatistics totalStats;
        for (auto& pair : instance.m_loggers) {
            auto stats = pair.second->GetStatistics();
            totalStats.totalMessages += stats.totalMessages;
            totalStats.droppedMessages += stats.droppedMessages;
            totalStats.activeLoggers += stats.activeLoggers;
            totalStats.memoryUsage += stats.memoryUsage;
        }

        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - instance.m_startTime).count();
        totalStats.messagesPerSecond = elapsed > 0 ? totalStats.totalMessages / elapsed : 0;

        return totalStats;
    }

    void LogManager::FlushAll() {
        auto& instance = GetInstance();
        std::lock_guard<std::mutex> lock(instance.m_mutex);

        for (auto& pair : instance.m_loggers) {
            pair.second->ClearSinks(); // This flushes all sinks
        }
    }

} // namespace VoxelCraft
