/**
 * @file InputSystem.cpp
 * @brief VoxelCraft Advanced Input Management System Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#include "InputSystem.hpp"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <functional>
#include <queue>
#include <memory>
#include <variant>

#include "../utils/Random.hpp"
#include "../logging/Logger.hpp"

namespace VoxelCraft {

    // Static member initialization
    std::atomic<uint32_t> InputSystem::s_nextDeviceId{1};

    // InputContext implementation
    InputContext::InputContext(const std::string& name)
        : m_name(name), m_enabled(true) {
    }

    void InputContext::AddAction(std::shared_ptr<InputAction> action) {
        if (!action) return;

        std::unique_lock<std::shared_mutex> lock(m_actionsMutex);
        m_actions[action->name] = action;
    }

    void InputContext::RemoveAction(const std::string& name) {
        std::unique_lock<std::shared_mutex> lock(m_actionsMutex);
        m_actions.erase(name);
    }

    std::shared_ptr<InputAction> InputContext::GetAction(const std::string& name) const {
        std::shared_lock<std::shared_mutex> lock(m_actionsMutex);

        auto it = m_actions.find(name);
        return it != m_actions.end() ? it->second : nullptr;
    }

    void InputContext::Update(float deltaTime) {
        if (!m_enabled) return;

        std::shared_lock<std::shared_mutex> lock(m_actionsMutex);

        for (auto& pair : m_actions) {
            auto& action = pair.second;
            if (action->enabled) {
                action->previousValue = action->value;
                // Update action value based on input bindings
                // This would be implemented based on the actual input bindings
            }
        }
    }

    bool InputContext::ProcessEvent(const InputEvent& event) {
        if (!m_enabled) return false;

        // Process event through actions
        // This would be implemented based on the actual event processing logic
        return false;
    }

    void InputContext::ClearActions() {
        std::unique_lock<std::shared_mutex> lock(m_actionsMutex);
        m_actions.clear();
    }

    // InputDevice implementation
    InputDevice::InputDevice(uint32_t id, InputDeviceType type, const std::string& name)
        : m_id(id), m_type(type), m_name(name), m_connected(false) {
    }

    // InputSystem implementation
    InputSystem& InputSystem::GetInstance() {
        static InputSystem instance;
        return instance;
    }

    InputSystem::InputSystem()
        : m_inputEnabled(true) {
    }

    InputSystem::~InputSystem() {
        Shutdown();
    }

    bool InputSystem::Initialize() {
        if (!System::Initialize()) {
            return false;
        }

        Logger::GetInstance().Info("InputSystem initializing", "InputSystem");

        // Initialize default input devices
        // This would initialize keyboard, mouse, gamepad, etc.

        Logger::GetInstance().Info("InputSystem initialized successfully", "InputSystem");
        return true;
    }

    void InputSystem::Update(float deltaTime) {
        System::Update(deltaTime);

        if (!m_inputEnabled) return;

        // Update all devices
        UpdateDevices(deltaTime);

        // Update active contexts
        UpdateContexts(deltaTime);

        // Update actions
        UpdateActions(deltaTime);

        // Process queued events
        ProcessQueuedEvents();
    }

    void InputSystem::Shutdown() {
        if (!System::Shutdown()) {
            return;
        }

        Logger::GetInstance().Info("InputSystem shutting down", "InputSystem");

        // Clear all devices
        {
            std::unique_lock<std::shared_mutex> lock(m_devicesMutex);
            for (auto& pair : m_devices) {
                if (pair.second) {
                    pair.second->Disconnect();
                }
            }
            m_devices.clear();
        }

        // Clear all contexts
        {
            std::unique_lock<std::shared_mutex> lock(m_contextsMutex);
            m_contexts.clear();
            m_activeContexts.clear();
        }

        // Clear all actions
        {
            std::unique_lock<std::shared_mutex> lock(m_actionsMutex);
            m_actions.clear();
        }

        // Clear event queue
        {
            std::unique_lock<std::mutex> lock(m_eventQueueMutex);
            std::queue<InputEvent> empty;
            std::swap(m_eventQueue, empty);
        }

        Logger::GetInstance().Info("InputSystem shutdown complete", "InputSystem");
    }

    uint32_t InputSystem::RegisterDevice(std::shared_ptr<InputDevice> device) {
        if (!device) return 0;

        std::unique_lock<std::shared_mutex> lock(m_devicesMutex);

        uint32_t deviceId = device->GetId();
        if (m_devices.find(deviceId) != m_devices.end()) {
            return deviceId; // Already registered
        }

        m_devices[deviceId] = device;

        Logger::GetInstance().Debug("Registered input device: " + device->GetName(), "InputSystem");
        return deviceId;
    }

    bool InputSystem::UnregisterDevice(uint32_t deviceId) {
        std::unique_lock<std::shared_mutex> lock(m_devicesMutex);

        auto it = m_devices.find(deviceId);
        if (it == m_devices.end()) {
            return false;
        }

        auto device = it->second;
        if (device && device->IsConnected()) {
            device->Disconnect();
        }

        m_devices.erase(it);

        Logger::GetInstance().Debug("Unregistered input device ID: " + std::to_string(deviceId), "InputSystem");
        return true;
    }

    std::shared_ptr<InputDevice> InputSystem::GetDevice(uint32_t deviceId) const {
        std::shared_lock<std::shared_mutex> lock(m_devicesMutex);

        auto it = m_devices.find(deviceId);
        return it != m_devices.end() ? it->second : nullptr;
    }

    size_t InputSystem::GetDeviceCount() const {
        std::shared_lock<std::shared_mutex> lock(m_devicesMutex);
        return m_devices.size();
    }

    std::shared_ptr<InputContext> InputSystem::CreateContext(const std::string& name) {
        std::unique_lock<std::shared_mutex> lock(m_contextsMutex);

        if (m_contexts.find(name) != m_contexts.end()) {
            return m_contexts[name];
        }

        auto context = std::make_shared<InputContext>(name);
        m_contexts[name] = context;

        Logger::GetInstance().Debug("Created input context: " + name, "InputSystem");
        return context;
    }

    bool InputSystem::DestroyContext(const std::string& name) {
        std::unique_lock<std::shared_mutex> lock(m_contextsMutex);

        auto it = m_contexts.find(name);
        if (it == m_contexts.end()) {
            return false;
        }

        // Remove from active contexts
        auto activeIt = std::find(m_activeContexts.begin(), m_activeContexts.end(), name);
        if (activeIt != m_activeContexts.end()) {
            m_activeContexts.erase(activeIt);
        }

        m_contexts.erase(it);

        Logger::GetInstance().Debug("Destroyed input context: " + name, "InputSystem");
        return true;
    }

    std::shared_ptr<InputContext> InputSystem::GetContext(const std::string& name) const {
        std::shared_lock<std::shared_mutex> lock(m_contextsMutex);

        auto it = m_contexts.find(name);
        return it != m_contexts.end() ? it->second : nullptr;
    }

    bool InputSystem::ActivateContext(const std::string& name) {
        std::unique_lock<std::shared_mutex> lock(m_contextsMutex);

        if (m_contexts.find(name) == m_contexts.end()) {
            return false;
        }

        // Check if already active
        if (std::find(m_activeContexts.begin(), m_activeContexts.end(), name) != m_activeContexts.end()) {
            return true;
        }

        m_activeContexts.push_back(name);
        SortActiveContextsByPriority();

        Logger::GetInstance().Debug("Activated input context: " + name, "InputSystem");
        return true;
    }

    bool InputSystem::DeactivateContext(const std::string& name) {
        std::unique_lock<std::shared_mutex> lock(m_contextsMutex);

        auto it = std::find(m_activeContexts.begin(), m_activeContexts.end(), name);
        if (it == m_activeContexts.end()) {
            return false;
        }

        m_activeContexts.erase(it);

        Logger::GetInstance().Debug("Deactivated input context: " + name, "InputSystem");
        return true;
    }

    std::shared_ptr<InputAction> InputSystem::CreateAction(const std::string& name, InputActionType type) {
        std::unique_lock<std::shared_mutex> lock(m_actionsMutex);

        if (m_actions.find(name) != m_actions.end()) {
            return m_actions[name];
        }

        auto action = std::make_shared<InputAction>();
        action->name = name;
        action->type = type;
        action->enabled = true;
        action->sensitivity = 1.0f;

        m_actions[name] = action;

        Logger::GetInstance().Debug("Created input action: " + name, "InputSystem");
        return action;
    }

    bool InputSystem::DestroyAction(const std::string& name) {
        std::unique_lock<std::shared_mutex> lock(m_actionsMutex);

        auto it = m_actions.find(name);
        if (it == m_actions.end()) {
            return false;
        }

        m_actions.erase(it);

        Logger::GetInstance().Debug("Destroyed input action: " + name, "InputSystem");
        return true;
    }

    std::shared_ptr<InputAction> InputSystem::GetAction(const std::string& name) const {
        std::shared_lock<std::shared_mutex> lock(m_actionsMutex);

        auto it = m_actions.find(name);
        return it != m_actions.end() ? it->second : nullptr;
    }

    void InputSystem::ProcessEvent(const InputEvent& event) {
        if (!m_inputEnabled) return;

        // Process event through active contexts
        std::shared_lock<std::shared_mutex> lock(m_contextsMutex);

        for (const auto& contextName : m_activeContexts) {
            auto it = m_contexts.find(contextName);
            if (it != m_contexts.end()) {
                if (it->second->ProcessEvent(event)) {
                    break; // Event was consumed
                }
            }
        }
    }

    void InputSystem::QueueEvent(const InputEvent& event) {
        std::unique_lock<std::mutex> lock(m_eventQueueMutex);
        m_eventQueue.push(event);
    }

    std::vector<InputEvent> InputSystem::PollEvents() {
        std::vector<InputEvent> events;

        // Poll events from all devices
        std::shared_lock<std::shared_mutex> lock(m_devicesMutex);

        for (auto& pair : m_devices) {
            if (pair.second && pair.second->IsConnected()) {
                auto deviceEvents = pair.second->PollEvents();
                events.insert(events.end(), deviceEvents.begin(), deviceEvents.end());
            }
        }

        return events;
    }

    bool InputSystem::IsActionPressed(const std::string& action) const {
        auto actionPtr = GetAction(action);
        return actionPtr ? actionPtr->IsPressed() : false;
    }

    bool InputSystem::IsActionHeld(const std::string& action) const {
        auto actionPtr = GetAction(action);
        return actionPtr ? actionPtr->IsHeld() : false;
    }

    bool InputSystem::WasActionPressed(const std::string& action) const {
        auto actionPtr = GetAction(action);
        return actionPtr ? actionPtr->WasPressed() : false;
    }

    bool InputSystem::WasActionReleased(const std::string& action) const {
        auto actionPtr = GetAction(action);
        return actionPtr ? actionPtr->WasReleased() : false;
    }

    InputActionValue InputSystem::GetActionValue(const std::string& action) const {
        auto actionPtr = GetAction(action);
        return actionPtr ? actionPtr->value : InputActionValue();
    }

    std::string InputSystem::GetInputInfo() const {
        std::stringstream ss;
        ss << "Input System Info:\n";
        ss << "Devices: " << GetDeviceCount() << "\n";
        ss << "Contexts: " << m_contexts.size() << "\n";
        ss << "Active Contexts: " << m_activeContexts.size() << "\n";
        ss << "Actions: " << m_actions.size() << "\n";
        ss << "Input Enabled: " << (m_inputEnabled ? "Yes" : "No") << "\n";
        return ss.str();
    }

    void InputSystem::ClearAllActions() {
        std::unique_lock<std::shared_mutex> lock(m_actionsMutex);
        m_actions.clear();
    }

    void InputSystem::UpdateDevices(float deltaTime) {
        std::shared_lock<std::shared_mutex> lock(m_devicesMutex);

        for (auto& pair : m_devices) {
            if (pair.second && pair.second->IsConnected()) {
                pair.second->Update(deltaTime);
            }
        }
    }

    void InputSystem::UpdateContexts(float deltaTime) {
        std::shared_lock<std::shared_mutex> lock(m_contextsMutex);

        for (const auto& contextName : m_activeContexts) {
            auto it = m_contexts.find(contextName);
            if (it != m_contexts.end()) {
                it->second->Update(deltaTime);
            }
        }
    }

    void InputSystem::UpdateActions(float deltaTime) {
        std::shared_lock<std::shared_mutex> lock(m_actionsMutex);

        for (auto& pair : m_actions) {
            auto& action = pair.second;
            if (action->enabled) {
                action->previousValue = action->value;
                // Update action value based on input bindings
                // This would be implemented based on the actual input bindings
            }
        }
    }

    void InputSystem::ProcessQueuedEvents() {
        std::unique_lock<std::mutex> lock(m_eventQueueMutex);

        while (!m_eventQueue.empty()) {
            InputEvent event = m_eventQueue.front();
            m_eventQueue.pop();
            lock.unlock();

            ProcessEvent(event);

            lock.lock();
        }
    }

    void InputSystem::SortActiveContextsByPriority() {
        // Sort contexts by priority (highest first)
        std::sort(m_activeContexts.begin(), m_activeContexts.end(),
            [this](const std::string& a, const std::string& b) {
                auto contextA = m_contexts[a];
                auto contextB = m_contexts[b];
                return contextA->GetPriority() > contextB->GetPriority();
            });
    }

} // namespace VoxelCraft
