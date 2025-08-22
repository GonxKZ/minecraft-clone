/**
 * @file Engine.hpp
 * @brief VoxelCraft Engine Core - Main Game Engine
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines the main Engine class that coordinates all engine subsystems
 * and provides the core game loop with advanced timing, threading, and performance
 * monitoring capabilities.
 */

#ifndef VOXELCRAFT_CORE_ENGINE_HPP
#define VOXELCRAFT_CORE_ENGINE_HPP

#include <memory>
#include <string>
#include <chrono>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>

#include "Config.hpp"
#include "Logger.hpp"
#include "../entities/EntityManager.hpp"

namespace VoxelCraft {

    // Forward declarations
    class Window;
    class Renderer;
    class PhysicsEngine;
    class AudioEngine;
    class InputManager;
    class World;
    class Player;
    class UIManager;
    class NetworkManager;
    class SaveManager;
    class Profiler;
    class ResourceManager;
    class ScriptEngine;
    class EntityManager;
    class RenderSystem;
    class ProceduralGenerator;

    /**
     * @enum EngineState
     * @brief Current state of the game engine
     */
    enum class EngineState {
        Uninitialized,    ///< Engine not yet initialized
        Initializing,     ///< Engine is initializing subsystems
        Running,         ///< Engine is running normally
        Paused,          ///< Engine is paused
        ShuttingDown,    ///< Engine is shutting down
        Error            ///< Engine encountered a fatal error
    };

    /**
     * @enum GameState
     * @brief Current state of the game
     */
    enum class GameState {
        Loading,         ///< Game is loading assets/world
        MainMenu,        ///< Main menu is active
        Playing,         ///< Game is being played
        Paused,          ///< Game is paused
        Saving,          ///< Game is saving
        LoadingLevel,    ///< Loading a new level/area
        Exiting          ///< Game is exiting
    };

    /**
     * @struct EngineMetrics
     * @brief Performance metrics for the engine
     */
    struct EngineMetrics {
        // Frame timing
        double frameTime = 0.0;              ///< Time for last frame (seconds)
        double fps = 0.0;                    ///< Current frames per second
        double averageFPS = 0.0;             ///< Average FPS over time
        uint64_t frameCount = 0;             ///< Total frames rendered
        double totalTime = 0.0;              ///< Total engine runtime (seconds)

        // Timing breakdown
        double updateTime = 0.0;             ///< Time spent in game logic updates
        double renderTime = 0.0;             ///< Time spent rendering
        double physicsTime = 0.0;            ///< Time spent in physics simulation
        double audioTime = 0.0;              ///< Time spent in audio processing
        double inputTime = 0.0;              ///< Time spent processing input
        double networkTime = 0.0;            ///< Time spent in network processing

        // Resource usage
        size_t memoryUsage = 0;              ///< Current memory usage (bytes)
        size_t peakMemoryUsage = 0;          ///< Peak memory usage (bytes)
        double cpuUsage = 0.0;               ///< CPU usage percentage
        double gpuUsage = 0.0;               ///< GPU usage percentage

        // Subsystem metrics
        uint32_t activeThreads = 0;          ///< Number of active threads
        uint32_t queuedTasks = 0;            ///< Tasks in queue
        uint64_t processedTasks = 0;         ///< Total tasks processed
    };

    /**
     * @struct EngineConfig
     * @brief Configuration for engine initialization
     */
    struct EngineConfig {
        // Graphics settings
        int windowWidth = 1920;             ///< Window width
        int windowHeight = 1080;            ///< Window height
        bool fullscreen = false;            ///< Fullscreen mode
        bool vsync = true;                  ///< Vertical synchronization
        int msaaLevel = 4;                  ///< MSAA anti-aliasing level
        bool enableDebugRenderer = false;   ///< Enable debug rendering

        // Engine settings
        double targetFPS = 60.0;            ///< Target frames per second
        double fixedTimestep = 1.0/60.0;    ///< Fixed physics timestep
        bool enableMultithreading = true;   ///< Enable multithreaded subsystems
        int workerThreads = 4;              ///< Number of worker threads

        // Performance settings
        size_t maxMemoryUsage = 0;          ///< Maximum memory usage (0 = unlimited)
        double maxFrameTime = 0.1;          ///< Maximum frame time before slowdown
        bool enableProfiling = false;       ///< Enable performance profiling

        // Debug settings
        bool showFPS = true;                ///< Show FPS counter
        bool showPerformanceStats = false;  ///< Show detailed performance stats
        bool enableConsole = true;          ///< Enable developer console
        std::string logLevel = "info";      ///< Logging level
    };

    /**
     * @class Engine
     * @brief Main game engine class
     *
     * The Engine class is the central hub of the VoxelCraft game engine. It manages:
     * - All engine subsystems (graphics, physics, audio, input)
     * - Main game loop with advanced timing
     * - Multithreaded task processing
     * - Performance monitoring and profiling
     * - Engine state management
     * - Resource management and cleanup
     *
     * The engine uses a modular architecture where each subsystem operates
     * independently but is coordinated through the main engine loop.
     */
    class Engine {
    public:
        /**
         * @brief Constructor
         * @param config Engine configuration
         */
        explicit Engine(const EngineConfig& config = EngineConfig());

        /**
         * @brief Destructor
         */
        ~Engine();

        /**
         * @brief Deleted copy constructor
         */
        Engine(const Engine&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        Engine& operator=(const Engine&) = delete;

        // Engine lifecycle

        /**
         * @brief Initialize the engine and all subsystems
         * @return true if initialization successful, false otherwise
         */
        bool Initialize();

        /**
         * @brief Run the main game loop
         * @return Exit code
         */
        int Run();

        /**
         * @brief Shutdown the engine and cleanup resources
         */
        void Shutdown();

        /**
         * @brief Pause the engine
         */
        void Pause();

        /**
         * @brief Resume the engine from paused state
         */
        void Resume();

        /**
         * @brief Request engine shutdown
         * @param exitCode Exit code to return
         */
        void RequestShutdown(int exitCode = 0);

        // Engine state

        /**
         * @brief Get current engine state
         * @return Engine state
         */
        EngineState GetState() const { return m_state; }

        /**
         * @brief Get current game state
         * @return Game state
         */
        GameState GetGameState() const { return m_gameState; }

        /**
         * @brief Set game state
         * @param state New game state
         */
        void SetGameState(GameState state);

        /**
         * @brief Get engine configuration
         * @return Engine configuration
         */
        const EngineConfig& GetConfig() const { return m_config; }

        /**
         * @brief Get engine metrics
         * @return Engine performance metrics
         */
        const EngineMetrics& GetMetrics() const { return m_metrics; }

        // Subsystem access

        /**
         * @brief Get window subsystem
         * @return Window instance
         */
        Window* GetWindow() const { return m_window.get(); }

        /**
         * @brief Get renderer subsystem
         * @return Renderer instance
         */
        Renderer* GetRenderer() const { return m_renderer.get(); }

        /**
         * @brief Get physics engine
         * @return Physics engine instance
         */
        PhysicsEngine* GetPhysicsEngine() const { return m_physicsEngine.get(); }

        /**
         * @brief Get audio engine
         * @return Audio engine instance
         */
        AudioEngine* GetAudioEngine() const { return m_audioEngine.get(); }

        /**
         * @brief Get input manager
         * @return Input manager instance
         */
        InputManager* GetInputManager() const { return m_inputManager.get(); }

        /**
         * @brief Get resource manager
         * @return Resource manager instance
         */
        ResourceManager* GetResourceManager() const { return m_resourceManager.get(); }

        /**
         * @brief Get world instance
         * @return World instance
         */
        World* GetWorld() const { return m_world.get(); }

        /**
         * @brief Get player instance
         * @return Player instance
         */
        Player* GetPlayer() const { return m_player.get(); }

        /**
         * @brief Get UI manager
         * @return UI manager instance
         */
        UIManager* GetUIManager() const { return m_uiManager.get(); }

        /**
         * @brief Get network manager
         * @return Network manager instance
         */
        NetworkManager* GetNetworkManager() const { return m_networkManager.get(); }

        /**
         * @brief Get save manager
         * @return Save manager instance
         */
        SaveManager* GetSaveManager() const { return m_saveManager.get(); }

        /**
         * @brief Get profiler
         * @return Profiler instance
         */
        Profiler* GetProfiler() const { return m_profiler.get(); }

        /**
         * @brief Get entity manager
         * @return Entity manager instance
         */
        EntityManager* GetEntityManager() const { return m_entityManager.get(); }

        /**
         * @brief Get render system
         * @return Render system instance
         */
        RenderSystem* GetRenderSystem() const { return m_renderSystem.get(); }

        /**
         * @brief Get procedural generator
         * @return Procedural generator instance
         */
        ProceduralGenerator* GetProceduralGenerator() const { return m_proceduralGenerator.get(); }

        // Task management

        /**
         * @brief Add task to be executed asynchronously
         * @param task Task function to execute
         * @param priority Task priority
         * @return Task ID for tracking
         */
        uint64_t AddTask(std::function<void()> task, int priority = 0);

        /**
         * @brief Wait for task completion
         * @param taskId Task ID to wait for
         * @param timeoutMs Timeout in milliseconds (0 = infinite)
         * @return true if task completed, false if timeout or invalid ID
         */
        bool WaitForTask(uint64_t taskId, uint32_t timeoutMs = 0);

        /**
         * @brief Cancel pending task
         * @param taskId Task ID to cancel
         * @return true if cancelled, false if not found or already executing
         */
        bool CancelTask(uint64_t taskId);

        /**
         * @brief Get number of queued tasks
         * @return Number of tasks in queue
         */
        size_t GetQueuedTaskCount() const;

        // Performance monitoring

        /**
         * @brief Take performance snapshot
         * @return Performance snapshot data
         */
        std::string TakePerformanceSnapshot();

        /**
         * @brief Enable/disable performance profiling
         * @param enabled Enable state
         */
        void SetProfilingEnabled(bool enabled);

        /**
         * @brief Check if profiling is enabled
         * @return true if enabled, false otherwise
         */
        bool IsProfilingEnabled() const { return m_profilingEnabled; }

        // Engine utilities

        /**
         * @brief Get current time in seconds
         * @return Time since engine start
         */
        double GetTime() const;

        /**
         * @brief Get current frame number
         * @return Frame number
         */
        uint64_t GetFrameNumber() const { return m_metrics.frameCount; }

        /**
         * @brief Get engine version
         * @return Version string
         */
        static std::string GetVersion();

        /**
         * @brief Check if engine is running
         * @return true if running, false otherwise
         */
        bool IsRunning() const { return m_state == EngineState::Running; }

    private:
        /**
         * @brief Initialize all engine subsystems
         * @return true if successful, false otherwise
         */
        bool InitializeSubsystems();

        /**
         * @brief Initialize worker threads
         * @return true if successful, false otherwise
         */
        bool InitializeWorkerThreads();

        /**
         * @brief Main game loop implementation
         */
        void MainLoop();

        /**
         * @brief Process a single frame
         * @param deltaTime Time elapsed since last frame
         */
        void ProcessFrame(double deltaTime);

        /**
         * @brief Update game logic
         * @param deltaTime Time elapsed since last frame
         */
        void Update(double deltaTime);

        /**
         * @brief Fixed update for physics
         * @param fixedDeltaTime Fixed time step
         */
        void FixedUpdate(double fixedDeltaTime);

        /**
         * @brief Render current frame
         */
        void Render();

        /**
         * @brief Process asynchronous tasks
         */
        void ProcessTasks();

        /**
         * @brief Worker thread function
         * @param threadId Worker thread ID
         */
        void WorkerThreadFunction(int threadId);

        /**
         * @brief Update performance metrics
         * @param deltaTime Time elapsed since last frame
         */
        void UpdateMetrics(double deltaTime);

        /**
         * @brief Handle engine errors
         * @param error Error message
         */
        void HandleError(const std::string& error);

        /**
         * @brief Cleanup engine resources
         */
        void Cleanup();

        // Engine configuration and state
        EngineConfig m_config;                      ///< Engine configuration
        EngineState m_state;                        ///< Current engine state
        GameState m_gameState;                      ///< Current game state
        EngineMetrics m_metrics;                    ///< Performance metrics

        // Shutdown management
        std::atomic<bool> m_shutdownRequested;      ///< Shutdown request flag
        std::atomic<int> m_exitCode;                ///< Engine exit code

        // Timing
        std::chrono::steady_clock::time_point m_startTime;    ///< Engine start time
        std::chrono::steady_clock::time_point m_lastFrameTime; ///< Last frame time
        double m_accumulatedTime;                  ///< Accumulated time for fixed timestep
        double m_lastUpdateTime;                   ///< Last update time

        // Engine subsystems
        std::unique_ptr<Window> m_window;                  ///< Main window
        std::unique_ptr<Renderer> m_renderer;              ///< Graphics renderer
        std::unique_ptr<PhysicsEngine> m_physicsEngine;    ///< Physics simulation
        std::unique_ptr<AudioEngine> m_audioEngine;        ///< Audio system
        std::unique_ptr<InputManager> m_inputManager;      ///< Input handling
        std::unique_ptr<ResourceManager> m_resourceManager; ///< Resource management
        std::unique_ptr<World> m_world;                    ///< Game world
        std::unique_ptr<Player> m_player;                  ///< Player instance
        std::unique_ptr<UIManager> m_uiManager;            ///< UI system
        std::unique_ptr<NetworkManager> m_networkManager;  ///< Network system
        std::unique_ptr<SaveManager> m_saveManager;        ///< Save system
        std::unique_ptr<Profiler> m_profiler;              ///< Performance profiler
        std::unique_ptr<ScriptEngine> m_scriptEngine;      ///< Script system
        std::unique_ptr<EntityManager> m_entityManager;    ///< Entity system manager
        std::unique_ptr<RenderSystem> m_renderSystem;      ///< Render system
        std::unique_ptr<ProceduralGenerator> m_proceduralGenerator; ///< Procedural generation system

        // Task management
        struct Task {
            uint64_t id;                                  ///< Task ID
            std::function<void()> function;               ///< Task function
            int priority;                                 ///< Task priority
            std::chrono::steady_clock::time_point submitTime; ///< Submission time
        };

        std::queue<Task> m_taskQueue;                      ///< Task queue
        std::vector<std::thread> m_workerThreads;          ///< Worker threads
        std::atomic<bool> m_workersRunning;                ///< Worker threads running flag
        std::atomic<uint64_t> m_nextTaskId;                ///< Next task ID
        mutable std::mutex m_taskMutex;                    ///< Task queue synchronization
        std::condition_variable m_taskCondition;           ///< Task queue condition variable

        // Threading and synchronization
        mutable std::mutex m_stateMutex;                   ///< State synchronization
        mutable std::mutex m_metricsMutex;          ///< Metrics synchronization

        // Performance profiling
        bool m_profilingEnabled;                           ///< Profiling enabled flag
        std::chrono::steady_clock::time_point m_profileStartTime; ///< Profile start time

        // Debug and monitoring
        bool m_debugMode;                                  ///< Debug mode enabled
        std::string m_lastError;                           ///< Last error message
    };

    /**
     * @brief Global engine instance accessor
     * @return Reference to global engine instance
     */
    Engine& GetEngine();

} // namespace VoxelCraft

#endif // VOXELCRAFT_CORE_ENGINE_HPP
