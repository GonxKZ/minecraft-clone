/**
 * @file AudioManager.cpp
 * @brief VoxelCraft Simple Audio Manager Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#include "AudioManager.hpp"
#include "../core/Logger.hpp"
#include <iostream>
#include <cmath>
#include <random>

// OpenAL headers (if available)
#ifdef __APPLE__
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#elif defined(_WIN32)
#include <AL/al.h>
#include <AL/alc.h>
#else
#include <AL/al.h>
#include <AL/alc.h>
#endif

namespace VoxelCraft {

    AudioManager::AudioManager(std::shared_ptr<Config> config)
        : m_config(config)
        , m_state(AudioState::STOPPED)
        , m_device(nullptr)
        , m_context(nullptr)
        , m_listenerX(0.0f)
        , m_listenerY(0.0f)
        , m_listenerZ(0.0f)
    {
        VOXELCRAFT_TRACE("AudioManager created");
    }

    AudioManager::~AudioManager() {
        Shutdown();
    }

    bool AudioManager::Initialize() {
        VOXELCRAFT_INFO("Initializing Audio System");

        m_state = AudioState::INITIALIZED;

        // Try to initialize OpenAL
        if (!InitializeOpenAL()) {
            VOXELCRAFT_WARN("Failed to initialize OpenAL, using silent mode");
            return true; // Don't fail completely, just disable audio
        }

        // Load sound files (placeholder)
        if (!LoadSounds()) {
            VOXELCRAFT_WARN("Failed to load sounds, using generated sounds");
        }

        // Create sound sources
        if (!CreateSoundSources()) {
            VOXELCRAFT_ERROR("Failed to create sound sources");
            ShutdownOpenAL();
            return false;
        }

        VOXELCRAFT_INFO("Audio System initialized successfully");
        return true;
    }

    void AudioManager::Shutdown() {
        if (m_state == AudioState::STOPPED) {
            return;
        }

        VOXELCRAFT_INFO("Shutting down Audio System");

        StopMusic();

        // Clear sound sources
        m_soundSources.clear();
        m_musicSource.reset();

        // Clear buffers
        for (auto& buffer : m_soundBuffers) {
            alDeleteBuffers(1, &buffer.second);
        }
        m_soundBuffers.clear();

        for (auto& buffer : m_musicBuffers) {
            alDeleteBuffers(1, &buffer.second);
        }
        m_musicBuffers.clear();

        // Shutdown OpenAL
        ShutdownOpenAL();

        m_state = AudioState::STOPPED;
        VOXELCRAFT_INFO("Audio System shutdown complete");
    }

    void AudioManager::Update(float deltaTime) {
        // Update listener position based on player (simplified)
        // In a real implementation, this would get position from player entity

        if (m_context && m_state != AudioState::ERROR) {
            // Update listener position
            alListener3f(AL_POSITION, m_listenerX, m_listenerY, m_listenerZ);
            alListener3f(AL_VELOCITY, 0.0f, 0.0f, 0.0f);

            // Update listener orientation (forward: 0,0,-1, up: 0,1,0)
            ALfloat orientation[] = { 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f };
            alListenerfv(AL_ORIENTATION, orientation);
        }
    }

    bool AudioManager::PlaySound(SoundType soundType, float volume) {
        if (m_state == AudioState::ERROR || !m_audioConfig.enableSoundEffects) {
            return false;
        }

        SoundSource* source = FindAvailableSource();
        if (!source) {
            VOXELCRAFT_TRACE("No available sound sources for {}", static_cast<int>(soundType));
            return false;
        }

        auto it = m_soundBuffers.find(soundType);
        if (it == m_soundBuffers.end()) {
            // Generate sound buffer if not found
            unsigned int bufferId = GenerateSoundBuffer(soundType);
            if (bufferId == 0) {
                return false;
            }
            m_soundBuffers[soundType] = bufferId;
            it = m_soundBuffers.find(soundType);
        }

        // Set buffer and play
        alSourcei(source->m_sourceId, AL_BUFFER, it->second);

        // Set volume
        float finalVolume = (volume >= 0.0f) ? volume : GetVolumeForType(soundType);
        source->SetVolume(finalVolume);

        // Play sound
        source->Play();

        VOXELCRAFT_TRACE("Playing sound: {}", static_cast<int>(soundType));
        return true;
    }

    bool AudioManager::PlaySound3D(SoundType soundType, float x, float y, float z, float volume) {
        if (!PlaySound(soundType, volume)) {
            return false;
        }

        // Find the source we just started playing
        SoundSource* source = FindAvailableSource();
        if (source && source->IsPlaying()) {
            source->SetPosition(x, y, z);
        }

        return true;
    }

    bool AudioManager::PlayMusic(const std::string& musicName) {
        if (m_state == AudioState::ERROR || !m_audioConfig.enableMusic) {
            return false;
        }

        StopMusic();

        auto it = m_musicBuffers.find(musicName);
        if (it == m_musicBuffers.end()) {
            // Generate music buffer if not found
            unsigned int bufferId = GenerateMusicBuffer(musicName);
            if (bufferId == 0) {
                return false;
            }
            m_musicBuffers[musicName] = bufferId;
            it = m_musicBuffers.find(musicName);
        }

        // Set buffer and play
        alSourcei(m_musicSource->m_sourceId, AL_BUFFER, it->second);
        m_musicSource->SetLooping(true);
        m_musicSource->SetVolume(m_audioConfig.musicVolume * m_audioConfig.masterVolume);
        m_musicSource->Play();

        VOXELCRAFT_INFO("Playing music: {}", musicName);
        return true;
    }

    void AudioManager::StopMusic() {
        if (m_musicSource) {
            m_musicSource->Stop();
        }
    }

    void AudioManager::PauseMusic() {
        if (m_musicSource) {
            m_musicSource->Pause();
        }
    }

    void AudioManager::ResumeMusic() {
        if (m_musicSource && !m_musicSource->IsPlaying()) {
            m_musicSource->Play();
        }
    }

    void AudioManager::SetMasterVolume(float volume) {
        m_audioConfig.masterVolume = std::max(0.0f, std::min(1.0f, volume));

        if (m_context) {
            alListenerf(AL_GAIN, m_audioConfig.masterVolume);
        }
    }

    void AudioManager::SetMusicVolume(float volume) {
        m_audioConfig.musicVolume = std::max(0.0f, std::min(1.0f, volume));

        if (m_musicSource) {
            float finalVolume = m_audioConfig.musicVolume * m_audioConfig.masterVolume;
            m_musicSource->SetVolume(finalVolume);
        }
    }

    void AudioManager::SetSFXVolume(float volume) {
        m_audioConfig.sfxVolume = std::max(0.0f, std::min(1.0f, volume));
    }

    bool AudioManager::InitializeOpenAL() {
        // Initialize OpenAL device
        m_device = alcOpenDevice(nullptr);
        if (!m_device) {
            VOXELCRAFT_ERROR("Failed to open OpenAL device");
            return false;
        }

        // Create OpenAL context
        m_context = alcCreateContext(static_cast<ALCdevice*>(m_device), nullptr);
        if (!m_context) {
            VOXELCRAFT_ERROR("Failed to create OpenAL context");
            alcCloseDevice(static_cast<ALCdevice*>(m_device));
            m_device = nullptr;
            return false;
        }

        // Make context current
        if (!alcMakeContextCurrent(static_cast<ALCcontext*>(m_context))) {
            VOXELCRAFT_ERROR("Failed to make OpenAL context current");
            alcDestroyContext(static_cast<ALCcontext*>(m_context));
            alcCloseDevice(static_cast<ALCdevice*>(m_device));
            m_context = nullptr;
            m_device = nullptr;
            return false;
        }

        // Set listener properties
        alListenerf(AL_GAIN, m_audioConfig.masterVolume);
        alListener3f(AL_POSITION, 0.0f, 0.0f, 0.0f);
        alListener3f(AL_VELOCITY, 0.0f, 0.0f, 0.0f);

        VOXELCRAFT_INFO("OpenAL initialized successfully");
        return true;
    }

    void AudioManager::ShutdownOpenAL() {
        if (m_context) {
            alcMakeContextCurrent(nullptr);
            alcDestroyContext(static_cast<ALCcontext*>(m_context));
            m_context = nullptr;
        }

        if (m_device) {
            alcCloseDevice(static_cast<ALCdevice*>(m_device));
            m_device = nullptr;
        }
    }

    bool AudioManager::LoadSounds() {
        // Placeholder - in a real implementation, this would load WAV/MP3 files
        // For now, we'll generate simple sounds programmatically
        return true;
    }

    bool AudioManager::CreateSoundSources() {
        m_soundSources.clear();

        for (int i = 0; i < m_audioConfig.maxSoundSources; ++i) {
            auto source = std::make_unique<SoundSource>();
            m_soundSources.push_back(std::move(source));
        }

        m_musicSource = std::make_unique<SoundSource>();
        return true;
    }

    float AudioManager::GetVolumeForType(SoundType soundType) const {
        float baseVolume = m_audioConfig.masterVolume;

        switch (soundType) {
            case SoundType::MUSIC:
                return baseVolume * m_audioConfig.musicVolume;
            case SoundType::AMBIENT:
                return baseVolume * m_audioConfig.ambientVolume;
            default:
                return baseVolume * m_audioConfig.sfxVolume;
        }
    }

    SoundSource* AudioManager::FindAvailableSource() {
        for (auto& source : m_soundSources) {
            if (!source->IsPlaying()) {
                return source.get();
            }
        }
        return nullptr;
    }

    unsigned int AudioManager::GenerateSoundBuffer(SoundType soundType) {
        if (!m_context) return 0;

        // Generate a simple beep sound
        const int sampleRate = 44100;
        const float duration = 0.2f; // 200ms
        const int numSamples = static_cast<int>(sampleRate * duration);

        std::vector<short> samples(numSamples);

        // Generate different tones for different sound types
        float frequency = 440.0f; // A4 note
        switch (soundType) {
            case SoundType::BLOCK_PLACE: frequency = 523.25f; break; // C5
            case SoundType::BLOCK_BREAK: frequency = 349.23f; break; // F4
            case SoundType::PLAYER_WALK: frequency = 220.0f; break;  // A3
            case SoundType::PLAYER_JUMP: frequency = 659.25f; break; // E5
            case SoundType::PLAYER_LAND: frequency = 293.66f; break; // D4
            default: frequency = 440.0f; break;
        }

        for (int i = 0; i < numSamples; ++i) {
            float t = static_cast<float>(i) / sampleRate;
            float sample = std::sin(2.0f * 3.14159f * frequency * t);

            // Add some noise for texture
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_real_distribution<> dis(-0.1, 0.1);
            sample += dis(gen);

            // Fade out at the end
            if (i > numSamples * 0.8f) {
                float fade = 1.0f - (static_cast<float>(i) - numSamples * 0.8f) / (numSamples * 0.2f);
                sample *= fade;
            }

            samples[i] = static_cast<short>(sample * 32767);
        }

        // Create OpenAL buffer
        unsigned int bufferId;
        alGenBuffers(1, &bufferId);

        if (alGetError() != AL_NO_ERROR) {
            return 0;
        }

        // Fill buffer with data
        alBufferData(bufferId, AL_FORMAT_MONO16, samples.data(),
                    samples.size() * sizeof(short), sampleRate);

        if (alGetError() != AL_NO_ERROR) {
            alDeleteBuffers(1, &bufferId);
            return 0;
        }

        return bufferId;
    }

    unsigned int AudioManager::GenerateMusicBuffer(const std::string& musicName) {
        if (!m_context) return 0;

        // Generate a simple melody
        const int sampleRate = 44100;
        const float duration = 30.0f; // 30 seconds
        const int numSamples = static_cast<int>(sampleRate * duration);

        std::vector<short> samples(numSamples, 0);

        // Simple melody with multiple notes
        const float notes[] = { 261.63f, 293.66f, 329.63f, 349.23f, 392.00f, 440.00f, 493.88f, 523.25f };
        const float noteDuration = 0.5f; // 500ms per note
        const int samplesPerNote = static_cast<int>(sampleRate * noteDuration);

        for (size_t i = 0; i < sizeof(notes) / sizeof(notes[0]); ++i) {
            int startSample = i * samplesPerNote;
            if (startSample >= numSamples) break;

            int endSample = std::min(startSample + samplesPerNote, numSamples);

            for (int j = startSample; j < endSample; ++j) {
                float t = static_cast<float>(j - startSample) / sampleRate;
                float sample = std::sin(2.0f * 3.14159f * notes[i] * t) * 0.3f;

                // Add some harmonics for richness
                sample += std::sin(2.0f * 3.14159f * notes[i] * 2 * t) * 0.1f;
                sample += std::sin(2.0f * 3.14159f * notes[i] * 3 * t) * 0.05f;

                samples[j] = static_cast<short>(sample * 32767);
            }
        }

        // Create OpenAL buffer
        unsigned int bufferId;
        alGenBuffers(1, &bufferId);

        if (alGetError() != AL_NO_ERROR) {
            return 0;
        }

        // Fill buffer with data
        alBufferData(bufferId, AL_FORMAT_MONO16, samples.data(),
                    samples.size() * sizeof(short), sampleRate);

        if (alGetError() != AL_NO_ERROR) {
            alDeleteBuffers(1, &bufferId);
            return 0;
        }

        return bufferId;
    }

    // SoundSource implementation

    SoundSource::SoundSource()
        : m_sourceId(0)
        , m_initialized(false)
    {
        if (alGetError() == AL_NO_ERROR) {
            alGenSources(1, &m_sourceId);
            if (alGetError() == AL_NO_ERROR) {
                m_initialized = true;
            }
        }
    }

    SoundSource::~SoundSource() {
        if (m_initialized && m_sourceId) {
            alDeleteSources(1, &m_sourceId);
        }
    }

    void SoundSource::Play() {
        if (m_initialized) {
            alSourcePlay(m_sourceId);
        }
    }

    void SoundSource::Stop() {
        if (m_initialized) {
            alSourceStop(m_sourceId);
        }
    }

    void SoundSource::Pause() {
        if (m_initialized) {
            alSourcePause(m_sourceId);
        }
    }

    bool SoundSource::IsPlaying() const {
        if (!m_initialized) return false;

        ALint state;
        alGetSourcei(m_sourceId, AL_SOURCE_STATE, &state);
        return state == AL_PLAYING;
    }

    void SoundSource::SetVolume(float volume) {
        if (m_initialized) {
            alSourcef(m_sourceId, AL_GAIN, volume);
        }
    }

    void SoundSource::SetPosition(float x, float y, float z) {
        if (m_initialized) {
            alSource3f(m_sourceId, AL_POSITION, x, y, z);
        }
    }

    void SoundSource::SetLooping(bool loop) {
        if (m_initialized) {
            alSourcei(m_sourceId, AL_LOOPING, loop ? AL_TRUE : AL_FALSE);
        }
    }

} // namespace VoxelCraft
