/**
 * @file AchievementSystem.cpp
 * @brief VoxelCraft Advanced Achievement System Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#include "AchievementSystem.hpp"

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

#include "../utils/Random.hpp"
#include "../logging/Logger.hpp"
#include "../event/EventSystem.hpp"

namespace VoxelCraft {

    // AchievementSystem implementation
    AchievementSystem& AchievementSystem::GetInstance() {
        static AchievementSystem instance;
        return instance;
    }

    AchievementSystem::AchievementSystem()
        : m_playerId("default_player"),
          m_autoSave(true),
          m_notificationsEnabled(true),
          m_cloudSyncEnabled(false),
          m_challengeNotifications(true),
          m_autoSaveInterval(std::chrono::seconds(60)) {
    }

    AchievementSystem::~AchievementSystem() {
        Shutdown();
    }

    bool AchievementSystem::Initialize() {
        if (!System::Initialize()) {
            return false;
        }

        Logger::GetInstance().Info("AchievementSystem initializing", "AchievementSystem");

        // Initialize achievement database
        if (!LoadAchievementDefinitions()) {
            Logger::GetInstance().Warning("Failed to load achievement definitions", "AchievementSystem");
        }

        // Initialize default achievements
        InitializeDefaultAchievements();

        // Load player progress
        LoadProgress();

        // Register event handlers
        RegisterEventHandlers();

        Logger::GetInstance().Info("AchievementSystem initialized successfully", "AchievementSystem");
        return true;
    }

    void AchievementSystem::Update(float deltaTime) {
        System::Update(deltaTime);

        // Check for achievement updates
        CheckAllAchievements();

        // Update challenges
        UpdateChallenges(deltaTime);

        // Auto-save progress
        if (m_autoSave) {
            auto now = std::chrono::steady_clock::now();
            if (now - m_lastAutoSave >= m_autoSaveInterval) {
                SaveProgress();
                m_lastAutoSave = now;
            }
        }

        // Update daily statistics
        UpdateDailyStats();
    }

    void AchievementSystem::Shutdown() {
        if (!System::Shutdown()) {
            return;
        }

        Logger::GetInstance().Info("AchievementSystem shutting down", "AchievementSystem");

        // Save progress
        SaveProgress();

        // Clear data
        {
            std::unique_lock<std::shared_mutex> lock(m_achievementsMutex);
            m_achievementDefinitions.clear();
            m_playerProgress.clear();
        }

        {
            std::unique_lock<std::shared_mutex> lock(m_challengesMutex);
            m_challenges.clear();
        }

        {
            std::unique_lock<std::shared_mutex> lock(m_leaderboardsMutex);
            m_leaderboards.clear();
        }

        {
            std::unique_lock<std::shared_mutex> lock(m_notificationsMutex);
            m_pendingNotifications.clear();
        }

        Logger::GetInstance().Info("AchievementSystem shutdown complete", "AchievementSystem");
    }

    bool AchievementSystem::RegisterAchievement(const AchievementDefinition& definition) {
        std::unique_lock<std::shared_mutex> lock(m_achievementsMutex);

        if (m_achievementDefinitions.find(definition.id) != m_achievementDefinitions.end()) {
            return false; // Already registered
        }

        m_achievementDefinitions[definition.id] = definition;

        // Initialize progress data
        AchievementProgressData progress;
        progress.achievementId = definition.id;
        progress.state = definition.isHidden ? AchievementState::HIDDEN : AchievementState::VISIBLE;
        progress.currentValue = GetInitialProgressValue(definition.progressType);

        m_playerProgress[definition.id] = progress;

        // Register event triggers
        RegisterAchievementTriggers(definition);

        Logger::GetInstance().Debug("Registered achievement: " + definition.name, "AchievementSystem");
        return true;
    }

    bool AchievementSystem::UnregisterAchievement(const std::string& achievementId) {
        std::unique_lock<std::shared_mutex> lock(m_achievementsMutex);

        auto defIt = m_achievementDefinitions.find(achievementId);
        if (defIt == m_achievementDefinitions.end()) {
            return false;
        }

        m_achievementDefinitions.erase(defIt);
        m_playerProgress.erase(achievementId);

        // Unregister event triggers
        UnregisterAchievementTriggers(achievementId);

        Logger::GetInstance().Debug("Unregistered achievement: " + achievementId, "AchievementSystem");
        return true;
    }

    std::shared_ptr<AchievementDefinition> AchievementSystem::GetAchievement(const std::string& achievementId) const {
        std::shared_lock<std::shared_mutex> lock(m_achievementsMutex);

        auto it = m_achievementDefinitions.find(achievementId);
        if (it != m_achievementDefinitions.end()) {
            return std::make_shared<AchievementDefinition>(it->second);
        }

        return nullptr;
    }

    std::vector<std::string> AchievementSystem::GetAllAchievements() const {
        std::shared_lock<std::shared_mutex> lock(m_achievementsMutex);

        std::vector<std::string> achievements;
        for (const auto& pair : m_achievementDefinitions) {
            achievements.push_back(pair.first);
        }

        return achievements;
    }

    std::vector<std::string> AchievementSystem::GetAchievementsByType(AchievementType type) const {
        std::shared_lock<std::shared_mutex> lock(m_achievementsMutex);

        std::vector<std::string> achievements;
        for (const auto& pair : m_achievementDefinitions) {
            if (pair.second.type == type) {
                achievements.push_back(pair.first);
            }
        }

        return achievements;
    }

    std::vector<std::string> AchievementSystem::GetAchievementsByCategory(const std::string& category) const {
        std::shared_lock<std::shared_mutex> lock(m_achievementsMutex);

        std::vector<std::string> achievements;
        for (const auto& pair : m_achievementDefinitions) {
            if (pair.second.category == category) {
                achievements.push_back(pair.first);
            }
        }

        return achievements;
    }

    bool AchievementSystem::UpdateAchievementProgress(const std::string& achievementId, const std::any& progress) {
        std::unique_lock<std::shared_mutex> lock(m_achievementsMutex);

        auto progressIt = m_playerProgress.find(achievementId);
        if (progressIt == m_playerProgress.end()) {
            return false;
        }

        auto& progressData = progressIt->second;
        progressData.currentValue = progress;
        progressData.lastUpdatedTime = std::chrono::system_clock::now();

        if (progressData.state == AchievementState::VISIBLE) {
            progressData.state = AchievementState::IN_PROGRESS;
        }

        // Check if achievement should be unlocked
        if (CheckAchievementCompletion(achievementId)) {
            UnlockAchievement(achievementId);
        }

        Logger::GetInstance().Debug("Updated achievement progress: " + achievementId, "AchievementSystem");
        return true;
    }

    AchievementProgressData AchievementSystem::GetAchievementProgress(const std::string& achievementId) const {
        std::shared_lock<std::shared_mutex> lock(m_achievementsMutex);

        auto it = m_playerProgress.find(achievementId);
        return it != m_playerProgress.end() ? it->second : AchievementProgressData();
    }

    float AchievementSystem::GetAchievementProgressPercentage(const std::string& achievementId) const {
        std::shared_lock<std::shared_mutex> lock(m_achievementsMutex);

        auto defIt = m_achievementDefinitions.find(achievementId);
        auto progressIt = m_playerProgress.find(achievementId);

        if (defIt == m_achievementDefinitions.end() || progressIt == m_playerProgress.end()) {
            return 0.0f;
        }

        const auto& definition = defIt->second;
        const auto& progress = progressIt->second;

        return CalculateProgressPercentage(definition, progress);
    }

    bool AchievementSystem::UnlockAchievement(const std::string& achievementId) {
        std::unique_lock<std::shared_mutex> lock(m_achievementsMutex);

        auto defIt = m_achievementDefinitions.find(achievementId);
        auto progressIt = m_playerProgress.find(achievementId);

        if (defIt == m_achievementDefinitions.end() || progressIt == m_playerProgress.end()) {
            return false;
        }

        auto& progress = progressIt->second;

        if (progress.state == AchievementState::UNLOCKED) {
            return false; // Already unlocked
        }

        // Check requirements
        if (!CheckRequirements(defIt->second.requirements)) {
            return false;
        }

        // Unlock achievement
        progress.state = AchievementState::COMPLETED;
        progress.unlockedTime = std::chrono::system_clock::now();
        progress.completionCount++;
        progress.completionTimes.push_back(progress.unlockedTime);

        // Grant rewards
        GrantRewards(achievementId);

        // Create notification
        if (m_notificationsEnabled) {
            CreateNotification(achievementId);
        }

        // Update statistics
        UpdateStatistics(achievementId, true);

        Logger::GetInstance().Info("Achievement unlocked: " + defIt->second.name, "AchievementSystem");
        return true;
    }

    bool AchievementSystem::IsAchievementUnlocked(const std::string& achievementId) const {
        auto progress = GetAchievementProgress(achievementId);
        return progress.state == AchievementState::UNLOCKED || progress.state == AchievementState::COMPLETED;
    }

    bool AchievementSystem::CanUnlockAchievement(const std::string& achievementId) const {
        auto achievement = GetAchievement(achievementId);
        if (!achievement) return false;

        // Check requirements
        return CheckRequirements(achievement->requirements);
    }

    std::vector<std::string> AchievementSystem::GetRecentlyUnlockedAchievements() const {
        std::shared_lock<std::shared_mutex> lock(m_achievementsMutex);

        std::vector<std::pair<std::string, std::chrono::system_clock::time_point>> recent;

        for (const auto& pair : m_playerProgress) {
            if (pair.second.state == AchievementState::UNLOCKED ||
                pair.second.state == AchievementState::COMPLETED) {
                recent.emplace_back(pair.first, pair.second.unlockedTime);
            }
        }

        // Sort by unlock time (most recent first)
        std::sort(recent.begin(), recent.end(),
            [](const auto& a, const auto& b) { return a.second > b.second; });

        std::vector<std::string> result;
        for (size_t i = 0; i < std::min(size_t(10), recent.size()); ++i) {
            result.push_back(recent[i].first);
        }

        return result;
    }

    bool AchievementSystem::ClaimAchievementRewards(const std::string& achievementId) {
        std::unique_lock<std::shared_mutex> lock(m_achievementsMutex);

        auto progressIt = m_playerProgress.find(achievementId);
        if (progressIt == m_playerProgress.end()) {
            return false;
        }

        auto& progress = progressIt->second;
        if (progress.rewardsClaimed || progress.state != AchievementState::COMPLETED) {
            return false;
        }

        // Mark as claimed (rewards are granted immediately on unlock)
        progress.rewardsClaimed = true;
        progress.state = AchievementState::UNLOCKED;

        return true;
    }

    bool AchievementSystem::HasUnclaimedRewards(const std::string& achievementId) const {
        auto progress = GetAchievementProgress(achievementId);
        return !progress.rewardsClaimed && progress.state == AchievementState::COMPLETED;
    }

    std::vector<AchievementRewardData> AchievementSystem::GetUnclaimedRewards(const std::string& achievementId) const {
        auto achievement = GetAchievement(achievementId);
        if (!achievement || !HasUnclaimedRewards(achievementId)) {
            return {};
        }

        return achievement->rewards;
    }

    bool AchievementSystem::RevealAchievement(const std::string& achievementId) {
        std::unique_lock<std::shared_mutex> lock(m_achievementsMutex);

        auto progressIt = m_playerProgress.find(achievementId);
        if (progressIt == m_playerProgress.end()) {
            return false;
        }

        auto& progress = progressIt->second;
        if (progress.state != AchievementState::HIDDEN) {
            return false;
        }

        progress.state = AchievementState::VISIBLE;
        return true;
    }

    bool AchievementSystem::IsAchievementVisible(const std::string& achievementId) const {
        auto progress = GetAchievementProgress(achievementId);
        return progress.state != AchievementState::HIDDEN;
    }

    std::vector<std::string> AchievementSystem::GetVisibleAchievements() const {
        std::shared_lock<std::shared_mutex> lock(m_achievementsMutex);

        std::vector<std::string> visible;
        for (const auto& pair : m_playerProgress) {
            if (pair.second.state != AchievementState::HIDDEN) {
                visible.push_back(pair.first);
            }
        }

        return visible;
    }

    AchievementStats AchievementSystem::GetPlayerStats() const {
        std::shared_lock<std::shared_mutex> lock(m_statsMutex);
        return m_playerStats;
    }

    std::vector<std::pair<std::string, int>> AchievementSystem::GetMostUnlockedAchievements() const {
        std::shared_lock<std::shared_mutex> lock(m_achievementsMutex);

        std::vector<std::pair<std::string, int>> mostUnlocked;
        for (const auto& pair : m_playerProgress) {
            if (pair.second.state == AchievementState::UNLOCKED ||
                pair.second.state == AchievementState::COMPLETED) {
                mostUnlocked.emplace_back(pair.first, pair.second.completionCount);
            }
        }

        // Sort by completion count
        std::sort(mostUnlocked.begin(), mostUnlocked.end(),
            [](const auto& a, const auto& b) { return a.second > b.second; });

        return mostUnlocked;
    }

    std::vector<std::pair<std::string, int>> AchievementSystem::GetHardestAchievements() const {
        // Implementation would analyze achievement completion rates
        // For now, return based on rarity
        std::shared_lock<std::shared_mutex> lock(m_achievementsMutex);

        std::vector<std::pair<std::string, int>> hardest;
        for (const auto& pair : m_achievementDefinitions) {
            int difficulty = static_cast<int>(pair.second.rarity);
            hardest.emplace_back(pair.first, difficulty);
        }

        std::sort(hardest.begin(), hardest.end(),
            [](const auto& a, const auto& b) { return a.second > b.second; });

        return hardest;
    }

    std::vector<std::pair<std::string, std::chrono::milliseconds>> AchievementSystem::GetFastestUnlockedAchievements() const {
        std::shared_lock<std::shared_mutex> lock(m_achievementsMutex);

        std::vector<std::pair<std::string, std::chrono::milliseconds>> fastest;
        for (const auto& pair : m_playerProgress) {
            if (pair.second.state == AchievementState::UNLOCKED ||
                pair.second.state == AchievementState::COMPLETED) {
                // Calculate time to unlock (simplified)
                auto timeToUnlock = std::chrono::milliseconds(1000); // Placeholder
                fastest.emplace_back(pair.first, timeToUnlock);
            }
        }

        std::sort(fastest.begin(), fastest.end(),
            [](const auto& a, const auto& b) { return a.second < b.second; });

        return fastest;
    }

    bool AchievementSystem::StartChallenge(const std::string& challengeId) {
        std::unique_lock<std::shared_mutex> lock(m_challengesMutex);

        auto it = m_challenges.find(challengeId);
        if (it == m_challenges.end()) {
            return false;
        }

        auto& challenge = it->second;
        challenge.isActive = true;
        challenge.startTime = std::chrono::system_clock::now();

        return true;
    }

    bool AchievementSystem::EndChallenge(const std::string& challengeId) {
        std::unique_lock<std::shared_mutex> lock(m_challengesMutex);

        auto it = m_challenges.find(challengeId);
        if (it == m_challenges.end()) {
            return false;
        }

        auto& challenge = it->second;
        challenge.isActive = false;
        challenge.endTime = std::chrono::system_clock::now();

        // Grant bonus rewards if completed
        if (IsChallengeCompleted(challengeId)) {
            GrantChallengeRewards(challengeId);
        }

        return true;
    }

    AchievementChallenge AchievementSystem::GetChallenge(const std::string& challengeId) const {
        std::shared_lock<std::shared_mutex> lock(m_challengesMutex);

        auto it = m_challenges.find(challengeId);
        return it != m_challenges.end() ? it->second : AchievementChallenge();
    }

    std::vector<AchievementChallenge> AchievementSystem::GetActiveChallenges() const {
        std::shared_lock<std::shared_mutex> lock(m_challengesMutex);

        std::vector<AchievementChallenge> active;
        for (const auto& pair : m_challenges) {
            if (pair.second.isActive) {
                active.push_back(pair.second);
            }
        }

        return active;
    }

    bool AchievementSystem::SubmitScore(const std::string& leaderboardId, int score) {
        std::unique_lock<std::shared_mutex> lock(m_leaderboardsMutex);

        auto& leaderboard = m_leaderboards[leaderboardId];

        // Find existing entry or create new one
        auto it = std::find_if(leaderboard.begin(), leaderboard.end(),
            [this](const LeaderboardEntry& entry) { return entry.playerId == m_playerId; });

        if (it != leaderboard.end()) {
            it->score = score;
            it->lastUpdated = std::chrono::system_clock::now();
        } else {
            LeaderboardEntry entry;
            entry.playerId = m_playerId;
            entry.playerName = m_playerId; // Would get actual player name
            entry.score = score;
            entry.lastUpdated = std::chrono::system_clock::now();
            leaderboard.push_back(entry);
        }

        // Sort leaderboard
        std::sort(leaderboard.begin(), leaderboard.end(),
            [](const LeaderboardEntry& a, const LeaderboardEntry& b) {
                return a.score > b.score;
            });

        // Update ranks
        for (size_t i = 0; i < leaderboard.size(); ++i) {
            leaderboard[i].rank = static_cast<int>(i + 1);
        }

        return true;
    }

    std::vector<LeaderboardEntry> AchievementSystem::GetLeaderboard(const std::string& leaderboardId, int limit) const {
        std::shared_lock<std::shared_mutex> lock(m_leaderboardsMutex);

        auto it = m_leaderboards.find(leaderboardId);
        if (it == m_leaderboards.end()) {
            return {};
        }

        const auto& leaderboard = it->second;
        size_t count = std::min(static_cast<size_t>(limit), leaderboard.size());

        return std::vector<LeaderboardEntry>(leaderboard.begin(), leaderboard.begin() + count);
    }

    LeaderboardEntry AchievementSystem::GetPlayerRank(const std::string& leaderboardId) const {
        std::shared_lock<std::shared_mutex> lock(m_leaderboardsMutex);

        auto it = m_leaderboards.find(leaderboardId);
        if (it == m_leaderboards.end()) {
            return LeaderboardEntry();
        }

        const auto& leaderboard = it->second;
        auto playerIt = std::find_if(leaderboard.begin(), leaderboard.end(),
            [this](const LeaderboardEntry& entry) { return entry.playerId == m_playerId; });

        return playerIt != leaderboard.end() ? *playerIt : LeaderboardEntry();
    }

    bool AchievementSystem::ShowNotification(const AchievementNotification& notification) {
        std::unique_lock<std::shared_mutex> lock(m_notificationsMutex);
        m_pendingNotifications.push_back(notification);
        return true;
    }

    std::vector<AchievementNotification> AchievementSystem::GetPendingNotifications() const {
        std::shared_lock<std::shared_mutex> lock(m_notificationsMutex);
        return m_pendingNotifications;
    }

    void AchievementSystem::ClearNotifications() {
        std::unique_lock<std::shared_mutex> lock(m_notificationsMutex);
        m_pendingNotifications.clear();
    }

    void AchievementSystem::OnEvent(const EventBase& event) {
        // Process event triggers
        ProcessEventTriggers(event);
    }

    bool AchievementSystem::RegisterEventTrigger(const std::string& achievementId, const std::string& eventName,
                                               const std::function<bool(const EventBase&)>& condition) {
        std::unique_lock<std::shared_mutex> lock(m_triggersMutex);
        m_eventTriggers[eventName].emplace_back(achievementId, condition);
        return true;
    }

    bool AchievementSystem::SyncWithCloud() {
        if (!m_cloudSyncEnabled) {
            return false;
        }

        // Implement cloud synchronization
        return true;
    }

    bool AchievementSystem::LoadFromCloud() {
        if (!m_cloudSyncEnabled) {
            return false;
        }

        // Implement cloud loading
        return true;
    }

    bool AchievementSystem::SaveToCloud() {
        if (!m_cloudSyncEnabled) {
            return false;
        }

        // Implement cloud saving
        return true;
    }

    bool AchievementSystem::IsCloudSyncEnabled() const {
        return m_cloudSyncEnabled;
    }

    bool AchievementSystem::LoadAchievementDatabase(const std::string& path) {
        // Implement database loading
        return true;
    }

    bool AchievementSystem::SaveAchievementDatabase(const std::string& path) {
        // Implement database saving
        return true;
    }

    bool AchievementSystem::ImportAchievements(const std::string& data) {
        // Implement achievement import
        return true;
    }

    bool AchievementSystem::ExportAchievements(const std::string& path) const {
        // Implement achievement export
        return true;
    }

    bool AchievementSystem::LoadPlayerProgress(const std::string& playerId) {
        m_playerId = playerId;
        return LoadProgress();
    }

    bool AchievementSystem::SavePlayerProgress(const std::string& playerId) {
        m_playerId = playerId;
        return SaveProgress();
    }

    bool AchievementSystem::ResetPlayerProgress() {
        std::unique_lock<std::shared_mutex> lock(m_achievementsMutex);

        for (auto& pair : m_playerProgress) {
            pair.second.state = AchievementState::VISIBLE;
            pair.second.currentValue = GetInitialProgressValue(
                m_achievementDefinitions[pair.first].progressType);
            pair.second.completionCount = 0;
            pair.second.completionTimes.clear();
            pair.second.rewardsClaimed = false;
        }

        m_playerStats = AchievementStats();
        return true;
    }

    bool AchievementSystem::TransferProgress(const std::string& fromPlayerId, const std::string& toPlayerId) {
        // Implement progress transfer
        return true;
    }

    std::string AchievementSystem::GetAchievementInfo(const std::string& achievementId) const {
        auto achievement = GetAchievement(achievementId);
        if (!achievement) return "Achievement not found";

        auto progress = GetAchievementProgress(achievementId);

        std::stringstream ss;
        ss << "Achievement: " << achievement->name << "\n";
        ss << "Description: " << achievement->description << "\n";
        ss << "Type: " << static_cast<int>(achievement->type) << "\n";
        ss << "Rarity: " << static_cast<int>(achievement->rarity) << "\n";
        ss << "State: " << static_cast<int>(progress.state) << "\n";
        ss << "Progress: " << GetAchievementProgressPercentage(achievementId) << "%\n";

        return ss.str();
    }

    void AchievementSystem::PrintAchievementReport() const {
        auto stats = GetPlayerStats();
        std::stringstream report;

        report << "=== Achievement System Report ===\n";
        report << "Total Achievements: " << stats.totalAchievements << "\n";
        report << "Unlocked Achievements: " << stats.unlockedAchievements << "\n";
        report << "Hidden Achievements: " << stats.hiddenAchievements << "\n";
        report << "In Progress: " << stats.inProgressAchievements << "\n";
        report << "Completion Percentage: " << stats.completionPercentage << "%\n";
        report << "Total Experience Gained: " << stats.totalExperienceGained << "\n";
        report << "Total Currency Gained: " << stats.totalCurrencyGained << "\n";
        report << "Completion Streak: " << stats.streakDays << " days\n";

        Logger::GetInstance().Info(report.str(), "AchievementSystem");
    }

    bool AchievementSystem::ValidateAchievement(const std::string& achievementId) const {
        auto achievement = GetAchievement(achievementId);
        if (!achievement) return false;

        // Validate achievement structure
        return !achievement->id.empty() && !achievement->name.empty();
    }

    bool AchievementSystem::ValidateAllAchievements() const {
        std::shared_lock<std::shared_mutex> lock(m_achievementsMutex);

        for (const auto& pair : m_achievementDefinitions) {
            if (!ValidateAchievement(pair.first)) {
                return false;
            }
        }

        return true;
    }

    void AchievementSystem::ResetAchievement(const std::string& achievementId) {
        std::unique_lock<std::shared_mutex> lock(m_achievementsMutex);

        auto it = m_playerProgress.find(achievementId);
        if (it != m_playerProgress.end()) {
            auto defIt = m_achievementDefinitions.find(achievementId);
            if (defIt != m_achievementDefinitions.end()) {
                it->second.state = defIt->second.isHidden ? AchievementState::HIDDEN : AchievementState::VISIBLE;
                it->second.currentValue = GetInitialProgressValue(defIt->second.progressType);
                it->second.completionCount = 0;
                it->second.completionTimes.clear();
                it->second.rewardsClaimed = false;
            }
        }
    }

    void AchievementSystem::ResetAllAchievements() {
        ResetPlayerProgress();
    }

    void AchievementSystem::DebugTriggerAchievement(const std::string& achievementId) {
        // Debug method to force unlock an achievement
        UpdateAchievementProgress(achievementId, GetMaxProgressValue(achievementId));
    }

    void AchievementSystem::DebugSetAchievementProgress(const std::string& achievementId, const std::any& progress) {
        UpdateAchievementProgress(achievementId, progress);
    }

    std::vector<std::string> AchievementSystem::DebugGetDebugAchievements() const {
        // Return achievements marked for debugging
        return {};
    }

    // Private methods implementation
    bool AchievementSystem::LoadAchievementDefinitions() {
        // Load achievement definitions from file or database
        return true;
    }

    bool AchievementSystem::InitializeDefaultAchievements() {
        // Initialize exploration achievements
        InitializeExplorationAchievements();

        // Initialize combat achievements
        InitializeCombatAchievements();

        // Initialize crafting achievements
        InitializeCraftingAchievements();

        // Initialize social achievements
        InitializeSocialAchievements();

        // Initialize collection achievements
        InitializeCollectionAchievements();

        // Initialize building achievements
        InitializeBuildingAchievements();

        // Initialize magic achievements
        InitializeMagicAchievements();

        // Initialize farming achievements
        InitializeFarmingAchievements();

        // Initialize vehicle achievements
        InitializeVehicleAchievements();

        // Initialize hidden achievements
        InitializeHiddenAchievements();

        return true;
    }

    void AchievementSystem::RegisterEventHandlers() {
        // Register with event system to receive game events
    }

    void AchievementSystem::CheckAllAchievements() {
        std::shared_lock<std::shared_mutex> lock(m_achievementsMutex);

        for (const auto& pair : m_achievementDefinitions) {
            CheckAchievementConditions(pair.first);
        }
    }

    void AchievementSystem::UpdateChallenges(float deltaTime) {
        // Update active challenges
    }

    void AchievementSystem::UpdateDailyStats() {
        // Update daily achievement statistics
    }

    void AchievementSystem::UpdateAchievementState(const std::string& achievementId) {
        // Update achievement state based on progress
    }

    void AchievementSystem::CheckAchievementConditions(const std::string& achievementId) {
        auto achievement = GetAchievement(achievementId);
        if (!achievement) return;

        bool allConditionsMet = true;
        for (const auto& condition : achievement->conditions) {
            if (!EvaluateCondition(condition)) {
                allConditionsMet = false;
                break;
            }
        }

        if (allConditionsMet && CheckRequirements(achievement->requirements)) {
            UnlockAchievement(achievementId);
        }
    }

    void AchievementSystem::ProcessAchievementUnlock(const std::string& achievementId) {
        // Process achievement unlock logic
        GrantRewards(achievementId);
        CreateNotification(achievementId);
        UpdateStatistics(achievementId, true);
    }

    void AchievementSystem::GrantRewards(const std::string& achievementId) {
        auto achievement = GetAchievement(achievementId);
        if (!achievement) return;

        for (const auto& reward : achievement->rewards) {
            // Grant reward based on type
            switch (reward.type) {
                case AchievementRewardType::EXPERIENCE:
                    // Grant experience
                    break;
                case AchievementRewardType::ITEM:
                    // Grant item
                    break;
                case AchievementRewardType::CURRENCY:
                    // Grant currency
                    break;
                // Handle other reward types...
                default:
                    break;
            }
        }
    }

    void AchievementSystem::CreateNotification(const std::string& achievementId) {
        auto achievement = GetAchievement(achievementId);
        if (!achievement) return;

        AchievementNotification notification;
        notification.achievementId = achievementId;
        notification.title = achievement->name;
        notification.description = achievement->description;
        notification.rarity = achievement->rarity;
        notification.rewards = achievement->rewards;
        notification.timestamp = std::chrono::system_clock::now();

        ShowNotification(notification);
    }

    bool AchievementSystem::EvaluateCondition(const AchievementCondition& condition) const {
        // Evaluate achievement condition
        return true;
    }

    bool AchievementSystem::CheckRequirements(const AchievementRequirement& requirements) const {
        // Check achievement requirements
        return true;
    }

    void AchievementSystem::UpdateStatistics(const std::string& achievementId, bool unlocked) {
        std::unique_lock<std::shared_mutex> lock(m_statsMutex);

        if (unlocked) {
            m_playerStats.unlockedAchievements++;
            m_playerStats.completionPercentage =
                (static_cast<float>(m_playerStats.unlockedAchievements) / m_playerStats.totalAchievements) * 100.0f;

            auto achievement = GetAchievement(achievementId);
            if (achievement) {
                m_playerStats.achievementsByRarity[achievement->rarity]++;
                m_playerStats.achievementsByType[achievement->type]++;
                m_playerStats.achievementsByCategory[achievement->category]++;
            }
        }
    }

    void AchievementSystem::SaveProgress() {
        // Save player progress to file
    }

    void AchievementSystem::LoadProgress() {
        // Load player progress from file
    }

    void AchievementSystem::ProcessEventTriggers(const EventBase& event) {
        std::shared_lock<std::shared_mutex> lock(m_triggersMutex);

        auto it = m_eventTriggers.find(event.GetTypeName());
        if (it != m_eventTriggers.end()) {
            for (const auto& trigger : it->second) {
                if (trigger.second(event)) {
                    // Trigger achievement check
                    CheckAchievementConditions(trigger.first);
                }
            }
        }
    }

    void AchievementSystem::RegisterAchievementTriggers(const AchievementDefinition& definition) {
        for (const auto& condition : definition.conditions) {
            if (condition.trigger == AchievementTrigger::EVENT) {
                if (std::holds_alternative<std::string>(condition.triggerData)) {
                    std::string eventName = std::get<std::string>(condition.triggerData);
                    RegisterEventTrigger(definition.id, eventName,
                        [condition](const EventBase& event) {
                            // Check if event matches condition
                            return true;
                        });
                }
            }
        }
    }

    void AchievementSystem::UnregisterAchievementTriggers(const std::string& achievementId) {
        // Unregister event triggers for achievement
    }

    std::any AchievementSystem::GetInitialProgressValue(AchievementProgressType type) const {
        switch (type) {
            case AchievementProgressType::BOOLEAN: return false;
            case AchievementProgressType::INTEGER: return 0;
            case AchievementProgressType::FLOAT: return 0.0f;
            case AchievementProgressType::PERCENTAGE: return 0.0f;
            case AchievementProgressType::TIME: return std::chrono::milliseconds(0);
            case AchievementProgressType::MULTI_COUNTER: return std::vector<std::pair<std::string, int>>();
            default: return std::any();
        }
    }

    std::any AchievementSystem::GetMaxProgressValue(const std::string& achievementId) const {
        auto achievement = GetAchievement(achievementId);
        if (!achievement) return std::any();

        return achievement->targetValue;
    }

    float AchievementSystem::CalculateProgressPercentage(const AchievementDefinition& definition,
                                                       const AchievementProgressData& progress) const {
        // Calculate progress percentage based on type
        return 0.0f;
    }

    bool AchievementSystem::CheckAchievementCompletion(const std::string& achievementId) const {
        auto achievement = GetAchievement(achievementId);
        if (!achievement) return false;

        auto progress = GetAchievementProgress(achievementId);

        // Check if progress meets target value
        return GetAchievementProgressPercentage(achievementId) >= 100.0f;
    }

    bool AchievementSystem::IsChallengeCompleted(const std::string& challengeId) const {
        // Check if challenge requirements are met
        return false;
    }

    void AchievementSystem::GrantChallengeRewards(const std::string& challengeId) {
        // Grant challenge bonus rewards
    }

    // Achievement category initialization methods
    void AchievementSystem::InitializeExplorationAchievements() {
        // First Steps
        AchievementDefinition firstSteps;
        firstSteps.id = "exploration_first_steps";
        firstSteps.name = "First Steps";
        firstSteps.description = "Take your first steps in the world";
        firstSteps.type = AchievementType::EXPLORATION;
        firstSteps.rarity = AchievementRarity::COMMON;
        firstSteps.progressType = AchievementProgressType::INTEGER;
        firstSteps.targetValue = 1;
        RegisterAchievement(firstSteps);

        // World Explorer
        AchievementDefinition worldExplorer;
        worldExplorer.id = "exploration_world_explorer";
        worldExplorer.name = "World Explorer";
        worldExplorer.description = "Explore 10 different biomes";
        worldExplorer.type = AchievementType::EXPLORATION;
        worldExplorer.rarity = AchievementRarity::UNCOMMON;
        worldExplorer.progressType = AchievementProgressType::INTEGER;
        worldExplorer.targetValue = 10;
        RegisterAchievement(worldExplorer);

        // Mountain Climber
        AchievementDefinition mountainClimber;
        mountainClimber.id = "exploration_mountain_climber";
        mountainClimber.name = "Mountain Climber";
        mountainClimber.description = "Reach the highest point in the world";
        mountainClimber.type = AchievementType::EXPLORATION;
        mountainClimber.rarity = AchievementRarity::RARE;
        mountainClimber.progressType = AchievementProgressType::BOOLEAN;
        mountainClimber.targetValue = true;
        RegisterAchievement(mountainClimber);
    }

    void AchievementSystem::InitializeCombatAchievements() {
        // First Blood
        AchievementDefinition firstBlood;
        firstBlood.id = "combat_first_blood";
        firstBlood.name = "First Blood";
        firstBlood.description = "Defeat your first enemy";
        firstBlood.type = AchievementType::COMBAT;
        firstBlood.rarity = AchievementRarity::COMMON;
        firstBlood.progressType = AchievementProgressType::INTEGER;
        firstBlood.targetValue = 1;
        RegisterAchievement(firstBlood);

        // Warrior
        AchievementDefinition warrior;
        warrior.id = "combat_warrior";
        warrior.name = "Warrior";
        warrior.description = "Defeat 100 enemies";
        warrior.type = AchievementType::COMBAT;
        warrior.rarity = AchievementRarity::UNCOMMON;
        warrior.progressType = AchievementProgressType::INTEGER;
        warrior.targetValue = 100;
        RegisterAchievement(warrior);

        // Unstoppable
        AchievementDefinition unstoppable;
        unstoppable.id = "combat_unstoppable";
        unstoppable.name = "Unstoppable";
        unstoppable.description = "Defeat 10 enemies without taking damage";
        unstoppable.type = AchievementType::COMBAT;
        unstoppable.rarity = AchievementRarity::EPIC;
        unstoppable.progressType = AchievementProgressType::INTEGER;
        unstoppable.targetValue = 10;
        RegisterAchievement(unstoppable);
    }

    void AchievementSystem::InitializeCraftingAchievements() {
        // First Craft
        AchievementDefinition firstCraft;
        firstCraft.id = "crafting_first_craft";
        firstCraft.name = "First Craft";
        firstCraft.description = "Craft your first item";
        firstCraft.type = AchievementType::CREATIVE;
        firstCraft.rarity = AchievementRarity::COMMON;
        firstCraft.progressType = AchievementProgressType::INTEGER;
        firstCraft.targetValue = 1;
        RegisterAchievement(firstCraft);

        // Master Craftsman
        AchievementDefinition masterCraftsman;
        masterCraftsman.id = "crafting_master_craftsman";
        masterCraftsman.name = "Master Craftsman";
        masterCraftsman.description = "Craft 1000 items";
        masterCraftsman.type = AchievementType::CREATIVE;
        masterCraftsman.rarity = AchievementRarity::EPIC;
        masterCraftsman.progressType = AchievementProgressType::INTEGER;
        masterCraftsman.targetValue = 1000;
        RegisterAchievement(masterCraftsman);
    }

    void AchievementSystem::InitializeSocialAchievements() {
        // Welcome to the Community
        AchievementDefinition welcomeCommunity;
        welcomeCommunity.id = "social_welcome_community";
        welcomeCommunity.name = "Welcome to the Community";
        welcomeCommunity.description = "Join your first multiplayer game";
        welcomeCommunity.type = AchievementType::SOCIAL;
        welcomeCommunity.rarity = AchievementRarity::COMMON;
        welcomeCommunity.progressType = AchievementProgressType::INTEGER;
        welcomeCommunity.targetValue = 1;
        RegisterAchievement(welcomeCommunity);

        // Team Player
        AchievementDefinition teamPlayer;
        teamPlayer.id = "social_team_player";
        teamPlayer.name = "Team Player";
        teamPlayer.description = "Complete 50 team objectives";
        teamPlayer.type = AchievementType::SOCIAL;
        teamPlayer.rarity = AchievementRarity::RARE;
        teamPlayer.progressType = AchievementProgressType::INTEGER;
        teamPlayer.targetValue = 50;
        RegisterAchievement(teamPlayer);
    }

    void AchievementSystem::InitializeCollectionAchievements() {
        // Collector
        AchievementDefinition collector;
        collector.id = "collection_collector";
        collector.name = "Collector";
        collector.description = "Collect 50 different items";
        collector.type = AchievementType::COLLECTOR;
        collector.rarity = AchievementRarity::UNCOMMON;
        collector.progressType = AchievementProgressType::INTEGER;
        collector.targetValue = 50;
        RegisterAchievement(collector);

        // Hoarder
        AchievementDefinition hoarder;
        hoarder.id = "collection_hoarder";
        hoarder.name = "Hoarder";
        hoarder.description = "Have 10,000 items in your inventory";
        hoarder.type = AchievementType::COLLECTOR;
        hoarder.rarity = AchievementRarity::RARE;
        hoarder.progressType = AchievementProgressType::INTEGER;
        hoarder.targetValue = 10000;
        RegisterAchievement(hoarder);
    }

    void AchievementSystem::InitializeBuildingAchievements() {
        // First House
        AchievementDefinition firstHouse;
        firstHouse.id = "building_first_house";
        firstHouse.name = "First House";
        firstHouse.description = "Build your first house";
        firstHouse.type = AchievementType::CREATIVE;
        firstHouse.rarity = AchievementRarity::COMMON;
        firstHouse.progressType = AchievementProgressType::INTEGER;
        firstHouse.targetValue = 1;
        RegisterAchievement(firstHouse);

        // Architect
        AchievementDefinition architect;
        architect.id = "building_architect";
        architect.name = "Architect";
        architect.description = "Build structures with 1000+ blocks";
        architect.type = AchievementType::CREATIVE;
        architect.rarity = AchievementRarity::EPIC;
        architect.progressType = AchievementProgressType::INTEGER;
        architect.targetValue = 1000;
        RegisterAchievement(architect);
    }

    void AchievementSystem::InitializeMagicAchievements() {
        // First Spell
        AchievementDefinition firstSpell;
        firstSpell.id = "magic_first_spell";
        firstSpell.name = "First Spell";
        firstSpell.description = "Cast your first spell";
        firstSpell.type = AchievementType::SKILL;
        firstSpell.rarity = AchievementRarity::COMMON;
        firstSpell.progressType = AchievementProgressType::INTEGER;
        firstSpell.targetValue = 1;
        RegisterAchievement(firstSpell);

        // Archmage
        AchievementDefinition archmage;
        archmage.id = "magic_archmage";
        archmage.name = "Archmage";
        archmage.description = "Learn all spells";
        archmage.type = AchievementType::SKILL;
        archmage.rarity = AchievementRarity::LEGENDARY;
        archmage.progressType = AchievementProgressType::INTEGER;
        archmage.targetValue = 100; // All spells
        RegisterAchievement(archmage);
    }

    void AchievementSystem::InitializeFarmingAchievements() {
        // Green Thumb
        AchievementDefinition greenThumb;
        greenThumb.id = "farming_green_thumb";
        greenThumb.name = "Green Thumb";
        greenThumb.description = "Harvest your first crop";
        greenThumb.type = AchievementType::SKILL;
        greenThumb.rarity = AchievementRarity::COMMON;
        greenThumb.progressType = AchievementProgressType::INTEGER;
        greenThumb.targetValue = 1;
        RegisterAchievement(greenThumb);

        // Farmer
        AchievementDefinition farmer;
        farmer.id = "farming_farmer";
        farmer.name = "Farmer";
        farmer.description = "Harvest 1000 crops";
        farmer.type = AchievementType::SKILL;
        farmer.rarity = AchievementRarity::UNCOMMON;
        farmer.progressType = AchievementProgressType::INTEGER;
        farmer.targetValue = 1000;
        RegisterAchievement(farmer);
    }

    void AchievementSystem::InitializeVehicleAchievements() {
        // First Ride
        AchievementDefinition firstRide;
        firstRide.id = "vehicle_first_ride";
        firstRide.name = "First Ride";
        firstRide.description = "Ride your first vehicle";
        firstRide.type = AchievementType::SKILL;
        firstRide.rarity = AchievementRarity::COMMON;
        firstRide.progressType = AchievementProgressType::INTEGER;
        firstRide.targetValue = 1;
        RegisterAchievement(firstRide);

        // Speed Demon
        AchievementDefinition speedDemon;
        speedDemon.id = "vehicle_speed_demon";
        speedDemon.name = "Speed Demon";
        speedDemon.description = "Reach maximum speed in any vehicle";
        speedDemon.type = AchievementType::SKILL;
        speedDemon.rarity = AchievementRarity::RARE;
        speedDemon.progressType = AchievementProgressType::BOOLEAN;
        speedDemon.targetValue = true;
        RegisterAchievement(speedDemon);
    }

    void AchievementSystem::InitializeHiddenAchievements() {
        // Secret achievements
        AchievementDefinition secret1;
        secret1.id = "hidden_secret_1";
        secret1.name = "???";
        secret1.description = "Discover a hidden secret";
        secret1.type = AchievementType::HIDDEN;
        secret1.rarity = AchievementRarity::MYTHICAL;
        secret1.isHidden = true;
        secret1.progressType = AchievementProgressType::BOOLEAN;
        secret1.targetValue = true;
        RegisterAchievement(secret1);

        AchievementDefinition secret2;
        secret2.id = "hidden_secret_2";
        secret2.name = "???";
        secret2.description = "Find the legendary treasure";
        secret2.type = AchievementType::HIDDEN;
        secret2.rarity = AchievementRarity::LEGENDARY;
        secret2.isHidden = true;
        secret2.progressType = AchievementProgressType::BOOLEAN;
        secret2.targetValue = true;
        RegisterAchievement(secret2);
    }

} // namespace VoxelCraft
