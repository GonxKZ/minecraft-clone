/**
 * @file ParticleSystem.hpp
 * @brief VoxelCraft Advanced Particle System with GPU Acceleration
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines the ParticleSystem class that provides comprehensive particle
 * effects for the VoxelCraft game engine, including GPU-accelerated particle simulation,
 * advanced rendering techniques, collision detection, and performance optimization
 * for complex visual effects and environmental phenomena.
 */

#ifndef VOXELCRAFT_GRAPHICS_PARTICLE_SYSTEM_HPP
#define VOXELCRAFT_GRAPHICS_PARTICLE_SYSTEM_HPP

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
#include <random>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../core/Config.hpp"

namespace VoxelCraft {

    // Forward declarations
    class Renderer;
    class Material;
    class ShaderManager;
    class TextureManager;

    /**
     * @enum ParticleSimulation
     * @brief Particle simulation methods
     */
    enum class ParticleSimulation {
        CPU,                     ///< CPU-based simulation
        GPU,                     ///< GPU compute shader simulation
        Hybrid                   ///< Hybrid CPU/GPU simulation
    };

    /**
     * @enum ParticleRenderMode
     * @brief Particle rendering modes
     */
    enum class ParticleRenderMode {
        Billboard,               ///< Billboard rendering
        Mesh,                    ///< Mesh-based particles
        Ribbon,                  ///< Ribbon/trail particles
        Volume,                  ///< Volumetric particles
        Custom                   ///< Custom rendering mode
    };

    /**
     * @enum ParticleEmitterShape
     * @brief Particle emitter shapes
     */
    enum class ParticleEmitterShape {
        Point,                   ///< Point emitter
        Sphere,                  ///< Spherical emitter
        Box,                     ///< Box/cube emitter
        Cylinder,                ///< Cylindrical emitter
        Cone,                    ///< Conical emitter
        Torus,                   ///< Torus/donut emitter
        Mesh,                    ///< Mesh surface emitter
        Custom                   ///< Custom shape emitter
    };

    /**
     * @enum ParticleBlendMode
     * @brief Particle blending modes
     */
    enum class ParticleBlendMode {
        Opaque,                  ///< Opaque particles
        Alpha,                   ///< Alpha blending
        Additive,                ///< Additive blending
        Multiply,                ///< Multiply blending
        Screen,                  ///< Screen blending
        Custom                   ///< Custom blend mode
    };

    /**
     * @enum ForceFieldType
     * @brief Force field types for particle simulation
     */
    enum class ForceFieldType {
        Gravity,                 ///< Gravitational force
        Wind,                    ///< Wind force
        Vortex,                  ///< Vortex force field
        Turbulence,              ///< Turbulence field
        Magnetic,                ///< Magnetic force field
        Custom                   ///< Custom force field
    };

    /**
     * @enum ParticleQuality
     * @brief Particle system quality levels
     */
    enum class ParticleQuality {
        Low,                     ///< Low quality for performance
        Medium,                  ///< Medium quality balance
        High,                    ///< High quality for modern GPUs
        Ultra,                   ///< Ultra quality with advanced features
        Custom                   ///< Custom quality settings
    };

    /**
     * @struct ParticleConfig
     * @brief Particle system configuration
     */
    struct ParticleConfig {
        // Basic settings
        std::string name;                            ///< Particle system name
        uint32_t maxParticles;                       ///< Maximum number of particles
        float duration;                              ///< System duration (0 = infinite)
        bool looping;                                ///< Loop system when duration ends
        float startDelay;                            ///< Start delay in seconds

        // Emission settings
        float emissionRate;                          ///< Particles per second
        uint32_t burstCount;                         ///< Burst emission count
        float burstInterval;                         ///< Burst interval
        ParticleEmitterShape emitterShape;           ///< Emitter shape
        glm::vec3 emitterSize;                       ///< Emitter size
        glm::vec3 emitterPosition;                   ///< Emitter position
        glm::quat emitterRotation;                   ///< Emitter rotation

        // Particle properties
        float lifetime;                              ///< Particle lifetime (seconds)
        float lifetimeVariance;                      ///< Lifetime variance
        glm::vec3 initialVelocity;                   ///< Initial velocity
        glm::vec3 velocityVariance;                  ///< Velocity variance
        float initialSize;                           ///< Initial particle size
        float sizeVariance;                          ///< Size variance
        float initialRotation;                       ///< Initial rotation
        float rotationVariance;                      ///< Rotation variance
        glm::vec4 initialColor;                      ///< Initial color
        glm::vec4 colorVariance;                     ///< Color variance

        // Rendering settings
        ParticleRenderMode renderMode;               ///< Rendering mode
        ParticleBlendMode blendMode;                 ///< Blending mode
        uint32_t textureAtlas;                       ///< Texture atlas ID
        glm::vec2 textureSize;                       ///< Texture size in atlas
        bool enableSorting;                          ///< Enable particle sorting
        bool enableShadows;                          ///< Enable shadow casting
        bool enableLighting;                         ///< Enable lighting

        // Physics settings
        float mass;                                  ///< Particle mass
        float drag;                                  ///< Air drag coefficient
        bool enableCollision;                        ///< Enable collision detection
        float bounceFactor;                          ///< Bounce factor on collision
        float friction;                              ///< Friction coefficient

        // Animation settings
        bool enableSizeAnimation;                    ///< Enable size animation
        glm::vec2 sizeCurve;                         ///< Size animation curve
        bool enableColorAnimation;                   ///< Enable color animation
        std::vector<glm::vec4> colorCurve;           ///< Color animation curve
        bool enableRotationAnimation;                ///< Enable rotation animation
        float rotationSpeed;                         ///< Rotation speed
        float rotationSpeedVariance;                 ///< Rotation speed variance

        // Advanced settings
        ParticleSimulation simulation;               ///< Simulation method
        ParticleQuality quality;                     ///< Quality level
        bool enableGPUAcceleration;                  ///< Enable GPU acceleration
        bool enableSubpixelRendering;                ///< Enable subpixel rendering
        int maxSubpixelParticles;                    ///< Maximum subpixel particles

        // Performance settings
        bool enableCulling;                          ///< Enable frustum culling
        float cullDistance;                          ///< Culling distance
        bool enableLOD;                              ///< Enable level of detail
        int lodLevel;                                ///< Current LOD level
        float lodDistance;                           ///< LOD transition distance

        ParticleConfig()
            : maxParticles(1000)
            , duration(0.0f)
            , looping(false)
            , startDelay(0.0f)
            , emissionRate(100.0f)
            , burstCount(0)
            , burstInterval(1.0f)
            , emitterShape(ParticleEmitterShape::Point)
            , emitterSize(1.0f, 1.0f, 1.0f)
            , emitterPosition(0.0f, 0.0f, 0.0f)
            , emitterRotation(1.0f, 0.0f, 0.0f, 0.0f)
            , lifetime(5.0f)
            , lifetimeVariance(1.0f)
            , initialVelocity(0.0f, 1.0f, 0.0f)
            , velocityVariance(0.5f, 0.5f, 0.5f)
            , initialSize(1.0f)
            , sizeVariance(0.2f)
            , initialRotation(0.0f)
            , rotationVariance(180.0f)
            , initialColor(1.0f, 1.0f, 1.0f, 1.0f)
            , colorVariance(0.1f, 0.1f, 0.1f, 0.1f)
            , renderMode(ParticleRenderMode::Billboard)
            , blendMode(ParticleBlendMode::Alpha)
            , textureAtlas(0)
            , textureSize(1.0f, 1.0f)
            , enableSorting(true)
            , enableShadows(false)
            , enableLighting(true)
            , mass(1.0f)
            , drag(0.1f)
            , enableCollision(false)
            , bounceFactor(0.5f)
            , friction(0.2f)
            , enableSizeAnimation(true)
            , sizeCurve(0.0f, 1.0f)
            , enableColorAnimation(true)
            , enableRotationAnimation(false)
            , rotationSpeed(0.0f)
            , rotationSpeedVariance(0.0f)
            , simulation(ParticleSimulation::GPU)
            , quality(ParticleQuality::High)
            , enableGPUAcceleration(true)
            , enableSubpixelRendering(false)
            , maxSubpixelParticles(100)
            , enableCulling(true)
            , cullDistance(100.0f)
            , enableLOD(false)
            , lodLevel(0)
            , lodDistance(50.0f)
        {}
    };

    /**
     * @struct Particle
     * @brief Individual particle data
     */
    struct Particle {
        // Position and motion
        glm::vec3 position;                          ///< Current position
        glm::vec3 velocity;                          ///< Current velocity
        glm::vec3 acceleration;                      ///< Current acceleration

        // Appearance
        float size;                                  ///< Current size
        float rotation;                              ///< Current rotation
        glm::vec4 color;                             ///< Current color

        // Animation
        float life;                                  ///< Current life (0-1)
        float lifetime;                              ///< Total lifetime
        float age;                                   ///< Age in seconds

        // Physics
        float mass;                                  ///< Particle mass
        bool active;                                 ///< Particle is active

        // Custom data
        std::unordered_map<std::string, std::any> customData; ///< Custom particle data

        Particle()
            : position(0.0f, 0.0f, 0.0f)
            , velocity(0.0f, 0.0f, 0.0f)
            , acceleration(0.0f, 0.0f, 0.0f)
            , size(1.0f)
            , rotation(0.0f)
            , color(1.0f, 1.0f, 1.0f, 1.0f)
            , life(1.0f)
            , lifetime(1.0f)
            , age(0.0f)
            , mass(1.0f)
            , active(false)
        {}
    };

    /**
     * @struct ForceField
     * @brief Force field for particle simulation
     */
    struct ForceField {
        ForceFieldType type;                          ///< Force field type
        glm::vec3 position;                           ///< Field position
        glm::vec3 direction;                          ///< Field direction
        float strength;                               ///< Field strength
        float radius;                                 ///< Field radius
        float falloff;                                ///< Field falloff
        bool enabled;                                 ///< Field is enabled

        // Type-specific parameters
        union {
            struct { // Gravity
                float gravitationalConstant;
            } gravity;

            struct { // Wind
                float turbulence;
                float gustStrength;
            } wind;

            struct { // Vortex
                float angularVelocity;
                float height;
            } vortex;

            struct { // Turbulence
                float noiseScale;
                float timeScale;
                int octaves;
            } turbulence;

            struct { // Magnetic
                float charge;
                float fieldStrength;
            } magnetic;
        };

        ForceField()
            : type(ForceFieldType::Gravity)
            , position(0.0f, 0.0f, 0.0f)
            , direction(0.0f, -1.0f, 0.0f)
            , strength(9.81f)
            , radius(10.0f)
            , falloff(1.0f)
            , enabled(true)
        {
            gravity.gravitationalConstant = 9.81f;
        }
    };

    /**
     * @struct ParticleEmitter
     * @brief Particle emitter configuration
     */
    struct ParticleEmitter {
        ParticleEmitterShape shape;                   ///< Emitter shape
        glm::vec3 position;                           ///< Emitter position
        glm::quat rotation;                           ///< Emitter rotation
        glm::vec3 size;                              ///< Emitter size
        float emissionRate;                          ///< Particles per second
        float timeSinceLastEmission;                 ///< Time since last emission
        bool enabled;                                ///< Emitter is enabled

        // Emission parameters
        float lifetime;                              ///< Particle lifetime
        float lifetimeVariance;                      ///< Lifetime variance
        glm::vec3 initialVelocity;                   ///< Initial velocity
        glm::vec3 velocityVariance;                  ///< Velocity variance
        float initialSize;                           ///< Initial size
        float sizeVariance;                          ///< Size variance
        float initialRotation;                       ///< Initial rotation
        float rotationVariance;                      ///< Rotation variance
        glm::vec4 initialColor;                      ///< Initial color
        glm::vec4 colorVariance;                     ///< Color variance

        ParticleEmitter()
            : shape(ParticleEmitterShape::Point)
            , position(0.0f, 0.0f, 0.0f)
            , rotation(1.0f, 0.0f, 0.0f, 0.0f)
            , size(1.0f, 1.0f, 1.0f)
            , emissionRate(100.0f)
            , timeSinceLastEmission(0.0f)
            , enabled(true)
            , lifetime(5.0f)
            , lifetimeVariance(1.0f)
            , initialVelocity(0.0f, 1.0f, 0.0f)
            , velocityVariance(0.5f, 0.5f, 0.5f)
            , initialSize(1.0f)
            , sizeVariance(0.2f)
            , initialRotation(0.0f)
            , rotationVariance(180.0f)
            , initialColor(1.0f, 1.0f, 1.0f, 1.0f)
            , colorVariance(0.1f, 0.1f, 0.1f, 0.1f)
        {}
    };

    /**
     * @struct ParticleStats
     * @brief Particle system performance statistics
     */
    struct ParticleStats {
        // Performance metrics
        uint64_t totalParticles;                     ///< Total particles created
        uint32_t activeParticles;                    ///< Currently active particles
        uint32_t maxActiveParticles;                 ///< Maximum active particles
        double totalSimulationTime;                  ///< Total simulation time (ms)
        double averageSimulationTime;                ///< Average simulation time (ms)

        // Emission metrics
        uint64_t particlesEmitted;                   ///< Total particles emitted
        uint64_t particlesDied;                      ///< Total particles that died
        float emissionRate;                          ///< Current emission rate
        float averageLifetime;                       ///< Average particle lifetime

        // GPU metrics
        uint64_t gpuParticles;                       ///< Particles simulated on GPU
        uint64_t cpuParticles;                       ///< Particles simulated on CPU
        size_t gpuMemoryUsage;                       ///< GPU memory usage
        size_t cpuMemoryUsage;                       ///< CPU memory usage

        // Rendering metrics
        uint32_t drawCalls;                          ///< Particle draw calls
        uint32_t trianglesRendered;                  ///< Triangles rendered
        uint32_t particlesRendered;                  ///< Particles rendered
        uint32_t culledParticles;                    ///< Particles culled

        // Quality metrics
        float averageFPS;                            ///< Average FPS
        float particleDensity;                       ///< Particle density
        float simulationAccuracy;                    ///< Simulation accuracy (0-1)
        float renderingQuality;                      ///< Rendering quality (0-1)
    };

    /**
     * @class ParticleSystem
     * @brief Advanced particle system with GPU acceleration
     *
     * The ParticleSystem class provides comprehensive particle effects for the
     * VoxelCraft game engine, featuring GPU-accelerated particle simulation,
     * advanced rendering techniques, collision detection, and performance optimization
     * for complex visual effects including smoke, fire, explosions, weather, and
     * environmental phenomena with support for millions of particles.
     *
     * Key Features:
     * - GPU-accelerated particle simulation using compute shaders
     * - Multiple emitter shapes and emission patterns
     * - Advanced particle rendering (billboards, meshes, ribbons, volumes)
     * - Force fields and physical simulation
     * - Collision detection with world geometry
     * - Particle animation curves and keyframe animation
     * - Level of detail and performance scaling
     * - Texture atlases and sprite animation
     * - Custom particle behaviors and events
     * - Hybrid CPU/GPU simulation modes
     * - Performance monitoring and optimization
     * - Instancing for high particle counts
     * - Subpixel rendering for small particles
     * - Particle sorting and transparency
     *
     * The particle system is designed to handle everything from simple smoke effects
     * to complex weather systems and explosions, with automatic performance scaling
     * based on hardware capabilities and particle count requirements.
     */
    class ParticleSystem {
    public:
        /**
         * @brief Constructor
         * @param config Particle system configuration
         */
        explicit ParticleSystem(const ParticleConfig& config);

        /**
         * @brief Destructor
         */
        ~ParticleSystem();

        /**
         * @brief Deleted copy constructor
         */
        ParticleSystem(const ParticleSystem&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        ParticleSystem& operator=(const ParticleSystem&) = delete;

        // Particle system lifecycle

        /**
         * @brief Initialize particle system
         * @param renderer Renderer instance
         * @return true if successful, false otherwise
         */
        bool Initialize(Renderer* renderer);

        /**
         * @brief Shutdown particle system
         */
        void Shutdown();

        /**
         * @brief Update particle system
         * @param deltaTime Time elapsed since last update
         */
        void Update(double deltaTime);

        /**
         * @brief Render particle system
         * @return true if successful, false otherwise
         */
        bool Render();

        /**
         * @brief Get particle system configuration
         * @return Current configuration
         */
        const ParticleConfig& GetConfig() const { return m_config; }

        /**
         * @brief Set particle system configuration
         * @param config New configuration
         * @return true if successful, false otherwise
         */
        bool SetConfig(const ParticleConfig& config);

        // Particle emission and control

        /**
         * @brief Start particle emission
         * @return true if successful, false otherwise
         */
        bool Start();

        /**
         * @brief Stop particle emission
         * @return true if successful, false otherwise
         */
        bool Stop();

        /**
         * @brief Pause particle system
         * @return true if successful, false otherwise
         */
        bool Pause();

        /**
         * @brief Resume particle system
         * @return true if successful, false otherwise
         */
        bool Resume();

        /**
         * @brief Check if system is active
         * @return true if active, false otherwise
         */
        bool IsActive() const { return m_isActive; }

        /**
         * @brief Check if system is emitting
         * @return true if emitting, false otherwise
         */
        bool IsEmitting() const { return m_isEmitting; }

        /**
         * @brief Emit burst of particles
         * @param count Number of particles to emit
         * @return Number of particles actually emitted
         */
        uint32_t EmitBurst(uint32_t count);

        /**
         * @brief Get active particle count
         * @return Number of active particles
         */
        uint32_t GetActiveParticleCount() const { return m_activeParticles; }

        /**
         * @brief Get maximum particle count
         * @return Maximum particle capacity
         */
        uint32_t GetMaxParticleCount() const { return m_config.maxParticles; }

        // Emitter management

        /**
         * @brief Add particle emitter
         * @param emitter Emitter configuration
         * @return Emitter ID or 0 if failed
         */
        uint32_t AddEmitter(const ParticleEmitter& emitter);

        /**
         * @brief Remove emitter
         * @param emitterId Emitter ID
         * @return true if successful, false otherwise
         */
        bool RemoveEmitter(uint32_t emitterId);

        /**
         * @brief Get emitter
         * @param emitterId Emitter ID
         * @return Emitter configuration or nullopt
         */
        std::optional<ParticleEmitter> GetEmitter(uint32_t emitterId) const;

        /**
         * @brief Set emitter properties
         * @param emitterId Emitter ID
         * @param emitter New emitter configuration
         * @return true if successful, false otherwise
         */
        bool SetEmitter(uint32_t emitterId, const ParticleEmitter& emitter);

        // Force field management

        /**
         * @brief Add force field
         * @param forceField Force field configuration
         * @return Force field ID or 0 if failed
         */
        uint32_t AddForceField(const ForceField& forceField);

        /**
         * @brief Remove force field
         * @param forceFieldId Force field ID
         * @return true if successful, false otherwise
         */
        bool RemoveForceField(uint32_t forceFieldId);

        /**
         * @brief Get force field
         * @param forceFieldId Force field ID
         * @return Force field configuration or nullopt
         */
        std::optional<ForceField> GetForceField(uint32_t forceFieldId) const;

        /**
         * @brief Set force field properties
         * @param forceFieldId Force field ID
         * @param forceField New force field configuration
         * @return true if successful, false otherwise
         */
        bool SetForceField(uint32_t forceFieldId, const ForceField& forceField);

        // Particle access

        /**
         * @brief Get particle data
         * @param index Particle index
         * @return Particle data or nullopt if invalid
         */
        std::optional<Particle> GetParticle(uint32_t index) const;

        /**
         * @brief Set particle data
         * @param index Particle index
         * @param particle New particle data
         * @return true if successful, false otherwise
         */
        bool SetParticle(uint32_t index, const Particle& particle);

        /**
         * @brief Get all active particles
         * @return Vector of active particles
         */
        std::vector<Particle> GetActiveParticles() const;

        // Material and rendering

        /**
         * @brief Set particle material
         * @param material Material to use
         * @return true if successful, false otherwise
         */
        bool SetMaterial(std::shared_ptr<Material> material);

        /**
         * @brief Get current material
         * @return Current material
         */
        std::shared_ptr<Material> GetMaterial() const { return m_material; }

        /**
         * @brief Set texture atlas
         * @param textureId Texture atlas ID
         * @param size Texture size in atlas
         * @return true if successful, false otherwise
         */
        bool SetTextureAtlas(uint32_t textureId, const glm::vec2& size = glm::vec2(1.0f));

        /**
         * @brief Set render mode
         * @param mode Render mode
         * @return true if successful, false otherwise
         */
        bool SetRenderMode(ParticleRenderMode mode);

        // Transformation and positioning

        /**
         * @brief Set system position
         * @param position New position
         * @return true if successful, false otherwise
         */
        bool SetPosition(const glm::vec3& position);

        /**
         * @brief Get system position
         * @return Current position
         */
        const glm::vec3& GetPosition() const { return m_position; }

        /**
         * @brief Set system rotation
         * @param rotation New rotation
         * @return true if successful, false otherwise
         */
        bool SetRotation(const glm::quat& rotation);

        /**
         * @brief Get system rotation
         * @return Current rotation
         */
        const glm::quat& GetRotation() const { return m_rotation; }

        /**
         * @brief Set system scale
         * @param scale New scale
         * @return true if successful, false otherwise
         */
        bool SetScale(const glm::vec3& scale);

        /**
         * @brief Get system scale
         * @return Current scale
         */
        const glm::vec3& GetScale() const { return m_scale; }

        // Simulation control

        /**
         * @brief Set simulation method
         * @param simulation Simulation method
         * @return true if successful, false otherwise
         */
        bool SetSimulationMethod(ParticleSimulation simulation);

        /**
         * @brief Enable GPU acceleration
         * @param enabled Enable state
         * @return true if successful, false otherwise
         */
        bool EnableGPUAcceleration(bool enabled);

        /**
         * @brief Set quality level
         * @param quality Quality level
         * @return true if successful, false otherwise
         */
        bool SetQualityLevel(ParticleQuality quality);

        // Performance monitoring

        /**
         * @brief Get particle system statistics
         * @return Performance statistics
         */
        const ParticleStats& GetStats() const { return m_stats; }

        /**
         * @brief Reset statistics
         */
        void ResetStats();

        /**
         * @brief Get performance report
         * @return Detailed performance report
         */
        std::string GetPerformanceReport() const;

        // Event system

        /**
         * @brief Add particle event listener
         * @param eventType Event type
         * @param listener Event listener function
         * @return Listener ID
         */
        uint32_t AddEventListener(const std::string& eventType,
                                 std::function<void(const std::unordered_map<std::string, std::any>&)> listener);

        /**
         * @brief Remove particle event listener
         * @param listenerId Listener ID
         * @return true if removed, false otherwise
         */
        bool RemoveEventListener(uint32_t listenerId);

        /**
         * @brief Trigger particle event
         * @param eventType Event type
         * @param eventData Event data
         */
        void TriggerEvent(const std::string& eventType,
                         const std::unordered_map<std::string, std::any>& eventData);

        // Utility functions

        /**
         * @brief Validate particle system
         * @return true if valid, false otherwise
         */
        bool Validate() const;

        /**
         * @brief Get status report
         * @return Detailed status report
         */
        std::string GetStatusReport() const;

        /**
         * @brief Optimize particle system
         * @return Optimization results
         */
        std::unordered_map<std::string, size_t> Optimize();

    private:
        /**
         * @brief Initialize particle system
         * @return true if successful, false otherwise
         */
        bool InitializeSystem();

        /**
         * @brief Initialize GPU resources
         * @return true if successful, false otherwise
         */
        bool InitializeGPUResources();

        /**
         * @brief Update particle simulation (CPU)
         * @param deltaTime Time elapsed
         */
        void UpdateCPUSimulation(double deltaTime);

        /**
         * @brief Update particle simulation (GPU)
         * @param deltaTime Time elapsed
         */
        void UpdateGPUSimulation(double deltaTime);

        /**
         * @brief Emit new particles
         * @param deltaTime Time elapsed
         */
        void EmitParticles(double deltaTime);

        /**
         * @brief Update particle physics
         * @param deltaTime Time elapsed
         */
        void UpdateParticlePhysics(double deltaTime);

        /**
         * @brief Update particle animation
         * @param deltaTime Time elapsed
         */
        void UpdateParticleAnimation(double deltaTime);

        /**
         * @brief Handle particle collisions
         * @param deltaTime Time elapsed
         */
        void HandleParticleCollisions(double deltaTime);

        /**
         * @brief Kill dead particles
         */
        void KillDeadParticles();

        /**
         * @brief Sort particles for rendering
         */
        void SortParticles();

        /**
         * @brief Render particles (CPU path)
         * @return true if successful, false otherwise
         */
        bool RenderCPU();

        /**
         * @brief Render particles (GPU path)
         * @return true if successful, false otherwise
         */
        bool RenderGPU();

        /**
         * @brief Generate particle emission position
         * @param emitter Emitter configuration
         * @return Emission position
         */
        glm::vec3 GenerateEmissionPosition(const ParticleEmitter& emitter);

        /**
         * @brief Apply force fields to particle
         * @param particle Particle to affect
         * @param deltaTime Time elapsed
         */
        void ApplyForceFields(Particle& particle, double deltaTime);

        /**
         * @brief Update performance statistics
         * @param deltaTime Time elapsed
         */
        void UpdateStats(double deltaTime);

        /**
         * @brief Handle particle system errors
         * @param error Error message
         */
        void HandleError(const std::string& error);

        // Particle system data
        ParticleConfig m_config;                      ///< System configuration
        ParticleStats m_stats;                        ///< Performance statistics

        // Core systems
        Renderer* m_renderer;                         ///< Renderer instance
        std::shared_ptr<Material> m_material;         ///< Particle material

        // Particle storage
        std::vector<Particle> m_particles;            ///< Particle array
        std::vector<uint32_t> m_activeParticleIndices; ///< Active particle indices
        uint32_t m_activeParticles;                   ///< Number of active particles
        mutable std::shared_mutex m_particlesMutex;   ///< Particles synchronization

        // Emitters and force fields
        std::unordered_map<uint32_t, ParticleEmitter> m_emitters; ///< Particle emitters
        std::unordered_map<uint32_t, ForceField> m_forceFields; ///< Force fields
        mutable std::shared_mutex m_simulationMutex;  ///< Simulation synchronization

        // Event system
        std::unordered_map<uint32_t, std::function<void(const std::unordered_map<std::string, std::any>&)>> m_eventListeners; ///< Event listeners
        mutable std::shared_mutex m_eventsMutex;      ///< Events synchronization

        // Transformation
        glm::vec3 m_position;                         ///< System position
        glm::quat m_rotation;                         ///< System rotation
        glm::vec3 m_scale;                            ///< System scale

        // State tracking
        bool m_isInitialized;                          ///< System is initialized
        bool m_isActive;                              ///< System is active
        bool m_isEmitting;                            ///< System is emitting
        double m_systemAge;                           ///< System age in seconds
        double m_lastUpdateTime;                      ///< Last update time
        double m_lastEmissionTime;                    ///< Last emission time
        std::string m_lastError;                      ///< Last error message

        // GPU resources
        uint32_t m_particleBuffer;                    ///< GPU particle buffer
        uint32_t m_emitterBuffer;                     ///< GPU emitter buffer
        uint32_t m_forceFieldBuffer;                  ///< GPU force field buffer
        uint32_t m_computeProgram;                    ///< Compute shader program
        uint32_t m_renderProgram;                     ///< Render shader program

        // Random number generation
        std::mt19937 m_randomEngine;                  ///< Random number engine
        std::uniform_real_distribution<float> m_randomDistribution; ///< Random distribution

        static std::atomic<uint32_t> s_nextSystemId;  ///< Next system ID counter
        static std::atomic<uint32_t> s_nextEmitterId; ///< Next emitter ID counter
        static std::atomic<uint32_t> s_nextForceFieldId; ///< Next force field ID counter
        static std::atomic<uint32_t> s_nextListenerId; ///< Next listener ID counter
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_GRAPHICS_PARTICLE_SYSTEM_HPP
