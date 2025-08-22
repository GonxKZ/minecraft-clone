/**
 * @file PlayerInput.hpp
 * @brief VoxelCraft Player Input System
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines the PlayerInput class that handles all player input
 * including keyboard, mouse, gamepad, and touch controls with advanced
 * input processing, deadzones, smoothing, and gesture recognition.
 */

#ifndef VOXELCRAFT_PLAYER_PLAYER_INPUT_HPP
#define VOXELCRAFT_PLAYER_PLAYER_INPUT_HPP

#include <memory>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <chrono>

#include "../core/Config.hpp"

namespace VoxelCraft {

    // Forward declarations
    class Player;

    /**
     * @enum InputDevice
     * @brief Input device types
     */
    enum class InputDevice {
        Keyboard,               ///< Keyboard input
        Mouse,                  ///< Mouse input
        Gamepad,                ///< Gamepad/controller input
        Touch,                  ///< Touch input
        Virtual                 ///< Virtual/programmatic input
    };

    /**
     * @enum InputAction
     * @brief Input action types
     */
    enum class InputAction {
        // Movement actions
        MoveForward,            ///< Move forward
        MoveBackward,           ///< Move backward
        MoveLeft,               ///< Move left
        MoveRight,              ///< Move right
        Jump,                   ///< Jump
        Sneak,                  ///< Sneak/crouch
        Sprint,                 ///< Sprint
        Fly,                    ///< Toggle fly mode
        SwimUp,                 ///< Swim up
        SwimDown,               ///< Swim down
        Climb,                  ///< Climb

        // Camera actions
        LookUp,                 ///< Look up
        LookDown,               ///< Look down
        LookLeft,               ///< Look left
        LookRight,              ///< Look right
        ZoomIn,                 ///< Zoom in
        ZoomOut,                ///< Zoom out
        CenterView,             ///< Center camera view

        // Combat actions
        Attack,                 ///< Primary attack
        Block,                  ///< Block/defend
        UseItem,                ///< Use held item
        Reload,                 ///< Reload weapon
        SwitchWeapon,           ///< Switch to next weapon

        // Interaction actions
        Interact,               ///< Interact with object
        PickUp,                 ///< Pick up item
        Drop,                   ///< Drop item
        OpenInventory,          ///< Open inventory
        OpenMap,                ///< Open map
        OpenMenu,               ///< Open menu
        QuickSave,              ///< Quick save
        QuickLoad,              ///< Quick load

        // Communication actions
        Chat,                   ///< Open chat
        VoiceChat,              ///< Voice chat
        TeamChat,               ///< Team chat
        Command,                ///< Enter command

        // System actions
        Pause,                  ///< Pause game
        Screenshot,             ///< Take screenshot
        Debug,                  ///< Toggle debug mode
        Console,                ///< Open console
        Fullscreen,             ///< Toggle fullscreen

        // Custom actions
        Custom1,                ///< Custom action 1
        Custom2,                ///< Custom action 2
        Custom3,                ///< Custom action 3
        Custom4                 ///< Custom action 4
    };

    /**
     * @enum InputState
     * @brief Input state for buttons and keys
     */
    enum class InputState {
        Released,               ///< Input is not pressed
        Pressed,                ///< Input was just pressed
        Held,                   ///< Input is being held
        ReleasedJustNow         ///< Input was just released
    };

    /**
     * @enum InputAxis
     * @brief Input axis types
     */
    enum class InputAxis {
        MoveX,                  ///< Movement X axis (-1.0 to 1.0)
        MoveY,                  ///< Movement Y axis (-1.0 to 1.0)
        LookX,                  ///< Look X axis (-1.0 to 1.0)
        LookY,                  ///< Look Y axis (-1.0 to 1.0)
        TriggerLeft,            ///< Left trigger (0.0 to 1.0)
        TriggerRight,           ///< Right trigger (0.0 to 1.0)
        ScrollWheel,            ///< Mouse scroll wheel
        CustomAxis1,            ///< Custom axis 1
        CustomAxis2,            ///< Custom axis 2
        CustomAxis3,            ///< Custom axis 3
        CustomAxis4             ///< Custom axis 4
    };

    /**
     * @struct InputBinding
     * @brief Input binding configuration
     */
    struct InputBinding {
        InputAction action;                     ///< Action to perform
        InputDevice device;                     ///< Input device
        int keyCode;                           ///< Key/button code
        float scale;                           ///< Input scale factor
        float deadzone;                        ///< Input deadzone
        bool invert;                           ///< Invert input axis
        std::string description;               ///< Binding description
        bool isAxis;                           ///< Is axis binding
        InputAxis axis;                        ///< Axis type (if axis binding)
    };

    /**
     * @struct InputConfig
     * @brief Input system configuration
     */
    struct InputConfig {
        // General settings
        float mouseSensitivity;                 ///< Mouse sensitivity (0.0 - 1.0)
        float gamepadSensitivity;               ///< Gamepad sensitivity (0.0 - 1.0)
        bool invertMouseY;                      ///< Invert mouse Y axis
        bool invertGamepadY;                    ///< Invert gamepad Y axis
        bool enableVibration;                   ///< Enable gamepad vibration
        float vibrationStrength;                ///< Vibration strength (0.0 - 1.0)

        // Deadzone settings
        float mouseDeadzone;                    ///< Mouse deadzone
        float gamepadDeadzone;                  ///< Gamepad deadzone
        float triggerDeadzone;                  ///< Trigger deadzone

        // Smoothing settings
        float inputSmoothing;                   ///< Input smoothing factor (0.0 - 1.0)
        float axisSmoothing;                    ///< Axis smoothing factor (0.0 - 1.0)
        int smoothingSamples;                   ///< Number of smoothing samples

        // Advanced settings
        bool enableKeyRepeat;                   ///< Enable key repeat
        float keyRepeatDelay;                   ///< Key repeat delay (seconds)
        float keyRepeatRate;                    ///< Key repeat rate (keys per second)
        bool enableGestureRecognition;          ///< Enable gesture recognition
        bool enableDoubleTap;                   ///< Enable double-tap detection

        // Touch settings (for mobile)
        bool enableTouchControls;               ///< Enable touch controls
        float touchDeadzone;                    ///< Touch deadzone
        float touchSensitivity;                 ///< Touch sensitivity
        bool enableSwipeGestures;               ///< Enable swipe gestures
        bool enablePinchGestures;               ///< Enable pinch gestures

        // Accessibility settings
        bool enableLargeCursor;                 ///< Large cursor for accessibility
        bool enableStickyKeys;                  ///< Sticky keys for accessibility
        bool enableHighContrast;                ///< High contrast mode
        float buttonHoldTime;                   ///< Time to hold button for action

        // Debug settings
        bool enableInputLogging;                ///< Enable input logging
        bool showInputDebug;                    ///< Show input debug overlay
        bool enableInputRecording;              ///< Enable input recording
        std::string recordingFile;              ///< Input recording file path
    };

    /**
     * @struct InputStateData
     * @brief Current input state data
     */
    struct InputStateData {
        // Button/key states
        std::unordered_map<int, InputState> buttonStates;
        std::unordered_map<InputAction, InputState> actionStates;

        // Axis values
        std::unordered_map<InputAxis, float> axisValues;
        std::unordered_map<InputAxis, float> rawAxisValues;
        std::unordered_map<InputAxis, float> smoothedAxisValues;

        // Mouse state
        glm::vec2 mousePosition;                ///< Current mouse position
        glm::vec2 mouseDelta;                   ///< Mouse movement delta
        glm::vec2 mouseWheelDelta;              ///< Mouse wheel delta

        // Touch state
        std::vector<glm::vec2> touchPositions;  ///< Touch positions
        std::vector<glm::vec2> touchDeltas;     ///< Touch movement deltas
        int touchCount;                         ///< Number of active touches

        // Timing
        double currentTime;                     ///< Current time
        double deltaTime;                       ///< Time delta
        double lastInputTime;                   ///< Last input time

        // Flags
        bool hasFocus;                          ///< Window has focus
        bool isMinimized;                       ///< Window is minimized
        bool isFullscreen;                      ///< Window is fullscreen
    };

    /**
     * @struct InputMetrics
     * @brief Performance and usage metrics for input system
     */
    struct InputMetrics {
        // Performance metrics
        uint64_t updateCount;                   ///< Number of updates performed
        double totalUpdateTime;                 ///< Total update time (ms)
        double averageUpdateTime;               ///< Average update time (ms)
        double maxUpdateTime;                   ///< Maximum update time (ms)

        // Input event metrics
        uint64_t keyboardEvents;                ///< Keyboard events processed
        uint64_t mouseEvents;                   ///< Mouse events processed
        uint64_t gamepadEvents;                 ///< Gamepad events processed
        uint64_t touchEvents;                   ///< Touch events processed
        uint64_t totalInputEvents;              ///< Total input events

        // Action metrics
        uint64_t actionsTriggered;              ///< Actions triggered
        uint64_t actionsCompleted;              ///< Actions completed
        uint64_t actionsCancelled;              ///< Actions cancelled
        std::unordered_map<InputAction, uint64_t> actionCounts;

        // Axis metrics
        float averageAxisValue;                 ///< Average axis value
        float maxAxisValue;                     ///< Maximum axis value recorded
        float axisUpdateRate;                   ///< Axis update rate (Hz)

        // Gesture metrics
        uint64_t gesturesRecognized;            ///< Gestures recognized
        uint64_t gestureErrors;                 ///< Gesture recognition errors
        uint64_t doubleTapsDetected;            ///< Double-taps detected
        uint64_t swipeGestures;                 ///< Swipe gestures detected

        // Error metrics
        uint64_t inputErrors;                   ///< Input processing errors
        uint64_t bindingErrors;                 ///< Binding configuration errors
        uint64_t deviceErrors;                  ///< Device connection errors
        uint64_t overflowEvents;                ///< Input buffer overflow events

        // Device metrics
        bool keyboardConnected;                 ///< Keyboard is connected
        bool mouseConnected;                    ///< Mouse is connected
        int gamepadCount;                       ///< Number of connected gamepads
        bool touchSupported;                    ///< Touch input is supported
        float batteryLevel;                     ///< Gamepad battery level (0.0 - 1.0)
    };

    /**
     * @struct GestureData
     * @brief Gesture recognition data
     */
    struct GestureData {
        std::string gestureType;                ///< Type of gesture
        glm::vec2 startPosition;                ///< Gesture start position
        glm::vec2 endPosition;                  ///< Gesture end position
        glm::vec2 currentPosition;              ///< Current gesture position
        double startTime;                       ///< Gesture start time
        double duration;                        ///< Gesture duration
        float strength;                         ///< Gesture strength (0.0 - 1.0)
        bool isComplete;                        ///< Gesture is complete
        bool isValid;                           ///< Gesture is valid
    };

    /**
     * @class PlayerInput
     * @brief Advanced player input system with multiple device support
     *
     * The PlayerInput class provides a comprehensive input system for VoxelCraft
     * supporting keyboard, mouse, gamepad, and touch input with advanced features
     * like gesture recognition, input smoothing, deadzone handling, and accessibility
     * features.
     *
     * Key Features:
     * - Multi-device input support (keyboard, mouse, gamepad, touch)
     * - Configurable input bindings and key mappings
     * - Input smoothing and deadzone handling
     * - Gesture recognition for touch and mouse
     * - Double-tap and hold detection
     * - Accessibility features (sticky keys, large cursor)
     * - Input recording and playback
     * - Vibration feedback for gamepads
     * - Debug visualization and profiling
     *
     * Input Processing Pipeline:
     * 1. Raw input capture from devices
     * 2. Input filtering and deadzone application
     * 3. Gesture recognition and pattern matching
     * 4. Input smoothing and interpolation
     * 5. Action binding and state updates
     * 6. Event dispatching and callback handling
     */
    class PlayerInput {
    public:
        /**
         * @brief Constructor
         * @param config Input configuration
         */
        explicit PlayerInput(const InputConfig& config);

        /**
         * @brief Destructor
         */
        ~PlayerInput();

        /**
         * @brief Deleted copy constructor
         */
        PlayerInput(const PlayerInput&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        PlayerInput& operator=(const PlayerInput&) = delete;

        // Input system lifecycle

        /**
         * @brief Initialize input system
         * @return true if successful, false otherwise
         */
        bool Initialize();

        /**
         * @brief Shutdown input system
         */
        void Shutdown();

        /**
         * @brief Update input system
         * @param deltaTime Time elapsed since last update
         */
        void Update(double deltaTime);

        /**
         * @brief Process input events
         */
        void ProcessEvents();

        /**
         * @brief Clear all input states
         */
        void ClearStates();

        // Input configuration

        /**
         * @brief Get input configuration
         * @return Current configuration
         */
        const InputConfig& GetConfig() const { return m_config; }

        /**
         * @brief Set input configuration
         * @param config New configuration
         */
        void SetConfig(const InputConfig& config);

        /**
         * @brief Get input state data
         * @return Current input state
         */
        const InputStateData& GetState() const { return m_state; }

        // Input bindings

        /**
         * @brief Add input binding
         * @param binding Input binding to add
         * @return true if added, false otherwise
         */
        bool AddBinding(const InputBinding& binding);

        /**
         * @brief Remove input binding
         * @param action Action to remove binding for
         * @param device Device type
         * @param keyCode Key/button code
         * @return true if removed, false otherwise
         */
        bool RemoveBinding(InputAction action, InputDevice device, int keyCode);

        /**
         * @brief Get binding for action and device
         * @param action Input action
         * @param device Input device
         * @return Binding or nullptr if not found
         */
        const InputBinding* GetBinding(InputAction action, InputDevice device) const;

        /**
         * @brief Get all bindings for action
         * @param action Input action
         * @return Vector of bindings
         */
        std::vector<InputBinding> GetBindingsForAction(InputAction action) const;

        /**
         * @brief Get all bindings
         * @return Vector of all bindings
         */
        std::vector<InputBinding> GetAllBindings() const;

        /**
         * @brief Load default bindings
         */
        void LoadDefaultBindings();

        /**
         * @brief Save bindings to file
         * @param filename File path
         * @return true if saved, false otherwise
         */
        bool SaveBindings(const std::string& filename) const;

        /**
         * @brief Load bindings from file
         * @param filename File path
         * @return true if loaded, false otherwise
         */
        bool LoadBindings(const std::string& filename);

        // Input state queries

        /**
         * @brief Check if action is pressed
         * @param action Input action
         * @return true if pressed, false otherwise
         */
        bool IsPressed(InputAction action) const;

        /**
         * @brief Check if action is held
         * @param action Input action
         * @return true if held, false otherwise
         */
        bool IsHeld(InputAction action) const;

        /**
         * @brief Check if action was just pressed
         * @param action Input action
         * @return true if just pressed, false otherwise
         */
        bool WasJustPressed(InputAction action) const;

        /**
         * @brief Check if action was just released
         * @param action Input action
         * @return true if just released, false otherwise
         */
        bool WasJustReleased(InputAction action) const;

        /**
         * @brief Get axis value
         * @param axis Input axis
         * @return Axis value (-1.0 to 1.0)
         */
        float GetAxisValue(InputAxis axis) const;

        /**
         * @brief Get raw axis value (without smoothing)
         * @param axis Input axis
         * @return Raw axis value (-1.0 to 1.0)
         */
        float GetRawAxisValue(InputAxis axis) const;

        /**
         * @brief Get smoothed axis value
         * @param axis Input axis
         * @return Smoothed axis value (-1.0 to 1.0)
         */
        float GetSmoothedAxisValue(InputAxis axis) const;

        // Mouse input

        /**
         * @brief Get mouse position
         * @return Mouse position in screen coordinates
         */
        const glm::vec2& GetMousePosition() const { return m_state.mousePosition; }

        /**
         * @brief Get mouse delta
         * @return Mouse movement delta
         */
        const glm::vec2& GetMouseDelta() const { return m_state.mouseDelta; }

        /**
         * @brief Get mouse wheel delta
         * @return Mouse wheel delta
         */
        const glm::vec2& GetMouseWheelDelta() const { return m_state.mouseWheelDelta; }

        /**
         * @brief Set mouse position
         * @param position New mouse position
         */
        void SetMousePosition(const glm::vec2& position);

        /**
         * @brief Center mouse cursor
         */
        void CenterMouse();

        /**
         * @brief Show mouse cursor
         */
        void ShowMouseCursor();

        /**
         * @brief Hide mouse cursor
         */
        void HideMouseCursor();

        /**
         * @brief Check if mouse cursor is visible
         * @return true if visible, false otherwise
         */
        bool IsMouseCursorVisible() const { return m_mouseCursorVisible; }

        // Touch input

        /**
         * @brief Get touch count
         * @return Number of active touches
         */
        int GetTouchCount() const { return m_state.touchCount; }

        /**
         * @brief Get touch position
         * @param index Touch index
         * @return Touch position or zero vector if invalid index
         */
        glm::vec2 GetTouchPosition(int index) const;

        /**
         * @brief Get touch delta
         * @param index Touch index
         * @return Touch movement delta
         */
        glm::vec2 GetTouchDelta(int index) const;

        /**
         * @brief Check if touch is active
         * @param index Touch index
         * @return true if active, false otherwise
         */
        bool IsTouchActive(int index) const;

        // Gesture recognition

        /**
         * @brief Check if gesture is active
         * @param gestureType Gesture type
         * @return true if active, false otherwise
         */
        bool IsGestureActive(const std::string& gestureType) const;

        /**
         * @brief Get gesture data
         * @param gestureType Gesture type
         * @return Gesture data or empty optional
         */
        std::optional<GestureData> GetGestureData(const std::string& gestureType) const;

        /**
         * @brief Add gesture recognition callback
         * @param gestureType Gesture type
         * @param callback Callback function
         */
        void AddGestureCallback(const std::string& gestureType, std::function<void(const GestureData&)> callback);

        /**
         * @brief Remove gesture recognition callback
         * @param gestureType Gesture type
         */
        void RemoveGestureCallback(const std::string& gestureType);

        // Vibration and feedback

        /**
         * @brief Set gamepad vibration
         * @param leftMotor Left motor strength (0.0 - 1.0)
         * @param rightMotor Right motor strength (0.0 - 1.0)
         * @param duration Duration in seconds
         */
        void SetVibration(float leftMotor, float rightMotor, float duration);

        /**
         * @brief Stop gamepad vibration
         */
        void StopVibration();

        /**
         * @brief Check if gamepad is vibrating
         * @return true if vibrating, false otherwise
         */
        bool IsVibrating() const { return m_vibrationEndTime > m_state.currentTime; }

        // Input recording and playback

        /**
         * @brief Start input recording
         * @return true if started, false otherwise
         */
        bool StartRecording();

        /**
         * @brief Stop input recording
         * @return true if stopped, false otherwise
         */
        bool StopRecording();

        /**
         * @brief Start input playback
         * @return true if started, false otherwise
         */
        bool StartPlayback();

        /**
         * @brief Stop input playback
         * @return true if stopped, false otherwise
         */
        bool StopPlayback();

        /**
         * @brief Check if recording
         * @return true if recording, false otherwise
         */
        bool IsRecording() const { return m_isRecording; }

        /**
         * @brief Check if playing back
         * @return true if playing back, false otherwise
         */
        bool IsPlayingBack() const { return m_isPlayingBack; }

        /**
         * @brief Save recording to file
         * @param filename File path
         * @return true if saved, false otherwise
         */
        bool SaveRecording(const std::string& filename) const;

        /**
         * @brief Load recording from file
         * @param filename File path
         * @return true if loaded, false otherwise
         */
        bool LoadRecording(const std::string& filename);

        // Event system

        /**
         * @brief Add input event listener
         * @param eventType Event type
         * @param listener Listener function
         */
        void AddEventListener(const std::string& eventType, std::function<void(const std::string&)> listener);

        /**
         * @brief Remove input event listener
         * @param eventType Event type
         */
        void RemoveEventListener(const std::string& eventType);

        /**
         * @brief Trigger input event
         * @param eventType Event type
         * @param eventData Event data
         */
        void TriggerEvent(const std::string& eventType, const std::string& eventData = "");

        // Metrics and debugging

        /**
         * @brief Get input metrics
         * @return Performance metrics
         */
        const InputMetrics& GetMetrics() const { return m_metrics; }

        /**
         * @brief Reset metrics
         */
        void ResetMetrics();

        /**
         * @brief Get debug information
         * @return Debug information string
         */
        std::string GetDebugInfo() const;

        /**
         * @brief Validate input system
         * @return true if valid, false otherwise
         */
        bool Validate() const;

        /**
         * @brief Get input bindings as string
         * @return Bindings information
         */
        std::string GetBindingsInfo() const;

    private:
        /**
         * @brief Initialize input subsystems
         * @return true if successful, false otherwise
         */
        bool InitializeSubsystems();

        /**
         * @brief Process keyboard input
         * @param deltaTime Time elapsed
         */
        void ProcessKeyboardInput(double deltaTime);

        /**
         * @brief Process mouse input
         * @param deltaTime Time elapsed
         */
        void ProcessMouseInput(double deltaTime);

        /**
         * @brief Process gamepad input
         * @param deltaTime Time elapsed
         */
        void ProcessGamepadInput(double deltaTime);

        /**
         * @brief Process touch input
         * @param deltaTime Time elapsed
         */
        void ProcessTouchInput(double deltaTime);

        /**
         * @brief Update input actions from bindings
         */
        void UpdateActionsFromBindings();

        /**
         * @brief Apply input smoothing
         * @param deltaTime Time elapsed
         */
        void ApplyInputSmoothing(double deltaTime);

        /**
         * @brief Apply deadzones to axis values
         */
        void ApplyDeadzones();

        /**
         * @brief Detect gestures
         * @param deltaTime Time elapsed
         */
        void DetectGestures(double deltaTime);

        /**
         * @brief Detect double-tap inputs
         * @param deltaTime Time elapsed
         */
        void DetectDoubleTaps(double deltaTime);

        /**
         * @brief Update key repeat
         * @param deltaTime Time elapsed
         */
        void UpdateKeyRepeat(double deltaTime);

        /**
         * @brief Update input metrics
         * @param deltaTime Time elapsed
         */
        void UpdateMetrics(double deltaTime);

        /**
         * @brief Handle input errors
         * @param error Error message
         */
        void HandleInputError(const std::string& error);

        /**
         * @brief Process gesture completion
         * @param gesture Completed gesture
         */
        void ProcessGestureCompletion(const GestureData& gesture);

        // Input data
        InputConfig m_config;                              ///< Input configuration
        InputStateData m_state;                            ///< Current input state
        InputMetrics m_metrics;                            ///< Performance metrics

        // Bindings and mappings
        std::unordered_map<InputAction, std::vector<InputBinding>> m_actionBindings;
        std::unordered_map<int, std::vector<InputBinding>> m_keyBindings;
        mutable std::shared_mutex m_bindingsMutex;         ///< Bindings synchronization

        // Gesture recognition
        std::unordered_map<std::string, GestureData> m_activeGestures;
        std::unordered_map<std::string, std::vector<std::function<void(const GestureData&)>>> m_gestureCallbacks;
        mutable std::shared_mutex m_gesturesMutex;         ///< Gestures synchronization

        // Event system
        std::unordered_map<std::string, std::vector<std::function<void(const std::string&)>>> m_eventListeners;
        mutable std::shared_mutex m_eventsMutex;           ///< Events synchronization

        // Recording system
        std::vector<InputStateData> m_recordedInput;      ///< Recorded input data
        size_t m_playbackIndex;                           ///< Current playback index
        bool m_isRecording;                               ///< Recording active flag
        bool m_isPlayingBack;                             ///< Playback active flag

        // Vibration system
        double m_vibrationEndTime;                        ///< Vibration end time
        float m_leftMotorStrength;                        ///< Left motor strength
        float m_rightMotorStrength;                       ///< Right motor strength

        // State tracking
        std::unordered_map<int, double> m_keyPressTimes;  ///< Key press timestamps
        std::unordered_map<int, double> m_keyRepeatTimes; ///< Key repeat timestamps
        std::unordered_map<InputAction, double> m_actionPressTimes; ///< Action press timestamps
        std::unordered_map<InputAxis, std::vector<float>> m_axisHistory; ///< Axis value history

        // Mouse state
        bool m_mouseCursorVisible;                        ///< Mouse cursor visibility

        // Initialization flags
        bool m_isInitialized;                             ///< Input system initialized
        double m_lastUpdateTime;                          ///< Last update timestamp
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_PLAYER_PLAYER_INPUT_HPP
