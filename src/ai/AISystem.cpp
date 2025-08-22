/**
 * @file AISystem.cpp
 * @brief VoxelCraft AI System Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file implements the AISystem class that coordinates all AI functionality
 * in the VoxelCraft game world.
 */

#include "AISystem.hpp"
#include "Mob.hpp"
#include "BehaviorTree.hpp"
#include "PathfindingSystem.hpp"
#include "VillageSystem.hpp"
#include "SensorySystem.hpp"
#include "Blackboard.hpp"

#include "../core/Logger.hpp"
#include "../world/World.hpp"
#include "../entities/EntityManager.hpp"
#include "../core/Config.hpp"

#include <algorithm>
#include <chrono>
#include <random>
#include <sstream>
#include <any>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace VoxelCraft {

    // Static member initialization
    std::atomic<uint32_t> AISystem::s_nextTaskId(1);

    AISystem::AISystem(const AISystemConfig& config)
        : m_config(config)
        , m_state(AIState::Inactive)
        , m_world(nullptr)
        , m_entityManager(nullptr)
        , m_isInitialized(false)
        , m_lastUpdateTime(0.0)
        , m_creationTime(0.0)
        , m_workersRunning(false)
        , m_activeThreads(0)
    {
        // Initialize metrics
        m_metrics = {};

        // Set creation time
        m_creationTime = std::chrono::duration<double>(
            std::chrono::high_resolution_clock::now().time_since_epoch()).count();

        VOXELCRAFT_INFO("AISystem created with mode: " + std::to_string(static_cast<int>(m_config.mode)));
    }

    AISystem::~AISystem() {
        Shutdown();
    }

    bool AISystem::Initialize() {
        if (m_isInitialized) {
            VOXELCRAFT_WARNING("AISystem already initialized");
            return true;
        }

        VOXELCRAFT_INFO("Initializing AISystem...");

        try {
            // Initialize subsystems
            if (!InitializeSubsystems()) {
                VOXELCRAFT_ERROR("Failed to initialize AI subsystems");
                return false;
            }

            // Initialize worker threads
            if (m_config.enableMultithreading) {
                if (!InitializeWorkerThreads()) {
                    VOXELCRAFT_ERROR("Failed to initialize worker threads");
                    return false;
                }
            }

            m_isInitialized = true;
            m_state = AIState::Active;

            VOXELCRAFT_INFO("AISystem initialized successfully");
            return true;

        } catch (const std::exception& e) {
            VOXELCRAFT_ERROR("Exception during AISystem initialization: {}", e.what());
            return false;
        }
    }

    void AISystem::Shutdown() {
        if (!m_isInitialized) {
            return;
        }

        VOXELCRAFT_INFO("Shutting down AISystem...");

        // Stop system
        Stop();

        // Shutdown subsystems
        if (m_villageSystem) {
            m_villageSystem->Shutdown();
        }

        if (m_pathfindingSystem) {
            m_pathfindingSystem->Shutdown();
        }

        // Clear all mobs
        {
            std::unique_lock<std::shared_mutex> lock(m_mobMutex);
            m_activeMobs.clear();
            m_mobFactories.clear();
        }

        // Clear task queue
        {
            std::unique_lock<std::shared_mutex> lock(m_taskMutex);
            m_taskQueue = std::priority_queue<AITask, std::vector<AITask>,
                std::function<bool(const AITask&, const AITask&)>>();
            m_scheduledTasks.clear();
        }

        // Clear log entries
        {
            std::unique_lock<std::shared_mutex> lock(m_logMutex);
            m_logEntries.clear();
        }

        // Clear debug data
        {
            std::unique_lock<std::shared_mutex> lock(m_debugMutex);
            m_debugRenderData.clear();
        }

        m_isInitialized = false;
        m_state = AIState::Inactive;

        VOXELCRAFT_INFO("AISystem shutdown complete");
    }

    bool AISystem::Start() {
        if (m_state == AIState::Active) {
            return true;
        }

        if (!m_isInitialized) {
            VOXELCRAFT_ERROR("Cannot start AISystem: not initialized");
            return false;
        }

        VOXELCRAFT_INFO("Starting AISystem...");

        m_state = AIState::Active;

        // Start subsystems
        if (m_pathfindingSystem) {
            // Pathfinding system is already initialized
        }

        if (m_villageSystem) {
            // Village system is already initialized
        }

        VOXELCRAFT_INFO("AISystem started successfully");
        return true;
    }

    void AISystem::Stop() {
        if (m_state == AIState::Inactive) {
            return;
        }

        VOXELCRAFT_INFO("Stopping AISystem...");

        m_state = AIState::Inactive;

        // Stop worker threads
        if (m_workersRunning) {
            m_workersRunning = false;
            m_taskCondition.notify_all();

            for (auto& thread : m_workerThreads) {
                if (thread.joinable()) {
                    thread.join();
                }
            }
            m_workerThreads.clear();
        }

        // Stop all active mobs
        {
            std::shared_lock<std::shared_mutex> lock(m_mobMutex);
            for (const auto& [id, mob] : m_activeMobs) {
                if (mob) {
                    mob->SetState(MobState::Idle);
                }
            }
        }

        VOXELCRAFT_INFO("AISystem stopped");
    }

    void AISystem::Pause() {
        if (m_state != AIState::Active) {
            return;
        }

        VOXELCRAFT_INFO("Pausing AISystem...");
        m_state = AIState::Paused;
    }

    void AISystem::Resume() {
        if (m_state != AIState::Paused) {
            return;
        }

        VOXELCRAFT_INFO("Resuming AISystem...");
        m_state = AIState::Active;
    }

    void AISystem::Update(double deltaTime) {
        if (!m_isInitialized || m_state != AIState::Active) {
            return;
        }

        double currentTime = std::chrono::duration<double>(
            std::chrono::high_resolution_clock::now().time_since_epoch()).count();

        // Update timing
        m_lastUpdateTime = currentTime;

        try {
            // Process scheduled tasks
            ProcessScheduledTasks(currentTime);

            // Update subsystems
            UpdateSubsystems(deltaTime);

            // Update active mobs
            UpdateMobs(deltaTime);

            // Cleanup inactive mobs
            CleanupInactiveMobs();

            // Update performance metrics
            UpdateMetrics(deltaTime);

            // Collect debug render data
            CollectDebugRenderData();

        } catch (const std::exception& e) {
            VOXELCRAFT_ERROR("Exception during AISystem update: {}", e.what());
            HandleSystemError("Update exception: " + std::string(e.what()));
        }
    }

    bool AISystem::InitializeSubsystems() {
        VOXELCRAFT_INFO("Initializing AI subsystems...");

        try {
            // Create pathfinding system
            PathfindingConfig pathConfig;
            pathConfig.defaultAlgorithm = PathfindingAlgorithm::AStar;
            pathConfig.defaultHeuristic = HeuristicType::Octile;
            pathConfig.enableMultithreading = m_config.enableMultithreading;
            pathConfig.maxConcurrentRequests = m_config.maxConcurrentUpdates / 2;
            pathConfig.enablePathCaching = true;
            pathConfig.cacheSize = 1000;

            m_pathfindingSystem = std::make_unique<PathfindingSystem>(pathConfig);

            // Create navigation system
            // m_navigationSystem = std::make_unique<NavigationSystem>(...);

            // Create procedural generator
            // m_proceduralGenerator = std::make_unique<ProceduralGenerator>(...);

            // Create village system
            VillageConfig villageConfig;
            villageConfig.minVillageSize = 5;
            villageConfig.maxVillageSize = 20;
            villageConfig.enableVillageGeneration = true;
            villageConfig.enableResourceManagement = true;

            m_villageSystem = std::make_unique<VillageSystem>(villageConfig);

            // Create sensory system
            // m_sensorySystem = std::make_unique<SensorySystem>(...);

            // Create decision system
            // m_decisionSystem = std::make_unique<DecisionSystem>(...);

            // Create learning system
            // m_learningSystem = std::make_unique<LearningSystem>(...);

            // Create blackboard system
            // m_blackboardSystem = std::make_unique<BlackboardSystem>(...);

            VOXELCRAFT_INFO("AI subsystems initialized successfully");
            return true;

        } catch (const std::exception& e) {
            VOXELCRAFT_ERROR("Failed to initialize AI subsystems: {}", e.what());
            return false;
        }
    }

    bool AISystem::InitializeWorkerThreads() {
        if (!m_config.enableMultithreading) {
            return true;
        }

        VOXELCRAFT_INFO("Initializing {} AI worker threads...", m_config.maxConcurrentUpdates);

        try {
            m_workersRunning = true;
            m_activeThreads = 0;

            for (int i = 0; i < m_config.maxConcurrentUpdates; ++i) {
                m_workerThreads.emplace_back(&AISystem::WorkerThreadFunction, this, i);
            }

            VOXELCRAFT_INFO("AI worker threads initialized successfully");
            return true;

        } catch (const std::exception& e) {
            VOXELCRAFT_ERROR("Failed to initialize worker threads: {}", e.what());
            m_workersRunning = false;
            return false;
        }
    }

    void AISystem::WorkerThreadFunction(int threadId) {
        VOXELCRAFT_DEBUG("AI worker thread {} started", threadId);

        while (m_workersRunning) {
            try {
                AITask task;

                // Wait for task
                {
                    std::unique_lock<std::mutex> lock(m_taskMutex.mutex);
                    m_taskCondition.wait(lock, [this]() {
                        return !m_workersRunning || !m_taskQueue.empty();
                    });

                    if (!m_workersRunning) {
                        break;
                    }

                    if (!m_taskQueue.empty()) {
                        task = m_taskQueue.top();
                        m_taskQueue.pop();
                    } else {
                        continue;
                    }
                }

                // Execute task
                if (task.function) {
                    ++m_activeThreads;
                    task.function();
                    --m_activeThreads;
                }

            } catch (const std::exception& e) {
                VOXELCRAFT_ERROR("Exception in AI worker thread {}: {}", threadId, e.what());
                --m_activeThreads;
            }
        }

        VOXELCRAFT_DEBUG("AI worker thread {} stopped", threadId);
    }

    void AISystem::ProcessTasks() {
        if (!m_config.enableMultithreading) {
            return;
        }

        std::unique_lock<std::shared_mutex> lock(m_taskMutex);

        // Process completed tasks and schedule new ones
        // This is a simplified implementation - in a real system,
        // you'd want more sophisticated task management
    }

    void AISystem::UpdateMobs(double deltaTime) {
        std::vector<std::shared_ptr<Mob>> mobsToUpdate;

        // Get mobs to update (thread-safe copy)
        {
            std::shared_lock<std::shared_mutex> lock(m_mobMutex);
            mobsToUpdate.reserve(m_activeMobs.size());

            for (const auto& [id, mob] : m_activeMobs) {
                if (mob && mob->IsAlive()) {
                    mobsToUpdate.push_back(mob);
                }
            }
        }

        // Update mobs
        size_t updatedCount = 0;
        for (auto& mob : mobsToUpdate) {
            if (updatedCount >= static_cast<size_t>(m_config.maxActiveMobs)) {
                break;
            }

            try {
                mob->Update(deltaTime);
                ++updatedCount;
            } catch (const std::exception& e) {
                VOXELCRAFT_ERROR("Exception updating mob {}: {}", mob->GetId(), e.what());
                HandleSystemError("Mob update exception: " + std::string(e.what()));
            }
        }

        m_metrics.activeMobs = updatedCount;
        m_metrics.totalMobs = m_activeMobs.size();
    }

    void AISystem::UpdateSubsystems(double deltaTime) {
        // Update pathfinding system
        if (m_pathfindingSystem) {
            try {
                m_pathfindingSystem->Update(deltaTime);
            } catch (const std::exception& e) {
                VOXELCRAFT_ERROR("Exception updating pathfinding system: {}", e.what());
            }
        }

        // Update village system
        if (m_villageSystem) {
            try {
                m_villageSystem->Update(deltaTime);
            } catch (const std::exception& e) {
                VOXELCRAFT_ERROR("Exception updating village system: {}", e.what());
            }
        }

        // Update other subsystems
        // ... (similar pattern for other subsystems)
    }

    void AISystem::CleanupInactiveMobs() {
        std::unique_lock<std::shared_mutex> lock(m_mobMutex);

        auto it = m_activeMobs.begin();
        while (it != m_activeMobs.end()) {
            auto& mob = it->second;

            // Remove dead or invalid mobs
            if (!mob || !mob->IsAlive() || !mob->Validate()) {
                // Remove from entity manager if present
                if (m_entityManager && mob) {
                    m_entityManager->RemoveEntity(mob->GetId());
                }

                it = m_activeMobs.erase(it);
                ++m_metrics.totalMobs; // This is actually a decrement in logic
            } else {
                ++it;
            }
        }
    }

    void AISystem::ProcessScheduledTasks(double currentTime) {
        std::unique_lock<std::shared_mutex> lock(m_taskMutex);

        auto it = m_scheduledTasks.begin();
        while (it != m_scheduledTasks.end()) {
            auto& task = it->second;

            if (currentTime >= task.scheduledTime) {
                // Execute task
                if (task.function) {
                    try {
                        task.function();
                    } catch (const std::exception& e) {
                        VOXELCRAFT_ERROR("Exception in scheduled task {}: {}", task.taskId, e.what());
                    }
                }

                // Handle recurring tasks
                if (task.isRecurring && m_workersRunning) {
                    task.scheduledTime = currentTime + task.interval;
                } else {
                    task.isCompleted = true;
                    it = m_scheduledTasks.erase(it);
                    continue;
                }
            }

            ++it;
        }
    }

    void AISystem::UpdateMetrics(double deltaTime) {
        // Update performance metrics
        ++m_metrics.updateCount;

        // Calculate update time
        static double lastMetricsUpdate = 0.0;
        double currentTime = std::chrono::duration<double>(
            std::chrono::high_resolution_clock::now().time_since_epoch()).count();

        if (currentTime - lastMetricsUpdate >= 1.0) { // Update every second
            m_metrics.averageUpdateTime = m_metrics.totalUpdateTime / std::max(1ULL, m_metrics.updateCount);
            lastMetricsUpdate = currentTime;
        }

        // Reset metrics if they get too large (prevent overflow)
        if (m_metrics.updateCount > 1000000) {
            m_metrics.updateCount = 1;
            m_metrics.totalUpdateTime = 0.0;
        }
    }

    void AISystem::CollectDebugRenderData() {
        if (!m_config.enableDebugDrawing) {
            return;
        }

        std::unique_lock<std::shared_mutex> lock(m_debugMutex);
        m_debugRenderData.clear();

        // Collect debug data from subsystems
        if (m_pathfindingSystem) {
            auto pathDebugData = m_pathfindingSystem->GetDebugRenderData();
            m_debugRenderData.insert(m_debugRenderData.end(),
                                   pathDebugData.begin(), pathDebugData.end());
        }

        if (m_villageSystem) {
            auto villageDebugData = m_villageSystem->GetDebugRenderData();
            m_debugRenderData.insert(m_debugRenderData.end(),
                                   villageDebugData.begin(), villageDebugData.end());
        }

        // Add mob debug data
        {
            std::shared_lock<std::shared_mutex> mobLock(m_mobMutex);
            for (const auto& [id, mob] : m_activeMobs) {
                if (mob) {
                    // Add mob position indicator
                    DebugRenderData debugData;
                    debugData.type = DebugRenderData::Type::Sphere;
                    debugData.position = mob->GetPosition();
                    debugData.size = glm::vec3(0.5f);
                    debugData.color = glm::vec4(1.0f, 0.0f, 0.0f, 0.8f);
                    debugData.duration = 0.0f;
                    m_debugRenderData.push_back(debugData);
                }
            }
        }
    }

    void AISystem::HandleSystemError(const std::string& error) {
        VOXELCRAFT_ERROR("AI System Error: {}", error);
        m_state = AIState::Error;

        // Log error for debugging
        LogMessage("ERROR", "AISystem", error);
    }

    // Mob Management

    bool AISystem::RegisterMobType(const std::string& mobType,
                                  std::function<std::shared_ptr<Mob>()> factory) {
        std::unique_lock<std::shared_mutex> lock(m_mobMutex);

        if (m_mobFactories.find(mobType) != m_mobFactories.end()) {
            VOXELCRAFT_WARNING("Mob type '{}' already registered", mobType);
            return false;
        }

        m_mobFactories[mobType] = factory;
        VOXELCRAFT_INFO("Registered mob type: {}", mobType);
        return true;
    }

    bool AISystem::UnregisterMobType(const std::string& mobType) {
        std::unique_lock<std::shared_mutex> lock(m_mobMutex);

        auto it = m_mobFactories.find(mobType);
        if (it == m_mobFactories.end()) {
            VOXELCRAFT_WARNING("Mob type '{}' not found", mobType);
            return false;
        }

        m_mobFactories.erase(it);
        VOXELCRAFT_INFO("Unregistered mob type: {}", mobType);
        return true;
    }

    uint32_t AISystem::SpawnMob(const std::string& mobType,
                               const glm::vec3& position,
                               const glm::quat& rotation) {
        // Check if mob type is registered
        auto factoryIt = m_mobFactories.find(mobType);
        if (factoryIt == m_mobFactories.end()) {
            VOXELCRAFT_ERROR("Unknown mob type: {}", mobType);
            return 0;
        }

        // Create mob
        std::shared_ptr<Mob> mob;
        try {
            mob = factoryIt->second();
            if (!mob) {
                VOXELCRAFT_ERROR("Failed to create mob of type: {}", mobType);
                return 0;
            }
        } catch (const std::exception& e) {
            VOXELCRAFT_ERROR("Exception creating mob of type {}: {}", mobType, e.what());
            return 0;
        }

        // Initialize mob
        if (!mob->Initialize()) {
            VOXELCRAFT_ERROR("Failed to initialize mob of type: {}", mobType);
            return 0;
        }

        // Set mob properties
        mob->SetPosition(position);
        mob->SetRotation(rotation);
        mob->SetAISystem(this);

        // Generate unique ID
        static std::atomic<uint32_t> nextMobId(1);
        uint32_t mobId = nextMobId++;

        // Add to active mobs
        {
            std::unique_lock<std::shared_mutex> lock(m_mobMutex);
            m_activeMobs[mobId] = mob;
        }

        // Add to entity manager
        if (m_entityManager) {
            m_entityManager->AddEntity(mob);
        }

        // Add to world
        if (m_world) {
            // World integration would go here
        }

        VOXELCRAFT_DEBUG("Spawned mob {} of type {} at position ({}, {}, {})",
                 mobId, mobType, position.x, position.y, position.z);

        ++m_metrics.totalMobs;
        return mobId;
    }

    bool AISystem::DespawnMob(uint32_t mobId) {
        std::unique_lock<std::shared_mutex> lock(m_mobMutex);

        auto it = m_activeMobs.find(mobId);
        if (it == m_activeMobs.end()) {
            VOXELCRAFT_WARNING("Mob {} not found for despawn", mobId);
            return false;
        }

        auto& mob = it->second;
        if (mob) {
            // Remove from entity manager
            if (m_entityManager) {
                m_entityManager->RemoveEntity(mobId);
            }

            // Remove from world
            if (m_world) {
                // World cleanup would go here
            }

            VOXELCRAFT_DEBUG("Despawned mob {}", mobId);
        }

        m_activeMobs.erase(it);
        return true;
    }

    std::shared_ptr<Mob> AISystem::GetMob(uint32_t mobId) const {
        std::shared_lock<std::shared_mutex> lock(m_mobMutex);

        auto it = m_activeMobs.find(mobId);
        return (it != m_activeMobs.end()) ? it->second : nullptr;
    }

    std::vector<std::shared_ptr<Mob>> AISystem::GetActiveMobs() const {
        std::shared_lock<std::shared_mutex> lock(m_mobMutex);

        std::vector<std::shared_ptr<Mob>> result;
        result.reserve(m_activeMobs.size());

        for (const auto& [id, mob] : m_activeMobs) {
            if (mob && mob->IsAlive()) {
                result.push_back(mob);
            }
        }

        return result;
    }

    std::vector<std::shared_ptr<Mob>> AISystem::GetMobsInRadius(
        const glm::vec3& position, float radius) const {

        std::shared_lock<std::shared_mutex> lock(m_mobMutex);

        std::vector<std::shared_ptr<Mob>> result;

        for (const auto& [id, mob] : m_activeMobs) {
            if (mob && mob->IsAlive()) {
                float distance = glm::distance(mob->GetPosition(), position);
                if (distance <= radius) {
                    result.push_back(mob);
                }
            }
        }

        return result;
    }

    // Task Management

    uint32_t AISystem::ScheduleTask(const AITask& task) {
        if (!m_isInitialized) {
            VOXELCRAFT_ERROR("Cannot schedule task: AISystem not initialized");
            return 0;
        }

        AITask newTask = task;
        newTask.taskId = s_nextTaskId++;
        newTask.isCompleted = false;

        {
            std::unique_lock<std::shared_mutex> lock(m_taskMutex);

            if (newTask.scheduledTime == 0.0) {
                newTask.scheduledTime = std::chrono::duration<double>(
                    std::chrono::high_resolution_clock::now().time_since_epoch()).count();
            }

            m_scheduledTasks[newTask.taskId] = newTask;
        }

        VOXELCRAFT_DEBUG("Scheduled AI task {} for execution", newTask.taskId);
        return newTask.taskId;
    }

    bool AISystem::CancelTask(uint32_t taskId) {
        std::unique_lock<std::shared_mutex> lock(m_taskMutex);

        auto it = m_scheduledTasks.find(taskId);
        if (it == m_scheduledTasks.end()) {
            VOXELCRAFT_WARNING("Task {} not found for cancellation", taskId);
            return false;
        }

        it->second.isCompleted = true;
        m_scheduledTasks.erase(it);

        VOXELCRAFT_DEBUG("Cancelled AI task {}", taskId);
        return true;
    }

    size_t AISystem::GetPendingTasksCount() const {
        std::shared_lock<std::shared_mutex> lock(m_taskMutex);
        return m_scheduledTasks.size();
    }

    // Event System

    void AISystem::AddEventListener(const std::string& eventType,
                                   std::function<void(const std::string&)> listener) {
        std::unique_lock<std::shared_mutex> lock(m_eventMutex);
        m_eventListeners[eventType].push_back(listener);
    }

    void AISystem::RemoveEventListener(const std::string& eventType) {
        std::unique_lock<std::shared_mutex> lock(m_eventMutex);
        m_eventListeners.erase(eventType);
    }

    void AISystem::TriggerEvent(const std::string& eventType,
                               const std::string& eventData) {
        std::shared_lock<std::shared_mutex> lock(m_eventMutex);

        auto it = m_eventListeners.find(eventType);
        if (it != m_eventListeners.end()) {
            for (auto& listener : it->second) {
                try {
                    listener(eventData);
                } catch (const std::exception& e) {
                    VOXELCRAFT_ERROR("Exception in AI event listener: {}", e.what());
                }
            }
        }

        // Log event
        LogMessage("EVENT", eventType, eventData);
    }

    // Logging

    void AISystem::LogMessage(const std::string& level,
                             const std::string& component,
                             const std::string& message,
                             const std::unordered_map<std::string, std::any>& data) {
        std::unique_lock<std::shared_mutex> lock(m_logMutex);

        AILogEntry entry;
        entry.timestamp = std::chrono::duration<double>(
            std::chrono::high_resolution_clock::now().time_since_epoch()).count();
        entry.level = level;
        entry.component = component;
        entry.message = message;
        entry.data = data;

        m_logEntries.push_back(entry);

        // Maintain log size limit
        if (m_logEntries.size() > m_maxLogEntries) {
            m_logEntries.erase(m_logEntries.begin());
        }
    }

    std::vector<AILogEntry> AISystem::GetRecentLogEntries(size_t count) const {
        std::shared_lock<std::shared_mutex> lock(m_logMutex);

        size_t startIndex = (m_logEntries.size() > count) ?
                           (m_logEntries.size() - count) : 0;

        return std::vector<AILogEntry>(m_logEntries.begin() + startIndex,
                                      m_logEntries.end());
    }

    // Configuration

    void AISystem::SetConfig(const AISystemConfig& config) {
        // Validate config changes
        if (config.maxConcurrentUpdates != m_config.maxConcurrentUpdates) {
            VOXELCRAFT_INFO("AI max concurrent updates changed from {} to {}",
                    m_config.maxConcurrentUpdates, config.maxConcurrentUpdates);
        }

        m_config = config;

        // Update subsystems with new config
        if (m_pathfindingSystem) {
            PathfindingConfig pathConfig;
            pathConfig.enableMultithreading = m_config.enableMultithreading;
            pathConfig.maxConcurrentRequests = m_config.maxConcurrentUpdates / 2;
            m_pathfindingSystem->SetConfig(pathConfig);
        }
    }

    void AISystem::SetMode(AISystemMode mode) {
        if (mode == m_config.mode) {
            return;
        }

        VOXELCRAFT_INFO("Changing AI system mode from {} to {}",
                static_cast<int>(m_config.mode), static_cast<int>(mode));

        m_config.mode = mode;

        // Adjust system behavior based on mode
        switch (mode) {
            case AISystemMode::Normal:
                m_config.maxActiveMobs = 500;
                break;
            case AISystemMode::Debug:
                m_config.maxActiveMobs = 100;
                m_config.enableDebugDrawing = true;
                break;
            case AISystemMode::Performance:
                m_config.maxActiveMobs = 1000;
                m_config.enableMultithreading = true;
                break;
            case AISystemMode::Learning:
                m_config.maxActiveMobs = 50;
                m_config.enableLearning = true;
                break;
            case AISystemMode::Minimal:
                m_config.maxActiveMobs = 10;
                m_config.enableMultithreading = false;
                break;
        }
    }

    // Debug and Visualization

    bool AISystem::SetDebugDrawingEnabled(bool enabled) {
        bool previous = m_config.enableDebugDrawing;
        m_config.enableDebugDrawing = enabled;

        if (m_pathfindingSystem) {
            m_pathfindingSystem->SetDebugDrawingEnabled(enabled);
        }

        if (m_villageSystem) {
            m_villageSystem->SetDebugDrawingEnabled(enabled);
        }

        return previous;
    }

    const std::vector<DebugRenderData>& AISystem::GetDebugRenderData() const {
        return m_debugRenderData;
    }

    void AISystem::ClearDebugRenderData() {
        std::unique_lock<std::shared_mutex> lock(m_debugMutex);
        m_debugRenderData.clear();
    }

    // World Integration

    void AISystem::SetWorld(World* world) {
        if (m_world == world) {
            return;
        }

        m_world = world;

        // Initialize subsystems with world
        if (m_pathfindingSystem) {
            m_pathfindingSystem->Initialize(world);
        }

        if (m_villageSystem) {
            // m_villageSystem->Initialize(world, m_proceduralGenerator.get());
        }
    }

    void AISystem::SetEntityManager(EntityManager* entityManager) {
        m_entityManager = entityManager;
    }

    // Utility Functions

    bool AISystem::Validate() const {
        if (!m_isInitialized) {
            return false;
        }

        // Validate subsystems
        if (m_pathfindingSystem && !m_pathfindingSystem->Validate()) {
            return false;
        }

        if (m_villageSystem && !m_villageSystem->Validate()) {
            return false;
        }

        // Validate active mobs
        {
            std::shared_lock<std::shared_mutex> lock(m_mobMutex);
            for (const auto& [id, mob] : m_activeMobs) {
                if (mob && !mob->Validate()) {
                    return false;
                }
            }
        }

        return true;
    }

    std::string AISystem::GetStatusReport() const {
        std::stringstream ss;

        ss << "=== AISystem Status Report ===\n";
        ss << "State: " << static_cast<int>(m_state) << "\n";
        ss << "Mode: " << static_cast<int>(m_config.mode) << "\n";
        ss << "Initialized: " << (m_isInitialized ? "Yes" : "No") << "\n";
        ss << "Active Mobs: " << m_metrics.activeMobs << "\n";
        ss << "Total Mobs: " << m_metrics.totalMobs << "\n";
        ss << "Update Count: " << m_metrics.updateCount << "\n";
        ss << "Average Update Time: " << m_metrics.averageUpdateTime * 1000.0 << "ms\n";
        ss << "Multithreading: " << (m_config.enableMultithreading ? "Enabled" : "Disabled") << "\n";
        ss << "Worker Threads: " << m_workerThreads.size() << "\n";
        ss << "Pending Tasks: " << GetPendingTasksCount() << "\n";

        if (m_pathfindingSystem) {
            ss << "Pathfinding: Active\n";
            ss << "Path Requests: " << m_pathfindingSystem->GetMetrics().requestsProcessed << "\n";
        } else {
            ss << "Pathfinding: Inactive\n";
        }

        if (m_villageSystem) {
            ss << "Villages: Active\n";
            ss << "Generated Villages: " << m_villageSystem->GetMetrics().villagesGenerated << "\n";
        } else {
            ss << "Villages: Inactive\n";
        }

        return ss.str();
    }

    std::string AISystem::GetPerformanceReport() const {
        std::stringstream ss;

        ss << "=== AISystem Performance Report ===\n";
        ss << "Updates: " << m_metrics.updateCount << "\n";
        ss << "Average Update Time: " << m_metrics.averageUpdateTime * 1000.0 << "ms\n";
        ss << "Max Update Time: " << m_metrics.maxUpdateTime * 1000.0 << "ms\n";
        ss << "Active Mobs: " << m_metrics.activeMobs << "\n";
        ss << "Mob Updates: " << m_metrics.mobUpdates << "\n";

        if (m_pathfindingSystem) {
            auto& pathMetrics = m_pathfindingSystem->GetMetrics();
            ss << "Pathfinding Performance:\n";
            ss << "  Requests: " << pathMetrics.requestsProcessed << "\n";
            ss << "  Successful: " << pathMetrics.successfulPaths << "\n";
            ss << "  Failed: " << pathMetrics.failedPaths << "\n";
            ss << "  Average Time: " << pathMetrics.averageProcessingTime * 1000.0 << "ms\n";
            ss << "  Cache Hit Rate: " << pathMetrics.cacheHitRate * 100.0 << "%\n";
        }

        if (m_villageSystem) {
            auto& villageMetrics = m_villageSystem->GetMetrics();
            ss << "Village Performance:\n";
            ss << "  Generated: " << villageMetrics.villagesGenerated << "\n";
            ss << "  Active: " << villageMetrics.villagesActive << "\n";
            ss << "  Average Generation Time: " << villageMetrics.averageGenerationTime * 1000.0 << "ms\n";
        }

        return ss.str();
    }

    std::unordered_map<std::string, size_t> AISystem::OptimizeSystem() {
        std::unordered_map<std::string, size_t> optimizations;

        // Clear inactive mobs
        {
            std::unique_lock<std::shared_mutex> lock(m_mobMutex);
            size_t before = m_activeMobs.size();

            auto it = m_activeMobs.begin();
            while (it != m_activeMobs.end()) {
                auto& mob = it->second;
                if (!mob || !mob->IsAlive()) {
                    it = m_activeMobs.erase(it);
                } else {
                    ++it;
                }
            }

            size_t after = m_activeMobs.size();
            optimizations["inactive_mobs_cleared"] = before - after;
        }

        // Optimize pathfinding cache
        if (m_pathfindingSystem) {
            m_pathfindingSystem->ClearPathCache();
            optimizations["pathfinding_cache_cleared"] = 1;
        }

        // Optimize village system
        if (m_villageSystem) {
            // Village-specific optimizations would go here
            optimizations["village_optimizations"] = 1;
        }

        return optimizations;
    }

} // namespace VoxelCraft
