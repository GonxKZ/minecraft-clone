/**
 * @file AudioSource.hpp
 * @brief VoxelCraft Audio Source Interface
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_AUDIO_AUDIO_SOURCE_HPP
#define VOXELCRAFT_AUDIO_AUDIO_SOURCE_HPP

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

namespace VoxelCraft {

    class AudioEffect;

    /**
     * @class AudioSource
     * @brief Represents an audio source in 3D space
     */
    class AudioSource {
    public:
        AudioSource();
        virtual ~AudioSource() = default;

        /**
         * @brief Play the audio source
         */
        virtual void Play() = 0;

        /**
         * @brief Stop the audio source
         */
        virtual void Stop() = 0;

        /**
         * @brief Pause the audio source
         */
        virtual void Pause() = 0;

        /**
         * @brief Check if source is playing
         * @return true if playing
         */
        virtual bool IsPlaying() const = 0;

        /**
         * @brief Set volume
         * @param volume Volume (0.0 - 1.0)
         */
        virtual void SetVolume(float volume) = 0;

        /**
         * @brief Set pitch
         * @param pitch Pitch multiplier
         */
        virtual void SetPitch(float pitch) = 0;

        /**
         * @brief Set position in 3D space
         * @param x X coordinate
         * @param y Y coordinate
         * @param z Z coordinate
         */
        virtual void SetPosition(float x, float y, float z) = 0;

        /**
         * @brief Set velocity for Doppler effect
         * @param x X velocity
         * @param y Y velocity
         * @param z Z velocity
         */
        virtual void SetVelocity(float x, float y, float z) = 0;

        /**
         * @brief Set looping
         * @param loop true to loop
         */
        virtual void SetLooping(bool loop) = 0;

        /**
         * @brief Apply audio effect
         * @param effect Effect to apply
         * @return true if applied successfully
         */
        virtual bool ApplyEffect(std::shared_ptr<AudioEffect> effect) = 0;

        /**
         * @brief Remove audio effect
         * @param effectName Effect name to remove
         * @return true if removed successfully
         */
        virtual bool RemoveEffect(const std::string& effectName) = 0;

        /**
         * @brief Get source ID
         * @return Unique source identifier
         */
        virtual uint32_t GetId() const = 0;

        /**
         * @brief Set completion callback
         * @param callback Callback function
         */
        void SetCompletionCallback(std::function<void()> callback) {
            m_completionCallback = callback;
        }

    protected:
        std::function<void()> m_completionCallback;
        std::unordered_map<std::string, std::shared_ptr<AudioEffect>> m_effects;
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_AUDIO_AUDIO_SOURCE_HPP
