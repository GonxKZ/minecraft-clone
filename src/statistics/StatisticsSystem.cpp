/**
 * @file StatisticsSystem.cpp
 * @brief VoxelCraft Advanced Statistics System Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#include "StatisticsSystem.hpp"

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
#include <random>
#include <cmath>

#include "../utils/Random.hpp"
#include "../logging/Logger.hpp"
#include "../event/EventSystem.hpp"

namespace VoxelCraft {

    // StatisticsSystem implementation
    StatisticsSystem& StatisticsSystem::GetInstance() {
        static StatisticsSystem instance;
        return instance;
    }

    StatisticsSystem::StatisticsSystem()
        : m_playerId("default_player"),
          m_autoSave(true),
          m_cloudSyncEnabled(false),
          m_maxHistoryDays(30),
          m_autoSaveInterval(std::chrono::seconds(300)), // 5 minutes
          m_totalUpdates(0),
          m_totalDataPoints(0) {
        m_systemStartTime = std::chrono::steady_clock::now();
    }

    StatisticsSystem::~StatisticsSystem() {
        Shutdown();
    }

    bool StatisticsSystem::Initialize() {
        if (!System::Initialize()) {
            return false;
        }

        Logger::GetInstance().Info("StatisticsSystem initializing", "StatisticsSystem");

        // Load statistic definitions
        if (!LoadStatisticDefinitions()) {
            Logger::GetInstance().Warning("Failed to load statistic definitions", "StatisticsSystem");
        }

        // Initialize default statistics
        InitializeDefaultStatistics();

        // Load saved statistics
        LoadStatistics();

        // Register event handlers
        RegisterEventHandlers();

        Logger::GetInstance().Info("StatisticsSystem initialized successfully", "StatisticsSystem");
        return true;
    }

    void StatisticsSystem::Update(float deltaTime) {
        System::Update(deltaTime);

        // Auto-save statistics
        if (m_autoSave) {
            auto now = std::chrono::steady_clock::now();
            if (now - m_lastAutoSave >= m_autoSaveInterval) {
                SaveStatistics();
                m_lastAutoSave = now;
            }
        }

        // Cleanup old data periodically
        static float cleanupTimer = 0.0f;
        cleanupTimer += deltaTime;
        if (cleanupTimer >= 60.0f) { // Every minute
            CleanupExpiredHistory();
            cleanupTimer = 0.0f;
        }

        // Aggregate statistics periodically
        static float aggregationTimer = 0.0f;
        aggregationTimer += deltaTime;
        if (aggregationTimer >= 300.0f) { // Every 5 minutes
            AggregateStatistics();
            aggregationTimer = 0.0f;
        }
    }

    void StatisticsSystem::Shutdown() {
        if (!System::Shutdown()) {
            return;
        }

        Logger::GetInstance().Info("StatisticsSystem shutting down", "StatisticsSystem");

        // Save statistics
        SaveStatistics();

        // Clear data
        {
            std::unique_lock<std::shared_mutex> lock(m_definitionsMutex);
            m_statisticDefinitions.clear();
        }

        {
            std::unique_lock<std::shared_mutex> lock(m_valuesMutex);
            m_statisticValues.clear();
        }

        {
            std::unique_lock<std::shared_mutex> lock(m_historyMutex);
            m_statisticHistory.clear();
        }

        {
            std::unique_lock<std::shared_mutex> lock(m_dashboardsMutex);
            m_dashboards.clear();
        }

        Logger::GetInstance().Info("StatisticsSystem shutdown complete", "StatisticsSystem");
    }

    bool StatisticsSystem::RegisterStatistic(const StatisticsDefinition& definition) {
        std::unique_lock<std::shared_mutex> lock(m_definitionsMutex);

        if (m_statisticDefinitions.find(definition.id) != m_statisticDefinitions.end()) {
            return false; // Already registered
        }

        m_statisticDefinitions[definition.id] = definition;

        // Initialize value
        StatisticsValue value;
        value.type = definition.dataType;
        value.value = GetDefaultValueForType(definition.dataType);
        value.lastUpdated = std::chrono::system_clock::now();
        value.firstRecorded = value.lastUpdated;
        value.updateCount = 0;
        value.isValid = true;

        {
            std::unique_lock<std::shared_mutex> valueLock(m_valuesMutex);
            m_statisticValues[definition.id] = value;
        }

        // Initialize history
        {
            std::unique_lock<std::shared_mutex> historyLock(m_historyMutex);
            m_statisticHistory[definition.id] = std::vector<StatisticsDataPoint>();
        }

        Logger::GetInstance().Debug("Registered statistic: " + definition.name, "StatisticsSystem");
        return true;
    }

    bool StatisticsSystem::UnregisterStatistic(const std::string& statisticId) {
        std::unique_lock<std::shared_mutex> lock(m_definitionsMutex);

        auto defIt = m_statisticDefinitions.find(statisticId);
        if (defIt == m_statisticDefinitions.end()) {
            return false;
        }

        m_statisticDefinitions.erase(defIt);

        {
            std::unique_lock<std::shared_mutex> valueLock(m_valuesMutex);
            m_statisticValues.erase(statisticId);
        }

        {
            std::unique_lock<std::shared_mutex> historyLock(m_historyMutex);
            m_statisticHistory.erase(statisticId);
        }

        Logger::GetInstance().Debug("Unregistered statistic: " + statisticId, "StatisticsSystem");
        return true;
    }

    std::shared_ptr<StatisticsDefinition> StatisticsSystem::GetStatisticDefinition(const std::string& statisticId) const {
        std::shared_lock<std::shared_mutex> lock(m_definitionsMutex);

        auto it = m_statisticDefinitions.find(statisticId);
        if (it != m_statisticDefinitions.end()) {
            return std::make_shared<StatisticsDefinition>(it->second);
        }

        return nullptr;
    }

    bool StatisticsSystem::UpdateStatistic(const std::string& statisticId, const std::any& value,
                                         const std::unordered_map<std::string, std::any>& metadata) {
        std::shared_lock<std::shared_mutex> defLock(m_definitionsMutex);

        auto defIt = m_statisticDefinitions.find(statisticId);
        if (defIt == m_statisticDefinitions.end()) {
            return false;
        }

        auto& definition = defIt->second;
        defLock.unlock();

        // Validate value type
        if (!ValidateValueType(value, definition.dataType)) {
            Logger::GetInstance().Warning("Invalid value type for statistic: " + statisticId, "StatisticsSystem");
            return false;
        }

        // Create data point
        StatisticsDataPoint dataPoint;
        dataPoint.statisticId = statisticId;
        dataPoint.timestamp = std::chrono::system_clock::now();
        dataPoint.value = ConvertValueType(value, definition.dataType);
        dataPoint.metadata = metadata;

        // Process data point
        ProcessDataPoint(dataPoint);

        m_totalUpdates++;
        return true;
    }

    StatisticsValue StatisticsSystem::GetStatisticValue(const std::string& statisticId) const {
        std::shared_lock<std::shared_mutex> lock(m_valuesMutex);

        auto it = m_statisticValues.find(statisticId);
        if (it != m_statisticValues.end()) {
            return it->second;
        }

        return StatisticsValue();
    }

    bool StatisticsSystem::ResetStatistic(const std::string& statisticId) {
        std::shared_lock<std::shared_mutex> defLock(m_definitionsMutex);

        auto defIt = m_statisticDefinitions.find(statisticId);
        if (defIt == m_statisticDefinitions.end()) {
            return false;
        }

        auto& definition = defIt->second;
        defLock.unlock();

        StatisticsValue newValue;
        newValue.type = definition.dataType;
        newValue.value = GetDefaultValueForType(definition.dataType);
        newValue.lastUpdated = std::chrono::system_clock::now();
        newValue.firstRecorded = newValue.lastUpdated;
        newValue.updateCount = 0;
        newValue.isValid = true;

        {
            std::unique_lock<std::shared_mutex> valueLock(m_valuesMutex);
            m_statisticValues[statisticId] = newValue;
        }

        {
            std::unique_lock<std::shared_mutex> historyLock(m_historyMutex);
            m_statisticHistory[statisticId].clear();
        }

        return true;
    }

    bool StatisticsSystem::UpdateMultipleStatistics(const std::unordered_map<std::string, std::any>& updates) {
        bool allSuccess = true;

        for (const auto& update : updates) {
            if (!UpdateStatistic(update.first, update.second)) {
                allSuccess = false;
            }
        }

        return allSuccess;
    }

    std::unordered_map<std::string, StatisticsValue> StatisticsSystem::GetMultipleStatisticValues(const std::vector<std::string>& statisticIds) const {
        std::unordered_map<std::string, StatisticsValue> result;
        std::shared_lock<std::shared_mutex> lock(m_valuesMutex);

        for (const auto& statisticId : statisticIds) {
            auto it = m_statisticValues.find(statisticId);
            if (it != m_statisticValues.end()) {
                result[statisticId] = it->second;
            }
        }

        return result;
    }

    std::vector<std::string> StatisticsSystem::GetAllStatisticIds() const {
        std::shared_lock<std::shared_mutex> lock(m_definitionsMutex);

        std::vector<std::string> ids;
        for (const auto& pair : m_statisticDefinitions) {
            ids.push_back(pair.first);
        }

        return ids;
    }

    std::vector<std::string> StatisticsSystem::GetStatisticIdsByCategory(const std::string& category) const {
        std::shared_lock<std::shared_mutex> lock(m_definitionsMutex);

        std::vector<std::string> ids;
        for (const auto& pair : m_statisticDefinitions) {
            if (pair.second.category == category) {
                ids.push_back(pair.first);
            }
        }

        return ids;
    }

    std::vector<std::string> StatisticsSystem::GetStatisticIdsByType(StatisticsCategory type) const {
        std::shared_lock<std::shared_mutex> lock(m_definitionsMutex);

        std::vector<std::string> ids;
        for (const auto& pair : m_statisticDefinitions) {
            if (pair.second.statCategory == type) {
                ids.push_back(pair.first);
            }
        }

        return ids;
    }

    StatisticsValue StatisticsSystem::GetStatisticValueAtTime(const std::string& statisticId,
                                                            const std::chrono::system_clock::time_point& time) const {
        std::shared_lock<std::shared_mutex> historyLock(m_historyMutex);

        auto historyIt = m_statisticHistory.find(statisticId);
        if (historyIt == m_statisticHistory.end() || historyIt->second.empty()) {
            return StatisticsValue();
        }

        const auto& history = historyIt->second;

        // Find closest data point
        auto it = std::lower_bound(history.begin(), history.end(), time,
            [](const StatisticsDataPoint& point, const std::chrono::system_clock::time_point& t) {
                return point.timestamp < t;
            });

        if (it == history.end()) {
            return StatisticsValue();
        }

        // Convert data point to value
        StatisticsValue value;
        value.type = GetDataTypeForStatistic(statisticId);
        value.value = it->value;
        value.lastUpdated = it->timestamp;
        value.firstRecorded = it->timestamp;
        value.updateCount = 1;
        value.isValid = true;

        return value;
    }

    std::vector<StatisticsDataPoint> StatisticsSystem::GetStatisticHistory(const std::string& statisticId,
                                                                         const std::chrono::system_clock::time_point& startTime,
                                                                         const std::chrono::system_clock::time_point& endTime) const {
        std::shared_lock<std::shared_mutex> historyLock(m_historyMutex);

        auto historyIt = m_statisticHistory.find(statisticId);
        if (historyIt == m_statisticHistory.end()) {
            return {};
        }

        const auto& history = historyIt->second;
        std::vector<StatisticsDataPoint> result;

        for (const auto& point : history) {
            if (point.timestamp >= startTime && point.timestamp <= endTime) {
                result.push_back(point);
            }
        }

        return result;
    }

    std::unordered_map<StatisticsAggregation, double> StatisticsSystem::GetStatisticAggregations(const std::string& statisticId,
                                                                                               const std::chrono::system_clock::time_point& startTime,
                                                                                               const std::chrono::system_clock::time_point& endTime) const {
        auto history = GetStatisticHistory(statisticId, startTime, endTime);
        std::unordered_map<StatisticsAggregation, double> aggregations;

        if (history.empty()) {
            return aggregations;
        }

        // Calculate basic aggregations
        double sum = 0.0;
        double min = std::numeric_limits<double>::max();
        double max = std::numeric_limits<double>::lowest();
        std::vector<double> values;

        for (const auto& point : history) {
            double value = ExtractDoubleValue(point.value);
            values.push_back(value);
            sum += value;
            min = std::min(min, value);
            max = std::max(max, value);
        }

        aggregations[StatisticsAggregation::SUM] = sum;
        aggregations[StatisticsAggregation::COUNT] = static_cast<double>(history.size());
        aggregations[StatisticsAggregation::MINIMUM] = min;
        aggregations[StatisticsAggregation::MAXIMUM] = max;
        aggregations[StatisticsAggregation::AVERAGE] = sum / history.size();

        // Calculate median
        std::sort(values.begin(), values.end());
        if (values.size() % 2 == 0) {
            aggregations[StatisticsAggregation::MEDIAN] = (values[values.size() / 2 - 1] + values[values.size() / 2]) / 2.0;
        } else {
            aggregations[StatisticsAggregation::MEDIAN] = values[values.size() / 2];
        }

        return aggregations;
    }

    StatisticsTrend StatisticsSystem::AnalyzeStatisticTrend(const std::string& statisticId, StatisticsTimePeriod period) const {
        StatisticsTrend trend;
        trend.statisticId = statisticId;
        trend.period = period;

        auto currentValue = GetStatisticValue(statisticId);
        trend.currentValue = ExtractDoubleValue(currentValue.value);

        // Calculate time range
        auto now = std::chrono::system_clock::now();
        auto periodStart = GetPeriodStartTime(now, period);

        // Get previous period
        auto previousPeriodEnd = periodStart;
        auto previousPeriodStart = GetPeriodStartTime(previousPeriodEnd, period);

        // Get values for both periods
        auto currentHistory = GetStatisticHistory(statisticId, periodStart, now);
        auto previousHistory = GetStatisticHistory(statisticId, previousPeriodStart, previousPeriodEnd);

        if (!currentHistory.empty()) {
            trend.currentValue = ExtractDoubleValue(currentHistory.back().value);
        }

        if (!previousHistory.empty()) {
            trend.previousValue = ExtractDoubleValue(previousHistory.back().value);
        }

        // Calculate trend
        trend.changeAbsolute = trend.currentValue - trend.previousValue;
        if (trend.previousValue != 0.0) {
            trend.changePercentage = (trend.changeAbsolute / trend.previousValue) * 100.0;
        }

        if (trend.changeAbsolute > 0.001) {
            trend.trend = "increasing";
        } else if (trend.changeAbsolute < -0.001) {
            trend.trend = "decreasing";
        } else {
            trend.trend = "stable";
        }

        // Calculate rate of change
        auto periodDays = std::chrono::duration_cast<std::chrono::hours>(now - periodStart).count() / 24.0;
        if (periodDays > 0) {
            trend.rateOfChange = trend.changeAbsolute / periodDays;
        }

        trend.analysisTimestamp = now;
        return trend;
    }

    std::vector<StatisticsTrend> StatisticsSystem::AnalyzeMultipleTrends(const std::vector<std::string>& statisticIds,
                                                                       StatisticsTimePeriod period) const {
        std::vector<StatisticsTrend> trends;

        for (const auto& statisticId : statisticIds) {
            trends.push_back(AnalyzeStatisticTrend(statisticId, period));
        }

        return trends;
    }

    StatisticsComparisonData StatisticsSystem::CompareStatistic(const std::string& statisticId, StatisticsComparison comparison) const {
        StatisticsComparisonData comparisonData;
        comparisonData.statisticId = statisticId;
        comparisonData.comparisonType = comparison;

        auto playerValue = GetStatisticValue(statisticId);
        comparisonData.playerValue = ExtractDoubleValue(playerValue.value);

        // This would typically fetch comparison data from cloud/social features
        // For now, generate mock comparison data
        switch (comparison) {
            case StatisticsComparison::FRIENDS_AVERAGE:
                comparisonData.comparisonValue = comparisonData.playerValue * (0.8 + Random::GetInstance().GetFloat() * 0.4);
                comparisonData.comparisonLabel = "Friends Average";
                break;
            case StatisticsComparison::GUILD_AVERAGE:
                comparisonData.comparisonValue = comparisonData.playerValue * (0.9 + Random::GetInstance().GetFloat() * 0.2);
                comparisonData.comparisonLabel = "Guild Average";
                break;
            case StatisticsComparison::WORLD_AVERAGE:
                comparisonData.comparisonValue = comparisonData.playerValue * (0.7 + Random::GetInstance().GetFloat() * 0.6);
                comparisonData.comparisonLabel = "World Average";
                break;
            default:
                comparisonData.comparisonValue = comparisonData.playerValue;
                comparisonData.comparisonLabel = "No Comparison";
                break;
        }

        // Calculate performance
        if (comparisonData.playerValue > comparisonData.comparisonValue * 1.1) {
            comparisonData.performance = "above_average";
        } else if (comparisonData.playerValue < comparisonData.comparisonValue * 0.9) {
            comparisonData.performance = "below_average";
        } else {
            comparisonData.performance = "average";
        }

        return comparisonData;
    }

    std::vector<StatisticsComparisonData> StatisticsSystem::CompareMultipleStatistics(const std::vector<std::string>& statisticIds,
                                                                                    StatisticsComparison comparison) const {
        std::vector<StatisticsComparisonData> comparisons;

        for (const auto& statisticId : statisticIds) {
            comparisons.push_back(CompareStatistic(statisticId, comparison));
        }

        return comparisons;
    }

    StatisticsReport StatisticsSystem::GenerateReport(const std::string& reportName, const std::vector<std::string>& statisticIds,
                                                    StatisticsTimePeriod period) const {
        StatisticsReport report;
        report.reportId = "report_" + std::to_string(Random::GetInstance().GetUint64());
        report.reportName = reportName;
        report.playerId = m_playerId;
        report.generatedTime = std::chrono::system_clock::now();
        report.timePeriod = period;

        // Get values
        report.values = GetMultipleStatisticValues(statisticIds);

        // Generate trends
        for (const auto& statisticId : statisticIds) {
            report.trends[statisticId] = AnalyzeStatisticTrend(statisticId, period);
        }

        // Generate comparisons
        for (const auto& statisticId : statisticIds) {
            report.comparisons[statisticId] = CompareStatistic(statisticId, StatisticsComparison::PERSONAL_BEST);
        }

        return report;
    }

    StatisticsReport StatisticsSystem::GenerateComprehensiveReport(StatisticsTimePeriod period) const {
        auto allIds = GetAllStatisticIds();
        return GenerateReport("Comprehensive Report", allIds, period);
    }

    bool StatisticsSystem::CreateDashboard(const StatisticsDashboard& dashboard) {
        std::unique_lock<std::shared_mutex> lock(m_dashboardsMutex);

        if (m_dashboards.find(dashboard.dashboardId) != m_dashboards.end()) {
            return false;
        }

        m_dashboards[dashboard.dashboardId] = dashboard;
        return true;
    }

    bool StatisticsSystem::UpdateDashboard(const std::string& dashboardId, const StatisticsDashboard& dashboard) {
        std::unique_lock<std::shared_mutex> lock(m_dashboardsMutex);

        auto it = m_dashboards.find(dashboardId);
        if (it == m_dashboards.end()) {
            return false;
        }

        it->second = dashboard;
        return true;
    }

    bool StatisticsSystem::DeleteDashboard(const std::string& dashboardId) {
        std::unique_lock<std::shared_mutex> lock(m_dashboardsMutex);
        return m_dashboards.erase(dashboardId) > 0;
    }

    std::shared_ptr<StatisticsDashboard> StatisticsSystem::GetDashboard(const std::string& dashboardId) const {
        std::shared_lock<std::shared_mutex> lock(m_dashboardsMutex);

        auto it = m_dashboards.find(dashboardId);
        if (it != m_dashboards.end()) {
            return std::make_shared<StatisticsDashboard>(it->second);
        }

        return nullptr;
    }

    std::string StatisticsSystem::GenerateVisualizationData(const std::string& statisticId, StatisticsVisualization type,
                                                          StatisticsTimePeriod period) const {
        // Generate visualization data based on type
        // This would return JSON or other format data for charts/graphs
        return "{}"; // Placeholder
    }

    std::string StatisticsSystem::GenerateDashboardVisualization(const std::string& dashboardId) const {
        // Generate complete dashboard visualization
        return "{}"; // Placeholder
    }

    bool StatisticsSystem::ExportStatistics(const std::vector<std::string>& statisticIds, StatisticsExportFormat format,
                                          const std::string& filename) const {
        // Implement export based on format
        return true;
    }

    bool StatisticsSystem::ImportStatistics(const std::string& filename, StatisticsExportFormat format) {
        // Implement import based on format
        return true;
    }

    bool StatisticsSystem::SyncWithCloud() {
        if (!m_cloudSyncEnabled) {
            return false;
        }

        // Implement cloud synchronization
        return true;
    }

    bool StatisticsSystem::LoadFromCloud() {
        if (!m_cloudSyncEnabled) {
            return false;
        }

        // Implement cloud loading
        return true;
    }

    bool StatisticsSystem::SaveToCloud() {
        if (!m_cloudSyncEnabled) {
            return false;
        }

        // Implement cloud saving
        return true;
    }

    bool StatisticsSystem::IsCloudSyncEnabled() const {
        return m_cloudSyncEnabled;
    }

    double StatisticsSystem::GetStatisticsUpdateRate() const {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - m_systemStartTime).count();

        if (elapsed > 0) {
            return static_cast<double>(m_totalUpdates) / elapsed;
        }

        return 0.0;
    }

    size_t StatisticsSystem::GetMemoryUsage() const {
        size_t memory = 0;

        {
            std::shared_lock<std::shared_mutex> defLock(m_definitionsMutex);
            memory += m_statisticDefinitions.size() * sizeof(StatisticsDefinition);
        }

        {
            std::shared_lock<std::shared_mutex> valueLock(m_valuesMutex);
            memory += m_statisticValues.size() * sizeof(StatisticsValue);
        }

        {
            std::shared_lock<std::shared_mutex> historyLock(m_historyMutex);
            for (const auto& pair : m_statisticHistory) {
                memory += pair.second.size() * sizeof(StatisticsDataPoint);
            }
        }

        return memory;
    }

    uint64_t StatisticsSystem::GetTotalDataPoints() const {
        return m_totalDataPoints;
    }

    uint64_t StatisticsSystem::GetDataPointsPerSecond() const {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - m_systemStartTime).count();

        if (elapsed > 0) {
            return m_totalDataPoints / elapsed;
        }

        return 0;
    }

    void StatisticsSystem::OnEvent(const EventBase& event) {
        // Process event tracking
        ProcessEventTracking(event);
    }

    bool StatisticsSystem::RegisterEventTracking(const std::string& eventName, const std::string& statisticId,
                                               std::function<std::any(const EventBase&)> extractor) {
        std::unique_lock<std::shared_mutex> lock(m_eventTrackingMutex);
        m_eventTracking[eventName].emplace_back(statisticId, extractor);
        return true;
    }

    std::string StatisticsSystem::GetStatisticsInfo() const {
        std::stringstream ss;
        ss << "Statistics System Info:\n";
        ss << "Statistics: " << m_statisticDefinitions.size() << "\n";
        ss << "Values: " << m_statisticValues.size() << "\n";
        ss << "Total Data Points: " << GetTotalDataPoints() << "\n";
        ss << "Memory Usage: " << GetMemoryUsage() << " bytes\n";
        ss << "Update Rate: " << GetStatisticsUpdateRate() << " updates/sec\n";
        ss << "Cloud Sync: " << (m_cloudSyncEnabled ? "Enabled" : "Disabled") << "\n";
        return ss.str();
    }

    void StatisticsSystem::PrintStatisticsReport() const {
        auto report = GenerateComprehensiveReport(StatisticsTimePeriod::ALL_TIME);
        std::stringstream ss;

        ss << "=== Statistics System Report ===\n";
        ss << "Generated: " << std::chrono::system_clock::to_time_t(report.generatedTime) << "\n";
        ss << "Statistics: " << report.values.size() << "\n";
        ss << "Trends: " << report.trends.size() << "\n";
        ss << "Comparisons: " << report.comparisons.size() << "\n";

        Logger::GetInstance().Info(ss.str(), "StatisticsSystem");
    }

    bool StatisticsSystem::ValidateStatistic(const std::string& statisticId) const {
        auto definition = GetStatisticDefinition(statisticId);
        return definition != nullptr;
    }

    bool StatisticsSystem::ValidateAllStatistics() const {
        std::shared_lock<std::shared_mutex> lock(m_definitionsMutex);

        for (const auto& pair : m_statisticDefinitions) {
            if (!ValidateStatistic(pair.first)) {
                return false;
            }
        }

        return true;
    }

    void StatisticsSystem::ResetAllStatistics() {
        auto statisticIds = GetAllStatisticIds();

        for (const auto& statisticId : statisticIds) {
            ResetStatistic(statisticId);
        }
    }

    void StatisticsSystem::CleanupOldData() {
        CleanupExpiredHistory();
    }

    void StatisticsSystem::DebugUpdateStatistic(const std::string& statisticId, const std::any& value) {
        UpdateStatistic(statisticId, value);
    }

    std::vector<std::string> StatisticsSystem::DebugGetTrackedStatistics() const {
        return GetAllStatisticIds();
    }

    bool StatisticsSystem::DebugGenerateSampleData() {
        // Generate sample data for testing
        auto statisticIds = GetAllStatisticIds();

        for (const auto& statisticId : statisticIds) {
            auto definition = GetStatisticDefinition(statisticId);
            if (definition) {
                std::any sampleValue = GenerateSampleValue(definition->dataType);
                UpdateStatistic(statisticId, sampleValue);
            }
        }

        return true;
    }

    // Private methods implementation
    bool StatisticsSystem::LoadStatisticDefinitions() {
        // Load from file or database
        return true;
    }

    bool StatisticsSystem::InitializeDefaultStatistics() {
        // Initialize all statistic categories
        InitializePlayerStatistics();
        InitializeCombatStatistics();
        InitializeCraftingStatistics();
        InitializeExplorationStatistics();
        InitializeSocialStatistics();
        InitializeEconomyStatistics();
        InitializePerformanceStatistics();
        InitializeResourceStatistics();
        InitializeTimeStatistics();
        InitializeDistanceStatistics();
        InitializeDamageStatistics();
        InitializeMagicStatistics();
        InitializeVehicleStatistics();
        InitializePetStatistics();
        InitializeFarmingStatistics();
        InitializeMiningStatistics();
        InitializeFishingStatistics();
        InitializeCookingStatistics();
        InitializeTradingStatistics();
        InitializeBuildingStatistics();
        InitializeQuestStatistics();
        InitializeAchievementStatistics();

        return true;
    }

    void StatisticsSystem::RegisterEventHandlers() {
        // Register with event system
    }

    void StatisticsSystem::ProcessDataPoint(const StatisticsDataPoint& dataPoint) {
        // Update value
        UpdateValueFromDataPoint(dataPoint);

        // Add to history
        {
            std::unique_lock<std::shared_mutex> historyLock(m_historyMutex);
            auto& history = m_statisticHistory[dataPoint.statisticId];
            history.push_back(dataPoint);

            // Keep history within limits
            if (history.size() > 10000) { // Arbitrary limit
                history.erase(history.begin());
            }
        }

        m_totalDataPoints++;
    }

    void StatisticsSystem::UpdateValueFromDataPoint(const StatisticsDataPoint& dataPoint) {
        std::unique_lock<std::shared_mutex> lock(m_valuesMutex);

        auto& value = m_statisticValues[dataPoint.statisticId];
        value.value = dataPoint.value;
        value.lastUpdated = dataPoint.timestamp;
        value.updateCount++;

        if (value.updateCount == 1) {
            value.firstRecorded = dataPoint.timestamp;
        }
    }

    bool StatisticsSystem::SaveStatistics() {
        // Save to file
        return true;
    }

    bool StatisticsSystem::LoadStatistics() {
        // Load from file
        return true;
    }

    void StatisticsSystem::CleanupExpiredHistory() {
        auto cutoff = std::chrono::system_clock::now() - std::chrono::hours(24 * m_maxHistoryDays);

        std::unique_lock<std::shared_mutex> historyLock(m_historyMutex);

        for (auto& pair : m_statisticHistory) {
            auto& history = pair.second;
            history.erase(std::remove_if(history.begin(), history.end(),
                [cutoff](const StatisticsDataPoint& point) {
                    return point.timestamp < cutoff;
                }), history.end());
        }
    }

    void StatisticsSystem::AggregateStatistics() {
        // Perform periodic aggregation
    }

    std::any StatisticsSystem::ConvertValueType(const std::any& value, StatisticsDataType targetType) const {
        // Convert between types
        return value;
    }

    bool StatisticsSystem::ValidateValueType(const std::any& value, StatisticsDataType expectedType) const {
        // Validate type matches expectation
        return true;
    }

    std::any StatisticsSystem::GetDefaultValueForType(StatisticsDataType type) const {
        switch (type) {
            case StatisticsDataType::INTEGER: return int64_t(0);
            case StatisticsDataType::FLOAT: return double(0.0);
            case StatisticsDataType::BOOLEAN: return false;
            case StatisticsDataType::TIME: return std::chrono::milliseconds(0);
            case StatisticsDataType::DISTANCE: return float(0.0f);
            case StatisticsDataType::VELOCITY: return float(0.0f);
            case StatisticsDataType::COUNT: return int64_t(0);
            case StatisticsDataType::PERCENTAGE: return double(0.0);
            case StatisticsDataType::RATIO: return double(0.0);
            case StatisticsDataType::VECTOR2: return glm::vec2(0.0f);
            case StatisticsDataType::VECTOR3: return glm::vec3(0.0f);
            case StatisticsDataType::STRING: return std::string("");
            case StatisticsDataType::TIMESTAMP: return std::chrono::system_clock::now();
            case StatisticsDataType::DURATION: return std::chrono::milliseconds(0);
            case StatisticsDataType::FREQUENCY: return double(0.0);
            default: return std::any();
        }
    }

    std::chrono::system_clock::time_point StatisticsSystem::GetPeriodStartTime(const std::chrono::system_clock::time_point& endTime,
                                                                             StatisticsTimePeriod period) const {
        switch (period) {
            case StatisticsTimePeriod::TODAY:
                return std::chrono::floor<std::chrono::days>(endTime) - std::chrono::days(1);
            case StatisticsTimePeriod::THIS_WEEK:
                return endTime - std::chrono::weeks(1);
            case StatisticsTimePeriod::THIS_MONTH:
                return endTime - std::chrono::months(1);
            case StatisticsTimePeriod::THIS_YEAR:
                return endTime - std::chrono::years(1);
            case StatisticsTimePeriod::ALL_TIME:
                return std::chrono::system_clock::time_point::min();
            default:
                return endTime - std::chrono::hours(1);
        }
    }

    StatisticsDataType StatisticsSystem::GetDataTypeForStatistic(const std::string& statisticId) const {
        auto definition = GetStatisticDefinition(statisticId);
        return definition ? definition->dataType : StatisticsDataType::INTEGER;
    }

    double StatisticsSystem::ExtractDoubleValue(const std::variant<int64_t, double, bool, std::chrono::milliseconds, float, glm::vec2, glm::vec3, std::string>& value) const {
        if (std::holds_alternative<int64_t>(value)) return static_cast<double>(std::get<int64_t>(value));
        if (std::holds_alternative<double>(value)) return std::get<double>(value);
        if (std::holds_alternative<bool>(value)) return std::get<bool>(value) ? 1.0 : 0.0;
        if (std::holds_alternative<std::chrono::milliseconds>(value)) return static_cast<double>(std::get<std::chrono::milliseconds>(value).count());
        if (std::holds_alternative<float>(value)) return static_cast<double>(std::get<float>(value));
        if (std::holds_alternative<glm::vec2>(value)) return static_cast<double>(glm::length(std::get<glm::vec2>(value)));
        if (std::holds_alternative<glm::vec3>(value)) return static_cast<double>(glm::length(std::get<glm::vec3>(value)));
        return 0.0;
    }

    std::any StatisticsSystem::GenerateSampleValue(StatisticsDataType type) const {
        switch (type) {
            case StatisticsDataType::INTEGER: return int64_t(Random::GetInstance().GetInt(0, 1000));
            case StatisticsDataType::FLOAT: return Random::GetInstance().GetFloat() * 100.0;
            case StatisticsDataType::BOOLEAN: return Random::GetInstance().GetBool();
            case StatisticsDataType::TIME: return std::chrono::milliseconds(Random::GetInstance().GetInt(0, 3600000));
            case StatisticsDataType::DISTANCE: return Random::GetInstance().GetFloat() * 1000.0f;
            case StatisticsDataType::VELOCITY: return Random::GetInstance().GetFloat() * 50.0f;
            case StatisticsDataType::COUNT: return int64_t(Random::GetInstance().GetInt(0, 10000));
            case StatisticsDataType::PERCENTAGE: return Random::GetInstance().GetFloat() * 100.0;
            case StatisticsDataType::RATIO: return Random::GetInstance().GetFloat() * 10.0;
            case StatisticsDataType::VECTOR2: return glm::vec2(Random::GetInstance().GetFloat() * 100.0f);
            case StatisticsDataType::VECTOR3: return glm::vec3(Random::GetInstance().GetFloat() * 100.0f);
            case StatisticsDataType::STRING: return std::string("sample_value");
            case StatisticsDataType::TIMESTAMP: return std::chrono::system_clock::now();
            case StatisticsDataType::DURATION: return std::chrono::milliseconds(Random::GetInstance().GetInt(0, 86400000));
            case StatisticsDataType::FREQUENCY: return Random::GetInstance().GetFloat() * 60.0;
            default: return std::any();
        }
    }

    void StatisticsSystem::ProcessEventTracking(const EventBase& event) {
        std::shared_lock<std::shared_mutex> lock(m_eventTrackingMutex);

        auto it = m_eventTracking.find(event.GetTypeName());
        if (it != m_eventTracking.end()) {
            for (const auto& tracking : it->second) {
                try {
                    std::any value = tracking.second(event);
                    UpdateStatistic(tracking.first, value);
                } catch (const std::exception& e) {
                    Logger::GetInstance().Warning("Exception in event tracking for statistic '" + tracking.first + "': " + e.what(), "StatisticsSystem");
                }
            }
        }
    }

    // Statistics category initialization methods
    void StatisticsSystem::InitializePlayerStatistics() {
        // Player activity
        RegisterStatistic({"player_time_played", "Time Played", "Total time spent playing", "Player", StatisticsCategory::PLAYER, StatisticsDataType::TIME, "seconds", "%d", true, true, {}, 0});
        RegisterStatistic({"player_sessions_started", "Sessions Started", "Number of game sessions started", "Player", StatisticsCategory::PLAYER, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"player_distance_traveled", "Distance Traveled", "Total distance traveled", "Player", StatisticsCategory::PLAYER, StatisticsDataType::DISTANCE, "meters", "%.2f", true, true, {}, 0});
        RegisterStatistic({"player_jumps_made", "Jumps Made", "Total number of jumps", "Player", StatisticsCategory::PLAYER, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"player_falls_suffered", "Falls Suffered", "Total number of falls", "Player", StatisticsCategory::PLAYER, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"player_deaths", "Deaths", "Total number of deaths", "Player", StatisticsCategory::PLAYER, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"player_revives", "Revives", "Total number of revives", "Player", StatisticsCategory::PLAYER, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"player_level", "Level", "Current player level", "Player", StatisticsCategory::PLAYER, StatisticsDataType::INTEGER, "", "%d", true, true, {}, 0});
        RegisterStatistic({"player_experience", "Experience", "Total experience earned", "Player", StatisticsCategory::PLAYER, StatisticsDataType::INTEGER, "", "%d", true, true, {}, 0});
        RegisterStatistic({"player_skill_points", "Skill Points", "Available skill points", "Player", StatisticsCategory::PLAYER, StatisticsDataType::INTEGER, "", "%d", true, true, {}, 0});
        RegisterStatistic({"player_attribute_points", "Attribute Points", "Available attribute points", "Player", StatisticsCategory::PLAYER, StatisticsDataType::INTEGER, "", "%d", true, true, {}, 0});
        RegisterStatistic({"player_health_max", "Max Health", "Maximum health", "Player", StatisticsCategory::PLAYER, StatisticsDataType::FLOAT, "", "%.1f", true, true, {}, 0});
        RegisterStatistic({"player_stamina_max", "Max Stamina", "Maximum stamina", "Player", StatisticsCategory::PLAYER, StatisticsDataType::FLOAT, "", "%.1f", true, true, {}, 0});
        RegisterStatistic({"player_mana_max", "Max Mana", "Maximum mana", "Player", StatisticsCategory::PLAYER, StatisticsDataType::FLOAT, "", "%.1f", true, true, {}, 0});
        RegisterStatistic({"player_strength", "Strength", "Strength attribute", "Player", StatisticsCategory::PLAYER, StatisticsDataType::INTEGER, "", "%d", true, true, {}, 0});
        RegisterStatistic({"player_agility", "Agility", "Agility attribute", "Player", StatisticsCategory::PLAYER, StatisticsDataType::INTEGER, "", "%d", true, true, {}, 0});
        RegisterStatistic({"player_intelligence", "Intelligence", "Intelligence attribute", "Player", StatisticsCategory::PLAYER, StatisticsDataType::INTEGER, "", "%d", true, true, {}, 0});
        RegisterStatistic({"player_charisma", "Charisma", "Charisma attribute", "Player", StatisticsCategory::PLAYER, StatisticsDataType::INTEGER, "", "%d", true, true, {}, 0});
        RegisterStatistic({"player_luck", "Luck", "Luck attribute", "Player", StatisticsCategory::PLAYER, StatisticsDataType::INTEGER, "", "%d", true, true, {}, 0});
    }

    void StatisticsSystem::InitializeCombatStatistics() {
        // Combat statistics
        RegisterStatistic({"combat_enemies_defeated", "Enemies Defeated", "Total enemies defeated", "Combat", StatisticsCategory::COMBAT, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"combat_damage_dealt", "Damage Dealt", "Total damage dealt", "Combat", StatisticsCategory::COMBAT, StatisticsDataType::FLOAT, "", "%.1f", true, true, {}, 0});
        RegisterStatistic({"combat_damage_received", "Damage Received", "Total damage received", "Combat", StatisticsCategory::COMBAT, StatisticsDataType::FLOAT, "", "%.1f", true, true, {}, 0});
        RegisterStatistic({"combat_healing_done", "Healing Done", "Total healing done", "Combat", StatisticsCategory::COMBAT, StatisticsDataType::FLOAT, "", "%.1f", true, true, {}, 0});
        RegisterStatistic({"combat_healing_received", "Healing Received", "Total healing received", "Combat", StatisticsCategory::COMBAT, StatisticsDataType::FLOAT, "", "%.1f", true, true, {}, 0});
        RegisterStatistic({"combat_critical_hits", "Critical Hits", "Total critical hits", "Combat", StatisticsCategory::COMBAT, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"combat_kills_streak", "Kill Streak", "Current kill streak", "Combat", StatisticsCategory::COMBAT, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"combat_deaths_streak", "Death Streak", "Current death streak", "Combat", StatisticsCategory::COMBAT, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"combat_weapons_used", "Weapons Used", "Number of different weapons used", "Combat", StatisticsCategory::COMBAT, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"combat_spells_cast", "Spells Cast", "Total spells cast", "Combat", StatisticsCategory::COMBAT, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"combat_abilities_used", "Abilities Used", "Total abilities used", "Combat", StatisticsCategory::COMBAT, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"combat_pvp_kills", "PvP Kills", "Player vs Player kills", "Combat", StatisticsCategory::COMBAT, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"combat_pvp_deaths", "PvP Deaths", "Player vs Player deaths", "Combat", StatisticsCategory::COMBAT, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"combat_pve_kills", "PvE Kills", "Player vs Environment kills", "Combat", StatisticsCategory::COMBAT, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"combat_boss_kills", "Boss Kills", "Boss enemies defeated", "Combat", StatisticsCategory::COMBAT, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"combat_dungeon_completes", "Dungeon Completes", "Dungeons completed", "Combat", StatisticsCategory::COMBAT, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
    }

    void StatisticsSystem::InitializeCraftingStatistics() {
        // Crafting statistics
        RegisterStatistic({"crafting_items_crafted", "Items Crafted", "Total items crafted", "Crafting", StatisticsCategory::CRAFTING, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"crafting_recipes_learned", "Recipes Learned", "Total recipes learned", "Crafting", StatisticsCategory::CRAFTING, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"crafting_recipes_mastered", "Recipes Mastered", "Recipes mastered", "Crafting", StatisticsCategory::CRAFTING, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"crafting_materials_gathered", "Materials Gathered", "Total materials gathered", "Crafting", StatisticsCategory::CRAFTING, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"crafting_fails", "Crafting Fails", "Total crafting failures", "Crafting", StatisticsCategory::CRAFTING, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"crafting_success_rate", "Success Rate", "Crafting success rate", "Crafting", StatisticsCategory::CRAFTING, StatisticsDataType::PERCENTAGE, "%", "%.1f", true, true, {}, 0});
        RegisterStatistic({"crafting_time_spent", "Time Spent Crafting", "Total time spent crafting", "Crafting", StatisticsCategory::CRAFTING, StatisticsDataType::TIME, "seconds", "%d", true, true, {}, 0});
        RegisterStatistic({"crafting_rare_items_crafted", "Rare Items Crafted", "Rare items crafted", "Crafting", StatisticsCategory::CRAFTING, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"crafting_legendary_items_crafted", "Legendary Items Crafted", "Legendary items crafted", "Crafting", StatisticsCategory::CRAFTING, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"crafting_enchantments_applied", "Enchantments Applied", "Total enchantments applied", "Crafting", StatisticsCategory::CRAFTING, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
    }

    void StatisticsSystem::InitializeExplorationStatistics() {
        // Exploration statistics
        RegisterStatistic({"exploration_biomes_discovered", "Biomes Discovered", "Number of biomes discovered", "Exploration", StatisticsCategory::EXPLORATION, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"exploration_locations_discovered", "Locations Discovered", "Number of locations discovered", "Exploration", StatisticsCategory::EXPLORATION, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"exploration_secrets_found", "Secrets Found", "Number of secrets found", "Exploration", StatisticsCategory::EXPLORATION, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"exploration_mountains_climbed", "Mountains Climbed", "Number of mountains climbed", "Exploration", StatisticsCategory::EXPLORATION, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"exploration_caves_explored", "Caves Explored", "Number of caves explored", "Exploration", StatisticsCategory::EXPLORATION, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"exploration_underwater_time", "Underwater Time", "Time spent underwater", "Exploration", StatisticsCategory::EXPLORATION, StatisticsDataType::TIME, "seconds", "%d", true, true, {}, 0});
        RegisterStatistic({"exploration_flight_time", "Flight Time", "Time spent flying", "Exploration", StatisticsCategory::EXPLORATION, StatisticsDataType::TIME, "seconds", "%d", true, true, {}, 0});
        RegisterStatistic({"exploration_fast_travel_uses", "Fast Travel Uses", "Number of fast travel uses", "Exploration", StatisticsCategory::EXPLORATION, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"exploration_teleports_used", "Teleports Used", "Number of teleports used", "Exploration", StatisticsCategory::EXPLORATION, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"exploration_worlds_visited", "Worlds Visited", "Number of different worlds visited", "Exploration", StatisticsCategory::EXPLORATION, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
    }

    void StatisticsSystem::InitializeSocialStatistics() {
        // Social statistics
        RegisterStatistic({"social_friends_added", "Friends Added", "Number of friends added", "Social", StatisticsCategory::SOCIAL, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"social_messages_sent", "Messages Sent", "Total messages sent", "Social", StatisticsCategory::SOCIAL, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"social_messages_received", "Messages Received", "Total messages received", "Social", StatisticsCategory::SOCIAL, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"social_guild_joined", "Guild Joined", "Whether player has joined a guild", "Social", StatisticsCategory::SOCIAL, StatisticsDataType::BOOLEAN, "", "%s", true, true, {}, 0});
        RegisterStatistic({"social_guild_rank", "Guild Rank", "Current guild rank", "Social", StatisticsCategory::SOCIAL, StatisticsDataType::INTEGER, "", "%d", true, true, {}, 0});
        RegisterStatistic({"social_guild_contribution", "Guild Contribution", "Total guild contribution", "Social", StatisticsCategory::SOCIAL, StatisticsDataType::INTEGER, "", "%d", true, true, {}, 0});
        RegisterStatistic({"social_players_helped", "Players Helped", "Number of players helped", "Social", StatisticsCategory::SOCIAL, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"social_quests_shared", "Quests Shared", "Number of quests shared", "Social", StatisticsCategory::SOCIAL, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"social_trades_completed", "Trades Completed", "Number of trades completed", "Social", StatisticsCategory::SOCIAL, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"social_duels_won", "Duels Won", "Number of duels won", "Social", StatisticsCategory::SOCIAL, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
    }

    void StatisticsSystem::InitializeEconomyStatistics() {
        // Economy statistics
        RegisterStatistic({"economy_currency_earned", "Currency Earned", "Total currency earned", "Economy", StatisticsCategory::ECONOMY, StatisticsDataType::INTEGER, "", "%d", true, true, {}, 0});
        RegisterStatistic({"economy_currency_spent", "Currency Spent", "Total currency spent", "Economy", StatisticsCategory::ECONOMY, StatisticsDataType::INTEGER, "", "%d", true, true, {}, 0});
        RegisterStatistic({"economy_items_sold", "Items Sold", "Total items sold", "Economy", StatisticsCategory::ECONOMY, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"economy_items_bought", "Items Bought", "Total items bought", "Economy", StatisticsCategory::ECONOMY, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"economy_auctions_created", "Auctions Created", "Number of auctions created", "Economy", StatisticsCategory::ECONOMY, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"economy_auctions_won", "Auctions Won", "Number of auctions won", "Economy", StatisticsCategory::ECONOMY, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"economy_market_transactions", "Market Transactions", "Total market transactions", "Economy", StatisticsCategory::ECONOMY, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"economy_bank_balance", "Bank Balance", "Current bank balance", "Economy", StatisticsCategory::ECONOMY, StatisticsDataType::INTEGER, "", "%d", true, true, {}, 0});
        RegisterStatistic({"economy_investments_made", "Investments Made", "Number of investments made", "Economy", StatisticsCategory::ECONOMY, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"economy_loans_taken", "Loans Taken", "Number of loans taken", "Economy", StatisticsCategory::ECONOMY, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
    }

    void StatisticsSystem::InitializePerformanceStatistics() {
        // Performance statistics
        RegisterStatistic({"performance_avg_fps", "Average FPS", "Average frames per second", "Performance", StatisticsCategory::PERFORMANCE, StatisticsDataType::FLOAT, "fps", "%.1f", true, true, {}, 0});
        RegisterStatistic({"performance_min_fps", "Minimum FPS", "Minimum frames per second", "Performance", StatisticsCategory::PERFORMANCE, StatisticsDataType::FLOAT, "fps", "%.1f", true, true, {}, 0});
        RegisterStatistic({"performance_max_fps", "Maximum FPS", "Maximum frames per second", "Performance", StatisticsCategory::PERFORMANCE, StatisticsDataType::FLOAT, "fps", "%.1f", true, true, {}, 0});
        RegisterStatistic({"performance_frame_drops", "Frame Drops", "Total frame drops", "Performance", StatisticsCategory::PERFORMANCE, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"performance_memory_peak", "Peak Memory Usage", "Peak memory usage", "Performance", StatisticsCategory::PERFORMANCE, StatisticsDataType::INTEGER, "MB", "%d", true, true, {}, 0});
        RegisterStatistic({"performance_loading_time", "Loading Time", "Total loading time", "Performance", StatisticsCategory::PERFORMANCE, StatisticsDataType::TIME, "seconds", "%d", true, true, {}, 0});
        RegisterStatistic({"performance_crashes", "Crashes", "Number of crashes", "Performance", StatisticsCategory::PERFORMANCE, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"performance_freezes", "Freezes", "Number of freezes", "Performance", StatisticsCategory::PERFORMANCE, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"performance_input_lag", "Input Lag", "Average input lag", "Performance", StatisticsCategory::PERFORMANCE, StatisticsDataType::FLOAT, "ms", "%.2f", true, true, {}, 0});
        RegisterStatistic({"performance_network_latency", "Network Latency", "Average network latency", "Performance", StatisticsCategory::PERFORMANCE, StatisticsDataType::FLOAT, "ms", "%.2f", true, true, {}, 0});
    }

    void StatisticsSystem::InitializeResourceStatistics() {
        // Resource statistics
        RegisterStatistic({"resource_wood_gathered", "Wood Gathered", "Total wood gathered", "Resources", StatisticsCategory::RESOURCE, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"resource_stone_gathered", "Stone Gathered", "Total stone gathered", "Resources", StatisticsCategory::RESOURCE, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"resource_iron_gathered", "Iron Gathered", "Total iron gathered", "Resources", StatisticsCategory::RESOURCE, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"resource_gold_gathered", "Gold Gathered", "Total gold gathered", "Resources", StatisticsCategory::RESOURCE, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"resource_diamond_gathered", "Diamond Gathered", "Total diamond gathered", "Resources", StatisticsCategory::RESOURCE, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"resource_food_consumed", "Food Consumed", "Total food consumed", "Resources", StatisticsCategory::RESOURCE, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"resource_water_consumed", "Water Consumed", "Total water consumed", "Resources", StatisticsCategory::RESOURCE, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"resource_potions_used", "Potions Used", "Total potions used", "Resources", StatisticsCategory::RESOURCE, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"resource_scrolls_used", "Scrolls Used", "Total scrolls used", "Resources", StatisticsCategory::RESOURCE, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"resource_ammo_used", "Ammo Used", "Total ammunition used", "Resources", StatisticsCategory::RESOURCE, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
    }

    void StatisticsSystem::InitializeTimeStatistics() {
        // Time statistics
        RegisterStatistic({"time_day_played", "Time Played (Day)", "Time played during day", "Time", StatisticsCategory::TIME, StatisticsDataType::TIME, "seconds", "%d", true, true, {}, 0});
        RegisterStatistic({"time_night_played", "Time Played (Night)", "Time played during night", "Time", StatisticsCategory::TIME, StatisticsDataType::TIME, "seconds", "%d", true, true, {}, 0});
        RegisterStatistic({"time_morning_played", "Time Played (Morning)", "Time played during morning", "Time", StatisticsCategory::TIME, StatisticsDataType::TIME, "seconds", "%d", true, true, {}, 0});
        RegisterStatistic({"time_evening_played", "Time Played (Evening)", "Time played during evening", "Time", StatisticsCategory::TIME, StatisticsDataType::TIME, "seconds", "%d", true, true, {}, 0});
        RegisterStatistic({"time_spring_played", "Time Played (Spring)", "Time played during spring", "Time", StatisticsCategory::TIME, StatisticsDataType::TIME, "seconds", "%d", true, true, {}, 0});
        RegisterStatistic({"time_summer_played", "Time Played (Summer)", "Time played during summer", "Time", StatisticsCategory::TIME, StatisticsDataType::TIME, "seconds", "%d", true, true, {}, 0});
        RegisterStatistic({"time_autumn_played", "Time Played (Autumn)", "Time played during autumn", "Time", StatisticsCategory::TIME, StatisticsDataType::TIME, "seconds", "%d", true, true, {}, 0});
        RegisterStatistic({"time_winter_played", "Time Played (Winter)", "Time played during winter", "Time", StatisticsCategory::TIME, StatisticsDataType::TIME, "seconds", "%d", true, true, {}, 0});
        RegisterStatistic({"time_rain_played", "Time Played (Rain)", "Time played during rain", "Time", StatisticsCategory::TIME, StatisticsDataType::TIME, "seconds", "%d", true, true, {}, 0});
        RegisterStatistic({"time_storm_played", "Time Played (Storm)", "Time played during storm", "Time", StatisticsCategory::TIME, StatisticsDataType::TIME, "seconds", "%d", true, true, {}, 0});
    }

    void StatisticsSystem::InitializeDistanceStatistics() {
        // Distance statistics
        RegisterStatistic({"distance_walked", "Distance Walked", "Total distance walked", "Distance", StatisticsCategory::DISTANCE, StatisticsDataType::DISTANCE, "meters", "%.2f", true, true, {}, 0});
        RegisterStatistic({"distance_run", "Distance Run", "Total distance run", "Distance", StatisticsCategory::DISTANCE, StatisticsDataType::DISTANCE, "meters", "%.2f", true, true, {}, 0});
        RegisterStatistic({"distance_swam", "Distance Swam", "Total distance swam", "Distance", StatisticsCategory::DISTANCE, StatisticsDataType::DISTANCE, "meters", "%.2f", true, true, {}, 0});
        RegisterStatistic({"distance_flown", "Distance Flown", "Total distance flown", "Distance", StatisticsCategory::DISTANCE, StatisticsDataType::DISTANCE, "meters", "%.2f", true, true, {}, 0});
        RegisterStatistic({"distance_climbed", "Distance Climbed", "Total distance climbed", "Distance", StatisticsCategory::DISTANCE, StatisticsDataType::DISTANCE, "meters", "%.2f", true, true, {}, 0});
        RegisterStatistic({"distance_fallen", "Distance Fallen", "Total distance fallen", "Distance", StatisticsCategory::DISTANCE, StatisticsDataType::DISTANCE, "meters", "%.2f", true, true, {}, 0});
        RegisterStatistic({"distance_vehicle", "Distance in Vehicles", "Total distance in vehicles", "Distance", StatisticsCategory::DISTANCE, StatisticsDataType::DISTANCE, "meters", "%.2f", true, true, {}, 0});
        RegisterStatistic({"distance_mount", "Distance on Mounts", "Total distance on mounts", "Distance", StatisticsCategory::DISTANCE, StatisticsDataType::DISTANCE, "meters", "%.2f", true, true, {}, 0});
        RegisterStatistic({"distance_teleported", "Distance Teleported", "Total distance teleported", "Distance", StatisticsCategory::DISTANCE, StatisticsDataType::DISTANCE, "meters", "%.2f", true, true, {}, 0});
        RegisterStatistic({"distance_fast_travel", "Distance Fast Traveled", "Total distance fast traveled", "Distance", StatisticsCategory::DISTANCE, StatisticsDataType::DISTANCE, "meters", "%.2f", true, true, {}, 0});
    }

    void StatisticsSystem::InitializeDamageStatistics() {
        // Damage statistics
        RegisterStatistic({"damage_physical_dealt", "Physical Damage Dealt", "Total physical damage dealt", "Damage", StatisticsCategory::DAMAGE, StatisticsDataType::FLOAT, "", "%.1f", true, true, {}, 0});
        RegisterStatistic({"damage_magical_dealt", "Magical Damage Dealt", "Total magical damage dealt", "Damage", StatisticsCategory::DAMAGE, StatisticsDataType::FLOAT, "", "%.1f", true, true, {}, 0});
        RegisterStatistic({"damage_fire_dealt", "Fire Damage Dealt", "Total fire damage dealt", "Damage", StatisticsCategory::DAMAGE, StatisticsDataType::FLOAT, "", "%.1f", true, true, {}, 0});
        RegisterStatistic({"damage_cold_dealt", "Cold Damage Dealt", "Total cold damage dealt", "Damage", StatisticsCategory::DAMAGE, StatisticsDataType::FLOAT, "", "%.1f", true, true, {}, 0});
        RegisterStatistic({"damage_lightning_dealt", "Lightning Damage Dealt", "Total lightning damage dealt", "Damage", StatisticsCategory::DAMAGE, StatisticsDataType::FLOAT, "", "%.1f", true, true, {}, 0});
        RegisterStatistic({"damage_physical_received", "Physical Damage Received", "Total physical damage received", "Damage", StatisticsCategory::DAMAGE, StatisticsDataType::FLOAT, "", "%.1f", true, true, {}, 0});
        RegisterStatistic({"damage_magical_received", "Magical Damage Received", "Total magical damage received", "Damage", StatisticsCategory::DAMAGE, StatisticsDataType::FLOAT, "", "%.1f", true, true, {}, 0});
        RegisterStatistic({"damage_fire_received", "Fire Damage Received", "Total fire damage received", "Damage", StatisticsCategory::DAMAGE, StatisticsDataType::FLOAT, "", "%.1f", true, true, {}, 0});
        RegisterStatistic({"damage_cold_received", "Cold Damage Received", "Total cold damage received", "Damage", StatisticsCategory::DAMAGE, StatisticsDataType::FLOAT, "", "%.1f", true, true, {}, 0});
        RegisterStatistic({"damage_lightning_received", "Lightning Damage Received", "Total lightning damage received", "Damage", StatisticsCategory::DAMAGE, StatisticsDataType::FLOAT, "", "%.1f", true, true, {}, 0});
    }

    void StatisticsSystem::InitializeMagicStatistics() {
        // Magic statistics
        RegisterStatistic({"magic_spells_cast", "Spells Cast", "Total spells cast", "Magic", StatisticsCategory::MAGIC, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"magic_mana_spent", "Mana Spent", "Total mana spent", "Magic", StatisticsCategory::MAGIC, StatisticsDataType::FLOAT, "", "%.1f", true, true, {}, 0});
        RegisterStatistic({"magic_mana_regenerated", "Mana Regenerated", "Total mana regenerated", "Magic", StatisticsCategory::MAGIC, StatisticsDataType::FLOAT, "", "%.1f", true, true, {}, 0});
        RegisterStatistic({"magic_spells_learned", "Spells Learned", "Total spells learned", "Magic", StatisticsCategory::MAGIC, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"magic_spells_mastered", "Spells Mastered", "Spells mastered", "Magic", StatisticsCategory::MAGIC, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"magic_elements_mastered", "Elements Mastered", "Elements mastered", "Magic", StatisticsCategory::MAGIC, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"magic_critical_casts", "Critical Casts", "Total critical spell casts", "Magic", StatisticsCategory::MAGIC, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"magic_spell_interrupts", "Spell Interrupts", "Total spell interrupts", "Magic", StatisticsCategory::MAGIC, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"magic_teleports_cast", "Teleports Cast", "Total teleport spells cast", "Magic", StatisticsCategory::MAGIC, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"magic_summons_cast", "Summons Cast", "Total summon spells cast", "Magic", StatisticsCategory::MAGIC, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
    }

    void StatisticsSystem::InitializeVehicleStatistics() {
        // Vehicle statistics
        RegisterStatistic({"vehicle_distance_driven", "Distance Driven", "Total distance driven in vehicles", "Vehicle", StatisticsCategory::VEHICLE, StatisticsDataType::DISTANCE, "meters", "%.2f", true, true, {}, 0});
        RegisterStatistic({"vehicle_crashes", "Vehicle Crashes", "Total vehicle crashes", "Vehicle", StatisticsCategory::VEHICLE, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"vehicle_repairs", "Vehicle Repairs", "Total vehicle repairs", "Vehicle", StatisticsCategory::VEHICLE, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"vehicle_fuel_used", "Fuel Used", "Total fuel used", "Vehicle", StatisticsCategory::VEHICLE, StatisticsDataType::FLOAT, "", "%.1f", true, true, {}, 0});
        RegisterStatistic({"vehicle_max_speed", "Max Speed Achieved", "Maximum speed achieved", "Vehicle", StatisticsCategory::VEHICLE, StatisticsDataType::VELOCITY, "m/s", "%.2f", true, true, {}, 0});
        RegisterStatistic({"vehicle_tricks_performed", "Tricks Performed", "Total vehicle tricks", "Vehicle", StatisticsCategory::VEHICLE, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"vehicle_passengers_carried", "Passengers Carried", "Total passengers carried", "Vehicle", StatisticsCategory::VEHICLE, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"vehicle_time_driven", "Time Driven", "Total time spent driving", "Vehicle", StatisticsCategory::VEHICLE, StatisticsDataType::TIME, "seconds", "%d", true, true, {}, 0});
        RegisterStatistic({"vehicle_upgrades_purchased", "Upgrades Purchased", "Total vehicle upgrades", "Vehicle", StatisticsCategory::VEHICLE, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"vehicle_customizations", "Customizations", "Total customizations applied", "Vehicle", StatisticsCategory::VEHICLE, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
    }

    void StatisticsSystem::InitializePetStatistics() {
        // Pet statistics
        RegisterStatistic({"pet_time_played", "Time with Pets", "Total time spent with pets", "Pet", StatisticsCategory::PET, StatisticsDataType::TIME, "seconds", "%d", true, true, {}, 0});
        RegisterStatistic({"pet_commands_given", "Commands Given", "Total commands given to pets", "Pet", StatisticsCategory::PET, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"pet_tricks_taught", "Tricks Taught", "Total tricks taught to pets", "Pet", StatisticsCategory::PET, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"pet_battles_won", "Pet Battles Won", "Total pet battles won", "Pet", StatisticsCategory::PET, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"pet_happiness_level", "Pet Happiness", "Average pet happiness level", "Pet", StatisticsCategory::PET, StatisticsDataType::FLOAT, "", "%.1f", true, true, {}, 0});
        RegisterStatistic({"pet_food_consumed", "Pet Food Consumed", "Total pet food consumed", "Pet", StatisticsCategory::PET, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"pet_grooming_sessions", "Grooming Sessions", "Total pet grooming sessions", "Pet", StatisticsCategory::PET, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"pet_training_sessions", "Training Sessions", "Total pet training sessions", "Pet", StatisticsCategory::PET, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"pet_photos_taken", "Pet Photos Taken", "Total pet photos taken", "Pet", StatisticsCategory::PET, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"pet_costumes_used", "Pet Costumes Used", "Total pet costumes used", "Pet", StatisticsCategory::PET, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
    }

    void StatisticsSystem::InitializeFarmingStatistics() {
        // Farming statistics
        RegisterStatistic({"farming_crops_planted", "Crops Planted", "Total crops planted", "Farming", StatisticsCategory::FARMING, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"farming_crops_harvested", "Crops Harvested", "Total crops harvested", "Farming", StatisticsCategory::FARMING, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"farming_animals_bred", "Animals Bred", "Total animals bred", "Farming", StatisticsCategory::FARMING, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"farming_milk_collected", "Milk Collected", "Total milk collected", "Farming", StatisticsCategory::FARMING, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"farming_eggs_collected", "Eggs Collected", "Total eggs collected", "Farming", StatisticsCategory::FARMING, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"farming_wool_shaved", "Wool Shaved", "Total wool shaved", "Farming", StatisticsCategory::FARMING, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"farming_fences_built", "Fences Built", "Total fences built", "Farming", StatisticsCategory::FARMING, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"farming_water_placed", "Water Placed", "Total water sources placed", "Farming", StatisticsCategory::FARMING, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"farming_scarecrows_placed", "Scarecrows Placed", "Total scarecrows placed", "Farming", StatisticsCategory::FARMING, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"farming_time_spent", "Time Spent Farming", "Total time spent farming", "Farming", StatisticsCategory::FARMING, StatisticsDataType::TIME, "seconds", "%d", true, true, {}, 0});
    }

    void StatisticsSystem::InitializeMiningStatistics() {
        // Mining statistics
        RegisterStatistic({"mining_ores_mined", "Ores Mined", "Total ores mined", "Mining", StatisticsCategory::MINING, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"mining_stone_mined", "Stone Mined", "Total stone mined", "Mining", StatisticsCategory::MINING, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"mining_coal_mined", "Coal Mined", "Total coal mined", "Mining", StatisticsCategory::MINING, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"mining_iron_mined", "Iron Mined", "Total iron mined", "Mining", StatisticsCategory::MINING, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"mining_gold_mined", "Gold Mined", "Total gold mined", "Mining", StatisticsCategory::MINING, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"mining_diamond_mined", "Diamond Mined", "Total diamond mined", "Mining", StatisticsCategory::MINING, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"mining_pickaxes_broken", "Pickaxes Broken", "Total pickaxes broken", "Mining", StatisticsCategory::MINING, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"mining_tunnels_dug", "Tunnels Dug", "Total tunnel length dug", "Mining", StatisticsCategory::MINING, StatisticsDataType::DISTANCE, "meters", "%.2f", true, true, {}, 0});
        RegisterStatistic({"mining_caves_found", "Caves Found", "Total caves discovered", "Mining", StatisticsCategory::MINING, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"mining_time_spent", "Time Spent Mining", "Total time spent mining", "Mining", StatisticsCategory::MINING, StatisticsDataType::TIME, "seconds", "%d", true, true, {}, 0});
    }

    void StatisticsSystem::InitializeFishingStatistics() {
        // Fishing statistics
        RegisterStatistic({"fishing_fish_caught", "Fish Caught", "Total fish caught", "Fishing", StatisticsCategory::FISHING, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"fishing_rare_fish_caught", "Rare Fish Caught", "Total rare fish caught", "Fishing", StatisticsCategory::FISHING, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"fishing_legendary_fish_caught", "Legendary Fish Caught", "Total legendary fish caught", "Fishing", StatisticsCategory::FISHING, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"fishing_rods_broken", "Fishing Rods Broken", "Total fishing rods broken", "Fishing", StatisticsCategory::FISHING, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"fishing_bait_used", "Bait Used", "Total bait used", "Fishing", StatisticsCategory::FISHING, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"fishing_biggest_catch", "Biggest Catch", "Size of biggest fish caught", "Fishing", StatisticsCategory::FISHING, StatisticsDataType::FLOAT, "kg", "%.2f", true, true, {}, 0});
        RegisterStatistic({"fishing_fastest_catch", "Fastest Catch", "Fastest fish caught time", "Fishing", StatisticsCategory::FISHING, StatisticsDataType::FLOAT, "seconds", "%.2f", true, true, {}, 0});
        RegisterStatistic({"fishing_fishing_spots_found", "Fishing Spots Found", "Total fishing spots discovered", "Fishing", StatisticsCategory::FISHING, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"fishing_competitions_won", "Fishing Competitions Won", "Total fishing competitions won", "Fishing", StatisticsCategory::FISHING, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"fishing_time_spent", "Time Spent Fishing", "Total time spent fishing", "Fishing", StatisticsCategory::FISHING, StatisticsDataType::TIME, "seconds", "%d", true, true, {}, 0});
    }

    void StatisticsSystem::InitializeCookingStatistics() {
        // Cooking statistics
        RegisterStatistic({"cooking_meals_cooked", "Meals Cooked", "Total meals cooked", "Cooking", StatisticsCategory::COOKING, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"cooking_recipes_discovered", "Recipes Discovered", "Total recipes discovered", "Cooking", StatisticsCategory::COOKING, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"cooking_master_recipes", "Master Recipes", "Recipes mastered", "Cooking", StatisticsCategory::COOKING, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"cooking_ingredients_used", "Ingredients Used", "Total ingredients used", "Cooking", StatisticsCategory::COOKING, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"cooking_burned_meals", "Burned Meals", "Total meals burned", "Cooking", StatisticsCategory::COOKING, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"cooking_perfect_meals", "Perfect Meals", "Total perfect meals cooked", "Cooking", StatisticsCategory::COOKING, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"cooking_cooking_fails", "Cooking Fails", "Total cooking failures", "Cooking", StatisticsCategory::COOKING, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"cooking_spices_used", "Spices Used", "Total spices used", "Cooking", StatisticsCategory::COOKING, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"cooking_dishes_served", "Dishes Served", "Total dishes served to others", "Cooking", StatisticsCategory::COOKING, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"cooking_time_spent", "Time Spent Cooking", "Total time spent cooking", "Cooking", StatisticsCategory::COOKING, StatisticsDataType::TIME, "seconds", "%d", true, true, {}, 0});
    }

    void StatisticsSystem::InitializeTradingStatistics() {
        // Trading statistics
        RegisterStatistic({"trading_trades_completed", "Trades Completed", "Total trades completed", "Trading", StatisticsCategory::TRADING, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"trading_items_traded", "Items Traded", "Total items traded", "Trading", StatisticsCategory::TRADING, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"trading_gold_earned", "Gold Earned from Trading", "Total gold earned from trading", "Trading", StatisticsCategory::TRADING, StatisticsDataType::INTEGER, "", "%d", true, true, {}, 0});
        RegisterStatistic({"trading_gold_spent", "Gold Spent Trading", "Total gold spent on trading", "Trading", StatisticsCategory::TRADING, StatisticsDataType::INTEGER, "", "%d", true, true, {}, 0});
        RegisterStatistic({"trading_profitable_trades", "Profitable Trades", "Total profitable trades", "Trading", StatisticsCategory::TRADING, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"trading_loss_trades", "Loss Trades", "Total trades with loss", "Trading", StatisticsCategory::TRADING, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"trading_best_trade", "Best Trade Profit", "Highest profit from single trade", "Trading", StatisticsCategory::TRADING, StatisticsDataType::INTEGER, "", "%d", true, true, {}, 0});
        RegisterStatistic({"trading_worst_trade", "Worst Trade Loss", "Highest loss from single trade", "Trading", StatisticsCategory::TRADING, StatisticsDataType::INTEGER, "", "%d", true, true, {}, 0});
        RegisterStatistic({"trading_trading_partners", "Trading Partners", "Total unique trading partners", "Trading", StatisticsCategory::TRADING, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"trading_time_spent", "Time Spent Trading", "Total time spent trading", "Trading", StatisticsCategory::TRADING, StatisticsDataType::TIME, "seconds", "%d", true, true, {}, 0});
    }

    void StatisticsSystem::InitializeBuildingStatistics() {
        // Building statistics
        RegisterStatistic({"building_blocks_placed", "Blocks Placed", "Total blocks placed", "Building", StatisticsCategory::BUILDING, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"building_blocks_broken", "Blocks Broken", "Total blocks broken", "Building", StatisticsCategory::BUILDING, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"building_structures_built", "Structures Built", "Total structures built", "Building", StatisticsCategory::BUILDING, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"building_blueprints_used", "Blueprints Used", "Total blueprints used", "Building", StatisticsCategory::BUILDING, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"building_houses_built", "Houses Built", "Total houses built", "Building", StatisticsCategory::BUILDING, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"building_towers_built", "Towers Built", "Total towers built", "Building", StatisticsCategory::BUILDING, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"building_bridges_built", "Bridges Built", "Total bridges built", "Building", StatisticsCategory::BUILDING, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"building_underground_structures", "Underground Structures", "Total underground structures built", "Building", StatisticsCategory::BUILDING, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"building_aerial_structures", "Aerial Structures", "Total aerial structures built", "Building", StatisticsCategory::BUILDING, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"building_time_spent", "Time Spent Building", "Total time spent building", "Building", StatisticsCategory::BUILDING, StatisticsDataType::TIME, "seconds", "%d", true, true, {}, 0});
    }

    void StatisticsSystem::InitializeQuestStatistics() {
        // Quest statistics
        RegisterStatistic({"quest_quests_completed", "Quests Completed", "Total quests completed", "Quest", StatisticsCategory::QUEST, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"quest_main_quests_completed", "Main Quests Completed", "Total main quests completed", "Quest", StatisticsCategory::QUEST, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"quest_side_quests_completed", "Side Quests Completed", "Total side quests completed", "Quest", StatisticsCategory::QUEST, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"quest_quest_lines_completed", "Quest Lines Completed", "Total quest lines completed", "Quest", StatisticsCategory::QUEST, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"quest_fastest_completion", "Fastest Quest Completion", "Fastest quest completion time", "Quest", StatisticsCategory::QUEST, StatisticsDataType::TIME, "seconds", "%d", true, true, {}, 0});
        RegisterStatistic({"quest_quest_fails", "Quest Fails", "Total quest failures", "Quest", StatisticsCategory::QUEST, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"quest_quest_restarts", "Quest Restarts", "Total quest restarts", "Quest", StatisticsCategory::QUEST, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"quest_companions_recruited", "Companions Recruited", "Total companions recruited", "Quest", StatisticsCategory::QUEST, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"quest_locations_discovered", "Quest Locations Discovered", "Total quest locations discovered", "Quest", StatisticsCategory::QUEST, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"quest_time_spent", "Time Spent on Quests", "Total time spent on quests", "Quest", StatisticsCategory::QUEST, StatisticsDataType::TIME, "seconds", "%d", true, true, {}, 0});
    }

    void StatisticsSystem::InitializeAchievementStatistics() {
        // Achievement statistics
        RegisterStatistic({"achievement_achievements_unlocked", "Achievements Unlocked", "Total achievements unlocked", "Achievement", StatisticsCategory::ACHIEVEMENT, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"achievement_hidden_discovered", "Hidden Achievements Discovered", "Total hidden achievements discovered", "Achievement", StatisticsCategory::ACHIEVEMENT, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"achievement_challenges_completed", "Challenges Completed", "Total challenges completed", "Achievement", StatisticsCategory::ACHIEVEMENT, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"achievement_rewards_claimed", "Rewards Claimed", "Total achievement rewards claimed", "Achievement", StatisticsCategory::ACHIEVEMENT, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"achievement_perfect_games", "Perfect Games", "Games with all achievements", "Achievement", StatisticsCategory::ACHIEVEMENT, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"achievement_speedruns", "Speedruns", "Achievement-based speedruns completed", "Achievement", StatisticsCategory::ACHIEVEMENT, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"achievement_no_damage_runs", "No Damage Runs", "No damage achievement runs", "Achievement", StatisticsCategory::ACHIEVEMENT, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"achievement_pacifist_runs", "Pacifist Runs", "Pacifist achievement runs", "Achievement", StatisticsCategory::ACHIEVEMENT, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"achievement_ironman_runs", "Ironman Runs", "Ironman achievement runs", "Achievement", StatisticsCategory::ACHIEVEMENT, StatisticsDataType::COUNT, "", "%d", true, true, {}, 0});
        RegisterStatistic({"achievement_time_spent", "Time Spent on Achievements", "Total time spent working on achievements", "Achievement", StatisticsCategory::ACHIEVEMENT, StatisticsDataType::TIME, "seconds", "%d", true, true, {}, 0});
    }

} // namespace VoxelCraft
