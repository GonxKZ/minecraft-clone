/**
 * @file LightingEngine.hpp
 * @brief VoxelCraft Lighting System - Day/Night Cycle and Dynamic Lighting
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_WORLD_LIGHTING_ENGINE_HPP
#define VOXELCRAFT_WORLD_LIGHTING_ENGINE_HPP

#include <vector>
#include <memory>
#include <unordered_map>
#include <atomic>
#include <mutex>

#include "../blocks/Block.hpp"

namespace VoxelCraft {

    // Forward declarations
    class World;
    class Chunk;
    struct Vec3;

    /**
     * @enum LightType
     * @brief Types of light sources
     */
    enum class LightType {
        SKY,           // Sun/moon light
        BLOCK,         // Block light (torches, lava, etc.)
        AMBIENT        // Ambient light level
    };

    /**
     * @enum TimeOfDay
     * @brief Time periods in the day
     */
    enum class TimeOfDay {
        DAWN,          // Sunrise (6:00)
        DAY,           // Daytime (6:00 - 18:00)
        DUSK,          // Sunset (18:00)
        NIGHT,         // Nighttime (18:00 - 6:00)
        MIDNIGHT       // Midnight (00:00)
    };

    /**
     * @struct LightLevel
     * @brief Light level information
     */
    struct LightLevel {
        uint8_t sky;           // Sky light level (0-15)
        uint8_t block;         // Block light level (0-15)
        uint8_t ambient;       // Ambient light level (0-15)

        LightLevel()
            : sky(15)
            , block(0)
            , ambient(0)
        {}

        LightLevel(uint8_t s, uint8_t b, uint8_t a = 0)
            : sky(s)
            , block(b)
            , ambient(a)
        {}

        /**
         * @brief Get combined light level
         * @return Combined light level
         */
        uint8_t GetCombined() const {
            return std::max({sky, block, ambient});
        }

        /**
         * @brief Check if position is dark enough for mob spawning
         * @return true if dark enough
         */
        bool IsDarkForMobs() const {
            return GetCombined() <= 7;
        }

        /**
         * @brief Check if plants can grow
         * @return true if light level is sufficient
         */
        bool CanPlantsGrow() const {
            return sky >= 9 || block >= 9;
        }
    };

    /**
     * @struct LightSource
     * @brief Information about a light source
     */
    struct LightSource {
        Vec3 position;                // Position of light source
        uint8_t level;               // Light level (0-15)
        LightType type;              // Type of light source
        BlockType blockType;         // Block type if block light
        float radius;                // Light radius
        bool isDynamic;              // Whether light can change
        float flickerIntensity;      // Flicker intensity (0-1)

        LightSource()
            : level(15)
            , type(LightType::SKY)
            , blockType(BlockType::AIR)
            , radius(15.0f)
            , isDynamic(false)
            , flickerIntensity(0.0f)
        {}

        LightSource(const Vec3& pos, uint8_t lvl, LightType t = LightType::BLOCK)
            : position(pos)
            , level(lvl)
            , type(t)
            , blockType(BlockType::AIR)
            , radius(static_cast<float>(lvl))
            , isDynamic(false)
            , flickerIntensity(0.0f)
        {}
    };

    /**
     * @struct SkyColor
     * @brief Sky color information
     */
    struct SkyColor {
        float r, g, b, a;            // RGBA color components

        SkyColor(float red = 1.0f, float green = 1.0f, float blue = 1.0f, float alpha = 1.0f)
            : r(red), g(green), b(blue), a(alpha)
        {}

        /**
         * @brief Interpolate between two colors
         * @param other Other color
         * @param t Interpolation factor (0-1)
         * @return Interpolated color
         */
        SkyColor Interpolate(const SkyColor& other, float t) const {
            return SkyColor(
                r + (other.r - r) * t,
                g + (other.g - g) * t,
                b + (other.b - b) * t,
                a + (other.a - a) * t
            );
        }

        /**
         * @brief Convert to array
         * @return Color as float array
         */
        std::array<float, 4> ToArray() const {
            return {r, g, b, a};
        }
    };

    /**
     * @struct WeatherCondition
     * @brief Weather information
     */
    struct WeatherCondition {
        enum class Type {
            CLEAR,
            RAIN,
            THUNDERSTORM,
            SNOW
        };

        Type type;
        float intensity;              // Weather intensity (0-1)
        float temperatureEffect;      // Effect on temperature
        SkyColor skyColor;            // Sky color modification
        float visibility;             // Visibility multiplier (0-1)

        WeatherCondition()
            : type(Type::CLEAR)
            , intensity(0.0f)
            , temperatureEffect(0.0f)
            , visibility(1.0f)
        {}
    };

    /**
     * @class LightingEngine
     * @brief Handles all lighting calculations and day/night cycle
     */
    class LightingEngine {
    public:
        /**
         * @brief Constructor
         */
        LightingEngine();

        /**
         * @brief Update lighting system
         * @param deltaTime Time since last update
         */
        void Update(float deltaTime);

        /**
         * @brief Get current time of day
         * @return Time of day
         */
        TimeOfDay GetTimeOfDay() const;

        /**
         * @brief Get current game time
         * @return Game time in ticks
         */
        int64_t GetGameTime() const { return m_gameTime; }

        /**
         * @brief Set game time
         * @param time New game time
         */
        void SetGameTime(int64_t time);

        /**
         * @brief Get day time (0-24000)
         * @return Day time
         */
        int GetDayTime() const { return m_dayTime; }

        /**
         * @brief Set day time
         * @param time New day time
         */
        void SetDayTime(int time);

        /**
         * @brief Get current sky light level
         * @param y Y coordinate
         * @return Sky light level (0-15)
         */
        uint8_t GetSkyLightLevel(int y) const;

        /**
         * @brief Get light level at position
         * @param position World position
         * @return Light level information
         */
        LightLevel GetLightLevel(const Vec3& position) const;

        /**
         * @brief Get light level in chunk
         * @param chunk Chunk to query
         * @param localX Local X coordinate (0-15)
         * @param y World Y coordinate
         * @param localZ Local Z coordinate (0-15)
         * @return Light level information
         */
        LightLevel GetLightLevel(const Chunk* chunk, int localX, int y, int localZ) const;

        /**
         * @brief Set block light level
         * @param position World position
         * @param level Light level (0-15)
         */
        void SetBlockLightLevel(const Vec3& position, uint8_t level);

        /**
         * @brief Add light source
         * @param lightSource Light source to add
         */
        void AddLightSource(const LightSource& lightSource);

        /**
         * @brief Remove light source
         * @param position Position of light source
         */
        void RemoveLightSource(const Vec3& position);

        /**
         * @brief Get sky color for current time
         * @return Current sky color
         */
        SkyColor GetSkyColor() const;

        /**
         * @brief Get fog color for current time
         * @return Current fog color
         */
        SkyColor GetFogColor() const;

        /**
         * @brief Get current weather condition
         * @return Weather condition
         */
        const WeatherCondition& GetWeather() const { return m_weather; }

        /**
         * @brief Set weather condition
         * @param weather New weather condition
         */
        void SetWeather(const WeatherCondition& weather);

        /**
         * @brief Check if it's raining
         * @return true if raining
         */
        bool IsRaining() const { return m_weather.type == WeatherCondition::Type::RAIN; }

        /**
         * @brief Check if there's a thunderstorm
         * @return true if thunderstorm
         */
        bool IsThunderstorm() const { return m_weather.type == WeatherCondition::Type::THUNDERSTORM; }

        /**
         * @brief Check if it's snowing
         * @return true if snowing
         */
        bool IsSnowing() const { return m_weather.type == WeatherCondition::Type::SNOW; }

        /**
         * @brief Get moon phase
         * @return Moon phase (0-7)
         */
        int GetMoonPhase() const;

        /**
         * @brief Calculate light propagation
         * @param chunk Chunk to update
         */
        void CalculateLightPropagation(Chunk* chunk);

        /**
         * @brief Update sky light for chunk
         * @param chunk Chunk to update
         */
        void UpdateSkyLight(Chunk* chunk);

        /**
         * @brief Update block light for chunk
         * @param chunk Chunk to update
         */
        void UpdateBlockLight(Chunk* chunk);

        /**
         * @brief Check if position can see sky
         * @param position World position
         * @return true if can see sky
         */
        bool CanSeeSky(const Vec3& position) const;

        /**
         * @brief Get brightness multiplier for current time
         * @return Brightness multiplier (0-1)
         */
        float GetBrightness() const;

        /**
         * @brief Set time speed multiplier
         * @param multiplier Speed multiplier
         */
        void SetTimeSpeed(float multiplier) { m_timeSpeed = multiplier; }

        /**
         * @brief Get time speed multiplier
         * @return Speed multiplier
         */
        float GetTimeSpeed() const { return m_timeSpeed; }

        /**
         * @brief Convert game time to hours
         * @param gameTime Game time
         * @return Hours (0-24)
         */
        static float GameTimeToHours(int gameTime);

        /**
         * @brief Convert hours to game time
         * @param hours Hours (0-24)
         * @return Game time
         */
        static int HoursToGameTime(float hours);

    private:
        // Time system
        int64_t m_gameTime;                    // Total game time in ticks
        int m_dayTime;                        // Time of day (0-24000)
        float m_timeSpeed;                    // Time speed multiplier

        // Lighting data
        std::unordered_map<Vec3, LightSource, std::hash<Vec3>> m_lightSources;
        mutable std::mutex m_lightSourcesMutex;

        // Weather system
        WeatherCondition m_weather;
        float m_weatherTimer;                 // Timer for weather changes
        float m_weatherDuration;              // Duration of current weather

        // Sky colors for different times
        std::unordered_map<TimeOfDay, SkyColor> m_skyColors;
        std::unordered_map<TimeOfDay, SkyColor> m_fogColors;

        // Cached values
        mutable TimeOfDay m_cachedTimeOfDay;
        mutable SkyColor m_cachedSkyColor;
        mutable SkyColor m_cachedFogColor;

        /**
         * @brief Update game time
         * @param deltaTime Time since last update
         */
        void UpdateGameTime(float deltaTime);

        /**
         * @brief Update weather system
         * @param deltaTime Time since last update
         */
        void UpdateWeather(float deltaTime);

        /**
         * @brief Update cached values
         */
        void UpdateCachedValues();

        /**
         * @brief Calculate sky light level for Y coordinate
         * @param y Y coordinate
         * @return Sky light level (0-15)
         */
        uint8_t CalculateSkyLightLevel(int y) const;

        /**
         * @brief Get light level from block
         * @param blockType Block type
         * @return Light level (0-15)
         */
        uint8_t GetBlockLightLevel(BlockType blockType) const;

        /**
         * @brief Propagate light from position
         * @param position Starting position
         * @param level Light level
         * @param type Light type
         */
        void PropagateLight(const Vec3& position, uint8_t level, LightType type);

        /**
         * @brief Initialize sky colors
         */
        void InitializeSkyColors();

        /**
         * @brief Initialize fog colors
         */
        void InitializeFogColors();

        /**
         * @brief Get interpolated sky color
         * @param time Time of day
         * @param factor Interpolation factor (0-1)
         * @return Interpolated color
         */
        SkyColor GetInterpolatedSkyColor(TimeOfDay time, float factor) const;

        /**
         * @brief Get interpolated fog color
         * @param time Time of day
         * @param factor Interpolation factor (0-1)
         * @return Interpolated color
         */
        SkyColor GetInterpolatedFogColor(TimeOfDay time, float factor) const;
    };

    /**
     * @class DayNightCycle
     * @brief Manages the day/night cycle and transitions
     */
    class DayNightCycle {
    public:
        /**
         * @brief Constructor
         * @param lightingEngine Lighting engine reference
         */
        DayNightCycle(LightingEngine* lightingEngine);

        /**
         * @brief Update day/night cycle
         * @param deltaTime Time since last update
         */
        void Update(float deltaTime);

        /**
         * @brief Get current phase progress
         * @return Progress (0-1) within current phase
         */
        float GetPhaseProgress() const { return m_phaseProgress; }

        /**
         * @brief Get current phase duration
         * @return Phase duration in seconds
         */
        float GetPhaseDuration() const;

        /**
         * @brief Check if it's currently transitioning
         * @return true if transitioning
         */
        bool IsTransitioning() const { return m_isTransitioning; }

        /**
         * @brief Force time of day
         * @param timeOfDay Time of day to set
         */
        void ForceTimeOfDay(TimeOfDay timeOfDay);

        /**
         * @brief Add time change listener
         * @param listener Function called when time changes
         */
        void AddTimeChangeListener(std::function<void(TimeOfDay oldTime, TimeOfDay newTime)> listener);

        /**
         * @brief Remove all time change listeners
         */
        void ClearTimeChangeListeners();

    private:
        LightingEngine* m_lightingEngine;
        TimeOfDay m_currentTimeOfDay;
        float m_phaseProgress;
        bool m_isTransitioning;
        float m_transitionTimer;

        std::vector<std::function<void(TimeOfDay oldTime, TimeOfDay newTime)>> m_timeChangeListeners;

        /**
         * @brief Update current time of day
         */
        void UpdateCurrentTimeOfDay();

        /**
         * @brief Handle time transition
         * @param oldTime Previous time of day
         * @param newTime New time of day
         */
        void HandleTimeTransition(TimeOfDay oldTime, TimeOfDay newTime);

        /**
         * @brief Get transition duration between times
         * @param fromTime Starting time
         * @param toTime Ending time
         * @return Transition duration in seconds
         */
        float GetTransitionDuration(TimeOfDay fromTime, TimeOfDay toTime) const;
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_WORLD_LIGHTING_ENGINE_HPP