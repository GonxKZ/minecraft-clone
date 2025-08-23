/**
 * @file SurvivalSystem.cpp
 * @brief VoxelCraft Survival System Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#include "SurvivalSystem.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>

#include "../entities/Player.hpp"
#include "../world/World.hpp"
#include "../math/MathUtils.hpp"
#include "../utils/Logger.hpp"
#include "../utils/Random.hpp"

namespace VoxelCraft {

    // Static instance
    static SurvivalSystem* s_instance = nullptr;

    SurvivalSystem& SurvivalSystem::GetInstance() {
        if (!s_instance) {
            s_instance = new SurvivalSystem();
        }
        return *s_instance;
    }

    bool SurvivalSystem::Initialize(const SurvivalConfig& config) {
        if (m_initialized) {
            Logger::Warning("SurvivalSystem already initialized");
            return true;
        }

        m_config = config;
        m_initialized = true;
        m_debugMode = false;

        Logger::Info("SurvivalSystem initialized with config: health={}, hunger={}, thirst={}",
                    m_config.enableHealth, m_config.enableHunger, m_config.enableThirst);

        return true;
    }

    void SurvivalSystem::Shutdown() {
        if (!m_initialized) {
            return;
        }

        // Clear all data
        m_playerStats.clear();
        m_environmentalEffects.clear();
        m_environmentalMap.clear();

        m_initialized = false;
        Logger::Info("SurvivalSystem shutdown");
    }

    void SurvivalSystem::Update(float deltaTime) {
        if (!m_initialized || !m_config.enableSurvival) {
            return;
        }

        // Update all survival systems
        UpdatePlayerNeeds(deltaTime);
        UpdateEnvironmentalEffects(deltaTime);
        UpdateStatusEffects(deltaTime);
        UpdatePlayerStates(deltaTime);
    }

    bool SurvivalSystem::RegisterPlayer(Player* player) {
        if (!player || !m_initialized) {
            return false;
        }

        std::unique_lock<std::shared_mutex> lock(m_survivalMutex);

        if (m_playerStats.find(player) != m_playerStats.end()) {
            Logger::Warning("Player already registered in SurvivalSystem");
            return false;
        }

        PlayerStats stats;
        stats.health = m_config.maxHealth;
        stats.maxHealth = m_config.maxHealth;
        stats.hunger = m_config.maxHunger;
        stats.thirst = m_config.maxThirst;
        stats.stamina = m_config.maxStamina;
        stats.oxygen = 20.0f;
        stats.sanity = 20.0f;
        stats.temperature = m_config.optimalTemperature;
        stats.sleep = 20.0f;

        m_playerStats[player] = stats;
        m_environmentalEffects[player] = EnvironmentalEffects();

        m_stats.playersTracked++;

        Logger::Info("Player registered in SurvivalSystem");
        return true;
    }

    bool SurvivalSystem::UnregisterPlayer(Player* player) {
        if (!player) {
            return false;
        }

        std::unique_lock<std::shared_mutex> lock(m_survivalMutex);

        auto it = m_playerStats.find(player);
        if (it == m_playerStats.end()) {
            return false;
        }

        m_playerStats.erase(it);
        m_environmentalEffects.erase(player);
        m_stats.playersTracked--;

        Logger::Info("Player unregistered from SurvivalSystem");
        return true;
    }

    const PlayerStats& SurvivalSystem::GetPlayerStats(Player* player) const {
        static PlayerStats s_defaultStats;

        if (!player) {
            return s_defaultStats;
        }

        std::shared_lock<std::shared_mutex> lock(m_survivalMutex);

        auto it = m_playerStats.find(player);
        if (it == m_playerStats.end()) {
            return s_defaultStats;
        }

        return it->second;
    }

    bool SurvivalSystem::UpdatePlayerStats(Player* player, const PlayerStats& stats) {
        if (!player) {
            return false;
        }

        std::unique_lock<std::shared_mutex> lock(m_survivalMutex);

        auto it = m_playerStats.find(player);
        if (it == m_playerStats.end()) {
            return false;
        }

        it->second = stats;
        return true;
    }

    bool SurvivalSystem::ModifyHealth(Player* player, float amount, const std::string& cause) {
        if (!player || !m_config.enableSurvival) {
            return false;
        }

        std::unique_lock<std::shared_mutex> lock(m_survivalMutex);

        auto it = m_playerStats.find(player);
        if (it == m_playerStats.end()) {
            return false;
        }

        PlayerStats& stats = it->second;
        float oldHealth = stats.health;

        stats.health = std::clamp(stats.health + amount, 0.0f, stats.maxHealth);

        if (amount < 0) {
            m_stats.damagesProcessed++;
            if (!cause.empty()) {
                Logger::Debug("Player damaged by {}: {} -> {}", cause, oldHealth, stats.health);
            }
        } else {
            m_stats.healingsProcessed++;
        }

        // Check for death
        if (stats.health <= 0.0f && oldHealth > 0.0f) {
            ProcessPlayerDeath(player, cause);
        }

        return true;
    }

    bool SurvivalSystem::ModifyHunger(Player* player, float amount) {
        if (!player || !m_config.enableHunger) {
            return false;
        }

        std::unique_lock<std::shared_mutex> lock(m_survivalMutex);

        auto it = m_playerStats.find(player);
        if (it == m_playerStats.end()) {
            return false;
        }

        PlayerStats& stats = it->second;
        stats.hunger = std::clamp(stats.hunger + amount, 0.0f, m_config.maxHunger);

        return true;
    }

    bool SurvivalSystem::ModifyThirst(Player* player, float amount) {
        if (!player || !m_config.enableThirst) {
            return false;
        }

        std::unique_lock<std::shared_mutex> lock(m_survivalMutex);

        auto it = m_playerStats.find(player);
        if (it == m_playerStats.end()) {
            return false;
        }

        PlayerStats& stats = it->second;
        stats.thirst = std::clamp(stats.thirst + amount, 0.0f, m_config.maxThirst);

        return true;
    }

    bool SurvivalSystem::ModifyStamina(Player* player, float amount) {
        if (!player || !m_config.enableStamina) {
            return false;
        }

        std::unique_lock<std::shared_mutex> lock(m_survivalMutex);

        auto it = m_playerStats.find(player);
        if (it == m_playerStats.end()) {
            return false;
        }

        PlayerStats& stats = it->second;
        stats.stamina = std::clamp(stats.stamina + amount, 0.0f, m_config.maxStamina);

        return true;
    }

    bool SurvivalSystem::FeedPlayer(Player* player, float foodValue, const std::string& foodType) {
        if (!player || !m_config.enableHunger) {
            return false;
        }

        std::unique_lock<std::shared_mutex> lock(m_survivalMutex);

        auto it = m_playerStats.find(player);
        if (it == m_playerStats.end()) {
            return false;
        }

        PlayerStats& stats = it->second;
        float oldHunger = stats.hunger;

        stats.hunger = std::clamp(stats.hunger + foodValue, 0.0f, m_config.maxHunger);

        // Restore some health based on food type
        float healthBonus = 0.0f;
        if (foodType == "healing") {
            healthBonus = foodValue * 0.5f;
        } else if (foodType == "premium") {
            healthBonus = foodValue * 0.3f;
        } else if (foodType == "raw") {
            healthBonus = -foodValue * 0.1f; // Raw food can cause minor damage
        }

        if (healthBonus != 0.0f) {
            stats.health = std::clamp(stats.health + healthBonus, 0.0f, stats.maxHealth);
        }

        Logger::Debug("Player fed: {} hunger ({} -> {}), health bonus: {}",
                     foodType, oldHunger, stats.hunger, healthBonus);

        return true;
    }

    bool SurvivalSystem::HydratePlayer(Player* player, float waterValue, const std::string& waterType) {
        if (!player || !m_config.enableThirst) {
            return false;
        }

        std::unique_lock<std::shared_mutex> lock(m_survivalMutex);

        auto it = m_playerStats.find(player);
        if (it == m_playerStats.end()) {
            return false;
        }

        PlayerStats& stats = it->second;
        float oldThirst = stats.thirst;

        stats.thirst = std::clamp(stats.thirst + waterValue, 0.0f, m_config.maxThirst);

        // Handle different water types
        if (waterType == "dirty" || waterType == "salt") {
            // Dirty water can cause disease
            stats.disease = std::clamp(stats.disease + 0.5f, 0.0f, 10.0f);
        } else if (waterType == "clean" || waterType == "pure") {
            // Clean water can slightly reduce disease
            stats.disease = std::max(stats.disease - 0.1f, 0.0f);
        }

        Logger::Debug("Player hydrated: {} water ({} -> {})",
                     waterType, oldThirst, stats.thirst);

        return true;
    }

    bool SurvivalSystem::ApplyEnvironmentalEffects(Player* player, const EnvironmentalEffects& effects) {
        if (!player || !m_config.enableEnvironmentalEffects) {
            return false;
        }

        std::unique_lock<std::shared_mutex> lock(m_survivalMutex);

        auto it = m_environmentalEffects.find(player);
        if (it == m_environmentalEffects.end()) {
            return false;
        }

        it->second = effects;
        return true;
    }

    EnvironmentalEffects SurvivalSystem::GetEnvironmentalEffects(const Vec3& position) const {
        std::shared_lock<std::shared_mutex> lock(m_survivalMutex);

        // Check for local environmental effects first
        auto it = m_environmentalMap.find(position);
        if (it != m_environmentalMap.end()) {
            return it->second;
        }

        // Return default environmental effects
        EnvironmentalEffects effects;
        effects.temperature = CalculateEnvironmentalTemperature(position);
        effects.humidity = 50.0f; // Default humidity
        effects.oxygenLevel = 21.0f; // Standard oxygen level
        effects.pressure = 1.0f; // Standard pressure

        return effects;
    }

    bool SurvivalSystem::SetEnvironmentalEffects(const Vec3& position, const EnvironmentalEffects& effects) {
        std::unique_lock<std::shared_mutex> lock(m_survivalMutex);

        m_environmentalMap[position] = effects;
        return true;
    }

    bool SurvivalSystem::ApplyStatusEffect(Player* player, const std::string& effectName,
                                         float duration, float strength) {
        if (!player || !m_config.enableStatusEffects || duration <= 0.0f) {
            return false;
        }

        std::unique_lock<std::shared_mutex> lock(m_survivalMutex);

        auto it = m_playerStats.find(player);
        if (it == m_playerStats.end()) {
            return false;
        }

        PlayerStats& stats = it->second;
        stats.statusEffects[effectName] = strength;

        uint64_t currentTime = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();

        stats.effectStartTimes[effectName] = currentTime;
        stats.effectDurations[effectName] = duration;

        m_stats.statusEffectsActive++;

        Logger::Debug("Applied status effect '{}' to player (strength: {}, duration: {}s)",
                     effectName, strength, duration);

        return true;
    }

    bool SurvivalSystem::RemoveStatusEffect(Player* player, const std::string& effectName) {
        if (!player) {
            return false;
        }

        std::unique_lock<std::shared_mutex> lock(m_survivalMutex);

        auto it = m_playerStats.find(player);
        if (it == m_playerStats.end()) {
            return false;
        }

        PlayerStats& stats = it->second;

        auto effectIt = stats.statusEffects.find(effectName);
        if (effectIt == stats.statusEffects.end()) {
            return false;
        }

        stats.statusEffects.erase(effectIt);
        stats.effectStartTimes.erase(effectName);
        stats.effectDurations.erase(effectName);

        m_stats.statusEffectsActive--;

        Logger::Debug("Removed status effect '{}' from player", effectName);
        return true;
    }

    bool SurvivalSystem::HasStatusEffect(Player* player, const std::string& effectName) const {
        if (!player) {
            return false;
        }

        std::shared_lock<std::shared_mutex> lock(m_survivalMutex);

        auto it = m_playerStats.find(player);
        if (it == m_playerStats.end()) {
            return false;
        }

        return it->second.statusEffects.find(effectName) != it->second.statusEffects.end();
    }

    float SurvivalSystem::GetStatusEffectStrength(Player* player, const std::string& effectName) const {
        if (!player) {
            return 0.0f;
        }

        std::shared_lock<std::shared_mutex> lock(m_survivalMutex);

        auto it = m_playerStats.find(player);
        if (it == m_playerStats.end()) {
            return 0.0f;
        }

        auto effectIt = it->second.statusEffects.find(effectName);
        if (effectIt == it->second.statusEffects.end()) {
            return 0.0f;
        }

        return effectIt->second;
    }

    PlayerState SurvivalSystem::GetPlayerState(Player* player) const {
        if (!player) {
            return PlayerState::NORMAL;
        }

        std::shared_lock<std::shared_mutex> lock(m_survivalMutex);

        auto it = m_playerStats.find(player);
        if (it == m_playerStats.end()) {
            return PlayerState::NORMAL;
        }

        return it->second.currentState;
    }

    bool SurvivalSystem::SetPlayerState(Player* player, PlayerState state) {
        if (!player) {
            return false;
        }

        std::unique_lock<std::shared_mutex> lock(m_survivalMutex);

        auto it = m_playerStats.find(player);
        if (it == m_playerStats.end()) {
            return false;
        }

        PlayerState oldState = it->second.currentState;
        it->second.currentState = state;

        HandleStateTransitions(player, oldState, state);

        Logger::Debug("Player state changed: {} -> {}",
                     static_cast<int>(oldState), static_cast<int>(state));

        return true;
    }

    bool SurvivalSystem::IsPlayerCritical(Player* player) const {
        if (!player) {
            return false;
        }

        std::shared_lock<std::shared_mutex> lock(m_survivalMutex);

        auto it = m_playerStats.find(player);
        if (it == m_playerStats.end()) {
            return false;
        }

        const PlayerStats& stats = it->second;

        return stats.health <= m_config.criticalHealthThreshold ||
               stats.hunger <= 2.0f ||
               stats.thirst <= 2.0f ||
               stats.oxygen <= 2.0f ||
               stats.currentState == PlayerState::DROWNING ||
               stats.currentState == PlayerState::SUFFOCATING ||
               stats.currentState == PlayerState::DEAD;
    }

    bool SurvivalSystem::IsPlayerDead(Player* player) const {
        if (!player) {
            return false;
        }

        std::shared_lock<std::shared_mutex> lock(m_survivalMutex);

        auto it = m_playerStats.find(player);
        if (it == m_playerStats.end()) {
            return false;
        }

        return it->second.currentState == PlayerState::DEAD || it->second.health <= 0.0f;
    }

    bool SurvivalSystem::KillPlayer(Player* player, const std::string& cause) {
        if (!player) {
            return false;
        }

        std::unique_lock<std::shared_mutex> lock(m_survivalMutex);

        auto it = m_playerStats.find(player);
        if (it == m_playerStats.end()) {
            return false;
        }

        PlayerStats& stats = it->second;

        if (stats.currentState == PlayerState::DEAD) {
            return false;
        }

        ProcessPlayerDeath(player, cause);
        return true;
    }

    bool SurvivalSystem::RespawnPlayer(Player* player, const Vec3& position) {
        if (!player) {
            return false;
        }

        std::unique_lock<std::shared_mutex> lock(m_survivalMutex);

        auto it = m_playerStats.find(player);
        if (it == m_playerStats.end()) {
            return false;
        }

        PlayerStats& stats = it->second;

        // Reset player stats for respawn
        stats.health = m_config.maxHealth * 0.5f; // Start with half health
        stats.hunger = m_config.maxHunger * 0.3f; // Start hungry
        stats.thirst = m_config.maxThirst * 0.3f; // Start thirsty
        stats.stamina = m_config.maxStamina * 0.5f; // Start with half stamina
        stats.oxygen = 20.0f;
        stats.sanity = 15.0f; // Slightly reduced sanity
        stats.temperature = m_config.optimalTemperature;
        stats.sleep = 15.0f; // Slightly tired

        // Clear negative effects
        stats.radiation = 0.0f;
        stats.toxicity = 0.0f;
        stats.bleeding = 0.0f;
        stats.disease = 0.0f;
        stats.wetLevel = 0.0f;
        stats.burnLevel = 0.0f;
        stats.freezeLevel = 0.0f;
        stats.poisonLevel = 0.0f;
        stats.infectionLevel = 0.0f;

        // Clear status effects
        stats.statusEffects.clear();
        stats.effectStartTimes.clear();
        stats.effectDurations.clear();

        // Reset state
        stats.currentState = PlayerState::NORMAL;
        stats.isSprinting = false;
        stats.isSwimming = false;
        stats.isUnderwater = false;
        stats.isInWater = false;
        stats.isOnFire = false;
        stats.isFreezing = false;

        // Move player to respawn position
        player->SetPosition(position);

        stats.deaths++;
        m_stats.deathsProcessed++;

        Logger::Info("Player respawned at position ({}, {}, {})", position.x, position.y, position.z);
        return true;
    }

    bool SurvivalSystem::UseBandage(Player* player, const std::string& bandageType) {
        if (!player) {
            return false;
        }

        std::unique_lock<std::shared_mutex> lock(m_survivalMutex);

        auto it = m_playerStats.find(player);
        if (it == m_playerStats.end()) {
            return false;
        }

        PlayerStats& stats = it->second;

        if (stats.bleeding <= 0.0f) {
            return false; // No bleeding to stop
        }

        float healAmount = 2.0f;
        float bleedReduction = 1.0f;

        if (bandageType == "advanced") {
            healAmount = 4.0f;
            bleedReduction = 2.0f;
        } else if (bandageType == "sterile") {
            healAmount = 3.0f;
            bleedReduction = 1.5f;
        }

        stats.health = std::clamp(stats.health + healAmount, 0.0f, stats.maxHealth);
        stats.bleeding = std::max(stats.bleeding - bleedReduction, 0.0f);

        // Remove bleeding status effect if bleeding is stopped
        if (stats.bleeding <= 0.0f) {
            RemoveStatusEffect(player, "bleeding");
        }

        Logger::Debug("Player used {} bandage: +{} health, -{} bleeding",
                     bandageType, healAmount, bleedReduction);

        return true;
    }

    bool SurvivalSystem::UseMedicine(Player* player, const std::string& medicineType) {
        if (!player) {
            return false;
        }

        std::unique_lock<std::shared_mutex> lock(m_survivalMutex);

        auto it = m_playerStats.find(player);
        if (it == m_playerStats.end()) {
            return false;
        }

        PlayerStats& stats = it->second;

        if (medicineType == "antibiotic") {
            stats.disease = std::max(stats.disease - 3.0f, 0.0f);
            stats.infectionLevel = std::max(stats.infectionLevel - 2.0f, 0.0f);
        } else if (medicineType == "painkiller") {
            stats.health = std::clamp(stats.health + 3.0f, 0.0f, stats.maxHealth);
            // Reduce pain-related status effects
        } else if (medicineType == "stimulant") {
            stats.stamina = std::clamp(stats.stamina + 5.0f, 0.0f, m_config.maxStamina);
            stats.sleep = std::clamp(stats.sleep + 3.0f, 0.0f, 20.0f);
        } else if (medicineType == "radiation_pills") {
            stats.radiation = std::max(stats.radiation - 5.0f, 0.0f);
        }

        Logger::Debug("Player used medicine: {}", medicineType);
        return true;
    }

    bool SurvivalSystem::UseAntidote(Player* player, const std::string& antidoteType) {
        if (!player) {
            return false;
        }

        std::unique_lock<std::shared_mutex> lock(m_survivalMutex);

        auto it = m_playerStats.find(player);
        if (it == m_playerStats.end()) {
            return false;
        }

        PlayerStats& stats = it->second;

        if (antidoteType == "poison") {
            stats.toxicity = std::max(stats.toxicity - 5.0f, 0.0f);
            stats.poisonLevel = 0.0f;
            RemoveStatusEffect(player, "poisoned");
        } else if (antidoteType == "radiation") {
            stats.radiation = std::max(stats.radiation - 8.0f, 0.0f);
        }

        Logger::Debug("Player used antidote: {}", antidoteType);
        return true;
    }

    const PlayerStats& SurvivalSystem::GetSurvivalStats(Player* player) const {
        return GetPlayerStats(player);
    }

    bool SurvivalSystem::ResetSurvivalStats(Player* player) {
        if (!player) {
            return false;
        }

        std::unique_lock<std::shared_mutex> lock(m_survivalMutex);

        auto it = m_playerStats.find(player);
        if (it == m_playerStats.end()) {
            return false;
        }

        PlayerStats& stats = it->second;

        // Reset statistics but keep current survival state
        stats.playTime = 0;
        stats.survivalTime = 0;
        stats.deaths = 0;
        stats.monstersKilled = 0;
        stats.distanceTraveled = 0;
        stats.itemsCrafted = 0;
        stats.blocksMined = 0;
        stats.structuresBuilt = 0;

        return true;
    }

    void SurvivalSystem::ResetStats() {
        m_stats = SurvivalStats();
        m_stats.playersTracked = static_cast<int>(m_playerStats.size());
    }

    void SurvivalSystem::RenderDebugInfo() {
        if (!m_debugMode) {
            return;
        }

        // This would integrate with the rendering system
        // For now, just log debug info
        Logger::Debug("SurvivalSystem Debug: {} players, {} effects active",
                     m_stats.playersTracked, m_stats.statusEffectsActive);
    }

    std::string SurvivalSystem::GetDebugInfo() const {
        std::stringstream ss;
        ss << std::fixed << std::setprecision(2);

        ss << "SurvivalSystem Debug Info:\n";
        ss << "Players tracked: " << m_stats.playersTracked << "\n";
        ss << "Status effects active: " << m_stats.statusEffectsActive << "\n";
        ss << "Deaths processed: " << m_stats.deathsProcessed << "\n";
        ss << "Healings processed: " << m_stats.healingsProcessed << "\n";
        ss << "Damages processed: " << m_stats.damagesProcessed << "\n";
        ss << "Average health: " << m_stats.averageHealth << "\n";
        ss << "Average hunger: " << m_stats.averageHunger << "\n";
        ss << "Average thirst: " << m_stats.averageThirst << "\n";
        ss << "Critical states: " << m_stats.criticalStates << "\n";

        return ss.str();
    }

    std::string SurvivalSystem::GetPlayerDebugInfo(Player* player) const {
        if (!player) {
            return "Invalid player";
        }

        std::shared_lock<std::shared_mutex> lock(m_survivalMutex);

        auto it = m_playerStats.find(player);
        if (it == m_playerStats.end()) {
            return "Player not found in survival system";
        }

        const PlayerStats& stats = it->second;

        std::stringstream ss;
        ss << std::fixed << std::setprecision(2);

        ss << "Player Survival Debug Info:\n";
        ss << "Health: " << stats.health << "/" << stats.maxHealth << "\n";
        ss << "Hunger: " << stats.hunger << "/" << m_config.maxHunger << "\n";
        ss << "Thirst: " << stats.thirst << "/" << m_config.maxThirst << "\n";
        ss << "Stamina: " << stats.stamina << "/" << m_config.maxStamina << "\n";
        ss << "Oxygen: " << stats.oxygen << "\n";
        ss << "Sanity: " << stats.sanity << "\n";
        ss << "Temperature: " << stats.temperature << "°C\n";
        ss << "Sleep: " << stats.sleep << "\n";
        ss << "Radiation: " << stats.radiation << "\n";
        ss << "Toxicity: " << stats.toxicity << "\n";
        ss << "Bleeding: " << stats.bleeding << "\n";
        ss << "Disease: " << stats.disease << "\n";
        ss << "State: " << static_cast<int>(stats.currentState) << "\n";
        ss << "Status effects: " << stats.statusEffects.size() << "\n";

        return ss.str();
    }

    // Private methods implementation

    void SurvivalSystem::UpdatePlayerNeeds(float deltaTime) {
        std::unique_lock<std::shared_mutex> lock(m_survivalMutex);

        for (auto& pair : m_playerStats) {
            Player* player = pair.first;
            PlayerStats& stats = pair.second;

            // Update play time
            stats.playTime += static_cast<uint64_t>(deltaTime);

            if (stats.currentState != PlayerState::DEAD) {
                stats.survivalTime += static_cast<uint64_t>(deltaTime);
            }

            // Update hunger
            if (m_config.enableHunger) {
                CalculateHungerEffects(player, deltaTime);
            }

            // Update thirst
            if (m_config.enableThirst) {
                CalculateThirstEffects(player, deltaTime);
            }

            // Update temperature
            if (m_config.enableTemperature) {
                CalculateTemperatureEffects(player, deltaTime);
            }

            // Update oxygen
            if (m_config.enableOxygen && stats.isUnderwater) {
                stats.oxygen = std::max(stats.oxygen - deltaTime * 2.0f, 0.0f);

                if (stats.oxygen <= 0.0f) {
                    stats.health = std::max(stats.health - m_config.drowningDamageRate * deltaTime, 0.0f);
                }
            } else {
                stats.oxygen = std::min(stats.oxygen + deltaTime * 5.0f, 20.0f);
            }

            // Update stamina
            if (m_config.enableStamina) {
                if (stats.isSprinting) {
                    stats.stamina = std::max(stats.stamina - m_config.sprintStaminaCost * deltaTime, 0.0f);
                } else {
                    stats.stamina = std::min(stats.stamina + m_config.staminaRegenRate * deltaTime,
                                           m_config.maxStamina);
                }
            }

            // Update sleep
            if (stats.currentState != PlayerState::DEAD) {
                stats.sleep = std::max(stats.sleep - deltaTime * 0.1f, 0.0f);
            }
        }
    }

    void SurvivalSystem::UpdateEnvironmentalEffects(float deltaTime) {
        std::unique_lock<std::shared_mutex> lock(m_survivalMutex);

        for (auto& pair : m_playerStats) {
            Player* player = pair.first;
            PlayerStats& stats = pair.second;

            if (stats.currentState == PlayerState::DEAD) {
                continue;
            }

            // Get environmental effects at player position
            EnvironmentalEffects envEffects = GetEnvironmentalEffects(player->GetPosition());

            // Apply environmental damage
            CalculateEnvironmentalDamage(player, deltaTime);

            // Update wet level based on weather
            if (envEffects.weather == WeatherCondition::RAINING ||
                envEffects.weather == WeatherCondition::STORMING) {
                stats.wetLevel = std::min(stats.wetLevel + deltaTime * 0.1f, 1.0f);
            } else if (!stats.isUnderwater) {
                stats.wetLevel = std::max(stats.wetLevel - deltaTime * 0.05f, 0.0f);
            }

            // Apply environmental temperature effects
            if (m_config.enableTemperature) {
                float envTemp = envEffects.temperature;
                float playerHeat = CalculatePlayerHeatGeneration(player);
                float heatLoss = CalculatePlayerHeatLoss(player);

                float tempChange = (envTemp - stats.temperature) * 0.01f * deltaTime;
                tempChange += playerHeat * deltaTime;
                tempChange -= heatLoss * deltaTime;

                stats.temperature += tempChange;

                // Temperature damage
                if (stats.temperature < m_config.hypothermiaThreshold) {
                    stats.freezeLevel += deltaTime * 0.1f;
                    stats.health = std::max(stats.health - m_config.temperatureDamageRate * deltaTime, 0.0f);
                } else if (stats.temperature > m_config.hyperthermiaThreshold) {
                    stats.burnLevel += deltaTime * 0.1f;
                    stats.health = std::max(stats.health - m_config.temperatureDamageRate * deltaTime, 0.0f);
                } else {
                    stats.freezeLevel = std::max(stats.freezeLevel - deltaTime * 0.05f, 0.0f);
                    stats.burnLevel = std::max(stats.burnLevel - deltaTime * 0.05f, 0.0f);
                }
            }
        }
    }

    void SurvivalSystem::UpdateStatusEffects(float deltaTime) {
        std::unique_lock<std::shared_mutex> lock(m_survivalMutex);

        for (auto& pair : m_playerStats) {
            Player* player = pair.first;
            PlayerStats& stats = pair.second;

            // Process active status effects
            for (auto& effect : stats.statusEffects) {
                ProcessStatusEffect(player, effect.first, deltaTime);
            }

            // Remove expired effects
            RemoveExpiredStatusEffects(player);
        }
    }

    void SurvivalSystem::UpdatePlayerStates(float deltaTime) {
        std::unique_lock<std::shared_mutex> lock(m_survivalMutex);

        for (auto& pair : m_playerStats) {
            Player* player = pair.first;
            PlayerStats& stats = pair.second;

            PlayerState oldState = stats.currentState;
            UpdatePlayerState(player);

            if (oldState != stats.currentState) {
                HandleStateTransitions(player, oldState, stats.currentState);
            }
        }

        // Update statistics
        UpdateSurvivalStats();
    }

    void SurvivalSystem::ProcessPlayerDeath(Player* player, const std::string& cause) {
        if (!player) {
            return;
        }

        std::unique_lock<std::shared_mutex> lock(m_survivalMutex);

        auto it = m_playerStats.find(player);
        if (it == m_playerStats.end()) {
            return;
        }

        PlayerStats& stats = it->second;

        stats.currentState = PlayerState::DEAD;
        stats.health = 0.0f;

        m_stats.deathsProcessed++;

        Logger::Info("Player died: {}", cause);

        // TODO: Trigger death event, drop inventory, etc.
        // This would integrate with other systems
    }

    void SurvivalSystem::CalculateHungerEffects(Player* player, float deltaTime) {
        PlayerStats& stats = m_playerStats[player];

        // Hunger decreases over time
        stats.hunger = std::max(stats.hunger - m_config.hungerDecayRate * deltaTime, 0.0f);

        // Health damage when starving
        if (stats.hunger <= 0.0f) {
            stats.health = std::max(stats.health - m_config.hungerDamageRate * deltaTime, 0.0f);
        } else if (stats.hunger > m_config.maxHunger * 0.5f) {
            // Health regeneration when well fed
            stats.health = std::min(stats.health + m_config.healthRegenRate * deltaTime, stats.maxHealth);
        }
    }

    void SurvivalSystem::CalculateThirstEffects(Player* player, float deltaTime) {
        PlayerStats& stats = m_playerStats[player];

        // Thirst decreases over time
        stats.thirst = std::max(stats.thirst - m_config.thirstDecayRate * deltaTime, 0.0f);

        // Health damage when dehydrated
        if (stats.thirst <= 0.0f) {
            stats.health = std::max(stats.health - m_config.thirstDamageRate * deltaTime, 0.0f);
        }
    }

    void SurvivalSystem::CalculateTemperatureEffects(Player* player, float deltaTime) {
        PlayerStats& stats = m_playerStats[player];

        // Get environmental effects
        EnvironmentalEffects envEffects = GetEnvironmentalEffects(player->GetPosition());

        // Calculate temperature change based on environment
        float targetTemp = envEffects.temperature;

        // Modify based on player conditions
        if (stats.isSprinting) {
            targetTemp += 2.0f; // Sprinting generates heat
        }

        if (stats.wetLevel > 0.5f) {
            targetTemp -= stats.wetLevel * 5.0f; // Wet players cool faster
        }

        if (IsPlayerInShelter(player)) {
            targetTemp = MathUtils::Lerp(targetTemp, 20.0f, 0.5f); // Shelter provides insulation
        }

        // Temperature approaches target
        float tempDiff = targetTemp - stats.temperature;
        stats.temperature += tempDiff * m_config.temperatureChangeRate * deltaTime;
    }

    void SurvivalSystem::CalculateEnvironmentalDamage(Player* player, float deltaTime) {
        PlayerStats& stats = m_playerStats[player];

        // Radiation damage
        if (stats.radiation > 0.0f) {
            float radiationDamage = stats.radiation * m_config.radiationDamageRate * deltaTime;
            stats.health = std::max(stats.health - radiationDamage, 0.0f);
        }

        // Toxicity damage
        if (stats.toxicity > 0.0f) {
            float toxicityDamage = stats.toxicity * m_config.toxicityDamageRate * deltaTime;
            stats.health = std::max(stats.health - toxicityDamage, 0.0f);
        }

        // Fire damage
        if (stats.isOnFire) {
            stats.health = std::max(stats.health - m_config.fireDamageRate * deltaTime, 0.0f);
            stats.burnLevel += deltaTime * 0.2f;
        }

        // Bleeding damage
        if (stats.bleeding > 0.0f) {
            float bleedDamage = stats.bleeding * 0.5f * deltaTime;
            stats.health = std::max(stats.health - bleedDamage, 0.0f);
        }

        // Disease damage
        if (stats.disease > 0.0f) {
            float diseaseDamage = stats.disease * 0.1f * deltaTime;
            stats.health = std::max(stats.health - diseaseDamage, 0.0f);
        }
    }

    void SurvivalSystem::UpdatePlayerState(Player* player) {
        PlayerStats& stats = m_playerStats[player];

        if (stats.health <= 0.0f) {
            stats.currentState = PlayerState::DEAD;
            return;
        }

        if (stats.oxygen <= 2.0f && stats.isUnderwater) {
            stats.currentState = PlayerState::DROWNING;
            return;
        }

        if (stats.hunger <= 2.0f) {
            stats.currentState = PlayerState::STARVING;
            return;
        }

        if (stats.thirst <= 2.0f) {
            stats.currentState = PlayerState::DEHYDRATED;
            return;
        }

        if (stats.temperature < m_config.hypothermiaThreshold) {
            stats.currentState = PlayerState::HYPOTHERMIA;
            return;
        }

        if (stats.temperature > m_config.hyperthermiaThreshold) {
            stats.currentState = PlayerState::HYPERTHERMIA;
            return;
        }

        if (stats.toxicity > 5.0f) {
            stats.currentState = PlayerState::POISONED;
            return;
        }

        if (stats.radiation > 5.0f) {
            stats.currentState = PlayerState::RADIATED;
            return;
        }

        if (stats.disease > 5.0f) {
            stats.currentState = PlayerState::INFECTED;
            return;
        }

        if (stats.bleeding > 0.0f) {
            stats.currentState = PlayerState::BLEEDING;
            return;
        }

        if (stats.sanity < 5.0f) {
            stats.currentState = PlayerState::INSANE;
            return;
        }

        if (stats.sleep < 5.0f) {
            stats.currentState = PlayerState::TIRED;
            return;
        }

        // Default to normal state
        stats.currentState = PlayerState::NORMAL;
    }

    void SurvivalSystem::HandleStateTransitions(Player* player, PlayerState oldState, PlayerState newState) {
        if (oldState == newState) {
            return;
        }

        PlayerStats& stats = m_playerStats[player];

        // Handle state exit effects
        switch (oldState) {
            case PlayerState::DROWNING:
                if (newState != PlayerState::DEAD) {
                    Logger::Info("Player stopped drowning");
                }
                break;

            case PlayerState::STARVING:
                Logger::Info("Player no longer starving");
                break;

            case PlayerState::DEHYDRATED:
                Logger::Info("Player no longer dehydrated");
                break;

            default:
                break;
        }

        // Handle state entry effects
        switch (newState) {
            case PlayerState::DROWNING:
                Logger::Warning("Player is drowning!");
                ApplyStatusEffect(player, "drowning", 5.0f, 1.0f);
                break;

            case PlayerState::STARVING:
                Logger::Warning("Player is starving!");
                break;

            case PlayerState::DEHYDRATED:
                Logger::Warning("Player is dehydrated!");
                break;

            case PlayerState::HYPOTHERMIA:
                Logger::Warning("Player has hypothermia!");
                ApplyStatusEffect(player, "freezing", 10.0f, 1.0f);
                break;

            case PlayerState::HYPERTHERMIA:
                Logger::Warning("Player has hyperthermia!");
                ApplyStatusEffect(player, "burning", 10.0f, 1.0f);
                break;

            case PlayerState::POISONED:
                Logger::Warning("Player is poisoned!");
                break;

            case PlayerState::RADIATED:
                Logger::Warning("Player is radiated!");
                break;

            case PlayerState::DEAD:
                Logger::Info("Player died");
                break;

            default:
                break;
        }
    }

    void SurvivalSystem::ProcessStatusEffect(Player* player, const std::string& effectName, float deltaTime) {
        PlayerStats& stats = m_playerStats[player];
        float strength = stats.statusEffects[effectName];

        if (effectName == "poisoned") {
            stats.health = std::max(stats.health - strength * 0.5f * deltaTime, 0.0f);
            stats.toxicity += strength * 0.1f * deltaTime;
        } else if (effectName == "bleeding") {
            stats.bleeding = std::min(stats.bleeding + strength * 0.2f * deltaTime, 5.0f);
        } else if (effectName == "burning") {
            stats.health = std::max(stats.health - strength * 1.0f * deltaTime, 0.0f);
            stats.burnLevel += strength * 0.1f * deltaTime;
        } else if (effectName == "freezing") {
            stats.temperature -= strength * 2.0f * deltaTime;
            stats.freezeLevel += strength * 0.1f * deltaTime;
        } else if (effectName == "regeneration") {
            stats.health = std::min(stats.health + strength * 0.5f * deltaTime, stats.maxHealth);
        }
    }

    void SurvivalSystem::RemoveExpiredStatusEffects(Player* player) {
        PlayerStats& stats = m_playerStats[player];
        uint64_t currentTime = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();

        std::vector<std::string> expiredEffects;

        for (const auto& pair : stats.effectStartTimes) {
            const std::string& effectName = pair.first;
            uint64_t startTime = pair.second;
            float duration = stats.effectDurations[effectName];

            if (currentTime - startTime >= static_cast<uint64_t>(duration)) {
                expiredEffects.push_back(effectName);
            }
        }

        for (const std::string& effectName : expiredEffects) {
            RemoveStatusEffect(player, effectName);
        }
    }

    void SurvivalSystem::UpdateSurvivalStats() {
        m_stats.averageHealth = 0.0f;
        m_stats.averageHunger = 0.0f;
        m_stats.averageThirst = 0.0f;
        m_stats.criticalStates = 0;

        if (m_playerStats.empty()) {
            return;
        }

        for (const auto& pair : m_playerStats) {
            const PlayerStats& stats = pair.second;
            m_stats.averageHealth += stats.health;
            m_stats.averageHunger += stats.hunger;
            m_stats.averageThirst += stats.thirst;

            if (IsPlayerCritical(pair.first)) {
                m_stats.criticalStates++;
            }
        }

        size_t count = m_playerStats.size();
        m_stats.averageHealth /= count;
        m_stats.averageHunger /= count;
        m_stats.averageThirst /= count;
    }

    float SurvivalSystem::CalculateEnvironmentalTemperature(const Vec3& position) const {
        // Simple temperature calculation based on height
        // Higher altitudes are colder
        float baseTemp = 20.0f;
        float heightFactor = position.y / 100.0f; // Temperature decreases by 1°C per 100 blocks
        return baseTemp - heightFactor;
    }

    float SurvivalSystem::CalculatePlayerHeatGeneration(Player* player) const {
        const PlayerStats& stats = m_playerStats.at(player);

        float heatGeneration = 0.0f;

        if (stats.isSprinting) {
            heatGeneration += 2.0f;
        }

        // More heat generation when wet (evaporation)
        if (stats.wetLevel > 0.0f) {
            heatGeneration += stats.wetLevel * 1.0f;
        }

        return heatGeneration;
    }

    float SurvivalSystem::CalculatePlayerHeatLoss(Player* player) const {
        const PlayerStats& stats = m_playerStats.at(player);

        float heatLoss = 1.0f; // Base heat loss

        // More heat loss when wet
        if (stats.wetLevel > 0.0f) {
            heatLoss += stats.wetLevel * 2.0f;
        }

        // Wind increases heat loss
        EnvironmentalEffects envEffects = GetEnvironmentalEffects(player->GetPosition());
        if (envEffects.windSpeed > 5.0f) {
            heatLoss += envEffects.windSpeed * 0.1f;
        }

        return heatLoss;
    }

    bool SurvivalSystem::IsPlayerInShelter(Player* player) const {
        // This would check if the player is in a structure that provides shelter
        // For now, return false
        return false;
    }

    bool SurvivalSystem::IsPositionInWater(const Vec3& position) const {
        // This would check if the position is underwater
        // For now, return false
        return false;
    }

} // namespace VoxelCraft
