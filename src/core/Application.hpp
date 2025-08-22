/**
 * @file Application.hpp
 * @brief VoxelCraft Engine Main Application Class
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines the main Application class that serves as the entry point
 * and central coordinator for the VoxelCraft engine. It manages the main game
 * loop, subsystem initialization, and application lifecycle.
 */

#ifndef VOXELCRAFT_CORE_APPLICATION_HPP
#define VOXELCRAFT_CORE_APPLICATION_HPP

#include <memory>
#include <string>
#include <chrono>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "Config.hpp"
#include "EventSystem.hpp"

// Forward declarations
namespace VoxelCraft {

    class Engine;
    class Window;
    class Renderer;
    class World;
    class Player;
    class UIManager;
    class NetworkManager;
    class SaveManager;
    class Profiler;
    class MemoryManager;
    class ResourceManager;

    /**
     * @enum ApplicationState
     * @brief Represents the current state of the application
     */
    enum class ApplicationState {
        Uninitialized,      ///< Application not yet initialized
        Initializing,       ///< Application is initializing
        Running,           ///< Application is running normally
        Paused,            ///< Application is paused
        ShuttingDown,      ///< Application is shutting down
        Error              ///< Application encountered an error
    };

    /**
     * @enum GameState
     * @brief Represents the current state of the game
     */
    enum class GameState {
        Loading,           ///< Game is loading
        MainMenu,          ///< Main menu is active
        Playing,           ///< Game is being played
        Paused,            ///< Game is paused
        Saving,            ///< Game is saving
        LoadingLevel,      ///< Loading a level
        Exiting            ///< Game is exiting
    };

    /**
     * @struct ApplicationMetrics
     * @brief Performance and runtime metrics for the application
     */
    struct ApplicationMetrics {
        double frameTime = 0.0;           ///< Time taken for last frame (seconds)
        double fps = 0.0;                 ///< Current frames per second
        double averageFPS = 0.0;          ///< Average FPS over time
        double updateTime = 0.0;          ///< Time spent on updates (seconds)
        double renderTime = 0.0;          ///< Time spent on rendering (seconds)
        double physicsTime = 0.0;         ///< Time spent on physics (seconds)
        uint64_t frameCount = 0;          ///< Total number of frames rendered
        uint64_t totalTime = 0;           ///< Total application runtime (microseconds)
        size_t memoryUsage = 0;           ///< Current memory usage (bytes)
        size_t peakMemoryUsage = 0;       ///< Peak memory usage (bytes)
        uint32_t activeThreads = 0;       ///< Number of active threads
        double cpuUsage = 0.0;            ///< CPU usage percentage
        double gpuUsage = 0.0;            ///< GPU usage percentage
    };

    /**
     * @class Application
     * @brief Main application class for VoxelCraft engine
     *
     * The Application class is responsible for:
     * - Initializing and managing all engine subsystems
     * - Running the main game loop
     * - Handling application lifecycle events
     * - Coordinating between different engine components
     * - Managing performance metrics and profiling
     */
    class Application {
    public:
        /**
         * @brief Constructor
         */
        Application();

        /**
         * @brief Destructor
         */
        ~Application();

        /**
         * @brief Deleted copy constructor
         */
        Application(const Application&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        Application& operator=(const Application&) = delete;

        /**
         * @brief Initialize the application and all subsystems
         * @return true if initialization was successful, false otherwise
         */
        bool Initialize();

        /**
         * @brief Run the main application loop
         * @return Exit code
         */
        int Run();

        /**
         * @brief Shutdown the application and clean up resources
         */
        void Shutdown();

        /**
         * @brief Pause the application
         */
        void Pause();

        /**
         * @brief Resume the application from paused state
         */
        void Resume();

        /**
         * @brief Get the current application state
         * @return Current application state
         */
        ApplicationState GetState() const { return m_state; }

        /**
         * @brief Get application configuration
         * @return Reference to configuration object
         */
        Config& GetConfig() { return m_config; }

        /**
         * @brief Get application metrics
         * @return Reference to metrics object
         */
        const ApplicationMetrics& GetMetrics() const { return m_metrics; }

        /**
         * @brief Request application shutdown
         * @param exitCode Exit code to return
         */
        void RequestShutdown(int exitCode = 0);

        /**
         * @brief Check if application should shutdown
         * @return true if shutdown was requested
         */
        bool ShouldShutdown() const { return m_shutdownRequested; }

        /**
         * @brief Get the main engine instance
         * @return Pointer to engine instance
         */
        Engine* GetEngine() const { return m_engine.get(); }

        /**
         * @brief Get the world instance
         * @return Pointer to world instance
         */
        World* GetWorld() const { return m_world.get(); }

        /**
         * @brief Get the player instance
         * @return Pointer to player instance
         */
        Player* GetPlayer() const { return m_player.get(); }

        /**
         * @brief Get the UI manager instance
         * @return Pointer to UI manager instance
         */
        UIManager* GetUIManager() const { return m_uiManager.get(); }

        /**
         * @brief Get the network manager instance
         * @return Pointer to network manager instance
         */
        NetworkManager* GetNetworkManager() const { return m_networkManager.get(); }

        /**
         * @brief Get the save manager instance
         * @return Pointer to save manager instance
         */
        SaveManager* GetSaveManager() const { return m_saveManager.get(); }

        /**
         * @brief Get the profiler instance
         * @return Pointer to profiler instance
         */
        Profiler* GetProfiler() const { return m_profiler.get(); }

    private:
        /**
         * @brief Initialize core subsystems
         * @return true if successful
         */
        bool InitializeCoreSystems();

        /**
         * @brief Initialize graphics subsystem
         * @return true if successful
         */
        bool InitializeGraphicsSystem();

        /**
         * @brief Initialize world and game systems
         * @return true if successful
         */
        bool InitializeGameSystems();

        /**
         * @brief Initialize networking system
         * @return true if successful
         */
        bool InitializeNetworkSystem();

        /**
         * @brief Initialize UI system
         * @return true if successful
         */
        bool InitializeUISystem();

        /**
         * @brief Initialize development tools
         * @return true if successful
         */
        bool InitializeDevelopmentTools();

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
         * @brief Update application logic
         * @param deltaTime Time elapsed since last frame
         */
        void Update(double deltaTime);

        /**
         * @brief Render current frame
         */
        void Render();

        /**
         * @brief Handle application events
         */
        void HandleEvents();

        /**
         * @brief Update performance metrics
         * @param deltaTime Time elapsed since last frame
         */
        void UpdateMetrics(double deltaTime);

        /**
         * @brief Process command line arguments
         */
        void ProcessCommandLine();

        /**
         * @brief Load application configuration
         */
        void LoadConfiguration();

        /**
         * @brief Save application configuration
         */
        void SaveConfiguration();

        /**
         * @brief Handle application errors
         * @param error Error message
         */
        void HandleError(const std::string& error);

        /**
         * @brief Cleanup resources on shutdown
         */
        void Cleanup();

        // Core systems
        std::unique_ptr<MemoryManager> m_memoryManager;       ///< Memory management system
        std::unique_ptr<ResourceManager> m_resourceManager;   ///< Resource management system
        std::unique_ptr<EventSystem> m_eventSystem;           ///< Event system

        // Engine systems
        std::unique_ptr<Engine> m_engine;                     ///< Main engine instance
        std::unique_ptr<Window> m_window;                     ///< Main window
        std::unique_ptr<Renderer> m_renderer;                 ///< Graphics renderer

        // Game systems
        std::unique_ptr<World> m_world;                       ///< Game world
        std::unique_ptr<Player> m_player;                     ///< Player instance
        std::unique_ptr<UIManager> m_uiManager;               ///< UI management system
        std::unique_ptr<NetworkManager> m_networkManager;     ///< Network management system
        std::unique_ptr<SaveManager> m_saveManager;           ///< Save management system

        // Development tools
        std::unique_ptr<Profiler> m_profiler;                 ///< Performance profiler

        // Configuration and state
        Config m_config;                                      ///< Application configuration
        ApplicationState m_state;                             ///< Current application state
        GameState m_gameState;                                ///< Current game state
        ApplicationMetrics m_metrics;                         ///< Performance metrics

        // Timing
        std::chrono::steady_clock::time_point m_startTime;    ///< Application start time
        std::chrono::steady_clock::time_point m_lastFrameTime; ///< Last frame time
        double m_accumulatedTime;                             ///< Accumulated time for fixed timestep

        // Threading
        std::atomic<bool> m_shutdownRequested;                ///< Shutdown request flag
        std::atomic<int> m_exitCode;                          ///< Application exit code
        mutable std::mutex m_stateMutex;                      ///< State synchronization mutex

        // Performance settings
        double m_targetFrameTime;                             ///< Target frame time
        bool m_useFixedTimestep;                              ///< Use fixed timestep
        int m_maxFrameSkip;                                   ///< Maximum frames to skip

        // Debug settings
        bool m_debugMode;                                     ///< Debug mode enabled
        bool m_profilingEnabled;                              ///< Profiling enabled

        // Friend classes for internal access
        friend class Engine;
        friend class Window;
        friend class ConfigLoader;
    };

    /**
     * @brief Global application instance accessor
     * @return Reference to global application instance
     */
    Application& GetApplication();

    /**
     * @brief Get application configuration globally
     * @return Reference to global configuration
     */
    Config& GetConfig();

} // namespace VoxelCraft

#endif // VOXELCRAFT_CORE_APPLICATION_HPP
