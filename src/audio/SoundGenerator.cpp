/**
 * @file SoundGenerator.cpp
 * @brief VoxelCraft Procedural Sound Generator Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#include "SoundGenerator.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <numeric>

namespace VoxelCraft {

    SoundGenerator::SoundGenerator()
        : m_randomEngine(std::random_device()())
        , m_randomFloat(0.0f, 1.0f)
    {
    }

    SoundGenerator::~SoundGenerator() {
        Shutdown();
    }

    bool SoundGenerator::Initialize() {
        if (m_initialized) {
            return true;
        }

        std::cout << "Initializing SoundGenerator..." << std::endl;

        // Initialize default sound parameters
        InitializeSoundParameters();

        // Initialize biome profiles
        InitializeBiomeProfiles();

        m_initialized = true;
        std::cout << "SoundGenerator initialized successfully" << std::endl;
        return true;
    }

    void SoundGenerator::Shutdown() {
        if (!m_initialized) {
            return;
        }

        // Clear cache
        m_soundCache.clear();
        m_currentCacheSize = 0;

        m_initialized = false;
        std::cout << "SoundGenerator shutdown complete" << std::endl;
    }

    void SoundGenerator::InitializeSoundParameters() {
        // Initialize default parameters for each sound type
        m_soundParameters[ProceduralSoundType::BLOCK_BREAK] = {
            300.0f, 0.3f, 1.0f, 0.01f, 0.05f, 0.0f, 0.1f,
            {1.0f, 0.5f, 0.3f, 0.2f}, 0.2f, true
        };

        m_soundParameters[ProceduralSoundType::BLOCK_PLACE] = {
            400.0f, 0.2f, 1.0f, 0.005f, 0.02f, 0.0f, 0.05f,
            {1.0f, 0.3f, 0.1f}, 0.1f, true
        };

        m_soundParameters[ProceduralSoundType::PLAYER_WALK] = {
            150.0f, 0.15f, 0.8f, 0.01f, 0.02f, 0.0f, 0.03f,
            {1.0f, 0.4f, 0.2f}, 0.3f, true
        };

        m_soundParameters[ProceduralSoundType::PLAYER_JUMP] = {
            250.0f, 0.25f, 1.0f, 0.01f, 0.05f, 0.0f, 0.1f,
            {1.0f, 0.6f, 0.4f, 0.2f}, 0.15f, true
        };

        m_soundParameters[ProceduralSoundType::ENTITY_HURT] = {
            180.0f, 0.4f, 0.9f, 0.02f, 0.1f, 0.0f, 0.2f,
            {1.0f, 0.7f, 0.5f, 0.3f}, 0.4f, true
        };

        m_soundParameters[ProceduralSoundType::ENTITY_DEATH] = {
            120.0f, 0.8f, 0.7f, 0.05f, 0.3f, 0.0f, 0.4f,
            {1.0f, 0.5f, 0.3f, 0.2f, 0.1f}, 0.5f, true
        };

        m_soundParameters[ProceduralSoundType::AMBIENT_CAVE] = {
            80.0f, 30.0f, 0.3f, 2.0f, 5.0f, 0.8f, 3.0f,
            {1.0f}, 0.8f, true
        };

        m_soundParameters[ProceduralSoundType::WEATHER_RAIN] = {
            200.0f, 10.0f, 0.6f, 1.0f, 2.0f, 0.9f, 1.0f,
            {1.0f}, 0.9f, true
        };
    }

    void SoundGenerator::InitializeBiomeProfiles() {
        // Forest biome
        m_biomeProfiles["forest"] = {
            "forest", 220.0f, 0.7f,
            {"birds", "wind", "leaves", "insects"},
            0.3f, 0.1f
        };

        // Desert biome
        m_biomeProfiles["desert"] = {
            "desert", 180.0f, 0.4f,
            {"wind", "sand", "heat"},
            0.1f, 0.2f
        };

        // Cave biome
        m_biomeProfiles["cave"] = {
            "cave", 100.0f, 0.8f,
            {"drips", "echoes", "minerals"},
            0.9f, 0.7f
        };

        // Ocean biome
        m_biomeProfiles["ocean"] = {
            "ocean", 150.0f, 0.6f,
            {"waves", "bubbles", "marine_life"},
            0.4f, 0.3f
        };

        // Nether biome
        m_biomeProfiles["nether"] = {
            "nether", 300.0f, 0.9f,
            {"lava", "souls", "fire", "growls"},
            0.6f, 0.4f
        };

        // End biome
        m_biomeProfiles["end"] = {
            "end", 400.0f, 0.5f,
            {"void", "dragons", "portals"},
            0.8f, 0.6f
        };
    }

    std::vector<short> SoundGenerator::GenerateSound(ProceduralSoundType type,
                                                   unsigned int seed,
                                                   const SoundParameters& parameters) {
        if (!m_initialized) {
            return {};
        }

        // Set seed for reproducible results
        if (seed != 0) {
            m_randomEngine.seed(seed);
        }

        // Get base parameters
        SoundParameters params = parameters;
        auto it = m_soundParameters.find(type);
        if (it != m_soundParameters.end()) {
            // Merge with default parameters
            const auto& defaultParams = it->second;
            if (params.frequency == 440.0f) params.frequency = defaultParams.frequency;
            if (params.duration == 1.0f) params.duration = defaultParams.duration;
            if (params.attack == 0.01f) params.attack = defaultParams.attack;
            if (params.decay == 0.1f) params.decay = defaultParams.decay;
            if (params.sustain == 0.7f) params.sustain = defaultParams.sustain;
            if (params.release == 0.2f) params.release = defaultParams.release;
            if (params.harmonics.empty()) params.harmonics = defaultParams.harmonics;
            if (params.noiseAmount == 0.0f) params.noiseAmount = defaultParams.noiseAmount;
        }

        // Add some randomness for variation
        params.frequency *= 0.9f + 0.2f * m_randomFloat(m_randomEngine);

        // Generate base waveform
        std::vector<float> soundData = GenerateWaveform(params.frequency, params.duration, "sine");

        // Apply ADSR envelope if enabled
        if (params.useEnvelope) {
            ApplyADSREnvelope(soundData, params.attack, params.decay, params.sustain, params.release);
        }

        // Add harmonics
        if (!params.harmonics.empty()) {
            AddHarmonics(soundData, params.harmonics);
        }

        // Add noise
        if (params.noiseAmount > 0.0f) {
            AddNoise(soundData, params.noiseAmount);
        }

        // Apply volume
        for (auto& sample : soundData) {
            sample *= params.volume;
        }

        return ConvertToPCM16(soundData);
    }

    std::vector<short> SoundGenerator::GenerateBiomeAmbient(const std::string& biomeName,
                                                          float duration,
                                                          unsigned int seed) {
        if (seed != 0) {
            m_randomEngine.seed(seed);
        }

        auto it = m_biomeProfiles.find(biomeName);
        if (it == m_biomeProfiles.end()) {
            // Default ambient sound
            return GenerateSound(ProceduralSoundType::AMBIENT_CAVE,
                               seed,
                               {100.0f, duration, 0.4f});
        }

        const auto& profile = it->second;

        // Generate layered ambient sound
        std::vector<std::vector<short>> layers;
        std::vector<float> volumes;

        // Base layer
        SoundParameters baseParams = {profile.baseFrequency, duration, 0.3f};
        layers.push_back(GenerateSound(ProceduralSoundType::PROCEDURAL_AMBIENT_PAD, seed, baseParams));
        volumes.push_back(0.6f);

        // Additional layers based on complexity
        int numLayers = static_cast<int>(profile.complexity * 5) + 1;
        for (int i = 0; i < numLayers; ++i) {
            float freq = profile.baseFrequency * (0.5f + m_randomFloat(m_randomEngine));
            SoundParameters layerParams = {freq, duration, 0.2f};
            layers.push_back(GenerateSound(ProceduralSoundType::PROCEDURAL_DRONE, seed + i, layerParams));
            volumes.push_back(0.2f / (i + 1));
        }

        return MixSounds(layers, volumes);
    }

    std::vector<short> SoundGenerator::GenerateWeatherSound(const std::string& weatherType,
                                                          float intensity,
                                                          float duration) {
        if (weatherType == "rain") {
            SoundParameters params = {200.0f, duration, intensity * 0.8f};
            params.noiseAmount = 0.9f;
            return GenerateSound(ProceduralSoundType::WEATHER_RAIN, 0, params);
        } else if (weatherType == "thunder") {
            // Generate thunder with low frequency rumble
            SoundParameters params = {50.0f, duration, intensity, 2.0f, 3.0f, 0.0f, 5.0f};
            params.noiseAmount = 0.7f;
            return GenerateSound(ProceduralSoundType::PROCEDURAL_BROWN_NOISE, 0, params);
        } else if (weatherType == "wind") {
            SoundParameters params = {150.0f, duration, intensity * 0.6f};
            params.noiseAmount = 0.8f;
            return GenerateSound(ProceduralSoundType::PROCEDURAL_PINK_NOISE, 0, params);
        }

        // Default weather sound
        return GenerateSound(ProceduralSoundType::PROCEDURAL_WHITE_NOISE,
                           0,
                           {200.0f, duration, intensity * 0.5f});
    }

    std::vector<short> SoundGenerator::GenerateToolSound(const std::string& toolType,
                                                       const std::string& materialType,
                                                       const std::string& action) {
        float baseFreq = 300.0f;
        float hardness = 1.0f;

        // Adjust frequency based on tool type
        if (toolType == "pickaxe") baseFreq = 400.0f;
        else if (toolType == "axe") baseFreq = 350.0f;
        else if (toolType == "shovel") baseFreq = 250.0f;
        else if (toolType == "sword") baseFreq = 450.0f;

        // Adjust based on material
        if (materialType == "stone") hardness = 1.5f;
        else if (materialType == "wood") hardness = 0.8f;
        else if (materialType == "metal") hardness = 2.0f;
        else if (materialType == "dirt") hardness = 0.6f;

        baseFreq *= hardness;

        SoundParameters params = {baseFreq, 0.3f, 1.0f, 0.01f, 0.05f, 0.0f, 0.1f};
        params.harmonics = {1.0f, 0.5f, 0.3f, 0.2f};
        params.noiseAmount = 0.3f;

        return GenerateSound(ProceduralSoundType::TOOL_DIG, 0, params);
    }

    std::vector<short> SoundGenerator::GenerateEntitySound(const std::string& entityType,
                                                         const std::string& soundType,
                                                         float size) {
        ProceduralSoundType soundEnum = ProceduralSoundType::ENTITY_AMBIENT;
        float baseFreq = 220.0f;

        // Adjust frequency based on entity type
        if (entityType == "zombie") {
            baseFreq = 150.0f;
            soundEnum = ProceduralSoundType::ENTITY_HURT;
        } else if (entityType == "skeleton") {
            baseFreq = 180.0f;
            soundEnum = ProceduralSoundType::ENTITY_ATTACK;
        } else if (entityType == "spider") {
            baseFreq = 300.0f;
            soundEnum = ProceduralSoundType::ENTITY_AMBIENT;
        } else if (entityType == "creeper") {
            baseFreq = 100.0f;
            soundEnum = ProceduralSoundType::ENTITY_HURT;
        }

        // Adjust frequency based on size
        baseFreq *= size;

        SoundParameters params = {baseFreq, 1.0f, 1.0f};
        return GenerateSound(soundEnum, 0, params);
    }

    std::vector<short> SoundGenerator::GenerateBlockSound(const std::string& blockType,
                                                        const std::string& action,
                                                        float hardness) {
        ProceduralSoundType soundEnum = ProceduralSoundType::BLOCK_PLACE;
        float baseFreq = 300.0f;

        if (action == "break") {
            soundEnum = ProceduralSoundType::BLOCK_BREAK;
            baseFreq = 200.0f + hardness * 100.0f;
        } else if (action == "place") {
            soundEnum = ProceduralSoundType::BLOCK_PLACE;
            baseFreq = 400.0f;
        } else if (action == "step") {
            soundEnum = ProceduralSoundType::BLOCK_STEP;
            baseFreq = 150.0f;
        }

        // Adjust frequency based on block type
        if (blockType == "stone") baseFreq *= 1.2f;
        else if (blockType == "wood") baseFreq *= 0.9f;
        else if (blockType == "glass") baseFreq *= 1.5f;
        else if (blockType == "metal") baseFreq *= 1.8f;

        SoundParameters params = {baseFreq, 0.3f, 1.0f};
        params.noiseAmount = hardness * 0.2f;

        return GenerateSound(soundEnum, 0, params);
    }

    std::vector<short> SoundGenerator::GenerateMusic(const std::string& style,
                                                   float duration,
                                                   unsigned int seed) {
        if (seed != 0) {
            m_randomEngine.seed(seed);
        }

        // Generate a simple melody based on style
        std::vector<float> notes;
        if (style == "overworld") {
            notes = {261.63f, 293.66f, 329.63f, 349.23f, 392.00f, 440.00f, 493.88f, 523.25f};
        } else if (style == "nether") {
            notes = {146.83f, 174.61f, 220.00f, 246.94f, 293.66f, 349.23f, 440.00f, 523.25f};
        } else if (style == "end") {
            notes = {329.63f, 392.00f, 493.88f, 523.25f, 659.25f, 783.99f, 987.77f, 1046.50f};
        } else {
            // Default scale
            notes = {261.63f, 293.66f, 329.63f, 349.23f, 392.00f, 440.00f, 493.88f, 523.25f};
        }

        const float noteDuration = 0.5f;
        const int samplesPerNote = static_cast<int>(m_sampleRate * noteDuration);
        const int totalSamples = static_cast<int>(m_sampleRate * duration);

        std::vector<float> musicData(totalSamples, 0.0f);

        for (int i = 0; i < totalSamples; i += samplesPerNote) {
            int noteIndex = (i / samplesPerNote) % notes.size();
            float freq = notes[noteIndex];

            for (int j = 0; j < samplesPerNote && i + j < totalSamples; ++j) {
                float t = static_cast<float>(j) / m_sampleRate;
                float sample = std::sin(2.0f * 3.14159f * freq * t) * 0.3f;

                // Add harmonics
                sample += std::sin(2.0f * 3.14159f * freq * 2 * t) * 0.1f;
                sample += std::sin(2.0f * 3.14159f * freq * 3 * t) * 0.05f;

                // Apply envelope
                float envelope = 1.0f;
                if (j < samplesPerNote * 0.1f) { // Attack
                    envelope = static_cast<float>(j) / (samplesPerNote * 0.1f);
                } else if (j > samplesPerNote * 0.8f) { // Release
                    envelope = 1.0f - (static_cast<float>(j - samplesPerNote * 0.8f) / (samplesPerNote * 0.2f));
                }

                musicData[i + j] += sample * envelope;
            }
        }

        return ConvertToPCM16(musicData);
    }

    std::vector<short> SoundGenerator::ApplyEffect(const std::vector<short>& soundData,
                                                 const std::string& effectType,
                                                 float intensity) {
        // Convert to float for processing
        std::vector<float> floatData(soundData.size());
        for (size_t i = 0; i < soundData.size(); ++i) {
            floatData[i] = static_cast<float>(soundData[i]) / 32767.0f;
        }

        if (effectType == "lowpass") {
            ApplyLowPassFilter(floatData, 1000.0f * intensity, m_sampleRate);
        } else if (effectType == "highpass") {
            ApplyHighPassFilter(floatData, 100.0f * intensity, m_sampleRate);
        } else if (effectType == "reverb") {
            // Simple reverb effect
            std::vector<float> reverbBuffer = floatData;
            for (size_t i = m_sampleRate / 10; i < floatData.size(); ++i) {
                floatData[i] += reverbBuffer[i - m_sampleRate / 10] * intensity * 0.3f;
            }
        }

        return ConvertToPCM16(floatData);
    }

    std::vector<short> SoundGenerator::MixSounds(const std::vector<std::vector<short>>& sounds,
                                               const std::vector<float>& volumes) {
        if (sounds.empty()) return {};

        size_t maxLength = 0;
        for (const auto& sound : sounds) {
            maxLength = std::max(maxLength, sound.size());
        }

        std::vector<short> mixed(maxLength, 0);

        for (size_t i = 0; i < sounds.size(); ++i) {
            const auto& sound = sounds[i];
            float volume = (i < volumes.size()) ? volumes[i] : 1.0f;

            for (size_t j = 0; j < sound.size() && j < maxLength; ++j) {
                mixed[j] += static_cast<short>(sound[j] * volume);
            }
        }

        return mixed;
    }

    std::vector<std::vector<short>> SoundGenerator::GenerateVariations(const std::vector<short>& baseSound,
                                                                     int variationCount) {
        std::vector<std::vector<short>> variations;

        for (int i = 0; i < variationCount; ++i) {
            // Convert to float for processing
            std::vector<float> floatData(baseSound.size());
            for (size_t j = 0; j < baseSound.size(); ++j) {
                floatData[j] = static_cast<float>(baseSound[j]) / 32767.0f;
            }

            // Apply random variations
            float pitchShift = 0.9f + 0.2f * m_randomFloat(m_randomEngine);
            float volumeVariation = 0.8f + 0.4f * m_randomFloat(m_randomEngine);

            for (auto& sample : floatData) {
                sample *= volumeVariation;
            }

            variations.push_back(ConvertToPCM16(floatData));
        }

        return variations;
    }

    std::vector<float> SoundGenerator::GenerateWaveform(float frequency, float duration,
                                                      const std::string& waveformType,
                                                      int sampleRate) {
        int numSamples = static_cast<int>(sampleRate * duration);
        std::vector<float> waveform(numSamples);

        for (int i = 0; i < numSamples; ++i) {
            float t = static_cast<float>(i) / sampleRate;
            float phase = 2.0f * 3.14159f * frequency * t;

            if (waveformType == "sine") {
                waveform[i] = std::sin(phase);
            } else if (waveformType == "square") {
                waveform[i] = std::sin(phase) > 0.0f ? 1.0f : -1.0f;
            } else if (waveformType == "saw") {
                waveform[i] = 2.0f * (frequency * t - std::floor(frequency * t + 0.5f));
            } else if (waveformType == "noise") {
                waveform[i] = m_randomFloat(m_randomEngine) * 2.0f - 1.0f;
            } else {
                // Default to sine
                waveform[i] = std::sin(phase);
            }
        }

        return waveform;
    }

    void SoundGenerator::ApplyADSREnvelope(std::vector<float>& soundData,
                                         float attack, float decay, float sustain, float release,
                                         int sampleRate) {
        int attackSamples = static_cast<int>(attack * sampleRate);
        int decaySamples = static_cast<int>(decay * sampleRate);
        int releaseSamples = static_cast<int>(release * sampleRate);

        for (size_t i = 0; i < soundData.size(); ++i) {
            float envelope = 1.0f;

            if (i < attackSamples) {
                // Attack phase
                envelope = static_cast<float>(i) / attackSamples;
            } else if (i < attackSamples + decaySamples) {
                // Decay phase
                float decayProgress = static_cast<float>(i - attackSamples) / decaySamples;
                envelope = 1.0f - decayProgress * (1.0f - sustain);
            } else if (i > soundData.size() - releaseSamples) {
                // Release phase
                float releaseProgress = static_cast<float>(i - (soundData.size() - releaseSamples)) / releaseSamples;
                envelope = sustain * (1.0f - releaseProgress);
            } else {
                // Sustain phase
                envelope = sustain;
            }

            soundData[i] *= envelope;
        }
    }

    void SoundGenerator::AddHarmonics(std::vector<float>& soundData, const std::vector<float>& harmonics) {
        std::vector<float> original = soundData;
        std::fill(soundData.begin(), soundData.end(), 0.0f);

        for (size_t h = 0; h < harmonics.size(); ++h) {
            float harmonicGain = harmonics[h] / (h + 1); // Reduce amplitude for higher harmonics
            for (size_t i = 0; i < soundData.size(); ++i) {
                soundData[i] += original[i] * harmonicGain;
            }
        }
    }

    void SoundGenerator::AddNoise(std::vector<float>& soundData, float amount, const std::string& noiseType) {
        for (auto& sample : soundData) {
            float noise = m_randomFloat(m_randomEngine) * 2.0f - 1.0f;

            if (noiseType == "pink") {
                // Simple pink noise approximation
                static float prev = 0.0f;
                noise = (noise + prev) * 0.5f;
                prev = noise;
            } else if (noiseType == "brown") {
                // Simple brown noise approximation
                static float prev = 0.0f;
                noise = (noise + prev) * 0.02f;
                prev += noise;
            }

            sample += noise * amount;
        }
    }

    void SoundGenerator::ApplyLowPassFilter(std::vector<float>& soundData, float cutoff, int sampleRate) {
        float rc = 1.0f / (2.0f * 3.14159f * cutoff);
        float dt = 1.0f / sampleRate;
        float alpha = dt / (rc + dt);

        float prev = soundData[0];
        for (size_t i = 1; i < soundData.size(); ++i) {
            soundData[i] = prev + alpha * (soundData[i] - prev);
            prev = soundData[i];
        }
    }

    void SoundGenerator::ApplyHighPassFilter(std::vector<float>& soundData, float cutoff, int sampleRate) {
        float rc = 1.0f / (2.0f * 3.14159f * cutoff);
        float dt = 1.0f / sampleRate;
        float alpha = rc / (rc + dt);

        float prevInput = soundData[0];
        float prevOutput = soundData[0];

        for (size_t i = 1; i < soundData.size(); ++i) {
            soundData[i] = alpha * (prevOutput + soundData[i] - prevInput);
            prevInput = soundData[i];
            prevOutput = soundData[i];
        }
    }

    std::vector<short> SoundGenerator::ConvertToPCM16(const std::vector<float>& floatData) {
        std::vector<short> pcmData(floatData.size());

        for (size_t i = 0; i < floatData.size(); ++i) {
            // Clamp to [-1, 1] and convert to 16-bit
            float clamped = std::max(-1.0f, std::min(1.0f, floatData[i]));
            pcmData[i] = static_cast<short>(clamped * 32767.0f);
        }

        return pcmData;
    }

} // namespace VoxelCraft
