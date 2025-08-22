/**
 * @file InputManager.cpp
 * @brief VoxelCraft Input System - Input handling implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#include "InputManager.hpp"
#include <GLFW/glfw3.h>
#include <iostream>
#include <algorithm>

// Simple vector structures
struct Vec2 {
    float x, y;
    Vec2(float x = 0, float y = 0) : x(x), y(y) {}
};

struct Vec3 {
    float x, y, z;
    Vec3(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {}
};

namespace VoxelCraft {

InputManager::InputManager(std::shared_ptr<Window> window, std::shared_ptr<Config> config)
    : m_window(window)
    , m_config(config)
    , m_initialized(false)
    , m_nextCallbackId(1)
    , m_lastUpdateTime(0.0)
{
    // Initialize states
    m_currentState.mousePosition = Vec2(0, 0);
    m_currentState.mouseDelta = Vec2(0, 0);
    m_currentState.scrollDelta = Vec2(0, 0);
    m_currentState.mouseLocked = false;
    m_currentState.windowFocused = true;
    m_currentState.windowSize = Vec2(1280, 720);
    m_currentState.currentTime = 0.0;
    m_currentState.deltaTime = 0.0;

    m_lastMousePosition = Vec2(0, 0);
}

InputManager::~InputManager() {
    Shutdown();
}

bool InputManager::Initialize() {
    if (m_initialized) {
        return true;
    }

    if (!m_window) {
        std::cerr << "No window provided to input manager" << std::endl;
        return false;
    }

    // Setup default bindings
    SetupDefaultBindings();

    // Load bindings from config
    LoadBindingsFromConfig();

    // Set initial mouse position
    auto [width, height] = m_window->GetSize();
    m_currentState.windowSize = Vec2(static_cast<float>(width), static_cast<float>(height));
    m_currentState.mousePosition = Vec2(static_cast<float>(width) / 2.0f, static_cast<float>(height) / 2.0f);
    m_lastMousePosition = m_currentState.mousePosition;

    m_initialized = true;
    std::cout << "InputManager initialized successfully" << std::endl;
    return true;
}

void InputManager::Shutdown() {
    if (!m_initialized) {
        return;
    }

    // Save bindings to config
    SaveBindingsToConfig();

    // Clear all callbacks
    m_inputCallbacks.clear();
    m_actionCallbacks.clear();

    // Clear event queue
    std::lock_guard<std::mutex> lock(m_eventMutex);
    while (!m_eventQueue.empty()) {
        m_eventQueue.pop();
    }

    m_initialized = false;
}

void InputManager::Update(double deltaTime) {
    if (!m_initialized) return;

    m_currentState.currentTime += deltaTime;
    m_currentState.deltaTime = deltaTime;

    // Update input state
    UpdateInputState();

    // Process pending events
    ProcessEvents();

    // Update mouse delta
    m_currentState.mouseDelta = Vec2(
        m_currentState.mousePosition.x - m_lastMousePosition.x,
        m_currentState.mousePosition.y - m_lastMousePosition.y
    );
    m_lastMousePosition = m_currentState.mousePosition;

    // Handle camera movement if camera is set
    if (m_camera && m_currentState.mouseLocked) {
        // Camera rotation based on mouse movement
        const float sensitivity = 0.1f;
        float deltaYaw = m_currentState.mouseDelta.x * sensitivity;
        float deltaPitch = m_currentState.mouseDelta.y * sensitivity;

        // Update camera rotation
        m_camera->rotation.x -= deltaPitch; // Pitch
        m_camera->rotation.y += deltaYaw;   // Yaw

        // Clamp pitch to prevent flipping
        m_camera->rotation.x = std::max(-89.0f, std::min(89.0f, m_camera->rotation.x));

        // Wrap yaw around
        if (m_camera->rotation.y > 360.0f) m_camera->rotation.y -= 360.0f;
        if (m_camera->rotation.y < -360.0f) m_camera->rotation.y += 360.0f;
    }

    // Handle player movement if player is set
    if (m_player) {
        Vec3 movement(0, 0, 0);

        // Forward/backward movement
        if (IsKeyPressed(KeyCode::W)) movement.z -= 1.0f;
        if (IsKeyPressed(KeyCode::S)) movement.z += 1.0f;

        // Left/right movement
        if (IsKeyPressed(KeyCode::A)) movement.x -= 1.0f;
        if (IsKeyPressed(KeyCode::D)) movement.x += 1.0f;

        // Jump
        if (IsKeyPressed(KeyCode::SPACE)) movement.y += 1.0f;

        // Sneak (crouch)
        if (IsKeyPressed(KeyCode::LEFT_SHIFT)) movement.y -= 1.0f;

        // Normalize movement vector if it's not zero
        float length = std::sqrt(movement.x * movement.x + movement.z * movement.z);
        if (length > 0.0f) {
            movement.x /= length;
            movement.z /= length;
        }

        // Apply movement speed
        float speed = 5.0f; // Base speed
        if (IsKeyPressed(KeyCode::LEFT_CONTROL)) speed *= 2.0f; // Sprint

        movement.x *= speed * static_cast<float>(deltaTime);
        movement.y *= speed * static_cast<float>(deltaTime);
        movement.z *= speed * static_cast<float>(deltaTime);

        // TODO: Apply movement to player
        // m_player->Move(movement);
    }
}

void InputManager::ProcessEvents() {
    std::lock_guard<std::mutex> lock(m_eventMutex);

    while (!m_eventQueue.empty()) {
        InputEvent event = m_eventQueue.front();
        m_eventQueue.pop();

        // Notify input callbacks
        for (const auto& callback : m_inputCallbacks) {
            callback.second(event);
        }
    }
}

int InputManager::RegisterInputCallback(InputCallback callback) {
    int id = m_nextCallbackId++;
    m_inputCallbacks[id] = callback;
    return id;
}

int InputManager::RegisterActionCallback(ActionCallback callback) {
    int id = m_nextCallbackId++;
    m_actionCallbacks[id] = callback;
    return id;
}

void InputManager::UnregisterCallback(int callbackId) {
    m_inputCallbacks.erase(callbackId);
    m_actionCallbacks.erase(callbackId);
}

void InputManager::BindAction(const InputBinding& binding) {
    m_actionBindings[binding.action] = binding;

    if (binding.isMouse) {
        m_mouseBindings[binding.mouseButton] = binding.action;
    } else {
        m_keyBindings[binding.key] = binding.action;
    }
}

void InputManager::UnbindAction(GameAction action) {
    auto it = m_actionBindings.find(action);
    if (it != m_actionBindings.end()) {
        const InputBinding& binding = it->second;
        if (binding.isMouse) {
            m_mouseBindings.erase(binding.mouseButton);
        } else {
            m_keyBindings.erase(binding.key);
        }
        m_actionBindings.erase(it);
    }
}

bool InputManager::IsKeyPressed(KeyCode key) const {
    return m_currentState.pressedKeys.find(key) != m_currentState.pressedKeys.end();
}

bool InputManager::IsMouseButtonPressed(MouseButton button) const {
    return m_currentState.pressedButtons.find(button) != m_currentState.pressedButtons.end();
}

void InputManager::SetMouseLocked(bool locked) {
    m_currentState.mouseLocked = locked;

    if (m_window) {
        if (locked) {
            m_window->SetInputMode(GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            // Center cursor for smooth camera movement
            auto [width, height] = m_window->GetSize();
            m_window->SetCursorPos(width / 2.0, height / 2.0);
        } else {
            m_window->SetInputMode(GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }
}

void InputManager::LoadBindingsFromConfig() {
    if (!m_config) return;

    // TODO: Load custom bindings from config
    // For now, use default bindings
}

void InputManager::SaveBindingsToConfig() {
    if (!m_config) return;

    // TODO: Save custom bindings to config
}

void InputManager::SetCamera(std::shared_ptr<Camera> camera) {
    m_camera = camera;
}

void InputManager::SetPlayer(std::shared_ptr<Player> player) {
    m_player = player;
}

void InputManager::Reset() {
    m_currentState.pressedKeys.clear();
    m_currentState.pressedButtons.clear();
    m_currentState.mouseDelta = Vec2(0, 0);
    m_currentState.scrollDelta = Vec2(0, 0);

    // Clear event queue
    std::lock_guard<std::mutex> lock(m_eventMutex);
    while (!m_eventQueue.empty()) {
        m_eventQueue.pop();
    }
}

void InputManager::ProcessKeyEvent(KeyCode key, int scancode, InputAction action, int mods) {
    InputEvent event;
    event.type = InputEvent::Type::KEY;
    event.key.key = key;
    event.key.scancode = scancode;
    event.key.action = action;
    event.key.mods = mods;
    event.timestamp = m_currentState.currentTime;

    // Update pressed keys
    if (action == InputAction::PRESS) {
        m_currentState.pressedKeys.insert(key);
        m_currentState.keyPressTime[key] = m_currentState.currentTime;
    } else if (action == InputAction::RELEASE) {
        m_currentState.pressedKeys.erase(key);
    }

    // Check for bound actions
    auto it = m_keyBindings.find(key);
    if (it != m_keyBindings.end()) {
        bool pressed = (action == InputAction::PRESS);
        TriggerAction(it->second, pressed);
    }

    // Queue event
    std::lock_guard<std::mutex> lock(m_eventMutex);
    m_eventQueue.push(event);
}

void InputManager::ProcessMouseButtonEvent(MouseButton button, InputAction action, int mods) {
    InputEvent event;
    event.type = InputEvent::Type::MOUSE_BUTTON;
    event.mouseButton.button = button;
    event.mouseButton.action = action;
    event.mouseButton.mods = mods;
    event.timestamp = m_currentState.currentTime;

    // Update pressed buttons
    if (action == InputAction::PRESS) {
        m_currentState.pressedButtons.insert(button);
    } else if (action == InputAction::RELEASE) {
        m_currentState.pressedButtons.erase(button);
    }

    // Check for bound actions
    auto it = m_mouseBindings.find(button);
    if (it != m_mouseBindings.end()) {
        bool pressed = (action == InputAction::PRESS);
        TriggerAction(it->second, pressed);
    }

    // Queue event
    std::lock_guard<std::mutex> lock(m_eventMutex);
    m_eventQueue.push(event);
}

void InputManager::ProcessMouseMoveEvent(double xpos, double ypos) {
    m_currentState.mousePosition = Vec2(static_cast<float>(xpos), static_cast<float>(ypos));

    InputEvent event;
    event.type = InputEvent::Type::MOUSE_MOVE;
    event.mouseMove.x = xpos;
    event.mouseMove.y = ypos;
    event.mouseMove.deltaX = xpos - m_lastMousePosition.x;
    event.mouseMove.deltaY = ypos - m_lastMousePosition.y;
    event.timestamp = m_currentState.currentTime;

    // Queue event
    std::lock_guard<std::mutex> lock(m_eventMutex);
    m_eventQueue.push(event);
}

void InputManager::ProcessMouseScrollEvent(double xoffset, double yoffset) {
    m_currentState.scrollDelta = Vec2(static_cast<float>(xoffset), static_cast<float>(yoffset));

    InputEvent event;
    event.type = InputEvent::Type::MOUSE_SCROLL;
    event.mouseScroll.xoffset = xoffset;
    event.mouseScroll.yoffset = yoffset;
    event.timestamp = m_currentState.currentTime;

    // Queue event
    std::lock_guard<std::mutex> lock(m_eventMutex);
    m_eventQueue.push(event);
}

void InputManager::ProcessWindowResizeEvent(int width, int height) {
    m_currentState.windowSize = Vec2(static_cast<float>(width), static_cast<float>(height));

    InputEvent event;
    event.type = InputEvent::Type::WINDOW_RESIZE;
    event.windowResize.width = width;
    event.windowResize.height = height;
    event.timestamp = m_currentState.currentTime;

    // Queue event
    std::lock_guard<std::mutex> lock(m_eventMutex);
    m_eventQueue.push(event);
}

void InputManager::UpdateInputState() {
    // Update previous state
    m_previousState = m_currentState;

    // Reset per-frame values
    m_currentState.mouseDelta = Vec2(0, 0);
    m_currentState.scrollDelta = Vec2(0, 0);
}

void InputManager::TriggerAction(GameAction action, bool pressed) {
    // Notify action callbacks
    for (const auto& callback : m_actionCallbacks) {
        callback.second(action, pressed);
    }
}

void InputManager::SetupDefaultBindings() {
    // Movement
    BindAction({GameAction::MOVE_FORWARD, KeyCode::W, MouseButton::LEFT, false, 0});
    BindAction({GameAction::MOVE_BACKWARD, KeyCode::S, MouseButton::LEFT, false, 0});
    BindAction({GameAction::MOVE_LEFT, KeyCode::A, MouseButton::LEFT, false, 0});
    BindAction({GameAction::MOVE_RIGHT, KeyCode::D, MouseButton::LEFT, false, 0});
    BindAction({GameAction::JUMP, KeyCode::SPACE, MouseButton::LEFT, false, 0});
    BindAction({GameAction::SNEAK, KeyCode::LEFT_SHIFT, MouseButton::LEFT, false, 0});
    BindAction({GameAction::SPRINT, KeyCode::LEFT_CONTROL, MouseButton::LEFT, false, 0});

    // Interaction
    BindAction({GameAction::ATTACK, KeyCode::UNKNOWN, MouseButton::LEFT, true, 0});
    BindAction({GameAction::USE_ITEM, KeyCode::UNKNOWN, MouseButton::RIGHT, true, 0});
    BindAction({GameAction::INTERACT, KeyCode::E, MouseButton::LEFT, false, 0});

    // Inventory
    BindAction({GameAction::INVENTORY, KeyCode::E, MouseButton::LEFT, false, 0});
    BindAction({GameAction::DROP_ITEM, KeyCode::Q, MouseButton::LEFT, false, 0});

    // System
    BindAction({GameAction::PAUSE, KeyCode::ESCAPE, MouseButton::LEFT, false, 0});
    BindAction({GameAction::DEBUG, KeyCode::F3, MouseButton::LEFT, false, 0});
    BindAction({GameAction::TOGGLE_WIREFRAME, KeyCode::F4, MouseButton::LEFT, false, 0});
    BindAction({GameAction::TOGGLE_FULLSCREEN, KeyCode::F11, MouseButton::LEFT, false, 0});

    // Chat and commands
    BindAction({GameAction::CHAT, KeyCode::T, MouseButton::LEFT, false, 0});
    BindAction({GameAction::COMMAND, KeyCode::SLASH, MouseButton::LEFT, false, 0});
}

// Static helper functions
KeyCode InputManager::GLFWKeyToKeyCode(int glfwKey) {
    if (glfwKey >= GLFW_KEY_SPACE && glfwKey <= GLFW_KEY_GRAVE_ACCENT) {
        return static_cast<KeyCode>(glfwKey);
    }

    switch (glfwKey) {
        case GLFW_KEY_ESCAPE: return KeyCode::ESCAPE;
        case GLFW_KEY_ENTER: return KeyCode::ENTER;
        case GLFW_KEY_TAB: return KeyCode::TAB;
        case GLFW_KEY_BACKSPACE: return KeyCode::BACKSPACE;
        case GLFW_KEY_INSERT: return KeyCode::INSERT;
        case GLFW_KEY_DELETE: return KeyCode::DELETE;
        case GLFW_KEY_RIGHT: return KeyCode::RIGHT;
        case GLFW_KEY_LEFT: return KeyCode::LEFT;
        case GLFW_KEY_DOWN: return KeyCode::DOWN;
        case GLFW_KEY_UP: return KeyCode::UP;
        case GLFW_KEY_PAGE_UP: return KeyCode::PAGE_UP;
        case GLFW_KEY_PAGE_DOWN: return KeyCode::PAGE_DOWN;
        case GLFW_KEY_HOME: return KeyCode::HOME;
        case GLFW_KEY_END: return KeyCode::END;
        case GLFW_KEY_CAPS_LOCK: return KeyCode::CAPS_LOCK;
        case GLFW_KEY_SCROLL_LOCK: return KeyCode::SCROLL_LOCK;
        case GLFW_KEY_NUM_LOCK: return KeyCode::NUM_LOCK;
        case GLFW_KEY_PRINT_SCREEN: return KeyCode::PRINT_SCREEN;
        case GLFW_KEY_PAUSE: return KeyCode::PAUSE;
        case GLFW_KEY_F1: return KeyCode::F1;
        case GLFW_KEY_F2: return KeyCode::F2;
        case GLFW_KEY_F3: return KeyCode::F3;
        case GLFW_KEY_F4: return KeyCode::F4;
        case GLFW_KEY_F5: return KeyCode::F5;
        case GLFW_KEY_F6: return KeyCode::F6;
        case GLFW_KEY_F7: return KeyCode::F7;
        case GLFW_KEY_F8: return KeyCode::F8;
        case GLFW_KEY_F9: return KeyCode::F9;
        case GLFW_KEY_F10: return KeyCode::F10;
        case GLFW_KEY_F11: return KeyCode::F11;
        case GLFW_KEY_F12: return KeyCode::F12;
        case GLFW_KEY_F13: return KeyCode::F13;
        case GLFW_KEY_F14: return KeyCode::F14;
        case GLFW_KEY_F15: return KeyCode::F15;
        case GLFW_KEY_F16: return KeyCode::F16;
        case GLFW_KEY_F17: return KeyCode::F17;
        case GLFW_KEY_F18: return KeyCode::F18;
        case GLFW_KEY_F19: return KeyCode::F19;
        case GLFW_KEY_F20: return KeyCode::F20;
        case GLFW_KEY_F21: return KeyCode::F21;
        case GLFW_KEY_F22: return KeyCode::F22;
        case GLFW_KEY_F23: return KeyCode::F23;
        case GLFW_KEY_F24: return KeyCode::F24;
        case GLFW_KEY_F25: return KeyCode::F25;
        case GLFW_KEY_KP_0: return KeyCode::KP_0;
        case GLFW_KEY_KP_1: return KeyCode::KP_1;
        case GLFW_KEY_KP_2: return KeyCode::KP_2;
        case GLFW_KEY_KP_3: return KeyCode::KP_3;
        case GLFW_KEY_KP_4: return KeyCode::KP_4;
        case GLFW_KEY_KP_5: return KeyCode::KP_5;
        case GLFW_KEY_KP_6: return KeyCode::KP_6;
        case GLFW_KEY_KP_7: return KeyCode::KP_7;
        case GLFW_KEY_KP_8: return KeyCode::KP_8;
        case GLFW_KEY_KP_9: return KeyCode::KP_9;
        case GLFW_KEY_KP_DECIMAL: return KeyCode::KP_DECIMAL;
        case GLFW_KEY_KP_DIVIDE: return KeyCode::KP_DIVIDE;
        case GLFW_KEY_KP_MULTIPLY: return KeyCode::KP_MULTIPLY;
        case GLFW_KEY_KP_SUBTRACT: return KeyCode::KP_SUBTRACT;
        case GLFW_KEY_KP_ADD: return KeyCode::KP_ADD;
        case GLFW_KEY_KP_ENTER: return KeyCode::KP_ENTER;
        case GLFW_KEY_KP_EQUAL: return KeyCode::KP_EQUAL;
        case GLFW_KEY_LEFT_SHIFT: return KeyCode::LEFT_SHIFT;
        case GLFW_KEY_LEFT_CONTROL: return KeyCode::LEFT_CONTROL;
        case GLFW_KEY_LEFT_ALT: return KeyCode::LEFT_ALT;
        case GLFW_KEY_LEFT_SUPER: return KeyCode::LEFT_SUPER;
        case GLFW_KEY_RIGHT_SHIFT: return KeyCode::RIGHT_SHIFT;
        case GLFW_KEY_RIGHT_CONTROL: return KeyCode::RIGHT_CONTROL;
        case GLFW_KEY_RIGHT_ALT: return KeyCode::RIGHT_ALT;
        case GLFW_KEY_RIGHT_SUPER: return KeyCode::RIGHT_SUPER;
        case GLFW_KEY_MENU: return KeyCode::MENU;
        default: return KeyCode::UNKNOWN;
    }
}

MouseButton InputManager::GLFWMouseButtonToMouseButton(int glfwButton) {
    if (glfwButton >= 0 && glfwButton <= 7) {
        return static_cast<MouseButton>(glfwButton);
    }
    return MouseButton::LEFT;
}

InputAction InputManager::GLFWActionToInputAction(int glfwAction) {
    switch (glfwAction) {
        case GLFW_PRESS: return InputAction::PRESS;
        case GLFW_RELEASE: return InputAction::RELEASE;
        case GLFW_REPEAT: return InputAction::REPEAT;
        default: return InputAction::RELEASE;
    }
}

} // namespace VoxelCraft
