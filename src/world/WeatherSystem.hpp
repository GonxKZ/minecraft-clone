/**
 * @file WeatherSystem.hpp
 * @brief VoxelCraft Dynamic Weather System
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines the weather system that creates dynamic atmospheric
 * conditions, weather events, and environmental effects for the voxel world.
 */

#ifndef VOXELCRAFT_WORLD_WEATHER_SYSTEM_HPP
#define VOXELCRAFT_WORLD_WEATHER_SYSTEM_HPP

#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
#include <queue>
#include <random>
#include <atomic>
#include <mutex>
#include <shared_mutex>
#include <functional>

#include "../core/Config.hpp"
#include "World.hpp"
#include "Biome.hpp"

namespace VoxelCraft {

    // Forward declarations
    class World;
    class Entity;
    struct Biome;

    /**
     * @enum WeatherType
     * @brief Type of weather condition
     */
    enum class WeatherType {
        Clear,              ///< Clear weather
        Cloudy,             ///< Overcast/cloudy
        Rain,               ///< Light rain
        HeavyRain,          ///< Heavy rain
        Thunderstorm,       ///< Thunderstorm with lightning
        Snow,               ///< Light snow
        HeavySnow,          ///< Heavy snow
        Blizzard,           ///< Severe blizzard
        Fog,                ///< Dense fog
        Mist,               ///< Light mist
        DustStorm,          ///< Desert dust storm
        Sandstorm,          ///< Desert sandstorm
        AcidRain,           ///< Corrosive rain
        AshFall,            ///< Volcanic ash
        BloodMoon,          ///< Rare blood moon event
        Aurora,             ///< Northern lights
        Custom              ///< Custom weather type
    };

    /**
     * @enum WeatherIntensity
     * @brief Intensity level of weather effects
     */
    enum class WeatherIntensity {
        None,               ///< No weather
        Light,              ///< Light effects
        Moderate,           ///< Moderate effects
        Heavy,              ///< Heavy effects
        Extreme             ///< Extreme effects
    };

    /**
     * @struct WeatherCondition
     * @brief Current weather condition data
     */
    struct WeatherCondition {
        WeatherType type;                   ///< Weather type
        WeatherIntensity intensity;         ///< Weather intensity
        float duration;                    ///< Duration in seconds
        float elapsedTime;                 ///< Time elapsed in current condition
        double startTime;                  ///< When weather started
        double endTime;                    ///< When weather will end
        glm::vec3 windDirection;           ///< Wind direction vector
        float windSpeed;                   ///< Wind speed (m/s)
        float temperature;                 ///< Temperature effect
        float humidity;                    ///< Humidity effect
        glm::vec3 skyColor;                ///< Sky color modification
        float visibility;                  ///< Visibility modifier (0.0 - 1.0)
        float precipitation;               ///< Precipitation amount (0.0 - 1.0)
        std::unordered_map<std::string, std::any> properties; ///< Custom properties
    };

    /**
     * @struct WeatherPattern
     * @brief Defines a weather pattern with transition rules
     */
    struct WeatherPattern {
        std::string name;                  ///< Pattern name
        std::vector<WeatherType> sequence; ///< Sequence of weather types
        std::vector<float> durations;      ///< Duration for each weather type
        std::vector<float> probabilities;  ///< Transition probabilities
        bool canRepeat;                    ///< Can pattern repeat
        float patternDuration;             ///< Total pattern duration
        std::unordered_map<std::string, std::any> metadata; ///< Pattern metadata
    };

    /**
     * @struct WeatherEffect
     * @brief Visual and audio effects for weather
     */
    struct WeatherEffect {
        std::string particleSystem;        ///< Particle system name
        std::string soundEffect;           ///< Sound effect name
        std::string visualEffect;          ///< Visual effect shader
        float intensity;                   ///< Effect intensity
        float radius;                      ///< Effect radius
        glm::vec3 position;                ///< Effect position
        bool isActive;                     ///< Effect is active
        double startTime;                  ///< Effect start time
        float duration;                    ///< Effect duration
    };

    /**
     * @struct WeatherConfig
     * @brief Configuration for the weather system
     */
    struct WeatherConfig {
        // Timing settings
        float minWeatherDuration;          ///< Minimum weather duration (seconds)
        float maxWeatherDuration;          ///< Maximum weather duration (seconds)
        float transitionDuration;          ///< Weather transition duration (seconds)
        float updateInterval;              ///< Weather update interval (seconds)

        // Probability settings
        float clearProbability;            ///< Probability of clear weather
        float rainProbability;             ///< Probability of rain
        float snowProbability;             ///< Probability of snow
        float stormProbability;            ///< Probability of storms
        float extremeWeatherProbability;   ///< Probability of extreme weather

        // Environmental settings
        float maxWindSpeed;                ///< Maximum wind speed (m/s)
        float temperatureEffect;           ///< Temperature effect multiplier
        float humidityEffect;              ///< Humidity effect multiplier

        // Visual settings
        bool enableParticles;              ///< Enable weather particles
        bool enableSounds;                 ///< Enable weather sounds
        bool enableVisualEffects;          ///< Enable visual effects
        float particleDensity;             ///< Weather particle density

        // Performance settings
        int maxActiveEffects;              ///< Maximum active effects
        float effectUpdateDistance;        ///< Effect update distance
        bool enableMultithreading;         ///< Enable multithreaded weather
    };

    /**
     * @struct WeatherMetrics
     * @brief Performance metrics for the weather system
     */
    struct WeatherMetrics {
        uint64_t totalWeatherChanges;      ///< Total weather changes
        uint64_t activeWeatherEffects;     ///< Number of active effects
        double averageWeatherDuration;     ///< Average weather duration
        double weatherSystemLoad;          ///< System load percentage
        uint64_t particlesSpawned;         ///< Total particles spawned
        uint64_t soundsPlayed;             ///< Total sounds played
        size_t memoryUsage;                ///< Memory usage (bytes)
        double updateTime;                 ///< Last update time (ms)
        uint32_t biomeWeatherQueries;      ///< Biome weather queries
    };

    /**
     * @class WeatherSystem
     * @brief Dynamic weather system for voxel worlds
     *
     * The WeatherSystem provides:
     * - Dynamic weather patterns and transitions
     * - Regional weather variations based on biomes
     * - Atmospheric effects (rain, snow, fog, wind)
     * - Visual and audio weather effects
     * - Weather-dependent gameplay mechanics
     * - Seasonal weather patterns
     * - Weather prediction and forecasting
     *
     * Features:
     * - Realistic weather transitions
     * - Biome-specific weather patterns
     * - Particle-based precipitation
     * - Dynamic audio ambiance
     * - Lighting and atmospheric changes
     * - Performance-optimized effects
     */
    class WeatherSystem {
    public:
        /**
         * @brief Constructor
         * @param config Weather configuration
         */
        explicit WeatherSystem(const WeatherConfig& config);

        /**
         * @brief Destructor
         */
        ~WeatherSystem();

        /**
         * @brief Deleted copy constructor
         */
        WeatherSystem(const WeatherSystem&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        WeatherSystem& operator=(const WeatherSystem&) = delete;

        // System lifecycle

        /**
         * @brief Initialize the weather system
         * @param world Parent world instance
         * @return true if successful, false otherwise
         */
        bool Initialize(World* world);

        /**
         * @brief Shutdown the weather system
         */
        void Shutdown();

        /**
         * @brief Update weather system
         * @param deltaTime Time elapsed since last update
         */
        void Update(double deltaTime);

        // Weather control

        /**
         * @brief Set current weather condition
         * @param type Weather type
         * @param intensity Weather intensity
         * @param duration Duration in seconds (0 = automatic)
         */
        void SetWeather(WeatherType type, WeatherIntensity intensity = WeatherIntensity::Moderate, float duration = 0.0f);

        /**
         * @brief Set weather pattern
         * @param pattern Weather pattern to follow
         */
        void SetWeatherPattern(const WeatherPattern& pattern);

        /**
         * @brief Force weather change
         * @param type New weather type
         * @param intensity New weather intensity
         */
        void ForceWeatherChange(WeatherType type, WeatherIntensity intensity = WeatherIntensity::Moderate);

        /**
         * @brief Clear current weather
         */
        void ClearWeather();

        /**
         * @brief Get current weather condition
         * @return Current weather condition
         */
        const WeatherCondition& GetCurrentWeather() const { return m_currentWeather; }

        /**
         * @brief Get weather intensity
         * @return Current weather intensity (0.0 - 1.0)
         */
        float GetWeatherIntensity() const;

        /**
         * @brief Get weather transition progress
         * @return Transition progress (0.0 - 1.0)
         */
        float GetTransitionProgress() const;

        // Weather queries

        /**
         * @brief Check if it's raining at position
         * @param x World X coordinate
         * @param z World Z coordinate
         * @return true if raining, false otherwise
         */
        bool IsRainingAt(int x, int z) const;

        /**
         * @brief Check if it's snowing at position
         * @param x World X coordinate
         * @param z World Z coordinate
         * @return true if snowing, false otherwise
         */
        bool IsSnowingAt(int x, int z) const;

        /**
         * @brief Get precipitation amount at position
         * @param x World X coordinate
         * @param z World Z coordinate
         * @return Precipitation amount (0.0 - 1.0)
         */
        float GetPrecipitationAt(int x, int z) const;

        /**
         * @brief Get wind direction at position
         * @param x World X coordinate
         * @param z World Z coordinate
         * @return Wind direction vector
         */
        glm::vec3 GetWindDirectionAt(int x, int z) const;

        /**
         * @brief Get wind speed at position
         * @param x World X coordinate
         * @param z World Z coordinate
         * @return Wind speed (m/s)
         */
        float GetWindSpeedAt(int x, int z) const;

        /**
         * @brief Get temperature effect at position
         * @param x World X coordinate
         * @param z World Z coordinate
         * @return Temperature effect (-1.0 - 1.0)
         */
        float GetTemperatureEffectAt(int x, int z) const;

        /**
         * @brief Get humidity effect at position
         * @param x World X coordinate
         * @param z World Z coordinate
         * @return Humidity effect (-1.0 - 1.0)
         */
        float GetHumidityEffectAt(int x, int z) const;

        // Weather effects

        /**
         * @brief Create weather effect
         * @param effect Weather effect to create
         * @return Effect ID or empty optional on failure
         */
        std::optional<uint32_t> CreateWeatherEffect(const WeatherEffect& effect);

        /**
         * @brief Remove weather effect
         * @param effectId Effect ID
         * @return true if removed, false if not found
         */
        bool RemoveWeatherEffect(uint32_t effectId);

        /**
         * @brief Update weather effect
         * @param effectId Effect ID
         * @param effect Updated effect
         * @return true if updated, false if not found
         */
        bool UpdateWeatherEffect(uint32_t effectId, const WeatherEffect& effect);

        /**
         * @brief Get weather effect by ID
         * @param effectId Effect ID
         * @return Weather effect or empty optional if not found
         */
        std::optional<WeatherEffect> GetWeatherEffect(uint32_t effectId) const;

        // Biome integration

        /**
         * @brief Get weather probability for biome
         * @param biome Biome to query
         * @param weatherType Weather type
         * @return Probability (0.0 - 1.0)
         */
        float GetBiomeWeatherProbability(const Biome& biome, WeatherType weatherType) const;

        /**
         * @brief Get biome-appropriate weather
         * @param biome Biome to query
         * @return Appropriate weather type
         */
        WeatherType GetBiomeWeather(const Biome& biome) const;

        // Configuration

        /**
         * @brief Get weather configuration
         * @return Current configuration
         */
        const WeatherConfig& GetConfig() const { return m_config; }

        /**
         * @brief Set weather configuration
         * @param config New configuration
         */
        void SetConfig(const WeatherConfig& config);

        /**
         * @brief Get weather metrics
         * @return Performance metrics
         */
        const WeatherMetrics& GetMetrics() const { return m_metrics; }

        /**
         * @brief Reset metrics
         */
        void ResetMetrics();

        // Weather patterns

        /**
         * @brief Register weather pattern
         * @param pattern Weather pattern to register
         * @return true if registered, false if name exists
         */
        bool RegisterWeatherPattern(const WeatherPattern& pattern);

        /**
         * @brief Get weather pattern by name
         * @param name Pattern name
         * @return Weather pattern or empty optional if not found
         */
        std::optional<WeatherPattern> GetWeatherPattern(const std::string& name) const;

        /**
         * @brief List available weather patterns
         * @return Vector of pattern names
         */
        std::vector<std::string> GetAvailablePatterns() const;

    private:
        /**
         * @brief Initialize weather patterns
         */
        void InitializeWeatherPatterns();

        /**
         * @brief Initialize biome weather mappings
         */
        void InitializeBiomeWeather();

        /**
         * @brief Update current weather condition
         * @param deltaTime Time elapsed
         */
        void UpdateWeather(double deltaTime);

        /**
         * @brief Transition between weather conditions
         * @param fromWeather Previous weather
         * @param toWeather New weather
         * @param transitionTime Transition duration
         */
        void TransitionWeather(const WeatherCondition& fromWeather,
                              WeatherCondition& toWeather,
                              float transitionTime);

        /**
         * @brief Generate next weather condition
         * @return New weather condition
         */
        WeatherCondition GenerateNextWeather();

        /**
         * @brief Apply weather effects to world
         */
        void ApplyWeatherEffects();

        /**
         * @brief Update weather effects
         * @param deltaTime Time elapsed
         */
        void UpdateWeatherEffects(double deltaTime);

        /**
         * @brief Create precipitation effects
         * @param weather Current weather
         */
        void CreatePrecipitationEffects(const WeatherCondition& weather);

        /**
         * @brief Create wind effects
         * @param weather Current weather
         */
        void CreateWindEffects(const WeatherCondition& weather);

        /**
         * @brief Create atmospheric effects
         * @param weather Current weather
         */
        void CreateAtmosphericEffects(const WeatherCondition& weather);

        /**
         * @brief Update weather audio
         * @param weather Current weather
         */
        void UpdateWeatherAudio(const WeatherCondition& weather);

        /**
         * @brief Calculate regional weather variations
         * @param x World X coordinate
         * @param z World Z coordinate
         * @param baseWeather Base weather condition
         * @return Modified weather condition
         */
        WeatherCondition CalculateRegionalWeather(int x, int z, const WeatherCondition& baseWeather);

        /**
         * @brief Update weather metrics
         * @param deltaTime Time elapsed
         */
        void UpdateMetrics(double deltaTime);

        /**
         * @brief Get or create weather effect ID
         * @param effect Weather effect
         * @return Effect ID
         */
        uint32_t GetOrCreateEffectId(const WeatherEffect& effect);

        // Configuration and state
        WeatherConfig m_config;                    ///< Weather configuration
        WeatherMetrics m_metrics;                  ///< Performance metrics
        World* m_world;                           ///< Parent world

        // Current weather state
        WeatherCondition m_currentWeather;        ///< Current weather condition
        WeatherCondition m_targetWeather;         ///< Target weather for transitions
        float m_transitionProgress;               ///< Weather transition progress
        bool m_isTransitioning;                   ///< Weather transition in progress

        // Weather patterns
        std::unordered_map<std::string, WeatherPattern> m_weatherPatterns; ///< Registered patterns
        std::string m_currentPattern;             ///< Current pattern name
        size_t m_patternIndex;                    ///< Current pattern step

        // Weather effects
        std::unordered_map<uint32_t, WeatherEffect> m_weatherEffects; ///< Active effects
        mutable std::shared_mutex m_effectsMutex; ///< Effects synchronization
        std::atomic<uint32_t> m_nextEffectId;    ///< Next effect ID

        // Biome weather mappings
        std::unordered_map<BiomeType, std::vector<std::pair<WeatherType, float>>> m_biomeWeather; ///< Biome weather probabilities

        // Random number generation
        std::mt19937 m_randomEngine;              ///< Random number engine
        std::uniform_real_distribution<float> m_randomFloat; ///< Float distribution
        std::uniform_int_distribution<int> m_randomInt; ///< Integer distribution

        // Pending operations
        std::queue<std::function<void()>> m_pendingOperations; ///< Pending weather operations
        mutable std::mutex m_operationsMutex;    ///< Operations synchronization

        // Statistics
        double m_lastUpdateTime;                 ///< Last update timestamp
        uint32_t m_totalWeatherChanges;          ///< Total weather changes
        uint32_t m_totalEffectsCreated;          ///< Total effects created
    };

    /**
     * @class WeatherUtils
     * @brief Utility functions for weather operations
     */
    class WeatherUtils {
    public:
        /**
         * @brief Convert weather type to string
         * @param type Weather type
         * @return String representation
         */
        static std::string WeatherTypeToString(WeatherType type);

        /**
         * @brief Convert string to weather type
         * @param str String representation
         * @return Weather type or empty optional if invalid
         */
        static std::optional<WeatherType> StringToWeatherType(const std::string& str);

        /**
         * @brief Get weather display name
         * @param type Weather type
         * @return Display name
         */
        static std::string GetWeatherDisplayName(WeatherType type);

        /**
         * @brief Get weather intensity name
         * @param intensity Weather intensity
         * @return Intensity name
         */
        static std::string GetWeatherIntensityName(WeatherIntensity intensity);

        /**
         * @brief Check if weather type causes precipitation
         * @param type Weather type
         * @return true if causes precipitation, false otherwise
         */
        static bool CausesPrecipitation(WeatherType type);

        /**
         * @brief Check if weather type is extreme
         * @param type Weather type
         * @return true if extreme, false otherwise
         */
        static bool IsExtremeWeather(WeatherType type);

        /**
         * @brief Get weather particle system name
         * @param type Weather type
         * @return Particle system name
         */
        static std::string GetWeatherParticleSystem(WeatherType type);

        /**
         * @brief Get weather sound effect name
         * @param type Weather type
         * @return Sound effect name
         */
        static std::string GetWeatherSoundEffect(WeatherType type);

        /**
         * @brief Calculate weather visibility modifier
         * @param type Weather type
         * @param intensity Weather intensity
         * @return Visibility modifier (0.0 - 1.0)
         */
        static float CalculateVisibility(WeatherType type, WeatherIntensity intensity);

        /**
         * @brief Calculate weather movement speed modifier
         * @param type Weather type
         * @param intensity Weather intensity
         * @return Speed modifier (0.0 - 1.0)
         */
        static float CalculateMovementSpeed(WeatherType type, WeatherIntensity intensity);

        /**
         * @brief Get weather color modification
         * @param type Weather type
         * @param intensity Weather intensity
         * @return Color modification vector
         */
        static glm::vec3 GetWeatherColorMod(WeatherType type, WeatherIntensity intensity);
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_WORLD_WEATHER_SYSTEM_HPP
