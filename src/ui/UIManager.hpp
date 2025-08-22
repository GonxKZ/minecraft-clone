/**
 * @file UIManager.hpp
 * @brief VoxelCraft UI Manager - Main UI Controller
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines the UIManager class that provides comprehensive UI management
 * for the VoxelCraft game engine, including HUD, menus, inventory, chat, and
 * advanced UI rendering with modern graphics and animations.
 */

#ifndef VOXELCRAFT_UI_UI_MANAGER_HPP
#define VOXELCRAFT_UI_UI_MANAGER_HPP

#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <chrono>
#include <queue>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../core/Config.hpp"
#include "../core/System.hpp"

namespace VoxelCraft {

    // Forward declarations
    class Window;
    class Renderer;
    class InputSystem;
    class Player;
    class Inventory;
    class World;
    class FontManager;
    class TextureManager;
    class SoundManager;

    class UIManager;
    class UIElement;
    class UIWidget;
    class UILayout;
    class UIStyle;
    class UIRenderer;
    class HUD;
    class MenuSystem;
    class InventoryUI;
    class ChatSystem;
    class SettingsUI;

    /**
     * @enum UIState
     * @brief Global UI system state
     */
    enum class UIState {
        Active,                 ///< UI system is active
        Paused,                 ///< UI system is paused
        Inactive,               ///< UI system is inactive
        Loading,                ///< UI system is loading
        Error                   ///< UI system is in error state
    };

    /**
     * @enum UIMode
     * @brief UI operational modes
     */
    enum class UIMode {
        Game,                   ///< In-game UI mode
        Menu,                   ///< Menu UI mode
        Inventory,              ///< Inventory UI mode
        Chat,                   ///< Chat UI mode
        Settings,               ///< Settings UI mode
        Debug,                  ///< Debug UI mode
        Custom                  ///< Custom UI mode
    };

    /**
     * @enum UIInputMode
     * @brief UI input handling modes
     */
    enum class UIInputMode {
        MouseKeyboard,          ///< Mouse and keyboard input
        Gamepad,                ///< Gamepad input
        Touch,                  ///< Touch input
        Mixed                   ///< Mixed input modes
    };

    /**
     * @enum UIAnimationType
     * @brief Types of UI animations
     */
    enum class UIAnimationType {
        Fade,                   ///< Fade in/out animation
        Slide,                  ///< Slide animation
        Scale,                  ///< Scale animation
        Rotate,                 ///< Rotation animation
        Bounce,                 ///< Bounce animation
        Elastic,                ///< Elastic animation
        Custom                  ///< Custom animation
    };

    /**
     * @enum UIFontType
     * @brief Font types for UI rendering
     */
    enum class UIFontType {
        Regular,                ///< Regular font
        Bold,                   ///< Bold font
        Italic,                 ///< Italic font
        BoldItalic,             ///< Bold italic font
        Light,                  ///< Light font
        Medium,                 ///< Medium font
        Custom                  ///< Custom font
    };

    /**
     * @struct UIManagerConfig
     * @brief UI manager configuration settings
     */
    struct UIManagerConfig {
        // Basic settings
        UIMode defaultMode;                      ///< Default UI mode
        UIInputMode inputMode;                   ///< Input handling mode
        float uiScale;                          ///< Global UI scale factor
        bool enableAnimations;                  ///< Enable UI animations
        bool enableParticles;                   ///< Enable UI particle effects
        bool enableSounds;                      ///< Enable UI sound effects

        // Performance settings
        int maxElements;                        ///< Maximum UI elements
        float updateInterval;                   ///< UI update interval (seconds)
        bool enableMultithreading;              ///< Enable multithreaded UI updates
        bool enableCulling;                     ///< Enable UI culling
        float maxRenderDistance;                ///< Maximum UI render distance

        // Visual settings
        glm::vec4 defaultBackgroundColor;       ///< Default background color
        glm::vec4 defaultTextColor;             ///< Default text color
        std::string defaultFontName;            ///< Default font name
        float defaultFontSize;                  ///< Default font size

        // Layout settings
        float elementSpacing;                   ///< Default element spacing
        float elementPadding;                   ///< Default element padding
        bool enableAutoLayout;                  ///< Enable automatic layout
        bool snapToGrid;                        ///< Snap elements to grid

        // Animation settings
        float animationSpeed;                   ///< Global animation speed
        float fadeDuration;                     ///< Default fade duration
        float slideDuration;                    ///< Default slide duration
        float bounceDuration;                   ///< Default bounce duration

        // Accessibility settings
        bool highContrast;                      ///< High contrast mode
        bool largeText;                         ///< Large text mode
        bool screenReader;                      ///< Screen reader support
        float textToSpeechSpeed;                ///< Text-to-speech speed

        // Debug settings
        bool showDebugInfo;                     ///< Show debug information
        bool showBounds;                        ///< Show element bounds
        bool showLayoutGuides;                  ///< Show layout guides
        bool enableProfiling;                   ///< Enable UI profiling
    };

    /**
     * @struct UIManagerMetrics
     * @brief Performance metrics for UI manager
     */
    struct UIManagerMetrics {
        // Performance metrics
        uint64_t updateCount;                  ///< Number of updates performed
        double totalUpdateTime;                ///< Total update time (ms)
        double averageUpdateTime;              ///< Average update time (ms)
        double maxUpdateTime;                  ///< Maximum update time (ms)

        // Element metrics
        uint32_t activeElements;               ///< Number of active elements
        uint32_t totalElements;                ///< Total number of elements
        uint32_t visibleElements;              ///< Number of visible elements
        uint32_t animatedElements;             ///< Number of animated elements

        // Render metrics
        uint32_t drawCalls;                    ///< Number of draw calls
        uint32_t verticesDrawn;                ///< Number of vertices drawn
        uint32_t texturesUsed;                 ///< Number of textures used
        size_t memoryUsage;                    ///< Memory usage (bytes)

        // Animation metrics
        uint32_t activeAnimations;             ///< Number of active animations
        uint32_t completedAnimations;          ///< Number of completed animations
        uint32_t pendingAnimations;            ///< Number of pending animations

        // Input metrics
        uint32_t inputEvents;                  ///< Number of input events processed
        uint32_t mouseEvents;                  ///< Number of mouse events
        uint32_t keyboardEvents;               ///< Number of keyboard events
        uint32_t touchEvents;                  ///< Number of touch events

        // Error metrics
        uint32_t renderErrors;                 ///< Number of render errors
        uint32_t layoutErrors;                 ///< Number of layout errors
        uint32_t inputErrors;                  ///< Number of input errors
        uint32_t memoryErrors;                 ///< Number of memory errors
    };

    /**
     * @struct UIEvent
     * @brief UI event structure
     */
    struct UIEvent {
        enum class Type {
            MouseClick,                        ///< Mouse click event
            MouseMove,                         ///< Mouse move event
            MouseEnter,                        ///< Mouse enter event
            MouseLeave,                         ///< Mouse leave event
            KeyPress,                          ///< Key press event
            KeyRelease,                        ///< Key release event
            TextInput,                         ///< Text input event
            FocusGained,                       ///< Focus gained event
            FocusLost,                         ///< Focus lost event
            AnimationComplete,                 ///< Animation complete event
            ElementAdded,                      ///< Element added event
            ElementRemoved,                    ///< Element removed event
            Custom                             ///< Custom event
        };

        Type type;                              ///< Event type
        uint32_t elementId;                     ///< Element ID
        std::string elementName;                ///< Element name
        glm::vec2 position;                     ///< Event position
        int button;                             ///< Mouse button (for mouse events)
        int key;                                ///< Key code (for keyboard events)
        std::string text;                       ///< Text input (for text events)
        double timestamp;                       ///< Event timestamp
        std::unordered_map<std::string, std::any> data; ///< Additional event data
    };

    /**
     * @class UIManager
     * @brief Main UI manager for VoxelCraft game engine
     *
     * The UIManager class provides comprehensive UI management functionality
     * for the VoxelCraft game engine. It handles HUD, menus, inventory systems,
     * chat interfaces, and advanced UI rendering with modern graphics and
     * smooth animations.
     *
     * Key Features:
     * - Modern UI with vector graphics and animations
     * - Comprehensive input handling (mouse, keyboard, touch, gamepad)
     * - Advanced layout system with auto-positioning
     * - Style system with themes and customization
     * - Particle effects and visual feedback
     * - Accessibility support and screen reader compatibility
     * - Performance optimization with culling and batching
     * - Debug tools and profiling capabilities
     *
     * The UI system is designed to be modular and extensible, allowing for
     * easy addition of new UI components and customization of existing ones.
     */
    class UIManager : public System {
    public:
        /**
         * @brief Constructor
         * @param config UI manager configuration
         */
        explicit UIManager(const UIManagerConfig& config);

        /**
         * @brief Destructor
         */
        ~UIManager();

        /**
         * @brief Deleted copy constructor
         */
        UIManager(const UIManager&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        UIManager& operator=(const UIManager&) = delete;

        // System lifecycle

        /**
         * @brief Initialize UI manager
         * @param window Game window
         * @param renderer Graphics renderer
         * @return true if successful, false otherwise
         */
        bool Initialize(Window* window, Renderer* renderer);

        /**
         * @brief Shutdown UI manager
         */
        void Shutdown();

        /**
         * @brief Update UI manager
         * @param deltaTime Time elapsed since last update
         */
        void Update(double deltaTime);

        /**
         * @brief Render UI elements
         */
        void Render();

        /**
         * @brief Get current UI state
         * @return UI state
         */
        UIState GetState() const { return m_state; }

        /**
         * @brief Get current UI mode
         * @return UI mode
         */
        UIMode GetMode() const { return m_mode; }

        // Element management

        /**
         * @brief Add UI element
         * @param element UI element to add
         * @return Element ID or 0 if failed
         */
        uint32_t AddElement(std::shared_ptr<UIElement> element);

        /**
         * @brief Remove UI element
         * @param elementId Element ID
         * @return true if removed, false otherwise
         */
        bool RemoveElement(uint32_t elementId);

        /**
         * @brief Get UI element by ID
         * @param elementId Element ID
         * @return UI element or nullptr if not found
         */
        std::shared_ptr<UIElement> GetElement(uint32_t elementId) const;

        /**
         * @brief Get UI element by name
         * @param name Element name
         * @return UI element or nullptr if not found
         */
        std::shared_ptr<UIElement> GetElement(const std::string& name) const;

        /**
         * @brief Find elements by type
         * @tparam T Element type
         * @return Vector of elements of specified type
         */
        template<typename T>
        std::vector<std::shared_ptr<T>> FindElementsByType() const;

        /**
         * @brief Clear all UI elements
         */
        void ClearElements();

        // Layout management

        /**
         * @brief Create layout
         * @param name Layout name
         * @param type Layout type
         * @return Layout ID or 0 if failed
         */
        uint32_t CreateLayout(const std::string& name, const std::string& type);

        /**
         * @brief Apply layout to elements
         * @param layoutId Layout ID
         * @param elementIds Element IDs
         * @return true if successful, false otherwise
         */
        bool ApplyLayout(uint32_t layoutId, const std::vector<uint32_t>& elementIds);

        /**
         * @brief Update layouts
         * @param screenSize New screen size
         */
        void UpdateLayouts(const glm::vec2& screenSize);

        // Style management

        /**
         * @brief Load style sheet
         * @param styleSheet Style sheet data
         * @return true if successful, false otherwise
         */
        bool LoadStyleSheet(const std::string& styleSheet);

        /**
         * @brief Apply style to element
         * @param elementId Element ID
         * @param styleName Style name
         * @return true if successful, false otherwise
         */
        bool ApplyStyle(uint32_t elementId, const std::string& styleName);

        /**
         * @brief Set global theme
         * @param themeName Theme name
         * @return true if successful, false otherwise
         */
        bool SetTheme(const std::string& themeName);

        // Input handling

        /**
         * @brief Handle mouse input
         * @param position Mouse position
         * @param button Mouse button
         * @param action Button action
         * @param mods Modifier keys
         * @return true if handled, false otherwise
         */
        bool HandleMouseInput(const glm::vec2& position, int button, int action, int mods);

        /**
         * @brief Handle keyboard input
         * @param key Key code
         * @param scancode Scan code
         * @param action Key action
         * @param mods Modifier keys
         * @return true if handled, false otherwise
         */
        bool HandleKeyboardInput(int key, int scancode, int action, int mods);

        /**
         * @brief Handle text input
         * @param text Input text
         * @return true if handled, false otherwise
         */
        bool HandleTextInput(const std::string& text);

        /**
         * @brief Handle touch input
         * @param position Touch position
         * @param action Touch action
         * @return true if handled, false otherwise
         */
        bool HandleTouchInput(const glm::vec2& position, int action);

        // Animation system

        /**
         * @brief Start animation on element
         * @param elementId Element ID
         * @param animationType Animation type
         * @param duration Animation duration
         * @param properties Animation properties
         * @return Animation ID or 0 if failed
         */
        uint32_t StartAnimation(uint32_t elementId, UIAnimationType animationType,
                               float duration, const std::unordered_map<std::string, float>& properties);

        /**
         * @brief Stop animation
         * @param animationId Animation ID
         * @return true if stopped, false otherwise
         */
        bool StopAnimation(uint32_t animationId);

        /**
         * @brief Pause animation
         * @param animationId Animation ID
         * @return true if paused, false otherwise
         */
        bool PauseAnimation(uint32_t animationId);

        /**
         * @brief Resume animation
         * @param animationId Animation ID
         * @return true if resumed, false otherwise
         */
        bool ResumeAnimation(uint32_t animationId);

        // Event system

        /**
         * @brief Add event listener
         * @param eventType Event type
         * @param listener Event listener function
         * @return Listener ID
         */
        uint32_t AddEventListener(UIEvent::Type eventType,
                                 std::function<void(const UIEvent&)> listener);

        /**
         * @brief Remove event listener
         * @param listenerId Listener ID
         * @return true if removed, false otherwise
         */
        bool RemoveEventListener(uint32_t listenerId);

        /**
         * @brief Trigger UI event
         * @param event UI event
         */
        void TriggerEvent(const UIEvent& event);

        // Mode management

        /**
         * @brief Set UI mode
         * @param mode New UI mode
         * @return true if successful, false otherwise
         */
        bool SetMode(UIMode mode);

        /**
         * @brief Switch to game mode
         */
        void SwitchToGameMode();

        /**
         * @brief Switch to menu mode
         */
        void SwitchToMenuMode();

        /**
         * @brief Switch to inventory mode
         */
        void SwitchToInventoryMode();

        /**
         * @brief Switch to chat mode
         */
        void SwitchToChatMode();

        /**
         * @brief Switch to settings mode
         */
        void SwitchToSettingsMode();

        // HUD management

        /**
         * @brief Show HUD
         */
        void ShowHUD();

        /**
         * @brief Hide HUD
         */
        void HideHUD();

        /**
         * @brief Toggle HUD visibility
         */
        void ToggleHUD();

        /**
         * @brief Update HUD with player data
         * @param player Player instance
         */
        void UpdateHUD(Player* player);

        // Menu management

        /**
         * @brief Show main menu
         */
        void ShowMainMenu();

        /**
         * @brief Show pause menu
         */
        void ShowPauseMenu();

        /**
         * @brief Show options menu
         */
        void ShowOptionsMenu();

        /**
         * @brief Hide current menu
         */
        void HideCurrentMenu();

        // Inventory management

        /**
         * @brief Show inventory
         * @param inventory Player inventory
         */
        void ShowInventory(Inventory* inventory);

        /**
         * @brief Hide inventory
         */
        void HideInventory();

        /**
         * @brief Toggle inventory visibility
         */
        void ToggleInventory();

        // Chat management

        /**
         * @brief Show chat
         */
        void ShowChat();

        /**
         * @brief Hide chat
         */
        void HideChat();

        /**
         * @brief Toggle chat visibility
         */
        void ToggleChat();

        /**
         * @brief Add chat message
         * @param message Chat message
         * @param sender Sender name
         * @param color Message color
         */
        void AddChatMessage(const std::string& message, const std::string& sender = "",
                           const glm::vec4& color = glm::vec4(1.0f));

        // Settings management

        /**
         * @brief Show settings menu
         */
        void ShowSettings();

        /**
         * @brief Hide settings menu
         */
        void HideSettings();

        /**
         * @brief Apply settings
         * @param settings Settings data
         */
        void ApplySettings(const std::unordered_map<std::string, std::any>& settings);

        // Configuration

        /**
         * @brief Get UI manager configuration
         * @return Current configuration
         */
        const UIManagerConfig& GetConfig() const { return m_config; }

        /**
         * @brief Set UI manager configuration
         * @param config New configuration
         */
        void SetConfig(const UIManagerConfig& config);

        // Metrics and monitoring

        /**
         * @brief Get UI manager metrics
         * @return Performance metrics
         */
        const UIManagerMetrics& GetMetrics() const { return m_metrics; }

        /**
         * @brief Reset metrics
         */
        void ResetMetrics();

        /**
         * @brief Get performance report
         * @return Detailed performance report
         */
        std::string GetPerformanceReport() const;

        // Utility functions

        /**
         * @brief Convert screen coordinates to UI coordinates
         * @param screenPos Screen position
         * @return UI position
         */
        glm::vec2 ScreenToUICoordinates(const glm::vec2& screenPos) const;

        /**
         * @brief Convert UI coordinates to screen coordinates
         * @param uiPos UI position
         * @return Screen position
         */
        glm::vec2 UIToScreenCoordinates(const glm::vec2& uiPos) const;

        /**
         * @brief Get element at position
         * @param position Position to check
         * @return Element ID or 0 if none found
         */
        uint32_t GetElementAtPosition(const glm::vec2& position) const;

        /**
         * @brief Set focus to element
         * @param elementId Element ID
         * @return true if successful, false otherwise
         */
        bool SetFocus(uint32_t elementId);

        /**
         * @brief Clear focus
         */
        void ClearFocus();

        /**
         * @brief Get focused element
         * @return Focused element ID or 0 if none
         */
        uint32_t GetFocusedElement() const { return m_focusedElement; }

        /**
         * @brief Validate UI manager state
         * @return true if valid, false otherwise
         */
        bool Validate() const;

        /**
         * @brief Get status report
         * @return Detailed status report
         */
        std::string GetStatusReport() const;

        /**
         * @brief Optimize UI manager
         * @return Optimization results
         */
        std::unordered_map<std::string, size_t> Optimize();

    private:
        /**
         * @brief Initialize subsystems
         * @return true if successful, false otherwise
         */
        bool InitializeSubsystems();

        /**
         * @brief Update UI elements
         * @param deltaTime Time elapsed
         */
        void UpdateElements(double deltaTime);

        /**
         * @brief Update animations
         * @param deltaTime Time elapsed
         */
        void UpdateAnimations(double deltaTime);

        /**
         * @brief Update input handling
         * @param deltaTime Time elapsed
         */
        void UpdateInput(double deltaTime);

        /**
         * @brief Render UI elements
         */
        void RenderElements();

        /**
         * @brief Handle element events
         * @param event UI event
         */
        void HandleElementEvent(const UIEvent& event);

        /**
         * @brief Process event queue
         */
        void ProcessEventQueue();

        /**
         * @brief Update performance metrics
         * @param deltaTime Time elapsed
         */
        void UpdateMetrics(double deltaTime);

        /**
         * @brief Handle UI errors
         * @param error Error message
         */
        void HandleError(const std::string& error);

        /**
         * @brief Create default UI elements
         */
        void CreateDefaultElements();

        // UI manager data
        UIManagerConfig m_config;                     ///< Configuration
        UIManagerMetrics m_metrics;                   ///< Performance metrics
        UIState m_state;                             ///< Current state
        UIMode m_mode;                               ///< Current mode
        UIInputMode m_inputMode;                     ///< Input mode

        // Core systems
        Window* m_window;                            ///< Game window
        Renderer* m_renderer;                        ///< Graphics renderer
        InputSystem* m_inputSystem;                  ///< Input system
        FontManager* m_fontManager;                  ///< Font manager
        TextureManager* m_textureManager;            ///< Texture manager
        SoundManager* m_soundManager;                ///< Sound manager

        // Player and world references
        Player* m_player;                            ///< Player reference
        World* m_world;                              ///< World reference

        // UI subsystems
        std::unique_ptr<HUD> m_hud;                  ///< HUD system
        std::unique_ptr<MenuSystem> m_menuSystem;    ///< Menu system
        std::unique_ptr<InventoryUI> m_inventoryUI;  ///< Inventory UI
        std::unique_ptr<ChatSystem> m_chatSystem;    ///< Chat system
        std::unique_ptr<SettingsUI> m_settingsUI;    ///< Settings UI
        std::unique_ptr<UIRenderer> m_uiRenderer;    ///< UI renderer

        // Element management
        std::unordered_map<uint32_t, std::shared_ptr<UIElement>> m_elements; ///< UI elements
        std::vector<uint32_t> m_visibleElements;     ///< Visible elements
        std::unordered_map<std::string, uint32_t> m_elementNames; ///< Element names
        mutable std::shared_mutex m_elementsMutex;   ///< Elements synchronization

        // Layout system
        std::unordered_map<uint32_t, std::shared_ptr<UILayout>> m_layouts; ///< UI layouts
        mutable std::shared_mutex m_layoutsMutex;     ///< Layouts synchronization

        // Style system
        std::unordered_map<std::string, std::shared_ptr<UIStyle>> m_styles; ///< UI styles
        std::string m_currentTheme;                   ///< Current theme
        mutable std::shared_mutex m_stylesMutex;      ///< Styles synchronization

        // Animation system
        struct UIAnimation {
            uint32_t animationId;                     ///< Animation ID
            uint32_t elementId;                       ///< Element ID
            UIAnimationType type;                     ///< Animation type
            double startTime;                         ///< Start time
            double duration;                          ///< Duration
            bool isPlaying;                          ///< Is playing
            bool isPaused;                           ///< Is paused
            std::unordered_map<std::string, float> startValues; ///< Start values
            std::unordered_map<std::string, float> endValues; ///< End values
            std::function<void()> completionCallback; ///< Completion callback
        };

        std::unordered_map<uint32_t, UIAnimation> m_animations; ///< Active animations
        mutable std::shared_mutex m_animationsMutex; ///< Animations synchronization

        // Event system
        std::queue<UIEvent> m_eventQueue;             ///< Event queue
        std::unordered_map<uint32_t, std::function<void(const UIEvent&)>> m_eventListeners; ///< Event listeners
        mutable std::shared_mutex m_eventsMutex;      ///< Events synchronization

        // Input handling
        uint32_t m_focusedElement;                    ///< Currently focused element
        glm::vec2 m_lastMousePosition;                ///< Last mouse position
        bool m_mouseCaptured;                         ///< Mouse is captured
        bool m_keyboardCaptured;                      ///< Keyboard is captured

        // State flags
        bool m_isInitialized;                         ///< System is initialized
        double m_lastUpdateTime;                      ///< Last update time

        static std::atomic<uint32_t> s_nextElementId;  ///< Next element ID counter
        static std::atomic<uint32_t> s_nextAnimationId; ///< Next animation ID counter
        static std::atomic<uint32_t> s_nextListenerId;  ///< Next listener ID counter
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_UI_UI_MANAGER_HPP
