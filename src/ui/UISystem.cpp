/**
 * @file UISystem.cpp
 * @brief VoxelCraft UI System Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#include "UISystem.hpp"
#include "../core/Logger.hpp"
#include "../graphics/Renderer.hpp"
#include "../input/InputManager.hpp"
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace VoxelCraft {

// UI Element base class forward declaration
class UIElement {
public:
    UIElement(const std::string& id, UIElementType type);
    virtual ~UIElement() = default;

    virtual void Update(float deltaTime);
    virtual void Render();
    virtual bool HandleInput(const UIEvent& event);
    virtual UIRect GetBounds() const;

    // Properties
    std::string GetId() const { return m_id; }
    UIElementType GetType() const { return m_type; }
    UIState GetState() const { return m_state; }
    bool IsVisible() const { return m_visible; }
    bool IsEnabled() const { return m_enabled; }
    int GetLayer() const { return m_layer; }

    void SetVisible(bool visible) { m_visible = visible; }
    void SetEnabled(bool enabled) { m_enabled = enabled; }
    void SetPosition(const Vec2& position) { m_position = position; }
    void SetSize(const Vec2& size) { m_size = size; }
    void SetStyle(const UIStyle& style) { m_style = style; }

    Vec2 GetPosition() const { return m_position; }
    Vec2 GetSize() const { return m_size; }
    const UIStyle& GetStyle() const { return m_style; }

    // Event handling
    using UIEventCallback = std::function<void(const UIEvent&)>;
    void AddEventListener(UIEventType type, UIEventCallback callback);
    void RemoveEventListener(UIEventType type);

    // Animation
    void StartAnimation(const std::string& property, float targetValue, float duration);
    bool IsAnimating(const std::string& property) const;

    // Hierarchy
    void AddChild(std::shared_ptr<UIElement> child);
    void RemoveChild(const std::string& childId);
    std::shared_ptr<UIElement> GetChild(const std::string& childId);
    const std::vector<std::shared_ptr<UIElement>>& GetChildren() const { return m_children; }

protected:
    std::string m_id;
    UIElementType m_type;
    UIState m_state = UIState::NORMAL;
    Vec2 m_position;
    Vec2 m_size;
    UIStyle m_style;
    bool m_visible = true;
    bool m_enabled = true;
    int m_layer = 0;

    // Hierarchy
    std::weak_ptr<UIElement> m_parent;
    std::vector<std::shared_ptr<UIElement>> m_children;

    // Event handling
    std::unordered_map<UIEventType, std::vector<UIEventCallback>> m_eventListeners;

    // Animation system
    struct Animation {
        std::string property;
        float startValue;
        float targetValue;
        float currentValue;
        float duration;
        float elapsedTime;
        bool active;
    };
    std::vector<Animation> m_animations;

    virtual void OnStateChanged(UIState oldState, UIState newState) {}
    virtual void OnStyleChanged() {}
    virtual void OnBoundsChanged() {}

    void SetState(UIState state);
    void UpdateAnimations(float deltaTime);
    void FireEvent(const UIEvent& event);
};

// Simple Vec4 implementation for colors
struct Vec4 {
    float x, y, z, w;
    Vec4(float x = 0.0f, float y = 0.0f, float z = 0.0f, float w = 1.0f) : x(x), y(y), z(z), w(w) {}
};

// Forward declarations for UI components
class UIManager;
class UIRenderer;
class UIInputHandler;
class UILayoutManager;
class UIStyleManager;

// Simple implementations for now - will be expanded
class UIManager {
public:
    std::shared_ptr<UIElement> CreateElement(UIElementType type, const std::string& id) {
        return std::make_shared<UIElement>(id, type);
    }
    std::shared_ptr<UIElement> GetElement(const std::string& id) {
        auto it = m_elements.find(id);
        return it != m_elements.end() ? it->second : nullptr;
    }
    void DestroyElement(const std::string& id) {
        m_elements.erase(id);
    }
private:
    std::unordered_map<std::string, std::shared_ptr<UIElement>> m_elements;
};

class UIRenderer {
public:
    void RenderElement(const std::shared_ptr<UIElement>& element) {
        // Placeholder rendering
    }
};

class UIInputHandler {
public:
    bool ProcessInput(const UIEvent& event) {
        return false;
    }
};

class UILayoutManager {
public:
    void UpdateLayout() {
        // Placeholder layout update
    }
};

class UIStyleManager {
public:
    void ApplyStyle(UIElement* element, const UIStyle& style) {
        // Placeholder style application
    }
};

// UIElement implementation
UIElement::UIElement(const std::string& id, UIElementType type)
    : m_id(id), m_type(type) {
}

void UIElement::Update(float deltaTime) {
    UpdateAnimations(deltaTime);
    for (auto& child : m_children) {
        child->Update(deltaTime);
    }
}

void UIElement::Render() {
    if (!m_visible) return;

    // Render self (placeholder)
    for (auto& child : m_children) {
        child->Render();
    }
}

bool UIElement::HandleInput(const UIEvent& event) {
    // Handle input for children first (reverse order for proper layering)
    for (auto it = m_children.rbegin(); it != m_children.rend(); ++it) {
        if ((*it)->HandleInput(event)) {
            return true;
        }
    }

    // Handle input for self
    UIRect bounds = GetBounds();
    if (bounds.Contains(event.position.x, event.position.y)) {
        FireEvent(event);
        return true;
    }

    return false;
}

UIRect UIElement::GetBounds() const {
    return UIRect(m_position.x, m_position.y, m_size.x, m_size.y);
}

void UIElement::AddEventListener(UIEventType type, UIEventCallback callback) {
    m_eventListeners[type].push_back(callback);
}

void UIElement::RemoveEventListener(UIEventType type) {
    m_eventListeners.erase(type);
}

void UIElement::StartAnimation(const std::string& property, float targetValue, float duration) {
    Animation anim;
    anim.property = property;
    anim.targetValue = targetValue;
    anim.duration = duration;
    anim.elapsedTime = 0.0f;
    anim.active = true;

    // Get current value based on property
    if (property == "x") anim.startValue = m_position.x;
    else if (property == "y") anim.startValue = m_position.y;
    else if (property == "width") anim.startValue = m_size.x;
    else if (property == "height") anim.startValue = m_size.y;
    else if (property == "opacity") anim.startValue = m_style.opacity;
    else anim.startValue = 0.0f;

    anim.currentValue = anim.startValue;
    m_animations.push_back(anim);
}

bool UIElement::IsAnimating(const std::string& property) const {
    for (const auto& anim : m_animations) {
        if (anim.property == property && anim.active) {
            return true;
        }
    }
    return false;
}

void UIElement::AddChild(std::shared_ptr<UIElement> child) {
    child->m_parent = shared_from_this();
    m_children.push_back(child);
}

void UIElement::RemoveChild(const std::string& childId) {
    m_children.erase(
        std::remove_if(m_children.begin(), m_children.end(),
            [&childId](const std::shared_ptr<UIElement>& child) {
                return child->GetId() == childId;
            }),
        m_children.end());
}

std::shared_ptr<UIElement> UIElement::GetChild(const std::string& childId) {
    for (auto& child : m_children) {
        if (child->GetId() == childId) {
            return child;
        }
    }
    return nullptr;
}

void UIElement::SetState(UIState state) {
    if (m_state != state) {
        UIState oldState = m_state;
        m_state = state;
        OnStateChanged(oldState, state);
    }
}

void UIElement::UpdateAnimations(float deltaTime) {
    for (auto& anim : m_animations) {
        if (!anim.active) continue;

        anim.elapsedTime += deltaTime;
        float progress = std::min(anim.elapsedTime / anim.duration, 1.0f);

        // Simple linear interpolation
        anim.currentValue = anim.startValue + (anim.targetValue - anim.startValue) * progress;

        // Apply animation value
        if (anim.property == "x") m_position.x = anim.currentValue;
        else if (anim.property == "y") m_position.y = anim.currentValue;
        else if (anim.property == "width") m_size.x = anim.currentValue;
        else if (anim.property == "height") m_size.y = anim.currentValue;
        else if (anim.property == "opacity") m_style.opacity = anim.currentValue;

        if (progress >= 1.0f) {
            anim.active = false;
        }
    }

    // Remove completed animations
    m_animations.erase(
        std::remove_if(m_animations.begin(), m_animations.end(),
            [](const Animation& anim) { return !anim.active; }),
        m_animations.end());
}

void UIElement::FireEvent(const UIEvent& event) {
    auto it = m_eventListeners.find(event.type);
    if (it != m_eventListeners.end()) {
        for (auto& callback : it->second) {
            callback(event);
        }
    }
}

// UISystem implementation
UISystem& UISystem::GetInstance() {
    static UISystem instance;
    return instance;
}

UISystem::~UISystem() {
    Shutdown();
}

bool UISystem::Initialize(const UIConfig& config) {
    if (m_initialized) {
        return true;
    }

    VOXELCRAFT_INFO("Initializing UI System...");

    m_config = config;

    // Initialize core components
    m_uiManager = std::make_unique<UIManager>();
    m_renderer = std::make_unique<UIRenderer>();
    m_inputHandler = std::make_unique<UIInputHandler>();
    m_layoutManager = std::make_unique<UILayoutManager>();
    m_styleManager = std::make_unique<UIStyleManager>();

    // Initialize default styles and elements
    InitializeDefaultStyles();
    InitializeDefaultElements();

    m_initialized = true;
    VOXELCRAFT_INFO("UI System initialized successfully");
    return true;
}

void UISystem::Shutdown() {
    if (!m_initialized) {
        return;
    }

    VOXELCRAFT_INFO("Shutting down UI System...");

    m_eventListeners.clear();
    m_modalStack.clear();
    m_focusedElement.clear();
    m_activeModal.clear();

    m_styleManager.reset();
    m_layoutManager.reset();
    m_inputHandler.reset();
    m_renderer.reset();
    m_uiManager.reset();

    m_initialized = false;
    VOXELCRAFT_INFO("UI System shutdown complete");
}

void UISystem::Update(float deltaTime) {
    if (!m_initialized) return;

    m_frameCount++;
    m_totalFrameTime += deltaTime;

    // Update animations
    UpdateAnimations(deltaTime);

    // Process pending events
    ProcessEvents();

    // Update layout if needed
    UpdateLayout();

    // Update performance stats
    if (m_frameCount % 60 == 0) {
        float avgFrameTime = m_totalFrameTime / 60.0f;
        m_totalFrameTime = 0.0f;
        // Could log performance stats here
    }
}

void UISystem::Render() {
    if (!m_initialized) return;

    // Render all elements
    // This would iterate through all UI elements and render them

    if (m_debugMode) {
        RenderDebugOverlay();
    }
}

bool UISystem::HandleInput(const UIEvent& event) {
    if (!m_initialized) return false;

    // Update mouse position
    if (event.type == UIEventType::DRAG_MOVE || event.type == UIEventType::CLICK) {
        m_config.mousePosition = event.position;
    }

    // Handle modal dialogs first
    if (IsModalActive()) {
        auto modal = GetElement(m_activeModal);
        if (modal && modal->HandleInput(event)) {
            return true;
        }
        return true; // Modal blocks all other input
    }

    // Handle focused element
    if (!m_focusedElement.empty()) {
        auto focused = GetElement(m_focusedElement);
        if (focused && focused->HandleInput(event)) {
            return true;
        }
    }

    // Handle all elements (in reverse layer order for proper hit testing)
    // This would iterate through elements by layer and handle input

    return false;
}

std::shared_ptr<UIElement> UISystem::CreateElement(UIElementType type, const std::string& id) {
    if (!m_initialized) return nullptr;

    if (!ValidateElementId(id)) {
        VOXELCRAFT_WARN("Invalid element ID: {}", id);
        return nullptr;
    }

    if (GetElement(id)) {
        VOXELCRAFT_WARN("Element with ID '{}' already exists", id);
        return nullptr;
    }

    auto element = m_uiManager->CreateElement(type, id);
    if (element) {
        m_peakElementCount = std::max(m_peakElementCount.load(), GetElementCount() + 1);
    }

    return element;
}

std::shared_ptr<UIElement> UISystem::GetElement(const std::string& id) {
    return m_uiManager ? m_uiManager->GetElement(id) : nullptr;
}

void UISystem::DestroyElement(const std::string& id) {
    if (m_uiManager) {
        // Clear focus if this element is focused
        if (m_focusedElement == id) {
            ClearFocus();
        }

        // Hide modal if this is the active modal
        if (m_activeModal == id) {
            m_activeModal.clear();
        }

        m_uiManager->DestroyElement(id);
    }
}

void UISystem::DestroyElement(std::shared_ptr<UIElement> element) {
    if (element) {
        DestroyElement(element->GetId());
    }
}

std::vector<std::shared_ptr<UIElement>> UISystem::GetElementsByType(UIElementType type) {
    std::vector<std::shared_ptr<UIElement>> result;
    // This would iterate through all elements and collect by type
    return result;
}

std::vector<std::shared_ptr<UIElement>> UISystem::GetElementsByLayer(int layer) {
    std::vector<std::shared_ptr<UIElement>> result;
    // This would iterate through all elements and collect by layer
    return result;
}

void UISystem::ShowModal(const std::string& modalId) {
    auto element = GetElement(modalId);
    if (element && element->GetType() == UIElementType::MODAL) {
        if (!IsModalActive()) {
            m_modalStack.push_back(modalId);
        }
        m_activeModal = modalId;
        element->SetVisible(true);
    }
}

void UISystem::HideModal(const std::string& modalId) {
    if (m_activeModal == modalId) {
        auto element = GetElement(modalId);
        if (element) {
            element->SetVisible(false);
        }

        // Remove from stack
        m_modalStack.erase(
            std::remove(m_modalStack.begin(), m_modalStack.end(), modalId),
            m_modalStack.end());

        // Set new active modal
        if (!m_modalStack.empty()) {
            m_activeModal = m_modalStack.back();
        } else {
            m_activeModal.clear();
        }
    }
}

bool UISystem::IsModalActive() const {
    return !m_activeModal.empty();
}

std::string UISystem::GetActiveModal() const {
    return m_activeModal;
}

void UISystem::SetFocus(const std::string& elementId) {
    if (m_focusedElement != elementId) {
        // Remove focus from previous element
        if (!m_focusedElement.empty()) {
            auto prev = GetElement(m_focusedElement);
            if (prev) {
                UIEvent blurEvent{UIEventType::BLUR, m_focusedElement};
                prev->HandleInput(blurEvent);
            }
        }

        m_focusedElement = elementId;

        // Set focus on new element
        if (!elementId.empty()) {
            auto element = GetElement(elementId);
            if (element) {
                UIEvent focusEvent{UIEventType::FOCUS, elementId};
                element->HandleInput(focusEvent);
            }
        }
    }
}

std::string UISystem::GetFocusedElement() const {
    return m_focusedElement;
}

void UISystem::ClearFocus() {
    SetFocus("");
}

void UISystem::UpdateLayout() {
    if (m_layoutManager) {
        m_layoutManager->UpdateLayout();
    }
}

void UISystem::SetScreenSize(const Vec2& size) {
    m_config.screenSize = size;
    UpdateLayout();
}

void UISystem::SetScale(float scale) {
    m_config.scale = scale;
    UpdateLayout();
}

void UISystem::SetGlobalStyle(const UIStyle& style) {
    // This would apply the style to all elements
}

void UISystem::ApplyTheme(const std::string& themeName) {
    // This would load and apply a theme
}

void UISystem::SetElementStyle(const std::string& elementId, const UIStyle& style) {
    auto element = GetElement(elementId);
    if (element && m_styleManager) {
        m_styleManager->ApplyStyle(element.get(), style);
    }
}

void UISystem::EnableAccessibility(bool enable) {
    m_accessibilityEnabled = enable;
}

bool UISystem::IsAccessibilityEnabled() const {
    return m_accessibilityEnabled;
}

void UISystem::SetScreenReaderCallback(std::function<void(const std::string&)> callback) {
    m_screenReaderCallback = callback;
}

void UISystem::AnnounceText(const std::string& text, bool interrupt) {
    if (m_accessibilityEnabled && m_screenReaderCallback) {
        m_screenReaderCallback(text);
    }
}

void UISystem::PlayAnimation(const std::string& elementId, const std::string& animationName) {
    auto element = GetElement(elementId);
    if (element) {
        // This would start a predefined animation
    }
}

void UISystem::StopAnimation(const std::string& elementId) {
    auto element = GetElement(elementId);
    if (element) {
        // This would stop all animations on the element
    }
}

bool UISystem::IsAnimationPlaying(const std::string& elementId) const {
    auto element = GetElement(elementId);
    return element ? element->IsAnimating("any") : false;
}

void UISystem::SetMousePosition(const Vec2& position) {
    m_config.mousePosition = position;
}

void UISystem::EnableDebugMode(bool enable) {
    m_debugMode = enable;
}

std::string UISystem::GetDebugInfo() const {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2);
    ss << "UI System Debug Info:\n";
    ss << "  Elements: " << GetElementCount() << "\n";
    ss << "  Visible: " << GetVisibleElementCount() << "\n";
    ss << "  Animations: " << GetActiveAnimationCount() << "\n";
    ss << "  Focus: " << (m_focusedElement.empty() ? "None" : m_focusedElement) << "\n";
    ss << "  Modal: " << (m_activeModal.empty() ? "None" : m_activeModal) << "\n";
    ss << "  Frame Time: " << GetAverageFrameTime() << "ms\n";
    return ss.str();
}

void UISystem::HighlightElement(const std::string& elementId, bool highlight) {
    auto element = GetElement(elementId);
    if (element) {
        // This would add a highlight effect to the element
    }
}

void UISystem::AddEventListener(const std::string& eventType, UIEventCallback callback) {
    std::unique_lock<std::shared_mutex> lock(m_eventMutex);
    m_eventListeners[eventType].push_back(callback);
}

void UISystem::RemoveEventListener(const std::string& eventType) {
    std::unique_lock<std::shared_mutex> lock(m_eventMutex);
    m_eventListeners.erase(eventType);
}

size_t UISystem::GetElementCount() const {
    return m_uiManager ? 0 : 0; // Would return actual count
}

size_t UISystem::GetVisibleElementCount() const {
    return 0; // Would count visible elements
}

size_t UISystem::GetActiveAnimationCount() const {
    return 0; // Would count active animations
}

float UISystem::GetAverageFrameTime() const {
    return m_frameCount > 0 ? (m_totalFrameTime / m_frameCount) * 1000.0f : 0.0f;
}

void UISystem::OptimizeMemoryUsage() {
    ClearUnusedElements();
}

void UISystem::ClearUnusedElements() {
    // This would remove elements that are no longer referenced
}

void UISystem::InitializeDefaultStyles() {
    // Initialize default UI styles
}

void UISystem::InitializeDefaultElements() {
    // Create default UI elements like crosshair, hotbar, etc.
}

void UISystem::ProcessEvents() {
    // Process pending UI events
}

void UISystem::UpdateAnimations(float deltaTime) {
    // Update all element animations
}

void UISystem::RenderDebugOverlay() {
    // Render debug information overlay
}

bool UISystem::ValidateElementId(const std::string& id) const {
    if (id.empty()) return false;
    if (id.length() > 256) return false;

    // Check for valid characters
    for (char c : id) {
        if (!std::isalnum(c) && c != '_' && c != '-') {
            return false;
        }
    }

    return true;
}

} // namespace VoxelCraft
