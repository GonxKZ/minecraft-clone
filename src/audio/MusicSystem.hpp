/**
 * @file MusicSystem.hpp
 * @brief VoxelCraft Dynamic Music System
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines the MusicSystem class that provides comprehensive dynamic
 * music management for the VoxelCraft game engine, including adaptive music,
 * procedural composition, mood-based transitions, and seamless audio integration
 * for immersive gaming experiences.
 */

#ifndef VOXELCRAFT_AUDIO_MUSIC_SYSTEM_HPP
#define VOXELCRAFT_AUDIO_MUSIC_SYSTEM_HPP

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
#include <queue>
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
     * @enum MusicState
     * @brief Music system states
     */
    enum class MusicState {
        Stopped,                ///< Music is stopped
        Playing,                ///< Music is playing
        Paused,                 ///< Music is paused
        FadingIn,               ///< Music is fading in
        FadingOut,              ///< Music is fading out
        Transitioning           ///< Music is transitioning
    };

    /**
     * @enum MusicGenre
     * @brief Music genres for categorization
     */
    enum class MusicGenre {
        Ambient,                ///< Ambient/background music
        Action,                 ///< Action/intense music
        Adventure,              ///< Adventure/exploration music
        Combat,                 ///< Combat/battle music
        Exploration,            ///< Exploration/discovery music
        Mystery,                ///< Mystery/stealth music
        Tension,                ///< Tension/suspense music
        Victory,                ///< Victory/triumph music
        Defeat,                 ///< Defeat/loss music
        Menu,                   ///< Menu/interface music
        Custom                  ///< Custom genre
    };

    /**
     * @enum MusicMood
     * @brief Music mood/intensity levels
     */
    enum class MusicMood {
        Calm,                   ///< Calm and peaceful
        Normal,                 ///< Normal intensity
        Intense,                ///< High intensity
        Critical,               ///< Critical/high tension
        Epic                    ///< Epic/majestic
    };

    /**
     * @enum MusicTransition
     * @brief Music transition types
     */
    enum class MusicTransition {
        Immediate,              ///< Immediate transition
        Crossfade,              ///< Crossfade transition
        BeatSync,               ///< Beat-synchronized transition
        Layered,                ///< Layered transition
        Procedural              ///< Procedural transition
    };

    /**
     * @enum MusicLayer
     * @brief Music layer types
     */
    enum class MusicLayer {
        Base,                   ///< Base music layer
        Melody,                 ///< Melody layer
        Harmony,                ///< Harmony layer
        Rhythm,                 ///< Rhythm layer
        Effects,                ///< Sound effects layer
        Ambient                 ///< Ambient layer
    };

    /**
     * @struct MusicTrack
     * @brief Music track configuration
     */
    struct MusicTrack {
        uint32_t trackId;                      ///< Unique track ID
        std::string trackName;                 ///< Track name
        std::string filePath;                  ///< Audio file path
        MusicGenre genre;                      ///< Music genre
        MusicMood mood;                        ///< Music mood

        // Playback properties
        float volume;                          ///< Track volume (0.0 - 1.0)
        float pitch;                           ///< Track pitch (0.5 - 2.0)
        bool loop;                            ///< Loop playback
        float fadeInTime;                     ///< Fade in time (seconds)
        float fadeOutTime;                    ///< Fade out time (seconds)
        float startTime;                      ///< Start time offset (seconds)
        float endTime;                        ///< End time (0 = full length)

        // Adaptive properties
        std::vector<MusicMood> compatibleMoods; ///< Compatible moods
        std::vector<MusicGenre> compatibleGenres; ///< Compatible genres
        float intensity;                       ///< Track intensity (0.0 - 1.0)
        float energy;                          ///< Track energy level (0.0 - 1.0)

        // Metadata
        float duration;                        ///< Track duration (seconds)
        float bpm;                             ///< Beats per minute
        std::string composer;                  ///< Track composer
        std::string description;               ///< Track description
        std::unordered_map<std::string, std::any> metadata; ///< Custom metadata

        MusicTrack()
            : trackId(0)
            , genre(MusicGenre::Ambient)
            , mood(MusicMood::Normal)
            , volume(1.0f)
            , pitch(1.0f)
            , loop(true)
            , fadeInTime(2.0f)
            , fadeOutTime(2.0f)
            , startTime(0.0f)
            , endTime(0.0f)
            , intensity(0.5f)
            , energy(0.5f)
            , duration(0.0f)
            , bpm(120.0f)
        {}
    };

    /**
     * @struct MusicLayerConfig
     * @brief Music layer configuration
     */
    struct MusicLayerConfig {
        MusicLayer layerType;                  ///< Layer type
        std::string layerName;                 ///< Layer name
        uint32_t sourceId;                     ///< Audio source ID
        float volume;                          ///< Layer volume (0.0 - 1.0)
        float targetVolume;                    ///< Target volume for transitions
        float fadeTime;                        ///< Current fade time
        float fadeDuration;                    ///< Total fade duration
        bool active;                           ///< Layer is active

        // Layer properties
        float minIntensity;                    ///< Minimum intensity threshold
        float maxIntensity;                    ///< Maximum intensity threshold
        MusicMood requiredMood;                ///< Required mood for activation

        MusicLayerConfig()
            : layerType(MusicLayer::Base)
            , sourceId(0)
            , volume(0.0f)
            , targetVolume(0.0f)
            , fadeTime(0.0f)
            , fadeDuration(0.0f)
            , active(false)
            , minIntensity(0.0f)
            , maxIntensity(1.0f)
            , requiredMood(MusicMood::Normal)
        {}
    };

    /**
     * @struct MusicPlaylist
     * @brief Music playlist configuration
     */
    struct MusicPlaylist {
        uint32_t playlistId;                   ///< Unique playlist ID
        std::string playlistName;              ///< Playlist name
        std::vector<uint32_t> trackIds;        ///< Track IDs in playlist
        PlaylistMode mode;                     ///< Playlist playback mode
        bool shuffle;                          ///< Shuffle tracks
        bool repeat;                           ///< Repeat playlist

        // Adaptive properties
        MusicGenre genre;                      ///< Playlist genre
        std::vector<MusicMood> supportedMoods; ///< Supported moods
        float minIntensity;                    ///< Minimum intensity
        float maxIntensity;                    ///< Maximum intensity

        MusicPlaylist()
            : playlistId(0)
            , mode(PlaylistMode::Sequential)
            , shuffle(false)
            , repeat(true)
            , genre(MusicGenre::Ambient)
            , minIntensity(0.0f)
            , maxIntensity(1.0f)
        {}
    };

    /**
     * @enum PlaylistMode
     * @brief Playlist playback modes
     */
    enum class PlaylistMode {
        Sequential,             ///< Play tracks in order
        Random,                 ///< Play tracks randomly
        Weighted,               ///< Play based on weights
        Adaptive,               ///< Adaptive playlist
        Custom                  ///< Custom mode
    };

    /**
     * @struct MusicContext
     * @brief Music context for adaptive music
     */
    struct MusicContext {
        MusicGenre currentGenre;               ///< Current music genre
        MusicMood currentMood;                 ///< Current music mood
        float intensity;                       ///< Current intensity (0.0 - 1.0)
        float tension;                         ///< Current tension level (0.0 - 1.0)
        float energy;                          ///< Current energy level (0.0 - 1.0)

        // Game state
        bool inCombat;                         ///< Player is in combat
        bool exploring;                        ///< Player is exploring
        bool inDanger;                         ///< Player is in danger
        bool celebrating;                      ///< Player is celebrating
        bool inMenu;                           ///< Player is in menu

        // Environmental factors
        std::string environment;               ///< Current environment
        std::string weather;                   ///< Current weather
        std::string timeOfDay;                 ///< Time of day

        // Player state
        float playerHealth;                    ///< Player health (0.0 - 1.0)
        float playerStamina;                   ///< Player stamina (0.0 - 1.0)
        bool playerMoving;                     ///< Player is moving
        bool playerSprinting;                  ///< Player is sprinting

        MusicContext()
            : currentGenre(MusicGenre::Ambient)
            , currentMood(MusicMood::Normal)
            , intensity(0.5f)
            , tension(0.0f)
            , energy(0.5f)
            , inCombat(false)
            , exploring(true)
            , inDanger(false)
            , celebrating(false)
            , inMenu(false)
            , playerHealth(1.0f)
            , playerStamina(1.0f)
            , playerMoving(false)
            , playerSprinting(false)
        {}
    };

    /**
     * @struct MusicStats
     * @brief Music system performance statistics
     */
    struct MusicStats {
        // Playback statistics
        uint64_t tracksPlayed;                 ///< Total tracks played
        uint64_t transitionsPerformed;         ///< Total transitions
        double totalPlayTime;                  ///< Total play time (seconds)
        double averageTrackLength;             ///< Average track length

        // Layer statistics
        uint32_t activeLayers;                 ///< Currently active layers
        uint32_t totalLayers;                  ///< Total configured layers
        uint32_t layerTransitions;             ///< Layer transitions

        // Adaptive statistics
        uint32_t contextChanges;               ///< Context changes detected
        uint32_t moodChanges;                  ///< Mood changes
        uint32_t genreChanges;                 ///< Genre changes
        float adaptationAccuracy;              ///< Adaptation accuracy (0-1)

        // Performance metrics
        double updateTime;                     ///< Average update time (ms)
        double transitionTime;                 ///< Average transition time (ms)
        uint32_t memoryUsage;                  ///< Memory usage (bytes)
        uint32_t loadedTracks;                 ///< Number of loaded tracks
    };

    /**
     * @class MusicSystem
     * @brief Advanced dynamic music system
     *
     * The MusicSystem class provides comprehensive dynamic music management for the
     * VoxelCraft game engine, featuring adaptive music, procedural composition, mood-
     * based transitions, layered audio, and seamless integration with game systems for
     * immersive audio experiences that respond to gameplay and player actions.
     *
     * Key Features:
     * - Adaptive music that responds to game state
     * - Multiple music layers for dynamic composition
     * - Seamless transitions and crossfading
     * - Procedural music generation
     * - Context-aware music selection
     * - Beat-synchronized transitions
     * - Layered audio mixing
     * - Mood and intensity-based adaptation
     * - Playlist management and shuffling
     * - Real-time music parameter adjustment
     * - Performance monitoring and optimization
     * - Memory management and streaming
     * - Integration with audio engine
     *
     * The music system is designed to create emotionally engaging audio experiences
     * that enhance gameplay and immersion through intelligent music adaptation.
     */
    class MusicSystem {
    public:
        /**
         * @brief Constructor
         * @param audioEngine Audio engine instance
         */
        explicit MusicSystem(AudioEngine* audioEngine);

        /**
         * @brief Destructor
         */
        ~MusicSystem();

        /**
         * @brief Deleted copy constructor
         */
        MusicSystem(const MusicSystem&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        MusicSystem& operator=(const MusicSystem&) = delete;

        // Music system lifecycle

        /**
         * @brief Initialize music system
         * @return true if successful, false otherwise
         */
        bool Initialize();

        /**
         * @brief Shutdown music system
         */
        void Shutdown();

        /**
         * @brief Update music system
         * @param deltaTime Time elapsed since last update
         */
        void Update(double deltaTime);

        /**
         * @brief Get music system state
         * @return Current music state
         */
        MusicState GetState() const { return m_state; }

        /**
         * @brief Get music statistics
         * @return Current music statistics
         */
        const MusicStats& GetStats() const { return m_stats; }

        // Music track management

        /**
         * @brief Load music track
         * @param name Track name
         * @param filePath File path
         * @param genre Music genre
         * @param mood Music mood
         * @return Track ID or 0 if failed
         */
        uint32_t LoadMusicTrack(const std::string& name, const std::string& filePath,
                              MusicGenre genre = MusicGenre::Ambient,
                              MusicMood mood = MusicMood::Normal);

        /**
         * @brief Unload music track
         * @param trackId Track ID
         * @return true if successful, false otherwise
         */
        bool UnloadMusicTrack(uint32_t trackId);

        /**
         * @brief Get music track
         * @param trackId Track ID
         * @return Music track or nullopt if not found
         */
        std::optional<MusicTrack> GetMusicTrack(uint32_t trackId) const;

        /**
         * @brief Get music track by name
         * @param name Track name
         * @return Track ID or 0 if not found
         */
        uint32_t GetMusicTrackByName(const std::string& name) const;

        // Music playback

        /**
         * @brief Play music track
         * @param trackId Track ID
         * @param fadeInTime Fade in time (seconds)
         * @return true if successful, false otherwise
         */
        bool PlayMusic(uint32_t trackId, float fadeInTime = 2.0f);

        /**
         * @brief Play music by name
         * @param trackName Track name
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
         * @brief Pause music
         * @return true if successful, false otherwise
         */
        bool PauseMusic();

        /**
         * @brief Resume music
         * @return true if successful, false otherwise
         */
        bool ResumeMusic();

        /**
         * @brief Check if music is playing
         * @return true if playing, false otherwise
         */
        bool IsMusicPlaying() const;

        // Music transitions

        /**
         * @brief Transition to new track
         * @param trackId Target track ID
         * @param transitionType Transition type
         * @param duration Transition duration (seconds)
         * @return true if successful, false otherwise
         */
        bool TransitionToTrack(uint32_t trackId, MusicTransition transitionType = MusicTransition::Crossfade,
                             float duration = 3.0f);

        /**
         * @brief Crossfade to new track
         * @param trackId Target track ID
         * @param crossfadeTime Crossfade duration (seconds)
         * @return true if successful, false otherwise
         */
        bool CrossfadeToTrack(uint32_t trackId, float crossfadeTime = 3.0f);

        /**
         * @brief Beat-sync transition
         * @param trackId Target track ID
         * @return true if successful, false otherwise
         */
        bool BeatSyncTransition(uint32_t trackId);

        // Music layers

        /**
         * @brief Add music layer
         * @param layerConfig Layer configuration
         * @return Layer ID or 0 if failed
         */
        uint32_t AddMusicLayer(const MusicLayerConfig& layerConfig);

        /**
         * @brief Remove music layer
         * @param layerId Layer ID
         * @return true if successful, false otherwise
         */
        bool RemoveMusicLayer(uint32_t layerId);

        /**
         * @brief Set layer volume
         * @param layerId Layer ID
         * @param volume Volume level (0.0 - 1.0)
         * @param fadeTime Fade duration (seconds)
         * @return true if successful, false otherwise
         */
        bool SetLayerVolume(uint32_t layerId, float volume, float fadeTime = 0.0f);

        /**
         * @brief Get layer configuration
         * @param layerId Layer ID
         * @return Layer configuration or nullopt if not found
         */
        std::optional<MusicLayerConfig> GetMusicLayer(uint32_t layerId) const;

        // Adaptive music

        /**
         * @brief Update music context
         * @param context New music context
         * @return true if successful, false otherwise
         */
        bool UpdateMusicContext(const MusicContext& context);

        /**
         * @brief Get current music context
         * @return Current music context
         */
        const MusicContext& GetMusicContext() const { return m_context; }

        /**
         * @brief Enable adaptive music
         * @param enabled Enable state
         * @return true if successful, false otherwise
         */
        bool EnableAdaptiveMusic(bool enabled);

        /**
         * @brief Set adaptive music parameters
         * @param sensitivity Context sensitivity (0.0 - 1.0)
         * @param transitionSpeed Transition speed (0.0 - 1.0)
         * @return true if successful, false otherwise
         */
        bool SetAdaptiveParameters(float sensitivity = 0.5f, float transitionSpeed = 0.5f);

        // Playlist management

        /**
         * @brief Create music playlist
         * @param name Playlist name
         * @param trackIds Track IDs
         * @param mode Playlist mode
         * @return Playlist ID or 0 if failed
         */
        uint32_t CreatePlaylist(const std::string& name, const std::vector<uint32_t>& trackIds,
                              PlaylistMode mode = PlaylistMode::Sequential);

        /**
         * @brief Play music playlist
         * @param playlistId Playlist ID
         * @return true if successful, false otherwise
         */
        bool PlayPlaylist(uint32_t playlistId);

        /**
         * @brief Stop playlist
         * @return true if successful, false otherwise
         */
        bool StopPlaylist();

        /**
         * @brief Get current playlist
         * @return Current playlist ID
         */
        uint32_t GetCurrentPlaylist() const { return m_currentPlaylist; }

        // Music parameters

        /**
         * @brief Set master music volume
         * @param volume Volume level (0.0 - 1.0)
         * @return true if successful, false otherwise
         */
        bool SetMusicVolume(float volume);

        /**
         * @brief Get master music volume
         * @return Current volume level
         */
        float GetMusicVolume() const { return m_masterVolume; }

        /**
         * @brief Set music pitch
         * @param pitch Pitch level (0.5 - 2.0)
         * @return true if successful, false otherwise
         */
        bool SetMusicPitch(float pitch);

        /**
         * @brief Get current music pitch
         * @return Current pitch level
         */
        float GetMusicPitch() const { return m_currentPitch; }

        // Procedural music

        /**
         * @brief Generate procedural music
         * @param genre Music genre
         * @param mood Music mood
         * @param duration Duration (seconds)
         * @return Generated track ID or 0 if failed
         */
        uint32_t GenerateProceduralMusic(MusicGenre genre, MusicMood mood, float duration = 60.0f);

        /**
         * @brief Enable procedural music generation
         * @param enabled Enable state
         * @return true if successful, false otherwise
         */
        bool EnableProceduralMusic(bool enabled);

        // Utility functions

        /**
         * @brief Get music track names
         * @return Vector of track names
         */
        std::vector<std::string> GetMusicTrackNames() const;

        /**
         * @brief Check if track exists
         * @param trackName Track name
         * @return true if exists, false otherwise
         */
        bool MusicTrackExists(const std::string& trackName) const;

        /**
         * @brief Get current track info
         * @return Current track information
         */
        std::string GetCurrentTrackInfo() const;

        /**
         * @brief Get music system status
         * @return Detailed status report
         */
        std::string GetStatusReport() const;

        /**
         * @brief Get performance report
         * @return Detailed performance report
         */
        std::string GetPerformanceReport() const;

        /**
         * @brief Optimize music system
         * @return Optimization results
         */
        std::unordered_map<std::string, size_t> Optimize();

    private:
        /**
         * @brief Initialize music layers
         * @return true if successful, false otherwise
         */
        bool InitializeMusicLayers();

        /**
         * @brief Update music playback
         * @param deltaTime Time elapsed
         */
        void UpdateMusicPlayback(double deltaTime);

        /**
         * @brief Update music transitions
         * @param deltaTime Time elapsed
         */
        void UpdateMusicTransitions(double deltaTime);

        /**
         * @brief Update adaptive music
         * @param deltaTime Time elapsed
         */
        void UpdateAdaptiveMusic(double deltaTime);

        /**
         * @brief Update music layers
         * @param deltaTime Time elapsed
         */
        void UpdateMusicLayers(double deltaTime);

        /**
         * @brief Perform music transition
         * @param targetTrackId Target track ID
         * @param transitionType Transition type
         * @param duration Transition duration
         */
        void PerformTransition(uint32_t targetTrackId, MusicTransition transitionType, float duration);

        /**
         * @brief Select appropriate track for context
         * @param context Music context
         * @return Selected track ID
         */
        uint32_t SelectTrackForContext(const MusicContext& context);

        /**
         * @brief Calculate track suitability score
         * @param trackId Track ID
         * @param context Music context
         * @return Suitability score (0.0 - 1.0)
         */
        float CalculateTrackSuitability(uint32_t trackId, const MusicContext& context);

        /**
         * @brief Update music context from game state
         */
        void UpdateContextFromGameState();

        /**
         * @brief Generate procedural music segment
         * @param genre Music genre
         * @param mood Music mood
         * @param duration Duration
         * @return Generated audio data
         */
        std::vector<uint8_t> GenerateProceduralSegment(MusicGenre genre, MusicMood mood, float duration);

        /**
         * @brief Update performance statistics
         * @param deltaTime Time elapsed
         */
        void UpdateStats(double deltaTime);

        /**
         * @brief Handle music system errors
         * @param error Error message
         */
        void HandleError(const std::string& error);

        // Music system data
        AudioEngine* m_audioEngine;                    ///< Audio engine instance
        MusicStats m_stats;                            ///< Performance statistics

        // Music state
        MusicState m_state;                            ///< Current music state
        MusicContext m_context;                        ///< Current music context
        uint32_t m_currentTrackId;                     ///< Currently playing track
        uint32_t m_nextTrackId;                        ///< Next track for transition
        uint32_t m_currentPlaylist;                    ///< Current playlist ID

        // Music tracks
        std::unordered_map<uint32_t, MusicTrack> m_musicTracks; ///< Loaded music tracks
        std::unordered_map<std::string, uint32_t> m_trackNames; ///< Track name mapping
        mutable std::shared_mutex m_tracksMutex;       ///< Tracks synchronization

        // Music layers
        std::unordered_map<uint32_t, MusicLayerConfig> m_musicLayers; ///< Music layers
        mutable std::shared_mutex m_layersMutex;       ///< Layers synchronization

        // Playlists
        std::unordered_map<uint32_t, MusicPlaylist> m_playlists; ///< Music playlists
        mutable std::shared_mutex m_playlistsMutex;    ///< Playlists synchronization

        // Audio sources
        uint32_t m_primarySource;                      ///< Primary audio source
        uint32_t m_secondarySource;                    ///< Secondary source for transitions
        std::unordered_map<uint32_t, uint32_t> m_layerSources; ///< Layer audio sources

        // Transition state
        MusicTransition m_currentTransition;           ///< Current transition type
        float m_transitionProgress;                    ///< Transition progress (0.0 - 1.0)
        float m_transitionDuration;                    ///< Transition duration
        double m_transitionStartTime;                  ///< Transition start time

        // Adaptive music
        bool m_adaptiveEnabled;                        ///< Adaptive music enabled
        float m_adaptiveSensitivity;                   ///< Context sensitivity
        float m_adaptiveTransitionSpeed;               ///< Transition speed
        double m_lastContextUpdate;                    ///< Last context update

        // Volume and parameters
        float m_masterVolume;                          ///< Master music volume
        float m_currentVolume;                         ///< Current volume
        float m_targetVolume;                          ///< Target volume for fading
        float m_currentPitch;                          ///< Current pitch

        // Procedural music
        bool m_proceduralEnabled;                      ///< Procedural music enabled
        std::mt19937 m_randomEngine;                   ///< Random number generator

        // State tracking
        bool m_isInitialized;                          ///< System is initialized
        double m_lastUpdateTime;                       ///< Last update time
        std::string m_lastError;                       ///< Last error message

        static std::atomic<uint32_t> s_nextTrackId;    ///< Next track ID counter
        static std::atomic<uint32_t> s_nextLayerId;    ///< Next layer ID counter
        static std::atomic<uint32_t> s_nextPlaylistId; ///< Next playlist ID counter
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_AUDIO_MUSIC_SYSTEM_HPP
