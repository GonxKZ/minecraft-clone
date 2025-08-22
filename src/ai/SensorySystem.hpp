/**
 * @file SensorySystem.hpp
 * @brief VoxelCraft AI Sensory System
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines the SensorySystem class that provides sensory perception
 * capabilities for AI entities, including vision, hearing, smell, and other
 * senses that allow mobs to perceive and react to their environment.
 */

#ifndef VOXELCRAFT_AI_SENSORY_SYSTEM_HPP
#define VOXELCRAFT_AI_SENSORY_SYSTEM_HPP

#include <memory>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <chrono>
#include <queue>
#include <any>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../core/Config.hpp"
#include "../physics/PhysicsTypes.hpp"

namespace VoxelCraft {

    // Forward declarations
    class Mob;
    class Entity;
    class World;
    class Blackboard;

    /**
     * @enum SenseType
     * @brief Types of senses available to AI entities
     */
    enum class SenseType {
        Vision,                 ///< Visual perception
        Hearing,                ///< Auditory perception
        Smell,                  ///< Olfactory perception
        Touch,                  ///< Tactile perception
        Vibration,              ///< Vibration detection
        Magnetic,               ///< Magnetic field detection
        Thermal,                ///< Temperature perception
        Chemical,               ///< Chemical detection
        Electric,               ///< Electric field detection
        Pressure,               ///< Pressure perception
        Custom                 ///< Custom sense type
    };

    /**
     * @enum StimulusType
     * @brief Types of stimuli that can be detected
     */
    enum class StimulusType {
        Entity,                 ///< Other entities (players, mobs, etc.)
        Sound,                  ///< Sound sources
        Scent,                  ///< Scent sources
        Light,                  ///< Light sources
        Movement,               ///< Movement detection
        Vibration,              ///< Ground vibrations
        Temperature,            ///< Temperature changes
        Chemical,               ///< Chemical substances
        Projectile,             ///< Projectiles and thrown objects
        Environmental,          ///< Environmental changes
        Custom                 ///< Custom stimulus type
    };

    /**
     * @enum SensoryState
     * @brief Current state of sensory processing
     */
    enum class SensoryState {
        Active,                 ///< Sensory system is active
        Inactive,               ///< Sensory system is inactive
        Degraded,               ///< Sensory system is degraded
        Overloaded,             ///< Sensory system is overloaded
        Error                  ///< Sensory system is in error state
    };

    /**
     * @struct Stimulus
     * @brief A sensory stimulus detected by an AI entity
     */
    struct Stimulus {
        StimulusType type;                      ///< Type of stimulus
        SenseType senseType;                    ///< Sense that detected it
        glm::vec3 position;                     ///< Position of stimulus
        glm::vec3 direction;                    ///< Direction of stimulus
        float intensity;                        ///< Intensity of stimulus (0.0 - 1.0)
        float confidence;                       ///< Detection confidence (0.0 - 1.0)
        double timestamp;                       ///< Detection timestamp
        Entity* source;                         ///< Source entity (if applicable)
        std::unordered_map<std::string, std::any> properties; ///< Additional properties

        Stimulus()
            : intensity(0.0f)
            , confidence(0.0f)
            , timestamp(0.0)
            , source(nullptr)
        {}
    };

    /**
     * @struct SensoryMemory
     * @brief Memory of past sensory stimuli
     */
    struct SensoryMemory {
        Stimulus stimulus;                      ///< The remembered stimulus
        double memoryStrength;                  ///< How strong the memory is (0.0 - 1.0)
        double firstDetected;                   ///< When first detected
        double lastDetected;                    ///< When last detected
        int detectionCount;                     ///< How many times detected
        float averageIntensity;                 ///< Average intensity
        std::vector<glm::vec3> positionHistory; ///< Position history
        std::unordered_map<std::string, std::any> metadata; ///< Additional memory data
    };

    /**
     * @struct SenseConfig
     * @brief Configuration for individual senses
     */
    struct SenseConfig {
        SenseType senseType;                    ///< Type of sense
        float range;                           ///< Detection range
        float fieldOfView;                     ///< Field of view (degrees, for vision)
        float sensitivity;                     ///< Sensitivity multiplier
        float noiseLevel;                      ///< Background noise level
        bool enabled;                          ///< Sense is enabled
        float updateFrequency;                 ///< Update frequency (Hz)
        std::unordered_map<std::string, float> customParameters; ///< Custom parameters
    };

    /**
     * @struct SensorySystemConfig
     * @brief Configuration for the sensory system
     */
    struct SensorySystemConfig {
        // Basic settings
        float globalSensitivity;                ///< Global sensitivity multiplier
        float maxRange;                        ///< Maximum sensory range
        bool enableMemory;                     ///< Enable sensory memory
        float memoryDuration;                  ///< How long memories last (seconds)
        int maxMemories;                       ///< Maximum number of memories

        // Processing settings
        float updateInterval;                  ///< Update interval (seconds)
        int maxStimuliPerUpdate;               ///< Maximum stimuli processed per update
        bool enableMultithreading;             ///< Enable multithreaded processing
        float processingBudget;                ///< Maximum processing time (ms)

        // Vision settings
        bool enableVision;                     ///< Enable vision sense
        float defaultSightRange;               ///< Default sight range
        float defaultFieldOfView;              ///< Default field of view (degrees)
        bool enableOcclusionChecking;          ///< Enable occlusion checking
        int visionRayCount;                    ///< Number of vision rays

        // Hearing settings
        bool enableHearing;                    ///< Enable hearing sense
        float defaultHearingRange;             ///< Default hearing range
        float hearingThreshold;                ///< Minimum sound intensity to detect
        bool enableSoundPropagation;           ///< Enable sound propagation modeling
        float soundAttenuation;                ///< Sound attenuation factor

        // Smell settings
        bool enableSmell;                      ///< Enable smell sense
        float defaultSmellRange;               ///< Default smell range
        float smellThreshold;                  ///< Minimum scent intensity to detect
        bool enableScentDiffusion;             ///< Enable scent diffusion modeling
        float scentDecayRate;                  ///< Scent decay rate

        // Advanced settings
        bool enableSenseDegradation;           ///< Enable sense degradation over time
        bool enableSenseFatigue;               ///< Enable sense fatigue
        bool enableSenseAdaptation;            ///< Enable sense adaptation
        bool enableCrossModalProcessing;       ///< Enable cross-modal stimulus processing

        // Debug settings
        bool enableDebugDrawing;               ///< Enable sensory debug drawing
        bool showVisionRays;                   ///< Show vision rays
        bool showHearingRange;                 ///< Show hearing range
        bool showSmellRange;                   ///< Show smell range
        bool logSensoryEvents;                 ///< Log sensory events
    };

    /**
     * @struct SensorySystemMetrics
     * @brief Performance metrics for sensory system
     */
    struct SensorySystemMetrics {
        // Performance metrics
        uint64_t updateCount;                  ///< Number of updates performed
        double totalUpdateTime;                ///< Total update time (ms)
        double averageUpdateTime;              ///< Average update time (ms)
        double maxUpdateTime;                  ///< Maximum update time (ms)

        // Stimulus metrics
        uint64_t stimuliDetected;              ///< Total stimuli detected
        uint64_t stimuliProcessed;             ///< Total stimuli processed
        uint64_t stimuliIgnored;               ///< Total stimuli ignored
        uint64_t falsePositives;               ///< False positive detections

        // Sense metrics
        std::unordered_map<SenseType, uint64_t> senseUsage; ///< Per-sense usage
        std::unordered_map<SenseType, double> senseProcessingTime; ///< Per-sense processing time
        std::unordered_map<SenseType, uint64_t> senseDetections; ///< Per-sense detections

        // Memory metrics
        uint32_t currentMemories;              ///< Current number of memories
        uint32_t maxMemories;                  ///< Maximum memories reached
        uint64_t memoriesCreated;              ///< Total memories created
        uint64_t memoriesForgotten;            ///< Total memories forgotten
        double averageMemoryAge;               ///< Average memory age (seconds)

        // Processing metrics
        uint64_t rayCastsPerformed;            ///< Total ray casts performed
        uint64_t occlusionTests;               ///< Total occlusion tests
        uint64_t rangeChecks;                  ///< Total range checks
        uint64_t fieldOfViewTests;             ///< Total field of view tests

        // Error metrics
        uint64_t processingErrors;             ///< Processing errors
        uint64_t memoryErrors;                 ///< Memory system errors
        uint64_t senseErrors;                  ///< Sense-specific errors
        uint64_t configurationErrors;          ///< Configuration errors

        // Accuracy metrics
        float detectionAccuracy;               ///< Overall detection accuracy (0.0 - 1.0)
        float falsePositiveRate;               ///< False positive rate (0.0 - 1.0)
        float falseNegativeRate;               ///< False negative rate (0.0 - 1.0)
        float averageConfidence;               ///< Average detection confidence
    };

    /**
     * @class SensorySystem
     * @brief Advanced sensory perception system for AI entities
     *
     * The SensorySystem class provides comprehensive sensory perception capabilities
     * for AI entities, allowing them to perceive and react to their environment
     * through multiple senses including vision, hearing, smell, and others.
     *
     * Key Features:
     * - Multi-sensory perception (vision, hearing, smell, touch, vibration)
     * - Realistic sensory modeling with range, field of view, and sensitivity
     * - Occlusion detection for vision and hearing
     * - Sensory memory with decay and reinforcement
     * - Cross-modal stimulus processing
     * - Sense degradation and fatigue modeling
     * - Environmental effects on sensory perception
     * - Thread-safe operation with performance optimization
     *
     * The system supports both immediate stimulus detection and memory-based
     * perception, allowing AI entities to react to current events and remember
     * past experiences.
     */
    class SensorySystem {
    public:
        /**
         * @brief Constructor
         * @param config Sensory system configuration
         */
        explicit SensorySystem(const SensorySystemConfig& config);

        /**
         * @brief Destructor
         */
        ~SensorySystem();

        /**
         * @brief Deleted copy constructor
         */
        SensorySystem(const SensorySystem&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        SensorySystem& operator=(const SensorySystem&) = delete;

        // System lifecycle

        /**
         * @brief Initialize sensory system
         * @param owner Owning mob
         * @param world Game world
         * @return true if successful, false otherwise
         */
        bool Initialize(Mob* owner, World* world);

        /**
         * @brief Shutdown sensory system
         */
        void Shutdown();

        /**
         * @brief Update sensory system
         * @param deltaTime Time elapsed since last update
         */
        void Update(double deltaTime);

        /**
         * @brief Get current sensory state
         * @return Sensory state
         */
        SensoryState GetState() const { return m_state; }

        // Sense management

        /**
         * @brief Enable sense
         * @param senseType Type of sense to enable
         * @return true if enabled, false otherwise
         */
        bool EnableSense(SenseType senseType);

        /**
         * @brief Disable sense
         * @param senseType Type of sense to disable
         * @return true if disabled, false otherwise
         */
        bool DisableSense(SenseType senseType);

        /**
         * @brief Check if sense is enabled
         * @param senseType Type of sense to check
         * @return true if enabled, false otherwise
         */
        bool IsSenseEnabled(SenseType senseType) const;

        /**
         * @brief Get sense configuration
         * @param senseType Type of sense
         * @return Sense configuration or nullptr if not found
         */
        const SenseConfig* GetSenseConfig(SenseType senseType) const;

        /**
         * @brief Set sense configuration
         * @param senseType Type of sense
         * @param config New configuration
         * @return true if set, false otherwise
         */
        bool SetSenseConfig(SenseType senseType, const SenseConfig& config);

        // Stimulus detection

        /**
         * @brief Detect stimuli
         * @return Vector of detected stimuli
         */
        std::vector<Stimulus> DetectStimuli();

        /**
         * @brief Get current stimuli
         * @return Vector of current stimuli
         */
        const std::vector<Stimulus>& GetCurrentStimuli() const { return m_currentStimuli; }

        /**
         * @brief Get stimuli by type
         * @param type Stimulus type
         * @return Vector of stimuli of specified type
         */
        std::vector<Stimulus> GetStimuliByType(StimulusType type) const;

        /**
         * @brief Get stimuli by sense
         * @param senseType Sense type
         * @return Vector of stimuli detected by specified sense
         */
        std::vector<Stimulus> GetStimuliBySense(SenseType senseType) const;

        /**
         * @brief Check if stimulus is detected
         * @param type Stimulus type
         * @return true if any stimulus of type is detected, false otherwise
         */
        bool IsStimulusDetected(StimulusType type) const;

        // Vision system

        /**
         * @brief Perform vision detection
         * @return Vector of visual stimuli
         */
        std::vector<Stimulus> DetectVision();

        /**
         * @brief Check line of sight to position
         * @param targetPosition Target position
         * @param maxRange Maximum range to check
         * @return true if line of sight exists, false otherwise
         */
        bool HasLineOfSight(const glm::vec3& targetPosition, float maxRange = -1.0f) const;

        /**
         * @brief Check line of sight to entity
         * @param targetEntity Target entity
         * @param maxRange Maximum range to check
         * @return true if line of sight exists, false otherwise
         */
        bool HasLineOfSightToEntity(Entity* targetEntity, float maxRange = -1.0f) const;

        /**
         * @brief Get visible entities
         * @param maxRange Maximum range
         * @return Vector of visible entities
         */
        std::vector<Entity*> GetVisibleEntities(float maxRange = -1.0f) const;

        // Hearing system

        /**
         * @brief Perform hearing detection
         * @return Vector of auditory stimuli
         */
        std::vector<Stimulus> DetectHearing();

        /**
         * @brief Register sound source
         * @param position Sound position
         * @param intensity Sound intensity (0.0 - 1.0)
         * @param source Source entity (optional)
         * @param properties Additional sound properties
         */
        void RegisterSound(const glm::vec3& position, float intensity,
                          Entity* source = nullptr,
                          const std::unordered_map<std::string, std::any>& properties = {});

        /**
         * @brief Check if position is audible
         * @param position Position to check
         * @param intensity Sound intensity
         * @return true if audible, false otherwise
         */
        bool IsAudible(const glm::vec3& position, float intensity) const;

        // Smell system

        /**
         * @brief Perform smell detection
         * @return Vector of olfactory stimuli
         */
        std::vector<Stimulus> DetectSmell();

        /**
         * @brief Register scent source
         * @param position Scent position
         * @param intensity Scent intensity (0.0 - 1.0)
         * @param source Source entity (optional)
         * @param properties Additional scent properties
         */
        void RegisterScent(const glm::vec3& position, float intensity,
                          Entity* source = nullptr,
                          const std::unordered_map<std::string, std::any>& properties = {});

        /**
         * @brief Check if position has scent
         * @param position Position to check
         * @param intensity Minimum scent intensity
         * @return true if detectable scent, false otherwise
         */
        bool HasScent(const glm::vec3& position, float intensity = 0.0f) const;

        // Memory system

        /**
         * @brief Get sensory memories
         * @return Vector of sensory memories
         */
        const std::vector<SensoryMemory>& GetMemories() const { return m_memories; }

        /**
         * @brief Remember stimulus
         * @param stimulus Stimulus to remember
         */
        void RememberStimulus(const Stimulus& stimulus);

        /**
         * @brief Forget memory
         * @param index Memory index
         * @return true if forgotten, false otherwise
         */
        bool ForgetMemory(size_t index);

        /**
         * @brief Clear all memories
         */
        void ClearMemories();

        /**
         * @brief Find memory by stimulus type
         * @param type Stimulus type
         * @return Vector of matching memories
         */
        std::vector<SensoryMemory> FindMemoriesByType(StimulusType type) const;

        /**
         * @brief Find memory by source entity
         * @param source Source entity
         * @return Vector of matching memories
         */
        std::vector<SensoryMemory> FindMemoriesBySource(Entity* source) const;

        // Configuration

        /**
         * @brief Get sensory system configuration
         * @return Current configuration
         */
        const SensorySystemConfig& GetConfig() const { return m_config; }

        /**
         * @brief Set sensory system configuration
         * @param config New configuration
         */
        void SetConfig(const SensorySystemConfig& config);

        // Metrics and monitoring

        /**
         * @brief Get sensory system metrics
         * @return Performance metrics
         */
        const SensorySystemMetrics& GetMetrics() const { return m_metrics; }

        /**
         * @brief Reset metrics
         */
        void ResetMetrics();

        /**
         * @brief Get performance report
         * @return Detailed performance report
         */
        std::string GetPerformanceReport() const;

        // Environmental effects

        /**
         * @brief Apply environmental effects to senses
         * @param environmentFactors Environmental factors
         */
        void ApplyEnvironmentalEffects(const std::unordered_map<std::string, float>& environmentFactors);

        /**
         * @brief Get current environmental factors
         * @return Current environmental factors
         */
        std::unordered_map<std::string, float> GetEnvironmentalFactors() const;

        // Debug and visualization

        /**
         * @brief Enable debug drawing
         * @param enabled Enable state
         * @return Previous state
         */
        bool SetDebugDrawingEnabled(bool enabled);

        /**
         * @brief Check if debug drawing is enabled
         * @return true if enabled, false otherwise
         */
        bool IsDebugDrawingEnabled() const { return m_config.enableDebugDrawing; }

        /**
         * @brief Get debug render data
         * @return Debug render data
         */
        const std::vector<DebugRenderData>& GetDebugRenderData() const;

        /**
         * @brief Clear debug render data
         */
        void ClearDebugRenderData();

        // Utility functions

        /**
         * @brief Get system owner
         * @return Owner mob
         */
        Mob* GetOwner() const { return m_owner; }

        /**
         * @brief Set system owner
         * @param owner New owner
         */
        void SetOwner(Mob* owner) { m_owner = owner; }

        /**
         * @brief Get game world
         * @return Game world
         */
        World* GetWorld() const { return m_world; }

        /**
         * @brief Set game world
         * @param world New world
         */
        void SetWorld(World* world) { m_world = world; }

        /**
         * @brief Validate sensory system
         * @return true if valid, false otherwise
         */
        bool Validate() const;

        /**
         * @brief Get status report
         * @return Detailed status report
         */
        std::string GetStatusReport() const;

        /**
         * @brief Optimize sensory system
         * @return Optimization results
         */
        std::unordered_map<std::string, size_t> Optimize();

    private:
        /**
         * @brief Initialize senses
         * @return true if successful, false otherwise
         */
        bool InitializeSenses();

        /**
         * @brief Update senses
         * @param deltaTime Time elapsed
         */
        void UpdateSenses(double deltaTime);

        /**
         * @brief Update sensory memory
         * @param deltaTime Time elapsed
         */
        void UpdateMemory(double deltaTime);

        /**
         * @brief Process detected stimuli
         * @param stimuli Vector of stimuli to process
         */
        void ProcessStimuli(std::vector<Stimulus>& stimuli);

        /**
         * @brief Filter stimuli by sense capabilities
         * @param stimuli Vector of stimuli to filter
         * @return Filtered stimuli
         */
        std::vector<Stimulus> FilterStimuli(const std::vector<Stimulus>& stimuli);

        /**
         * @brief Perform ray cast for vision
         * @param origin Ray origin
         * @param direction Ray direction
         * @param maxRange Maximum range
         * @return Raycast hit or empty optional
         */
        std::optional<RaycastHit> PerformVisionRaycast(const glm::vec3& origin,
                                                      const glm::vec3& direction,
                                                      float maxRange) const;

        /**
         * @brief Calculate stimulus confidence
         * @param stimulus Stimulus to evaluate
         * @return Confidence value (0.0 - 1.0)
         */
        float CalculateStimulusConfidence(const Stimulus& stimulus) const;

        /**
         * @brief Check if position is within field of view
         * @param position Position to check
         * @param fov Field of view (degrees)
         * @return true if within FOV, false otherwise
         */
        bool IsWithinFieldOfView(const glm::vec3& position, float fov) const;

        /**
         * @brief Update sense degradation
         * @param deltaTime Time elapsed
         */
        void UpdateSenseDegradation(double deltaTime);

        /**
         * @brief Collect debug render data
         */
        void CollectDebugRenderData();

        /**
         * @brief Update performance metrics
         * @param deltaTime Time elapsed
         * @param stimuliProcessed Number of stimuli processed
         */
        void UpdateMetrics(double deltaTime, size_t stimuliProcessed);

        /**
         * @brief Handle sensory system errors
         * @param error Error message
         */
        void HandleError(const std::string& error);

        // Sensory system data
        SensorySystemConfig m_config;                  ///< System configuration
        SensorySystemMetrics m_metrics;                ///< Performance metrics
        SensoryState m_state;                          ///< Current system state

        // System components
        Mob* m_owner;                                  ///< System owner
        World* m_world;                                ///< Game world
        Blackboard* m_blackboard;                      ///< Communication blackboard

        // Sense configurations
        std::unordered_map<SenseType, SenseConfig> m_senseConfigs; ///< Sense configurations
        mutable std::shared_mutex m_senseMutex;        ///< Sense synchronization

        // Current stimuli
        std::vector<Stimulus> m_currentStimuli;        ///< Currently detected stimuli
        mutable std::shared_mutex m_stimuliMutex;      ///< Stimuli synchronization

        // Sensory memory
        std::vector<SensoryMemory> m_memories;         ///< Sensory memories
        mutable std::shared_mutex m_memoryMutex;       ///< Memory synchronization

        // Sound and scent sources
        std::vector<std::tuple<glm::vec3, float, Entity*, double, std::unordered_map<std::string, std::any>>> m_soundSources;
        std::vector<std::tuple<glm::vec3, float, Entity*, double, std::unordered_map<std::string, std::any>>> m_scentSources;
        mutable std::shared_mutex m_sourcesMutex;      ///< Sources synchronization

        // Debug data
        std::vector<DebugRenderData> m_debugRenderData;
        mutable std::shared_mutex m_debugMutex;        ///< Debug synchronization

        // State flags
        bool m_isInitialized;                          ///< System is initialized
        double m_lastUpdateTime;                       ///< Last update timestamp
        double m_lastMemoryUpdate;                     ///< Last memory update timestamp

        // Environmental factors
        std::unordered_map<std::string, float> m_environmentalFactors;
        mutable std::shared_mutex m_environmentMutex;  ///< Environment synchronization
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_AI_SENSORY_SYSTEM_HPP
