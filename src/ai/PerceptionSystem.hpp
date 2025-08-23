/**
 * @file PerceptionSystem.hpp
 * @brief VoxelCraft AI Perception System for Entity Sensing
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_AI_PERCEPTION_SYSTEM_HPP
#define VOXELCRAFT_AI_PERCEPTION_SYSTEM_HPP

#include <vector>
#include <memory>
#include <unordered_map>
#include <functional>
#include <chrono>
#include <glm/glm.hpp>

namespace VoxelCraft {

    class Entity;
    class Player;
    class World;

    /**
     * @enum PerceptionType
     * @brief Types of perception sensors
     */
    enum class PerceptionType {
        SIGHT = 0,          ///< Visual perception
        HEARING,            ///< Audio perception
        SMELL,              ///< Scent perception
        TOUCH,              ///< Physical contact
        MAGICAL,            ///< Magical detection
        VIBRATION           ///< Ground vibration detection
    };

    /**
     * @enum StimulusType
     * @brief Types of stimuli that can be perceived
     */
    enum class StimulusType {
        PLAYER_SIGHT = 0,   ///< Player visible
        PLAYER_SOUND,       ///< Player made noise
        PLAYER_SMELL,       ///< Player scent
        PLAYER_TOUCH,       ///< Player touched entity
        BLOCK_BREAK,        ///< Block broken nearby
        BLOCK_PLACE,        ///< Block placed nearby
        EXPLOSION,          ///< Explosion occurred
        PROJECTILE,         ///< Projectile fired
        ENTITY_DEATH,       ///< Entity died nearby
        LIGHT_CHANGE        ///< Light level changed
    };

    /**
     * @struct PerceptionStimulus
     * @brief Represents a stimulus that can be perceived
     */
    struct PerceptionStimulus {
        StimulusType type;
        glm::vec3 position;
        float intensity = 1.0f;
        float range = 16.0f;
        std::chrono::steady_clock::time_point timestamp;
        Entity* sourceEntity = nullptr;
        void* userData = nullptr;

        PerceptionStimulus(StimulusType t = StimulusType::PLAYER_SIGHT,
                          const glm::vec3& pos = glm::vec3(0.0f))
            : type(t), position(pos), timestamp(std::chrono::steady_clock::now()) {}
    };

    /**
     * @struct PerceptionResult
     * @brief Result of a perception check
     */
    struct PerceptionResult {
        bool detected = false;
        float confidence = 0.0f;
        float distance = 0.0f;
        glm::vec3 direction = glm::vec3(0.0f);
        PerceptionStimulus stimulus;
        std::string debugInfo;

        PerceptionResult() = default;
        PerceptionResult(bool det, float conf, float dist, const glm::vec3& dir)
            : detected(det), confidence(conf), distance(dist), direction(dir) {}
    };

    /**
     * @class PerceptionSensor
     * @brief Base class for perception sensors
     */
    class PerceptionSensor {
    public:
        /**
         * @brief Constructor
         * @param type Sensor type
         * @param range Detection range
         * @param entity Owner entity
         */
        PerceptionSensor(PerceptionType type, float range, Entity* entity);

        /**
         * @brief Destructor
         */
        virtual ~PerceptionSensor() = default;

        /**
         * @brief Update sensor
         * @param deltaTime Time since last update
         */
        virtual void Update(float deltaTime) = 0;

        /**
         * @brief Check if stimulus can be perceived
         * @param stimulus Stimulus to check
         * @return Perception result
         */
        virtual PerceptionResult CanPerceive(const PerceptionStimulus& stimulus) = 0;

        /**
         * @brief Get sensor type
         * @return Sensor type
         */
        PerceptionType GetType() const { return m_type; }

        /**
         * @brief Get detection range
         * @return Detection range
         */
        float GetRange() const { return m_range; }

        /**
         * @brief Set detection range
         * @param range New range
         */
        void SetRange(float range) { m_range = range; }

        /**
         * @brief Check if sensor is enabled
         * @return true if enabled
         */
        bool IsEnabled() const { return m_enabled; }

        /**
         * @brief Enable or disable sensor
         * @param enabled Whether sensor should be enabled
         */
        void SetEnabled(bool enabled) { m_enabled = enabled; }

        /**
         * @brief Get owner entity
         * @return Owner entity
         */
        Entity* GetEntity() const { return m_entity; }

    protected:
        PerceptionType m_type;
        float m_range;
        Entity* m_entity;
        bool m_enabled = true;
        float m_updateTimer = 0.0f;
        float m_updateInterval = 0.1f; // Update every 100ms by default
    };

    /**
     * @class SightSensor
     * @brief Visual perception sensor
     */
    class SightSensor : public PerceptionSensor {
    public:
        /**
         * @brief Constructor
         * @param range Sight range
         * @param entity Owner entity
         */
        SightSensor(float range, Entity* entity);

        /**
         * @brief Update sight sensor
         * @param deltaTime Time since last update
         */
        void Update(float deltaTime) override;

        /**
         * @brief Check if can see stimulus
         * @param stimulus Stimulus to check
         * @return Perception result
         */
        PerceptionResult CanPerceive(const PerceptionStimulus& stimulus) override;

        /**
         * @brief Set field of view angle
         * @param angle FOV angle in degrees
         */
        void SetFieldOfView(float angle) { m_fieldOfView = angle; }

        /**
         * @brief Get field of view angle
         * @return FOV angle in degrees
         */
        float GetFieldOfView() const { return m_fieldOfView; }

        /**
         * @brief Set sight distance
         * @param distance Maximum sight distance
         */
        void SetSightDistance(float distance) { m_sightDistance = distance; }

        /**
         * @brief Get sight distance
         * @return Maximum sight distance
         */
        float GetSightDistance() const { return m_sightDistance; }

    private:
        float m_fieldOfView = 120.0f;      // Field of view in degrees
        float m_sightDistance = 16.0f;     // Maximum sight distance
        float m_eyeHeight = 1.6f;          // Eye height offset

        /**
         * @brief Check line of sight to position
         * @param targetPos Target position
         * @return true if line of sight is clear
         */
        bool HasLineOfSight(const glm::vec3& targetPos) const;

        /**
         * @brief Check if position is within field of view
         * @param targetPos Target position
         * @return true if within FOV
         */
        bool IsInFieldOfView(const glm::vec3& targetPos) const;

        /**
         * @brief Calculate view direction
         * @return Normalized view direction vector
         */
        glm::vec3 GetViewDirection() const;
    };

    /**
     * @class HearingSensor
     * @brief Audio perception sensor
     */
    class HearingSensor : public PerceptionSensor {
    public:
        /**
         * @brief Constructor
         * @param range Hearing range
         * @param entity Owner entity
         */
        HearingSensor(float range, Entity* entity);

        /**
         * @brief Update hearing sensor
         * @param deltaTime Time since last update
         */
        void Update(float deltaTime) override;

        /**
         * @brief Check if can hear stimulus
         * @param stimulus Stimulus to check
         * @return Perception result
         */
        PerceptionResult CanPerceive(const PerceptionStimulus& stimulus) override;

        /**
         * @brief Set hearing sensitivity
         * @param sensitivity Hearing sensitivity (0.0 - 1.0)
         */
        void SetSensitivity(float sensitivity) { m_sensitivity = sensitivity; }

        /**
         * @brief Get hearing sensitivity
         * @return Hearing sensitivity
         */
        float GetSensitivity() const { return m_sensitivity; }

    private:
        float m_sensitivity = 1.0f;

        /**
         * @brief Calculate sound attenuation
         * @param distance Distance to sound source
         * @param stimulusType Type of stimulus
         * @return Attenuation factor
         */
        float CalculateAttenuation(float distance, StimulusType stimulusType) const;
    };

    /**
     * @class PerceptionSystem
     * @brief Main perception system for entities
     */
    class PerceptionSystem {
    public:
        /**
         * @brief Constructor
         * @param entity Owner entity
         */
        PerceptionSystem(Entity* entity);

        /**
         * @brief Destructor
         */
        ~PerceptionSystem();

        /**
         * @brief Update all sensors
         * @param deltaTime Time since last update
         */
        void Update(float deltaTime);

        /**
         * @brief Add sensor to the system
         * @param sensor Sensor to add
         */
        void AddSensor(std::unique_ptr<PerceptionSensor> sensor);

        /**
         * @brief Remove sensor from the system
         * @param type Sensor type to remove
         */
        void RemoveSensor(PerceptionType type);

        /**
         * @brief Get sensor by type
         * @param type Sensor type
         * @return Sensor pointer or nullptr
         */
        PerceptionSensor* GetSensor(PerceptionType type) const;

        /**
         * @brief Check if any sensor can perceive stimulus
         * @param stimulus Stimulus to check
         * @return Best perception result
         */
        PerceptionResult CanPerceive(const PerceptionStimulus& stimulus);

        /**
         * @brief Get all perception results for stimulus
         * @param stimulus Stimulus to check
         * @return Vector of perception results
         */
        std::vector<PerceptionResult> GetAllPerceptionResults(const PerceptionStimulus& stimulus);

        /**
         * @brief Get owner entity
         * @return Owner entity
         */
        Entity* GetEntity() const { return m_entity; }

        /**
         * @brief Enable or disable all sensors
         * @param enabled Whether sensors should be enabled
         */
        void SetEnabled(bool enabled);

        /**
         * @brief Check if perception system is enabled
         * @return true if enabled
         */
        bool IsEnabled() const { return m_enabled; }

        /**
         * @brief Get perception statistics
         * @return Perception stats
         */
        const PerceptionStats& GetStats() const { return m_stats; }

        /**
         * @brief Clear perception memory
         */
        void ClearMemory();

    private:
        Entity* m_entity;
        bool m_enabled = true;
        std::unordered_map<PerceptionType, std::unique_ptr<PerceptionSensor>> m_sensors;
        PerceptionStats m_stats;
        std::vector<PerceptionStimulus> m_memory;
        float m_memoryDuration = 30.0f; // Remember stimuli for 30 seconds

        /**
         * @brief Update perception memory
         * @param deltaTime Time since last update
         */
        void UpdateMemory(float deltaTime);

        /**
         * @brief Add stimulus to memory
         * @param stimulus Stimulus to remember
         */
        void RememberStimulus(const PerceptionStimulus& stimulus);
    };

    /**
     * @struct PerceptionStats
     * @brief Statistics for perception system
     */
    struct PerceptionStats {
        int totalStimuli = 0;
        int detectedStimuli = 0;
        int missedStimuli = 0;
        float averageDetectionTime = 0.0f;
        std::unordered_map<PerceptionType, int> stimuliByType;
        std::unordered_map<PerceptionType, int> detectionsByType;
        int memorySize = 0;
        int maxMemorySize = 100;
    };

    /**
     * @class PerceptionManager
     * @brief Global manager for perception systems
     */
    class PerceptionManager {
    public:
        /**
         * @brief Get singleton instance
         * @return PerceptionManager instance
         */
        static PerceptionManager& GetInstance();

        /**
         * @brief Register entity perception system
         * @param entity Entity to register
         * @param system Perception system
         */
        void RegisterEntity(Entity* entity, std::unique_ptr<PerceptionSystem> system);

        /**
         * @brief Unregister entity perception system
         * @param entity Entity to unregister
         */
        void UnregisterEntity(Entity* entity);

        /**
         * @brief Broadcast stimulus to all entities
         * @param stimulus Stimulus to broadcast
         * @param sourceEntity Entity that generated the stimulus (optional)
         */
        void BroadcastStimulus(const PerceptionStimulus& stimulus, Entity* sourceEntity = nullptr);

        /**
         * @brief Update all perception systems
         * @param deltaTime Time since last update
         */
        void Update(float deltaTime);

        /**
         * @brief Get perception system for entity
         * @param entity Entity to get system for
         * @return Perception system or nullptr
         */
        PerceptionSystem* GetPerceptionSystem(Entity* entity) const;

    private:
        PerceptionManager() = default;
        ~PerceptionManager() = default;
        PerceptionManager(const PerceptionManager&) = delete;
        PerceptionManager& operator=(const PerceptionManager&) = delete;

        std::unordered_map<Entity*, std::unique_ptr<PerceptionSystem>> m_perceptionSystems;
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_AI_PERCEPTION_SYSTEM_HPP
