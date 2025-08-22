/**
 * @file SoundManager.hpp
 * @brief VoxelCraft Sound Management System
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines the SoundManager class that provides comprehensive sound
 * management for the VoxelCraft game engine, including sound playback, resource
 * management, audio events, sound pooling, and integration with the 3D audio
 * system for immersive sound experiences.
 */

#ifndef VOXELCRAFT_AUDIO_SOUND_MANAGER_HPP
#define VOXELCRAFT_AUDIO_SOUND_MANAGER_HPP

#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <chrono>
#include <optional>
#include <queue>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../core/Config.hpp"

namespace VoxelCraft {

    // Forward declarations
    class AudioEngine;
    class AudioSource;
    class AudioEmitter;
    class AudioResource;

    /**
     * @enum SoundCategory
     * @brief Sound categories for volume control
     */
    enum class SoundCategory {
        Master,                 ///< Master volume
        Music,                  ///< Music volume
        SoundEffects,           ///< Sound effects volume
        Voice,                  ///< Voice/Dialogue volume
        Ambient,                ///< Ambient sounds volume
        UI,                     ///< UI sounds volume
        Player,                 ///< Player sounds volume
        Environment,            ///< Environmental sounds volume
        Weather,                ///< Weather sounds volume
        Creatures,              ///< Creature sounds volume
        Blocks,                 ///< Block interaction sounds volume
        Items,                  ///< Item sounds volume
        Custom                  ///< Custom category
    };

    /**
     * @enum SoundPriority
     * @brief Sound playback priorities
     */
    enum class SoundPriority {
        VeryLow,                ///< Very low priority (can be culled)
        Low,                    ///< Low priority
        Normal,                 ///< Normal priority
        High,                   ///< High priority
        VeryHigh,               ///< Very high priority (always plays)
        Critical                ///< Critical priority (system sounds)
    };

    /**
     * @enum SoundPlayMode
     * @brief Sound playback modes
     */
    enum class SoundPlayMode {
        OneShot,                ///< Play once and destroy
        Loop,                   ///< Loop continuously
        PingPong,               ///< Ping-pong looping
        Scheduled,              ///< Scheduled playback
        Procedural              ///< Procedural sound generation
    };

    /**
     * @enum SoundAttenuation
     * @brief Sound attenuation models
     */
    enum class SoundAttenuation {
        None,                   ///< No attenuation
        Linear,                 ///< Linear attenuation
        Inverse,                ///< Inverse distance
        InverseSquare,          ///< Inverse square distance
        Exponential,            ///< Exponential attenuation
        Custom                  ///< Custom attenuation curve
    };

    /**
     * @struct SoundConfig
     * @brief Sound configuration settings
     */
    struct SoundConfig {
        // Basic settings
        std::string soundName;                  ///< Sound name/identifier
        SoundCategory category;                 ///< Sound category
        SoundPriority priority;                 ///< Sound priority
        SoundPlayMode playMode;                 ///< Playback mode

        // Audio properties
        float volume;                           ///< Base volume (0.0 - 1.0)
        float pitch;                            ///< Base pitch (0.5 - 2.0)
        bool loop;                              ///< Loop playback
        float fadeInTime;                       ///< Fade in time (seconds)
        float fadeOutTime;                      ///< Fade out time (seconds)

        // 3D audio properties
        bool enable3D;                          ///< Enable 3D audio
        glm::vec3 position;                     ///< 3D position
        float minDistance;                      ///< Minimum attenuation distance
        float maxDistance;                      ///< Maximum attenuation distance
        SoundAttenuation attenuation;            ///< Attenuation model
        float rolloffFactor;                    ///< Distance rolloff factor

        // Directional properties
        bool enableDirectivity;                 ///< Enable directional sound
        float coneInnerAngle;                   ///< Inner cone angle (degrees)
        float coneOuterAngle;                   ///< Outer cone angle (degrees)
        float coneOuterGain;                    ///< Outer cone gain

        // Playback properties
        float startTime;                        ///< Start time offset (seconds)
        float endTime;                          ///< End time (0 = full length)
        bool enableRandomization;               ///< Enable randomization
        float volumeVariance;                   ///< Volume randomization (0.0 - 1.0)
        float pitchVariance;                    ///< Pitch randomization (0.0 - 1.0)

        // Advanced properties
        bool enableReverb;                      ///< Enable reverb processing
        bool enableOcclusion;                   ///< Enable occlusion
        bool enableObstruction;                 ///< Enable obstruction
        bool enableCompression;                 ///< Enable dynamic compression
        bool enableEQ;                          ///< Enable equalization

        SoundConfig()
            : category(SoundCategory::SoundEffects)
            , priority(SoundPriority::Normal)
            , playMode(SoundPlayMode::OneShot)
            , volume(1.0f)
            , pitch(1.0f)
            , loop(false)
            , fadeInTime(0.0f)
            , fadeOutTime(0.0f)
            , enable3D(false)
            , position(0.0f, 0.0f, 0.0f)
            , minDistance(1.0f)
            , maxDistance(100.0f)
            , attenuation(SoundAttenuation::InverseSquare)
            , rolloffFactor(1.0f)
            , enableDirectivity(false)
            , coneInnerAngle(360.0f)
            , coneOuterAngle(360.0f)
            , coneOuterGain(0.0f)
            , startTime(0.0f)
            , endTime(0.0f)
            , enableRandomization(false)
            , volumeVariance(0.0f)
            , pitchVariance(0.0f)
            , enableReverb(true)
            , enableOcclusion(true)
            , enableObstruction(true)
            , enableCompression(false)
            , enableEQ(false)
        {}
    };

    /**
     * @struct SoundInstance
     * @brief Active sound instance
     */
    struct SoundInstance {
        uint32_t instanceId;                    ///< Unique instance ID
        uint32_t soundId;                       ///< Sound resource ID
        uint32_t sourceId;                      ///< Audio source ID
        std::string soundName;                  ///< Sound name
        SoundConfig config;                     ///< Sound configuration

        // Playback state
        bool playing;                           ///< Currently playing
        bool paused;                            ///< Currently paused
        bool stopping;                          ///< In the process of stopping
        float currentTime;                      ///< Current playback time
        float totalTime;                        ///< Total sound duration

        // Fade state
        bool fadingIn;                          ///< Currently fading in
        bool fadingOut;                         ///< Currently fading out
        float fadeTime;                         ///< Current fade time
        float fadeDuration;                     ///< Total fade duration

        // 3D state
        glm::vec3 position;                     ///< Current 3D position
        glm::vec3 velocity;                     ///< Current velocity
        float occlusionFactor;                  ///< Current occlusion factor
        float obstructionFactor;                ///< Current obstruction factor

        // Instance data
        float startTime;                        ///< When the instance started
        std::unordered_map<std::string, std::any> userData; ///< User-defined data

        SoundInstance()
            : instanceId(0)
            , soundId(0)
            , sourceId(0)
            , playing(false)
            , paused(false)
            , stopping(false)
            , currentTime(0.0f)
            , totalTime(0.0f)
            , fadingIn(false)
            , fadingOut(false)
            , fadeTime(0.0f)
            , fadeDuration(0.0f)
            , position(0.0f, 0.0f, 0.0f)
            , velocity(0.0f, 0.0f, 0.0f)
            , occlusionFactor(0.0f)
            , obstructionFactor(0.0f)
            , startTime(0.0f)
        {}
    };

    /**
     * @struct SoundPool
     * @brief Sound pooling system for performance
     */
    struct SoundPool {
        uint32_t poolId;                        ///< Pool ID
        std::string poolName;                   ///< Pool name
        std::vector<uint32_t> availableSources; ///< Available audio sources
        std::vector<uint32_t> usedSources;      ///< Currently used sources
        uint32_t maxSources;                    ///< Maximum sources in pool
        SoundCategory category;                 ///< Sound category for this pool
        SoundPriority minPriority;              ///< Minimum priority for culling

        SoundPool()
            : poolId(0)
            , maxSources(16)
            , category(SoundCategory::SoundEffects)
            , minPriority(SoundPriority::Low)
        {}
    };

    /**
     * @struct SoundEvent
     * @brief Sound event for game integration
     */
    struct SoundEvent {
        std::string eventName;                  ///< Event name
        std::string soundName;                  ///< Associated sound
        SoundConfig config;                     ///< Sound configuration
        std::vector<std::string> parameters;    ///< Event parameters
        std::function<void(uint32_t)> callback; ///< Event callback

        SoundEvent() = default;
    };

    /**
     * @struct SoundStats
     * @brief Sound manager performance statistics
     */
    struct SoundStats {
        // Instance statistics
        uint32_t totalInstances;                ///< Total sound instances created
        uint32_t activeInstances;               ///< Currently active instances
        uint32_t maxActiveInstances;            ///< Maximum active instances
        uint32_t instancesPlayed;               ///< Total instances played
        uint32_t instancesStopped;              ///< Total instances stopped

        // Pool statistics
        uint32_t totalPools;                    ///< Total sound pools
        uint32_t activePools;                   ///< Active pools
        uint32_t poolRequests;                  ///< Pool allocation requests
        uint32_t poolMisses;                    ///< Pool allocation misses

        // Performance metrics
        double totalPlayTime;                   ///< Total playback time (seconds)
        double averagePlayTime;                 ///< Average playback time
        uint32_t sourceSwitches;                ///< Audio source switches
        uint32_t priorityCulls;                 ///< Sounds culled by priority

        // Memory usage
        size_t memoryUsage;                     ///< Total memory usage
        uint32_t loadedSounds;                  ///< Number of loaded sounds
        uint32_t cachedSounds;                  ///< Number of cached sounds

        // Category statistics
        std::unordered_map<SoundCategory, uint32_t> categoryInstances; ///< Instances per category
        std::unordered_map<SoundCategory, double> categoryPlayTime; ///< Play time per category
    };

    /**
     * @class SoundManager
     * @brief Advanced sound management system
     *
     * The SoundManager class provides comprehensive sound management for the
     * VoxelCraft game engine, featuring sound pooling, priority systems, 3D
     * audio integration, sound events, resource management, and performance
     * optimization for efficient audio playback in complex game environments.
     *
     * Key Features:
     * - Sound pooling system for performance optimization
     * - Priority-based sound culling and management
     * - 3D audio positioning and spatialization
     * - Sound event system for game integration
     * - Resource management with streaming support
     * - Volume and category-based mixing
     * - Sound randomization and variation
     * - Occlusion and obstruction handling
     * - Fade in/out and crossfading
     * - Performance monitoring and optimization
     * - Memory management and caching
     * - Integration with audio engine systems
     *
     * The sound manager is designed to handle hundreds of concurrent sounds
     * while maintaining high performance and audio quality through intelligent
     * resource management and prioritization systems.
     */
    class SoundManager {
    public:
        /**
         * @brief Constructor
         * @param audioEngine Audio engine instance
         */
        explicit SoundManager(AudioEngine* audioEngine);

        /**
         * @brief Destructor
         */
        ~SoundManager();

        /**
         * @brief Deleted copy constructor
         */
        SoundManager(const SoundManager&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        SoundManager& operator=(const SoundManager&) = delete;

        // Sound manager lifecycle

        /**
         * @brief Initialize sound manager
         * @return true if successful, false otherwise
         */
        bool Initialize();

        /**
         * @brief Shutdown sound manager
         */
        void Shutdown();

        /**
         * @brief Update sound manager
         * @param deltaTime Time elapsed since last update
         */
        void Update(double deltaTime);

        /**
         * @brief Get sound statistics
         * @return Current sound statistics
         */
        const SoundStats& GetStats() const { return m_stats; }

        // Sound playback

        /**
         * @brief Play sound by name
         * @param soundName Sound name
         * @param config Sound configuration
         * @return Instance ID or 0 if failed
         */
        uint32_t PlaySound(const std::string& soundName, const SoundConfig& config = SoundConfig());

        /**
         * @brief Play sound at position
         * @param soundName Sound name
         * @param position 3D position
         * @param config Sound configuration
         * @return Instance ID or 0 if failed
         */
        uint32_t PlaySoundAt(const std::string& soundName, const glm::vec3& position,
                           const SoundConfig& config = SoundConfig());

        /**
         * @brief Play sound attached to entity
         * @param soundName Sound name
         * @param entityId Entity ID
         * @param config Sound configuration
         * @return Instance ID or 0 if failed
         */
        uint32_t PlaySoundOnEntity(const std::string& soundName, uint32_t entityId,
                                 const SoundConfig& config = SoundConfig());

        /**
         * @brief Stop sound instance
         * @param instanceId Instance ID
         * @param fadeOutTime Fade out time (seconds)
         * @return true if successful, false otherwise
         */
        bool StopSound(uint32_t instanceId, float fadeOutTime = 0.0f);

        /**
         * @brief Pause sound instance
         * @param instanceId Instance ID
         * @return true if successful, false otherwise
         */
        bool PauseSound(uint32_t instanceId);

        /**
         * @brief Resume sound instance
         * @param instanceId Instance ID
         * @return true if successful, false otherwise
         */
        bool ResumeSound(uint32_t instanceId);

        /**
         * @brief Check if sound is playing
         * @param instanceId Instance ID
         * @return true if playing, false otherwise
         */
        bool IsSoundPlaying(uint32_t instanceId) const;

        // Sound instance management

        /**
         * @brief Get sound instance
         * @param instanceId Instance ID
         * @return Sound instance or nullopt if not found
         */
        std::optional<SoundInstance> GetSoundInstance(uint32_t instanceId) const;

        /**
         * @brief Update sound instance
         * @param instanceId Instance ID
         * @param config New configuration
         * @return true if successful, false otherwise
         */
        bool UpdateSoundInstance(uint32_t instanceId, const SoundConfig& config);

        /**
         * @brief Set sound instance position
         * @param instanceId Instance ID
         * @param position New position
         * @return true if successful, false otherwise
         */
        bool SetSoundPosition(uint32_t instanceId, const glm::vec3& position);

        /**
         * @brief Set sound instance volume
         * @param instanceId Instance ID
         * @param volume New volume (0.0 - 1.0)
         * @return true if successful, false otherwise
         */
        bool SetSoundVolume(uint32_t instanceId, float volume);

        /**
         * @brief Set sound instance pitch
         * @param instanceId Instance ID
         * @param pitch New pitch (0.5 - 2.0)
         * @return true if successful, false otherwise
         */
        bool SetSoundPitch(uint32_t instanceId, float pitch);

        // Sound resource management

        /**
         * @brief Load sound resource
         * @param name Sound name
         * @param filePath File path
         * @param streaming Enable streaming for large files
         * @return Sound ID or 0 if failed
         */
        uint32_t LoadSound(const std::string& name, const std::string& filePath,
                         bool streaming = false);

        /**
         * @brief Unload sound resource
         * @param soundId Sound ID
         * @return true if successful, false otherwise
         */
        bool UnloadSound(uint32_t soundId);

        /**
         * @brief Preload sound for instant playback
         * @param soundId Sound ID
         * @return true if successful, false otherwise
         */
        bool PreloadSound(uint32_t soundId);

        /**
         * @brief Get sound resource info
         * @param soundId Sound ID
         * @return Sound information string
         */
        std::string GetSoundInfo(uint32_t soundId) const;

        // Sound pool management

        /**
         * @brief Create sound pool
         * @param poolName Pool name
         * @param maxSources Maximum audio sources
         * @param category Sound category
         * @return Pool ID or 0 if failed
         */
        uint32_t CreateSoundPool(const std::string& poolName, uint32_t maxSources,
                               SoundCategory category = SoundCategory::SoundEffects);

        /**
         * @brief Get sound pool
         * @param poolId Pool ID
         * @return Sound pool or nullopt if not found
         */
        std::optional<SoundPool> GetSoundPool(uint32_t poolId) const;

        /**
         * @brief Destroy sound pool
         * @param poolId Pool ID
         * @return true if successful, false otherwise
         */
        bool DestroySoundPool(uint32_t poolId);

        // Sound events

        /**
         * @brief Register sound event
         * @param eventName Event name
         * @param soundEvent Event configuration
         * @return true if successful, false otherwise
         */
        bool RegisterSoundEvent(const std::string& eventName, const SoundEvent& soundEvent);

        /**
         * @brief Unregister sound event
         * @param eventName Event name
         * @return true if successful, false otherwise
         */
        bool UnregisterSoundEvent(const std::string& eventName);

        /**
         * @brief Trigger sound event
         * @param eventName Event name
         * @param parameters Event parameters
         * @return Instance ID or 0 if failed
         */
        uint32_t TriggerSoundEvent(const std::string& eventName,
                                 const std::unordered_map<std::string, std::any>& parameters = {});

        // Volume and mixing

        /**
         * @brief Set category volume
         * @param category Sound category
         * @param volume Volume level (0.0 - 1.0)
         * @return true if successful, false otherwise
         */
        bool SetCategoryVolume(SoundCategory category, float volume);

        /**
         * @brief Get category volume
         * @param category Sound category
         * @return Current volume level
         */
        float GetCategoryVolume(SoundCategory category) const;

        /**
         * @brief Fade category volume
         * @param category Sound category
         * @param targetVolume Target volume
         * @param duration Fade duration (seconds)
         * @return true if successful, false otherwise
         */
        bool FadeCategoryVolume(SoundCategory category, float targetVolume, float duration);

        // Sound effects

        /**
         * @brief Apply reverb to sound
         * @param instanceId Instance ID
         * @param reverbType Reverb type
         * @param wetLevel Wet level (0.0 - 1.0)
         * @return true if successful, false otherwise
         */
        bool ApplyReverb(uint32_t instanceId, AudioReverbType reverbType, float wetLevel = 0.3f);

        /**
         * @brief Apply occlusion to sound
         * @param instanceId Instance ID
         * @param occlusion Occlusion factor (0.0 - 1.0)
         * @return true if successful, false otherwise
         */
        bool ApplyOcclusion(uint32_t instanceId, float occlusion);

        /**
         * @brief Apply obstruction to sound
         * @param instanceId Instance ID
         * @param obstruction Obstruction factor (0.0 - 1.0)
         * @return true if successful, false otherwise
         */
        bool ApplyObstruction(uint32_t instanceId, float obstruction);

        // Global sound management

        /**
         * @brief Stop all sounds
         * @param fadeOutTime Fade out time (seconds)
         * @return Number of sounds stopped
         */
        uint32_t StopAllSounds(float fadeOutTime = 0.0f);

        /**
         * @brief Pause all sounds
         * @param category Optional category filter
         * @return Number of sounds paused
         */
        uint32_t PauseAllSounds(SoundCategory category = SoundCategory::Master);

        /**
         * @brief Resume all sounds
         * @param category Optional category filter
         * @return Number of sounds resumed
         */
        uint32_t ResumeAllSounds(SoundCategory category = SoundCategory::Master);

        /**
         * @brief Get active sound count
         * @param category Optional category filter
         * @return Number of active sounds
         */
        uint32_t GetActiveSoundCount(SoundCategory category = SoundCategory::Master) const;

        // Performance optimization

        /**
         * @brief Set sound priority threshold
         * @param threshold Minimum priority to play
         * @return true if successful, false otherwise
         */
        bool SetPriorityThreshold(SoundPriority threshold);

        /**
         * @brief Enable sound culling
         * @param enabled Enable state
         * @return true if successful, false otherwise
         */
        bool EnableCulling(bool enabled);

        /**
         * @brief Set maximum sound distance
         * @param distance Maximum distance for sound playback
         * @return true if successful, false otherwise
         */
        bool SetMaxDistance(float distance);

        // Utility functions

        /**
         * @brief Get sound names
         * @return Vector of loaded sound names
         */
        std::vector<std::string> GetSoundNames() const;

        /**
         * @brief Check if sound exists
         * @param soundName Sound name
         * @return true if exists, false otherwise
         */
        bool SoundExists(const std::string& soundName) const;

        /**
         * @brief Validate sound manager state
         * @return true if valid, false otherwise
         */
        bool Validate() const;

        /**
         * @brief Get status report
         * @return Detailed status report
         */
        std::string GetStatusReport() const;

        /**
         * @brief Get performance report
         * @return Detailed performance report
         */
        std::string GetPerformanceReport() const;

        /**
         * @brief Optimize sound manager
         * @return Optimization results
         */
        std::unordered_map<std::string, size_t> Optimize();

    private:
        /**
         * @brief Initialize sound pools
         * @return true if successful, false otherwise
         */
        bool InitializeSoundPools();

        /**
         * @brief Update sound instances
         * @param deltaTime Time elapsed
         */
        void UpdateSoundInstances(double deltaTime);

        /**
         * @brief Update sound fading
         * @param deltaTime Time elapsed
         */
        void UpdateSoundFading(double deltaTime);

        /**
         * @brief Update 3D sound positioning
         */
        void Update3DSounds();

        /**
         * @brief Clean up finished sounds
         */
        void CleanupFinishedSounds();

        /**
         * @brief Allocate audio source from pool
         * @param poolId Pool ID
         * @param priority Sound priority
         * @return Source ID or 0 if failed
         */
        uint32_t AllocateSourceFromPool(uint32_t poolId, SoundPriority priority);

        /**
         * @brief Return audio source to pool
         * @param poolId Pool ID
         * @param sourceId Source ID
         */
        void ReturnSourceToPool(uint32_t poolId, uint32_t sourceId);

        /**
         * @brief Create sound instance
         * @param soundId Sound ID
         * @param config Sound configuration
         * @return Instance ID or 0 if failed
         */
        uint32_t CreateSoundInstance(uint32_t soundId, const SoundConfig& config);

        /**
         * @brief Destroy sound instance
         * @param instanceId Instance ID
         */
        void DestroySoundInstance(uint32_t instanceId);

        /**
         * @brief Update performance statistics
         * @param deltaTime Time elapsed
         */
        void UpdateStats(double deltaTime);

        /**
         * @brief Handle sound manager errors
         * @param error Error message
         */
        void HandleError(const std::string& error);

        // Sound manager data
        AudioEngine* m_audioEngine;                    ///< Audio engine instance
        SoundStats m_stats;                            ///< Performance statistics

        // Sound instances
        std::unordered_map<uint32_t, SoundInstance> m_soundInstances; ///< Active instances
        std::unordered_map<uint32_t, SoundPool> m_soundPools; ///< Sound pools
        mutable std::shared_mutex m_instancesMutex;    ///< Instances synchronization

        // Sound resources
        std::unordered_map<std::string, uint32_t> m_soundResources; ///< Sound name to ID mapping
        std::unordered_map<uint32_t, std::shared_ptr<AudioResource>> m_resources; ///< Loaded resources
        mutable std::shared_mutex m_resourcesMutex;    ///< Resources synchronization

        // Sound events
        std::unordered_map<std::string, SoundEvent> m_soundEvents; ///< Registered events
        mutable std::shared_mutex m_eventsMutex;       ///< Events synchronization

        // Volume management
        std::unordered_map<SoundCategory, float> m_categoryVolumes; ///< Category volumes
        std::unordered_map<SoundCategory, float> m_targetVolumes; ///< Target volumes for fading
        std::unordered_map<SoundCategory, float> m_fadeTimes; ///< Current fade times
        mutable std::shared_mutex m_volumeMutex;       ///< Volume synchronization

        // Configuration
        SoundPriority m_priorityThreshold;             ///< Current priority threshold
        float m_maxDistance;                           ///< Maximum sound distance
        bool m_cullingEnabled;                         ///< Culling enabled

        // State tracking
        bool m_isInitialized;                          ///< Manager is initialized
        double m_lastUpdateTime;                       ///< Last update time
        std::string m_lastError;                       ///< Last error message

        static std::atomic<uint32_t> s_nextInstanceId; ///< Next instance ID counter
        static std::atomic<uint32_t> s_nextPoolId;     ///< Next pool ID counter
        static std::atomic<uint32_t> s_nextResourceId; ///< Next resource ID counter
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_AUDIO_SOUND_MANAGER_HPP
