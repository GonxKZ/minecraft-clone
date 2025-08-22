/**
 * @file AudioManager.hpp
 * @brief VoxelCraft Simple Audio Manager - Basic Sound System
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines a simple audio system for the VoxelCraft game engine
 * using OpenAL for sound effects and music playback.
 */

#ifndef VOXELCRAFT_AUDIO_AUDIO_MANAGER_HPP
#define VOXELCRAFT_AUDIO_AUDIO_MANAGER_HPP

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "../core/Config.hpp"
#include "../entities/System.hpp"

namespace VoxelCraft {

    /**
     * @enum SoundType
     * @brief Types of sounds in the game
     */
    enum class SoundType {
        BLOCK_PLACE,       // Placing a block
        BLOCK_BREAK,       // Breaking a block
        PLAYER_WALK,       // Player walking
        PLAYER_JUMP,       // Player jumping
        PLAYER_LAND,       // Player landing
        AMBIENT,          // Ambient sounds
        MUSIC             // Background music
    };

    /**
     * @enum AudioState
     * @brief Current state of the audio system
     */
    enum class AudioState {
        INITIALIZED,
        PLAYING,
        PAUSED,
        STOPPED,
        ERROR
    };

    /**
     * @struct AudioConfig
     * @brief Configuration for audio system
     */
    struct AudioConfig {
        float masterVolume;        // 0.0 - 1.0
        float musicVolume;         // 0.0 - 1.0
        float sfxVolume;          // 0.0 - 1.0
        float ambientVolume;      // 0.0 - 1.0
        bool enableMusic;         // Enable background music
        bool enableSoundEffects;  // Enable sound effects
        bool enableAmbientSounds; // Enable ambient sounds
        int maxSoundSources;     // Maximum simultaneous sounds

        AudioConfig()
            : masterVolume(0.7f)
            , musicVolume(0.5f)
            , sfxVolume(0.8f)
            , ambientVolume(0.3f)
            , enableMusic(true)
            , enableSoundEffects(true)
            , enableAmbientSounds(true)
            , maxSoundSources(32)
        {}
    };

    /**
     * @class SoundSource
     * @brief Represents a single sound source
     */
    class SoundSource {
    public:
        SoundSource();
        ~SoundSource();

        /**
         * @brief Play the sound
         */
        void Play();

        /**
         * @brief Stop the sound
         */
        void Stop();

        /**
         * @brief Pause the sound
         */
        void Pause();

        /**
         * @brief Check if sound is playing
         */
        bool IsPlaying() const;

        /**
         * @brief Set volume
         */
        void SetVolume(float volume);

        /**
         * @brief Set position (3D audio)
         */
        void SetPosition(float x, float y, float z);

        /**
         * @brief Set looping
         */
        void SetLooping(bool loop);

    private:
        unsigned int m_sourceId;
        bool m_initialized;

        friend class AudioManager;
    };

    /**
     * @class AudioManager
     * @brief Simple audio system manager
     */
    class AudioManager : public System {
    public:
        /**
         * @brief Constructor
         * @param config Configuration system
         */
        AudioManager(std::shared_ptr<Config> config);

        /**
         * @brief Destructor
         */
        ~AudioManager();

        /**
         * @brief Initialize audio system
         * @return true if successful
         */
        bool Initialize();

        /**
         * @brief Shutdown audio system
         */
        void Shutdown();

        /**
         * @brief Update audio system
         * @param deltaTime Time since last update
         */
        void Update(float deltaTime) override;

        /**
         * @brief Play a sound effect
         * @param soundType Type of sound to play
         * @param volume Optional volume override
         * @return true if sound was played
         */
        bool PlaySound(SoundType soundType, float volume = -1.0f);

        /**
         * @brief Play a sound at specific position
         * @param soundType Type of sound to play
         * @param x X position
         * @param y Y position
         * @param z Z position
         * @param volume Optional volume override
         * @return true if sound was played
         */
        bool PlaySound3D(SoundType soundType, float x, float y, float z, float volume = -1.0f);

        /**
         * @brief Play background music
         * @param musicName Name of music track
         * @return true if music started playing
         */
        bool PlayMusic(const std::string& musicName);

        /**
         * @brief Stop background music
         */
        void StopMusic();

        /**
         * @brief Pause background music
         */
        void PauseMusic();

        /**
         * @brief Resume background music
         */
        void ResumeMusic();

        /**
         * @brief Set master volume
         * @param volume Volume (0.0 - 1.0)
         */
        void SetMasterVolume(float volume);

        /**
         * @brief Set music volume
         * @param volume Volume (0.0 - 1.0)
         */
        void SetMusicVolume(float volume);

        /**
         * @brief Set sound effects volume
         * @param volume Volume (0.0 - 1.0)
         */
        void SetSFXVolume(float volume);

        /**
         * @brief Get current audio state
         * @return Audio state
         */
        AudioState GetState() const { return m_state; }

        /**
         * @brief Check if audio system is initialized
         * @return true if initialized
         */
        bool IsInitialized() const { return m_state != AudioState::ERROR; }

    private:
        std::shared_ptr<Config> m_config;
        AudioConfig m_audioConfig;
        AudioState m_state;

        // OpenAL context and device
        void* m_device;        // ALCdevice*
        void* m_context;       // ALCcontext*

        // Sound sources
        std::vector<std::unique_ptr<SoundSource>> m_soundSources;
        std::unique_ptr<SoundSource> m_musicSource;

        // Sound buffers (simulated)
        std::unordered_map<SoundType, unsigned int> m_soundBuffers;
        std::unordered_map<std::string, unsigned int> m_musicBuffers;

        // Listener position (player position)
        float m_listenerX, m_listenerY, m_listenerZ;

        /**
         * @brief Initialize OpenAL
         * @return true if successful
         */
        bool InitializeOpenAL();

        /**
         * @brief Shutdown OpenAL
         */
        void ShutdownOpenAL();

        /**
         * @brief Load sound files
         * @return true if successful
         */
        bool LoadSounds();

        /**
         * @brief Create sound sources
         * @return true if successful
         */
        bool CreateSoundSources();

        /**
         * @brief Get volume for sound type
         * @param soundType Sound type
         * @return Volume multiplier
         */
        float GetVolumeForType(SoundType soundType) const;

        /**
         * @brief Find available sound source
         * @return Sound source or nullptr if none available
         */
        SoundSource* FindAvailableSource();

        /**
         * @brief Generate simple sound data (placeholder)
         * @param soundType Sound type
         * @return Buffer ID
         */
        unsigned int GenerateSoundBuffer(SoundType soundType);

        /**
         * @brief Generate music buffer (placeholder)
         * @param musicName Music name
         * @return Buffer ID
         */
        unsigned int GenerateMusicBuffer(const std::string& musicName);
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_AUDIO_AUDIO_MANAGER_HPP
