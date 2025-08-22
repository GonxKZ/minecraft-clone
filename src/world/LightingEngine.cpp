/**
 * @file LightingEngine.cpp
 * @brief VoxelCraft Lighting System - Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#include "LightingEngine.hpp"
#include "World.hpp"
#include "Chunk.hpp"
#include <algorithm>
#include <cmath>
#include <random>

namespace VoxelCraft {

    // LightingEngine Implementation
    LightingEngine::LightingEngine()
        : m_gameTime(0)
        , m_dayTime(6000) // Start at noon
        , m_timeSpeed(1.0f)
        , m_weatherTimer(0.0f)
        , m_weatherDuration(1200.0f) // 20 minutes default
        , m_cachedTimeOfDay(TimeOfDay::DAY)
    {
        InitializeSkyColors();
        InitializeFogColors();
        UpdateCachedValues();
    }

    void LightingEngine::Update(float deltaTime) {
        UpdateGameTime(deltaTime);
        UpdateWeather(deltaTime);
        UpdateCachedValues();
    }

    TimeOfDay LightingEngine::GetTimeOfDay() const {
        return m_cachedTimeOfDay;
    }

    void LightingEngine::SetGameTime(int64_t time) {
        m_gameTime = time;
        m_dayTime = static_cast<int>(m_gameTime % 24000);
        UpdateCachedValues();
    }

    void LightingEngine::SetDayTime(int time) {
        m_dayTime = time % 24000;
        UpdateCachedValues();
    }

    uint8_t LightingEngine::GetSkyLightLevel(int y) const {
        return CalculateSkyLightLevel(y);
    }

    LightLevel LightingEngine::GetLightLevel(const Vec3& position) const {
        // This would need to query the world/chunk for actual light data
        // For now, return basic calculation
        uint8_t skyLight = CalculateSkyLightLevel(static_cast<int>(position.y));
        uint8_t blockLight = 0; // Would query block light sources

        return LightLevel(skyLight, blockLight);
    }

    LightLevel LightingEngine::GetLightLevel(const Chunk* chunk, int localX, int y, int localZ) const {
        // Would query chunk light data
        uint8_t skyLight = CalculateSkyLightLevel(y);
        uint8_t blockLight = 0; // Would query chunk block lights

        return LightLevel(skyLight, blockLight);
    }

    void LightingEngine::SetBlockLightLevel(const Vec3& position, uint8_t level) {
        // Would update block light data in chunk
        if (level > 0) {
            LightSource source(position, level, LightType::BLOCK);
            AddLightSource(source);
        } else {
            RemoveLightSource(position);
        }
    }

    void LightingEngine::AddLightSource(const LightSource& lightSource) {
        std::lock_guard<std::mutex> lock(m_lightSourcesMutex);
        m_lightSources[lightSource.position] = lightSource;
    }

    void LightingEngine::RemoveLightSource(const Vec3& position) {
        std::lock_guard<std::mutex> lock(m_lightSourcesMutex);
        m_lightSources.erase(position);
    }

    SkyColor LightingEngine::GetSkyColor() const {
        return m_cachedSkyColor;
    }

    SkyColor LightingEngine::GetFogColor() const {
        return m_cachedFogColor;
    }

    void LightingEngine::SetWeather(const WeatherCondition& weather) {
        m_weather = weather;
        m_weatherTimer = 0.0f;
        UpdateCachedValues();
    }

    int LightingEngine::GetMoonPhase() const {
        // Moon phase changes every 8 days (192000 ticks)
        return static_cast<int>((m_gameTime / 24000) % 8);
    }

    void LightingEngine::CalculateLightPropagation(Chunk* chunk) {
        UpdateSkyLight(chunk);
        UpdateBlockLight(chunk);
    }

    void LightingEngine::UpdateSkyLight(Chunk* chunk) {
        // Would implement sky light calculation for chunk
        // This involves ray tracing from sky to each block
    }

    void LightingEngine::UpdateBlockLight(Chunk* chunk) {
        // Would implement block light calculation for chunk
        // This involves flood fill from light sources
    }

    bool LightingEngine::CanSeeSky(const Vec3& position) const {
        // Would implement sky visibility check
        // This would check blocks above the position
        return position.y >= 255; // Simplified
    }

    float LightingEngine::GetBrightness() const {
        // Calculate brightness based on time and weather
        float timeBrightness = GetSkyLightLevel(64) / 15.0f; // Light at sea level

        // Weather affects brightness
        float weatherMultiplier = 1.0f;
        if (IsRaining()) {
            weatherMultiplier = 0.6f - m_weather.intensity * 0.3f;
        } else if (IsSnowing()) {
            weatherMultiplier = 0.8f - m_weather.intensity * 0.2f;
        }

        return timeBrightness * weatherMultiplier;
    }

    float LightingEngine::GameTimeToHours(int gameTime) {
        // 24000 ticks per day, 20 minutes per day in real time
        // 72 minutes per hour = 1.2 hours per game hour
        return (gameTime % 24000) / 1000.0f;
    }

    int LightingEngine::HoursToGameTime(float hours) {
        return static_cast<int>(hours * 1000.0f) % 24000;
    }

    void LightingEngine::UpdateGameTime(float deltaTime) {
        // 20 ticks per second normally
        int ticksToAdd = static_cast<int>(20.0f * deltaTime * m_timeSpeed);
        m_gameTime += ticksToAdd;
        m_dayTime = static_cast<int>(m_gameTime % 24000);
    }

    void LightingEngine::UpdateWeather(float deltaTime) {
        m_weatherTimer += deltaTime;

        if (m_weatherTimer >= m_weatherDuration) {
            // Change weather
            static std::random_device rd;
            static std::mt19937 gen(rd());
            static std::uniform_real_distribution<float> weatherRand(0.0f, 1.0f);

            float randValue = weatherRand(gen);

            if (randValue < 0.3f) {
                // 30% chance of rain
                m_weather.type = WeatherCondition::Type::RAIN;
                m_weather.intensity = weatherRand(gen) * 0.8f + 0.2f;
            } else if (randValue < 0.35f) {
                // 5% chance of thunderstorm
                m_weather.type = WeatherCondition::Type::THUNDERSTORM;
                m_weather.intensity = weatherRand(gen) * 0.6f + 0.4f;
            } else {
                // 65% chance of clear
                m_weather.type = WeatherCondition::Type::CLEAR;
                m_weather.intensity = 0.0f;
            }

            m_weatherTimer = 0.0f;
            m_weatherDuration = 600.0f + weatherRand(gen) * 1200.0f; // 10-30 minutes
        }
    }

    void LightingEngine::UpdateCachedValues() {
        // Update time of day
        int hour = m_dayTime / 1000;
        if (hour >= 18 || hour < 6) {
            m_cachedTimeOfDay = TimeOfDay::NIGHT;
        } else if (hour >= 6 && hour < 7) {
            m_cachedTimeOfDay = TimeOfDay::DAWN;
        } else if (hour >= 7 && hour < 17) {
            m_cachedTimeOfDay = TimeOfDay::DAY;
        } else if (hour >= 17 && hour < 18) {
            m_cachedTimeOfDay = TimeOfDay::DUSK;
        }

        // Special case for midnight
        if (m_dayTime >= 18000 && m_dayTime < 18000 + 1000) {
            m_cachedTimeOfDay = TimeOfDay::MIDNIGHT;
        }

        // Update colors
        float timeProgress = (m_dayTime % 1000) / 1000.0f;
        m_cachedSkyColor = GetInterpolatedSkyColor(m_cachedTimeOfDay, timeProgress);
        m_cachedFogColor = GetInterpolatedFogColor(m_cachedTimeOfDay, timeProgress);

        // Apply weather effects
        if (m_weather.type != WeatherCondition::Type::CLEAR) {
            // Darken sky during weather
            float darkenFactor = m_weather.intensity * 0.3f;
            m_cachedSkyColor.r *= (1.0f - darkenFactor);
            m_cachedSkyColor.g *= (1.0f - darkenFactor);
            m_cachedSkyColor.b *= (1.0f - darkenFactor);
        }
    }

    uint8_t LightingEngine::CalculateSkyLightLevel(int y) const {
        // Sky light decreases with depth
        if (y >= 255) return 15; // Full sky light above world

        int skyLight = 15;

        // Reduce light based on time of day
        switch (m_cachedTimeOfDay) {
            case TimeOfDay::DAY:
                skyLight = 15;
                break;
            case TimeOfDay::DAWN:
            case TimeOfDay::DUSK:
                skyLight = 12;
                break;
            case TimeOfDay::NIGHT:
                skyLight = 4; // Moon light
                break;
            case TimeOfDay::MIDNIGHT:
                skyLight = 0; // No light at midnight
                break;
        }

        // Reduce light with depth
        if (y < 255) {
            int depth = 255 - y;
            skyLight = std::max(0, skyLight - depth / 16);
        }

        return static_cast<uint8_t>(skyLight);
    }

    uint8_t LightingEngine::GetBlockLightLevel(BlockType blockType) const {
        switch (blockType) {
            case BlockType::TORCH:
                return 14;
            case BlockType::GLOWSTONE:
                return 15;
            case BlockType::LAVA:
                return 15;
            case BlockType::SOUL_TORCH:
                return 10;
            case BlockType::SEA_LANTERN:
                return 15;
            case BlockType::END_ROD:
                return 14;
            case BlockType::CRYING_OBSIDIAN:
                return 10;
            case BlockType::SOUL_LANTERN:
                return 10;
            case BlockType::CAMPFIRE:
            case BlockType::SOUL_CAMPFIRE:
                return 15;
            case BlockType::LANTERN:
            case BlockType::SOUL_LANTERN:
                return 15;
            default:
                return 0;
        }
    }

    void LightingEngine::PropagateLight(const Vec3& position, uint8_t level, LightType type) {
        // Would implement light propagation algorithm
        // This is a complex flood fill algorithm that would need to be implemented
        // based on the specific lighting system design
    }

    void LightingEngine::InitializeSkyColors() {
        // Minecraft-like sky colors for different times
        m_skyColors[TimeOfDay::DAY] = SkyColor(0.6f, 0.8f, 1.0f, 1.0f);
        m_skyColors[TimeOfDay::DAWN] = SkyColor(1.0f, 0.6f, 0.4f, 1.0f);
        m_skyColors[TimeOfDay::DUSK] = SkyColor(1.0f, 0.4f, 0.2f, 1.0f);
        m_skyColors[TimeOfDay::NIGHT] = SkyColor(0.0f, 0.0f, 0.2f, 1.0f);
        m_skyColors[TimeOfDay::MIDNIGHT] = SkyColor(0.0f, 0.0f, 0.0f, 1.0f);
    }

    void LightingEngine::InitializeFogColors() {
        // Fog colors - usually darker versions of sky colors
        m_fogColors[TimeOfDay::DAY] = SkyColor(0.8f, 0.9f, 1.0f, 1.0f);
        m_fogColors[TimeOfDay::DAWN] = SkyColor(1.0f, 0.7f, 0.5f, 1.0f);
        m_fogColors[TimeOfDay::DUSK] = SkyColor(1.0f, 0.5f, 0.3f, 1.0f);
        m_fogColors[TimeOfDay::NIGHT] = SkyColor(0.0f, 0.0f, 0.3f, 1.0f);
        m_fogColors[TimeOfDay::MIDNIGHT] = SkyColor(0.0f, 0.0f, 0.1f, 1.0f);
    }

    SkyColor LightingEngine::GetInterpolatedSkyColor(TimeOfDay time, float factor) const {
        auto it = m_skyColors.find(time);
        if (it == m_skyColors.end()) {
            return SkyColor(1.0f, 1.0f, 1.0f, 1.0f);
        }

        // For transitions, we would interpolate between adjacent colors
        // For now, just return the base color
        return it->second;
    }

    SkyColor LightingEngine::GetInterpolatedFogColor(TimeOfDay time, float factor) const {
        auto it = m_fogColors.find(time);
        if (it == m_fogColors.end()) {
            return SkyColor(1.0f, 1.0f, 1.0f, 1.0f);
        }

        return it->second;
    }

    // DayNightCycle Implementation
    DayNightCycle::DayNightCycle(LightingEngine* lightingEngine)
        : m_lightingEngine(lightingEngine)
        , m_currentTimeOfDay(TimeOfDay::DAY)
        , m_phaseProgress(0.0f)
        , m_isTransitioning(false)
        , m_transitionTimer(0.0f)
    {
    }

    void DayNightCycle::Update(float deltaTime) {
        UpdateCurrentTimeOfDay();

        if (m_isTransitioning) {
            m_transitionTimer += deltaTime;
            float transitionDuration = GetTransitionDuration(m_currentTimeOfDay, m_currentTimeOfDay);
            m_phaseProgress = std::min(1.0f, m_transitionTimer / transitionDuration);

            if (m_phaseProgress >= 1.0f) {
                m_isTransitioning = false;
                m_phaseProgress = 0.0f;
                m_transitionTimer = 0.0f;
            }
        }
    }

    float DayNightCycle::GetPhaseDuration() const {
        // Return duration in seconds for current time period
        switch (m_currentTimeOfDay) {
            case TimeOfDay::DAWN:
            case TimeOfDay::DUSK:
                return 60.0f; // 1 minute transitions
            case TimeOfDay::DAY:
                return 720.0f; // 12 minutes day
            case TimeOfDay::NIGHT:
                return 480.0f; // 8 minutes night
            case TimeOfDay::MIDNIGHT:
                return 60.0f; // 1 minute midnight
            default:
                return 600.0f; // 10 minutes default
        }
    }

    void DayNightCycle::ForceTimeOfDay(TimeOfDay timeOfDay) {
        TimeOfDay oldTime = m_currentTimeOfDay;
        m_currentTimeOfDay = timeOfDay;
        HandleTimeTransition(oldTime, timeOfDay);
    }

    void DayNightCycle::AddTimeChangeListener(std::function<void(TimeOfDay oldTime, TimeOfDay newTime)> listener) {
        m_timeChangeListeners.push_back(listener);
    }

    void DayNightCycle::ClearTimeChangeListeners() {
        m_timeChangeListeners.clear();
    }

    void DayNightCycle::UpdateCurrentTimeOfDay() {
        TimeOfDay newTime = m_lightingEngine->GetTimeOfDay();

        if (newTime != m_currentTimeOfDay) {
            HandleTimeTransition(m_currentTimeOfDay, newTime);
            m_currentTimeOfDay = newTime;
        }
    }

    void DayNightCycle::HandleTimeTransition(TimeOfDay oldTime, TimeOfDay newTime) {
        m_isTransitioning = true;
        m_transitionTimer = 0.0f;
        m_phaseProgress = 0.0f;

        // Notify listeners
        for (const auto& listener : m_timeChangeListeners) {
            listener(oldTime, newTime);
        }
    }

    float DayNightCycle::GetTransitionDuration(TimeOfDay fromTime, TimeOfDay toTime) const {
        // Define transition durations between time periods
        if ((fromTime == TimeOfDay::DAY && toTime == TimeOfDay::DUSK) ||
            (fromTime == TimeOfDay::DUSK && toTime == TimeOfDay::NIGHT) ||
            (fromTime == TimeOfDay::NIGHT && toTime == TimeOfDay::DAWN) ||
            (fromTime == TimeOfDay::DAWN && toTime == TimeOfDay::DAY)) {
            return 60.0f; // 1 minute for dawn/dusk transitions
        }

        return 30.0f; // 30 seconds for other transitions
    }

} // namespace VoxelCraft
