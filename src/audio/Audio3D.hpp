/**
 * @file Audio3D.hpp
 * @brief VoxelCraft 3D Spatial Audio with HRTF
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines the Audio3D class that provides comprehensive 3D spatial audio
 * capabilities for the VoxelCraft game engine, including HRTF (Head-Related Transfer
 * Function) for realistic sound positioning, environmental audio processing, sound
 * propagation, occlusion, and advanced spatialization techniques for immersive
 * 3D audio experiences.
 */

#ifndef VOXELCRAFT_AUDIO_AUDIO3D_HPP
#define VOXELCRAFT_AUDIO_AUDIO3D_HPP

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
#include <array>
#include <complex>
#include <valarray>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../core/Config.hpp"

namespace VoxelCraft {

    // Forward declarations
    class AudioEngine;
    class AudioListener;
    class AudioEmitter;
    class AudioSource;

    /**
     * @enum HRTFQuality
     * @brief HRTF (Head-Related Transfer Function) quality levels
     */
    enum class HRTFQuality {
        Low,                    ///< Low quality HRTF (fast)
        Medium,                 ///< Medium quality HRTF
        High,                   ///< High quality HRTF
        Ultra,                  ///< Ultra quality HRTF (accurate)
        Custom                  ///< Custom HRTF dataset
    };

    /**
     * @enum SpatializationMethod
     * @brief Audio spatialization methods
     */
    enum class SpatializationMethod {
        None,                   ///< No spatialization
        Pan,                    ///< Simple stereo panning
        VBAP,                   ///< Vector Base Amplitude Panning
        Ambisonics,             ///< Ambisonic spatialization
        HRTF,                   ///< HRTF-based spatialization
        WaveField,              ///< Wave field synthesis
        Custom                  ///< Custom spatialization
    };

    /**
     * @enum SoundPropagationModel
     * @brief Sound propagation models
     */
    enum class SoundPropagationModel {
        Simple,                 ///< Simple distance-based attenuation
        Atmospheric,            ///< Atmospheric absorption model
        Geometric,              ///< Geometric spreading model
        Physical,               ///< Physical acoustics model
        Custom                  ///< Custom propagation model
    };

    /**
     * @enum OcclusionType
     * @brief Sound occlusion types
     */
    enum class OcclusionType {
        None,                   ///< No occlusion
        Simple,                 ///< Simple occlusion (frequency-independent)
        FrequencyDependent,     ///< Frequency-dependent occlusion
        Spectral,               ///< Spectral occlusion analysis
        RayTraced               ///< Ray-traced occlusion
    };

    /**
     * @struct HRTFData
     * @brief HRTF (Head-Related Transfer Function) data
     */
    struct HRTFData {
        int azimuth;                                ///< Azimuth angle (degrees)
        int elevation;                              ///< Elevation angle (degrees)
        int distance;                               ///< Distance (cm)
        std::vector<float> leftImpulseResponse;     ///< Left ear impulse response
        std::vector<float> rightImpulseResponse;    ///< Right ear impulse response
        int irLength;                               ///< Impulse response length
        float sampleRate;                           ///< Sample rate

        HRTFData()
            : azimuth(0)
            , elevation(0)
            , distance(0)
            , irLength(0)
            , sampleRate(44100.0f)
        {}
    };

    /**
     * @struct SpatialAudioConfig
     * @brief 3D audio configuration settings
     */
    struct SpatialAudioConfig {
        // HRTF settings
        HRTFQuality hrtfQuality;                    ///< HRTF quality level
        bool enableHRTF;                            ///< Enable HRTF processing
        std::string hrtfDataset;                    ///< HRTF dataset file
        bool enableHRTFInterpolation;               ///< Enable HRTF interpolation

        // Spatialization settings
        SpatializationMethod spatializationMethod;  ///< Spatialization method
        bool enableNearFieldEffects;                ///< Enable near-field effects
        bool enableFarFieldEffects;                 ///< Enable far-field effects
        bool enableDopplerEffect;                   ///< Enable Doppler effect
        float speedOfSound;                         ///< Speed of sound (m/s)

        // Propagation settings
        SoundPropagationModel propagationModel;     ///< Sound propagation model
        bool enableAirAbsorption;                  ///< Enable air absorption
        bool enableTemperatureEffects;              ///< Enable temperature effects
        bool enableHumidityEffects;                 ///< Enable humidity effects
        float airTemperature;                       ///< Air temperature (Celsius)
        float airHumidity;                          ///< Air humidity (0.0 - 1.0)

        // Occlusion settings
        OcclusionType occlusionType;                ///< Occlusion calculation type
        bool enableMultipleReflections;             ///< Enable multiple reflections
        bool enableDiffraction;                     ///< Enable sound diffraction
        float occlusionStrength;                    ///< Occlusion strength
        float diffractionStrength;                  ///< Diffraction strength

        // Performance settings
        bool enableDistanceCulling;                 ///< Enable distance culling
        float maxAudioDistance;                     ///< Maximum audio processing distance
        int maxConcurrentSources;                   ///< Maximum concurrent 3D sources
        bool enableSourcePooling;                   ///< Enable source pooling
        int updateFrequency;                        ///< 3D audio update frequency (Hz)

        // Advanced settings
        bool enableHeadTracking;                    ///< Enable head tracking
        bool enableDynamicHRTF;                     ///< Enable dynamic HRTF adjustment
        bool enableSourceDirectivity;               ///< Enable source directivity
        bool enableListenerDirectivity;             ///< Enable listener directivity
        int ambisonicsOrder;                        ///< Ambisonics order (if used)

        SpatialAudioConfig()
            : hrtfQuality(HRTFQuality::High)
            , enableHRTF(true)
            , enableHRTFInterpolation(true)
            , spatializationMethod(SpatializationMethod::HRTF)
            , enableNearFieldEffects(true)
            , enableFarFieldEffects(true)
            , enableDopplerEffect(true)
            , speedOfSound(343.0f)
            , propagationModel(SoundPropagationModel::Physical)
            , enableAirAbsorption(true)
            , enableTemperatureEffects(true)
            , enableHumidityEffects(true)
            , airTemperature(20.0f)
            , airHumidity(0.5f)
            , occlusionType(OcclusionType::FrequencyDependent)
            , enableMultipleReflections(true)
            , enableDiffraction(true)
            , occlusionStrength(0.7f)
            , diffractionStrength(0.3f)
            , enableDistanceCulling(true)
            , maxAudioDistance(1000.0f)
            , maxConcurrentSources(64)
            , enableSourcePooling(true)
            , updateFrequency(60)
            , enableHeadTracking(false)
            , enableDynamicHRTF(false)
            , enableSourceDirectivity(false)
            , enableListenerDirectivity(false)
            , ambisonicsOrder(1)
        {}
    };

    /**
     * @struct AudioSource3D
     * @brief 3D audio source with spatial properties
     */
    struct AudioSource3D {
        uint32_t sourceId;                          ///< Audio source ID
        glm::vec3 position;                         ///< Source position
        glm::vec3 velocity;                         ///< Source velocity
        glm::quat orientation;                      ///< Source orientation

        // Directivity properties
        bool enableDirectivity;                     ///< Enable source directivity
        float innerConeAngle;                       ///< Inner cone angle (degrees)
        float outerConeAngle;                       ///< Outer cone angle (degrees)
        float coneGain;                             ///< Cone gain factor

        // Spatial properties
        float minDistance;                          ///< Minimum distance
        float maxDistance;                          ///< Maximum distance
        float rolloffFactor;                        ///< Distance rolloff factor
        bool enableDoppler;                         ///< Enable Doppler effect

        // Environmental properties
        bool enableReverb;                          ///< Enable reverb processing
        bool enableOcclusion;                       ///< Enable occlusion
        bool enableObstruction;                     ///< Enable obstruction
        float occlusionFactor;                      ///< Current occlusion factor
        float obstructionFactor;                    ///< Current obstruction factor

        // HRTF properties
        bool enableHRTF;                            ///< Enable HRTF for this source
        HRTFData interpolatedHRTF;                  ///< Interpolated HRTF data

        // Performance properties
        float distanceToListener;                   ///< Distance to listener
        bool isVirtualized;                         ///< Source is virtualized
        float virtualizationFactor;                 ///< Virtualization factor

        AudioSource3D()
            : sourceId(0)
            , position(0.0f, 0.0f, 0.0f)
            , velocity(0.0f, 0.0f, 0.0f)
            , orientation(1.0f, 0.0f, 0.0f, 0.0f)
            , enableDirectivity(false)
            , innerConeAngle(360.0f)
            , outerConeAngle(360.0f)
            , coneGain(1.0f)
            , minDistance(1.0f)
            , maxDistance(1000.0f)
            , rolloffFactor(1.0f)
            , enableDoppler(true)
            , enableReverb(true)
            , enableOcclusion(true)
            , enableObstruction(true)
            , occlusionFactor(0.0f)
            , obstructionFactor(0.0f)
            , enableHRTF(true)
            , distanceToListener(0.0f)
            , isVirtualized(false)
            , virtualizationFactor(0.0f)
        {}
    };

    /**
     * @struct AudioListener3D
     * @brief 3D audio listener with head tracking
     */
    struct AudioListener3D {
        glm::vec3 position;                         ///< Listener position
        glm::quat orientation;                      ///< Listener orientation
        glm::vec3 velocity;                         ///< Listener velocity

        // Head tracking
        bool enableHeadTracking;                    ///< Enable head tracking
        glm::vec3 headPosition;                     ///< Head position
        glm::quat headOrientation;                  ///< Head orientation

        // Listener properties
        bool enableDirectivity;                     ///< Enable listener directivity
        float innerEarDistance;                     ///< Inter-aural distance (m)
        float headRadius;                           ///< Head radius (m)

        // HRTF properties
        HRTFQuality hrtfQuality;                    ///< HRTF quality for this listener
        bool enableITD;                             ///< Enable Interaural Time Difference
        bool enableILD;                             ///< Enable Interaural Level Difference

        // Environmental properties
        AudioReverbType reverbType;                 ///< Current reverb environment
        float reverbStrength;                       ///< Reverb strength
        bool enableEnvironmentalEffects;            ///< Enable environmental effects

        AudioListener3D()
            : position(0.0f, 0.0f, 0.0f)
            , orientation(1.0f, 0.0f, 0.0f, 0.0f)
            , velocity(0.0f, 0.0f, 0.0f)
            , enableHeadTracking(false)
            , headPosition(0.0f, 0.0f, 0.0f)
            , headOrientation(1.0f, 0.0f, 0.0f, 0.0f)
            , enableDirectivity(false)
            , innerEarDistance(0.215f)
            , headRadius(0.0875f)
            , hrtfQuality(HRTFQuality::High)
            , enableITD(true)
            , enableILD(true)
            , reverbType(AudioReverbType::Generic)
            , reverbStrength(0.5f)
            , enableEnvironmentalEffects(true)
        {}
    };

    /**
     * @struct AudioRay
     * @brief Audio ray for sound propagation and occlusion
     */
    struct AudioRay {
        glm::vec3 origin;                           ///< Ray origin
        glm::vec3 direction;                        ///< Ray direction
        float length;                               ///< Ray length
        float energy;                               ///< Ray energy
        int bounceCount;                            ///< Number of bounces
        bool hitsListener;                          ///< Ray hits listener
        float travelTime;                           ///< Travel time
        std::vector<glm::vec3> bouncePoints;        ///< Bounce points

        AudioRay()
            : origin(0.0f, 0.0f, 0.0f)
            , direction(0.0f, 0.0f, 0.0f)
            , length(0.0f)
            , energy(1.0f)
            , bounceCount(0)
            , hitsListener(false)
            , travelTime(0.0f)
        {}
    };

    /**
     * @struct Audio3DStats
     * @brief 3D audio performance statistics
     */
    struct Audio3DStats {
        // Performance metrics
        double totalUpdateTime;                     ///< Total update time (ms)
        double averageUpdateTime;                   ///< Average update time (ms)
        double maxUpdateTime;                       ///< Maximum update time (ms)
        int updatesPerSecond;                       ///< Updates per second

        // Spatialization metrics
        int active3DSources;                        ///< Number of active 3D sources
        int virtualizedSources;                     ///< Number of virtualized sources
        int spatializedSources;                     ///< Number of spatialized sources
        int hrtfProcessedSources;                   ///< Number of HRTF processed sources

        // Audio processing metrics
        int occlusionCalculations;                  ///< Occlusion calculations per frame
        int reflectionCalculations;                 ///< Reflection calculations per frame
        int diffractionCalculations;                ///< Diffraction calculations per frame
        int rayTracedRays;                          ///< Ray traced rays per frame

        // Quality metrics
        float averageSpatializationError;           ///< Average spatialization error
        float averageOcclusionAccuracy;             ///< Average occlusion accuracy
        float averageReverbAccuracy;                ///< Average reverb accuracy
        float hrtfInterpolationAccuracy;            ///< HRTF interpolation accuracy

        // Resource usage
        size_t hrtfMemoryUsage;                     ///< HRTF data memory usage
        size_t rayTracingMemory;                    ///< Ray tracing memory usage
        int activeAudioRays;                        ///< Number of active audio rays
        float rayPoolUtilization;                   ///< Ray pool utilization (0-1)
    };

    /**
     * @class Audio3D
     * @brief Advanced 3D spatial audio system
     *
     * The Audio3D class provides comprehensive 3D spatial audio capabilities for the
     * VoxelCraft game engine, featuring HRTF (Head-Related Transfer Function) for
     * realistic sound positioning, environmental audio processing, sound propagation,
     * occlusion, obstruction, and advanced spatialization techniques for truly
     * immersive 3D audio experiences that rival professional audio engines.
     *
     * Key Features:
     * - HRTF-based spatial audio for realistic sound positioning
     * - Multiple spatialization methods (HRTF, VBAP, Ambisonics)
     * - Sound propagation with air absorption and atmospheric effects
     * - Advanced occlusion and obstruction calculations
     * - Ray-traced audio for accurate reflections and diffraction
     * - Doppler effect and near-field/far-field audio processing
     * - Head tracking and dynamic HRTF adjustment
     * - Source and listener directivity modeling
     * - Environmental audio with reverb zones
     * - Performance optimization with source virtualization
     * - Multi-channel audio support (up to 7.1 surround)
     * - Real-time audio parameter adjustment
     * - Integration with physics engine for accurate occlusion
     * - Procedural audio generation for environmental sounds
     *
     * The 3D audio system is designed to provide cinematic-quality spatial audio
     * that enhances immersion and provides accurate audio cues for gameplay.
     */
    class Audio3D {
    public:
        /**
         * @brief Constructor
         * @param audioEngine Audio engine instance
         */
        explicit Audio3D(AudioEngine* audioEngine);

        /**
         * @brief Destructor
         */
        ~Audio3D();

        /**
         * @brief Deleted copy constructor
         */
        Audio3D(const Audio3D&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        Audio3D& operator=(const Audio3D&) = delete;

        // Audio3D lifecycle

        /**
         * @brief Initialize 3D audio system
         * @return true if successful, false otherwise
         */
        bool Initialize();

        /**
         * @brief Shutdown 3D audio system
         */
        void Shutdown();

        /**
         * @brief Update 3D audio system
         * @param deltaTime Time elapsed since last update
         */
        void Update(double deltaTime);

        /**
         * @brief Get 3D audio configuration
         * @return Current configuration
         */
        const SpatialAudioConfig& GetConfig() const { return m_config; }

        /**
         * @brief Set 3D audio configuration
         * @param config New configuration
         * @return true if successful, false otherwise
         */
        bool SetConfig(const SpatialAudioConfig& config);

        /**
         * @brief Get 3D audio statistics
         * @return Current statistics
         */
        const Audio3DStats& GetStats() const { return m_stats; }

        // Listener management

        /**
         * @brief Set 3D audio listener
         * @param listener 3D listener configuration
         * @return true if successful, false otherwise
         */
        bool SetListener(const AudioListener3D& listener);

        /**
         * @brief Get current 3D listener
         * @return Current 3D listener
         */
        const AudioListener3D& GetListener() const { return m_listener; }

        /**
         * @brief Update listener position and orientation
         * @param position New position
         * @param orientation New orientation
         * @param velocity New velocity
         * @return true if successful, false otherwise
         */
        bool UpdateListener(const glm::vec3& position, const glm::quat& orientation,
                          const glm::vec3& velocity = glm::vec3(0.0f));

        /**
         * @brief Update head tracking
         * @param headPosition Head position
         * @param headOrientation Head orientation
         * @return true if successful, false otherwise
         */
        bool UpdateHeadTracking(const glm::vec3& headPosition, const glm::quat& headOrientation);

        // 3D source management

        /**
         * @brief Create 3D audio source
         * @param source 3D source configuration
         * @return Source ID or 0 if failed
         */
        uint32_t Create3DSource(const AudioSource3D& source);

        /**
         * @brief Update 3D audio source
         * @param sourceId Source ID
         * @param source New source configuration
         * @return true if successful, false otherwise
         */
        bool Update3DSource(uint32_t sourceId, const AudioSource3D& source);

        /**
         * @brief Destroy 3D audio source
         * @param sourceId Source ID
         * @return true if successful, false otherwise
         */
        bool Destroy3DSource(uint32_t sourceId);

        /**
         * @brief Get 3D audio source
         * @param sourceId Source ID
         * @return 3D source or nullopt if not found
         */
        std::optional<AudioSource3D> Get3DSource(uint32_t sourceId) const;

        // HRTF management

        /**
         * @brief Load HRTF dataset
         * @param datasetPath Dataset file path
         * @return true if successful, false otherwise
         */
        bool LoadHRTFDataset(const std::string& datasetPath);

        /**
         * @brief Get HRTF data for position
         * @param azimuth Azimuth angle (degrees)
         * @param elevation Elevation angle (degrees)
         * @param distance Distance (optional)
         * @return HRTF data
         */
        HRTFData GetHRTFData(int azimuth, int elevation, int distance = 0) const;

        /**
         * @brief Interpolate HRTF data
         * @param azimuth Azimuth angle (degrees)
         * @param elevation Elevation angle (degrees)
         * @param distance Distance (optional)
         * @return Interpolated HRTF data
         */
        HRTFData InterpolateHRTF(float azimuth, float elevation, float distance = 0.0f);

        /**
         * @brief Set HRTF quality
         * @param quality HRTF quality level
         * @return true if successful, false otherwise
         */
        bool SetHRTFQuality(HRTFQuality quality);

        // Spatialization methods

        /**
         * @brief Set spatialization method
         * @param method Spatialization method
         * @return true if successful, false otherwise
         */
        bool SetSpatializationMethod(SpatializationMethod method);

        /**
         * @brief Calculate spatialization parameters
         * @param source 3D source
         * @param listener 3D listener
         * @return Spatialization parameters
         */
        std::unordered_map<std::string, float> CalculateSpatialization(const AudioSource3D& source,
                                                                      const AudioListener3D& listener);

        // Sound propagation and occlusion

        /**
         * @brief Calculate sound propagation
         * @param sourcePosition Source position
         * @param listenerPosition Listener position
         * @param environment Environment properties
         * @return Propagation parameters
         */
        std::unordered_map<std::string, float> CalculateSoundPropagation(
            const glm::vec3& sourcePosition,
            const glm::vec3& listenerPosition,
            const std::any& environment);

        /**
         * @brief Calculate occlusion factor
         * @param sourcePosition Source position
         * @param listenerPosition Listener position
         * @return Occlusion factor (0.0 - 1.0)
         */
        float CalculateOcclusion(const glm::vec3& sourcePosition, const glm::vec3& listenerPosition);

        /**
         * @brief Calculate obstruction factor
         * @param sourcePosition Source position
         * @param listenerPosition Listener position
         * @return Obstruction factor (0.0 - 1.0)
         */
        float CalculateObstruction(const glm::vec3& sourcePosition, const glm::vec3& listenerPosition);

        /**
         * @brief Perform ray-traced audio
         * @param sourceId Source ID
         * @param maxRays Maximum number of rays
         * @return Ray tracing results
         */
        std::vector<AudioRay> PerformRayTracing(uint32_t sourceId, int maxRays = 100);

        // Environmental audio

        /**
         * @brief Set reverb environment
         * @param reverbType Reverb environment type
         * @param strength Reverb strength (0.0 - 1.0)
         * @return true if successful, false otherwise
         */
        bool SetReverbEnvironment(AudioReverbType reverbType, float strength = 0.5f);

        /**
         * @brief Set environmental parameters
         * @param temperature Air temperature (Celsius)
         * @param humidity Air humidity (0.0 - 1.0)
         * @param pressure Air pressure (Pa)
         * @return true if successful, false otherwise
         */
        bool SetEnvironmentalParameters(float temperature = 20.0f,
                                      float humidity = 0.5f,
                                      float pressure = 101325.0f);

        /**
         * @brief Enable environmental effects
         * @param enabled Enable state
         * @return true if successful, false otherwise
         */
        bool EnableEnvironmentalEffects(bool enabled);

        // Doppler effect

        /**
         * @brief Calculate Doppler effect
         * @param sourceVelocity Source velocity
         * @param listenerVelocity Listener velocity
         * @param sourcePosition Source position
         * @param listenerPosition Listener position
         * @return Doppler factor
         */
        float CalculateDopplerEffect(const glm::vec3& sourceVelocity,
                                   const glm::vec3& listenerVelocity,
                                   const glm::vec3& sourcePosition,
                                   const glm::vec3& listenerPosition);

        // Performance optimization

        /**
         * @brief Enable source virtualization
         * @param enabled Enable state
         * @return true if successful, false otherwise
         */
        bool EnableSourceVirtualization(bool enabled);

        /**
         * @brief Set distance culling
         * @param enabled Enable state
         * @param maxDistance Maximum distance
         * @return true if successful, false otherwise
         */
        bool SetDistanceCulling(bool enabled, float maxDistance = 1000.0f);

        /**
         * @brief Set update frequency
         * @param frequency Update frequency (Hz)
         * @return true if successful, false otherwise
         */
        bool SetUpdateFrequency(int frequency);

        // Utility functions

        /**
         * @brief Convert 3D position to spherical coordinates
         * @param position 3D position
         * @param listenerPosition Listener position
         * @return Spherical coordinates (azimuth, elevation, distance)
         */
        glm::vec3 ConvertToSpherical(const glm::vec3& position, const glm::vec3& listenerPosition) const;

        /**
         * @brief Get audio ray from pool
         * @return Audio ray from pool
         */
        AudioRay GetAudioRayFromPool();

        /**
         * @brief Return audio ray to pool
         * @param ray Audio ray to return
         */
        void ReturnAudioRayToPool(AudioRay& ray);

        /**
         * @brief Validate 3D audio system
         * @return true if valid, false otherwise
         */
        bool Validate() const;

        /**
         * @brief Get 3D audio status report
         * @return Detailed status report
         */
        std::string GetStatusReport() const;

        /**
         * @brief Get performance report
         * @return Detailed performance report
         */
        std::string GetPerformanceReport() const;

        /**
         * @brief Optimize 3D audio system
         * @return Optimization results
         */
        std::unordered_map<std::string, size_t> Optimize();

    private:
        /**
         * @brief Initialize HRTF system
         * @return true if successful, false otherwise
         */
        bool InitializeHRTF();

        /**
         * @brief Initialize spatialization system
         * @return true if successful, false otherwise
         */
        bool InitializeSpatialization();

        /**
         * @brief Initialize ray tracing system
         * @return true if successful, false otherwise
         */
        bool InitializeRayTracing();

        /**
         * @brief Update 3D audio sources
         * @param deltaTime Time elapsed
         */
        void Update3DSources(double deltaTime);

        /**
         * @brief Update spatialization
         * @param deltaTime Time elapsed
         */
        void UpdateSpatialization(double deltaTime);

        /**
         * @brief Update occlusion and obstruction
         * @param deltaTime Time elapsed
         */
        void UpdateOcclusion(double deltaTime);

        /**
         * @brief Update ray tracing
         * @param deltaTime Time elapsed
         */
        void UpdateRayTracing(double deltaTime);

        /**
         * @brief Perform HRTF convolution
         * @param sourceId Source ID
         * @param inputAudio Input audio data
         * @return Processed audio data
         */
        std::vector<float> PerformHRTFConvolution(uint32_t sourceId, const std::vector<float>& inputAudio);

        /**
         * @brief Calculate VBAP (Vector Base Amplitude Panning)
         * @param azimuth Azimuth angle
         * @param elevation Elevation angle
         * @return VBAP gains for each speaker
         */
        std::vector<float> CalculateVBAP(float azimuth, float elevation);

        /**
         * @brief Calculate Ambisonics encoding
         * @param azimuth Azimuth angle
         * @param elevation Elevation angle
         * @param order Ambisonics order
         * @return Ambisonics coefficients
         */
        std::vector<float> CalculateAmbisonics(float azimuth, float elevation, int order);

        /**
         * @brief Calculate frequency-dependent occlusion
         * @param sourcePosition Source position
         * @param listenerPosition Listener position
         * @param frequencyBands Number of frequency bands
         * @return Occlusion factors per frequency band
         */
        std::vector<float> CalculateFrequencyOcclusion(const glm::vec3& sourcePosition,
                                                     const glm::vec3& listenerPosition,
                                                     int frequencyBands = 8);

        /**
         * @brief Trace audio ray
         * @param ray Audio ray to trace
         * @return true if ray hits something, false otherwise
         */
        bool TraceAudioRay(AudioRay& ray);

        /**
         * @brief Update performance statistics
         * @param deltaTime Time elapsed
         */
        void UpdateStats(double deltaTime);

        /**
         * @brief Handle 3D audio errors
         * @param error Error message
         */
        void HandleError(const std::string& error);

        // Audio3D data
        AudioEngine* m_audioEngine;                    ///< Audio engine instance
        SpatialAudioConfig m_config;                   ///< 3D audio configuration
        Audio3DStats m_stats;                          ///< Performance statistics

        // Core systems
        AudioListener3D m_listener;                    ///< 3D audio listener

        // 3D sources
        std::unordered_map<uint32_t, AudioSource3D> m_3dSources; ///< 3D audio sources
        mutable std::shared_mutex m_sourcesMutex;      ///< Sources synchronization

        // HRTF system
        std::unordered_map<std::string, HRTFData> m_hrtfDatabase; ///< HRTF database
        HRTFQuality m_currentHRTFQuality;              ///< Current HRTF quality
        bool m_hrtfInitialized;                        ///< HRTF system initialized
        mutable std::shared_mutex m_hrtfMutex;         ///< HRTF synchronization

        // Ray tracing system
        std::vector<AudioRay> m_audioRayPool;          ///< Audio ray pool
        std::vector<AudioRay> m_activeRays;            ///< Active audio rays
        mutable std::shared_mutex m_raysMutex;         ///< Rays synchronization

        // Environmental audio
        AudioReverbType m_currentReverbType;           ///< Current reverb type
        float m_reverbStrength;                        ///< Current reverb strength
        bool m_environmentalEffectsEnabled;            ///< Environmental effects enabled

        // Spatialization parameters
        SpatializationMethod m_spatializationMethod;   ///< Current spatialization method
        std::vector<float> m_speakerGains;             ///< Speaker gains for VBAP
        std::vector<float> m_ambisonicsCoeffs;         ///< Ambisonics coefficients

        // Performance optimization
        bool m_sourceVirtualizationEnabled;            ///< Source virtualization enabled
        float m_maxAudioDistance;                      ///< Maximum audio processing distance
        int m_maxConcurrentSources;                    ///< Maximum concurrent 3D sources

        // State tracking
        bool m_isInitialized;                          ///< System is initialized
        double m_lastUpdateTime;                       ///< Last update time
        std::string m_lastError;                       ///< Last error message

        static std::atomic<uint32_t> s_nextSourceId;   ///< Next source ID counter
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_AUDIO_AUDIO3D_HPP
