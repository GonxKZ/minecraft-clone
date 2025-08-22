/**
 * @file AudioEffect.hpp
 * @brief VoxelCraft Advanced Audio Effects System
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines the AudioEffect class that provides comprehensive audio effects
 * processing for the VoxelCraft game engine, including reverb, filters, distortion,
 * modulation effects, spatial effects, and advanced DSP (Digital Signal Processing)
 * with real-time parameter control and GPU acceleration for complex audio effects.
 */

#ifndef VOXELCRAFT_AUDIO_AUDIO_EFFECT_HPP
#define VOXELCRAFT_AUDIO_AUDIO_EFFECT_HPP

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
#include <random>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../core/Config.hpp"

namespace VoxelCraft {

    // Forward declarations
    class AudioEngine;
    class AudioSource;

    /**
     * @enum AudioEffectType
     * @brief Types of audio effects
     */
    enum class AudioEffectType {
        // Time-based effects
        Reverb,                 ///< Reverb effect
        Delay,                  ///< Delay/echo effect
        Chorus,                 ///< Chorus effect
        Flanger,                ///< Flanger effect
        Phaser,                 ///< Phaser effect

        // Filter effects
        LowPassFilter,          ///< Low-pass filter
        HighPassFilter,         ///< High-pass filter
        BandPassFilter,         ///< Band-pass filter
        NotchFilter,            ///< Notch filter
        AllPassFilter,          ///< All-pass filter
        Equalizer,              ///< Multi-band equalizer

        // Distortion effects
        Distortion,             ///< Distortion/overdrive
        Overdrive,              ///< Overdrive effect
        Fuzz,                   ///< Fuzz effect
        BitCrusher,             ///< Bit crushing effect
        Decimator,              ///< Sample rate reduction

        // Modulation effects
        Tremolo,                ///< Tremolo effect
        Vibrato,                ///< Vibrato effect
        RingModulator,          ///< Ring modulation
        FrequencyShifter,       ///< Frequency shifting

        // Spatial effects
        Panning,                ///< Stereo panning
        Spatializer,            ///< Spatial audio effect
        Binaural,               ///< Binaural audio effect

        // Dynamic effects
        Compressor,             ///< Dynamic range compressor
        Limiter,                ///< Peak limiter
        Expander,               ///< Noise gate/expander
        SidechainCompressor,    ///< Sidechain compressor

        // Special effects
        PitchShifter,           ///< Pitch shifting
        Harmonizer,             ///< Harmonizer effect
        Vocoder,                ///< Vocoder effect
        Granular,               ///< Granular synthesis
        Convolution,            ///< Convolution reverb

        // Custom effects
        Custom                  ///< Custom effect
    };

    /**
     * @enum EffectProcessingMode
     * @brief Effect processing modes
     */
    enum class EffectProcessingMode {
        RealTime,               ///< Real-time processing
        Offline,                ///< Offline processing
        Hybrid                  ///< Hybrid processing
    };

    /**
     * @enum EffectQuality
     * @brief Effect processing quality levels
     */
    enum class EffectQuality {
        Low,                    ///< Low quality (fast)
        Medium,                 ///< Medium quality
        High,                   ///< High quality (accurate)
        Ultra                   ///< Ultra quality (maximum accuracy)
    };

    /**
     * @enum FilterType
     * @brief Digital filter types
     */
    enum class FilterType {
        IIR,                    ///< Infinite Impulse Response
        FIR,                    ///< Finite Impulse Response
        Biquad,                 ///< Biquad filter
        Ladder,                 ///< Ladder filter
        StateVariable,          ///< State variable filter
        Custom                  ///< Custom filter
    };

    /**
     * @struct AudioEffectConfig
     * @brief Audio effect configuration
     */
    struct AudioEffectConfig {
        AudioEffectType effectType;                  ///< Effect type
        std::string effectName;                      ///< Effect name
        EffectProcessingMode processingMode;         ///< Processing mode
        EffectQuality quality;                       ///< Processing quality
        bool enabled;                                ///< Effect enabled
        float wetLevel;                              ///< Wet signal level (0.0 - 1.0)
        float dryLevel;                              ///< Dry signal level (0.0 - 1.0)

        // Performance settings
        bool enableBypass;                           ///< Enable bypass
        bool enableAutomation;                       ///< Enable parameter automation
        int maxBlockSize;                            ///< Maximum processing block size
        float latencyCompensation;                   ///< Latency compensation (ms)

        // Advanced settings
        bool enableGPUAcceleration;                  ///< Enable GPU acceleration
        bool enableMultithreading;                   ///< Enable multithreading
        int threadCount;                             ///< Number of processing threads
        bool enableSIMD;                             ///< Enable SIMD optimization

        AudioEffectConfig()
            : effectType(AudioEffectType::Reverb)
            , processingMode(EffectProcessingMode::RealTime)
            , quality(EffectQuality::High)
            , enabled(true)
            , wetLevel(0.5f)
            , dryLevel(0.5f)
            , enableBypass(false)
            , enableAutomation(false)
            , maxBlockSize(512)
            , latencyCompensation(0.0f)
            , enableGPUAcceleration(false)
            , enableMultithreading(false)
            , threadCount(1)
            , enableSIMD(true)
        {}
    };

    /**
     * @struct ReverbConfig
     * @brief Reverb effect configuration
     */
    struct ReverbConfig {
        // Room properties
        float roomSize;                              ///< Room size (0.0 - 1.0)
        float damping;                               ///< Damping factor (0.0 - 1.0)
        float width;                                 ///< Stereo width (0.0 - 1.0)
        float level;                                 ///< Reverb level (0.0 - 1.0)

        // Pre-delay and decay
        float preDelay;                              ///< Pre-delay time (ms)
        float decayTime;                             ///< Decay time (seconds)
        float earlyReflections;                      ///< Early reflections level
        float lateReverb;                            ///< Late reverb level

        // Frequency response
        float hfDamping;                             ///< High frequency damping
        float lfDamping;                             ///< Low frequency damping
        float diffusion;                             ///< Diffusion amount
        float density;                               ///< Density of reverb

        // Advanced settings
        int reverbType;                              ///< Reverb algorithm type
        bool enableModulation;                       ///< Enable modulation
        float modulationFrequency;                   ///< Modulation frequency
        float modulationDepth;                       ///< Modulation depth

        ReverbConfig()
            : roomSize(0.5f)
            , damping(0.5f)
            , width(1.0f)
            , level(0.3f)
            , preDelay(20.0f)
            , decayTime(1.5f)
            , earlyReflections(0.5f)
            , lateReverb(0.5f)
            , hfDamping(0.5f)
            , lfDamping(0.5f)
            , diffusion(0.7f)
            , density(0.8f)
            , reverbType(0)
            , enableModulation(true)
            , modulationFrequency(1.0f)
            , modulationDepth(0.1f)
        {}
    };

    /**
     * @struct FilterConfig
     * @brief Filter effect configuration
     */
    struct FilterConfig {
        FilterType filterType;                       ///< Filter type
        float cutoffFrequency;                       ///< Cutoff frequency (Hz)
        float resonance;                             ///< Resonance/Q factor
        float gain;                                  ///< Filter gain (dB)

        // Band-pass specific
        float bandwidth;                             ///< Bandwidth (octaves)

        // Advanced settings
        bool enableKeyTracking;                      ///< Enable key tracking
        float keyTracking;                           ///< Key tracking amount
        bool enableLFO;                              ///< Enable LFO modulation
        float lfoFrequency;                          ///< LFO frequency
        float lfoDepth;                              ///< LFO depth

        FilterConfig()
            : filterType(FilterType::Biquad)
            , cutoffFrequency(1000.0f)
            , resonance(1.0f)
            , gain(0.0f)
            , bandwidth(1.0f)
            , enableKeyTracking(false)
            , keyTracking(0.0f)
            , enableLFO(false)
            , lfoFrequency(1.0f)
            , lfoDepth(0.0f)
        {}
    };

    /**
     * @struct DelayConfig
     * @brief Delay effect configuration
     */
    struct DelayConfig {
        float delayTime;                             ///< Delay time (ms)
        float feedback;                              ///< Feedback amount (0.0 - 1.0)
        float mix;                                   ///< Wet/dry mix (0.0 - 1.0)
        bool enablePingPong;                         ///< Enable ping-pong delay
        bool enableFilter;                           ///< Enable delay filter
        float filterCutoff;                          ///< Delay filter cutoff

        // Modulation
        bool enableModulation;                       ///< Enable delay modulation
        float modulationRate;                        ///< Modulation rate (Hz)
        float modulationDepth;                       ///< Modulation depth (ms)

        // Advanced settings
        int maxDelayTime;                            ///< Maximum delay time (ms)
        bool enableSync;                             ///< Enable tempo sync
        float syncDivision;                          ///< Sync division

        DelayConfig()
            : delayTime(500.0f)
            , feedback(0.3f)
            , mix(0.5f)
            , enablePingPong(false)
            , enableFilter(true)
            , filterCutoff(5000.0f)
            , enableModulation(false)
            , modulationRate(1.0f)
            , modulationDepth(10.0f)
            , maxDelayTime(2000)
            , enableSync(false)
            , syncDivision(0.5f)
        {}
    };

    /**
     * @struct DistortionConfig
     * @brief Distortion effect configuration
     */
    struct DistortionConfig {
        float drive;                                 ///< Drive amount (0.0 - 1.0)
        float tone;                                  ///< Tone control (0.0 - 1.0)
        float mix;                                   ///< Wet/dry mix (0.0 - 1.0)
        int distortionType;                          ///< Distortion algorithm type

        // Advanced settings
        float inputGain;                             ///< Input gain
        float outputGain;                            ///< Output gain
        bool enableDCBlocker;                        ///< Enable DC blocker
        bool enableNoiseGate;                        ///< Enable noise gate
        float noiseGateThreshold;                    ///< Noise gate threshold

        DistortionConfig()
            : drive(0.5f)
            , tone(0.5f)
            , mix(0.5f)
            , distortionType(0)
            , inputGain(1.0f)
            , outputGain(1.0f)
            , enableDCBlocker(true)
            , enableNoiseGate(false)
            , noiseGateThreshold(-60.0f)
        {}
    };

    /**
     * @struct AudioEffectStats
     * @brief Audio effect performance statistics
     */
    struct AudioEffectStats {
        // Performance metrics
        uint64_t totalEffectsProcessed;              ///< Total effects processed
        uint64_t activeEffects;                      ///< Currently active effects
        double totalProcessingTime;                  ///< Total processing time (ms)
        double averageProcessingTime;                ///< Average processing time (ms)

        // Resource usage
        size_t memoryUsage;                          ///< Memory usage (bytes)
        size_t bufferMemory;                         ///< Audio buffer memory
        int activeThreads;                           ///< Number of active threads
        float cpuUsage;                              ///< CPU usage percentage

        // Effect-specific stats
        int reverbEffects;                           ///< Number of reverb effects
        int filterEffects;                           ///< Number of filter effects
        int delayEffects;                            ///< Number of delay effects
        int distortionEffects;                       ///< Number of distortion effects

        // Quality metrics
        float averageSignalToNoise;                  ///< Average signal-to-noise ratio
        float averageTHD;                            ///< Average total harmonic distortion
        float averageLatency;                        ///< Average processing latency (ms)
        int droppedFrames;                           ///< Number of dropped frames

        // GPU acceleration stats
        int gpuEffects;                              ///< Number of GPU-accelerated effects
        size_t gpuMemoryUsage;                       ///< GPU memory usage
        float gpuUtilization;                        ///< GPU utilization percentage
    };

    /**
     * @struct AudioEffectInstance
     * @brief Instance of an audio effect
     */
    struct AudioEffectInstance {
        uint32_t instanceId;                         ///< Unique instance ID
        uint32_t effectId;                           ///< Effect ID
        AudioEffectType effectType;                  ///< Effect type
        bool enabled;                                ///< Effect enabled
        float wetLevel;                              ///< Wet level
        float dryLevel;                              ///< Dry level

        // Effect-specific data
        std::any effectData;                         ///< Effect-specific data

        // Parameter automation
        std::unordered_map<std::string, std::function<float(double)>> parameterCurves; ///< Parameter curves

        // Performance data
        double lastProcessTime;                      ///< Last processing time
        size_t samplesProcessed;                     ///< Samples processed

        AudioEffectInstance()
            : instanceId(0)
            , effectId(0)
            , enabled(true)
            , wetLevel(0.5f)
            , dryLevel(0.5f)
            , lastProcessTime(0.0)
            , samplesProcessed(0)
        {}
    };

    /**
     * @class AudioEffect
     * @brief Advanced audio effects processing system
     *
     * The AudioEffect class provides comprehensive audio effects processing for the
     * VoxelCraft game engine, featuring reverb, filters, distortion, modulation effects,
     * spatial effects, and advanced DSP (Digital Signal Processing) with real-time
     * parameter control and GPU acceleration for complex audio effects that enhance
     * the immersive audio experience in games.
     *
     * Key Features:
     * - Wide range of audio effects (reverb, delay, chorus, flanger, phaser)
     * - Advanced filtering (low-pass, high-pass, band-pass, equalizer)
     * - Distortion and overdrive effects with multiple algorithms
     * - Modulation effects (tremolo, vibrato, ring modulation)
     * - Dynamic range processing (compressor, limiter, expander)
     * - Spatial audio effects (panning, spatialization, binaural)
     * - Real-time parameter automation and modulation
     * - GPU acceleration for compute-intensive effects
     * - Multi-threading support for parallel processing
     * - SIMD optimization for vector processing
     * - Low-latency processing with automatic compensation
     * - Effect chaining and complex routing
     * - Performance monitoring and optimization
     * - High-quality algorithms with multiple quality levels
     *
     * The audio effects system is designed to provide professional-grade audio
     * processing capabilities with the performance requirements of real-time gaming.
     */
    class AudioEffect {
    public:
        /**
         * @brief Constructor
         * @param audioEngine Audio engine instance
         */
        explicit AudioEffect(AudioEngine* audioEngine);

        /**
         * @brief Destructor
         */
        ~AudioEffect();

        /**
         * @brief Deleted copy constructor
         */
        AudioEffect(const AudioEffect&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        AudioEffect& operator=(const AudioEffect&) = delete;

        // Audio effect lifecycle

        /**
         * @brief Initialize audio effects system
         * @return true if successful, false otherwise
         */
        bool Initialize();

        /**
         * @brief Shutdown audio effects system
         */
        void Shutdown();

        /**
         * @brief Update audio effects system
         * @param deltaTime Time elapsed since last update
         */
        void Update(double deltaTime);

        /**
         * @brief Get audio effect statistics
         * @return Current statistics
         */
        const AudioEffectStats& GetStats() const { return m_stats; }

        // Effect creation and management

        /**
         * @brief Create audio effect
         * @param config Effect configuration
         * @return Effect ID or 0 if failed
         */
        uint32_t CreateEffect(const AudioEffectConfig& config);

        /**
         * @brief Destroy audio effect
         * @param effectId Effect ID
         * @return true if successful, false otherwise
         */
        bool DestroyEffect(uint32_t effectId);

        /**
         * @brief Get audio effect
         * @param effectId Effect ID
         * @return Effect instance or nullopt if not found
         */
        std::optional<AudioEffectInstance> GetEffect(uint32_t effectId) const;

        /**
         * @brief Enable audio effect
         * @param effectId Effect ID
         * @param enabled Enable state
         * @return true if successful, false otherwise
         */
        bool EnableEffect(uint32_t effectId, bool enabled);

        // Effect parameter control

        /**
         * @brief Set effect parameter
         * @param effectId Effect ID
         * @param parameterName Parameter name
         * @param value Parameter value
         * @return true if successful, false otherwise
         */
        bool SetEffectParameter(uint32_t effectId, const std::string& parameterName, const std::any& value);

        /**
         * @brief Get effect parameter
         * @param effectId Effect ID
         * @param parameterName Parameter name
         * @return Parameter value or nullopt if not found
         */
        std::optional<std::any> GetEffectParameter(uint32_t effectId, const std::string& parameterName) const;

        /**
         * @brief Reset effect parameters to defaults
         * @param effectId Effect ID
         * @return true if successful, false otherwise
         */
        bool ResetEffectParameters(uint32_t effectId);

        // Preset effects

        /**
         * @brief Create reverb effect
         * @param config Reverb configuration
         * @return Effect ID or 0 if failed
         */
        uint32_t CreateReverbEffect(const ReverbConfig& config);

        /**
         * @brief Create filter effect
         * @param config Filter configuration
         * @return Effect ID or 0 if failed
         */
        uint32_t CreateFilterEffect(const FilterConfig& config);

        /**
         * @brief Create delay effect
         * @param config Delay configuration
         * @return Effect ID or 0 if failed
         */
        uint32_t CreateDelayEffect(const DelayConfig& config);

        /**
         * @brief Create distortion effect
         * @param config Distortion configuration
         * @return Effect ID or 0 if failed
         */
        uint32_t CreateDistortionEffect(const DistortionConfig& config);

        // Effect processing

        /**
         * @brief Process audio through effect
         * @param effectId Effect ID
         * @param inputBuffer Input audio buffer
         * @param outputBuffer Output audio buffer
         * @param numSamples Number of samples to process
         * @return true if successful, false otherwise
         */
        bool ProcessEffect(uint32_t effectId, const float* inputBuffer, float* outputBuffer, size_t numSamples);

        /**
         * @brief Process audio through effect chain
         * @param effectIds Effect IDs in chain order
         * @param inputBuffer Input audio buffer
         * @param outputBuffer Output audio buffer
         * @param numSamples Number of samples to process
         * @return true if successful, false otherwise
         */
        bool ProcessEffectChain(const std::vector<uint32_t>& effectIds,
                              const float* inputBuffer, float* outputBuffer, size_t numSamples);

        // Effect chaining and routing

        /**
         * @brief Create effect chain
         * @param name Chain name
         * @param effectIds Effect IDs in chain
         * @return Chain ID or 0 if failed
         */
        uint32_t CreateEffectChain(const std::string& name, const std::vector<uint32_t>& effectIds);

        /**
         * @brief Destroy effect chain
         * @param chainId Chain ID
         * @return true if successful, false otherwise
         */
        bool DestroyEffectChain(uint32_t chainId);

        /**
         * @brief Process audio through effect chain
         * @param chainId Chain ID
         * @param inputBuffer Input audio buffer
         * @param outputBuffer Output audio buffer
         * @param numSamples Number of samples to process
         * @return true if successful, false otherwise
         */
        bool ProcessEffectChain(uint32_t chainId, const float* inputBuffer, float* outputBuffer, size_t numSamples);

        // Parameter automation

        /**
         * @brief Add parameter automation
         * @param effectId Effect ID
         * @param parameterName Parameter name
         * @param curve Automation curve function
         * @return true if successful, false otherwise
         */
        bool AddParameterAutomation(uint32_t effectId, const std::string& parameterName,
                                  std::function<float(double)> curve);

        /**
         * @brief Remove parameter automation
         * @param effectId Effect ID
         * @param parameterName Parameter name
         * @return true if successful, false otherwise
         */
        bool RemoveParameterAutomation(uint32_t effectId, const std::string& parameterName);

        /**
         * @brief Update parameter automation
         * @param effectId Effect ID
         * @param time Current time
         * @return true if successful, false otherwise
         */
        bool UpdateParameterAutomation(uint32_t effectId, double time);

        // Effect presets

        /**
         * @brief Load effect preset
         * @param effectId Effect ID
         * @param presetName Preset name
         * @return true if successful, false otherwise
         */
        bool LoadEffectPreset(uint32_t effectId, const std::string& presetName);

        /**
         * @brief Save effect preset
         * @param effectId Effect ID
         * @param presetName Preset name
         * @return true if successful, false otherwise
         */
        bool SaveEffectPreset(uint32_t effectId, const std::string& presetName);

        /**
         * @brief Get available presets for effect type
         * @param effectType Effect type
         * @return Vector of preset names
         */
        std::vector<std::string> GetEffectPresets(AudioEffectType effectType) const;

        // Advanced processing

        /**
         * @brief Enable GPU acceleration
         * @param enabled Enable state
         * @return true if successful, false otherwise
         */
        bool EnableGPUAcceleration(bool enabled);

        /**
         * @brief Set processing quality
         * @param quality Processing quality
         * @return true if successful, false otherwise
         */
        bool SetProcessingQuality(EffectQuality quality);

        /**
         * @brief Set multithreading
         * @param enabled Enable state
         * @param threadCount Number of threads
         * @return true if successful, false otherwise
         */
        bool SetMultithreading(bool enabled, int threadCount = 4);

        // Utility functions

        /**
         * @brief Get effect type name
         * @param effectType Effect type
         * @return Effect type name
         */
        std::string GetEffectTypeName(AudioEffectType effectType) const;

        /**
         * @brief Get supported effect types
         * @return Vector of supported effect types
         */
        std::vector<AudioEffectType> GetSupportedEffectTypes() const;

        /**
         * @brief Check if effect type is supported
         * @param effectType Effect type
         * @return true if supported, false otherwise
         */
        bool IsEffectTypeSupported(AudioEffectType effectType) const;

        /**
         * @brief Get effect processing latency
         * @param effectId Effect ID
         * @return Processing latency in milliseconds
         */
        float GetEffectLatency(uint32_t effectId) const;

        /**
         * @brief Validate audio effect system
         * @return true if valid, false otherwise
         */
        bool Validate() const;

        /**
         * @brief Get audio effect status report
         * @return Detailed status report
         */
        std::string GetStatusReport() const;

        /**
         * @brief Get performance report
         * @return Detailed performance report
         */
        std::string GetPerformanceReport() const;

        /**
         * @brief Optimize audio effect system
         * @return Optimization results
         */
        std::unordered_map<std::string, size_t> Optimize();

    private:
        /**
         * @brief Initialize effect processors
         * @return true if successful, false otherwise
         */
        bool InitializeEffectProcessors();

        /**
         * @brief Process reverb effect
         * @param instance Effect instance
         * @param inputBuffer Input buffer
         * @param outputBuffer Output buffer
         * @param numSamples Number of samples
         * @return true if successful, false otherwise
         */
        bool ProcessReverb(AudioEffectInstance& instance, const float* inputBuffer,
                         float* outputBuffer, size_t numSamples);

        /**
         * @brief Process filter effect
         * @param instance Effect instance
         * @param inputBuffer Input buffer
         * @param outputBuffer Output buffer
         * @param numSamples Number of samples
         * @return true if successful, false otherwise
         */
        bool ProcessFilter(AudioEffectInstance& instance, const float* inputBuffer,
                         float* outputBuffer, size_t numSamples);

        /**
         * @brief Process delay effect
         * @param instance Effect instance
         * @param inputBuffer Input buffer
         * @param outputBuffer Output buffer
         * @param numSamples Number of samples
         * @return true if successful, false otherwise
         */
        bool ProcessDelay(AudioEffectInstance& instance, const float* inputBuffer,
                        float* outputBuffer, size_t numSamples);

        /**
         * @brief Process distortion effect
         * @param instance Effect instance
         * @param inputBuffer Input buffer
         * @param outputBuffer Output buffer
         * @param numSamples Number of samples
         * @return true if successful, false otherwise
         */
        bool ProcessDistortion(AudioEffectInstance& instance, const float* inputBuffer,
                             float* outputBuffer, size_t numSamples);

        /**
         * @brief Create reverb processor
         * @param config Reverb configuration
         * @return Reverb processor data
         */
        std::any CreateReverbProcessor(const ReverbConfig& config);

        /**
         * @brief Create filter processor
         * @param config Filter configuration
         * @return Filter processor data
         */
        std::any CreateFilterProcessor(const FilterConfig& config);

        /**
         * @brief Create delay processor
         * @param config Delay configuration
         * @return Delay processor data
         */
        std::any CreateDelayProcessor(const DelayConfig& config);

        /**
         * @brief Create distortion processor
         * @param config Distortion configuration
         * @return Distortion processor data
         */
        std::any CreateDistortionProcessor(const DistortionConfig& config);

        /**
         * @brief Update effect automation
         * @param deltaTime Time elapsed
         */
        void UpdateEffectAutomation(double deltaTime);

        /**
         * @brief Update performance statistics
         * @param deltaTime Time elapsed
         */
        void UpdateStats(double deltaTime);

        /**
         * @brief Handle audio effect errors
         * @param error Error message
         */
        void HandleError(const std::string& error);

        // Audio effect data
        AudioEngine* m_audioEngine;                    ///< Audio engine instance
        AudioEffectStats m_stats;                      ///< Performance statistics

        // Effect instances
        std::unordered_map<uint32_t, AudioEffectInstance> m_effectInstances; ///< Effect instances
        std::unordered_map<uint32_t, std::vector<uint32_t>> m_effectChains; ///< Effect chains
        mutable std::shared_mutex m_effectsMutex;      ///< Effects synchronization

        // Effect processors
        std::unordered_map<uint32_t, std::any> m_effectProcessors; ///< Effect processors
        mutable std::shared_mutex m_processorsMutex;   ///< Processors synchronization

        // Effect presets
        std::unordered_map<AudioEffectType, std::vector<std::string>> m_effectPresets; ///< Effect presets
        mutable std::shared_mutex m_presetsMutex;      ///< Presets synchronization

        // Processing state
        bool m_gpuAccelerationEnabled;                 ///< GPU acceleration enabled
        bool m_multithreadingEnabled;                  ///< Multithreading enabled
        int m_threadCount;                             ///< Number of processing threads
        EffectQuality m_processingQuality;             ///< Current processing quality

        // Audio buffers
        std::vector<float> m_inputBuffer;              ///< Input audio buffer
        std::vector<float> m_outputBuffer;             ///< Output audio buffer
        std::vector<float> m_tempBuffer;               ///< Temporary processing buffer

        // State tracking
        bool m_isInitialized;                          ///< System is initialized
        double m_lastUpdateTime;                       ///< Last update time
        std::string m_lastError;                       ///< Last error message

        static std::atomic<uint32_t> s_nextEffectId;   ///< Next effect ID counter
        static std::atomic<uint32_t> s_nextChainId;    ///< Next chain ID counter
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_AUDIO_AUDIO_EFFECT_HPP
