/**
 * @file AudioEffectProcessor.hpp
 * @brief VoxelCraft Audio Effect Processing System
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_AUDIO_EFFECT_PROCESSOR_HPP
#define VOXELCRAFT_AUDIO_EFFECT_PROCESSOR_HPP

#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <functional>
#include <mutex>
#include <atomic>

namespace VoxelCraft {

    /**
     * @enum AudioEffectType
     * @brief Types of audio effects available
     */
    enum class AudioEffectType {
        REVERB,                    ///< Reverb effect
        ECHO,                      ///< Echo/delay effect
        CHORUS,                    ///< Chorus effect
        FLANGER,                   ///< Flanger effect
        PHASER,                    ///< Phaser effect
        DISTORTION,                ///< Distortion effect
        COMPRESSION,               ///< Dynamic compression
        LIMITER,                   ///< Peak limiter
        EQUALIZER,                 ///< Multi-band equalizer
        LOW_PASS_FILTER,           ///< Low-pass filter
        HIGH_PASS_FILTER,          ///< High-pass filter
        BAND_PASS_FILTER,          ///< Band-pass filter
        NOTCH_FILTER,              ///< Notch filter
        PITCH_SHIFTER,             ///< Pitch shifting
        TIME_STRETCHER,            ///< Time stretching
        HARMONIZER,                ///< Harmonic generation
        VOCODER,                   ///< Vocoder effect
        AUTOTUNE,                  ///< Auto-tune effect
        RING_MODULATOR,            ///< Ring modulation
        GRANULAR_SYNTH,            ///< Granular synthesis
        CONVOLUTION_REVERB        ///< Convolution reverb
    };

    /**
     * @struct AudioEffectConfig
     * @brief Configuration for audio effects
     */
    struct AudioEffectConfig {
        AudioEffectType type;                     ///< Effect type
        bool enabled = true;                      ///< Effect enabled flag
        float wetDryMix = 1.0f;                   ///< Wet/dry mix (0.0 = dry, 1.0 = wet)
        float intensity = 1.0f;                   ///< Effect intensity
        std::unordered_map<std::string, float> parameters; ///< Effect-specific parameters
    };

    /**
     * @struct ReverbConfig
     * @brief Reverb effect configuration
     */
    struct ReverbConfig {
        float roomSize = 0.5f;                    ///< Room size (0.0 - 1.0)
        float damping = 0.5f;                     ///< Damping factor (0.0 - 1.0)
        float width = 1.0f;                       ///< Stereo width (0.0 - 1.0)
        float wetLevel = 0.3f;                    ///< Wet signal level
        float dryLevel = 1.0f;                    ///< Dry signal level
        float preDelay = 0.01f;                   ///< Pre-delay in seconds
        float lateDelay = 0.02f;                  ///< Late delay in seconds
    };

    /**
     * @struct EchoConfig
     * @brief Echo/delay effect configuration
     */
    struct EchoConfig {
        float delayTime = 0.3f;                   ///< Delay time in seconds
        float feedback = 0.4f;                    ///< Feedback amount (0.0 - 1.0)
        float wetLevel = 0.5f;                    ///< Wet signal level
        float dryLevel = 1.0f;                    ///< Dry signal level
        bool pingPong = false;                    ///< Ping-pong mode
    };

    /**
     * @struct FilterConfig
     * @brief Filter effect configuration
     */
    struct FilterConfig {
        float cutoffFrequency = 1000.0f;          ///< Cutoff frequency in Hz
        float resonance = 1.0f;                   ///< Resonance/Q factor
        float slope = 12.0f;                      ///< Filter slope in dB/octave
        bool enabled = true;                      ///< Filter enabled
    };

    /**
     * @struct CompressorConfig
     * @brief Compressor/limiter configuration
     */
    struct CompressorConfig {
        float threshold = -20.0f;                 ///< Threshold in dB
        float ratio = 4.0f;                       ///< Compression ratio
        float attackTime = 0.01f;                 ///< Attack time in seconds
        float releaseTime = 0.1f;                 ///< Release time in seconds
        float makeupGain = 0.0f;                  ///< Makeup gain in dB
    };

    /**
     * @class AudioEffect
     * @brief Base class for audio effects
     */
    class AudioEffect {
    public:
        AudioEffect(AudioEffectType type, const std::string& name);
        virtual ~AudioEffect() = default;

        /**
         * @brief Get effect type
         * @return Effect type
         */
        AudioEffectType GetType() const { return m_type; }

        /**
         * @brief Get effect name
         * @return Effect name
         */
        const std::string& GetName() const { return m_name; }

        /**
         * @brief Check if effect is enabled
         * @return true if enabled
         */
        bool IsEnabled() const { return m_enabled; }

        /**
         * @brief Set enabled state
         * @param enabled Enabled state
         */
        void SetEnabled(bool enabled) { m_enabled = enabled; }

        /**
         * @brief Get wet/dry mix
         * @return Wet/dry mix (0.0 - 1.0)
         */
        float GetWetDryMix() const { return m_wetDryMix; }

        /**
         * @brief Set wet/dry mix
         * @param mix Wet/dry mix (0.0 - 1.0)
         */
        void SetWetDryMix(float mix) { m_wetDryMix = std::max(0.0f, std::min(1.0f, mix)); }

        /**
         * @brief Process audio through the effect
         * @param input Input audio buffer
         * @param output Output audio buffer
         * @param sampleRate Sample rate
         * @return true if processing successful
         */
        virtual bool ProcessAudio(const std::vector<float>& input,
                                std::vector<float>& output,
                                int sampleRate) = 0;

        /**
         * @brief Reset effect state
         */
        virtual void Reset();

        /**
         * @brief Get effect latency in samples
         * @return Latency in samples
         */
        virtual int GetLatency() const { return 0; }

        /**
         * @brief Get effect tail length in samples
         * @return Tail length in samples
         */
        virtual int GetTailLength() const { return 0; }

        /**
         * @brief Set effect parameter
         * @param name Parameter name
         * @param value Parameter value
         * @return true if parameter set successfully
         */
        virtual bool SetParameter(const std::string& name, float value);

        /**
         * @brief Get effect parameter
         * @param name Parameter name
         * @return Parameter value or 0.0 if not found
         */
        virtual float GetParameter(const std::string& name) const;

    protected:
        AudioEffectType m_type;                     ///< Effect type
        std::string m_name;                         ///< Effect name
        bool m_enabled;                             ///< Enabled flag
        float m_wetDryMix;                          ///< Wet/dry mix
        std::unordered_map<std::string, float> m_parameters; ///< Effect parameters
    };

    /**
     * @class ReverbEffect
     * @brief Reverb audio effect
     */
    class ReverbEffect : public AudioEffect {
    public:
        ReverbEffect();
        ~ReverbEffect() override;

        bool ProcessAudio(const std::vector<float>& input,
                        std::vector<float>& output,
                        int sampleRate) override;

        void Reset() override;
        int GetLatency() const override { return 1024; }
        int GetTailLength() const override { return static_cast<int>(m_config.roomSize * 44100); }

        bool SetParameter(const std::string& name, float value) override;
        float GetParameter(const std::string& name) const override;

        void SetConfig(const ReverbConfig& config) { m_config = config; }
        const ReverbConfig& GetConfig() const { return m_config; }

    private:
        ReverbConfig m_config;                       ///< Reverb configuration
        std::vector<float> m_delayBuffer;            ///< Delay buffer for reverb
        size_t m_bufferIndex;                        ///< Current buffer index
        float m_previousOutput[2];                   ///< Previous output samples
    };

    /**
     * @class EchoEffect
     * @brief Echo/delay audio effect
     */
    class EchoEffect : public AudioEffect {
    public:
        EchoEffect();
        ~EchoEffect() override;

        bool ProcessAudio(const std::vector<float>& input,
                        std::vector<float>& output,
                        int sampleRate) override;

        void Reset() override;
        int GetLatency() const override { return static_cast<int>(m_config.delayTime * 44100); }
        int GetTailLength() const override { return static_cast<int>(m_config.delayTime * 44100 * 4); }

        bool SetParameter(const std::string& name, float value) override;
        float GetParameter(const std::string& name) const override;

        void SetConfig(const EchoConfig& config) { m_config = config; }
        const EchoConfig& GetConfig() const { return m_config; }

    private:
        EchoConfig m_config;                         ///< Echo configuration
        std::vector<float> m_delayBuffer;            ///< Delay buffer
        size_t m_delayIndex;                         ///< Delay buffer index
    };

    /**
     * @class FilterEffect
     * @brief Multi-type filter effect
     */
    class FilterEffect : public AudioEffect {
    public:
        FilterEffect(AudioEffectType filterType);
        ~FilterEffect() override;

        bool ProcessAudio(const std::vector<float>& input,
                        std::vector<float>& output,
                        int sampleRate) override;

        void Reset() override;
        int GetLatency() const override { return 2; } // Biquad filter delay

        bool SetParameter(const std::string& name, float value) override;
        float GetParameter(const std::string& name) const override;

        void SetConfig(const FilterConfig& config) { m_config = config; }
        const FilterConfig& GetConfig() const { return m_config; }

    private:
        void UpdateFilterCoefficients(int sampleRate);

        FilterConfig m_config;                       ///< Filter configuration
        float m_x1, m_x2, m_y1, m_y2;                ///< Filter state variables
        float m_a0, m_a1, m_a2, m_b1, m_b2;         ///< Filter coefficients
    };

    /**
     * @class CompressorEffect
     * @brief Dynamic range compressor effect
     */
    class CompressorEffect : public AudioEffect {
    public:
        CompressorEffect();
        ~CompressorEffect() override;

        bool ProcessAudio(const std::vector<float>& input,
                        std::vector<float>& output,
                        int sampleRate) override;

        void Reset() override;
        int GetLatency() const override { return 1; }

        bool SetParameter(const std::string& name, float value) override;
        float GetParameter(const std::string& name) const override;

        void SetConfig(const CompressorConfig& config) { m_config = config; }
        const CompressorConfig& GetConfig() const { return m_config; }

    private:
        CompressorConfig m_config;                   ///< Compressor configuration
        float m_envelope;                            ///< Current envelope level
        float m_gainReduction;                       ///< Current gain reduction
    };

    /**
     * @class AudioEffectProcessor
     * @brief Main audio effect processing system
     */
    class AudioEffectProcessor {
    public:
        AudioEffectProcessor();
        ~AudioEffectProcessor();

        /**
         * @brief Initialize the effect processor
         * @return true if successful
         */
        bool Initialize();

        /**
         * @brief Shutdown the effect processor
         */
        void Shutdown();

        /**
         * @brief Create a new audio effect
         * @param type Effect type
         * @param name Effect name
         * @return Effect instance or nullptr if failed
         */
        std::shared_ptr<AudioEffect> CreateEffect(AudioEffectType type, const std::string& name);

        /**
         * @brief Destroy an audio effect
         * @param effect Effect to destroy
         */
        void DestroyEffect(std::shared_ptr<AudioEffect> effect);

        /**
         * @brief Process audio through a chain of effects
         * @param input Input audio buffer
         * @param effects Chain of effects to apply
         * @param sampleRate Sample rate
         * @return Processed audio buffer
         */
        std::vector<float> ProcessEffectChain(const std::vector<float>& input,
                                            const std::vector<std::shared_ptr<AudioEffect>>& effects,
                                            int sampleRate);

        /**
         * @brief Process audio through a single effect
         * @param input Input audio buffer
         * @param effect Effect to apply
         * @param sampleRate Sample rate
         * @return Processed audio buffer
         */
        std::vector<float> ProcessEffect(const std::vector<float>& input,
                                       std::shared_ptr<AudioEffect> effect,
                                       int sampleRate);

        /**
         * @brief Mix multiple audio signals
         * @param signals Vector of audio signals
         * @param levels Vector of mix levels
         * @return Mixed audio signal
         */
        std::vector<float> MixSignals(const std::vector<std::vector<float>>& signals,
                                   const std::vector<float>& levels);

        /**
         * @brief Get effect by name
         * @param name Effect name
         * @return Effect instance or nullptr if not found
         */
        std::shared_ptr<AudioEffect> GetEffect(const std::string& name);

        /**
         * @brief Get all active effects
         * @return Vector of active effects
         */
        std::vector<std::shared_ptr<AudioEffect>> GetAllEffects() const;

        /**
         * @brief Clear all effects
         */
        void ClearAllEffects();

        /**
         * @brief Get processor statistics
         * @return Statistics information
         */
        std::unordered_map<std::string, size_t> GetStatistics() const;

    private:
        std::unordered_map<std::string, std::shared_ptr<AudioEffect>> m_effects;
        mutable std::shared_mutex m_effectsMutex;

        // Statistics
        std::atomic<size_t> m_totalEffectsCreated;
        std::atomic<size_t> m_totalAudioProcessed;
        std::atomic<size_t> m_totalProcessingTime;
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_AUDIO_EFFECT_PROCESSOR_HPP
