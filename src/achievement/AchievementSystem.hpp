/**
 * @file AchievementSystem.hpp
 * @brief VoxelCraft Advanced Achievement System Header
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_ACHIEVEMENT_ACHIEVEMENT_SYSTEM_HPP
#define VOXELCRAFT_ACHIEVEMENT_ACHIEVEMENT_SYSTEM_HPP

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
#include <variant>
#include <any>

#include <glm/glm.hpp>

#include "../core/System.hpp"
#include "../event/EventSystem.hpp"

namespace VoxelCraft {

    // Forward declarations
    class Achievement;
    class AchievementProgress;
    class AchievementReward;
    class AchievementManager;
    class AchievementTracker;
    class AchievementDatabase;
    class AchievementNotification;
    class AchievementChallenge;
    class AchievementLeaderboard;
    class AchievementCloudSync;

    /**
     * @brief Achievement types
     */
    enum class AchievementType {
        PROGRESS,       // Progress-based (collect X items, reach level Y)
        SKILL,          // Skill-based (master a craft, learn a spell)
        EXPLORATION,    // Exploration-based (discover locations, biomes)
        COMBAT,         // Combat-based (defeat enemies, complete quests)
        SOCIAL,         // Social-based (make friends, join guilds)
        CREATIVE,       // Creative-based (build structures, create art)
        COLLECTOR,      // Collector-based (gather rare items, complete sets)
        CHALLENGE,      // Time-limited challenges
        HIDDEN,         // Secret achievements
        MILESTONE,      // Major game milestones
        SPECIAL,        // Special event achievements
        SEASONAL,       // Seasonal/temporary achievements
        CUSTOM          // Custom achievement type
    };

    /**
     * @brief Achievement rarity levels
     */
    enum class AchievementRarity {
        COMMON,         // Easy to obtain
        UNCOMMON,       // Moderate difficulty
        RARE,           // Challenging
        EPIC,           // Very difficult
        LEGENDARY,      // Extremely difficult
        MYTHICAL        // Nearly impossible
    };

    /**
     * @brief Achievement states
     */
    enum class AchievementState {
        HIDDEN,         // Not yet revealed to player
        VISIBLE,        // Visible but not unlocked
        IN_PROGRESS,    // Player is working on it
        COMPLETED,      // Recently completed
        UNLOCKED        // Fully unlocked and rewarded
    };

    /**
     * @brief Achievement trigger types
     */
    enum class AchievementTrigger {
        EVENT,          // Triggered by game events
        STATISTIC,      // Triggered by statistic thresholds
        CONDITION,      // Triggered by complex conditions
        TIME_BASED,     // Triggered by time-based events
        LOCATION_BASED, // Triggered by location/position
        INTERACTION,    // Triggered by player interactions
        CUSTOM          // Custom trigger logic
    };

    /**
     * @brief Achievement progress types
     */
    enum class AchievementProgressType {
        BOOLEAN,        // Simple true/false
        INTEGER,        // Integer counter (0 to target)
        FLOAT,          // Float counter (0.0 to target)
        PERCENTAGE,     // Percentage (0% to 100%)
        TIME,           // Time-based progress
        MULTI_COUNTER   // Multiple counters
    };

    /**
     * @brief Achievement reward types
     */
    enum class AchievementRewardType {
        EXPERIENCE,     // Experience points
        ITEM,           // In-game items
        CURRENCY,       // Virtual currency
        COSMETIC,       // Cosmetic items (skins, effects)
        UNLOCK,         // Unlock new content/features
        TITLE,          // Player titles
        BADGE,          // Achievement badges
        BOOST,          // Temporary stat boosts
        PET,            // Companion pets
        MOUNT,          // Mounts
        TELEPORT,       // Teleport locations
        RECIPE,         // Crafting recipes
        SKILL_POINT,    // Skill points
        ATTRIBUTE_POINT,// Attribute points
        CUSTOM          // Custom reward type
    };

    /**
     * @brief Achievement data structures
     */
    struct AchievementCondition {
        std::string name;
        AchievementTrigger trigger;
        std::variant<
            std::string,                    // Event name
            std::pair<std::string, int>,   // Statistic (name, threshold)
            std::function<bool()>          // Custom condition function
        > triggerData;
        bool isRequired = true;
        std::string description;
    };

    struct AchievementRequirement {
        std::vector<std::string> requiredAchievements;
        std::vector<std::pair<std::string, int>> requiredStats;
        std::vector<std::pair<std::string, bool>> requiredFlags;
        int minPlayerLevel = 0;
        std::chrono::system_clock::time_point unlockTime;
        bool requiresPremium = false;
    };

    struct AchievementRewardData {
        AchievementRewardType type;
        std::variant<
            int,                    // Experience/currency/skill points
            std::string,           // Item/recipe/title/badge
            std::pair<std::string, float>  // Boost (stat, multiplier)
        > data;
        int quantity = 1;
        float duration = 0.0f;     // For temporary effects
    };

    /**
     * @brief Achievement definition
     */
    struct AchievementDefinition {
        std::string id;
        std::string name;
        std::string description;
        std::string detailedDescription;
        AchievementType type;
        AchievementRarity rarity;
        std::string iconPath;
        std::string lockedIconPath;
        std::string backgroundPath;
        std::vector<std::string> tags;
        std::vector<AchievementCondition> conditions;
        AchievementRequirement requirements;
        std::vector<AchievementRewardData> rewards;
        AchievementProgressType progressType;
        std::variant<int, float, std::vector<std::pair<std::string, int>>> targetValue;
        bool isHidden = false;
        bool isRepeatable = false;
        int maxRepeats = 1;
        std::chrono::milliseconds cooldown;
        std::string category;
        std::string subcategory;
        int sortOrder = 0;
        std::unordered_map<std::string, std::any> properties;
    };

    /**
     * @brief Achievement progress data
     */
    struct AchievementProgressData {
        std::string achievementId;
        AchievementState state;
        std::chrono::system_clock::time_point unlockedTime;
        std::chrono::system_clock::time_point lastUpdatedTime;
        std::variant<bool, int, float, std::vector<std::pair<std::string, int>>> currentValue;
        int completionCount = 0;
        std::vector<std::chrono::system_clock::time_point> completionTimes;
        std::unordered_map<std::string, std::any> metadata;
        bool rewardsClaimed = false;
    };

    /**
     * @brief Achievement statistics
     */
    struct AchievementStats {
        int totalAchievements = 0;
        int unlockedAchievements = 0;
        int hiddenAchievements = 0;
        int inProgressAchievements = 0;
        int completedToday = 0;
        int completedThisWeek = 0;
        int completedThisMonth = 0;
        float completionPercentage = 0.0f;
        std::chrono::milliseconds totalPlaytimeForAchievements;
        std::chrono::milliseconds averageTimePerAchievement;
        std::chrono::system_clock::time_point firstAchievementTime;
        std::chrono::system_clock::time_point lastAchievementTime;
        std::unordered_map<AchievementRarity, int> achievementsByRarity;
        std::unordered_map<AchievementType, int> achievementsByType;
        std::unordered_map<std::string, int> achievementsByCategory;
        int totalExperienceGained = 0;
        int totalCurrencyGained = 0;
        int totalItemsGained = 0;
        int totalTitlesGained = 0;
        int totalBadgesGained = 0;
        int streakDays = 0;
        int longestStreak = 0;
        float averageAchievementsPerDay = 0.0f;
    };

    /**
     * @brief Achievement challenge data
     */
    struct AchievementChallenge {
        std::string id;
        std::string name;
        std::string description;
        std::chrono::system_clock::time_point startTime;
        std::chrono::system_clock::time_point endTime;
        std::vector<std::string> targetAchievements;
        std::vector<AchievementRewardData> bonusRewards;
        int participants = 0;
        bool isActive = false;
        std::unordered_map<std::string, int> leaderboard;
    };

    /**
     * @brief Achievement notification
     */
    struct AchievementNotification {
        std::string achievementId;
        std::string title;
        std::string description;
        AchievementRarity rarity;
        std::vector<AchievementRewardData> rewards;
        std::chrono::system_clock::time_point timestamp;
        bool isSilent = false;
        std::string soundEffect;
        std::string particleEffect;
        glm::vec3 notificationColor;
    };

    /**
     * @brief Achievement leaderboard entry
     */
    struct LeaderboardEntry {
        std::string playerId;
        std::string playerName;
        int score;
        int rank;
        std::chrono::system_clock::time_point lastUpdated;
        std::unordered_map<std::string, int> achievementCounts;
    };

    /**
     * @brief Main achievement system
     */
    class AchievementSystem : public System {
    public:
        static AchievementSystem& GetInstance();

        AchievementSystem();
        ~AchievementSystem() override;

        // System interface
        bool Initialize() override;
        void Update(float deltaTime) override;
        void Shutdown() override;
        std::string GetName() const override { return "AchievementSystem"; }
        System::Type GetType() const override { return System::Type::ACHIEVEMENT; }

        // Achievement management
        bool RegisterAchievement(const AchievementDefinition& definition);
        bool UnregisterAchievement(const std::string& achievementId);
        std::shared_ptr<AchievementDefinition> GetAchievement(const std::string& achievementId) const;
        std::vector<std::string> GetAllAchievements() const;
        std::vector<std::string> GetAchievementsByType(AchievementType type) const;
        std::vector<std::string> GetAchievementsByCategory(const std::string& category) const;

        // Progress tracking
        bool UpdateAchievementProgress(const std::string& achievementId, const std::any& progress);
        AchievementProgressData GetAchievementProgress(const std::string& achievementId) const;
        float GetAchievementProgressPercentage(const std::string& achievementId) const;

        // Achievement unlocking
        bool UnlockAchievement(const std::string& achievementId);
        bool IsAchievementUnlocked(const std::string& achievementId) const;
        bool CanUnlockAchievement(const std::string& achievementId) const;
        std::vector<std::string> GetRecentlyUnlockedAchievements() const;

        // Reward claiming
        bool ClaimAchievementRewards(const std::string& achievementId);
        bool HasUnclaimedRewards(const std::string& achievementId) const;
        std::vector<AchievementRewardData> GetUnclaimedRewards(const std::string& achievementId) const;

        // Achievement visibility
        bool RevealAchievement(const std::string& achievementId);
        bool IsAchievementVisible(const std::string& achievementId) const;
        std::vector<std::string> GetVisibleAchievements() const;

        // Statistics and analytics
        AchievementStats GetPlayerStats() const;
        std::vector<std::pair<std::string, int>> GetMostUnlockedAchievements() const;
        std::vector<std::pair<std::string, int>> GetHardestAchievements() const;
        std::vector<std::pair<std::string, std::chrono::milliseconds>> GetFastestUnlockedAchievements() const;

        // Achievement challenges
        bool StartChallenge(const std::string& challengeId);
        bool EndChallenge(const std::string& challengeId);
        AchievementChallenge GetChallenge(const std::string& challengeId) const;
        std::vector<AchievementChallenge> GetActiveChallenges() const;

        // Leaderboards
        bool SubmitScore(const std::string& leaderboardId, int score);
        std::vector<LeaderboardEntry> GetLeaderboard(const std::string& leaderboardId, int limit = 10) const;
        LeaderboardEntry GetPlayerRank(const std::string& leaderboardId) const;

        // Notifications
        bool ShowNotification(const AchievementNotification& notification);
        std::vector<AchievementNotification> GetPendingNotifications() const;
        void ClearNotifications();

        // Event system integration
        void OnEvent(const EventBase& event);
        bool RegisterEventTrigger(const std::string& achievementId, const std::string& eventName,
                                const std::function<bool(const EventBase&)>& condition);

        // Cloud synchronization
        bool SyncWithCloud();
        bool LoadFromCloud();
        bool SaveToCloud();
        bool IsCloudSyncEnabled() const;

        // Achievement database
        bool LoadAchievementDatabase(const std::string& path);
        bool SaveAchievementDatabase(const std::string& path);
        bool ImportAchievements(const std::string& data);
        bool ExportAchievements(const std::string& path) const;

        // Player progress
        bool LoadPlayerProgress(const std::string& playerId);
        bool SavePlayerProgress(const std::string& playerId);
        bool ResetPlayerProgress();
        bool TransferProgress(const std::string& fromPlayerId, const std::string& toPlayerId);

        // Configuration
        void SetAutoSave(bool enabled) { m_autoSave = enabled; }
        bool IsAutoSaveEnabled() const { return m_autoSave; }

        void SetNotificationsEnabled(bool enabled) { m_notificationsEnabled = enabled; }
        bool AreNotificationsEnabled() const { return m_notificationsEnabled; }

        void SetCloudSyncEnabled(bool enabled) { m_cloudSyncEnabled = enabled; }
        bool IsCloudSyncEnabled() const { return m_cloudSyncEnabled; }

        void SetChallengeNotifications(bool enabled) { m_challengeNotifications = enabled; }
        bool AreChallengeNotificationsEnabled() const { return m_challengeNotifications; }

        // Utility functions
        std::string GetAchievementInfo(const std::string& achievementId) const;
        void PrintAchievementReport() const;

        bool ValidateAchievement(const std::string& achievementId) const;
        bool ValidateAllAchievements() const;

        void ResetAchievement(const std::string& achievementId);
        void ResetAllAchievements();

        // Debug and testing
        void DebugTriggerAchievement(const std::string& achievementId);
        void DebugSetAchievementProgress(const std::string& achievementId, const std::any& progress);
        std::vector<std::string> DebugGetDebugAchievements() const;

    private:
        // Core components
        mutable std::shared_mutex m_achievementsMutex;
        std::unordered_map<std::string, AchievementDefinition> m_achievementDefinitions;
        std::unordered_map<std::string, AchievementProgressData> m_playerProgress;

        mutable std::shared_mutex m_challengesMutex;
        std::unordered_map<std::string, AchievementChallenge> m_challenges;

        mutable std::shared_mutex m_leaderboardsMutex;
        std::unordered_map<std::string, std::vector<LeaderboardEntry>> m_leaderboards;

        mutable std::shared_mutex m_notificationsMutex;
        std::vector<AchievementNotification> m_pendingNotifications;

        // Event triggers
        mutable std::shared_mutex m_triggersMutex;
        std::unordered_map<std::string, std::vector<std::pair<std::string, std::function<bool(const EventBase&)>>>> m_eventTriggers;

        // Statistics
        mutable std::shared_mutex m_statsMutex;
        AchievementStats m_playerStats;

        // Configuration
        std::string m_playerId;
        bool m_autoSave;
        bool m_notificationsEnabled;
        bool m_cloudSyncEnabled;
        bool m_challengeNotifications;
        std::chrono::seconds m_autoSaveInterval;
        std::chrono::steady_clock::time_point m_lastAutoSave;

        // Internal methods
        bool LoadAchievementDefinitions();
        bool InitializeDefaultAchievements();

        void UpdateAchievementState(const std::string& achievementId);
        void CheckAchievementConditions(const std::string& achievementId);
        void ProcessAchievementUnlock(const std::string& achievementId);

        void GrantRewards(const std::string& achievementId);
        void CreateNotification(const std::string& achievementId);

        bool EvaluateCondition(const AchievementCondition& condition) const;
        bool CheckRequirements(const AchievementRequirement& requirements) const;

        void UpdateStatistics(const std::string& achievementId, bool unlocked);
        void UpdateDailyStats();

        void SaveProgress();
        void LoadProgress();

        void ProcessEventTriggers(const EventBase& event);

        // Achievement definitions for different categories
        void InitializeExplorationAchievements();
        void InitializeCombatAchievements();
        void InitializeCraftingAchievements();
        void InitializeSocialAchievements();
        void InitializeCollectionAchievements();
        void InitializeBuildingAchievements();
        void InitializeMagicAchievements();
        void InitializeFarmingAchievements();
        void InitializeVehicleAchievements();
        void InitializeHiddenAchievements();
    };

    // Achievement system utility macros
    #define VOXELCRAFT_REGISTER_ACHIEVEMENT(def) \
        AchievementSystem::GetInstance().RegisterAchievement(def)

    #define VOXELCRAFT_UNLOCK_ACHIEVEMENT(id) \
        AchievementSystem::GetInstance().UnlockAchievement(id)

    #define VOXELCRAFT_UPDATE_ACHIEVEMENT_PROGRESS(id, progress) \
        AchievementSystem::GetInstance().UpdateAchievementProgress(id, progress)

    #define VOXELCRAFT_IS_ACHIEVEMENT_UNLOCKED(id) \
        AchievementSystem::GetInstance().IsAchievementUnlocked(id)

} // namespace VoxelCraft

#endif // VOXELCRAFT_ACHIEVEMENT_ACHIEVEMENT_SYSTEM_HPP
