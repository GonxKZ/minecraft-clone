/**
 * @file InputSystem.hpp
 * @brief VoxelCraft Advanced Input Management System Header
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_INPUT_INPUT_SYSTEM_HPP
#define VOXELCRAFT_INPUT_INPUT_SYSTEM_HPP

#include <memory>
#include <atomic>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <functional>
#include <queue>
#include <chrono>
#include <variant>
#include <any>

#include <glm/glm.hpp>

#include "../core/System.hpp"

namespace VoxelCraft {

    // Forward declarations
    class InputDevice;
    class InputContext;
    class GestureRecognizer;
    class ForceFeedbackController;
    class InputRecorder;
    class InputReplayer;
    class InputAnalytics;

    /**
     * @brief Input device types
     */
    enum class InputDeviceType {
        KEYBOARD,
        MOUSE,
        GAMEPAD,
        JOYSTICK,
        WHEEL,
        FLIGHT_STICK,
        DANCE_PAD,
        DRUM_KIT,
        GUITAR,
        MICROPHONE,
        CAMERA,
        MOTION_CONTROLLER,
        HAPTIC_DEVICE,
        TOUCH_SCREEN,
        TOUCH_PAD,
        PEN_TABLET,
        VR_HMD,
        VR_CONTROLLER,
        AR_GLASSES,
        BIO_SENSOR,
        CUSTOM_1,
        CUSTOM_2,
        MAX_TYPES
    };

    /**
     * @brief Input event types
     */
    enum class InputEventType {
        KEY_PRESSED,
        KEY_RELEASED,
        KEY_REPEATED,
        MOUSE_BUTTON_PRESSED,
        MOUSE_BUTTON_RELEASED,
        MOUSE_MOVED,
        MOUSE_SCROLLED,
        MOUSE_ENTERED,
        MOUSE_LEFT,
        GAMEPAD_CONNECTED,
        GAMEPAD_DISCONNECTED,
        GAMEPAD_BUTTON_PRESSED,
        GAMEPAD_BUTTON_RELEASED,
        GAMEPAD_AXIS_MOVED,
        GAMEPAD_TRIGGER_PRESSED,
        JOYSTICK_CONNECTED,
        JOYSTICK_DISCONNECTED,
        JOYSTICK_BUTTON_PRESSED,
        JOYSTICK_BUTTON_RELEASED,
        JOYSTICK_AXIS_MOVED,
        JOYSTICK_HAT_MOVED,
        TOUCH_STARTED,
        TOUCH_MOVED,
        TOUCH_ENDED,
        TOUCH_CANCELLED,
        GESTURE_RECOGNIZED,
        MOTION_DETECTED,
        VOICE_COMMAND,
        TEXT_INPUT,
        FILE_DROPPED,
        WINDOW_RESIZED,
        WINDOW_MOVED,
        WINDOW_FOCUS_GAINED,
        WINDOW_FOCUS_LOST,
        WINDOW_MINIMIZED,
        WINDOW_MAXIMIZED,
        WINDOW_RESTORED,
        WINDOW_CLOSED,
        CLIPBOARD_UPDATED,
        SYSTEM_COMMAND,
        CUSTOM_EVENT
    };

    /**
     * @brief Input action types
     */
    enum class InputActionType {
        BUTTON,         // Simple press/release
        AXIS,           // Continuous value (joysticks, triggers)
        VECTOR2,        // 2D vector (mouse, touch)
        VECTOR3,        // 3D vector (motion controllers)
        GESTURE,        // Gesture recognition
        VOICE,          // Voice command
        MOTION,         // Motion detection
        COMPOSITE       // Combination of multiple inputs
    };

    /**
     * @brief Gesture types
     */
    enum class GestureType {
        TAP,
        DOUBLE_TAP,
        LONG_PRESS,
        SWIPE_LEFT,
        SWIPE_RIGHT,
        SWIPE_UP,
        SWIPE_DOWN,
        PINCH_IN,
        PINCH_OUT,
        ROTATE,
        PAN,
        FLICK,
        CUSTOM_GESTURE
    };

    /**
     * @brief Input event data structures
     */
    struct KeyEvent {
        int key;
        int scancode;
        int mods;
        bool state;
    };

    struct MouseButtonEvent {
        int button;
        int mods;
        bool state;
        glm::vec2 position;
    };

    struct MouseMoveEvent {
        glm::vec2 position;
        glm::vec2 delta;
        glm::vec2 velocity;
    };

    struct MouseScrollEvent {
        glm::vec2 offset;
        int mods;
    };

    struct GamepadButtonEvent {
        int gamepadId;
        int button;
        bool state;
        float pressure;
    };

    struct GamepadAxisEvent {
        int gamepadId;
        int axis;
        float value;
        float delta;
    };

    struct TouchEvent {
        int finger;
        glm::vec2 position;
        glm::vec2 delta;
        float pressure;
        bool state;
    };

    struct GestureEvent {
        GestureType type;
        glm::vec2 startPosition;
        glm::vec2 endPosition;
        glm::vec2 velocity;
        float scale;
        float rotation;
        uint64_t duration;
    };

    struct VoiceEvent {
        std::string command;
        float confidence;
        std::vector<std::string> alternatives;
    };

    /**
     * @brief Generic input event
     */
    struct InputEvent {
        InputEventType type;
        uint64_t timestamp;
        uint32_t deviceId;
        std::variant<
            KeyEvent,
            MouseButtonEvent,
            MouseMoveEvent,
            MouseScrollEvent,
            GamepadButtonEvent,
            GamepadAxisEvent,
            TouchEvent,
            GestureEvent,
            VoiceEvent,
            std::any
        > data;
    };

    /**
     * @brief Input action value
     */
    struct InputActionValue {
        InputActionType type;
        std::variant<
            bool,           // Button
            float,          // Axis
            glm::vec2,      // Vector2
            glm::vec3,      // Vector3
            GestureEvent,   // Gesture
            VoiceEvent,     // Voice
            std::any        // Motion/Composite
        > value;

        float GetFloat() const {
            if (std::holds_alternative<bool>(value)) return std::get<bool>(value) ? 1.0f : 0.0f;
            if (std::holds_alternative<float>(value)) return std::get<float>(value);
            if (std::holds_alternative<glm::vec2>(value)) return glm::length(std::get<glm::vec2>(value));
            if (std::holds_alternative<glm::vec3>(value)) return glm::length(std::get<glm::vec3>(value));
            return 0.0f;
        }
    };

    /**
     * @brief Input action
     */
    struct InputAction {
        std::string name;
        InputActionType type;
        InputActionValue value;
        InputActionValue previousValue;
        uint64_t lastChangedTime = 0;
        bool enabled = true;
        float sensitivity = 1.0f;

        bool IsPressed() const { return value.GetFloat() > 0.5f; }
        bool WasPressed() const { return previousValue.GetFloat() <= 0.5f && value.GetFloat() > 0.5f; }
        bool WasReleased() const { return previousValue.GetFloat() > 0.5f && value.GetFloat() <= 0.5f; }
        bool IsHeld() const { return value.GetFloat() > 0.5f; }
    };

    /**
     * @brief Input context for modal input handling
     */
    class InputContext {
    public:
        InputContext(const std::string& name);
        ~InputContext() = default;

        const std::string& GetName() const { return m_name; }
        bool IsEnabled() const { return m_enabled; }
        void SetEnabled(bool enabled) { m_enabled = enabled; }

        void AddAction(std::shared_ptr<InputAction> action);
        void RemoveAction(const std::string& name);
        std::shared_ptr<InputAction> GetAction(const std::string& name) const;

        void Update(float deltaTime);
        bool ProcessEvent(const InputEvent& event);

        void ClearActions();
        size_t GetActionCount() const { return m_actions.size(); }

    private:
        std::string m_name;
        bool m_enabled;
        std::unordered_map<std::string, std::shared_ptr<InputAction>> m_actions;
        mutable std::shared_mutex m_actionsMutex;
    };

    /**
     * @brief Input device interface
     */
    class InputDevice {
    public:
        InputDevice(uint32_t id, InputDeviceType type, const std::string& name);
        virtual ~InputDevice() = default;

        uint32_t GetId() const { return m_id; }
        InputDeviceType GetType() const { return m_type; }
        const std::string& GetName() const { return m_name; }

        bool IsConnected() const { return m_connected; }
        virtual bool Connect() = 0;
        virtual void Disconnect() = 0;

        virtual void Update(float deltaTime) = 0;
        virtual bool ProcessEvent(const InputEvent& event) = 0;
        virtual std::vector<InputEvent> PollEvents() = 0;
        virtual bool HasCapability(const std::string& capability) const = 0;

    protected:
        uint32_t m_id;
        InputDeviceType m_type;
        std::string m_name;
        std::atomic<bool> m_connected;
    };

    /**
     * @brief Main input system
     */
    class InputSystem : public System {
    public:
        static InputSystem& GetInstance();

        InputSystem();
        ~InputSystem() override;

        // System interface
        bool Initialize() override;
        void Update(float deltaTime) override;
        void Shutdown() override;
        std::string GetName() const override { return "InputSystem"; }
        System::Type GetType() const override { return System::Type::INPUT; }

        // Device management
        uint32_t RegisterDevice(std::shared_ptr<InputDevice> device);
        bool UnregisterDevice(uint32_t deviceId);
        std::shared_ptr<InputDevice> GetDevice(uint32_t deviceId) const;
        size_t GetDeviceCount() const;

        // Context management
        std::shared_ptr<InputContext> CreateContext(const std::string& name);
        bool DestroyContext(const std::string& name);
        std::shared_ptr<InputContext> GetContext(const std::string& name) const;
        bool ActivateContext(const std::string& name);
        bool DeactivateContext(const std::string& name);

        // Action management
        std::shared_ptr<InputAction> CreateAction(const std::string& name, InputActionType type);
        bool DestroyAction(const std::string& name);
        std::shared_ptr<InputAction> GetAction(const std::string& name) const;

        // Event processing
        void ProcessEvent(const InputEvent& event);
        void QueueEvent(const InputEvent& event);
        std::vector<InputEvent> PollEvents();

        // Input state queries
        bool IsActionPressed(const std::string& action) const;
        bool IsActionHeld(const std::string& action) const;
        bool WasActionPressed(const std::string& action) const;
        bool WasActionReleased(const std::string& action) const;
        InputActionValue GetActionValue(const std::string& action) const;

        // Configuration
        void SetInputEnabled(bool enabled) { m_inputEnabled = enabled; }
        bool IsInputEnabled() const { return m_inputEnabled; }

        // Utility functions
        std::string GetInputInfo() const;
        void ClearAllActions();

    private:
        // Core components
        mutable std::shared_mutex m_devicesMutex;
        std::unordered_map<uint32_t, std::shared_ptr<InputDevice>> m_devices;

        mutable std::shared_mutex m_contextsMutex;
        std::unordered_map<std::string, std::shared_ptr<InputContext>> m_contexts;
        std::vector<std::string> m_activeContexts;

        mutable std::shared_mutex m_actionsMutex;
        std::unordered_map<std::string, std::shared_ptr<InputAction>> m_actions;

        // Event processing
        std::queue<InputEvent> m_eventQueue;
        mutable std::mutex m_eventQueueMutex;

        // Configuration
        std::atomic<bool> m_inputEnabled;

        // ID generation
        static std::atomic<uint32_t> s_nextDeviceId;

        // Internal methods
        void UpdateDevices(float deltaTime);
        void UpdateContexts(float deltaTime);
        void UpdateActions(float deltaTime);
        void ProcessQueuedEvents();

        void SortActiveContextsByPriority();
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_INPUT_INPUT_SYSTEM_HPP