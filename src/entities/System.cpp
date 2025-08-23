#include "System.hpp"
#include "EntityManager.hpp"
#include <atomic>
#include <sstream>

namespace VoxelCraft {

    // Variable estática para generar IDs únicos
    std::atomic<System::SystemID> System::s_NextID{1};

    System::System()
        : m_ID(GenerateID())
        , m_Name("System")
        , m_Manager(nullptr)
        , m_State(SystemState::INITIALIZED)
    {
    }

    System::~System() {
        OnDestroy();
    }

    System::System(System&& other) noexcept
        : m_ID(other.m_ID)
        , m_Name(std::move(other.m_Name))
        , m_Manager(other.m_Manager)
        , m_State(other.m_State)
    {
        other.m_State = SystemState::DESTROYED;
        other.m_Manager = nullptr;
    }

    System& System::operator=(System&& other) noexcept {
        if (this != &other) {
            OnDestroy();

            m_ID = other.m_ID;
            m_Name = std::move(other.m_Name);
            m_Manager = other.m_Manager;
            m_State = other.m_State;

            other.m_State = SystemState::DESTROYED;
            other.m_Manager = nullptr;
        }

        return *this;
    }

    void System::SetPaused(bool paused) {
        if (m_State == SystemState::DESTROYED) {
            return;
        }

        SystemState newState = paused ? SystemState::PAUSED : SystemState::RUNNING;

        if (m_State != newState) {
            m_State = newState;

            if (paused) {
                OnPause();
            } else {
                OnResume();
            }
        }
    }

    void System::Destroy() {
        if (m_State != SystemState::DESTROYED) {
            m_State = SystemState::DESTROYED;
            OnDestroy();
        }
    }

    std::string System::ToString() const {
        std::stringstream ss;
        ss << "System[ID=" << m_ID << ", Name='" << m_Name << "', Type=" << GetType().name();

        switch (m_State) {
            case SystemState::INITIALIZED: ss << ", State=INITIALIZED"; break;
            case SystemState::RUNNING: ss << ", State=RUNNING"; break;
            case SystemState::PAUSED: ss << ", State=PAUSED"; break;
            case SystemState::DESTROYED: ss << ", State=DESTROYED"; break;
        }

        ss << ", Manager=" << (m_Manager ? "set" : "null") << "]";
        return ss.str();
    }

    System::SystemID System::GenerateID() {
        return s_NextID.fetch_add(1, std::memory_order_relaxed);
    }

    // Implementación de TransformSystem
    TransformSystem::TransformSystem() {
        m_Name = "TransformSystem";
    }

    void TransformSystem::Update(float deltaTime) {
        // Update transform components
        UpdateTransformMatrices();
        HandleParenting();
    }

    void TransformSystem::FixedUpdate(float fixedDeltaTime) {
        UpdateWorldMatrices();
    }

    void TransformSystem::OnInit() {
        VOXELCRAFT_LOG_INFO("TransformSystem initialized");
    }

    void TransformSystem::UpdateTransformMatrices() {
        // Implementation for updating local transform matrices
        // This would iterate through all entities with TransformComponent
    }

    void TransformSystem::HandleParenting() {
        // Implementation for handling parent-child relationships
        // This manages hierarchical transformations
    }

    void TransformSystem::UpdateWorldMatrices() {
        // Implementation for updating world space matrices
        // This combines local transforms with parent transforms
    }

    // Implementación de PhysicsSystem
    PhysicsSystem::PhysicsSystem() {
        m_Name = "PhysicsSystem";
    }

    void PhysicsSystem::Update(float deltaTime) {
        // Handle physics updates
        IntegrateVelocities();
    }

    void PhysicsSystem::FixedUpdate(float fixedDeltaTime) {
        // Fixed timestep physics updates
        ResolveCollisions();
        ApplyConstraints();
    }

    void PhysicsSystem::OnInit() {
        VOXELCRAFT_LOG_INFO("PhysicsSystem initialized with gravity: ({}, {}, {})",
                           m_Gravity.x, m_Gravity.y, m_Gravity.z);
    }

    void PhysicsSystem::IntegrateVelocities() {
        // Implementation for velocity integration
        // Apply forces, update velocities and positions
    }

    void PhysicsSystem::ResolveCollisions() {
        // Implementation for collision resolution
        // Detect and resolve collisions between entities
    }

    void PhysicsSystem::ApplyConstraints() {
        // Implementation for applying physics constraints
        // Joints, springs, etc.
    }

    // Implementación de RenderSystem
    RenderSystem::RenderSystem() {
        m_Name = "RenderSystem";
    }

    void RenderSystem::Update(float deltaTime) {
        CollectVisibleEntities();
        SortByRenderOrder();
    }

    void RenderSystem::LateUpdate(float deltaTime) {
        SubmitToRenderer();
        HandleLevelOfDetail();
    }

    void RenderSystem::OnInit() {
        VOXELCRAFT_LOG_INFO("RenderSystem initialized with render distance: {} and frustum culling: {}",
                           m_RenderDistance, m_FrustumCulling ? "enabled" : "disabled");
    }

    void RenderSystem::CollectVisibleEntities() {
        // Implementation for collecting visible entities
        // Frustum culling, occlusion culling, etc.
    }

    void RenderSystem::SortByRenderOrder() {
        // Implementation for sorting entities by render order
        // Transparency, depth, material, etc.
    }

    void RenderSystem::SubmitToRenderer() {
        // Implementation for submitting entities to the renderer
        // Batch rendering, instancing, etc.
    }

    void RenderSystem::HandleLevelOfDetail() {
        // Implementation for level of detail management
        // Distance-based LOD switching
    }

    // Implementación de AISystem
    AISystem::AISystem() {
        m_Name = "AISystem";
    }

    void AISystem::Update(float deltaTime) {
        m_AccumulatedTime += deltaTime;

        if (m_AccumulatedTime >= m_UpdateFrequency) {
            UpdateAIEntities();
            m_AccumulatedTime = 0.0f;
        }
    }

    void AISystem::FixedUpdate(float fixedDeltaTime) {
        ProcessBehaviors();
        HandlePathfinding();
    }

    void AISystem::OnInit() {
        VOXELCRAFT_LOG_INFO("AISystem initialized with update frequency: {} and max AI entities: {}",
                           m_UpdateFrequency, m_MaxAIEntities);
    }

    void AISystem::UpdateAIEntities() {
        // Implementation for updating AI entities
        // Behavior trees, state machines, etc.
    }

    void AISystem::ProcessBehaviors() {
        // Implementation for processing AI behaviors
        // Execute behavior tree nodes
    }

    void AISystem::HandlePathfinding() {
        // Implementation for pathfinding
        // A* pathfinding, navigation meshes, etc.
    }

    void AISystem::ManageBehaviorTrees() {
        // Implementation for managing behavior trees
        // Tree construction, execution, debugging
    }

    // Implementación de AudioSystem
    AudioSystem::AudioSystem() {
        m_Name = "AudioSystem";
    }

    void AudioSystem::Update(float deltaTime) {
        Update3DAudio();
        ManageAudioSources();
    }

    void AudioSystem::LateUpdate(float deltaTime) {
        HandleSpatialAudio();
        ProcessAudioEvents();
    }

    void AudioSystem::OnInit() {
        VOXELCRAFT_LOG_INFO("AudioSystem initialized with max audio sources: {}",
                           m_MaxAudioSources);
    }

    void AudioSystem::Update3DAudio() {
        // Implementation for updating 3D audio
        // Update source positions, calculate attenuation, etc.
    }

    void AudioSystem::ManageAudioSources() {
        // Implementation for managing audio sources
        // Source pooling, priority management, etc.
    }

    void AudioSystem::HandleSpatialAudio() {
        // Implementation for spatial audio processing
        // HRTF, ambisonics, etc.
    }

    void AudioSystem::ProcessAudioEvents() {
        // Implementation for processing audio events
        // Play sounds, adjust volumes, etc.
    }

    // Implementación de ParticleSystem
    ParticleSystem::ParticleSystem() {
        m_Name = "ParticleSystem";
    }

    void ParticleSystem::Update(float deltaTime) {
        UpdateParticles();
        EmitNewParticles();
    }

    void ParticleSystem::FixedUpdate(float fixedDeltaTime) {
        SimulatePhysics();
        HandleCollisions();
    }

    void ParticleSystem::OnInit() {
        VOXELCRAFT_LOG_INFO("ParticleSystem initialized with max particles: {} and budget: {}",
                           m_MaxParticles, m_ParticleBudget);
    }

    void ParticleSystem::UpdateParticles() {
        // Implementation for updating particles
        // Update positions, velocities, lifetimes, etc.
    }

    void ParticleSystem::EmitNewParticles() {
        // Implementation for emitting new particles
        // Handle particle emitters
    }

    void ParticleSystem::SimulatePhysics() {
        // Implementation for particle physics simulation
        // Gravity, wind, collisions, etc.
    }

    void ParticleSystem::HandleCollisions() {
        // Implementation for particle collision detection
        // With world geometry and other objects
    }

}