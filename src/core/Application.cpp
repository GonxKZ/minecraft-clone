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
#include "EventSystem.hpp"
#include "../window/Window.hpp"
#include "../graphics/Renderer.hpp"
#include "../input/InputManager.hpp"
#include "../world/World.hpp"
#include "../player/Player.hpp"
#include "../ui/UIManager.hpp"
#include "../audio/AudioManager.hpp"
#include "../graphics/Renderer.hpp"
#include "../graphics/Camera.hpp"

#include <iostream>
#include <chrono>
#include <thread>
#include <exception>
#include <csignal>
#include <filesystem>

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

            // Initialize input system
            if (!InitializeInputSystem()) {
                VOXELCRAFT_ERROR("Failed to initialize input system");
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

        try {
            // Initialize memory manager
            m_memoryManager = std::make_unique<MemoryManager>();

            // Initialize resource manager
            m_resourceManager = std::make_unique<ResourceManager>();

            // Initialize event system
            m_eventSystem = std::make_unique<EventSystem>();

            VOXELCRAFT_INFO("Core systems initialized successfully");
            return true;

        } catch (const std::exception& e) {
            VOXELCRAFT_ERROR("Failed to initialize core systems: {}", e.what());
            return false;
        }
    }

    bool Application::InitializeGraphicsSystem() {
        VOXELCRAFT_INFO("Initializing graphics system");

        try {
            // Create window with properties from config
            WindowProperties windowProps;
            windowProps.title = "VoxelCraft - Minecraft Clone";
            windowProps.width = m_config.Get("graphics.width", 1280);
            windowProps.height = m_config.Get("graphics.height", 720);
            windowProps.mode = m_config.Get("graphics.fullscreen", false) ? WindowMode::FULLSCREEN : WindowMode::WINDOWED;
            windowProps.vsync = m_config.Get("engine.vsync", true);

            m_window = std::make_unique<Window>(windowProps);

            if (!m_window->Initialize()) {
                VOXELCRAFT_ERROR("Failed to initialize window");
                return false;
            }

            // Set window event callbacks
            WindowEventCallbacks callbacks;
            callbacks.keyCallback = [this](int key, int scancode, int action, int mods) {
                // Handle input events
                HandleKeyEvent(key, scancode, action, mods);
            };
            callbacks.mousePosCallback = [this](double xpos, double ypos) {
                HandleMousePosEvent(xpos, ypos);
            };
            callbacks.mouseButtonCallback = [this](int button, int action, int mods) {
                HandleMouseButtonEvent(button, action, mods);
            };
            callbacks.resizeCallback = [this](int width, int height) {
                HandleResizeEvent(width, height);
            };
            callbacks.closeCallback = [this]() {
                RequestShutdown();
            };

            m_window->SetEventCallbacks(callbacks);

            // Create renderer
            m_renderer = std::make_shared<Renderer>(m_window);
            if (!m_renderer->Initialize()) {
                VOXELCRAFT_ERROR("Failed to initialize renderer");
                return false;
            }

            // Pass renderer to engine
            if (m_engine) {
                m_engine->SetRenderer(m_renderer);
            }

            VOXELCRAFT_INFO("Graphics system initialized successfully");
            return true;

        } catch (const std::exception& e) {
            VOXELCRAFT_ERROR("Failed to initialize graphics system: {}", e.what());
            return false;
        }
    }

    bool Application::InitializeInputSystem() {
        try {
            VOXELCRAFT_INFO("Initializing input system");

            // Create input manager
            m_inputManager = std::make_shared<InputManager>(m_window, m_config);
            if (!m_inputManager->Initialize()) {
                VOXELCRAFT_ERROR("Failed to initialize input manager");
                return false;
            }

            // Pass input manager to engine
            if (m_engine) {
                m_engine->SetInputManager(m_inputManager);
            }

            // Connect input manager with renderer for camera control
            if (m_renderer && m_inputManager) {
                m_inputManager->SetCamera(m_renderer->GetCamera());
            }

            VOXELCRAFT_INFO("Input system initialized successfully");
            return true;

        } catch (const std::exception& e) {
            VOXELCRAFT_ERROR("Failed to initialize input system: {}", e.what());
            return false;
        }
    }

    // Event handling methods
    void Application::HandleKeyEvent(int key, int scancode, int action, int mods) {
        if (m_inputManager) {
            m_inputManager->ProcessKeyEvent(
                InputManager::GLFWKeyToKeyCode(key),
                scancode,
                InputManager::GLFWActionToInputAction(action),
                mods
            );
        }

        // Handle UI input
        if (m_uiManager) {
            m_uiManager->HandleInput(key, action);
        }

        VOXELCRAFT_TRACE("Key event: key={}, scancode={}, action={}, mods={}", key, scancode, action, mods);
    }

    void Application::HandleMousePosEvent(double xpos, double ypos) {
        if (m_inputManager) {
            m_inputManager->ProcessMouseMoveEvent(xpos, ypos);
        }
        VOXELCRAFT_TRACE("Mouse position: x={}, y={}", xpos, ypos);
    }

    void Application::HandleMouseButtonEvent(int button, int action, int mods) {
        if (m_inputManager) {
            m_inputManager->ProcessMouseButtonEvent(
                InputManager::GLFWMouseButtonToMouseButton(button),
                InputManager::GLFWActionToInputAction(action),
                mods
            );
        }
        VOXELCRAFT_TRACE("Mouse button: button={}, action={}", button, action);
    }

    void Application::HandleResizeEvent(int width, int height) {
        if (m_inputManager) {
            m_inputManager->ProcessWindowResizeEvent(width, height);
        }
        VOXELCRAFT_INFO("Window resized to {}x{}", width, height);
    }

    bool Application::InitializeGameSystems() {
        VOXELCRAFT_INFO("Initializing game systems");

        try {
            // Create engine with default config
            Engine::EngineConfig engineConfig;
            engineConfig.targetFPS = 60.0;
            engineConfig.enableMultithreading = true;
            engineConfig.workerThreads = 4;
            m_engine = std::make_unique<Engine>(engineConfig);

            // Initialize the engine
            if (!m_engine->Initialize()) {
                VOXELCRAFT_ERROR("Failed to initialize engine");
                return false;
            }

            // Create world with default settings
            WorldSettings worldSettings;
            worldSettings.worldName = "Minecraft Clone World";
            worldSettings.worldType = WorldType::INFINITE;
            worldSettings.renderDistance = 8;
            worldSettings.simulationDistance = 6;
            m_world = std::make_unique<World>(worldSettings);

            if (!m_world->Initialize()) {
                VOXELCRAFT_ERROR("Failed to initialize world");
                return false;
            }

            // Create player
            m_player = std::make_unique<Player>("Player");

            if (!m_player->Initialize()) {
                VOXELCRAFT_ERROR("Failed to initialize player");
                return false;
            }

            // Set player position in world
            Vec3 playerStartPos(0.0f, 70.0f, 0.0f); // Start above ground
            m_player->Teleport(playerStartPos);

            // Connect player with world
            m_player->SetWorld(m_world);

            // Create UI manager
            m_uiManager = std::make_unique<UIManager>();
            if (!m_uiManager->Initialize()) {
                VOXELCRAFT_ERROR("Failed to initialize UI manager");
                return false;
            }

            // Connect UI manager with player
            m_uiManager->SetPlayer(m_player);

            // Create audio manager
            m_audioManager = std::make_unique<AudioManager>(m_config);
            if (!m_audioManager->Initialize()) {
                VOXELCRAFT_WARN("Failed to initialize audio manager - continuing without audio");
            }

            // Create camera
            m_camera = std::make_shared<Camera>();

            VOXELCRAFT_INFO("Game systems initialized successfully");
            return true;

        } catch (const std::exception& e) {
            VOXELCRAFT_ERROR("Failed to initialize game systems: {}", e.what());
            return false;
        }
    }

    bool Application::InitializeNetworkSystem() {
        VOXELCRAFT_INFO("Initializing network system");

        try {
            // Create network manager - Using stub for now
            // m_networkManager = std::make_unique<NetworkManager>();

            VOXELCRAFT_INFO("Network system initialized (using stub)");
            return true;

        } catch (const std::exception& e) {
            VOXELCRAFT_ERROR("Failed to initialize network system: {}", e.what());
            return false;
        }
    }

    bool Application::InitializeUISystem() {
        VOXELCRAFT_INFO("Initializing UI system");

        try {
            // Create UI manager
            m_uiManager = std::make_unique<UIManager>();
            if (!m_uiManager->Initialize()) {
                VOXELCRAFT_ERROR("Failed to initialize UI manager");
                return false;
            }

            VOXELCRAFT_INFO("UI system initialized successfully");
            return true;

        } catch (const std::exception& e) {
            VOXELCRAFT_ERROR("Failed to initialize UI system: {}", e.what());
            return false;
        }
    }

    bool Application::InitializeDevelopmentTools() {
        VOXELCRAFT_INFO("Initializing development tools");

        try {
            // Create save manager
            m_saveManager = std::make_unique<SaveManager>();
            if (!m_saveManager->Initialize(m_config)) {
                VOXELCRAFT_ERROR("Failed to initialize save manager");
                return false;
            }

            // Create profiler
            m_profiler = std::make_unique<Profiler>();
            if (!m_profiler->Initialize()) {
                VOXELCRAFT_ERROR("Failed to initialize profiler");
                return false;
            }

            VOXELCRAFT_INFO("Development tools initialized successfully");
            return true;

        } catch (const std::exception& e) {
            VOXELCRAFT_ERROR("Failed to initialize development tools: {}", e.what());
            return false;
        }
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

        // Update input system
        if (m_inputManager) {
            m_inputManager->Update(deltaTime);
        }

        // Update world with player position for chunk loading
        if (m_world && m_player) {
            Vec3 playerPos = m_player->GetPosition();
            m_world->Update(deltaTime, playerPos);
        } else if (m_world) {
            // Default position if no player
            Vec3 defaultPos(0, 2, 0);
            m_world->Update(deltaTime, defaultPos);
        }

        // Update player with input
        if (m_player && m_inputManager) {
            // Process player input
            PlayerInput input;
            auto& inputState = m_inputManager->GetInputState();

            // Map input state to player input
            input.moveForward = inputState.pressedKeys.find(KeyCode::W) != inputState.pressedKeys.end() ? 1.0f : 0.0f;
            input.moveRight = inputState.pressedKeys.find(KeyCode::D) != inputState.pressedKeys.end() ? 1.0f : 0.0f;
            if (inputState.pressedKeys.find(KeyCode::S) != inputState.pressedKeys.end()) {
                input.moveForward = -1.0f;
            }
            if (inputState.pressedKeys.find(KeyCode::A) != inputState.pressedKeys.end()) {
                input.moveRight = -1.0f;
            }

            input.jumpPressed = inputState.pressedKeys.find(KeyCode::SPACE) != inputState.pressedKeys.end();
            input.sneakPressed = inputState.pressedKeys.find(KeyCode::LEFT_SHIFT) != inputState.pressedKeys.end();
            input.sprintPressed = inputState.pressedKeys.find(KeyCode::LEFT_CONTROL) != inputState.pressedKeys.end();

            m_player->HandleInput(input, deltaTime);
            m_player->Update(deltaTime);
        }

        // Update UI
        if (m_uiManager) {
            m_uiManager->Update(deltaTime);
        }

        // Update audio
        if (m_audioManager) {
            m_audioManager->Update(deltaTime);
        }

        // Update network - Using stub
        // if (m_networkManager) {
        //     m_networkManager->Update(deltaTime);
        // }

        // Update memory manager
        if (m_memoryManager) {
            m_memoryManager->UpdateStatistics();
        }

        // Update resource manager
        if (m_resourceManager) {
            // Resource manager doesn't need per-frame updates
        }
    }

    void Application::Render() {
        // Begin frame
        if (m_renderer) {
            m_renderer->BeginFrame();
        }

        // Clear screen
        if (m_renderer) {
            Vec4 clearColor(0.2f, 0.3f, 0.8f, 1.0f);
            m_renderer->Clear(clearColor);
        }

        // Render world with camera position
        if (m_world && m_camera) {
            Vec3 cameraPos = m_camera->GetPosition();
            m_world->Render(cameraPos);
        } else if (m_world) {
            // Default camera position if no camera
            Vec3 defaultPos(0, 2, 5);
            m_world->Render(defaultPos);
        }

        // Render engine (includes camera updates and test rendering)
        if (m_engine) {
            m_engine->Render();
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
            m_window->Update(); // This processes events and input
        }

        // Check if window should close
        if (m_window && m_window->ShouldClose()) {
            RequestShutdown();
        }

        // Process input events - Engine handles this
        if (m_engine) {
            // Engine will handle input processing internally
        }

        // Process network events - Using stub
        // if (m_networkManager) {
        //     m_networkManager->ProcessEvents();
        // }

        // Process event system
        if (m_eventSystem) {
            // Dispatch any pending events
            m_eventSystem->DispatchEvents();
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

        try {
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
            if (m_engine) {
                m_engine->Shutdown();
                m_engine.reset();
            }

            // Graphics system
            m_renderer.reset();
            m_window.reset();

            // Core systems
            m_resourceManager.reset();
            if (m_memoryManager) {
                m_memoryManager->Shutdown();
                m_memoryManager.reset();
            }
            if (m_eventSystem) {
                m_eventSystem->Stop();
                m_eventSystem.reset();
            }

            VOXELCRAFT_INFO("Application cleanup completed successfully");

        } catch (const std::exception& e) {
            VOXELCRAFT_ERROR("Exception during cleanup: {}", e.what());
        }
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
