/**
 * @file UIManager.cpp
 * @brief VoxelCraft UI Manager Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file implements the UIManager class that provides comprehensive UI management
 * for the VoxelCraft game engine, including HUD, menus, inventory, chat, and
 * advanced UI rendering with modern graphics and animations.
 */

#include "UIManager.hpp"
#include "UIElement.hpp"
#include "UILayout.hpp"
#include "UIStyle.hpp"
#include "UIRenderer.hpp"
#include "HUD.hpp"
#include "MenuSystem.hpp"
#include "InventoryUI.hpp"
#include "ChatSystem.hpp"
#include "SettingsUI.hpp"

#include "../core/Logger.hpp"
#include "../entities/System.hpp"
#include "../world/World.hpp"

#include <algorithm>
#include <chrono>
#include <sstream>
#include <filesystem>

namespace VoxelCraft {

    // Static member initialization
    std::atomic<uint32_t> UIManager::s_nextElementId(1);
    std::atomic<uint32_t> UIManager::s_nextAnimationId(1);
    std::atomic<uint32_t> UIManager::s_nextListenerId(1);

    UIManager::UIManager(const UIManagerConfig& config)
        : m_config(config)
        , m_state(UIState::Inactive)
        , m_mode(UIMode::Game)
        , m_inputMode(UIInputMode::MouseKeyboard)
        , m_window(nullptr)
        , m_renderer(nullptr)
        , m_inputSystem(nullptr)
        , m_fontManager(nullptr)
        , m_textureManager(nullptr)
        , m_soundManager(nullptr)
        , m_player(nullptr)
        , m_world(nullptr)
        , m_hud(nullptr)
        , m_menuSystem(nullptr)
        , m_inventoryUI(nullptr)
        , m_chatSystem(nullptr)
        , m_settingsUI(nullptr)
        , m_uiRenderer(nullptr)
        , m_focusedElement(0)
        , m_isInitialized(false)
        , m_lastUpdateTime(0.0)
    {
        // Initialize metrics
        m_metrics = {};

        // Set initial mode
        SetMode(m_config.defaultMode);
    }

    UIManager::~UIManager() {
        Shutdown();
    }

    bool UIManager::Initialize(Window* window, Renderer* renderer) {
        if (m_isInitialized) {
            VOXELCRAFT_WARNING("UIManager already initialized");
            return true;
        }

        VOXELCRAFT_INFO("Initializing UIManager...");

        try {
            m_window = window;
            m_renderer = renderer;

            // Initialize subsystems
            if (!InitializeSubsystems()) {
                VOXELCRAFT_ERROR("Failed to initialize UI subsystems");
                return false;
            }

            // Create default elements
            CreateDefaultElements();

            m_isInitialized = true;
            m_state = UIState::Active;

            VOXELCRAFT_INFO("UIManager initialized successfully");
            return true;

        } catch (const std::exception& e) {
            VOXELCRAFT_ERROR("Exception during UIManager initialization: " + std::string(e.what()));
            return false;
        }
    }

    void UIManager::Shutdown() {
        if (!m_isInitialized) {
            return;
        }

        VOXELCRAFT_INFO("Shutting down UIManager...");

        // Clear all elements
        ClearElements();

        // Shutdown subsystems
        if (m_uiRenderer) {
            m_uiRenderer->Shutdown();
        }

        if (m_settingsUI) {
            m_settingsUI->Shutdown();
        }

        if (m_chatSystem) {
            m_chatSystem->Shutdown();
        }

        if (m_inventoryUI) {
            m_inventoryUI->Shutdown();
        }

        if (m_menuSystem) {
            m_menuSystem->Shutdown();
        }

        if (m_hud) {
            m_hud->Shutdown();
        }

        // Clear managers
        m_fontManager = nullptr;
        m_textureManager = nullptr;
        m_soundManager = nullptr;

        // Clear references
        m_player = nullptr;
        m_world = nullptr;
        m_window = nullptr;
        m_renderer = nullptr;

        m_isInitialized = false;
        m_state = UIState::Inactive;

        VOXELCRAFT_INFO("UIManager shutdown complete");
    }

    void UIManager::Update(double deltaTime) {
        if (!m_isInitialized || m_state != UIState::Active) {
            return;
        }

        try {
            double currentTime = std::chrono::duration<double>(
                std::chrono::high_resolution_clock::now().time_since_epoch()).count();

            // Update elements
            UpdateElements(deltaTime);

            // Update animations
            UpdateAnimations(deltaTime);

            // Update input
            UpdateInput(deltaTime);

            // Update subsystems
            if (m_hud) {
                m_hud->Update(deltaTime);
            }

            if (m_menuSystem) {
                m_menuSystem->Update(deltaTime);
            }

            if (m_inventoryUI) {
                m_inventoryUI->Update(deltaTime);
            }

            if (m_chatSystem) {
                m_chatSystem->Update(deltaTime);
            }

            if (m_settingsUI) {
                m_settingsUI->Update(deltaTime);
            }

            // Update metrics
            UpdateMetrics(deltaTime);

            m_lastUpdateTime = currentTime;

        } catch (const std::exception& e) {
            VOXELCRAFT_ERROR("Exception during UIManager update: " + std::string(e.what()));
            HandleError("Update exception: " + std::string(e.what()));
        }
    }

    void UIManager::Render() {
        if (!m_isInitialized || m_state != UIState::Active) {
            return;
        }

        try {
            // Render elements
            RenderElements();

            // Render subsystems
            if (m_hud) {
                m_hud->Render();
            }

            if (m_menuSystem) {
                m_menuSystem->Render();
            }

            if (m_inventoryUI) {
                m_inventoryUI->Render();
            }

            if (m_chatSystem) {
                m_chatSystem->Render();
            }

            if (m_settingsUI) {
                m_settingsUI->Render();
            }

        } catch (const std::exception& e) {
            VOXELCRAFT_ERROR("Exception during UIManager render: " + std::string(e.what()));
            HandleError("Render exception: " + std::string(e.what()));
        }
    }

    bool UIManager::InitializeSubsystems() {
        VOXELCRAFT_INFO("Initializing UI subsystems...");

        try {
            // Create HUD system
            HUDConfig hudConfig;
            hudConfig.enabled = true;
            hudConfig.showInMenus = false;
            hudConfig.showInInventory = false;
            hudConfig.globalScale = m_config.uiScale;
            hudConfig.globalOpacity = m_config.globalOpacity;

            m_hud = std::make_unique<HUD>(hudConfig);
            if (!m_hud->Initialize(this)) {
                VOXELCRAFT_ERROR("Failed to initialize HUD system");
                return false;
            }

            // Create menu system
            // m_menuSystem = std::make_unique<MenuSystem>(...);

            // Create inventory UI
            // m_inventoryUI = std::make_unique<InventoryUI>(...);

            // Create chat system
            // m_chatSystem = std::make_unique<ChatSystem>(...);

            // Create settings UI
            // m_settingsUI = std::make_unique<SettingsUI>(...);

            // Create UI renderer
            // m_uiRenderer = std::make_unique<UIRenderer>(...);

            VOXELCRAFT_INFO("UI subsystems initialized successfully");
            return true;

        } catch (const std::exception& e) {
            VOXELCRAFT_ERROR("Exception initializing UI subsystems: " + std::string(e.what()));
            return false;
        }
    }

    void UIManager::UpdateElements(double deltaTime) {
        std::vector<std::shared_ptr<UIElement>> elementsToUpdate;

        // Get elements to update (thread-safe copy)
        {
            std::shared_lock<std::shared_mutex> lock(m_elementsMutex);
            elementsToUpdate.reserve(m_elements.size());

            for (const auto& [id, element] : m_elements) {
                if (element && element->IsVisible()) {
                    elementsToUpdate.push_back(element);
                }
            }
        }

        // Update elements
        for (auto& element : elementsToUpdate) {
            try {
                element->Update(deltaTime);
            } catch (const std::exception& e) {
                VOXELCRAFT_ERROR("Exception updating element " +
                               std::to_string(element->GetId()) + ": " + e.what());
            }
        }
    }

    void UIManager::UpdateAnimations(double deltaTime) {
        std::vector<std::shared_ptr<UIElement>> animatedElements;

        // Get animated elements
        {
            std::shared_lock<std::shared_mutex> lock(m_elementsMutex);

            for (const auto& [id, element] : m_elements) {
                if (element && element->IsVisible()) {
                    // Check if element has active animations
                    // This would be implemented in UIElement::UpdateAnimations
                    animatedElements.push_back(element);
                }
            }
        }

        // Update animations
        for (auto& element : animatedElements) {
            element->UpdateAnimations(deltaTime);
        }
    }

    void UIManager::UpdateInput(double deltaTime) {
        // Update input state
        // This would handle things like mouse hover detection,
        // keyboard focus management, etc.
    }

    void UIManager::RenderElements() {
        if (!m_uiRenderer) {
            return;
        }

        try {
            // Sort elements by Z-order for proper rendering
            std::vector<std::shared_ptr<UIElement>> renderList;

            {
                std::shared_lock<std::shared_mutex> lock(m_elementsMutex);
                renderList.reserve(m_elements.size());

                for (const auto& [id, element] : m_elements) {
                    if (element && element->IsVisible()) {
                        renderList.push_back(element);
                    }
                }
            }

            // Sort by Z-order
            std::sort(renderList.begin(), renderList.end(),
                     [](const std::shared_ptr<UIElement>& a, const std::shared_ptr<UIElement>& b) {
                         return a->GetZOrder() < b->GetZOrder();
                     });

            // Render elements
            for (auto& element : renderList) {
                try {
                    element->Render();
                } catch (const std::exception& e) {
                    VOXELCRAFT_ERROR("Exception rendering element " +
                                   std::to_string(element->GetId()) + ": " + e.what());
                }
            }

        } catch (const std::exception& e) {
            VOXELCRAFT_ERROR("Exception in RenderElements: " + std::string(e.what()));
        }
    }

    uint32_t UIManager::AddElement(std::shared_ptr<UIElement> element) {
        if (!m_isInitialized || !element) {
            return 0;
        }

        uint32_t elementId = s_nextElementId++;
        element->SetId(elementId);

        {
            std::unique_lock<std::shared_mutex> lock(m_elementsMutex);
            m_elements[elementId] = element;
            m_elementNames[element->GetName()] = elementId;
        }

        // Initialize element
        element->Initialize(this);

        VOXELCRAFT_DEBUG("Added UI element: " + element->GetName() +
                        " (ID: " + std::to_string(elementId) + ")");

        ++m_metrics.totalElements;
        return elementId;
    }

    bool UIManager::RemoveElement(uint32_t elementId) {
        std::unique_lock<std::shared_mutex> lock(m_elementsMutex);

        auto it = m_elements.find(elementId);
        if (it == m_elements.end()) {
            VOXELCRAFT_WARNING("UI element " + std::to_string(elementId) + " not found");
            return false;
        }

        auto& element = it->second;
        if (element) {
            // Remove from names map
            auto nameIt = std::find_if(m_elementNames.begin(), m_elementNames.end(),
                                      [elementId](const auto& pair) {
                                          return pair.second == elementId;
                                      });
            if (nameIt != m_elementNames.end()) {
                m_elementNames.erase(nameIt);
            }

            // Clear focus if this element had it
            if (m_focusedElement == elementId) {
                ClearFocus();
            }

            // Shutdown element
            element->Shutdown();

            VOXELCRAFT_DEBUG("Removed UI element: " + element->GetName() +
                            " (ID: " + std::to_string(elementId) + ")");
        }

        m_elements.erase(it);
        return true;
    }

    std::shared_ptr<UIElement> UIManager::GetElement(uint32_t elementId) const {
        std::shared_lock<std::shared_mutex> lock(m_elementsMutex);

        auto it = m_elements.find(elementId);
        return (it != m_elements.end()) ? it->second : nullptr;
    }

    std::shared_ptr<UIElement> UIManager::GetElement(const std::string& name) const {
        std::shared_lock<std::shared_mutex> lock(m_elementsMutex);

        auto it = m_elementNames.find(name);
        if (it != m_elementNames.end()) {
            auto elementIt = m_elements.find(it->second);
            return (elementIt != m_elements.end()) ? elementIt->second : nullptr;
        }

        return nullptr;
    }

    void UIManager::ClearElements() {
        std::unique_lock<std::shared_mutex> lock(m_elementsMutex);

        // Shutdown all elements
        for (auto& [id, element] : m_elements) {
            if (element) {
                element->Shutdown();
            }
        }

        m_elements.clear();
        m_elementNames.clear();
        m_visibleElements.clear();
        m_focusedElement = 0;

        VOXELCRAFT_DEBUG("Cleared all UI elements");
    }

    bool UIManager::HandleMouseInput(const glm::vec2& position, int button, int action, int mods) {
        if (!m_isInitialized) {
            return false;
        }

        ++m_metrics.inputEvents;
        ++m_metrics.mouseEvents;

        // Find element at position
        uint32_t targetElement = GetElementAtPosition(position);
        if (targetElement == 0) {
            return false;
        }

        auto element = GetElement(targetElement);
        if (!element || !element->IsEnabled()) {
            return false;
        }

        try {
            // Convert to local coordinates
            UIRect screenBounds = element->GetScreenBounds();
            glm::vec2 localPos = position - glm::vec2(screenBounds.x, screenBounds.y);

            bool handled = element->HandleMouseInput(localPos, button, action, mods);

            if (handled) {
                // Handle focus
                if (action == 1 && element->IsFocusable()) { // Mouse press
                    SetFocus(targetElement);
                }
            }

            return handled;

        } catch (const std::exception& e) {
            VOXELCRAFT_ERROR("Exception in HandleMouseInput: " + std::string(e.what()));
            return false;
        }
    }

    bool UIManager::HandleKeyboardInput(int key, int scancode, int action, int mods) {
        if (!m_isInitialized) {
            return false;
        }

        ++m_metrics.inputEvents;
        ++m_metrics.keyboardEvents;

        // Handle focused element first
        if (m_focusedElement != 0) {
            auto element = GetElement(m_focusedElement);
            if (element && element->IsEnabled()) {
                try {
                    bool handled = element->HandleKeyboardInput(key, scancode, action, mods);
                    if (handled) {
                        return true;
                    }
                } catch (const std::exception& e) {
                    VOXELCRAFT_ERROR("Exception in HandleKeyboardInput: " + std::string(e.what()));
                    return false;
                }
            }
        }

        // Handle global keyboard shortcuts
        return HandleGlobalKeyboardInput(key, scancode, action, mods);
    }

    bool UIManager::HandleTextInput(const std::string& text) {
        if (!m_isInitialized) {
            return false;
        }

        ++m_metrics.inputEvents;

        // Handle focused element
        if (m_focusedElement != 0) {
            auto element = GetElement(m_focusedElement);
            if (element && element->IsEnabled()) {
                try {
                    return element->HandleTextInput(text);
                } catch (const std::exception& e) {
                    VOXELCRAFT_ERROR("Exception in HandleTextInput: " + std::string(e.what()));
                    return false;
                }
            }
        }

        return false;
    }

    bool UIManager::HandleTouchInput(const glm::vec2& position, int action) {
        if (!m_isInitialized) {
            return false;
        }

        ++m_metrics.inputEvents;
        ++m_metrics.touchEvents;

        // Convert touch to mouse input for now
        return HandleMouseInput(position, 0, action, 0);
    }

    bool UIManager::HandleGlobalKeyboardInput(int key, int scancode, int action, int mods) {
        // Handle global shortcuts
        if (action == 1) { // Key press
            switch (key) {
                case 256: // Escape
                    if (m_mode == UIMode::Inventory) {
                        HideInventory();
                    } else if (m_mode == UIMode::Chat) {
                        HideChat();
                    } else if (m_mode == UIMode::Settings) {
                        HideSettings();
                    } else {
                        ShowPauseMenu();
                    }
                    return true;

                case 67: // F8 - Toggle debug info
                    if (m_hud) {
                        m_hud->ToggleDebugInfo();
                    }
                    return true;

                case 69: // E - Toggle inventory
                    ToggleInventory();
                    return true;

                case 84: // T - Toggle chat
                    ToggleChat();
                    return true;

                case 70: // F - Toggle HUD
                    if (m_hud) {
                        m_hud->ToggleHUD();
                    }
                    return true;
            }
        }

        return false;
    }

    uint32_t UIManager::GetElementAtPosition(const glm::vec2& position) const {
        std::shared_lock<std::shared_mutex> lock(m_elementsMutex);

        // Check in reverse order (top to bottom)
        for (auto it = m_elements.rbegin(); it != m_elements.rend(); ++it) {
            const auto& element = it->second;
            if (element && element->IsVisible() && element->ContainsPoint(position)) {
                return element->GetId();
            }
        }

        return 0;
    }

    bool UIManager::SetFocus(uint32_t elementId) {
        if (m_focusedElement == elementId) {
            return true;
        }

        // Clear current focus
        if (m_focusedElement != 0) {
            auto currentElement = GetElement(m_focusedElement);
            if (currentElement) {
                currentElement->SetFocus(false);
            }
        }

        // Set new focus
        auto newElement = GetElement(elementId);
        if (newElement && newElement->IsFocusable()) {
            newElement->SetFocus(true);
            m_focusedElement = elementId;

            VOXELCRAFT_DEBUG("Focus set to element: " + newElement->GetName() +
                            " (ID: " + std::to_string(elementId) + ")");
            return true;
        }

        m_focusedElement = 0;
        return false;
    }

    void UIManager::ClearFocus() {
        if (m_focusedElement != 0) {
            auto element = GetElement(m_focusedElement);
            if (element) {
                element->SetFocus(false);
                VOXELCRAFT_DEBUG("Focus cleared from element: " + element->GetName());
            }
            m_focusedElement = 0;
        }
    }

    bool UIManager::SetMode(UIMode mode) {
        if (mode == m_mode) {
            return true;
        }

        VOXELCRAFT_INFO("Changing UI mode from " + std::to_string(static_cast<int>(m_mode)) +
                       " to " + std::to_string(static_cast<int>(mode)));

        UIMode oldMode = m_mode;
        m_mode = mode;

        // Handle mode transitions
        switch (mode) {
            case UIMode::Game:
                SwitchToGameMode();
                break;
            case UIMode::Menu:
                SwitchToMenuMode();
                break;
            case UIMode::Inventory:
                SwitchToInventoryMode();
                break;
            case UIMode::Chat:
                SwitchToChatMode();
                break;
            case UIMode::Settings:
                SwitchToSettingsMode();
                break;
            default:
                break;
        }

        return true;
    }

    void UIManager::SwitchToGameMode() {
        // Hide all overlays
        if (m_inventoryUI) m_inventoryUI->Hide();
        if (m_chatSystem) m_chatSystem->Hide();
        if (m_settingsUI) m_settingsUI->Hide();
        if (m_menuSystem) m_menuSystem->HideCurrentMenu();

        // Show HUD
        if (m_hud) m_hud->ShowHUD();
    }

    void UIManager::SwitchToMenuMode() {
        // Hide game elements
        if (m_hud) m_hud->HideHUD();
        if (m_inventoryUI) m_inventoryUI->Hide();
        if (m_chatSystem) m_chatSystem->Hide();
    }

    void UIManager::SwitchToInventoryMode() {
        if (m_inventoryUI) {
            m_inventoryUI->Show();
        }
        SetMode(UIMode::Inventory);
    }

    void UIManager::SwitchToChatMode() {
        if (m_chatSystem) {
            m_chatSystem->Show();
        }
        SetMode(UIMode::Chat);
    }

    void UIManager::SwitchToSettingsMode() {
        if (m_settingsUI) {
            m_settingsUI->Show();
        }
        SetMode(UIMode::Settings);
    }

    // HUD Management
    void UIManager::ShowHUD() {
        if (m_hud) {
            m_hud->SetVisible(true);
        }
    }

    void UIManager::HideHUD() {
        if (m_hud) {
            m_hud->SetVisible(false);
        }
    }

    void UIManager::ToggleHUD() {
        if (m_hud) {
            m_hud->SetVisible(!m_hud->IsVisible());
        }
    }

    void UIManager::UpdateHUD(Player* player) {
        if (m_hud) {
            m_hud->UpdatePlayerInfo(player);
        }
    }

    // Menu Management
    void UIManager::ShowMainMenu() {
        if (m_menuSystem) {
            m_menuSystem->ShowMainMenu();
        }
        SetMode(UIMode::Menu);
    }

    void UIManager::ShowPauseMenu() {
        if (m_menuSystem) {
            m_menuSystem->ShowPauseMenu();
        }
        SetMode(UIMode::Menu);
    }

    void UIManager::ShowOptionsMenu() {
        if (m_menuSystem) {
            m_menuSystem->ShowOptionsMenu();
        }
        SetMode(UIMode::Menu);
    }

    void UIManager::HideCurrentMenu() {
        if (m_menuSystem) {
            m_menuSystem->HideCurrentMenu();
        }
        SetMode(UIMode::Game);
    }

    // Inventory Management
    void UIManager::ShowInventory(Inventory* inventory) {
        if (m_inventoryUI) {
            m_inventoryUI->ShowInventory(inventory);
        }
        SetMode(UIMode::Inventory);
    }

    void UIManager::HideInventory() {
        if (m_inventoryUI) {
            m_inventoryUI->Hide();
        }
        SetMode(UIMode::Game);
    }

    void UIManager::ToggleInventory() {
        if (m_mode == UIMode::Inventory) {
            HideInventory();
        } else {
            // Show current player's inventory
            if (m_player && m_player->GetInventory()) {
                ShowInventory(m_player->GetInventory());
            }
        }
    }

    // Chat Management
    void UIManager::ShowChat() {
        if (m_chatSystem) {
            m_chatSystem->Show();
        }
        SetMode(UIMode::Chat);
    }

    void UIManager::HideChat() {
        if (m_chatSystem) {
            m_chatSystem->Hide();
        }
        SetMode(UIMode::Game);
    }

    void UIManager::ToggleChat() {
        if (m_mode == UIMode::Chat) {
            HideChat();
        } else {
            ShowChat();
        }
    }

    void UIManager::AddChatMessage(const std::string& message, const std::string& sender,
                                  const glm::vec4& color) {
        if (m_chatSystem) {
            m_chatSystem->AddMessage(message, sender, color);
        }
    }

    // Settings Management
    void UIManager::ShowSettings() {
        if (m_settingsUI) {
            m_settingsUI->Show();
        }
        SetMode(UIMode::Settings);
    }

    void UIManager::HideSettings() {
        if (m_settingsUI) {
            m_settingsUI->Hide();
        }
        SetMode(UIMode::Game);
    }

    void UIManager::ApplySettings(const std::unordered_map<std::string, std::any>& settings) {
        // Apply UI-related settings
        for (const auto& [key, value] : settings) {
            if (key == "ui_scale") {
                try {
                    m_config.uiScale = std::any_cast<float>(value);
                } catch (const std::bad_any_cast&) {
                    VOXELCRAFT_WARNING("Invalid UI scale value");
                }
            } else if (key == "ui_opacity") {
                try {
                    m_config.globalOpacity = std::any_cast<float>(value);
                } catch (const std::bad_any_cast&) {
                    VOXELCRAFT_WARNING("Invalid UI opacity value");
                }
            }
        }

        // Update subsystems
        if (m_hud) {
            m_hud->SetConfig(m_config);
        }

        VOXELCRAFT_INFO("UI settings applied");
    }

    // Animation System
    uint32_t UIManager::StartAnimation(uint32_t elementId, UIAnimationType animationType,
                                      float duration, const std::unordered_map<std::string, float>& properties) {
        auto element = GetElement(elementId);
        if (!element) {
            return 0;
        }

        uint32_t animationId = s_nextAnimationId++;
        std::string animationTypeStr;

        // Convert animation type to string
        switch (animationType) {
            case UIAnimationType::Fade: animationTypeStr = "fade"; break;
            case UIAnimationType::Slide: animationTypeStr = "slide"; break;
            case UIAnimationType::Scale: animationTypeStr = "scale"; break;
            case UIAnimationType::Rotate: animationTypeStr = "rotate"; break;
            case UIAnimationType::Bounce: animationTypeStr = "bounce"; break;
            case UIAnimationType::Elastic: animationTypeStr = "elastic"; break;
            default: animationTypeStr = "custom"; break;
        }

        // Start animation on element
        element->StartAnimation(animationTypeStr, duration, properties);

        VOXELCRAFT_DEBUG("Started animation " + std::to_string(animationId) +
                        " on element " + element->GetName());
        return animationId;
    }

    bool UIManager::StopAnimation(uint32_t animationId) {
        // Find element with this animation
        std::shared_lock<std::shared_mutex> lock(m_elementsMutex);

        for (const auto& [id, element] : m_elements) {
            if (element) {
                // This would need to be implemented in UIElement to track animation IDs
                // For now, just stop all animations on all elements
                element->StopAnimation(animationId);
            }
        }

        return true;
    }

    bool UIManager::PauseAnimation(uint32_t animationId) {
        // Similar to StopAnimation - would need element-level tracking
        return false;
    }

    bool UIManager::ResumeAnimation(uint32_t animationId) {
        // Similar to StopAnimation - would need element-level tracking
        return false;
    }

    // Event System
    uint32_t UIManager::AddEventListener(UIEvent::Type eventType,
                                        std::function<void(const UIEvent&)> listener) {
        uint32_t listenerId = s_nextListenerId++;
        m_eventListeners[listenerId] = listener;
        return listenerId;
    }

    bool UIManager::RemoveEventListener(uint32_t listenerId) {
        auto it = m_eventListeners.find(listenerId);
        if (it != m_eventListeners.end()) {
            m_eventListeners.erase(it);
            return true;
        }
        return false;
    }

    void UIManager::TriggerEvent(const UIEvent& event) {
        // Add to event queue
        {
            std::unique_lock<std::shared_mutex> lock(m_eventsMutex);
            m_eventQueue.push(event);
        }
    }

    void UIManager::ProcessEventQueue() {
        std::queue<UIEvent> eventsToProcess;

        // Get events to process
        {
            std::unique_lock<std::shared_mutex> lock(m_eventsMutex);
            eventsToProcess = std::move(m_eventQueue);
            m_eventQueue = std::queue<UIEvent>();
        }

        // Process events
        while (!eventsToProcess.empty()) {
            const UIEvent& event = eventsToProcess.front();

            // Send to listeners
            for (const auto& [id, listener] : m_eventListeners) {
                try {
                    listener(event);
                } catch (const std::exception& e) {
                    VOXELCRAFT_ERROR("Exception in UI event listener: " + std::string(e.what()));
                }
            }

            // Send to element
            if (event.elementId != 0) {
                auto element = GetElement(event.elementId);
                if (element) {
                    element->TriggerEvent(event);
                }
            }

            eventsToProcess.pop();
        }
    }

    // Layout System
    uint32_t UIManager::CreateLayout(const std::string& name, const std::string& type) {
        // This would create a layout system - simplified for now
        VOXELCRAFT_INFO("Layout creation not implemented yet: " + name);
        return 0;
    }

    bool UIManager::ApplyLayout(uint32_t layoutId, const std::vector<uint32_t>& elementIds) {
        // This would apply a layout to elements - simplified for now
        VOXELCRAFT_INFO("Layout application not implemented yet: " + std::to_string(layoutId));
        return false;
    }

    void UIManager::UpdateLayouts(const glm::vec2& screenSize) {
        // Update element positions based on screen size and anchors
        std::shared_lock<std::shared_mutex> lock(m_elementsMutex);

        for (const auto& [id, element] : m_elements) {
            if (element) {
                // This would update element positions based on anchors
                // For now, just mark as needing update
                element->Invalidate();
            }
        }
    }

    // Style System
    bool UIManager::LoadStyleSheet(const std::string& styleSheet) {
        VOXELCRAFT_INFO("Style sheet loading not implemented yet: " + styleSheet);
        return false;
    }

    bool UIManager::ApplyStyle(uint32_t elementId, const std::string& styleName) {
        auto element = GetElement(elementId);
        if (!element) {
            return false;
        }

        // This would apply a style to the element
        VOXELCRAFT_INFO("Style application not implemented yet: " + styleName +
                       " to element " + element->GetName());
        return false;
    }

    bool UIManager::SetTheme(const std::string& themeName) {
        VOXELCRAFT_INFO("Theme setting not implemented yet: " + themeName);
        return false;
    }

    // Configuration
    void UIManager::SetConfig(const UIManagerConfig& config) {
        m_config = config;

        // Update subsystems
        if (m_hud) {
            HUDConfig hudConfig = m_hud->GetConfig();
            hudConfig.globalScale = m_config.uiScale;
            hudConfig.globalOpacity = m_config.globalOpacity;
            m_hud->SetConfig(hudConfig);
        }

        VOXELCRAFT_INFO("UI configuration updated");
    }

    // Metrics and Monitoring
    void UIManager::UpdateMetrics(double deltaTime) {
        ++m_metrics.updateCount;

        // Calculate average update time
        if (m_metrics.updateCount > 1) {
            m_metrics.averageUpdateTime = (m_metrics.averageUpdateTime +
                                          deltaTime) / 2.0;
        } else {
            m_metrics.averageUpdateTime = deltaTime;
        }

        m_metrics.maxUpdateTime = std::max(m_metrics.maxUpdateTime, deltaTime);

        // Update element counts
        {
            std::shared_lock<std::shared_mutex> lock(m_elementsMutex);
            m_metrics.activeElements = m_elements.size();
            m_metrics.visibleElements = std::count_if(m_elements.begin(), m_elements.end(),
                [](const auto& pair) {
                    return pair.second && pair.second->IsVisible();
                });
        }

        // Reset metrics periodically
        if (m_metrics.updateCount > 10000) {
            m_metrics.updateCount = 1;
            m_metrics.totalUpdateTime = 0.0;
        }
    }

    void UIManager::ResetMetrics() {
        m_metrics = {};
        VOXELCRAFT_DEBUG("UI metrics reset");
    }

    std::string UIManager::GetPerformanceReport() const {
        std::stringstream ss;

        ss << "=== UIManager Performance Report ===\n";
        ss << "Updates: " << m_metrics.updateCount << "\n";
        ss << "Average Update Time: " << m_metrics.averageUpdateTime * 1000.0 << "ms\n";
        ss << "Max Update Time: " << m_metrics.maxUpdateTime * 1000.0 << "ms\n";
        ss << "Active Elements: " << m_metrics.activeElements << "\n";
        ss << "Visible Elements: " << m_metrics.visibleElements << "\n";
        ss << "Input Events: " << m_metrics.inputEvents << "\n";
        ss << "Draw Calls: " << m_metrics.drawCalls << "\n";
        ss << "Memory Usage: " << m_metrics.memoryUsage / 1024.0 << "KB\n";

        return ss.str();
    }

    // Utility Functions
    glm::vec2 UIManager::ScreenToUICoordinates(const glm::vec2& screenPos) const {
        // Convert screen coordinates to UI coordinates (0,0 = top-left)
        if (!m_window) {
            return screenPos;
        }

        // This would get actual window size and convert coordinates
        // For now, return as-is
        return screenPos;
    }

    glm::vec2 UIManager::UIToScreenCoordinates(const glm::vec2& uiPos) const {
        // Convert UI coordinates to screen coordinates
        if (!m_window) {
            return uiPos;
        }

        // This would get actual window size and convert coordinates
        // For now, return as-is
        return uiPos;
    }

    // Error Handling
    void UIManager::HandleError(const std::string& error) {
        VOXELCRAFT_ERROR("UIManager Error: " + error);
        m_state = UIState::Error;
    }

    // Validation
    bool UIManager::Validate() const {
        if (!m_isInitialized) {
            return false;
        }

        // Validate subsystems
        if (m_hud && !m_hud->Validate()) {
            return false;
        }

        // Validate elements
        {
            std::shared_lock<std::shared_mutex> lock(m_elementsMutex);
            for (const auto& [id, element] : m_elements) {
                if (element && !element->Validate()) {
                    return false;
                }
            }
        }

        return true;
    }

    std::string UIManager::GetStatusReport() const {
        std::stringstream ss;

        ss << "=== UIManager Status Report ===\n";
        ss << "State: " << static_cast<int>(m_state) << "\n";
        ss << "Mode: " << static_cast<int>(m_mode) << "\n";
        ss << "Initialized: " << (m_isInitialized ? "Yes" : "No") << "\n";
        ss << "Active Elements: " << m_metrics.activeElements << "\n";
        ss << "Visible Elements: " << m_metrics.visibleElements << "\n";
        ss << "Focused Element: " << m_focusedElement << "\n";

        if (m_hud) {
            ss << "HUD: Active\n";
        } else {
            ss << "HUD: Inactive\n";
        }

        return ss.str();
    }

    std::unordered_map<std::string, size_t> UIManager::Optimize() {
        std::unordered_map<std::string, size_t> optimizations;

        // Remove invalid elements
        {
            std::unique_lock<std::shared_mutex> lock(m_elementsMutex);
            auto it = m_elements.begin();
            while (it != m_elements.end()) {
                if (!it->second || !it->second->Validate()) {
                    it = m_elements.erase(it);
                    ++optimizations["invalid_elements_removed"];
                } else {
                    ++it;
                }
            }
        }

        // Optimize visible elements list
        {
            std::shared_lock<std::shared_mutex> lock(m_elementsMutex);
            m_visibleElements.clear();
            for (const auto& [id, element] : m_elements) {
                if (element && element->IsVisible()) {
                    m_visibleElements.push_back(id);
                }
            }
            optimizations["visible_elements_optimized"] = m_visibleElements.size();
        }

        VOXELCRAFT_INFO("UI optimization completed: " +
                       std::to_string(optimizations.size()) + " optimizations applied");
        return optimizations;
    }

    void UIManager::CreateDefaultElements() {
        // This would create default UI elements like tooltips, cursors, etc.
        VOXELCRAFT_DEBUG("Default UI elements created");
    }

} // namespace VoxelCraft
