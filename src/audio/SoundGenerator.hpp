/**
 * @file SoundGenerator.hpp
 * @brief VoxelCraft Procedural Sound Generator
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_AUDIO_SOUND_GENERATOR_HPP
#define VOXELCRAFT_AUDIO_SOUND_GENERATOR_HPP

#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <random>
#include <functional>
#include <atomic>
#include <cmath>

// Simple Vec3 implementation for audio positioning
struct Vec3 {
    float x, y, z;
    Vec3() : x(0.0f), y(0.0f), z(0.0f) {}
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

    Vec3 operator-(const Vec3& other) const {
        return Vec3(x - other.x, y - other.y, z - other.z);
    }

    float length() const {
        return std::sqrt(x*x + y*y + z*z);
    }

    Vec3 normalize() const {
        float len = length();
        if (len > 0.0f) {
            return Vec3(x/len, y/len, z/len);
        }
        return Vec3(0.0f, 0.0f, 0.0f);
    }

    float dot(const Vec3& other) const {
        return x*other.x + y*other.y + z*other.z;
    }
};

namespace VoxelCraft {

    /**
     * @enum SoundType
     * @brief Types of procedural sounds that can be generated
     */
    enum class ProceduralSoundType {
        // Block sounds
        BLOCK_PLACE,
        BLOCK_BREAK,
        BLOCK_STEP,
        BLOCK_FALL,
        BLOCK_LAND,

        // Player sounds
        PLAYER_WALK,
        PLAYER_RUN,
        PLAYER_JUMP,
        PLAYER_LAND,
        PLAYER_SWIM,
        PLAYER_HURT,
        PLAYER_DEATH,

        // Entity sounds
        ENTITY_AMBIENT,
        ENTITY_HURT,
        ENTITY_DEATH,
        ENTITY_ATTACK,

        // Environmental sounds
        AMBIENT_CAVE,
        AMBIENT_FOREST,
        AMBIENT_DESERT,
        AMBIENT_OCEAN,
        AMBIENT_NETHER,
        AMBIENT_END,

        // Weather sounds
        WEATHER_RAIN,
        WEATHER_THUNDER,
        WEATHER_WIND,
        WEATHER_SNOW,

        // Tool sounds
        TOOL_DIG,
        TOOL_MINE,
        TOOL_BUILD,
        TOOL_ATTACK,

        // Item sounds
        ITEM_USE,
        ITEM_BREAK,
        ITEM_COLLECT,
        ITEM_DROP,

        // GUI sounds
        GUI_CLICK,
        GUI_HOVER,
        GUI_OPEN,
        GUI_CLOSE,

        // Music themes
        MUSIC_OVERWORLD,
        MUSIC_NETHER,
        MUSIC_END,
        MUSIC_CREDITS,

        // Custom procedural
        PROCEDURAL_WHITE_NOISE,
        PROCEDURAL_PINK_NOISE,
        PROCEDURAL_BROWN_NOISE,
        PROCEDURAL_SYNTH_WAVE,
        PROCEDURAL_AMBIENT_PAD,
        PROCEDURAL_DRONE
    };

    /**
     * @struct SoundParameters
     * @brief Parameters for procedural sound generation
     */
    struct SoundParameters {
        float frequency = 440.0f;          ///< Base frequency in Hz
        float duration = 1.0f;            ///< Duration in seconds
        float volume = 1.0f;              ///< Volume (0.0 - 1.0)
        float attack = 0.01f;             ///< Attack time in seconds
        float decay = 0.1f;               ///< Decay time in seconds
        float sustain = 0.7f;             ///< Sustain level (0.0 - 1.0)
        float release = 0.2f;             ///< Release time in seconds
        std::vector<float> harmonics;     ///< Harmonic multipliers
        float noiseAmount = 0.0f;         ///< Amount of noise to add
        bool useEnvelope = true;          ///< Use ADSR envelope
    };

    /**
     * @struct BiomeSoundProfile
     * @brief Sound characteristics for different biomes
     */
    struct BiomeSoundProfile {
        std::string name;
        float baseFrequency;
        float complexity;                 ///< Sound complexity (0.0 - 1.0)
        std::vector<std::string> soundLayers;
        float reverbAmount;
        float echoAmount;
    };

    /**
     * @class SoundGenerator
     * @brief Advanced procedural sound generator for VoxelCraft
     *
     * Features:
     * - 1000+ unique procedural sounds
     * - Biome-specific ambient generation
     * - Dynamic sound synthesis
     * - Real-time sound modification
     * - Memory-efficient sound caching
     */
    class SoundGenerator {
    public:
        SoundGenerator();
        ~SoundGenerator();

        /**
         * @brief Initialize the sound generator
         * @return true if successful
         */
        bool Initialize();

        /**
         * @brief Shutdown the sound generator
         */
        void Shutdown();

        /**
         * @brief Generate a procedural sound
         * @param type Sound type to generate
         * @param seed Random seed for variation
         * @param parameters Additional parameters
         * @return Generated audio data or nullptr if failed
         */
        std::vector<short> GenerateSound(ProceduralSoundType type,
                                        unsigned int seed = 0,
                                        const SoundParameters& parameters = SoundParameters());

        /**
         * @brief Generate biome-specific ambient sound
         * @param biomeName Name of the biome
         * @param duration Duration in seconds
         * @param seed Random seed
         * @return Generated audio data
         */
        std::vector<short> GenerateBiomeAmbient(const std::string& biomeName,
                                              float duration = 30.0f,
                                              unsigned int seed = 0);

        /**
         * @brief Generate weather sound
         * @param weatherType Type of weather
         * @param intensity Weather intensity (0.0 - 1.0)
         * @param duration Duration in seconds
         * @return Generated audio data
         */
        std::vector<short> GenerateWeatherSound(const std::string& weatherType,
                                               float intensity = 0.5f,
                                               float duration = 10.0f);

        /**
         * @brief Generate tool sound
         * @param toolType Type of tool
         * @param materialType Type of material being worked
         * @param action Action being performed
         * @return Generated audio data
         */
        std::vector<short> GenerateToolSound(const std::string& toolType,
                                           const std::string& materialType,
                                           const std::string& action);

        /**
         * @brief Generate entity sound
         * @param entityType Type of entity
         * @param soundType Type of sound (ambient, hurt, death, etc.)
         * @param size Entity size multiplier
         * @return Generated audio data
         */
        std::vector<short> GenerateEntitySound(const std::string& entityType,
                                             const std::string& soundType,
                                             float size = 1.0f);

        /**
         * @brief Generate block interaction sound
         * @param blockType Type of block
         * @param action Action being performed
         * @param hardness Block hardness
         * @return Generated audio data
         */
        std::vector<short> GenerateBlockSound(const std::string& blockType,
                                            const std::string& action,
                                            float hardness = 1.0f);

        /**
         * @brief Generate music track
         * @param style Music style/genre
         * @param duration Duration in seconds
         * @param seed Random seed
         * @return Generated audio data
         */
        std::vector<short> GenerateMusic(const std::string& style,
                                        float duration = 180.0f,
                                        unsigned int seed = 0);

        /**
         * @brief Modify existing sound with effects
         * @param soundData Original sound data
         * @param effectType Type of effect to apply
         * @param intensity Effect intensity (0.0 - 1.0)
         * @return Modified sound data
         */
        std::vector<short> ApplyEffect(const std::vector<short>& soundData,
                                     const std::string& effectType,
                                     float intensity = 0.5f);

        /**
         * @brief Mix multiple sounds together
         * @param sounds Vector of sound data to mix
         * @param volumes Vector of volumes for each sound
         * @return Mixed audio data
         */
        std::vector<short> MixSounds(const std::vector<std::vector<short>>& sounds,
                                   const std::vector<float>& volumes);

        /**
         * @brief Generate sound variations
         * @param baseSound Base sound to vary
         * @param variationCount Number of variations to generate
         * @return Vector of sound variations
         */
        std::vector<std::vector<short>> GenerateVariations(const std::vector<short>& baseSound,
                                                         int variationCount = 5);

    private:
        /**
         * @brief Generate basic waveform
         * @param frequency Base frequency
         * @param duration Duration in seconds
         * @param waveformType Type of waveform (sine, square, saw, noise)
         * @param sampleRate Sample rate
         * @return Generated waveform
         */
        std::vector<float> GenerateWaveform(float frequency, float duration,
                                          const std::string& waveformType = "sine",
                                          int sampleRate = 44100);

        /**
         * @brief Apply ADSR envelope to sound
         * @param soundData Sound data to process
         * @param attack Attack time in seconds
         * @param decay Decay time in seconds
         * @param sustain Sustain level
         * @param release Release time in seconds
         * @param sampleRate Sample rate
         */
        void ApplyADSREnvelope(std::vector<float>& soundData,
                             float attack, float decay, float sustain, float release,
                             int sampleRate = 44100);

        /**
         * @brief Add harmonics to sound
         * @param soundData Sound data to process
         * @param harmonics Vector of harmonic multipliers
         */
        void AddHarmonics(std::vector<float>& soundData, const std::vector<float>& harmonics);

        /**
         * @brief Add noise to sound
         * @param soundData Sound data to process
         * @param amount Noise amount (0.0 - 1.0)
         * @param noiseType Type of noise (white, pink, brown)
         */
        void AddNoise(std::vector<float>& soundData, float amount,
                    const std::string& noiseType = "white");

        /**
         * @brief Apply low-pass filter
         * @param soundData Sound data to filter
         * @param cutoff Cutoff frequency
         * @param sampleRate Sample rate
         */
        void ApplyLowPassFilter(std::vector<float>& soundData, float cutoff, int sampleRate = 44100);

        /**
         * @brief Apply high-pass filter
         * @param soundData Sound data to filter
         * @param cutoff Cutoff frequency
         * @param sampleRate Sample rate
         */
        void ApplyHighPassFilter(std::vector<float>& soundData, float cutoff, int sampleRate = 44100);

        /**
         * @brief Convert float samples to 16-bit PCM
         * @param floatData Float sound data
         * @return 16-bit PCM data
         */
        std::vector<short> ConvertToPCM16(const std::vector<float>& floatData);

        /**
         * @brief Generate block break sound
         * @param blockType Type of block
         * @param hardness Block hardness
         * @return Generated sound data
         */
        std::vector<short> GenerateBlockBreakSound(const std::string& blockType, float hardness);

        /**
         * @brief Generate entity ambient sound
         * @param entityType Type of entity
         * @param size Entity size
         * @return Generated sound data
         */
        std::vector<short> GenerateEntityAmbientSound(const std::string& entityType, float size);

        /**
         * @brief Generate tool interaction sound
         * @param toolType Type of tool
         * @param materialType Type of material
         * @return Generated sound data
         */
        std::vector<short> GenerateToolSound(const std::string& toolType, const std::string& materialType);

        // Random number generation
        std::mt19937 m_randomEngine;
        std::uniform_real_distribution<float> m_randomFloat;

        // Sample rate for all generated sounds
        int m_sampleRate = 44100;

        // Cache for generated sounds to avoid regeneration
        std::unordered_map<std::string, std::vector<short>> m_soundCache;
        size_t m_maxCacheSize = 100 * 1024 * 1024; // 100MB cache limit
        size_t m_currentCacheSize = 0;

        // Biome sound profiles
        std::unordered_map<std::string, BiomeSoundProfile> m_biomeProfiles;

        // Procedural sound parameters database
        std::unordered_map<ProceduralSoundType, SoundParameters> m_soundParameters;

        // Initialization flag
        bool m_initialized = false;
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_AUDIO_SOUND_GENERATOR_HPP
