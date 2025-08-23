#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
#include <functional>
#include <mutex>
#include <atomic>
#include <thread>
#include <queue>
#include <condition_variable>
#include <chrono>

namespace VoxelCraft {

    class AudioEngine;
    class SoundGenerator;
    class AudioSource;
    class AudioListener;
    class SoundBank;
    class AudioEffect;
    class SpatialAudioSystem;
    class AudioEffectProcessor;
    struct SpatialAudioSource;
    struct SpatialAudioListener;
    struct SpatialAudioEnvironment;

    /**
     * @enum AudioFormat
     * @brief Formatos de audio soportados
     */
    enum class AudioFormat {
        WAV = 0,       ///< Formato WAV
        MP3,           ///< Formato MP3
        OGG,           ///< Formato OGG Vorbis
        FLAC,          ///< Formato FLAC
        OPUS,          ///< Formato OPUS
        RAW            ///< Audio raw
    };

    /**
     * @enum AudioChannel
     * @brief Configuraciones de canales de audio
     */
    enum class AudioChannel {
        MONO = 1,      ///< Mono
        STEREO = 2,    ///< Estéreo
        QUAD = 4,      ///< Cuadrifónico
        SURROUND_5_1 = 6,  ///< 5.1 Surround
        SURROUND_7_1 = 8,  ///< 7.1 Surround
        DOLBY_ATMOS = 64   ///< Dolby Atmos (simplificado)
    };

    /**
     * @enum AudioPriority
     * @brief Prioridades de reproducción de audio
     */
    enum class AudioPriority {
        LOWEST = 0,    ///< Prioridad más baja
        LOW,           ///< Prioridad baja
        NORMAL,        ///< Prioridad normal
        HIGH,          ///< Prioridad alta
        HIGHEST        ///< Prioridad más alta
    };

    /**
     * @enum SoundCategory
     * @brief Categorías de sonidos del juego
     */
    enum class SoundCategory {
        MASTER = 0,        ///< Volumen maestro
        MUSIC,             ///< Música de fondo
        AMBIENT,           ///< Sonidos ambientales
        WEATHER,           ///< Sonidos del clima
        BLOCKS,            ///< Sonidos de bloques
        ENTITIES,          ///< Sonidos de entidades
        PLAYERS,           ///< Sonidos de jugadores
        HOSTILE,           ///< Sonidos de mobs hostiles
        NEUTRAL,           ///< Sonidos de mobs neutrales
        PASSIVE,           ///< Sonidos de mobs pasivos
        RECORD,            ///< Música de discos
        VOICE,             ///< Chat de voz
        INTERFACE,         ///< Sonidos de interfaz
        PROCEDURAL         ///< Sonidos procedurales
    };

    /**
     * @struct AudioConfig
     * @brief Configuración del sistema de audio
     */
    struct AudioConfig {
        int sampleRate = 44100;                    ///< Sample rate en Hz
        AudioChannel channels = AudioChannel::STEREO;  ///< Canales de audio
        int bufferSize = 1024;                     ///< Tamaño del buffer
        float masterVolume = 1.0f;                 ///< Volumen maestro (0.0 - 1.0)
        float musicVolume = 0.5f;                  ///< Volumen de música
        float soundVolume = 1.0f;                  ///< Volumen de efectos
        int maxAudioSources = 64;                  ///< Máximo de fuentes de audio
        int maxConcurrentSounds = 32;              ///< Máximo de sonidos simultáneos
        bool enable3DAudio = true;                 ///< Habilitar audio 3D
        bool enableReverb = true;                  ///< Habilitar reverberación
        bool enableDoppler = true;                 ///< Habilitar efecto Doppler
        bool enableHRTF = false;                   ///< Habilitar HRTF (si disponible)
        float soundAttenuation = 1.0f;             ///< Atenuación de sonido
        float maxSoundDistance = 64.0f;            ///< Distancia máxima de sonido
        float rolloffFactor = 1.0f;                ///< Factor de rolloff
        bool enableCompression = true;             ///< Habilitar compresión de audio
        bool enableProceduralAudio = true;         ///< Habilitar audio procedural
        std::string audioDevice = "default";       ///< Dispositivo de audio
    };

    /**
     * @struct AudioStats
     * @brief Estadísticas del sistema de audio
     */
    struct AudioStats {
        int activeSources = 0;                     ///< Fuentes de audio activas
        int totalSources = 0;                      ///< Total de fuentes creadas
        int playingSounds = 0;                     ///< Sonidos reproduciéndose
        float cpuUsage = 0.0f;                     ///< Uso de CPU del audio
        size_t memoryUsage = 0;                    ///< Memoria usada por audio
        int droppedSounds = 0;                     ///< Sonidos descartados
        std::chrono::steady_clock::time_point lastUpdate; ///< Última actualización
    };

    /**
     * @struct AudioSourceConfig
     * @brief Configuración de una fuente de audio
     */
    struct AudioSourceConfig {
        std::string soundName;                     ///< Nombre del sonido
        glm::vec3 position = glm::vec3(0.0f);      ///< Posición 3D
        glm::vec3 velocity = glm::vec3(0.0f);      ///< Velocidad para Doppler
        float volume = 1.0f;                       ///< Volumen (0.0 - 1.0)
        float pitch = 1.0f;                        ///< Pitch (0.5 - 2.0)
        bool loop = false;                         ///< Reproducir en bucle
        bool spatial = true;                       ///< Audio espacial
        float minDistance = 1.0f;                  ///< Distancia mínima
        float maxDistance = 64.0f;                 ///< Distancia máxima
        AudioPriority priority = AudioPriority::NORMAL; ///< Prioridad
        SoundCategory category = SoundCategory::MASTER; ///< Categoría
        std::function<void()> onComplete;          ///< Callback al completar
    };

    /**
     * @class AudioManager
     * @brief Gestor central del sistema de audio 3D
     */
    class AudioManager {
    public:
        static AudioManager& GetInstance();

        /**
         * @brief Inicializar el sistema de audio
         * @param config Configuración de audio
         * @return true si la inicialización fue exitosa
         */
        bool Initialize(const AudioConfig& config);

        /**
         * @brief Apagar el sistema de audio
         */
        void Shutdown();

        /**
         * @brief Actualizar el estado del audio (llamar cada frame)
         */
        void Update();

        // Configuración
        void SetConfig(const AudioConfig& config) { m_config = config; }
        const AudioConfig& GetConfig() const { return m_config; }

        // Estado
        bool IsInitialized() const { return m_initialized; }
        const AudioStats& GetStats() const { return m_stats; }

        // Control de volumen por categoría
        void SetMasterVolume(float volume);
        void SetCategoryVolume(SoundCategory category, float volume);
        float GetMasterVolume() const;
        float GetCategoryVolume(SoundCategory category) const;

        // Reproducción de audio
        std::shared_ptr<AudioSource> PlaySound(const AudioSourceConfig& config);
        std::shared_ptr<AudioSource> PlaySound(const std::string& soundName,
                                             const glm::vec3& position = glm::vec3(0.0f),
                                             float volume = 1.0f);
        std::shared_ptr<AudioSource> PlayMusic(const std::string& musicName,
                                             float volume = 0.5f,
                                             bool loop = true);
        std::shared_ptr<AudioSource> PlayAmbient(const std::string& ambientName,
                                               const glm::vec3& position = glm::vec3(0.0f),
                                               float volume = 1.0f);

        // Control de audio
        void StopAll();
        void StopCategory(SoundCategory category);
        void PauseAll();
        void PauseCategory(SoundCategory category);
        void ResumeAll();
        void ResumeCategory(SoundCategory category);

        // Listener 3D
        void SetListenerPosition(const glm::vec3& position);
        void SetListenerOrientation(const glm::vec3& forward, const glm::vec3& up);
        void SetListenerVelocity(const glm::vec3& velocity);
        const AudioListener* GetListener() const { return m_listener.get(); }

        // Gestión de sonidos
        bool LoadSound(const std::string& name, const std::string& filePath);
        bool UnloadSound(const std::string& name);
        bool IsSoundLoaded(const std::string& name) const;
        std::vector<std::string> GetLoadedSounds() const;

        // Audio procedural
        bool GenerateSound(const std::string& name, const std::string& type, float duration = 1.0f);
        bool GenerateMusic(const std::string& name, const std::string& style, float duration = 60.0f);
        bool GenerateAmbient(const std::string& name, const std::string& biome, float duration = 30.0f);

        // Audio espacial 3D
        std::shared_ptr<AudioSource> PlaySound3D(const AudioSourceConfig& config);
        std::shared_ptr<AudioSource> PlaySound3D(const std::string& soundName,
                                                const glm::vec3& position,
                                                float volume = 1.0f);
        void SetListenerPosition(const glm::vec3& position);
        void SetListenerOrientation(const glm::vec3& forward, const glm::vec3& up);
        void SetListenerVelocity(const glm::vec3& velocity);
        void SetEnvironment(const std::string& environmentType);

        // Efectos de audio
        std::shared_ptr<AudioEffect> CreateEffect(const std::string& effectType,
                                                const std::string& name);
        bool ApplyEffectToSource(std::shared_ptr<AudioSource> source,
                               std::shared_ptr<AudioEffect> effect);
        bool RemoveEffectFromSource(std::shared_ptr<AudioSource> source,
                                  const std::string& effectName);

        // Efectos de audio
        std::shared_ptr<AudioEffect> CreateReverbEffect(float decayTime = 1.5f, float dampening = 0.5f);
        std::shared_ptr<AudioEffect> CreateEchoEffect(float delay = 0.3f, float decay = 0.5f);
        std::shared_ptr<AudioEffect> CreateLowPassEffect(float cutoff = 1000.0f);
        std::shared_ptr<AudioEffect> CreateHighPassEffect(float cutoff = 100.0f);

        // Callbacks de eventos
        using AudioEventCallback = std::function<void(const std::string& event, const std::string& soundName)>;
        void SetEventCallback(const AudioEventCallback& callback) { m_eventCallback = callback; }

        // Debug y diagnóstico
        void EnableDebugMode(bool enable) { m_debugMode = enable; }
        bool IsDebugModeEnabled() const { return m_debugMode; }
        std::string GetDebugInfo() const;

        // Optimización
        void PreloadSounds(const std::vector<std::string>& soundNames);
        void UnloadUnusedSounds();
        void OptimizeMemoryUsage();

    private:
        AudioManager() = default;
        ~AudioManager();

        // Prevent copying
        AudioManager(const AudioManager&) = delete;
        AudioManager& operator=(const AudioManager&) = delete;

        // Componentes del sistema de audio
        std::unique_ptr<AudioEngine> m_audioEngine;
        std::unique_ptr<SoundGenerator> m_soundGenerator;
        std::unique_ptr<AudioListener> m_listener;
        std::unique_ptr<SoundBank> m_soundBank;
        std::unique_ptr<SpatialAudioSystem> m_spatialAudioSystem;
        std::unique_ptr<AudioEffectProcessor> m_effectProcessor;

        // Configuración y estado
        AudioConfig m_config;
        AudioStats m_stats;
        std::atomic<bool> m_initialized;
        bool m_debugMode = false;

        // Gestión de fuentes de audio
        std::vector<std::shared_ptr<AudioSource>> m_audioSources;
        std::unordered_map<SoundCategory, float> m_categoryVolumes;
        mutable std::shared_mutex m_audioMutex;

        // Callbacks
        AudioEventCallback m_eventCallback;

        // Métodos privados
        std::shared_ptr<AudioSource> CreateAudioSource();
        void UpdateAudioSources();
        void CleanupFinishedSources();
        bool ValidateAudioConfig(const AudioConfig& config);
        void ApplyCategoryVolumes();
        void LogAudioEvent(const std::string& event, const std::string& soundName = "");
    };

} // namespace VoxelCraft