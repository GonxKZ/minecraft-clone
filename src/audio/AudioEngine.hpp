/**
 * @file AudioEngine.hpp
 * @brief VoxelCraft Advanced Audio Engine - 3D Audio with HRTF
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines the AudioEngine class that provides comprehensive 3D audio
 * capabilities for the VoxelCraft game engine, including HRTF spatial audio,
 * procedural audio generation, dynamic music systems, and advanced audio
 * processing with support for multiple audio APIs and formats.
 */

#ifndef VOXELCRAFT_AUDIO_AUDIO_ENGINE_HPP
#define VOXELCRAFT_AUDIO_AUDIO_ENGINE_HPP

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
#include <queue>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../core/Config.hpp"
#include "../core/System.hpp"

namespace VoxelCraft {

    // Forward declarations
    class SoundManager;
    class MusicSystem;
    class Audio3D;
    class AudioEffect;
    class AudioResource;
    class AudioMixer;
    class AudioDSP;
    class AudioSource;
    class AudioListener;
    class AudioEmitter;

    /**
     * @enum AudioAPI
     * @brief Supported audio APIs
     */
    enum class AudioAPI {
        OpenAL,                 ///< OpenAL (cross-platform)
        XAudio2,                ///< XAudio2 (Windows)
        CoreAudio,              ///< Core Audio (macOS)
        ALSA,                   ///< ALSA (Linux)
        PulseAudio,             ///< PulseAudio (Linux)
        WASAPI,                 ///< WASAPI (Windows)
        Auto                    ///< Automatic selection
    };

    /**
     * @enum AudioFormat
     * @brief Supported audio formats
     */
    enum class AudioFormat {
        WAV,                    ///< WAV (uncompressed)
        OGG,                    ///< OGG Vorbis
        MP3,                    ///< MP3
        FLAC,                   ///< FLAC (lossless)
        OPUS,                   ///< Opus
        AAC,                    ///< AAC
        PCM,                    ///< Raw PCM
        ADPCM,                  ///< ADPCM
        Custom                  ///< Custom format
    };

    /**
     * @enum AudioChannel
     * @brief Audio channel configurations
     */
    enum class AudioChannel {
        Mono,                   ///< Mono (1 channel)
        Stereo,                 ///< Stereo (2 channels)
        Quad,                   ///< Quad (4 channels)
        Surround5_1,            ///< 5.1 Surround
        Surround7_1,            ///< 7.1 Surround
        Ambisonic1,             ///< First-order Ambisonics
        Ambisonic2,             ///< Second-order Ambisonics
        Ambisonic3              ///< Third-order Ambisonics
    };

    /**
     * @enum AudioQuality
     * @brief Audio quality levels
     */
    enum class AudioQuality {
        Low,                    ///< Low quality for performance
        Medium,                 ///< Medium quality balance
        High,                   ///< High quality for modern systems
        Ultra,                  ///< Ultra quality with advanced features
        Custom                  ///< Custom quality settings
    };

    /**
     * @enum AudioReverbType
     * @brief Reverb environment types
     */
    enum class AudioReverbType {
        Generic,                ///< Generic environment
        PaddedCell,             ///< Small enclosed space
        Room,                   ///< Small room
        Bathroom,               ///< Bathroom
        LivingRoom,             ///< Living room
        StoneRoom,              ///< Stone room
        Auditorium,             ///< Large auditorium
        ConcertHall,            ///< Concert hall
        Cave,                   ///< Cave
        Arena,                  ///< Large arena
        Hangar,                 ///< Aircraft hangar
        CarpetedHallway,        ///< Carpeted hallway
        Hallway,                ///< Empty hallway
        StoneCorridor,          ///< Stone corridor
        Alley,                  ///< Outdoor alley
        Forest,                 ///< Forest
        City,                   ///< City environment
        Mountains,              ///< Mountain environment
        Quarry,                 ///< Quarry
        Plain,                  ///< Open plain
        ParkingLot,             ///< Parking lot
        SewerPipe,              ///< Sewer pipe
        Underwater,             ///< Underwater
        Custom                  ///< Custom reverb
    };

    /**
     * @enum AudioFilterType
     * @brief Audio filter types
     */
    enum class AudioFilterType {
        LowPass,                ///< Low-pass filter
        HighPass,               ///< High-pass filter
        BandPass,               ///< Band-pass filter
        LowShelf,               ///< Low shelf filter
        HighShelf,              ///< High shelf filter
        Peak,                   ///< Peak filter
        Notch,                  ///< Notch filter
        AllPass,                ///< All-pass filter
        Custom                  ///< Custom filter
    };

    /**
     * @struct AudioConfig
     * @brief Audio engine configuration settings
     */
    struct AudioConfig {
        // Basic settings
        AudioAPI api;                           ///< Audio API to use
        AudioQuality quality;                   ///< Overall audio quality
        AudioChannel channelConfig;             ///< Channel configuration
        int sampleRate;                         ///< Sample rate (Hz)
        int bufferSize;                         ///< Audio buffer size

        // 3D Audio settings
        bool enable3DAudio;                     ///< Enable 3D spatial audio
        bool enableHRTF;                        ///< Enable HRTF for realistic audio
        float speedOfSound;                     ///< Speed of sound (m/s)
        float dopplerFactor;                    ///< Doppler effect factor
        float distanceFactor;                   ///< Distance attenuation factor

        // Performance settings
        int maxAudioSources;                    ///< Maximum concurrent audio sources
        int maxAudioEmitters;                   ///< Maximum audio emitters
        int maxConcurrentSounds;                ///< Maximum concurrent sounds
        float updateInterval;                   ///< Audio update interval (seconds)
        bool enableThreading;                   ///< Enable audio threading

        // Quality settings
        bool enableReverb;                      ///< Enable environmental reverb
        bool enableOcclusion;                   ///< Enable sound occlusion
        bool enableObstruction;                 ///< Enable sound obstruction
        bool enableReflections;                 ///< Enable early reflections
        bool enableHFDamping;                   ///< Enable high frequency damping

        // Resource management
        bool enableCompression;                 ///< Enable audio compression
        bool enableStreaming;                   ///< Enable audio streaming
        int maxLoadedSounds;                    ///< Maximum loaded sounds in memory
        int maxStreamedSounds;                  ///< Maximum streamed sounds
        std::string cachePath;                  ///< Audio cache directory

        // Advanced settings
        bool enableProfiling;                   ///< Enable audio profiling
        bool enableDebugOutput;                 ///< Enable debug output
        bool enableSpatialization;              ///< Enable advanced spatialization
        float masterVolume;                     ///< Master volume (0.0 - 1.0)
        float musicVolume;                      ///< Music volume (0.0 - 1.0)
        float sfxVolume;                        ///< Sound effects volume (0.0 - 1.0)
        float voiceVolume;                      ///< Voice volume (0.0 - 1.0)

        AudioConfig()
            : api(AudioAPI::Auto)
            , quality(AudioQuality::High)
            , channelConfig(AudioChannel::Stereo)
            , sampleRate(44100)
            , bufferSize(1024)
            , enable3DAudio(true)
            , enableHRTF(true)
            , speedOfSound(343.0f)
            , dopplerFactor(1.0f)
            , distanceFactor(1.0f)
            , maxAudioSources(256)
            , maxAudioEmitters(512)
            , maxConcurrentSounds(64)
            , updateInterval(1.0f / 60.0f)
            , enableThreading(true)
            , enableReverb(true)
            , enableOcclusion(true)
            , enableObstruction(true)
            , enableReflections(true)
            , enableHFDamping(true)
            , enableCompression(true)
            , enableStreaming(true)
            , maxLoadedSounds(100)
            , maxStreamedSounds(10)
            , cachePath("cache/audio")
            , enableProfiling(false)
            , enableDebugOutput(false)
            , enableSpatialization(true)
            , masterVolume(1.0f)
            , musicVolume(1.0f)
            , sfxVolume(1.0f)
            , voiceVolume(1.0f)
        {}
    };

    /**
     * @struct AudioStats
     * @brief Audio engine performance statistics
     */
    struct AudioStats {
        // Performance metrics
        double totalAudioTime;                  ///< Total audio processing time (ms)
        double averageAudioTime;                ///< Average audio processing time (ms)
        double maxAudioTime;                    ///< Maximum audio processing time (ms)
        int audioUpdates;                       ///< Number of audio updates

        // Resource usage
        int activeSources;                      ///< Number of active audio sources
        int activeEmitters;                     ///< Number of active audio emitters
        int loadedSounds;                       ///< Number of loaded sound resources
        int streamedSounds;                     ///< Number of streamed sounds
        size_t memoryUsage;                     ///< Total audio memory usage (bytes)

        // Audio metrics
        int soundsPlayed;                       ///< Total sounds played
        int soundsStopped;                      ///< Total sounds stopped
        float averageVolume;                    ///< Average playback volume
        float averageDistance;                  ///< Average source-listener distance

        // 3D Audio metrics
        int spatializedSounds;                  ///< Number of spatialized sounds
        int occludedSounds;                     ///< Number of occluded sounds
        int reverberatedSounds;                 ///< Number of reverberated sounds
        float averageRT60;                      ///< Average reverberation time (RT60)

        // Performance bottlenecks
        std::string bottleneckSource;           ///< Current bottleneck source
        float bottleneckPercentage;             ///< Bottleneck percentage
        std::vector<std::string> performanceWarnings; ///< Performance warnings
    };

    /**
     * @struct AudioListener
     * @brief Audio listener (player's ears)
     */
    struct AudioListener {
        glm::vec3 position;                     ///< Listener position
        glm::quat orientation;                  ///< Listener orientation
        glm::vec3 velocity;                     ///< Listener velocity
        float gain;                             ///< Master gain
        float metersPerUnit;                    ///< Distance units conversion

        AudioListener()
            : position(0.0f, 0.0f, 0.0f)
            , orientation(1.0f, 0.0f, 0.0f, 0.0f)
            , velocity(0.0f, 0.0f, 0.0f)
            , gain(1.0f)
            , metersPerUnit(1.0f)
        {}
    };

    /**
     * @struct AudioSource
     * @brief Audio source configuration
     */
    struct AudioSource {
        uint32_t sourceId;                      ///< Unique source ID
        std::string name;                       ///< Source name
        glm::vec3 position;                     ///< Source position
        glm::vec3 velocity;                     ///< Source velocity
        float gain;                             ///< Source gain
        float pitch;                            ///< Source pitch
        bool loop;                              ///< Loop playback
        bool playing;                           ///< Currently playing
        bool paused;                            ///< Currently paused

        // 3D audio properties
        float minDistance;                      ///< Minimum distance
        float maxDistance;                      ///< Maximum distance
        float rolloffFactor;                    ///< Distance rolloff factor
        float coneInnerAngle;                   ///< Inner cone angle
        float coneOuterAngle;                   ///< Outer cone angle
        float coneOuterGain;                    ///< Outer cone gain

        // Spatialization
        bool enableSpatialization;              ///< Enable spatialization
        bool enableHRTF;                        ///< Enable HRTF for this source

        // Effects
        std::vector<uint32_t> effectSlots;      ///< Applied effect slots

        AudioSource()
            : sourceId(0)
            , position(0.0f, 0.0f, 0.0f)
            , velocity(0.0f, 0.0f, 0.0f)
            , gain(1.0f)
            , pitch(1.0f)
            , loop(false)
            , playing(false)
            , paused(false)
            , minDistance(1.0f)
            , maxDistance(1000.0f)
            , rolloffFactor(1.0f)
            , coneInnerAngle(360.0f)
            , coneOuterAngle(360.0f)
            , coneOuterGain(0.0f)
            , enableSpatialization(true)
            , enableHRTF(true)
        {}
    };

    /**
     * @struct AudioEmitter
     * @brief Audio emitter for 3D sound sources
     */
    struct AudioEmitter {
        uint32_t emitterId;                     ///< Unique emitter ID
        std::string name;                       ///< Emitter name
        glm::vec3 position;                     ///< Emitter position
        glm::quat rotation;                     ///< Emitter rotation
        glm::vec3 velocity;                     ///< Emitter velocity
        float radius;                           ///< Emitter radius
        bool enabled;                           ///< Emitter enabled

        // Audio properties
        std::vector<uint32_t> audioSources;     ///< Associated audio sources
        std::unordered_map<std::string, float> parameters; ///< Custom parameters

        // Environmental effects
        AudioReverbType reverbType;             ///< Reverb environment type
        bool enableOcclusion;                   ///< Enable occlusion
        bool enableObstruction;                 ///< Enable obstruction
        float occlusionFactor;                  ///< Current occlusion factor

        AudioEmitter()
            : emitterId(0)
            , position(0.0f, 0.0f, 0.0f)
            , rotation(1.0f, 0.0f, 0.0f, 0.0f)
            , velocity(0.0f, 0.0f, 0.0f)
            , radius(1.0f)
            , enabled(true)
            , reverbType(AudioReverbType::Generic)
            , enableOcclusion(true)
            , enableObstruction(true)
            , occlusionFactor(0.0f)
        {}
    };

    /**
     * @class AudioEngine
     * @brief Advanced audio engine for VoxelCraft
     *
     * The AudioEngine class provides comprehensive 3D audio capabilities for the
     * VoxelCraft game engine, featuring HRTF spatial audio, procedural sound
     * generation, dynamic music systems, environmental audio processing, and
     * advanced audio effects with support for multiple audio APIs and high-quality
     * audio processing for immersive gaming experiences.
     *
     * Key Features:
     * - Multiple audio APIs (OpenAL, XAudio2, CoreAudio, ALSA, WASAPI)
     * - 3D spatial audio with HRTF for realistic sound positioning
     * - Procedural audio generation for environmental sounds
     * - Dynamic music systems with adaptive mixing
     * - Advanced audio effects (reverb, occlusion, obstruction)
     * - Multi-channel audio support (up to 7.1 surround)
     * - Real-time audio processing and filtering
     * - Audio resource management with streaming and compression
     * - Performance monitoring and optimization
     * - Cross-platform audio support
     * - Integration with physics and game systems
     *
     * The audio engine is designed to provide cinematic-quality audio experiences
     * while maintaining high performance and low latency for gaming applications.
     */
    class AudioEngine : public System {
    public:
        /**
         * @brief Constructor
         * @param config Audio engine configuration
         */
        explicit AudioEngine(const AudioConfig& config);

        /**
         * @brief Destructor
         */
        ~AudioEngine();

        /**
         * @brief Deleted copy constructor
         */
        AudioEngine(const AudioEngine&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        AudioEngine& operator=(const AudioEngine&) = delete;

        // Audio engine lifecycle

        /**
         * @brief Initialize audio engine
         * @return true if successful, false otherwise
         */
        bool Initialize();

        /**
         * @brief Shutdown audio engine
         */
        void Shutdown();

        /**
         * @brief Update audio engine
         * @param deltaTime Time elapsed since last update
         */
        void Update(double deltaTime);

        /**
         * @brief Get audio configuration
         * @return Current configuration
         */
        const AudioConfig& GetConfig() const { return m_config; }

        /**
         * @brief Set audio configuration
         * @param config New configuration
         * @return true if successful, false otherwise
         */
        bool SetConfig(const AudioConfig& config);

        /**
         * @brief Get audio statistics
         * @return Current audio statistics
         */
        const AudioStats& GetStats() const { return m_stats; }

        // Audio listener management

        /**
         * @brief Set audio listener
         * @param listener Audio listener configuration
         * @return true if successful, false otherwise
         */
        bool SetListener(const AudioListener& listener);

        /**
         * @brief Get current audio listener
         * @return Current audio listener
         */
        const AudioListener& GetListener() const { return m_listener; }

        /**
         * @brief Update listener position
         * @param position New position
         * @param orientation New orientation
         * @param velocity New velocity
         * @return true if successful, false otherwise
         */
        bool UpdateListener(const glm::vec3& position, const glm::quat& orientation,
                          const glm::vec3& velocity = glm::vec3(0.0f));

        // Audio source management

        /**
         * @brief Create audio source
         * @param name Source name
         * @param config Source configuration
         * @return Source ID or 0 if failed
         */
        uint32_t CreateAudioSource(const std::string& name, const AudioSource& config);

        /**
         * @brief Get audio source
         * @param sourceId Source ID
         * @return Audio source or nullopt if not found
         */
        std::optional<AudioSource> GetAudioSource(uint32_t sourceId) const;

        /**
         * @brief Update audio source
         * @param sourceId Source ID
         * @param config New configuration
         * @return true if successful, false otherwise
         */
        bool UpdateAudioSource(uint32_t sourceId, const AudioSource& config);

        /**
         * @brief Destroy audio source
         * @param sourceId Source ID
         * @return true if successful, false otherwise
         */
        bool DestroyAudioSource(uint32_t sourceId);

        // Audio emitter management

        /**
         * @brief Create audio emitter
         * @param name Emitter name
         * @param config Emitter configuration
         * @return Emitter ID or 0 if failed
         */
        uint32_t CreateAudioEmitter(const std::string& name, const AudioEmitter& config);

        /**
         * @brief Get audio emitter
         * @param emitterId Emitter ID
         * @return Audio emitter or nullopt if not found
         */
        std::optional<AudioEmitter> GetAudioEmitter(uint32_t emitterId) const;

        /**
         * @brief Update audio emitter
         * @param emitterId Emitter ID
         * @param config New configuration
         * @return true if successful, false otherwise
         */
        bool UpdateAudioEmitter(uint32_t emitterId, const AudioEmitter& config);

        /**
         * @brief Destroy audio emitter
         * @param emitterId Emitter ID
         * @return true if successful, false otherwise
         */
        bool DestroyAudioEmitter(uint32_t emitterId);

        // Sound playback

        /**
         * @brief Play sound from file
         * @param sourceId Audio source ID
         * @param filePath Sound file path
         * @return true if successful, false otherwise
         */
        bool PlaySound(uint32_t sourceId, const std::string& filePath);

        /**
         * @brief Play sound from resource
         * @param sourceId Audio source ID
         * @param resourceId Audio resource ID
         * @return true if successful, false otherwise
         */
        bool PlaySound(uint32_t sourceId, uint32_t resourceId);

        /**
         * @brief Stop sound
         * @param sourceId Audio source ID
         * @return true if successful, false otherwise
         */
        bool StopSound(uint32_t sourceId);

        /**
         * @brief Pause sound
         * @param sourceId Audio source ID
         * @return true if successful, false otherwise
         */
        bool PauseSound(uint32_t sourceId);

        /**
         * @brief Resume sound
         * @param sourceId Audio source ID
         * @return true if successful, false otherwise
         */
        bool ResumeSound(uint32_t sourceId);

        /**
         * @brief Check if sound is playing
         * @param sourceId Audio source ID
         * @return true if playing, false otherwise
         */
        bool IsSoundPlaying(uint32_t sourceId) const;

        // Audio resource management

        /**
         * @brief Load audio resource
         * @param name Resource name
         * @param filePath File path
         * @param streaming Enable streaming for large files
         * @return Resource ID or 0 if failed
         */
        uint32_t LoadAudioResource(const std::string& name, const std::string& filePath,
                                 bool streaming = false);

        /**
         * @brief Unload audio resource
         * @param resourceId Resource ID
         * @return true if successful, false otherwise
         */
        bool UnloadAudioResource(uint32_t resourceId);

        /**
         * @brief Get audio resource info
         * @param resourceId Resource ID
         * @return Resource information
         */
        std::string GetAudioResourceInfo(uint32_t resourceId) const;

        // Volume and mixing

        /**
         * @brief Set master volume
         * @param volume Volume level (0.0 - 1.0)
         * @return true if successful, false otherwise
         */
        bool SetMasterVolume(float volume);

        /**
         * @brief Set music volume
         * @param volume Volume level (0.0 - 1.0)
         * @return true if successful, false otherwise
         */
        bool SetMusicVolume(float volume);

        /**
         * @brief Set sound effects volume
         * @param volume Volume level (0.0 - 1.0)
         * @return true if successful, false otherwise
         */
        bool SetSFXVolume(float volume);

        /**
         * @brief Set voice volume
         * @param volume Volume level (0.0 - 1.0)
         * @return true if successful, false otherwise
         */
        bool SetVoiceVolume(float volume);

        /**
         * @brief Set category volume
         * @param category Audio category
         * @param volume Volume level (0.0 - 1.0)
         * @return true if successful, false otherwise
         */
        bool SetCategoryVolume(const std::string& category, float volume);

        // Audio effects

        /**
         * @brief Create audio effect
         * @param type Effect type
         * @param config Effect configuration
         * @return Effect ID or 0 if failed
         */
        uint32_t CreateAudioEffect(AudioFilterType type, const std::any& config);

        /**
         * @brief Apply effect to source
         * @param sourceId Source ID
         * @param effectId Effect ID
         * @param slot Effect slot (0-3)
         * @return true if successful, false otherwise
         */
        bool ApplyEffectToSource(uint32_t sourceId, uint32_t effectId, int slot = 0);

        /**
         * @brief Remove effect from source
         * @param sourceId Source ID
         * @param slot Effect slot
         * @return true if successful, false otherwise
         */
        bool RemoveEffectFromSource(uint32_t sourceId, int slot = 0);

        /**
         * @brief Set global reverb
         * @param type Reverb type
         * @param wetLevel Wet level (0.0 - 1.0)
         * @param dryLevel Dry level (0.0 - 1.0)
         * @return true if successful, false otherwise
         */
        bool SetGlobalReverb(AudioReverbType type, float wetLevel = 0.3f, float dryLevel = 0.7f);

        // 3D Audio features

        /**
         * @brief Enable HRTF
         * @param enabled Enable state
         * @return true if successful, false otherwise
         */
        bool EnableHRTF(bool enabled);

        /**
         * @brief Set sound occlusion
         * @param sourceId Source ID
         * @param occlusion Occlusion factor (0.0 - 1.0)
         * @return true if successful, false otherwise
         */
        bool SetSoundOcclusion(uint32_t sourceId, float occlusion);

        /**
         * @brief Set sound obstruction
         * @param sourceId Source ID
         * @param obstruction Obstruction factor (0.0 - 1.0)
         * @return true if successful, false otherwise
         */
        bool SetSoundObstruction(uint32_t sourceId, float obstruction);

        /**
         * @brief Set sound reflection
         * @param sourceId Source ID
         * @param reflection Reflection parameters
         * @return true if successful, false otherwise
         */
        bool SetSoundReflection(uint32_t sourceId, const std::any& reflection);

        // Procedural audio

        /**
         * @brief Generate procedural sound
         * @param type Sound type
         * @param parameters Generation parameters
         * @return Generated sound data or nullopt if failed
         */
        std::optional<std::vector<uint8_t>> GenerateProceduralSound(const std::string& type,
                                                                   const std::any& parameters);

        /**
         * @brief Create environmental sound
         * @param environment Environment type
         * @param intensity Sound intensity
         * @return Sound ID or 0 if failed
         */
        uint32_t CreateEnvironmentalSound(const std::string& environment, float intensity);

        /**
         * @brief Update environmental audio
         * @param environment Current environment
         * @param weather Current weather conditions
         * @return true if successful, false otherwise
         */
        bool UpdateEnvironmentalAudio(const std::string& environment, const std::any& weather);

        // Music system integration

        /**
         * @brief Play music track
         * @param trackName Music track name
         * @param fadeInTime Fade in time (seconds)
         * @return true if successful, false otherwise
         */
        bool PlayMusic(const std::string& trackName, float fadeInTime = 2.0f);

        /**
         * @brief Stop music
         * @param fadeOutTime Fade out time (seconds)
         * @return true if successful, false otherwise
         */
        bool StopMusic(float fadeOutTime = 2.0f);

        /**
         * @brief Crossfade to new music track
         * @param trackName New music track name
         * @param crossfadeTime Crossfade duration (seconds)
         * @return true if successful, false otherwise
         */
        bool CrossfadeMusic(const std::string& trackName, float crossfadeTime = 3.0f);

        /**
         * @brief Set music intensity
         * @param intensity Music intensity (0.0 - 1.0)
         * @return true if successful, false otherwise
         */
        bool SetMusicIntensity(float intensity);

        // Performance and debugging

        /**
         * @brief Enable audio profiling
         * @param enabled Enable state
         * @return true if successful, false otherwise
         */
        bool EnableProfiling(bool enabled);

        /**
         * @brief Get audio performance report
         * @return Detailed performance report
         */
        std::string GetPerformanceReport() const;

        /**
         * @brief Get memory usage report
         * @return Memory usage report
         */
        std::string GetMemoryReport() const;

        /**
         * @brief Optimize audio for current hardware
         * @return Optimization results
         */
        std::unordered_map<std::string, size_t> OptimizeForHardware();

        // Utility functions

        /**
         * @brief Get supported audio APIs
         * @return Vector of supported APIs
         */
        std::vector<AudioAPI> GetSupportedAPIs() const;

        /**
         * @brief Get supported audio formats
         * @return Vector of supported formats
         */
        std::vector<AudioFormat> GetSupportedFormats() const;

        /**
         * @brief Check if feature is supported
         * @param feature Feature name
         * @return true if supported, false otherwise
         */
        bool IsFeatureSupported(const std::string& feature) const;

        /**
         * @brief Get audio API name
         * @return Current API name
         */
        std::string GetAPIName() const;

        /**
         * @brief Validate audio engine state
         * @return true if valid, false otherwise
         */
        bool Validate() const;

        /**
         * @brief Get status report
         * @return Detailed status report
         */
        std::string GetStatusReport() const;

        /**
         * @brief Get debug information
         * @param sourceId Optional source ID for specific debug info
         * @return Debug information string
         */
        std::string GetDebugInfo(uint32_t sourceId = 0) const;

    private:
        /**
         * @brief Initialize audio API
         * @return true if successful, false otherwise
         */
        bool InitializeAPI();

        /**
         * @brief Initialize audio subsystems
         * @return true if successful, false otherwise
         */
        bool InitializeSubsystems();

        /**
         * @brief Update 3D audio positioning
         */
        void Update3DAudio();

        /**
         * @brief Update environmental effects
         */
        void UpdateEnvironmentalEffects();

        /**
         * @brief Process audio effects
         */
        void ProcessAudioEffects();

        /**
         * @brief Update performance statistics
         * @param deltaTime Time elapsed
         */
        void UpdateStats(double deltaTime);

        /**
         * @brief Handle audio errors
         * @param error Error message
         */
        void HandleError(const std::string& error);

        // Audio engine data
        AudioConfig m_config;                         ///< Audio configuration
        AudioStats m_stats;                           ///< Performance statistics

        // Core systems
        std::unique_ptr<SoundManager> m_soundManager;     ///< Sound management
        std::unique_ptr<MusicSystem> m_musicSystem;       ///< Music system
        std::unique_ptr<Audio3D> m_audio3D;               ///< 3D audio system
        std::unique_ptr<AudioEffect> m_audioEffect;       ///< Audio effects
        std::unique_ptr<AudioResource> m_audioResource;   ///< Audio resources
        std::unique_ptr<AudioMixer> m_audioMixer;         ///< Audio mixer
        std::unique_ptr<AudioDSP> m_audioDSP;             ///< Audio DSP

        // Audio state
        AudioListener m_listener;                      ///< Current audio listener
        std::unordered_map<uint32_t, AudioSource> m_audioSources; ///< Audio sources
        std::unordered_map<uint32_t, AudioEmitter> m_audioEmitters; ///< Audio emitters
        std::unordered_map<uint32_t, std::any> m_audioEffects; ///< Audio effects

        // Resource management
        std::unordered_map<uint32_t, std::shared_ptr<AudioResource>> m_resources; ///< Audio resources
        mutable std::shared_mutex m_resourcesMutex;     ///< Resources synchronization

        // Environmental audio
        AudioReverbType m_currentReverb;               ///< Current reverb type
        std::unordered_map<std::string, uint32_t> m_environmentalSounds; ///< Environmental sounds
        mutable std::shared_mutex m_environmentMutex;  ///< Environment synchronization

        // Performance tracking
        std::queue<double> m_updateTimes;              ///< Update time history
        bool m_profilingEnabled;                       ///< Profiling enabled
        double m_lastUpdateTime;                       ///< Last update time

        // State tracking
        bool m_isInitialized;                          ///< Engine is initialized
        std::string m_lastError;                       ///< Last error message

        static std::atomic<uint32_t> s_nextSourceId;   ///< Next source ID counter
        static std::atomic<uint32_t> s_nextEmitterId;  ///< Next emitter ID counter
        static std::atomic<uint32_t> s_nextResourceId; ///< Next resource ID counter
        static std::atomic<uint32_t> s_nextEffectId;   ///< Next effect ID counter
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_AUDIO_AUDIO_ENGINE_HPP
