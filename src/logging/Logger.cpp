/**
 * @file Logger.cpp
 * @brief VoxelCraft Advanced Logging System Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#include "Logger.hpp"

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
#include <regex>

#include "../utils/Random.hpp"

namespace VoxelCraft {

    // Static instance
    static Logger* s_instance = nullptr;

    // Console colors (ANSI escape codes)
    #ifdef _WIN32
        #define CONSOLE_COLOR_RESET ""
        #define CONSOLE_COLOR_TRACE ""
        #define CONSOLE_COLOR_DEBUG ""
        #define CONSOLE_COLOR_INFO ""
        #define CONSOLE_COLOR_WARNING ""
        #define CONSOLE_COLOR_ERROR ""
        #define CONSOLE_COLOR_CRITICAL ""
    #else
        #define CONSOLE_COLOR_RESET "\033[0m"
        #define CONSOLE_COLOR_TRACE "\033[90m"
        #define CONSOLE_COLOR_DEBUG "\033[36m"
        #define CONSOLE_COLOR_INFO "\033[32m"
        #define CONSOLE_COLOR_WARNING "\033[33m"
        #define CONSOLE_COLOR_ERROR "\033[31m"
        #define CONSOLE_COLOR_CRITICAL "\033[35m"
    #endif

    // LogSink implementations
    LogSink::LogSink(LogSinkType type, const std::string& name)
        : m_type(type), m_name(name) {
    }

    LogSink::~LogSink() = default;

    bool LogSink::ShouldWrite(const LogEntry& entry) const {
        for (const auto& filter : m_filters) {
            if (filter->ShouldFilter(entry)) {
                return false;
            }
        }
        return true;
    }

    std::string LogSink::FormatEntry(const LogEntry& entry) const {
        if (m_formatter) {
            return m_formatter->Format(entry);
        }
        return entry.message;
    }

    // ConsoleSink implementation
    ConsoleSink::ConsoleSink(const std::string& name)
        : LogSink(LogSinkType::CONSOLE, name) {
    }

    ConsoleSink::~ConsoleSink() = default;

    bool ConsoleSink::Initialize(const LogConfig& config) {
        m_useColors = true;  // Could be configurable
        m_useStderr = false;
        return true;
    }

    void ConsoleSink::Shutdown() {
        Flush();
    }

    bool ConsoleSink::Write(const LogEntry& entry) {
        if (!m_enabled || !ShouldWrite(entry)) {
            return true;
        }

        std::string formattedMessage = FormatEntry(entry);
        std::ostream& output = m_useStderr ? std::cerr : std::cout;

        if (m_useColors) {
            std::string colorCode;
            switch (entry.level) {
                case LogLevel::TRACE: colorCode = CONSOLE_COLOR_TRACE; break;
                case LogLevel::DEBUG: colorCode = CONSOLE_COLOR_DEBUG; break;
                case LogLevel::INFO: colorCode = CONSOLE_COLOR_INFO; break;
                case LogLevel::WARNING: colorCode = CONSOLE_COLOR_WARNING; break;
                case LogLevel::ERROR: colorCode = CONSOLE_COLOR_ERROR; break;
                case LogLevel::CRITICAL: colorCode = CONSOLE_COLOR_CRITICAL; break;
                default: colorCode = ""; break;
            }

            output << colorCode << formattedMessage << CONSOLE_COLOR_RESET << std::endl;
        } else {
            output << formattedMessage << std::endl;
        }

        return true;
    }

    bool ConsoleSink::Flush() {
        std::cout.flush();
        std::cerr.flush();
        return true;
    }

    // FileSink implementation
    FileSink::FileSink(const std::string& name)
        : LogSink(LogSinkType::FILE, name) {
    }

    FileSink::~FileSink() {
        Shutdown();
    }

    bool FileSink::Initialize(const LogConfig& config) {
        m_logDirectory = config.logDirectory;
        m_baseFilename = config.logFilename;

        // Create log directory if it doesn't exist
        std::filesystem::create_directories(m_logDirectory);

        // Set current filename
        m_currentFilename = m_logDirectory + "/" + m_baseFilename;

        // Open file stream
        m_fileStream.open(m_currentFilename, std::ios::app | std::ios::out);
        if (!m_fileStream.is_open()) {
            std::cerr << "Failed to open log file: " << m_currentFilename << std::endl;
            return false;
        }

        // Set rotation settings
        m_rotationType = config.rotationType;
        m_maxFileSize = config.maxFileSize;
        m_maxBackupFiles = config.maxBackupFiles;

        m_lastRotation = std::chrono::system_clock::now();

        return true;
    }

    void FileSink::Shutdown() {
        if (m_fileStream.is_open()) {
            m_fileStream.flush();
            m_fileStream.close();
        }
    }

    bool FileSink::Write(const LogEntry& entry) {
        if (!m_enabled || !m_fileStream.is_open() || !ShouldWrite(entry)) {
            return true;
        }

        std::string formattedMessage = FormatEntry(entry);
        formattedMessage += "\n";

        // Check if rotation is needed
        if (m_rotationType == LogRotationType::SIZE) {
            m_currentFileSize += formattedMessage.size();
            if (m_currentFileSize >= m_maxFileSize) {
                RotateFile();
            }
        }

        m_fileStream << formattedMessage;
        m_fileStream.flush();

        return true;
    }

    bool FileSink::Flush() {
        if (m_fileStream.is_open()) {
            m_fileStream.flush();
        }
        return true;
    }

    void FileSink::SetRotation(LogRotationType type, uint64_t maxSize) {
        m_rotationType = type;
        m_maxFileSize = maxSize;
    }

    void FileSink::SetCompression(LogCompression compression) {
        m_compression = compression;
    }

    bool FileSink::RotateFile() {
        if (!m_fileStream.is_open()) {
            return false;
        }

        // Close current file
        m_fileStream.close();

        // Generate rotated filename
        std::string rotatedFilename = GetRotatedFilename();

        // Rename current file
        try {
            std::filesystem::rename(m_currentFilename, rotatedFilename);
        } catch (const std::filesystem::filesystem_error& e) {
            std::cerr << "Failed to rotate log file: " << e.what() << std::endl;
            return false;
        }

        // Compress old file if needed
        if (m_compression != LogCompression::NONE) {
            CompressFile(rotatedFilename);
        }

        // Clean up old backup files
        CleanupOldBackups();

        // Open new file
        m_fileStream.open(m_currentFilename, std::ios::out | std::ios::trunc);
        if (!m_fileStream.is_open()) {
            std::cerr << "Failed to create new log file: " << m_currentFilename << std::endl;
            return false;
        }

        m_currentFileSize = 0;
        m_lastRotation = std::chrono::system_clock::now();

        return true;
    }

    bool FileSink::CompressFile(const std::string& filename) {
        // This would implement file compression
        // For now, just return true
        return true;
    }

    std::string FileSink::GetRotatedFilename() const {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);

        std::stringstream ss;
        ss << m_logDirectory << "/" << m_baseFilename << ".";
        ss << std::put_time(std::localtime(&time), "%Y%m%d_%H%M%S");

        if (m_compression == LogCompression::GZIP) {
            ss << ".gz";
        } else if (m_compression == LogCompression::ZIP) {
            ss << ".zip";
        }

        return ss.str();
    }

    void FileSink::CleanupOldBackups() {
        // This would implement cleanup of old backup files
        // For now, do nothing
    }

    // NetworkSink implementation
    NetworkSink::NetworkSink(const std::string& name)
        : LogSink(LogSinkType::NETWORK, name) {
    }

    NetworkSink::~NetworkSink() {
        Shutdown();
    }

    bool NetworkSink::Initialize(const LogConfig& config) {
        m_host = config.networkHost;
        m_port = config.networkPort;
        m_protocol = "udp";  // Default to UDP

        // This would initialize network connection
        // For now, just set connected to false
        m_connected = false;

        return true;
    }

    void NetworkSink::Shutdown() {
        if (m_socket != -1) {
            // Close socket
            m_socket = -1;
        }
        m_connected = false;
    }

    bool NetworkSink::Write(const LogEntry& entry) {
        if (!m_enabled || !m_connected || !ShouldWrite(entry)) {
            return true;
        }

        std::string formattedMessage = FormatEntry(entry);

        // This would send message over network
        // For now, just return true
        return true;
    }

    bool NetworkSink::Flush() {
        return true;
    }

    // LogFormatter implementations
    LogFormatter::LogFormatter(LogFormat format)
        : m_format(format) {
    }

    LogFormatter::~LogFormatter() = default;

    std::string LogFormatter::ReplacePlaceholders(const std::string& pattern, const LogEntry& entry) const {
        std::string result = pattern;

        // Replace placeholders
        std::unordered_map<std::string, std::string> replacements = {
            {"%timestamp%", GetTimestampString(entry.timestamp)},
            {"%level%", GetLevelString(entry.level)},
            {"%category%", entry.category},
            {"%message%", entry.message},
            {"%logger%", entry.loggerName},
            {"%thread%", GetThreadIdString(entry.threadId)},
            {"%file%", entry.file},
            {"%line%", std::to_string(entry.line)},
            {"%function%", entry.function}
        };

        for (const auto& replacement : replacements) {
            size_t pos = 0;
            while ((pos = result.find(replacement.first, pos)) != std::string::npos) {
                result.replace(pos, replacement.first.length(), replacement.second);
                pos += replacement.second.length();
            }
        }

        return result;
    }

    std::string LogFormatter::GetTimestampString(const std::chrono::system_clock::time_point& timestamp) const {
        auto time = std::chrono::system_clock::to_time_t(timestamp);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }

    std::string LogFormatter::GetLevelString(LogLevel level) const {
        switch (level) {
            case LogLevel::TRACE: return "TRACE";
            case LogLevel::DEBUG: return "DEBUG";
            case LogLevel::INFO: return "INFO";
            case LogLevel::WARNING: return "WARNING";
            case LogLevel::ERROR: return "ERROR";
            case LogLevel::CRITICAL: return "CRITICAL";
            default: return "UNKNOWN";
        }
    }

    std::string LogFormatter::GetThreadIdString(std::thread::id threadId) const {
        std::stringstream ss;
        ss << threadId;
        return ss.str();
    }

    // PlainTextFormatter implementation
    PlainTextFormatter::PlainTextFormatter()
        : LogFormatter(LogFormat::PLAIN_TEXT) {
        m_pattern = "[%timestamp%] [%level%] [%category%] %message%";
    }

    PlainTextFormatter::~PlainTextFormatter() = default;

    std::string PlainTextFormatter::Format(const LogEntry& entry) {
        return ReplacePlaceholders(m_pattern, entry);
    }

    // JSONFormatter implementation
    JSONFormatter::JSONFormatter()
        : LogFormatter(LogFormat::JSON) {
    }

    JSONFormatter::~JSONFormatter() = default;

    std::string JSONFormatter::Format(const LogEntry& entry) {
        std::stringstream ss;
        ss << "{";
        ss << "\"timestamp\":\"" << GetTimestampString(entry.timestamp) << "\",";
        ss << "\"level\":\"" << GetLevelString(entry.level) << "\",";
        ss << "\"category\":\"" << entry.category << "\",";
        ss << "\"message\":\"" << EscapeJsonString(entry.message) << "\",";
        ss << "\"logger\":\"" << entry.loggerName << "\",";
        ss << "\"thread\":\"" << GetThreadIdString(entry.threadId) << "\",";
        ss << "\"file\":\"" << entry.file << "\",";
        ss << "\"line\":" << entry.line << ",";
        ss << "\"function\":\"" << entry.function << "\"";
        ss << "}";
        return ss.str();
    }

    std::string JSONFormatter::EscapeJsonString(const std::string& str) const {
        std::string result;
        for (char c : str) {
            switch (c) {
                case '"': result += "\\\""; break;
                case '\\': result += "\\\\"; break;
                case '\b': result += "\\b"; break;
                case '\f': result += "\\f"; break;
                case '\n': result += "\\n"; break;
                case '\r': result += "\\r"; break;
                case '\t': result += "\\t"; break;
                default:
                    if (c < 32 || c > 126) {
                        char buffer[7];
                        std::snprintf(buffer, sizeof(buffer), "\\u%04x", c);
                        result += buffer;
                    } else {
                        result += c;
                    }
                    break;
            }
        }
        return result;
    }

    // LogFilter implementations
    LogFilter::LogFilter(const std::string& name)
        : m_name(name) {
    }

    LogFilter::~LogFilter() = default;

    // LevelFilter implementation
    LevelFilter::LevelFilter(LogLevel minLevel)
        : LogFilter("LevelFilter"), m_minLevel(minLevel) {
    }

    LevelFilter::~LevelFilter() = default;

    bool LevelFilter::ShouldFilter(const LogEntry& entry) {
        return static_cast<int>(entry.level) < static_cast<int>(m_minLevel);
    }

    // CategoryFilter implementation
    CategoryFilter::CategoryFilter(const std::vector<std::string>& categories, bool include)
        : LogFilter("CategoryFilter"), m_categories(categories), m_include(include) {
    }

    CategoryFilter::~CategoryFilter() = default;

    bool CategoryFilter::ShouldFilter(const LogEntry& entry) {
        bool found = std::find(m_categories.begin(), m_categories.end(), entry.category) != m_categories.end();

        if (m_include) {
            return !found;  // Filter out if not in list
        } else {
            return found;   // Filter out if in list
        }
    }

    // Logger implementation
    Logger& Logger::GetInstance() {
        if (!s_instance) {
            s_instance = new Logger();
        }
        return *s_instance;
    }

    bool Logger::Initialize(const LogConfig& config) {
        if (m_initialized) {
            std::cerr << "Logger already initialized" << std::endl;
            return true;
        }

        m_config = config;
        m_nextEntryId = 1;
        m_shutdownWorkers = false;

        try {
            // Initialize default formatter
            m_defaultFormatter = std::make_shared<PlainTextFormatter>();

            // Initialize default sinks
            InitializeDefaultSinks();

            // Initialize async processing if enabled
            if (m_config.enableAsyncLogging) {
                InitializeAsyncProcessing();
            }

            m_initialized = true;
            LogInternal(CreateLogEntry(LogLevel::INFO, "Logger initialized successfully", "Logger", {}));

            return true;

        } catch (const std::exception& e) {
            std::cerr << "Failed to initialize Logger: " << e.what() << std::endl;
            return false;
        }
    }

    void Logger::Shutdown() {
        if (!m_initialized) {
            return;
        }

        // Shutdown async processing
        if (m_config.enableAsyncLogging) {
            ShutdownAsyncProcessing();
        }

        // Flush all sinks
        Flush();

        // Shutdown all sinks
        for (auto& pair : m_sinks) {
            pair.second->Shutdown();
        }
        m_sinks.clear();

        m_initialized = false;
        std::cout << "Logger shutdown" << std::endl;
    }

    void Logger::Update(float deltaTime) {
        if (!m_initialized) {
            return;
        }

        // Update statistics
        UpdatePerformanceStats();

        // Process any queued entries if not using async
        if (!m_config.enableAsyncLogging) {
            ProcessQueuedEntries();
        }
    }

    // Core Logging API
    void Logger::Trace(const std::string& message, const std::string& category,
                      const std::unordered_map<std::string, std::string>& context) {
        Log(LogLevel::TRACE, message, category, context);
    }

    void Logger::Debug(const std::string& message, const std::string& category,
                      const std::unordered_map<std::string, std::string>& context) {
        Log(LogLevel::DEBUG, message, category, context);
    }

    void Logger::Info(const std::string& message, const std::string& category,
                     const std::unordered_map<std::string, std::string>& context) {
        Log(LogLevel::INFO, message, category, context);
    }

    void Logger::Warning(const std::string& message, const std::string& category,
                        const std::unordered_map<std::string, std::string>& context) {
        Log(LogLevel::WARNING, message, category, context);
    }

    void Logger::Error(const std::string& message, const std::string& category,
                      const std::unordered_map<std::string, std::string>& context) {
        Log(LogLevel::ERROR, message, category, context);
    }

    void Logger::Critical(const std::string& message, const std::string& category,
                         const std::unordered_map<std::string, std::string>& context) {
        Log(LogLevel::CRITICAL, message, category, context);
    }

    void Logger::Log(LogLevel level, const std::string& message, const std::string& category,
                    const std::unordered_map<std::string, std::string>& context) {
        if (!m_initialized || !ShouldLog(level, category)) {
            return;
        }

        LogEntry entry = CreateLogEntry(level, message, category, context);
        LogInternal(entry);
    }

    void Logger::LogInternal(const LogEntry& entry) {
        if (m_config.enableAsyncLogging) {
            // Add to queue for async processing
            {
                std::unique_lock<std::mutex> lock(m_queueMutex);
                if (m_entryQueue.size() >= m_config.maxQueueSize) {
                    m_stats.queueOverflows++;
                    m_entryQueue.pop_front();  // Remove oldest entry
                }
                m_entryQueue.push_back(entry);
            }
            m_queueCondition.notify_one();
        } else {
            // Process immediately
            ProcessLogEntry(entry);
        }
    }

    void Logger::ProcessLogEntry(const LogEntry& entry) {
        auto startTime = std::chrono::steady_clock::now();

        // Write to all enabled sinks
        WriteToSinks(entry);

        // Add to history
        {
            std::unique_lock<std::shared_mutex> lock(m_historyMutex);
            m_entryHistory.push_back(entry);
            if (m_entryHistory.size() > 10000) {  // Keep last 10k entries in memory
                m_entryHistory.pop_front();
            }
        }

        // Update statistics
        UpdateStatistics(entry);

        auto endTime = std::chrono::steady_clock::now();
        float processingTime = std::chrono::duration<float>(endTime - startTime).count() * 1000.0f;

        // Update performance stats
        {
            std::unique_lock<std::shared_mutex> lock(m_statsMutex);
            m_stats.averageProcessingTime = (m_stats.averageProcessingTime + processingTime) * 0.5f;
            m_stats.maxProcessingTime = std::max(m_stats.maxProcessingTime, processingTime);
        }
    }

    LogEntry Logger::CreateLogEntry(LogLevel level, const std::string& message, const std::string& category,
                                   const std::unordered_map<std::string, std::string>& context) {
        LogEntry entry;
        entry.id = GenerateEntryId();
        entry.level = level;
        entry.message = message;
        entry.category = category.empty() ? "Default" : category;
        entry.loggerName = "MainLogger";
        entry.timestamp = std::chrono::system_clock::now();
        entry.threadId = std::this_thread::get_id();
        entry.processId = 0;  // Would get actual process ID
        entry.file = "";      // Would be filled by macro
        entry.line = 0;       // Would be filled by macro
        entry.function = "";  // Would be filled by macro
        entry.metadata = context;
        entry.userData = nullptr;

        return entry;
    }

    uint64_t Logger::GenerateEntryId() {
        return m_nextEntryId++;
    }

    // Async processing
    void Logger::InitializeAsyncProcessing() {
        for (uint32_t i = 0; i < m_config.workerThreads; ++i) {
            m_workerThreads.emplace_back(&Logger::AsyncWorkerThread, this);
        }
    }

    void Logger::ShutdownAsyncProcessing() {
        {
            std::unique_lock<std::mutex> lock(m_queueMutex);
            m_shutdownWorkers = true;
        }
        m_queueCondition.notify_all();

        // Wait for worker threads to finish
        for (auto& thread : m_workerThreads) {
            if (thread.joinable()) {
                thread.join();
            }
        }
        m_workerThreads.clear();

        // Process any remaining entries
        ProcessQueuedEntries();
    }

    void Logger::AsyncWorkerThread() {
        while (true) {
            LogEntry entry;

            {
                std::unique_lock<std::mutex> lock(m_queueMutex);
                m_queueCondition.wait(lock, [this]() {
                    return m_shutdownWorkers || !m_entryQueue.empty();
                });

                if (m_shutdownWorkers && m_entryQueue.empty()) {
                    break;
                }

                if (!m_entryQueue.empty()) {
                    entry = m_entryQueue.front();
                    m_entryQueue.pop_front();
                } else {
                    continue;
                }
            }

            // Process the entry
            ProcessLogEntry(entry);
        }
    }

    void Logger::ProcessQueuedEntries() {
        std::unique_lock<std::mutex> lock(m_queueMutex);
        while (!m_entryQueue.empty()) {
            LogEntry entry = m_entryQueue.front();
            m_entryQueue.pop_front();
            lock.unlock();

            ProcessLogEntry(entry);

            lock.lock();
        }
    }

    // Sink management
    void Logger::InitializeDefaultSinks() {
        // Create console sink
        auto consoleSink = std::make_shared<ConsoleSink>("console");
        consoleSink->Initialize(m_config);
        consoleSink->SetFormatter(m_defaultFormatter);
        AddSink(consoleSink);

        // Create file sink if logging to file is enabled
        if (!m_config.logFilename.empty()) {
            auto fileSink = std::make_shared<FileSink>("file");
            fileSink->Initialize(m_config);
            fileSink->SetFormatter(m_defaultFormatter);
            AddSink(fileSink);
        }

        // Create network sink if network logging is enabled
        if (m_config.enableNetworkLogging) {
            auto networkSink = std::make_shared<NetworkSink>("network");
            networkSink->Initialize(m_config);
            networkSink->SetFormatter(m_defaultFormatter);
            AddSink(networkSink);
        }
    }

    bool Logger::AddSink(std::shared_ptr<LogSink> sink) {
        if (!sink) {
            return false;
        }

        std::unique_lock<std::shared_mutex> lock(m_statsMutex);
        m_sinks[sink->GetName()] = sink;
        return true;
    }

    bool Logger::RemoveSink(const std::string& name) {
        std::unique_lock<std::shared_mutex> lock(m_statsMutex);

        auto it = m_sinks.find(name);
        if (it == m_sinks.end()) {
            return false;
        }

        it->second->Shutdown();
        m_sinks.erase(it);
        return true;
    }

    std::shared_ptr<LogSink> Logger::GetSink(const std::string& name) {
        std::shared_lock<std::shared_mutex> lock(m_statsMutex);

        auto it = m_sinks.find(name);
        return it != m_sinks.end() ? it->second : nullptr;
    }

    bool Logger::SetSinkEnabled(const std::string& name, bool enabled) {
        auto sink = GetSink(name);
        if (sink) {
            sink->SetEnabled(enabled);
            return true;
        }
        return false;
    }

    void Logger::WriteToSinks(const LogEntry& entry) {
        std::shared_lock<std::shared_mutex> lock(m_statsMutex);

        for (const auto& pair : m_sinks) {
            try {
                pair.second->Write(entry);
            } catch (const std::exception& e) {
                // Log sink error to stderr
                std::cerr << "Log sink '" << pair.first << "' error: " << e.what() << std::endl;
                m_stats.sinkErrors++;
            }
        }
    }

    void Logger::FlushSinks() {
        std::shared_lock<std::shared_mutex> lock(m_statsMutex);

        for (const auto& pair : m_sinks) {
            try {
                pair.second->Flush();
            } catch (const std::exception& e) {
                std::cerr << "Log sink '" << pair.first << "' flush error: " << e.what() << std::endl;
            }
        }
    }

    // Category management
    void Logger::SetCategoryEnabled(const std::string& category, bool enabled) {
        if (enabled) {
            auto it = std::find(m_config.disabledCategories.begin(), m_config.disabledCategories.end(), category);
            if (it != m_config.disabledCategories.end()) {
                m_config.disabledCategories.erase(it);
            }
        } else {
            if (std::find(m_config.disabledCategories.begin(), m_config.disabledCategories.end(), category) ==
                m_config.disabledCategories.end()) {
                m_config.disabledCategories.push_back(category);
            }
        }
    }

    // Query and analysis
    std::vector<LogEntry> Logger::QueryLogs(const std::chrono::system_clock::time_point& startTime,
                                           const std::chrono::system_clock::time_point& endTime,
                                           LogLevel level, const std::string& category, uint32_t limit) const {
        std::vector<LogEntry> results;
        std::shared_lock<std::shared_mutex> lock(m_historyMutex);

        for (const auto& entry : m_entryHistory) {
            if (entry.timestamp >= startTime && entry.timestamp <= endTime &&
                static_cast<int>(entry.level) >= static_cast<int>(level) &&
                (category.empty() || entry.category == category)) {

                results.push_back(entry);

                if (results.size() >= limit) {
                    break;
                }
            }
        }

        return results;
    }

    LogStats Logger::GetLogStats(const std::chrono::system_clock::time_point& startTime,
                               const std::chrono::system_clock::time_point& endTime) const {
        LogStats stats = {};
        std::shared_lock<std::shared_mutex> lock(m_historyMutex);

        for (const auto& entry : m_entryHistory) {
            if (entry.timestamp >= startTime && entry.timestamp <= endTime) {
                stats.totalEntries++;
                stats.entriesByLevel[entry.level]++;
                stats.entriesByCategory[entry.category]++;
            }
        }

        stats.entriesPerSecond = stats.totalEntries / 60.0;  // Approximate
        return stats;
    }

    std::vector<LogEntry> Logger::SearchLogs(const std::string& query, bool caseSensitive, uint32_t limit) const {
        std::vector<LogEntry> results;
        std::shared_lock<std::shared_mutex> lock(m_historyMutex);

        std::regex::flag_type flags = std::regex::extended;
        if (!caseSensitive) {
            flags |= std::regex::icase;
        }

        try {
            std::regex searchRegex(query, flags);

            for (const auto& entry : m_entryHistory) {
                if (std::regex_search(entry.message, searchRegex) ||
                    std::regex_search(entry.category, searchRegex)) {

                    results.push_back(entry);

                    if (results.size() >= limit) {
                        break;
                    }
                }
            }
        } catch (const std::regex_error& e) {
            // Invalid regex, do simple substring search
            for (const auto& entry : m_entryHistory) {
                bool matches = caseSensitive ?
                    (entry.message.find(query) != std::string::npos ||
                     entry.category.find(query) != std::string::npos) :
                    (entry.message.find(query) != std::string::npos ||
                     entry.category.find(query) != std::string::npos);  // Case insensitive would need boost or custom implementation

                if (matches) {
                    results.push_back(entry);

                    if (results.size() >= limit) {
                        break;
                    }
                }
            }
        }

        return results;
    }

    bool Logger::ExportLogs(const std::string& filename,
                           const std::chrono::system_clock::time_point& startTime,
                           const std::chrono::system_clock::time_point& endTime,
                           LogFormat format) {
        try {
            std::ofstream file(filename);
            if (!file.is_open()) {
                return false;
            }

            auto entries = QueryLogs(startTime, endTime, LogLevel::TRACE, "", UINT32_MAX);

            std::shared_ptr<LogFormatter> formatter;
            switch (format) {
                case LogFormat::JSON:
                    formatter = std::make_shared<JSONFormatter>();
                    break;
                default:
                    formatter = std::make_shared<PlainTextFormatter>();
                    break;
            }

            for (const auto& entry : entries) {
                file << formatter->Format(entry) << std::endl;
            }

            file.close();
            return true;

        } catch (const std::exception& e) {
            std::cerr << "Failed to export logs: " << e.what() << std::endl;
            return false;
        }
    }

    // Performance measurement
    uint64_t Logger::StartPerformanceMeasurement(const std::string& name) {
        static std::atomic<uint64_t> measurementId = 1;
        uint64_t id = measurementId++;

        {
            std::unique_lock<std::shared_mutex> lock(m_performanceMutex);
            m_activeMeasurements[id] = std::chrono::steady_clock::now();
        }

        return id;
    }

    uint64_t Logger::EndPerformanceMeasurement(uint64_t id) {
        std::unique_lock<std::shared_mutex> lock(m_performanceMutex);

        auto it = m_activeMeasurements.find(id);
        if (it == m_activeMeasurements.end()) {
            return 0;
        }

        auto endTime = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - it->second);

        m_activeMeasurements.erase(it);
        return duration.count();
    }

    std::unordered_map<std::string, double> Logger::GetPerformanceStats() const {
        std::unordered_map<std::string, double> stats;
        std::shared_lock<std::shared_mutex> lock(m_performanceMutex);

        // This would aggregate performance measurements
        return stats;
    }

    // Utility methods
    void Logger::Flush() {
        if (m_config.enableAsyncLogging) {
            ProcessQueuedEntries();
        }
        FlushSinks();
    }

    void Logger::ClearHistory() {
        std::unique_lock<std::shared_mutex> lock(m_historyMutex);
        m_entryHistory.clear();
    }

    std::string Logger::GetSystemInfo() const {
        std::stringstream ss;
        ss << "Logger System Info:\n";
        ss << "Initialized: " << (m_initialized ? "Yes" : "No") << "\n";
        ss << "Async Logging: " << (m_config.enableAsyncLogging ? "Enabled" : "Disabled") << "\n";
        ss << "Sinks: " << m_sinks.size() << "\n";
        ss << "History Size: " << m_entryHistory.size() << "\n";
        ss << "Queue Size: " << m_entryQueue.size() << "\n";
        ss << "Active Measurements: " << m_activeMeasurements.size() << "\n";
        return ss.str();
    }

    bool Logger::Validate() const {
        bool valid = true;

        if (!m_initialized) {
            std::cerr << "Logger validation failed: Not initialized" << std::endl;
            valid = false;
        }

        if (m_sinks.empty()) {
            std::cerr << "Logger validation failed: No sinks configured" << std::endl;
            valid = false;
        }

        return valid;
    }

    void Logger::UpdateStatistics(const LogEntry& entry) {
        std::unique_lock<std::shared_mutex> lock(m_statsMutex);

        m_stats.totalEntries++;
        m_stats.entriesByLevel[entry.level]++;
        m_stats.entriesByCategory[entry.category]++;

        m_stats.queueSize = m_entryQueue.size();
        m_stats.maxQueueSize = std::max(m_stats.maxQueueSize, m_stats.queueSize);
    }

    void Logger::UpdatePerformanceStats() {
        std::unique_lock<std::shared_mutex> lock(m_statsMutex);

        // Update entries per second (rough estimate)
        static uint64_t lastEntryCount = 0;
        static auto lastUpdate = std::chrono::steady_clock::now();

        auto now = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration<float>(now - lastUpdate).count();

        if (duration >= 1.0f) {
            m_stats.entriesPerSecond = (m_stats.totalEntries - lastEntryCount) / duration;
            lastEntryCount = m_stats.totalEntries;
            lastUpdate = now;
        }
    }

    bool Logger::ShouldLog(LogLevel level, const std::string& category) const {
        // Check if level is enabled
        if (static_cast<int>(level) < static_cast<int>(m_config.defaultLevel)) {
            return false;
        }

        // Check if category is disabled
        if (std::find(m_config.disabledCategories.begin(), m_config.disabledCategories.end(), category) !=
            m_config.disabledCategories.end()) {
            return false;
        }

        return true;
    }

    void Logger::RotateLogsIfNeeded() {
        // This would implement log rotation based on configuration
        // For now, do nothing
    }

} // namespace VoxelCraft
