/**
 * @file Engine.cpp
 * @brief VoxelCraft Engine Core Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file implements the main Engine class that coordinates all engine subsystems.
 */

#include "Engine.hpp"
#include "Logger.hpp"

#include <iostream>
#include <algorithm>
#include <stdexcept>

#include "../entities/EntityManager.hpp"
#include "../entities/RenderSystem.hpp"
#include "../entities/ECSExample.hpp"

namespace VoxelCraft {

    // Global engine instance
    static std::unique_ptr<Engine> s_instance;
    static std::mutex s_instanceMutex;

    // Engine implementation

    Engine::Engine(const EngineConfig& config)
        : m_config(config)
        , m_state(EngineState::Uninitialized)
        , m_gameState(GameState::Loading)
        , m_shutdownRequested(false)
        , m_exitCode(0)
        , m_accumulatedTime(0.0)
        , m_lastUpdateTime(0.0)
        , m_workersRunning(false)
        , m_nextTaskId(1)
        , m_profilingEnabled(config.enableProfiling)
        , m_debugMode(false)
    {
        VOXELCRAFT_TRACE("Engine instance created");

        // Set engine start time
        m_startTime = std::chrono::steady_clock::now();
        m_lastFrameTime = m_startTime;
        m_profileStartTime = m_startTime;
    }

    Engine::~Engine() {
        VOXELCRAFT_TRACE("Engine instance destroyed");

        if (m_state != EngineState::Uninitialized) {
            Shutdown();
        }
    }

    bool Engine::Initialize() {
        std::lock_guard<std::mutex> lock(m_stateMutex);

        if (m_state != EngineState::Uninitialized) {
            VOXELCRAFT_ERROR("Engine already initialized");
            return false;
        }

        m_state = EngineState::Initializing;

        VOXELCRAFT_INFO("Initializing VoxelCraft Engine v{}", GetVersion());

        try {
            // Initialize subsystems
            if (!InitializeSubsystems()) {
                VOXELCRAFT_ERROR("Failed to initialize subsystems");
                m_state = EngineState::Error;
                return false;
            }

            // Initialize worker threads if multithreading is enabled
            if (m_config.enableMultithreading) {
                if (!InitializeWorkerThreads()) {
                    VOXELCRAFT_ERROR("Failed to initialize worker threads");
                    m_state = EngineState::Error;
                    return false;
                }
            }

            m_state = EngineState::Running;
            m_gameState = GameState::MainMenu;

            VOXELCRAFT_INFO("Engine initialized successfully");
            VOXELCRAFT_INFO("Target FPS: {}, Multithreading: {}, Profiling: {}",
                          m_config.targetFPS,
                          m_config.enableMultithreading,
                          m_profilingEnabled);

            return true;

        } catch (const std::exception& e) {
            VOXELCRAFT_ERROR("Exception during engine initialization: {}", e.what());
            m_state = EngineState::Error;
            return false;
        }
    }

    int Engine::Run() {
        if (m_state != EngineState::Running) {
            VOXELCRAFT_ERROR("Engine not initialized or in error state");
            return 1;
        }

        VOXELCRAFT_INFO("Starting main game loop");

        m_lastFrameTime = std::chrono::steady_clock::now();
        m_accumulatedTime = 0.0;

        while (!m_shutdownRequested) {
            auto currentTime = std::chrono::steady_clock::now();
            double frameTime = std::chrono::duration_cast<std::chrono::duration<double>>(
                currentTime - m_lastFrameTime
            ).count();

            m_lastFrameTime = currentTime;

            // Limit frame time to prevent large jumps
            if (frameTime > m_config.maxFrameTime) {
                frameTime = m_config.maxFrameTime;
            }

            // Update metrics
            UpdateMetrics(frameTime);

            // Process frame
            ProcessFrame(frameTime);

            // Calculate sleep time to maintain target frame rate
            auto frameEndTime = std::chrono::steady_clock::now();
            double actualFrameTime = std::chrono::duration_cast<std::chrono::duration<double>>(
                frameEndTime - currentTime
            ).count();

            double targetFrameTime = 1.0 / m_config.targetFPS;
            double sleepTime = targetFrameTime - actualFrameTime;

            if (sleepTime > 0.0) {
                std::this_thread::sleep_for(std::chrono::duration<double>(sleepTime));
            }
        }

        VOXELCRAFT_INFO("Main game loop ended");
        return m_exitCode;
    }

    void Engine::Shutdown() {
        std::lock_guard<std::mutex> lock(m_stateMutex);

        if (m_state == EngineState::Uninitialized) {
            return;
        }

        VOXELCRAFT_INFO("Shutting down engine");

        m_state = EngineState::ShuttingDown;
        m_shutdownRequested = true;

        try {
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

            // Cleanup resources
            Cleanup();

            m_state = EngineState::Uninitialized;

            auto endTime = std::chrono::steady_clock::now();
            auto runtime = std::chrono::duration_cast<std::chrono::seconds>(endTime - m_startTime);

            VOXELCRAFT_INFO("Engine shutdown complete. Total runtime: {} seconds", runtime.count());

        } catch (const std::exception& e) {
            VOXELCRAFT_ERROR("Exception during engine shutdown: {}", e.what());
        }
    }

    void Engine::Pause() {
        if (m_state == EngineState::Running) {
            m_state = EngineState::Paused;
            m_gameState = GameState::Paused;
            VOXELCRAFT_INFO("Engine paused");
        }
    }

    void Engine::Resume() {
        if (m_state == EngineState::Paused) {
            m_state = EngineState::Running;
            m_gameState = GameState::Playing;
            m_lastFrameTime = std::chrono::steady_clock::now();
            VOXELCRAFT_INFO("Engine resumed");
        }
    }

    void Engine::RequestShutdown(int exitCode) {
        m_shutdownRequested = true;
        m_exitCode = exitCode;
    }

    void Engine::SetGameState(GameState state) {
        m_gameState = state;
        VOXELCRAFT_INFO("Game state changed to: {}",
                       state == GameState::Loading ? "Loading" :
                       state == GameState::MainMenu ? "MainMenu" :
                       state == GameState::Playing ? "Playing" :
                       state == GameState::Paused ? "Paused" :
                       state == GameState::Saving ? "Saving" :
                       state == GameState::LoadingLevel ? "LoadingLevel" :
                       "Exiting");
    }

    bool Engine::InitializeSubsystems() {
        VOXELCRAFT_INFO("Initializing engine subsystems");

        try {
            // Initialize entity manager
            m_entityManager = std::make_unique<EntityManager>();

            // Initialize render system
            m_renderSystem = std::make_unique<RenderSystem>("MainRenderSystem");
            m_renderSystem->SetEntityManager(m_entityManager.get());
            if (!m_renderSystem->Initialize()) {
                VOXELCRAFT_ERROR("Failed to initialize render system");
                return false;
            }

            // Create some test entities with full components
            auto entity1 = m_entityManager->CreateEntity("TestEntity1");
            if (entity1) {
                entity1->AddComponent<TransformComponent>(glm::vec3(0.0f, 0.0f, 0.0f));
                entity1->AddComponent<RenderComponent>();
                VOXELCRAFT_INFO("Created test entity 1 with Transform and Render components");
            }

            auto entity2 = m_entityManager->CreateEntity("TestEntity2");
            if (entity2) {
                entity2->AddComponent<TransformComponent>(glm::vec3(5.0f, 0.0f, 5.0f));
                entity2->AddComponent<RenderComponent>();
                VOXELCRAFT_INFO("Created test entity 2 with Transform and Render components");
            }

            // Create a camera entity
            auto camera = m_entityManager->CreateEntity("MainCamera");
            if (camera) {
                camera->AddComponent<TransformComponent>(glm::vec3(0.0f, 2.0f, 10.0f));
                m_renderSystem->SetActiveCamera(camera);
                VOXELCRAFT_INFO("Created main camera entity");
            }

            // Initialize ECS example if in development mode
            if (InitializeECSExample()) {
                VOXELCRAFT_INFO("ECS Example initialized successfully");
            } else {
                VOXELCRAFT_WARNING("Failed to initialize ECS Example");
            }

            VOXELCRAFT_INFO("Entity manager initialized with {} entities",
                          m_entityManager->GetEntityCount());
            VOXELCRAFT_INFO("Core subsystems initialized");
            return true;

        } catch (const std::exception& e) {
            VOXELCRAFT_ERROR("Exception during subsystem initialization: {}", e.what());
            return false;
        }
    }

    bool Engine::InitializeWorkerThreads() {
        VOXELCRAFT_INFO("Initializing worker threads");

        try {
            m_workersRunning = true;

            for (int i = 0; i < m_config.workerThreads; ++i) {
                m_workerThreads.emplace_back(&Engine::WorkerThreadFunction, this, i);
            }

            VOXELCRAFT_INFO("Initialized {} worker threads", m_config.workerThreads);
            return true;

        } catch (const std::exception& e) {
            VOXELCRAFT_ERROR("Exception during worker thread initialization: {}", e.what());
            return false;
        }
    }

    void Engine::MainLoop() {
        // This is called by Run() - implementation is in ProcessFrame
    }

    void Engine::ProcessFrame(double deltaTime) {
        // Handle different game states
        switch (m_gameState) {
            case GameState::Loading:
                // Handle loading logic
                break;
            case GameState::MainMenu:
                // Handle main menu logic
                break;
            case GameState::Playing:
                // Handle playing logic
                Update(deltaTime);
                FixedUpdate(m_config.fixedTimestep);
                Render();
                break;
            case GameState::Paused:
                // Handle paused logic - minimal updates
                break;
            case GameState::Saving:
                // Handle saving logic
                break;
            case GameState::LoadingLevel:
                // Handle level loading logic
                break;
            case GameState::Exiting:
                RequestShutdown(0);
                break;
        }

        // Process asynchronous tasks
        ProcessTasks();
    }

    void Engine::Update(double deltaTime) {
        // Update all subsystems
        auto updateStart = std::chrono::steady_clock::now();

        // Update entity manager
        if (m_entityManager) {
            m_entityManager->UpdateEntities(deltaTime);
        }

        // Update render system
        if (m_renderSystem) {
            m_renderSystem->Update(deltaTime);
        }

        // Update ECS example
        UpdateECSExample(deltaTime);

        // In a real implementation, this would update all subsystems:
        // - Input processing
        // - Game logic
        // - UI updates
        // - Network updates

        auto updateEnd = std::chrono::steady_clock::now();
        m_metrics.updateTime = std::chrono::duration_cast<std::chrono::duration<double>>(
            updateEnd - updateStart
        ).count();
    }

    void Engine::FixedUpdate(double fixedDeltaTime) {
        // Fixed timestep updates for physics
        m_accumulatedTime += m_metrics.frameTime;

        while (m_accumulatedTime >= fixedDeltaTime) {
            // Update physics and other fixed timestep systems
            auto physicsStart = std::chrono::steady_clock::now();

            // In a real implementation, this would update:
            // - Physics simulation
            // - Animation systems
            // - Network synchronization

            auto physicsEnd = std::chrono::steady_clock::now();
            m_metrics.physicsTime = std::chrono::duration_cast<std::chrono::duration<double>>(
                physicsEnd - physicsStart
            ).count();

            m_accumulatedTime -= fixedDeltaTime;
        }
    }

    void Engine::Render() {
        // Render current frame
        auto renderStart = std::chrono::steady_clock::now();

        // Render entities through render system
        if (m_renderSystem) {
            m_renderSystem->Render();
        } else if (m_entityManager) {
            // Fallback to direct entity rendering
            m_entityManager->RenderEntities();
        }

        // Render ECS example
        RenderECSExample();

        // In a real implementation, this would:
        // - Render 3D world
        // - Render UI
        // - Apply post-processing effects
        // - Present to screen

        auto renderEnd = std::chrono::steady_clock::now();
        m_metrics.renderTime = std::chrono::duration_cast<std::chrono::duration<double>>(
            renderEnd - renderStart
        ).count();
    }

    void Engine::ProcessTasks() {
        std::lock_guard<std::mutex> lock(m_taskMutex);

        // Process completed tasks and start new ones
        // In a real implementation, this would manage the task queue
        m_metrics.queuedTasks = m_taskQueue.size();
    }

    void Engine::WorkerThreadFunction(int threadId) {
        VOXELCRAFT_INFO("Worker thread {} started", threadId);

        while (m_workersRunning) {
            std::unique_lock<std::mutex> lock(m_taskMutex);

            m_taskCondition.wait(lock, [this]() {
                return !m_taskQueue.empty() || !m_workersRunning;
            });

            if (!m_workersRunning) {
                break;
            }

            if (!m_taskQueue.empty()) {
                Task task = std::move(m_taskQueue.front());
                m_taskQueue.pop();
                lock.unlock();

                // Execute task
                try {
                    task.function();
                    m_metrics.processedTasks++;
                } catch (const std::exception& e) {
                    VOXELCRAFT_ERROR("Exception in worker thread {}: {}", threadId, e.what());
                }
            }
        }

        VOXELCRAFT_INFO("Worker thread {} stopped", threadId);
    }

    void Engine::UpdateMetrics(double deltaTime) {
        m_metrics.frameTime = deltaTime;
        m_metrics.fps = 1.0 / deltaTime;
        m_metrics.frameCount++;

        // Update average FPS
        static double fpsAccumulator = 0.0;
        static int fpsSampleCount = 0;
        const int fpsSampleSize = 60;

        fpsAccumulator += m_metrics.fps;
        fpsSampleCount++;

        if (fpsSampleCount >= fpsSampleSize) {
            m_metrics.averageFPS = fpsAccumulator / fpsSampleCount;
            fpsAccumulator = 0.0;
            fpsSampleCount = 0;
        }

        // Update total time
        auto now = std::chrono::steady_clock::now();
        m_metrics.totalTime = std::chrono::duration_cast<std::chrono::duration<double>>(
            now - m_startTime
        ).count();

        // Update thread count
        m_metrics.activeThreads = m_workerThreads.size() + 1; // +1 for main thread
    }

    void Engine::HandleError(const std::string& error) {
        VOXELCRAFT_ERROR("Engine error: {}", error);
        m_lastError = error;
        m_state = EngineState::Error;

        // In a real implementation, you might want to:
        // - Show error dialog
        // - Attempt recovery
        // - Generate crash dump
        // - Save error log
    }

    void Engine::Cleanup() {
        VOXELCRAFT_INFO("Cleaning up engine resources");

        // Clean up subsystems in reverse order of initialization

        // Shutdown ECS example
        ShutdownECSExample();

        // Clean up render system
        if (m_renderSystem) {
            m_renderSystem->Shutdown();
            m_renderSystem.reset();
        }

        // Clean up entity manager
        m_entityManager.reset();

        // Note: In a real implementation, this would properly clean up all other subsystems

        VOXELCRAFT_INFO("Engine cleanup completed");
    }

    uint64_t Engine::AddTask(std::function<void()> task, int priority) {
        std::lock_guard<std::mutex> lock(m_taskMutex);

        uint64_t taskId = m_nextTaskId++;
        Task newTask{taskId, task, priority, std::chrono::steady_clock::now()};

        m_taskQueue.push(newTask);
        m_taskCondition.notify_one();

        return taskId;
    }

    bool Engine::WaitForTask(uint64_t taskId, uint32_t timeoutMs) {
        // Simplified implementation - in a real system this would track task completion
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        return true;
    }

    bool Engine::CancelTask(uint64_t taskId) {
        // Simplified implementation - in a real system this would remove from queue
        return false;
    }

    size_t Engine::GetQueuedTaskCount() const {
        std::lock_guard<std::mutex> lock(m_taskMutex);
        return m_taskQueue.size();
    }

    std::string Engine::TakePerformanceSnapshot() {
        std::stringstream ss;
        ss << "=== Performance Snapshot ===\n";
        ss << "FPS: " << m_metrics.fps << " (avg: " << m_metrics.averageFPS << ")\n";
        ss << "Frame Time: " << m_metrics.frameTime * 1000.0 << "ms\n";
        ss << "Update Time: " << m_metrics.updateTime * 1000.0 << "ms\n";
        ss << "Render Time: " << m_metrics.renderTime * 1000.0 << "ms\n";
        ss << "Physics Time: " << m_metrics.physicsTime * 1000.0 << "ms\n";
        ss << "Total Time: " << m_metrics.totalTime << "s\n";
        ss << "Frame Count: " << m_metrics.frameCount << "\n";
        ss << "Active Threads: " << m_metrics.activeThreads << "\n";
        ss << "Queued Tasks: " << m_metrics.queuedTasks << "\n";
        ss << "Processed Tasks: " << m_metrics.processedTasks << "\n";
        return ss.str();
    }

    void Engine::SetProfilingEnabled(bool enabled) {
        m_profilingEnabled = enabled;
        if (enabled) {
            m_profileStartTime = std::chrono::steady_clock::now();
        }
    }

    double Engine::GetTime() const {
        auto now = std::chrono::steady_clock::now();
        return std::chrono::duration_cast<std::chrono::duration<double>>(
            now - m_startTime
        ).count();
    }

    std::string Engine::GetVersion() {
        return "1.0.0";
    }

    // Global engine accessor

    Engine& GetEngine() {
        std::lock_guard<std::mutex> lock(s_instanceMutex);
        if (!s_instance) {
            s_instance = std::make_unique<Engine>();
        }
        return *s_instance;
    }

} // namespace VoxelCraft
