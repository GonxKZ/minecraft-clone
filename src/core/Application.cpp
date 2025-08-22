/**
 * @file Application.cpp
 * @brief VoxelCraft Engine Application Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file implements the main Application class that serves as the entry point
 * and central coordinator for the VoxelCraft engine.
 */

#include "Application.hpp"
#include "Engine.hpp"
#include "Logger.hpp"
#include "Config.hpp"

#include <iostream>
#include <chrono>
#include <thread>
#include <exception>
#include <csignal>

namespace VoxelCraft {

    // Global application instance
    static std::unique_ptr<Application> s_instance;
    static std::mutex s_instanceMutex;

    // Signal handlers
    static void SignalHandler(int signal) {
        if (s_instance) {
            VOXELCRAFT_INFO("Received signal {}, shutting down application...", signal);
            s_instance->RequestShutdown(signal);
        }
    }

    // Application implementation

    Application::Application()
        : m_state(ApplicationState::Uninitialized)
        , m_gameState(GameState::Loading)
        , m_config(std::make_unique<Config>())
        , m_shutdownRequested(false)
        , m_exitCode(0)
        , m_targetFrameTime(1.0 / 60.0) // 60 FPS default
        , m_useFixedTimestep(true)
        , m_maxFrameSkip(5)
        , m_debugMode(false)
        , m_profilingEnabled(false)
    {
        // Set up signal handlers
        std::signal(SIGINT, SignalHandler);
        std::signal(SIGTERM, SignalHandler);

#ifdef _WIN32
        std::signal(SIGBREAK, SignalHandler);
#endif

        VOXELCRAFT_TRACE("Application instance created");
    }

    Application::~Application() {
        VOXELCRAFT_TRACE("Application instance destroyed");

        if (m_state != ApplicationState::Uninitialized) {
            Shutdown();
        }
    }

    bool Application::Initialize() {
        std::lock_guard<std::mutex> lock(m_stateMutex);

        if (m_state != ApplicationState::Uninitialized) {
            VOXELCRAFT_ERROR("Application already initialized");
            return false;
        }

        m_state = ApplicationState::Initializing;
        m_startTime = std::chrono::steady_clock::now();

        VOXELCRAFT_INFO("Initializing VoxelCraft Engine v1.0.0");

        try {
            // Initialize core systems
            if (!InitializeCoreSystems()) {
                VOXELCRAFT_ERROR("Failed to initialize core systems");
                m_state = ApplicationState::Error;
                return false;
            }

            // Initialize graphics system
            if (!InitializeGraphicsSystem()) {
                VOXELCRAFT_ERROR("Failed to initialize graphics system");
                m_state = ApplicationState::Error;
                return false;
            }

            // Initialize game systems
            if (!InitializeGameSystems()) {
                VOXELCRAFT_ERROR("Failed to initialize game systems");
                m_state = ApplicationState::Error;
                return false;
            }

            // Initialize network system
            if (!InitializeNetworkSystem()) {
                VOXELCRAFT_ERROR("Failed to initialize network system");
                m_state = ApplicationState::Error;
                return false;
            }

            // Initialize UI system
            if (!InitializeUISystem()) {
                VOXELCRAFT_ERROR("Failed to initialize UI system");
                m_state = ApplicationState::Error;
                return false;
            }

            // Initialize development tools
            if (!InitializeDevelopmentTools()) {
                VOXELCRAFT_ERROR("Failed to initialize development tools");
                m_state = ApplicationState::Error;
                return false;
            }

            // Load configuration
            LoadConfiguration();

            // Calculate target frame time
            double targetFPS = m_config.Get("engine.target_fps", 60.0);
            m_targetFrameTime = 1.0 / targetFPS;
            m_useFixedTimestep = m_config.Get("engine.fixed_timestep", true);
            m_maxFrameSkip = m_config.Get("engine.max_frame_skip", 5);
            m_debugMode = m_config.Get("engine.debug_mode", false);
            m_profilingEnabled = m_config.Get("profiling.enable_profiler", false);

            m_state = ApplicationState::Running;
            m_gameState = GameState::MainMenu;

            auto endTime = std::chrono::steady_clock::now();
            auto initTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - m_startTime);

            VOXELCRAFT_INFO("Application initialized successfully in {}ms", initTime.count());
            VOXELCRAFT_INFO("Target FPS: {}, Fixed Timestep: {}, Debug Mode: {}",
                          targetFPS, m_useFixedTimestep, m_debugMode);

            return true;

        } catch (const std::exception& e) {
            VOXELCRAFT_ERROR("Exception during initialization: {}", e.what());
            m_state = ApplicationState::Error;
            return false;
        } catch (...) {
            VOXELCRAFT_ERROR("Unknown exception during initialization");
            m_state = ApplicationState::Error;
            return false;
        }
    }

    int Application::Run() {
        if (m_state != ApplicationState::Running) {
            VOXELCRAFT_ERROR("Application not initialized or in error state");
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
            if (frameTime > m_maxFrameTime * m_maxFrameSkip) {
                frameTime = m_maxFrameTime * m_maxFrameSkip;
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

            double sleepTime = m_targetFrameTime - actualFrameTime;
            if (sleepTime > 0.0) {
                std::this_thread::sleep_for(std::chrono::duration<double>(sleepTime));
            }
        }

        VOXELCRAFT_INFO("Main game loop ended");
        return m_exitCode;
    }

    void Application::Shutdown() {
        std::lock_guard<std::mutex> lock(m_stateMutex);

        if (m_state == ApplicationState::Uninitialized) {
            return;
        }

        VOXELCRAFT_INFO("Shutting down application");

        m_state = ApplicationState::ShuttingDown;

        try {
            // Save configuration
            SaveConfiguration();

            // Cleanup resources
            Cleanup();

            m_state = ApplicationState::Uninitialized;

            auto endTime = std::chrono::steady_clock::now();
            auto runtime = std::chrono::duration_cast<std::chrono::seconds>(endTime - m_startTime);

            VOXELCRAFT_INFO("Application shutdown complete. Total runtime: {} seconds", runtime.count());

        } catch (const std::exception& e) {
            VOXELCRAFT_ERROR("Exception during shutdown: {}", e.what());
        } catch (...) {
            VOXELCRAFT_ERROR("Unknown exception during shutdown");
        }
    }

    void Application::Pause() {
        if (m_state == ApplicationState::Running) {
            m_state = ApplicationState::Paused;
            m_gameState = GameState::Paused;
            VOXELCRAFT_INFO("Application paused");
        }
    }

    void Application::Resume() {
        if (m_state == ApplicationState::Paused) {
            m_state = ApplicationState::Running;
            m_gameState = GameState::Playing;
            m_lastFrameTime = std::chrono::steady_clock::now();
            VOXELCRAFT_INFO("Application resumed");
        }
    }

    void Application::RequestShutdown(int exitCode) {
        m_shutdownRequested = true;
        m_exitCode = exitCode;
    }

    bool Application::InitializeCoreSystems() {
        VOXELCRAFT_INFO("Initializing core systems");

        // Initialize memory manager
        // m_memoryManager = std::make_unique<MemoryManager>();

        // Initialize resource manager
        // m_resourceManager = std::make_unique<ResourceManager>();

        // Initialize event system
        // m_eventSystem = std::make_unique<EventSystem>();

        VOXELCRAFT_INFO("Core systems initialized");
        return true;
    }

    bool Application::InitializeGraphicsSystem() {
        VOXELCRAFT_INFO("Initializing graphics system");

        // Create window
        // m_window = std::make_unique<Window>(...);

        // Create renderer
        // m_renderer = std::make_unique<Renderer>(...);

        VOXELCRAFT_INFO("Graphics system initialized");
        return true;
    }

    bool Application::InitializeGameSystems() {
        VOXELCRAFT_INFO("Initializing game systems");

        // Create engine
        // m_engine = std::make_unique<Engine>(...);

        // Create world
        // m_world = std::make_unique<World>(...);

        // Create player
        // m_player = std::make_unique<Player>(...);

        VOXELCRAFT_INFO("Game systems initialized");
        return true;
    }

    bool Application::InitializeNetworkSystem() {
        VOXELCRAFT_INFO("Initializing network system");

        // Create network manager
        // m_networkManager = std::make_unique<NetworkManager>(...);

        VOXELCRAFT_INFO("Network system initialized");
        return true;
    }

    bool Application::InitializeUISystem() {
        VOXELCRAFT_INFO("Initializing UI system");

        // Create UI manager
        // m_uiManager = std::make_unique<UIManager>(...);

        VOXELCRAFT_INFO("UI system initialized");
        return true;
    }

    bool Application::InitializeDevelopmentTools() {
        VOXELCRAFT_INFO("Initializing development tools");

        // Create save manager
        // m_saveManager = std::make_unique<SaveManager>(...);

        // Create profiler
        // m_profiler = std::make_unique<Profiler>(...);

        VOXELCRAFT_INFO("Development tools initialized");
        return true;
    }

    void Application::MainLoop() {
        // This method is called by Run() in a loop
        // Implementation is in ProcessFrame()
    }

    void Application::ProcessFrame(double deltaTime) {
        // Handle events
        HandleEvents();

        // Update game logic
        Update(deltaTime);

        // Render frame
        Render();

        // Update game state based on current state
        switch (m_gameState) {
            case GameState::MainMenu:
                // Handle main menu logic
                break;
            case GameState::Loading:
                // Handle loading logic
                break;
            case GameState::Playing:
                // Handle playing logic
                break;
            case GameState::Paused:
                // Handle paused logic
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
    }

    void Application::Update(double deltaTime) {
        // Update subsystems
        if (m_engine) {
            m_engine->Update(deltaTime);
        }

        // Update world
        if (m_world) {
            m_world->Update(deltaTime);
        }

        // Update player
        if (m_player) {
            m_player->Update(deltaTime);
        }

        // Update UI
        if (m_uiManager) {
            m_uiManager->Update(deltaTime);
        }

        // Update network
        if (m_networkManager) {
            m_networkManager->Update(deltaTime);
        }
    }

    void Application::Render() {
        // Begin frame
        if (m_renderer) {
            m_renderer->BeginFrame();
        }

        // Render world
        if (m_world) {
            m_world->Render();
        }

        // Render UI
        if (m_uiManager) {
            m_uiManager->Render();
        }

        // End frame
        if (m_renderer) {
            m_renderer->EndFrame();
        }

        // Present frame
        if (m_window) {
            m_window->Present();
        }
    }

    void Application::HandleEvents() {
        // Process window events
        if (m_window) {
            m_window->ProcessEvents();
        }

        // Process input events
        if (m_engine && m_engine->GetInputManager()) {
            m_engine->GetInputManager()->ProcessEvents();
        }

        // Process network events
        if (m_networkManager) {
            m_networkManager->ProcessEvents();
        }
    }

    void Application::UpdateMetrics(double deltaTime) {
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

        // Update timing breakdown (simplified)
        m_metrics.updateTime = deltaTime * 0.7;  // 70% of frame time
        m_metrics.renderTime = deltaTime * 0.25; // 25% of frame time
        m_metrics.physicsTime = deltaTime * 0.05; // 5% of frame time
    }

    void Application::LoadConfiguration() {
        try {
            // Load default configuration
            m_config.LoadFromString("", "toml"); // Empty config with defaults

            // Try to load from file
            if (std::filesystem::exists("config/voxelcraft.toml")) {
                m_config.LoadFromFile("config/voxelcraft.toml");
                VOXELCRAFT_INFO("Loaded configuration from config/voxelcraft.toml");
            } else if (std::filesystem::exists("assets/configs/voxelcraft.toml")) {
                m_config.LoadFromFile("assets/configs/voxelcraft.toml");
                VOXELCRAFT_INFO("Loaded configuration from assets/configs/voxelcraft.toml");
            } else {
                VOXELCRAFT_INFO("Using default configuration");
            }

        } catch (const std::exception& e) {
            VOXELCRAFT_ERROR("Failed to load configuration: {}", e.what());
        }
    }

    void Application::SaveConfiguration() {
        try {
            std::filesystem::create_directories("config");
            m_config.SaveToFile("config/voxelcraft.toml");
            VOXELCRAFT_INFO("Configuration saved to config/voxelcraft.toml");
        } catch (const std::exception& e) {
            VOXELCRAFT_ERROR("Failed to save configuration: {}", e.what());
        }
    }

    void Application::HandleError(const std::string& error) {
        VOXELCRAFT_ERROR("Application error: {}", error);
        m_state = ApplicationState::Error;

        // In a real implementation, you might want to:
        // - Show an error dialog
        // - Attempt to save state
        // - Try to recover gracefully
        // - Generate error reports
    }

    void Application::Cleanup() {
        VOXELCRAFT_INFO("Cleaning up application resources");

        // Clean up in reverse order of initialization

        // Development tools
        m_profiler.reset();
        m_saveManager.reset();

        // UI system
        m_uiManager.reset();

        // Network system
        m_networkManager.reset();

        // Game systems
        m_player.reset();
        m_world.reset();
        m_engine.reset();

        // Graphics system
        m_renderer.reset();
        m_window.reset();

        // Core systems
        m_resourceManager.reset();
        m_memoryManager.reset();
        m_eventSystem.reset();

        VOXELCRAFT_INFO("Application cleanup completed");
    }

    // Global application accessor

    Application& GetApplication() {
        std::lock_guard<std::mutex> lock(s_instanceMutex);
        if (!s_instance) {
            s_instance = std::make_unique<Application>();
        }
        return *s_instance;
    }

    Config& GetConfig() {
        return GetApplication().GetConfig();
    }

} // namespace VoxelCraft
