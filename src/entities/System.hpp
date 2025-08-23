#pragma once

#include <memory>
#include <string>
#include <vector>
#include <chrono>
#include <shared_mutex>

namespace VoxelCraft {

    class EntityManager;

    /**
     * @brief Clase base para todos los sistemas del ECS
     */
    class System {
    public:
        using SystemID = uint32_t;

        enum class SystemState {
            INITIALIZED,
            RUNNING,
            PAUSED,
            DESTROYED
        };

        System();
        virtual ~System();

        // Getters básicos
        SystemID GetID() const { return m_ID; }
        const std::string& GetName() const { return m_Name; }
        EntityManager* GetManager() const { return m_Manager; }
        SystemState GetState() const { return m_State; }
        bool IsRunning() const { return m_State == SystemState::RUNNING; }

        // Gestión de estado
        void SetPaused(bool paused);
        void Destroy();
        void SetName(const std::string& name) { m_Name = name; }

        // Gestión del EntityManager
        void SetManager(EntityManager* manager) { m_Manager = manager; }

        // Métodos de actualización
        virtual void Update(float deltaTime) {}
        virtual void FixedUpdate(float fixedDeltaTime) {}
        virtual void LateUpdate(float deltaTime) {}

        // Callbacks de ciclo de vida
        virtual void OnInit() {}
        virtual void OnStart() {}
        virtual void OnPause() {}
        virtual void OnResume() {}
        virtual void OnDestroy() {}

        // Utilidades
        std::string ToString() const;
        virtual std::type_index GetType() const { return typeid(*this); }

    protected:
        SystemID m_ID;
        std::string m_Name;
        EntityManager* m_Manager;
        SystemState m_State;

        static SystemID GenerateID();
        static SystemID s_NextID;
    };

    // Sistemas específicos del juego

    /**
     * @brief Sistema de transformación para manejar posiciones, rotaciones y escalas
     */
    class TransformSystem : public System {
    public:
        TransformSystem();
        ~TransformSystem() override = default;

        void Update(float deltaTime) override;
        void FixedUpdate(float fixedDeltaTime) override;

        void OnInit() override;

    private:
        void UpdateTransformMatrices();
        void HandleParenting();
        void UpdateWorldMatrices();
    };

    /**
     * @brief Sistema de físicas para manejar colisiones y movimiento
     */
    class PhysicsSystem : public System {
    public:
        PhysicsSystem();
        ~PhysicsSystem() override = default;

        void Update(float deltaTime) override;
        void FixedUpdate(float fixedDeltaTime) override;

        void OnInit() override;

        // Configuración de físicas
        void SetGravity(const glm::vec3& gravity) { m_Gravity = gravity; }
        void SetFixedTimeStep(float timeStep) { m_FixedTimeStep = timeStep; }

    private:
        glm::vec3 m_Gravity{0.0f, -9.81f, 0.0f};
        float m_FixedTimeStep{1.0f / 60.0f};

        void IntegrateVelocities();
        void ResolveCollisions();
        void ApplyConstraints();
    };

    /**
     * @brief Sistema de renderizado para dibujar entidades
     */
    class RenderSystem : public System {
    public:
        RenderSystem();
        ~RenderSystem() override = default;

        void Update(float deltaTime) override;
        void LateUpdate(float deltaTime) override;

        void OnInit() override;

        // Configuración de renderizado
        void SetRenderDistance(float distance) { m_RenderDistance = distance; }
        void SetFrustumCulling(bool enabled) { m_FrustumCulling = enabled; }

    private:
        float m_RenderDistance{1000.0f};
        bool m_FrustumCulling{true};

        void CollectVisibleEntities();
        void SortByRenderOrder();
        void SubmitToRenderer();
        void HandleLevelOfDetail();
    };

    /**
     * @brief Sistema de IA para entidades con comportamiento
     */
    class AISystem : public System {
    public:
        AISystem();
        ~AISystem() override = default;

        void Update(float deltaTime) override;
        void FixedUpdate(float fixedDeltaTime) override;

        void OnInit() override;

        // Configuración de IA
        void SetUpdateFrequency(float frequency) { m_UpdateFrequency = frequency; }
        void SetMaxAIEntities(size_t max) { m_MaxAIEntities = max; }

    private:
        float m_UpdateFrequency{0.1f};  // 10 updates por segundo
        size_t m_MaxAIEntities{1000};
        float m_AccumulatedTime{0.0f};

        void UpdateAIEntities();
        void ProcessBehaviors();
        void HandlePathfinding();
        void ManageBehaviorTrees();
    };

    /**
     * @brief Sistema de audio para entidades con sonido
     */
    class AudioSystem : public System {
    public:
        AudioSystem();
        ~AudioSystem() override = default;

        void Update(float deltaTime) override;
        void LateUpdate(float deltaTime) override;

        void OnInit() override;

        // Configuración de audio
        void SetMaxAudioSources(size_t max) { m_MaxAudioSources = max; }
        void SetListenerPosition(const glm::vec3& position) { m_ListenerPosition = position; }

    private:
        size_t m_MaxAudioSources{32};
        glm::vec3 m_ListenerPosition{0.0f};

        void Update3DAudio();
        void ManageAudioSources();
        void HandleSpatialAudio();
        void ProcessAudioEvents();
    };

    /**
     * @brief Sistema de partículas para efectos visuales
     */
    class ParticleSystem : public System {
    public:
        ParticleSystem();
        ~ParticleSystem() override = default;

        void Update(float deltaTime) override;
        void FixedUpdate(float fixedDeltaTime) override;

        void OnInit() override;

        // Configuración de partículas
        void SetMaxParticles(size_t max) { m_MaxParticles = max; }
        void SetParticleBudget(size_t budget) { m_ParticleBudget = budget; }

    private:
        size_t m_MaxParticles{10000};
        size_t m_ParticleBudget{1000};

        void UpdateParticles();
        void EmitNewParticles();
        void SimulatePhysics();
        void HandleCollisions();
    };

}