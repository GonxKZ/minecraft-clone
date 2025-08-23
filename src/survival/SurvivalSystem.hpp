/**
 * @file SurvivalSystem.hpp
 * @brief VoxelCraft Survival System
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_SURVIVAL_SURVIVAL_SYSTEM_HPP
#define VOXELCRAFT_SURVIVAL_SURVIVAL_SYSTEM_HPP

#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
#include <atomic>
#include <mutex>
#include <functional>

#include "../math/Vec3.hpp"
#include "../entities/Entity.hpp"

namespace VoxelCraft {

    // Forward declarations
    class Player;
    class World;
    struct SurvivalConfig;
    struct SurvivalStats;
    struct PlayerStats;
    struct EnvironmentalEffects;

    /**
     * @enum SurvivalNeed
     * @brief Basic survival needs
     */
    enum class SurvivalNeed {
        HEALTH,            ///< Health points
        HUNGER,            ///< Hunger level
        THIRST,            ///< Thirst level
        STAMINA,           ///< Stamina for sprinting/jumping
        OXYGEN,            ///< Oxygen for underwater/swimming
        SANITY,            ///< Mental health/sanity
        TEMPERATURE,       ///< Body temperature
        RADIATION,         ///< Radiation exposure
        TOXICITY,          ///< Toxicity level
        BLEEDING,          ///< Bleeding status
        DISEASE,           ///< Disease/infection status
        SLEEP,             ///< Sleep deprivation
        CUSTOM             ///< Custom survival need
    };

    /**
     * @enum PlayerState
     * @brief Player physical/mental states
     */
    enum class PlayerState {
        NORMAL,            ///< Normal state
        HUNGRY,            ///< Hungry state
        STARVING,          ///< Starving state
        THIRSTY,           ///< Thirsty state
        DEHYDRATED,        ///< Dehydrated state
        TIRED,             ///< Tired/sleepy
        EXHAUSTED,         ///< Exhausted
        HYPOTHERMIA,       ///< Too cold
        HYPERTHERMIA,      ///< Too hot
        DROWNING,          ///< Drowning
        SUFFOCATING,       ///< Suffocating
        POISONED,          ///< Poisoned
        BLEEDING,          ///< Bleeding
        INFECTED,          ///< Infected/diseased
        RADIATED,          ///< Radiation sickness
        INSANE,            ///< Mental breakdown
        DEAD               ///< Dead
    };

    /**
     * @enum WeatherCondition
     * @brief Environmental weather conditions
     */
    enum class WeatherCondition {
        CLEAR,             ///< Clear weather
        CLOUDY,            ///< Cloudy
        RAINING,           ///< Rain
        STORMING,          ///< Storm/thunderstorm
        SNOWING,           ///< Snow
        BLIZZARD,          ///< Blizzard
        FOGGY,             ///< Fog
        SANDSTORM,         ///< Sandstorm
        ACID_RAIN,         ///< Acid rain
        CUSTOM             ///< Custom weather
    };

    /**
     * @struct PlayerStats
     * @brief Player survival statistics
     */
    struct PlayerStats {
        // Basic needs
        float health = 20.0f;          ///< Current health (0-20)
        float maxHealth = 20.0f;       ///< Maximum health
        float hunger = 20.0f;          ///< Hunger level (0-20)
        float thirst = 20.0f;          ///< Thirst level (0-20)
        float stamina = 20.0f;         ///< Stamina level (0-20)
        float oxygen = 20.0f;          ///< Oxygen level (0-20)
        float sanity = 20.0f;          ///< Sanity level (0-20)
        float temperature = 37.0f;     ///< Body temperature (Celsius)
        float radiation = 0.0f;        ///< Radiation level
        float toxicity = 0.0f;         ///< Toxicity level
        float bleeding = 0.0f;         ///< Bleeding rate
        float disease = 0.0f;          ///< Disease level
        float sleep = 20.0f;           ///< Sleep level (0-20)

        // Environmental effects
        float wetLevel = 0.0f;         ///< How wet the player is (0-1)
        float burnLevel = 0.0f;        ///< Burn/fire damage level
        float freezeLevel = 0.0f;      ///< Freeze damage level
        float poisonLevel = 0.0f;      ///< Poison level
        float infectionLevel = 0.0f;   ///< Infection level

        // Status effects
        std::unordered_map<std::string, float> statusEffects;
        std::unordered_map<std::string, uint64_t> effectStartTimes;
        std::unordered_map<std::string, float> effectDurations;

        // Player state
        PlayerState currentState = PlayerState::NORMAL;
        bool isSprinting = false;
        bool isSwimming = false;
        bool isUnderwater = false;
        bool isInWater = false;
        bool isOnFire = false;
        bool isFreezing = false;

        // Survival metrics
        uint64_t playTime = 0;         ///< Total play time in seconds
        uint64_t survivalTime = 0;     ///< Time survived in current session
        int deaths = 0;                ///< Total deaths
        int monstersKilled = 0;        ///< Monsters defeated
        int distanceTraveled = 0;      ///< Distance traveled
        int itemsCrafted = 0;          ///< Items crafted
        int blocksMined = 0;           ///< Blocks mined
        int structuresBuilt = 0;       ///< Structures built
    };

    /**
     * @struct EnvironmentalEffects
     * @brief Environmental effects on player
     */
    struct EnvironmentalEffects {
        float temperature = 20.0f;     ///< Ambient temperature
        float humidity = 50.0f;        ///< Ambient humidity
        float radiation = 0.0f;        ///< Ambient radiation
        float toxicity = 0.0f;         ///< Ambient toxicity
        float oxygenLevel = 21.0f;     ///< Ambient oxygen percentage
        float pressure = 1.0f;         ///< Atmospheric pressure
        WeatherCondition weather = WeatherCondition::CLEAR;
        float weatherIntensity = 0.0f;
        Vec3 windDirection = Vec3(0.0f, 0.0f, 0.0f);
        float windSpeed = 0.0f;
    };

    /**
     * @struct SurvivalConfig
     * @brief Configuration for survival system
     */
    struct SurvivalConfig {
        bool enableSurvival = true;
        bool enableHunger = true;
        bool enableThirst = true;
        bool enableStamina = true;
        bool enableOxygen = true;
        bool enableTemperature = true;
        bool enableSanity = true;
        bool enableRadiation = true;
        bool enableDisease = true;
        bool enableEnvironmentalEffects = true;
        bool enableStatusEffects = true;

        // Health settings
        float healthRegenRate = 1.0f;          // Health per second when well fed
        float healthDecayRate = 0.5f;          // Health loss per second when starving
        float maxHealth = 20.0f;               // Maximum health
        float criticalHealthThreshold = 3.0f;  // Critical health level

        // Hunger settings
        float hungerDecayRate = 0.1f;          // Hunger loss per second
        float hungerDamageRate = 0.5f;         // Health loss per second when starving
        float foodHealingRate = 2.0f;          // Health gain per food point
        float maxHunger = 20.0f;               // Maximum hunger

        // Thirst settings
        float thirstDecayRate = 0.15f;         // Thirst loss per second
        float thirstDamageRate = 0.3f;         // Health loss per second when dehydrated
        float waterHealingRate = 1.5f;         // Health gain per water point
        float maxThirst = 20.0f;               // Maximum thirst

        // Stamina settings
        float staminaRegenRate = 2.0f;         // Stamina regen per second when resting
        float staminaDecayRate = 1.0f;         // Stamina loss per second when sprinting
        float maxStamina = 20.0f;              // Maximum stamina
        float sprintStaminaCost = 1.0f;        // Stamina cost per second sprinting

        // Temperature settings
        float optimalTemperature = 37.0f;      // Optimal body temperature
        float hypothermiaThreshold = 35.0f;    // Hypothermia threshold
        float hyperthermiaThreshold = 40.0f;   // Hyperthermia threshold
        float temperatureChangeRate = 0.1f;    // Temperature change per second
        float temperatureDamageRate = 0.2f;    // Health loss per second when extreme

        // Environmental settings
        float radiationDamageRate = 0.1f;      // Health loss per second per radiation unit
        float toxicityDamageRate = 0.2f;       // Health loss per second per toxicity unit
        float drowningDamageRate = 2.0f;       // Health loss per second when drowning
        float fireDamageRate = 1.0f;           // Health loss per second when on fire
        float freezeDamageRate = 0.5f;         // Health loss per second when freezing
    };

    /**
     * @struct SurvivalStats
     * @brief Performance statistics for survival system
     */
    struct SurvivalStats {
        int playersTracked = 0;
        int statusEffectsActive = 0;
        int environmentalEffectsActive = 0;
        int deathsProcessed = 0;
        int healingsProcessed = 0;
        int damagesProcessed = 0;
        float averageHealth = 0.0f;
        float averageHunger = 0.0f;
        float averageThirst = 0.0f;
        int criticalStates = 0;
    };

    /**
     * @class SurvivalSystem
     * @brief Advanced survival system for VoxelCraft
     *
     * Features:
     * - Health, hunger, thirst, and stamina management
     * - Temperature regulation and environmental effects
     * - Oxygen system for underwater/swimming
     * - Radiation and toxicity systems
     * - Disease and infection mechanics
     * - Sanity/mental health system
     * - Status effects and buffs/debuffs
     * - Weather-based survival mechanics
     * - Player state management
     * - Survival statistics and achievements
     * - Multiplayer survival synchronization
     * - Survival difficulty scaling
     * - Emergency systems (bandages, medicines)
     * - Survival crafting and resource management
     */
    class SurvivalSystem {
    public:
        static SurvivalSystem& GetInstance();

        /**
         * @brief Initialize the survival system
         * @param config Survival configuration
         * @return true if successful
         */
        bool Initialize(const SurvivalConfig& config);

        /**
         * @brief Shutdown the survival system
         */
        void Shutdown();

        /**
         * @brief Update survival system (call every frame)
         * @param deltaTime Time since last update
         */
        void Update(float deltaTime);

        // Player Management
        /**
         * @brief Register player with survival system
         * @param player Player to register
         * @return true if successful
         */
        bool RegisterPlayer(Player* player);

        /**
         * @brief Unregister player from survival system
         * @param player Player to unregister
         * @return true if successful
         */
        bool UnregisterPlayer(Player* player);

        /**
         * @brief Get player survival stats
         * @param player Player
         * @return Player survival statistics
         */
        const PlayerStats& GetPlayerStats(Player* player) const;

        /**
         * @brief Update player survival stats
         * @param player Player
         * @param stats New statistics
         * @return true if successful
         */
        bool UpdatePlayerStats(Player* player, const PlayerStats& stats);

        // Survival Needs Management
        /**
         * @brief Modify player health
         * @param player Player
         * @param amount Health change (positive = heal, negative = damage)
         * @param cause Damage cause
         * @return true if successful
         */
        bool ModifyHealth(Player* player, float amount, const std::string& cause = "");

        /**
         * @brief Modify player hunger
         * @param player Player
         * @param amount Hunger change
         * @return true if successful
         */
        bool ModifyHunger(Player* player, float amount);

        /**
         * @brief Modify player thirst
         * @param player Player
         * @param amount Thirst change
         * @return true if successful
         */
        bool ModifyThirst(Player* player, float amount);

        /**
         * @brief Modify player stamina
         * @param player Player
         * @param amount Stamina change
         * @return true if successful
         */
        bool ModifyStamina(Player* player, float amount);

        /**
         * @brief Feed player (restore hunger)
         * @param player Player
         * @param foodValue Food nutritional value
         * @param foodType Type of food
         * @return true if successful
         */
        bool FeedPlayer(Player* player, float foodValue, const std::string& foodType = "");

        /**
         * @brief Hydrate player (restore thirst)
         * @param player Player
         * @param waterValue Water value
         * @param waterType Type of water
         * @return true if successful
         */
        bool HydratePlayer(Player* player, float waterValue, const std::string& waterType = "");

        // Environmental Effects
        /**
         * @brief Apply environmental effects to player
         * @param player Player
         * @param effects Environmental effects
         * @return true if successful
         */
        bool ApplyEnvironmentalEffects(Player* player, const EnvironmentalEffects& effects);

        /**
         * @brief Get environmental effects at position
         * @param position World position
         * @return Environmental effects
         */
        EnvironmentalEffects GetEnvironmentalEffects(const Vec3& position) const;

        /**
         * @brief Set environmental effects at position
         * @param position World position
         * @param effects Environmental effects
         * @return true if successful
         */
        bool SetEnvironmentalEffects(const Vec3& position, const EnvironmentalEffects& effects);

        // Status Effects
        /**
         * @brief Apply status effect to player
         * @param player Player
         * @param effectName Effect name
         * @param duration Duration in seconds
         * @param strength Effect strength
         * @return true if successful
         */
        bool ApplyStatusEffect(Player* player, const std::string& effectName,
                             float duration, float strength = 1.0f);

        /**
         * @brief Remove status effect from player
         * @param player Player
         * @param effectName Effect name
         * @return true if successful
         */
        bool RemoveStatusEffect(Player* player, const std::string& effectName);

        /**
         * @brief Check if player has status effect
         * @param player Player
         * @param effectName Effect name
         * @return true if has effect
         */
        bool HasStatusEffect(Player* player, const std::string& effectName) const;

        /**
         * @brief Get status effect strength
         * @param player Player
         * @param effectName Effect name
         * @return Effect strength
         */
        float GetStatusEffectStrength(Player* player, const std::string& effectName) const;

        // Player States
        /**
         * @brief Get player current state
         * @param player Player
         * @return Player state
         */
        PlayerState GetPlayerState(Player* player) const;

        /**
         * @brief Set player state
         * @param player Player
         * @param state New state
         * @return true if successful
         */
        bool SetPlayerState(Player* player, PlayerState state);

        /**
         * @brief Check if player is in critical condition
         * @param player Player
         * @return true if critical
         */
        bool IsPlayerCritical(Player* player) const;

        /**
         * @brief Check if player is dead
         * @param player Player
         * @return true if dead
         */
        bool IsPlayerDead(Player* player) const;

        /**
         * @brief Kill player
         * @param player Player
         * @param cause Death cause
         * @return true if successful
         */
        bool KillPlayer(Player* player, const std::string& cause = "");

        /**
         * @brief Respawn player
         * @param player Player
         * @param position Respawn position
         * @return true if successful
         */
        bool RespawnPlayer(Player* player, const Vec3& position);

        // Emergency Systems
        /**
         * @brief Use bandage on player
         * @param player Player
         * @param bandageType Type of bandage
         * @return true if successful
         */
        bool UseBandage(Player* player, const std::string& bandageType = "basic");

        /**
         * @brief Use medicine on player
         * @param player Player
         * @param medicineType Type of medicine
         * @return true if successful
         */
        bool UseMedicine(Player* player, const std::string& medicineType);

        /**
         * @brief Use antidote on player
         * @param player Player
         * @param antidoteType Type of antidote
         * @return true if successful
         */
        bool UseAntidote(Player* player, const std::string& antidoteType);

        // Survival Statistics
        /**
         * @brief Get player survival statistics
         * @param player Player
         * @return Survival statistics
         */
        const PlayerStats& GetSurvivalStats(Player* player) const;

        /**
         * @brief Reset player survival statistics
         * @param player Player
         * @return true if successful
         */
        bool ResetSurvivalStats(Player* player);

        // Configuration
        /**
         * @brief Set survival configuration
         * @param config New configuration
         */
        void SetConfig(const SurvivalConfig& config) { m_config = config; }

        /**
         * @brief Get current configuration
         * @return Current configuration
         */
        const SurvivalConfig& GetConfig() const { return m_config; }

        // Statistics
        /**
         * @brief Get survival system statistics
         * @return Current statistics
         */
        const SurvivalStats& GetStats() const { return m_stats; }

        /**
         * @brief Reset statistics
         */
        void ResetStats();

        // Debug
        /**
         * @brief Enable debug mode
         * @param enable Enable state
         */
        void EnableDebugMode(bool enable) { m_debugMode = enable; }

        /**
         * @brief Render debug information
         */
        void RenderDebugInfo();

        /**
         * @brief Get debug information string
         * @return Debug info
         */
        std::string GetDebugInfo() const;

        /**
         * @brief Get player debug info
         * @param player Player
         * @return Debug info for player
         */
        std::string GetPlayerDebugInfo(Player* player) const;

    private:
        SurvivalSystem() = default;
        ~SurvivalSystem();

        // Prevent copying
        SurvivalSystem(const SurvivalSystem&) = delete;
        SurvivalSystem& operator=(const SurvivalSystem&) = delete;

        // Core survival processing
        void UpdatePlayerNeeds(float deltaTime);
        void UpdateEnvironmentalEffects(float deltaTime);
        void UpdateStatusEffects(float deltaTime);
        void UpdatePlayerStates(float deltaTime);
        void ProcessPlayerDeath(Player* player, const std::string& cause);

        // Survival calculations
        void CalculateHungerEffects(Player* player, float deltaTime);
        void CalculateThirstEffects(Player* player, float deltaTime);
        void CalculateTemperatureEffects(Player* player, float deltaTime);
        void CalculateEnvironmentalDamage(Player* player, float deltaTime);
        void CalculateStatusEffectDamage(Player* player, float deltaTime);

        // State management
        void UpdatePlayerState(Player* player);
        void HandleStateTransitions(Player* player, PlayerState oldState, PlayerState newState);
        bool IsStateTransitionValid(PlayerState from, PlayerState to) const;

        // Effect processing
        void ProcessStatusEffect(Player* player, const std::string& effectName, float deltaTime);
        void RemoveExpiredStatusEffects(Player* player);
        void ApplyStatusEffectDamage(Player* player, const std::string& effectName, float deltaTime);

        // Utility functions
        float CalculateEnvironmentalTemperature(const Vec3& position) const;
        float CalculatePlayerHeatGeneration(Player* player) const;
        float CalculatePlayerHeatLoss(Player* player) const;
        bool IsPlayerInShelter(Player* player) const;
        bool IsPositionInWater(const Vec3& position) const;

        // Member variables
        SurvivalConfig m_config;
        bool m_initialized = false;
        bool m_debugMode = false;

        // Player data
        std::unordered_map<Player*, PlayerStats> m_playerStats;
        std::unordered_map<Player*, EnvironmentalEffects> m_environmentalEffects;

        // Environmental data
        std::unordered_map<Vec3, EnvironmentalEffects> m_environmentalMap;

        // Statistics and performance
        SurvivalStats m_stats;

        // Thread safety
        mutable std::shared_mutex m_survivalMutex;
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_SURVIVAL_SURVIVAL_SYSTEM_HPP
