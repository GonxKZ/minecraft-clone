/**
 * @file InputManager.hpp
 * @brief VoxelCraft Input System - Input handling and processing
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_INPUT_INPUT_MANAGER_HPP
#define VOXELCRAFT_INPUT_INPUT_MANAGER_HPP

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <queue>
#include <mutex>

#include "../window/Window.hpp"
#include "../core/Config.hpp"

namespace VoxelCraft {

    // Forward declarations
    class Camera;
    class Player;
    struct Vec2;
    struct Vec3;

    /**
     * @enum KeyCode
     * @brief Keyboard key codes (GLFW-compatible)
     */
    enum class KeyCode {
        // Printable keys
        SPACE = 32,
        APOSTROPHE = 39,
        COMMA = 44,
        MINUS = 45,
        PERIOD = 46,
        SLASH = 47,
        NUM_0 = 48,
        NUM_1 = 49,
        NUM_2 = 50,
        NUM_3 = 51,
        NUM_4 = 52,
        NUM_5 = 53,
        NUM_6 = 54,
        NUM_7 = 55,
        NUM_8 = 56,
        NUM_9 = 57,
        SEMICOLON = 59,
        EQUAL = 61,
        A = 65,
        B = 66,
        C = 67,
        D = 68,
        E = 69,
        F = 70,
        G = 71,
        H = 72,
        I = 73,
        J = 74,
        K = 75,
        L = 76,
        M = 77,
        N = 78,
        O = 79,
        P = 80,
        Q = 81,
        R = 82,
        S = 83,
        T = 84,
        U = 85,
        V = 86,
        W = 87,
        X = 88,
        Y = 89,
        Z = 90,
        LEFT_BRACKET = 91,
        BACKSLASH = 92,
        RIGHT_BRACKET = 93,
        GRAVE_ACCENT = 96,

        // Function keys
        ESCAPE = 256,
        ENTER = 257,
        TAB = 258,
        BACKSPACE = 259,
        INSERT = 260,
        DELETE = 261,
        RIGHT = 262,
        LEFT = 263,
        DOWN = 264,
        UP = 265,
        PAGE_UP = 266,
        PAGE_DOWN = 267,
        HOME = 268,
        END = 269,
        CAPS_LOCK = 280,
        SCROLL_LOCK = 281,
        NUM_LOCK = 282,
        PRINT_SCREEN = 283,
        PAUSE = 284,
        F1 = 290,
        F2 = 291,
        F3 = 292,
        F4 = 293,
        F5 = 294,
        F6 = 295,
        F7 = 296,
        F8 = 297,
        F9 = 298,
        F10 = 299,
        F11 = 300,
        F12 = 301,
        F13 = 302,
        F14 = 303,
        F15 = 304,
        F16 = 305,
        F17 = 306,
        F18 = 307,
        F19 = 308,
        F20 = 309,
        F21 = 310,
        F22 = 311,
        F23 = 312,
        F24 = 313,
        F25 = 314,

        // Keypad
        KP_0 = 320,
        KP_1 = 321,
        KP_2 = 322,
        KP_3 = 323,
        KP_4 = 324,
        KP_5 = 325,
        KP_6 = 326,
        KP_7 = 327,
        KP_8 = 328,
        KP_9 = 329,
        KP_DECIMAL = 330,
        KP_DIVIDE = 331,
        KP_MULTIPLY = 332,
        KP_SUBTRACT = 333,
        KP_ADD = 334,
        KP_ENTER = 335,
        KP_EQUAL = 336,

        // Modifiers
        LEFT_SHIFT = 340,
        LEFT_CONTROL = 341,
        LEFT_ALT = 342,
        LEFT_SUPER = 343,
        RIGHT_SHIFT = 344,
        RIGHT_CONTROL = 345,
        RIGHT_ALT = 346,
        RIGHT_SUPER = 347,
        MENU = 348,

        // Special
        UNKNOWN = -1
    };

    /**
     * @enum MouseButton
     * @brief Mouse button codes (GLFW-compatible)
     */
    enum class MouseButton {
        LEFT = 0,
        RIGHT = 1,
        MIDDLE = 2,
        BUTTON_4 = 3,
        BUTTON_5 = 4,
        BUTTON_6 = 5,
        BUTTON_7 = 6,
        BUTTON_8 = 7
    };

    /**
     * @enum InputAction
     * @brief Input action types
     */
    enum class InputAction {
        PRESS = 1,
        RELEASE = 0,
        REPEAT = 2
    };

    /**
     * @enum InputModifier
     * @brief Input modifier flags
     */
    enum class InputModifier {
        NONE = 0,
        SHIFT = 1,
        CONTROL = 2,
        ALT = 4,
        SUPER = 8,
        CAPS_LOCK = 16,
        NUM_LOCK = 32
    };

    /**
     * @struct InputEvent
     * @brief Represents an input event
     */
    struct InputEvent {
        enum class Type {
            KEY,
            MOUSE_BUTTON,
            MOUSE_MOVE,
            MOUSE_SCROLL,
            WINDOW_RESIZE,
            WINDOW_FOCUS,
            WINDOW_CLOSE
        };

        Type type;
        union {
            struct {
                KeyCode key;
                int scancode;
                InputAction action;
                int mods;
            } key;

            struct {
                MouseButton button;
                InputAction action;
                int mods;
            } mouseButton;

            struct {
                double x, y;
                double deltaX, deltaY;
            } mouseMove;

            struct {
                double xoffset, yoffset;
            } mouseScroll;

            struct {
                int width, height;
            } windowResize;

            struct {
                bool focused;
            } windowFocus;
        };

        double timestamp;

        InputEvent() : type(Type::KEY), timestamp(0.0) {}
    };

    /**
     * @struct InputState
     * @brief Current state of all input devices
     */
    struct InputState {
        // Keyboard state
        std::unordered_set<KeyCode> pressedKeys;
        std::unordered_map<KeyCode, double> keyPressTime;

        // Mouse state
        std::unordered_set<MouseButton> pressedButtons;
        Vec2 mousePosition;
        Vec2 mouseDelta;
        Vec2 scrollDelta;
        bool mouseLocked;

        // Window state
        bool windowFocused;
        Vec2 windowSize;

        // Timing
        double currentTime;
        double deltaTime;
    };

    /**
     * @enum GameAction
     * @brief High-level game actions mapped from input
     */
    enum class GameAction {
        MOVE_FORWARD,
        MOVE_BACKWARD,
        MOVE_LEFT,
        MOVE_RIGHT,
        JUMP,
        SNEAK,
        SPRINT,
        INTERACT,
        ATTACK,
        USE_ITEM,
        DROP_ITEM,
        INVENTORY,
        PAUSE,
        DEBUG,
        TOGGLE_WIREFRAME,
        TOGGLE_FULLSCREEN,
        QUICK_SAVE,
        QUICK_LOAD,
        COMMAND,
        CHAT
    };

    /**
     * @struct InputBinding
     * @brief Maps input to game actions
     */
    struct InputBinding {
        GameAction action;
        KeyCode key;
        MouseButton mouseButton;
        bool isMouse;
        int modifiers; // InputModifier flags

        InputBinding() : action(GameAction::MOVE_FORWARD), key(KeyCode::UNKNOWN),
                        mouseButton(MouseButton::LEFT), isMouse(false), modifiers(0) {}
    };

    /**
     * @typedef InputCallback
     * @brief Callback function for input events
     */
    using InputCallback = std::function<void(const InputEvent&)>;

    /**
     * @typedef ActionCallback
     * @brief Callback function for game actions
     */
    using ActionCallback = std::function<void(GameAction, bool)>;

    /**
     * @class InputManager
     * @brief Manages all input handling and processing
     */
    class InputManager {
    public:
        /**
         * @brief Constructor
         * @param window Window to receive input from
         * @param config Configuration system
         */
        InputManager(std::shared_ptr<Window> window, std::shared_ptr<Config> config);

        /**
         * @brief Destructor
         */
        ~InputManager();

        /**
         * @brief Initialize the input manager
         * @return true if successful
         */
        bool Initialize();

        /**
         * @brief Shutdown the input manager
         */
        void Shutdown();

        /**
         * @brief Update input state
         * @param deltaTime Time since last update
         */
        void Update(double deltaTime);

        /**
         * @brief Process pending input events
         */
        void ProcessEvents();

        /**
         * @brief Register input event callback
         * @param callback Callback function
         * @return Callback ID for removal
         */
        int RegisterInputCallback(InputCallback callback);

        /**
         * @brief Register game action callback
         * @param callback Callback function
         * @return Callback ID for removal
         */
        int RegisterActionCallback(ActionCallback callback);

        /**
         * @brief Unregister callback
         * @param callbackId Callback ID
         */
        void UnregisterCallback(int callbackId);

        /**
         * @brief Bind input to game action
         * @param binding Input binding
         */
        void BindAction(const InputBinding& binding);

        /**
         * @brief Unbind game action
         * @param action Action to unbind
         */
        void UnbindAction(GameAction action);

        /**
         * @brief Check if key is pressed
         * @param key Key code
         * @return true if pressed
         */
        bool IsKeyPressed(KeyCode key) const;

        /**
         * @brief Check if mouse button is pressed
         * @param button Mouse button
         * @return true if pressed
         */
        bool IsMouseButtonPressed(MouseButton button) const;

        /**
         * @brief Get current input state
         * @return Input state
         */
        const InputState& GetInputState() const { return m_currentState; }

        /**
         * @brief Get mouse position
         * @return Mouse position
         */
        Vec2 GetMousePosition() const { return m_currentState.mousePosition; }

        /**
         * @brief Get mouse delta (movement)
         * @return Mouse delta
         */
        Vec2 GetMouseDelta() const { return m_currentState.mouseDelta; }

        /**
         * @brief Enable/disable mouse lock (for camera control)
         * @param locked Enable mouse lock
         */
        void SetMouseLocked(bool locked);

        /**
         * @brief Check if mouse is locked
         * @return true if locked
         */
        bool IsMouseLocked() const { return m_currentState.mouseLocked; }

        /**
         * @brief Load input bindings from config
         */
        void LoadBindingsFromConfig();

        /**
         * @brief Save input bindings to config
         */
        void SaveBindingsToConfig();

        /**
         * @brief Set camera for input processing
         * @param camera Camera to control
         */
        void SetCamera(std::shared_ptr<Camera> camera);

        /**
         * @brief Set player for input processing
         * @param player Player to control
         */
        void SetPlayer(std::shared_ptr<Player> player);

        /**
         * @brief Reset input state
         */
        void Reset();

    private:
        std::shared_ptr<Window> m_window;
        std::shared_ptr<Config> m_config;
        std::shared_ptr<Camera> m_camera;
        std::shared_ptr<Player> m_player;

        InputState m_currentState;
        InputState m_previousState;

        // Event queue
        std::queue<InputEvent> m_eventQueue;
        mutable std::mutex m_eventMutex;

        // Callbacks
        std::unordered_map<int, InputCallback> m_inputCallbacks;
        std::unordered_map<int, ActionCallback> m_actionCallbacks;
        int m_nextCallbackId;

        // Action bindings
        std::unordered_map<KeyCode, GameAction> m_keyBindings;
        std::unordered_map<MouseButton, GameAction> m_mouseBindings;
        std::unordered_map<GameAction, InputBinding> m_actionBindings;

        // Internal state
        bool m_initialized;
        double m_lastUpdateTime;
        Vec2 m_lastMousePosition;

        /**
         * @brief Process key event
         * @param key Key code
         * @param scancode Scan code
         * @param action Key action
         * @param mods Modifiers
         */
        void ProcessKeyEvent(KeyCode key, int scancode, InputAction action, int mods);

        /**
         * @brief Process mouse button event
         * @param button Mouse button
         * @param action Button action
         * @param mods Modifiers
         */
        void ProcessMouseButtonEvent(MouseButton button, InputAction action, int mods);

        /**
         * @brief Process mouse move event
         * @param xpos Mouse X position
         * @param ypos Mouse Y position
         */
        void ProcessMouseMoveEvent(double xpos, double ypos);

        /**
         * @brief Process mouse scroll event
         * @param xoffset Scroll X offset
         * @param yoffset Scroll Y offset
         */
        void ProcessMouseScrollEvent(double xoffset, double yoffset);

        /**
         * @brief Process window resize event
         * @param width New width
         * @param height New height
         */
        void ProcessWindowResizeEvent(int width, int height);

        /**
         * @brief Update input state
         */
        void UpdateInputState();

        /**
         * @brief Trigger game action
         * @param action Game action
         * @param pressed Action state
         */
        void TriggerAction(GameAction action, bool pressed);

        /**
         * @brief Setup default key bindings
         */
        void SetupDefaultBindings();

        /**
         * @brief Convert GLFW key code to KeyCode
         * @param glfwKey GLFW key code
         * @return KeyCode
         */
        static KeyCode GLFWKeyToKeyCode(int glfwKey);

        /**
         * @brief Convert GLFW mouse button to MouseButton
         * @param glfwButton GLFW mouse button
         * @return MouseButton
         */
        static MouseButton GLFWMouseButtonToMouseButton(int glfwButton);

        /**
         * @brief Convert GLFW action to InputAction
         * @param glfwAction GLFW action
         * @return InputAction
         */
        static InputAction GLFWActionToInputAction(int glfwAction);
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_INPUT_INPUT_MANAGER_HPP
