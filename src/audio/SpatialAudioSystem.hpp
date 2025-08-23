/**
 * @file SpatialAudioSystem.hpp
 * @brief VoxelCraft 3D Spatial Audio System
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_AUDIO_SPATIAL_AUDIO_SYSTEM_HPP
#define VOXELCRAFT_AUDIO_SPATIAL_AUDIO_SYSTEM_HPP

#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
#include <mutex>
#include <atomic>

#include "../math/Vec3.hpp"

namespace VoxelCraft {

    // Forward declarations
    class AudioSource;
    class AudioListener;
    class SoundGenerator;

    /**
     * @struct SpatialAudioConfig
     * @brief Configuration for spatial audio system
     */
    struct SpatialAudioConfig {
        bool enable3DAudio = true;                    ///< Enable 3D audio processing
        bool enableDoppler = true;                    ///< Enable Doppler effect
        bool enableDistanceAttenuation = true;        ///< Enable distance-based volume attenuation
        bool enableAirAbsorption = true;              ///< Enable air absorption effect
        bool enableOcclusion = true;                  ///< Enable occlusion detection
        bool enableReverb = true;                     ///< Enable environmental reverb
        float speedOfSound = 343.0f;                  ///< Speed of sound in m/s
        float referenceDistance = 1.0f;               ///< Reference distance for attenuation
        float maxDistance = 64.0f;                    ///< Maximum audible distance
        float rolloffFactor = 1.0f;                   ///< Distance rolloff factor
        float dopplerFactor = 1.0f;                   ///< Doppler effect intensity
        float airAbsorptionFactor = 0.001f;           ///< Air absorption per meter
        int maxOcclusionRays = 8;                     ///< Maximum rays for occlusion testing
        float occlusionStrength = 0.7f;               ///< Occlusion effect strength
        float reverbStrength = 0.5f;                  ///< Reverb effect strength
    };

    /**
     * @struct AudioOcclusion
     * @brief Audio occlusion information
     */
    struct AudioOcclusion {
        float occlusionFactor = 0.0f;                 ///< Occlusion amount (0.0 - 1.0)
        float transmissionFactor = 1.0f;              ///< Sound transmission through obstacles
        Vec3 closestObstacle;                         ///< Position of closest obstacle
        bool directPath = true;                       ///< Direct path available
    };

    /**
     * @struct SpatialAudioSource
     * @brief Spatial information for audio sources
     */
    struct SpatialAudioSource {
        Vec3 position;                                ///< 3D position in world space
        Vec3 velocity;                                ///< Velocity vector for Doppler effect
        float radius = 0.0f;                         ///< Spherical radius of sound source
        AudioOcclusion occlusion;                     ///< Occlusion information
        bool isOmniDirectional = true;                ///< Omni-directional vs directional
        Vec3 direction = Vec3(0.0f, 0.0f, 1.0f);      ///< Direction if not omni-directional
        float coneInnerAngle = 360.0f;                ///< Inner cone angle in degrees
        float coneOuterAngle = 360.0f;                ///< Outer cone angle in degrees
        float coneOuterGain = 0.0f;                   ///< Gain outside outer cone
    };

    /**
     * @struct SpatialAudioListener
     * @brief Spatial information for audio listener
     */
    struct SpatialAudioListener {
        Vec3 position;                                ///< 3D position in world space
        Vec3 velocity;                                ///< Velocity vector for Doppler effect
        Vec3 forward;                                 ///< Forward direction vector
        Vec3 up;                                      ///< Up direction vector
        float headRadius = 0.1f;                      ///< Head radius for HRTF (if enabled)
    };

    /**
     * @struct SpatialAudioEnvironment
     * @brief Environmental audio properties
     */
    struct SpatialAudioEnvironment {
        std::string environmentType = "default";      ///< Environment type (cave, forest, etc.)
        float roomSize = 10.0f;                       ///< Room size for reverb
        float reverbTime = 1.0f;                      ///< Reverb decay time
        float reverbDamping = 0.5f;                   ///< Reverb damping factor
        float reverbLevel = 0.3f;                     ///< Reverb level
        float airDensity = 1.0f;                      ///< Air density for absorption
        float humidity = 0.5f;                        ///< Humidity for absorption
        float temperature = 20.0f;                    ///< Temperature for sound speed
    };

    /**
     * @class SpatialAudioSystem
     * @brief Advanced 3D spatial audio processing system
     *
     * Features:
     * - 3D positional audio with accurate distance attenuation
     * - Doppler effect calculation
     * - Air absorption modeling
     * - Dynamic occlusion detection using ray casting
     * - Environmental reverb zones
     * - HRTF support (when available)
     * - Real-time audio processing pipeline
     */
    class SpatialAudioSystem {
    public:
        SpatialAudioSystem();
        ~SpatialAudioSystem();

        /**
         * @brief Initialize the spatial audio system
         * @param config Spatial audio configuration
         * @return true if successful
         */
        bool Initialize(const SpatialAudioConfig& config);

        /**
         * @brief Shutdown the spatial audio system
         */
        void Shutdown();

        /**
         * @brief Update spatial audio processing
         * @param deltaTime Time elapsed since last update
         */
        void Update(float deltaTime);

        /**
         * @brief Set spatial audio configuration
         * @param config New configuration
         */
        void SetConfig(const SpatialAudioConfig& config) { m_config = config; }

        /**
         * @brief Get current configuration
         * @return Current configuration
         */
        const SpatialAudioConfig& GetConfig() const { return m_config; }

        /**
         * @brief Set listener spatial information
         * @param listener Spatial listener data
         */
        void SetListener(const SpatialAudioListener& listener);

        /**
         * @brief Get current listener information
         * @return Current listener data
         */
        const SpatialAudioListener& GetListener() const { return m_listener; }

        /**
         * @brief Set environment properties
         * @param environment Environment audio properties
         */
        void SetEnvironment(const SpatialAudioEnvironment& environment);

        /**
         * @brief Get current environment
         * @return Current environment properties
         */
        const SpatialAudioEnvironment& GetEnvironment() const { return m_environment; }

        /**
         * @brief Process spatial audio for a source
         * @param sourceId Unique source identifier
         * @param spatialSource Spatial source information
         * @return Processed audio parameters
         */
        SpatialAudioSource ProcessSpatialAudio(const std::string& sourceId,
                                             const SpatialAudioSource& spatialSource);

        /**
         * @brief Calculate occlusion for a sound source
         * @param sourcePosition Source position
         * @param listenerPosition Listener position
         * @return Occlusion information
         */
        AudioOcclusion CalculateOcclusion(const Vec3& sourcePosition, const Vec3& listenerPosition);

        /**
         * @brief Calculate distance attenuation
         * @param distance Distance between source and listener
         * @param referenceDistance Reference distance
         * @param maxDistance Maximum distance
         * @param rolloffFactor Rolloff factor
         * @return Attenuation factor (0.0 - 1.0)
         */
        float CalculateDistanceAttenuation(float distance, float referenceDistance,
                                         float maxDistance, float rolloffFactor);

        /**
         * @brief Calculate Doppler effect
         * @param sourceVelocity Source velocity
         * @param listenerVelocity Listener velocity
         * @param sourcePosition Source position
         * @param listenerPosition Listener position
         * @return Doppler pitch shift factor
         */
        float CalculateDopplerEffect(const Vec3& sourceVelocity, const Vec3& listenerVelocity,
                                   const Vec3& sourcePosition, const Vec3& listenerPosition);

        /**
         * @brief Calculate air absorption
         * @param distance Distance through air
         * @param frequency Sound frequency
         * @return Absorption factor (0.0 - 1.0)
         */
        float CalculateAirAbsorption(float distance, float frequency);

        /**
         * @brief Calculate reverb parameters for current environment
         * @param sourcePosition Source position
         * @return Reverb parameters
         */
        std::unordered_map<std::string, float> CalculateReverb(const Vec3& sourcePosition);

        /**
         * @brief Check if HRTF is supported and enabled
         * @return true if HRTF is available
         */
        bool IsHRTFSupported() const { return m_hrtfSupported; }

        /**
         * @brief Enable/disable HRTF processing
         * @param enable Enable HRTF
         */
        void SetHRTFEnabled(bool enable) { m_hrtfEnabled = enable; }

        /**
         * @brief Get HRTF processing status
         * @return true if HRTF is enabled
         */
        bool IsHRTFEnabled() const { return m_hrtfEnabled && m_hrtfSupported; }

        /**
         * @brief Get debug information
         * @return Debug information string
         */
        std::string GetDebugInfo() const;

        /**
         * @brief Reset spatial audio system
         */
        void Reset();

    private:
        /**
         * @brief Initialize HRTF if available
         * @return true if HRTF initialized successfully
         */
        bool InitializeHRTF();

        /**
         * @brief Update occlusion for all active sources
         */
        void UpdateOcclusion();

        /**
         * @brief Update environmental effects
         */
        void UpdateEnvironment();

        /**
         * @brief Process HRTF for binaural audio
         * @param sourceId Source identifier
         * @param azimuth Azimuth angle in degrees
         * @param elevation Elevation angle in degrees
         * @param distance Distance from listener
         */
        void ProcessHRTF(const std::string& sourceId, float azimuth, float elevation, float distance);

        /**
         * @brief Ray cast for occlusion detection
         * @param start Start position
         * @param end End position
         * @return true if path is occluded
         */
        bool RayCastOcclusion(const Vec3& start, const Vec3& end);

        /**
         * @brief Calculate sound speed based on environment
         * @return Speed of sound in current environment
         */
        float CalculateSoundSpeed() const;

        /**
         * @brief Update listener orientation matrix
         */
        void UpdateListenerMatrix();

        SpatialAudioConfig m_config;                        ///< System configuration
        SpatialAudioListener m_listener;                    ///< Listener spatial data
        SpatialAudioEnvironment m_environment;              ///< Environment properties

        // HRTF system
        bool m_hrtfSupported = false;                       ///< HRTF support flag
        bool m_hrtfEnabled = false;                         ///< HRTF enabled flag
        std::unordered_map<std::string, void*> m_hrtfData; ///< HRTF data cache

        // Cached spatial data
        std::unordered_map<std::string, SpatialAudioSource> m_spatialSources;
        std::unordered_map<std::string, AudioOcclusion> m_occlusionCache;

        // Listener orientation matrix
        float m_listenerMatrix[16];                         ///< 4x4 orientation matrix

        // Performance tracking
        std::atomic<uint64_t> m_processedSources;          ///< Sources processed this frame
        std::atomic<uint64_t> m_occlusionRays;             ///< Occlusion rays cast this frame

        // Thread safety
        mutable std::shared_mutex m_systemMutex;           ///< System-wide synchronization

        // Debug information
        bool m_debugMode = false;                          ///< Debug mode flag
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_AUDIO_SPATIAL_AUDIO_SYSTEM_HPP
