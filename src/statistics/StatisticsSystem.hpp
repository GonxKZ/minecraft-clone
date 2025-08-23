/**
 * @file StatisticsSystem.hpp
 * @brief VoxelCraft Advanced Statistics System Header
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_STATISTICS_STATISTICS_SYSTEM_HPP
#define VOXELCRAFT_STATISTICS_STATISTICS_SYSTEM_HPP

#include <memory>
#include <atomic>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <functional>
#include <chrono>
#include <queue>
#include <variant>
#include <any>

#include <glm/glm.hpp>

#include "../core/System.hpp"
#include "../event/EventSystem.hpp"

namespace VoxelCraft {

    // Forward declarations
    class StatisticsTracker;
    class StatisticsAnalyzer;
    class StatisticsVisualizer;
    class StatisticsExporter;
    class StatisticsImporter;
    class StatisticsCloudSync;
    class PlayerMetrics;
    class WorldMetrics;
    class EconomyMetrics;
    class SocialMetrics;
    class PerformanceMetrics;

    /**
     * @brief Statistics data types
     */
    enum class StatisticsDataType {
        INTEGER,        // Whole numbers
        FLOAT,          // Decimal numbers
        BOOLEAN,        // True/false values
        TIME,           // Time duration
        DISTANCE,       // Distance measurements
        VELOCITY,       // Speed measurements
        COUNT,          // Item/block/mob counts
        PERCENTAGE,     // Percentage values
        RATIO,          // Ratio values
        VECTOR2,        // 2D vector data
        VECTOR3,        // 3D vector data
        STRING,         // Text data
        TIMESTAMP,      // Date/time stamps
        DURATION,       // Time spans
        FREQUENCY       // Rate measurements
    };

    /**
     * @brief Statistics categories
     */
    enum class StatisticsCategory {
        PLAYER,         // Player-specific stats
        WORLD,          // World/environment stats
        COMBAT,         // Combat-related stats
        CRAFTING,       // Crafting/building stats
        EXPLORATION,    // Exploration/discovery stats
        SOCIAL,         // Social interaction stats
        ECONOMY,        // Economic activity stats
        PERFORMANCE,    // Performance metrics
        ACHIEVEMENT,    // Achievement progress stats
        RESOURCE,       // Resource usage stats
        TIME,           // Time-based stats
        DISTANCE,       // Movement/distance stats
        DAMAGE,         // Damage/healing stats
        MAGIC,          // Magic/spell stats
        VEHICLE,        // Vehicle/transport stats
        PET,            // Pet/companion stats
        FARMING,        // Farming/agriculture stats
        MINING,         // Mining/gathering stats
        FISHING,        // Fishing stats
        COOKING,        // Cooking/food stats
        BREWING,        // Brewing/potion stats
        ENCHANTING,     // Enchanting stats
        TAMING,         // Animal taming stats
        BUILDING,       // Construction stats
        DESTROYING,     // Destruction stats
        TRADING,        // Trading stats
        QUEST,          // Quest/mission stats
        DUNGEON,        // Dungeon stats
        BOSS,           // Boss fight stats
        ARENA,          // PvP arena stats
        GUILD,          // Guild stats
        FACTION,        // Faction stats
        EVENT,          // Special event stats
        SEASONAL,       // Seasonal stats
        DAILY,          // Daily stats
        WEEKLY,         // Weekly stats
        MONTHLY,        // Monthly stats
        YEARLY,         // Yearly stats
        LIFETIME        // Lifetime stats
    };

    /**
     * @brief Statistics aggregation methods
     */
    enum class StatisticsAggregation {
        SUM,            // Sum of all values
        AVERAGE,        // Average of values
        MINIMUM,        // Minimum value
        MAXIMUM,        // Maximum value
        COUNT,          // Count of occurrences
        DISTINCT_COUNT, // Count of unique values
        FIRST,          // First recorded value
        LAST,           // Last recorded value
        RATE,           // Rate of change
        PERCENTILE,     // Percentile calculation
        STANDARD_DEV,   // Standard deviation
        MEDIAN,         // Median value
        MODE            // Most frequent value
    };

    /**
     * @brief Statistics time periods
     */
    enum class StatisticsTimePeriod {
        CURRENT_SESSION,
        TODAY,
        YESTERDAY,
        THIS_WEEK,
        LAST_WEEK,
        THIS_MONTH,
        LAST_MONTH,
        THIS_YEAR,
        LAST_YEAR,
        ALL_TIME
    };

    /**
     * @brief Statistics comparison types
     */
    enum class StatisticsComparison {
        PERSONAL_BEST,
        FRIENDS_AVERAGE,
        GUILD_AVERAGE,
        WORLD_AVERAGE,
        LOCAL_AVERAGE,
        GLOBAL_LEADERBOARD,
        PERSONAL_HISTORY,
        TREND_ANALYSIS
    };

    /**
     * @brief Statistics visualization types
     */
    enum class StatisticsVisualization {
        BAR_CHART,
        LINE_CHART,
        PIE_CHART,
        SCATTER_PLOT,
        HISTOGRAM,
        HEAT_MAP,
        RADAR_CHART,
        GAUGE,
        PROGRESS_BAR,
        TIMELINE,
        TABLE,
        TEXT_SUMMARY
    };

    /**
     * @brief Statistics export formats
     */
    enum class StatisticsExportFormat {
        JSON,
        XML,
        CSV,
        BINARY,
        HTML,
        PDF,
        EXCEL,
        GOOGLE_SHEETS
    };

    /**
     * @brief Statistics data point
     */
    struct StatisticsDataPoint {
        std::string statisticId;
        std::chrono::system_clock::time_point timestamp;
        std::variant<
            int64_t,        // Integer
            double,         // Float
            bool,           // Boolean
            std::chrono::milliseconds, // Time/Duration
            float,          // Distance/Velocity
            glm::vec2,      // Vector2
            glm::vec3,      // Vector3
            std::string     // String
        > value;
        std::unordered_map<std::string, std::any> metadata;
    };

    /**
     * @brief Statistics definition
     */
    struct StatisticsDefinition {
        std::string id;
        std::string name;
        std::string description;
        std::string category;
        StatisticsCategory statCategory;
        StatisticsDataType dataType;
        std::string unit;
        std::string formatString;
        bool persistent = true;
        bool visible = true;
        int sortOrder = 0;
        std::vector<StatisticsAggregation> supportedAggregations;
        std::unordered_map<std::string, std::any> properties;
    };

    /**
     * @brief Statistics value with metadata
     */
    struct StatisticsValue {
        StatisticsDataType type;
        std::variant<
            int64_t,
            double,
            bool,
            std::chrono::milliseconds,
            float,
            glm::vec2,
            glm::vec3,
            std::string
        > value;
        std::chrono::system_clock::time_point lastUpdated;
        std::chrono::system_clock::time_point firstRecorded;
        uint64_t updateCount = 0;
        bool isValid = true;
    };

    /**
     * @brief Statistics trend analysis
     */
    struct StatisticsTrend {
        std::string statisticId;
        StatisticsTimePeriod period;
        double currentValue;
        double previousValue;
        double changeAbsolute;
        double changePercentage;
        std::string trend; // "increasing", "decreasing", "stable"
        double rateOfChange; // per day/week/month
        std::vector<double> historicalValues;
        std::chrono::system_clock::time_point analysisTimestamp;
    };

    /**
     * @brief Statistics comparison data
     */
    struct StatisticsComparisonData {
        std::string statisticId;
        StatisticsComparison comparisonType;
        double playerValue;
        double comparisonValue;
        std::string comparisonLabel;
        int32_t rank = -1;
        int32_t totalParticipants = 0;
        std::string percentile;
        std::string performance; // "above_average", "average", "below_average"
    };

    /**
     * @brief Statistics report
     */
    struct StatisticsReport {
        std::string reportId;
        std::string reportName;
        std::string playerId;
        std::chrono::system_clock::time_point generatedTime;
        StatisticsTimePeriod timePeriod;
        std::unordered_map<std::string, StatisticsValue> values;
        std::unordered_map<std::string, StatisticsTrend> trends;
        std::unordered_map<std::string, StatisticsComparisonData> comparisons;
        std::unordered_map<std::string, std::any> metadata;
    };

    /**
     * @brief Statistics dashboard configuration
     */
    struct StatisticsDashboard {
        std::string dashboardId;
        std::string dashboardName;
        std::vector<std::string> statisticIds;
        std::vector<StatisticsVisualization> visualizations;
        std::vector<StatisticsTimePeriod> timePeriods;
        bool autoRefresh = true;
        int refreshIntervalSeconds = 60;
        std::unordered_map<std::string, std::any> layout;
    };

    /**
     * @brief Main statistics system
     */
    class StatisticsSystem : public System {
    public:
        static StatisticsSystem& GetInstance();

        StatisticsSystem();
        ~StatisticsSystem() override;

        // System interface
        bool Initialize() override;
        void Update(float deltaTime) override;
        void Shutdown() override;
        std::string GetName() const override { return "StatisticsSystem"; }
        System::Type GetType() const override { return System::Type::STATISTICS; }

        // Statistics tracking
        bool RegisterStatistic(const StatisticsDefinition& definition);
        bool UnregisterStatistic(const std::string& statisticId);
        std::shared_ptr<StatisticsDefinition> GetStatisticDefinition(const std::string& statisticId) const;

        bool UpdateStatistic(const std::string& statisticId, const std::any& value,
                           const std::unordered_map<std::string, std::any>& metadata = {});
        StatisticsValue GetStatisticValue(const std::string& statisticId) const;
        bool ResetStatistic(const std::string& statisticId);

        // Bulk operations
        bool UpdateMultipleStatistics(const std::unordered_map<std::string, std::any>& updates);
        std::unordered_map<std::string, StatisticsValue> GetMultipleStatisticValues(const std::vector<std::string>& statisticIds) const;

        // Statistics queries
        std::vector<std::string> GetAllStatisticIds() const;
        std::vector<std::string> GetStatisticIdsByCategory(const std::string& category) const;
        std::vector<std::string> GetStatisticIdsByType(StatisticsCategory type) const;

        // Time-based queries
        StatisticsValue GetStatisticValueAtTime(const std::string& statisticId,
                                               const std::chrono::system_clock::time_point& time) const;
        std::vector<StatisticsDataPoint> GetStatisticHistory(const std::string& statisticId,
                                                           const std::chrono::system_clock::time_point& startTime,
                                                           const std::chrono::system_clock::time_point& endTime) const;

        // Aggregation queries
        std::unordered_map<StatisticsAggregation, double> GetStatisticAggregations(const std::string& statisticId,
                                                                                 const std::chrono::system_clock::time_point& startTime,
                                                                                 const std::chrono::system_clock::time_point& endTime) const;

        // Trend analysis
        StatisticsTrend AnalyzeStatisticTrend(const std::string& statisticId, StatisticsTimePeriod period) const;
        std::vector<StatisticsTrend> AnalyzeMultipleTrends(const std::vector<std::string>& statisticIds,
                                                         StatisticsTimePeriod period) const;

        // Comparison analysis
        StatisticsComparisonData CompareStatistic(const std::string& statisticId, StatisticsComparison comparison) const;
        std::vector<StatisticsComparisonData> CompareMultipleStatistics(const std::vector<std::string>& statisticIds,
                                                                      StatisticsComparison comparison) const;

        // Report generation
        StatisticsReport GenerateReport(const std::string& reportName, const std::vector<std::string>& statisticIds,
                                      StatisticsTimePeriod period) const;
        StatisticsReport GenerateComprehensiveReport(StatisticsTimePeriod period) const;

        // Dashboard management
        bool CreateDashboard(const StatisticsDashboard& dashboard);
        bool UpdateDashboard(const std::string& dashboardId, const StatisticsDashboard& dashboard);
        bool DeleteDashboard(const std::string& dashboardId);
        std::shared_ptr<StatisticsDashboard> GetDashboard(const std::string& dashboardId) const;

        // Visualization
        std::string GenerateVisualizationData(const std::string& statisticId, StatisticsVisualization type,
                                            StatisticsTimePeriod period) const;
        std::string GenerateDashboardVisualization(const std::string& dashboardId) const;

        // Export/Import
        bool ExportStatistics(const std::vector<std::string>& statisticIds, StatisticsExportFormat format,
                            const std::string& filename) const;
        bool ImportStatistics(const std::string& filename, StatisticsExportFormat format);

        // Cloud synchronization
        bool SyncWithCloud();
        bool LoadFromCloud();
        bool SaveToCloud();
        bool IsCloudSyncEnabled() const;

        // Performance metrics
        double GetStatisticsUpdateRate() const;
        size_t GetMemoryUsage() const;
        uint64_t GetTotalDataPoints() const;
        uint64_t GetDataPointsPerSecond() const;

        // Configuration
        void SetPlayerId(const std::string& playerId) { m_playerId = playerId; }
        const std::string& GetPlayerId() const { return m_playerId; }

        void SetAutoSave(bool enabled) { m_autoSave = enabled; }
        bool IsAutoSaveEnabled() const { return m_autoSave; }

        void SetCloudSyncEnabled(bool enabled) { m_cloudSyncEnabled = enabled; }
        bool IsCloudSyncEnabled() const { return m_cloudSyncEnabled; }

        void SetMaxHistoryDays(uint32_t days) { m_maxHistoryDays = days; }
        uint32_t GetMaxHistoryDays() const { return m_maxHistoryDays; }

        // Event integration
        void OnEvent(const EventBase& event);
        bool RegisterEventTracking(const std::string& eventName, const std::string& statisticId,
                                 std::function<std::any(const EventBase&)> extractor);

        // Utility functions
        std::string GetStatisticsInfo() const;
        void PrintStatisticsReport() const;

        bool ValidateStatistic(const std::string& statisticId) const;
        bool ValidateAllStatistics() const;

        void ResetAllStatistics();
        void CleanupOldData();

        // Debug and testing
        void DebugUpdateStatistic(const std::string& statisticId, const std::any& value);
        std::vector<std::string> DebugGetTrackedStatistics() const;
        bool DebugGenerateSampleData();

    private:
        // Core components
        mutable std::shared_mutex m_definitionsMutex;
        std::unordered_map<std::string, StatisticsDefinition> m_statisticDefinitions;

        mutable std::shared_mutex m_valuesMutex;
        std::unordered_map<std::string, StatisticsValue> m_statisticValues;

        mutable std::shared_mutex m_historyMutex;
        std::unordered_map<std::string, std::vector<StatisticsDataPoint>> m_statisticHistory;

        mutable std::shared_mutex m_dashboardsMutex;
        std::unordered_map<std::string, StatisticsDashboard> m_dashboards;

        // Event tracking
        mutable std::shared_mutex m_eventTrackingMutex;
        std::unordered_map<std::string, std::vector<std::pair<std::string, std::function<std::any(const EventBase&)>>>> m_eventTracking;

        // Configuration
        std::string m_playerId;
        bool m_autoSave;
        bool m_cloudSyncEnabled;
        uint32_t m_maxHistoryDays;
        std::chrono::seconds m_autoSaveInterval;
        std::chrono::steady_clock::time_point m_lastAutoSave;

        // Performance tracking
        std::atomic<uint64_t> m_totalUpdates;
        std::atomic<uint64_t> m_totalDataPoints;
        std::chrono::steady_clock::time_point m_systemStartTime;

        // Internal methods
        bool LoadStatisticDefinitions();
        bool InitializeDefaultStatistics();

        void ProcessDataPoint(const StatisticsDataPoint& dataPoint);
        void UpdateValueFromDataPoint(const StatisticsDataPoint& dataPoint);

        bool SaveStatistics();
        bool LoadStatistics();

        void CleanupExpiredHistory();
        void AggregateStatistics();

        std::any ConvertValueType(const std::any& value, StatisticsDataType targetType) const;
        bool ValidateValueType(const std::any& value, StatisticsDataType expectedType) const;

        // Statistics category initialization
        void InitializePlayerStatistics();
        void InitializeCombatStatistics();
        void InitializeCraftingStatistics();
        void InitializeExplorationStatistics();
        void InitializeSocialStatistics();
        void InitializeEconomyStatistics();
        void InitializePerformanceStatistics();
        void InitializeResourceStatistics();
        void InitializeTimeStatistics();
        void InitializeDistanceStatistics();
        void InitializeDamageStatistics();
        void InitializeMagicStatistics();
        void InitializeVehicleStatistics();
        void InitializePetStatistics();
        void InitializeFarmingStatistics();
        void InitializeMiningStatistics();
        void InitializeFishingStatistics();
        void InitializeCookingStatistics();
        void InitializeTradingStatistics();
        void InitializeBuildingStatistics();
        void InitializeQuestStatistics();
        void InitializeAchievementStatistics();
    };

    // Statistics system utility macros
    #define VOXELCRAFT_UPDATE_STAT(stat, value) \
        StatisticsSystem::GetInstance().UpdateStatistic(stat, value)

    #define VOXELCRAFT_GET_STAT(stat) \
        StatisticsSystem::GetInstance().GetStatisticValue(stat)

    #define VOXELCRAFT_INCREMENT_STAT(stat, amount) \
        do { \
            auto current = StatisticsSystem::GetInstance().GetStatisticValue(stat); \
            if (current.type == StatisticsDataType::INTEGER) { \
                int64_t newValue = std::get<int64_t>(current.value) + amount; \
                StatisticsSystem::GetInstance().UpdateStatistic(stat, newValue); \
            } \
        } while(0)

    #define VOXELCRAFT_RECORD_EVENT_STAT(event, stat, extractor) \
        StatisticsSystem::GetInstance().RegisterEventTracking(event, stat, extractor)

} // namespace VoxelCraft

#endif // VOXELCRAFT_STATISTICS_STATISTICS_SYSTEM_HPP
