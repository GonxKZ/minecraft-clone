/**
 * @file UIElement.hpp
 * @brief VoxelCraft UI Element - Base UI Element Class
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines the UIElement class that serves as the base class for all
 * UI elements in the VoxelCraft game engine, providing common functionality
 * for positioning, sizing, rendering, input handling, and animation support.
 */

#ifndef VOXELCRAFT_UI_UI_ELEMENT_HPP
#define VOXELCRAFT_UI_UI_ELEMENT_HPP

#include <memory>
#include <vector>
#include <string>
#include <functional>
#include <unordered_map>
#include <any>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../core/Config.hpp"

namespace VoxelCraft {

    // Forward declarations
    class UIManager;
    class UIRenderer;
    class UIStyle;
    struct UIEvent;

    /**
     * @enum UIElementState
     * @brief UI element states
     */
    enum class UIElementState {
        Normal,                 ///< Normal state
        Hovered,                ///< Mouse is hovering over element
        Pressed,                ///< Element is being pressed
        Focused,                ///< Element has focus
        Disabled,               ///< Element is disabled
        Hidden,                 ///< Element is hidden
        Animating               ///< Element is animating
    };

    /**
     * @enum UIElementType
     * @brief Types of UI elements
     */
    enum class UIElementType {
        Container,              ///< Container element
        Button,                 ///< Button element
        Label,                  ///< Text label
        Image,                  ///< Image element
        TextBox,                ///< Text input box
        Slider,                 ///< Slider control
        Checkbox,               ///< Checkbox control
        RadioButton,            ///< Radio button control
        ListBox,                ///< List box control
        ComboBox,               ///< Combo box control
        ProgressBar,            ///< Progress bar
        ScrollBar,              ///< Scroll bar
        Panel,                  ///< Panel element
        TabControl,             ///< Tab control
        Menu,                   ///< Menu element
        MenuItem,               ///< Menu item
        Tooltip,                ///< Tooltip element
        Custom                  ///< Custom element
    };

    /**
     * @enum UIAnchor
     * @brief UI element anchoring options
     */
    enum class UIAnchor {
        TopLeft,                ///< Top-left corner
        TopCenter,              ///< Top center
        TopRight,               ///< Top-right corner
        MiddleLeft,             ///< Middle left
        MiddleCenter,           ///< Center
        MiddleRight,            ///< Middle right
        BottomLeft,             ///< Bottom-left corner
        BottomCenter,           ///< Bottom center
        BottomRight,            ///< Bottom-right corner
        StretchHorizontal,      ///< Stretch horizontally
        StretchVertical,        ///< Stretch vertically
        StretchBoth             ///< Stretch both directions
    };

    /**
     * @enum UIAlignment
     * @brief Text alignment options
     */
    enum class UIAlignment {
        Left,                   ///< Left alignment
        Center,                 ///< Center alignment
        Right,                  ///< Right alignment
        Justify                 ///< Justified alignment
    };

    /**
     * @enum UIVerticalAlignment
     * @brief Vertical text alignment options
     */
    enum class UIVerticalAlignment {
        Top,                    ///< Top alignment
        Middle,                 ///< Middle alignment
        Bottom                  ///< Bottom alignment
    };

    /**
     * @struct UIRect
     * @brief UI rectangle structure
     */
    struct UIRect {
        float x;                ///< X position
        float y;                ///< Y position
        float width;            ///< Width
        float height;           ///< Height

        UIRect() : x(0.0f), y(0.0f), width(0.0f), height(0.0f) {}
        UIRect(float x, float y, float w, float h) : x(x), y(y), width(w), height(h) {}

        /**
         * @brief Check if point is inside rectangle
         * @param point Point to check
         * @return true if inside, false otherwise
         */
        bool Contains(const glm::vec2& point) const {
            return point.x >= x && point.x <= x + width &&
                   point.y >= y && point.y <= y + height;
        }

        /**
         * @brief Check if rectangles intersect
         * @param other Other rectangle
         * @return true if intersect, false otherwise
         */
        bool Intersects(const UIRect& other) const {
            return x < other.x + other.width && x + width > other.x &&
                   y < other.y + other.height && y + height > other.y;
        }

        /**
         * @brief Get center point of rectangle
         * @return Center point
         */
        glm::vec2 GetCenter() const {
            return glm::vec2(x + width * 0.5f, y + height * 0.5f);
        }

        /**
         * @brief Convert to string representation
         * @return String representation
         */
        std::string ToString() const {
            return "(" + std::to_string(x) + ", " + std::to_string(y) + ", " +
                   std::to_string(width) + ", " + std::to_string(height) + ")";
        }
    };

    /**
     * @struct UIElementConfig
     * @brief UI element configuration
     */
    struct UIElementConfig {
        std::string name;                       ///< Element name
        UIElementType type;                     ///< Element type
        UIRect bounds;                          ///< Element bounds
        UIAnchor anchor;                        ///< Element anchor
        bool visible;                           ///< Element visibility
        bool enabled;                           ///< Element enabled state
        bool focusable;                         ///< Element can receive focus
        bool draggable;                         ///< Element can be dragged
        bool resizable;                         ///< Element can be resized
        int zOrder;                             ///< Z-order for rendering

        // Visual properties
        glm::vec4 backgroundColor;              ///< Background color
        glm::vec4 foregroundColor;              ///< Foreground color
        glm::vec4 borderColor;                  ///< Border color
        float borderWidth;                      ///< Border width
        float cornerRadius;                     ///< Corner radius for rounded corners
        std::string backgroundImage;            ///< Background image path
        std::string fontName;                   ///< Font name
        float fontSize;                         ///< Font size
        UIAlignment textAlignment;              ///< Text alignment
        UIVerticalAlignment verticalAlignment;  ///< Vertical text alignment

        // Layout properties
        float marginLeft;                       ///< Left margin
        float marginTop;                        ///< Top margin
        float marginRight;                      ///< Right margin
        float marginBottom;                     ///< Bottom margin
        float paddingLeft;                      ///< Left padding
        float paddingTop;                       ///< Top padding
        float paddingRight;                     ///< Right padding
        float paddingBottom;                    ///< Bottom padding

        // Animation properties
        float fadeInDuration;                   ///< Fade in duration
        float fadeOutDuration;                  ///< Fade out duration
        float hoverScale;                       ///< Scale on hover
        float pressScale;                       ///< Scale on press

        // Interaction properties
        std::string tooltip;                    ///< Tooltip text
        std::string helpText;                   ///< Help text
        bool playSounds;                        ///< Play interaction sounds
        std::string hoverSound;                 ///< Hover sound
        std::string clickSound;                 ///< Click sound

        // Custom properties
        std::unordered_map<std::string, std::any> customProperties; ///< Custom properties

        UIElementConfig()
            : type(UIElementType::Container)
            , anchor(UIAnchor::TopLeft)
            , visible(true)
            , enabled(true)
            , focusable(false)
            , draggable(false)
            , resizable(false)
            , zOrder(0)
            , backgroundColor(0.0f, 0.0f, 0.0f, 0.0f)
            , foregroundColor(1.0f, 1.0f, 1.0f, 1.0f)
            , borderColor(0.5f, 0.5f, 0.5f, 1.0f)
            , borderWidth(1.0f)
            , cornerRadius(0.0f)
            , fontSize(12.0f)
            , textAlignment(UIAlignment::Left)
            , verticalAlignment(UIVerticalAlignment::Middle)
            , marginLeft(0.0f)
            , marginTop(0.0f)
            , marginRight(0.0f)
            , marginBottom(0.0f)
            , paddingLeft(0.0f)
            , paddingTop(0.0f)
            , paddingRight(0.0f)
            , paddingBottom(0.0f)
            , fadeInDuration(0.2f)
            , fadeOutDuration(0.2f)
            , hoverScale(1.0f)
            , pressScale(0.95f)
            , playSounds(true)
        {}
    };

    /**
     * @struct UIElementMetrics
     * @brief Performance metrics for UI elements
     */
    struct UIElementMetrics {
        uint64_t renderCount;                   ///< Number of times rendered
        double totalRenderTime;                 ///< Total render time (ms)
        double averageRenderTime;               ///< Average render time (ms)
        double maxRenderTime;                   ///< Maximum render time (ms)

        uint64_t updateCount;                   ///< Number of times updated
        double totalUpdateTime;                 ///< Total update time (ms)
        double averageUpdateTime;               ///< Average update time (ms)
        double maxUpdateTime;                   ///< Maximum update time (ms)

        uint64_t inputEventCount;               ///< Number of input events processed
        uint64_t animationCount;                ///< Number of animations played
        size_t memoryUsage;                     ///< Memory usage (bytes)

        bool isVisible;                         ///< Element is currently visible
        bool isAnimating;                       ///< Element is currently animating
        bool hasFocus;                          ///< Element currently has focus
    };

    /**
     * @class UIElement
     * @brief Base class for all UI elements
     *
     * The UIElement class provides the foundation for all UI components in the
     * VoxelCraft game engine. It includes common functionality for positioning,
     * sizing, rendering, input handling, animation support, and event management.
     *
     * Key Features:
     * - Hierarchical element system with parent-child relationships
     * - Flexible positioning and anchoring system
     * - Advanced input handling with focus management
     * - Animation and transition support
     * - Style and theme system integration
     * - Event system for user interactions
     * - Performance monitoring and optimization
     * - Accessibility features
     * - Serialization support for UI layouts
     *
     * All UI elements inherit from this class and can override virtual methods
     * to provide custom behavior and rendering.
     */
    class UIElement {
    public:
        /**
         * @brief Constructor
         * @param config Element configuration
         */
        explicit UIElement(const UIElementConfig& config);

        /**
         * @brief Virtual destructor
         */
        virtual ~UIElement();

        /**
         * @brief Deleted copy constructor
         */
        UIElement(const UIElement&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        UIElement& operator=(const UIElement&) = delete;

        // Element lifecycle

        /**
         * @brief Initialize element
         * @param uiManager UI manager
         * @return true if successful, false otherwise
         */
        virtual bool Initialize(UIManager* uiManager);

        /**
         * @brief Shutdown element
         */
        virtual void Shutdown();

        /**
         * @brief Update element
         * @param deltaTime Time elapsed since last update
         */
        virtual void Update(double deltaTime);

        /**
         * @brief Render element
         */
        virtual void Render();

        // Element properties

        /**
         * @brief Get element ID
         * @return Element ID
         */
        uint32_t GetId() const { return m_id; }

        /**
         * @brief Set element ID
         * @param id New element ID
         */
        void SetId(uint32_t id) { m_id = id; }

        /**
         * @brief Get element name
         * @return Element name
         */
        const std::string& GetName() const { return m_config.name; }

        /**
         * @brief Set element name
         * @param name New element name
         */
        void SetName(const std::string& name) { m_config.name = name; }

        /**
         * @brief Get element type
         * @return Element type
         */
        UIElementType GetType() const { return m_config.type; }

        /**
         * @brief Get element state
         * @return Element state
         */
        UIElementState GetState() const { return m_state; }

        /**
         * @brief Set element state
         * @param state New element state
         */
        virtual void SetState(UIElementState state);

        // Position and size

        /**
         * @brief Get element bounds
         * @return Element bounds
         */
        const UIRect& GetBounds() const { return m_bounds; }

        /**
         * @brief Set element bounds
         * @param bounds New bounds
         */
        virtual void SetBounds(const UIRect& bounds);

        /**
         * @brief Get element position
         * @return Element position
         */
        glm::vec2 GetPosition() const { return glm::vec2(m_bounds.x, m_bounds.y); }

        /**
         * @brief Set element position
         * @param position New position
         */
        virtual void SetPosition(const glm::vec2& position);

        /**
         * @brief Get element size
         * @return Element size
         */
        glm::vec2 GetSize() const { return glm::vec2(m_bounds.width, m_bounds.height); }

        /**
         * @brief Set element size
         * @param size New size
         */
        virtual void SetSize(const glm::vec2& size);

        /**
         * @brief Get element anchor
         * @return Element anchor
         */
        UIAnchor GetAnchor() const { return m_config.anchor; }

        /**
         * @brief Set element anchor
         * @param anchor New anchor
         */
        void SetAnchor(UIAnchor anchor) { m_config.anchor = anchor; }

        // Visibility and interaction

        /**
         * @brief Check if element is visible
         * @return true if visible, false otherwise
         */
        bool IsVisible() const { return m_config.visible; }

        /**
         * @brief Set element visibility
         * @param visible New visibility state
         */
        virtual void SetVisible(bool visible);

        /**
         * @brief Check if element is enabled
         * @return true if enabled, false otherwise
         */
        bool IsEnabled() const { return m_config.enabled; }

        /**
         * @brief Set element enabled state
         * @param enabled New enabled state
         */
        virtual void SetEnabled(bool enabled);

        /**
         * @brief Check if element has focus
         * @return true if has focus, false otherwise
         */
        bool HasFocus() const { return m_hasFocus; }

        /**
         * @brief Set element focus
         * @param focus New focus state
         */
        virtual void SetFocus(bool focus);

        /**
         * @brief Check if element is draggable
         * @return true if draggable, false otherwise
         */
        bool IsDraggable() const { return m_config.draggable; }

        /**
         * @brief Set element draggable state
         * @param draggable New draggable state
         */
        void SetDraggable(bool draggable) { m_config.draggable = draggable; }

        // Parent-child relationships

        /**
         * @brief Get parent element
         * @return Parent element or nullptr
         */
        std::shared_ptr<UIElement> GetParent() const { return m_parent.lock(); }

        /**
         * @brief Set parent element
         * @param parent New parent element
         */
        virtual void SetParent(std::shared_ptr<UIElement> parent);

        /**
         * @brief Add child element
         * @param child Child element to add
         */
        virtual void AddChild(std::shared_ptr<UIElement> child);

        /**
         * @brief Remove child element
         * @param child Child element to remove
         * @return true if removed, false otherwise
         */
        virtual bool RemoveChild(std::shared_ptr<UIElement> child);

        /**
         * @brief Remove child element by ID
         * @param childId Child element ID
         * @return true if removed, false otherwise
         */
        virtual bool RemoveChild(uint32_t childId);

        /**
         * @brief Get child elements
         * @return Vector of child elements
         */
        const std::vector<std::shared_ptr<UIElement>>& GetChildren() const { return m_children; }

        /**
         * @brief Get child element by ID
         * @param childId Child element ID
         * @return Child element or nullptr if not found
         */
        std::shared_ptr<UIElement> GetChild(uint32_t childId) const;

        /**
         * @brief Get child element by name
         * @param name Child element name
         * @return Child element or nullptr if not found
         */
        std::shared_ptr<UIElement> GetChild(const std::string& name) const;

        /**
         * @brief Remove all children
         */
        void RemoveAllChildren();

        // Input handling

        /**
         * @brief Handle mouse input
         * @param position Mouse position
         * @param button Mouse button
         * @param action Button action
         * @param mods Modifier keys
         * @return true if handled, false otherwise
         */
        virtual bool HandleMouseInput(const glm::vec2& position, int button, int action, int mods);

        /**
         * @brief Handle keyboard input
         * @param key Key code
         * @param scancode Scan code
         * @param action Key action
         * @param mods Modifier keys
         * @return true if handled, false otherwise
         */
        virtual bool HandleKeyboardInput(int key, int scancode, int action, int mods);

        /**
         * @brief Handle text input
         * @param text Input text
         * @return true if handled, false otherwise
         */
        virtual bool HandleTextInput(const std::string& text);

        /**
         * @brief Handle touch input
         * @param position Touch position
         * @param action Touch action
         * @return true if handled, false otherwise
         */
        virtual bool HandleTouchInput(const glm::vec2& position, int action);

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
         * @brief Trigger event
         * @param event Event to trigger
         */
        void TriggerEvent(const UIEvent& event);

        // Animation system

        /**
         * @brief Start animation
         * @param animationType Animation type
         * @param duration Animation duration
         * @param properties Animation properties
         * @return Animation ID or 0 if failed
         */
        uint32_t StartAnimation(const std::string& animationType, float duration,
                               const std::unordered_map<std::string, float>& properties);

        /**
         * @brief Stop animation
         * @param animationId Animation ID
         * @return true if stopped, false otherwise
         */
        bool StopAnimation(uint32_t animationId);

        /**
         * @brief Update animations
         * @param deltaTime Time elapsed
         */
        void UpdateAnimations(double deltaTime);

        // Style and appearance

        /**
         * @brief Get element configuration
         * @return Element configuration
         */
        const UIElementConfig& GetConfig() const { return m_config; }

        /**
         * @brief Set element configuration
         * @param config New configuration
         */
        virtual void SetConfig(const UIElementConfig& config);

        /**
         * @brief Apply style
         * @param style Style to apply
         */
        virtual void ApplyStyle(const UIStyle& style);

        /**
         * @brief Set background color
         * @param color Background color
         */
        void SetBackgroundColor(const glm::vec4& color) { m_config.backgroundColor = color; }

        /**
         * @brief Set foreground color
         * @param color Foreground color
         */
        void SetForegroundColor(const glm::vec4& color) { m_config.foregroundColor = color; }

        /**
         * @brief Set text
         * @param text New text
         */
        virtual void SetText(const std::string& text) { m_text = text; }

        /**
         * @brief Get text
         * @return Current text
         */
        const std::string& GetText() const { return m_text; }

        /**
         * @brief Set font
         * @param fontName Font name
         * @param fontSize Font size
         */
        void SetFont(const std::string& fontName, float fontSize) {
            m_config.fontName = fontName;
            m_config.fontSize = fontSize;
        }

        // Utility functions

        /**
         * @brief Check if point is inside element
         * @param point Point to check
         * @return true if inside, false otherwise
         */
        bool ContainsPoint(const glm::vec2& point) const;

        /**
         * @brief Get element at position
         * @param position Position to check
         * @return Element at position or nullptr
         */
        std::shared_ptr<UIElement> GetElementAtPosition(const glm::vec2& position);

        /**
         * @brief Bring element to front
         */
        void BringToFront();

        /**
         * @brief Send element to back
         */
        void SendToBack();

        /**
         * @brief Set Z-order
         * @param zOrder New Z-order
         */
        void SetZOrder(int zOrder) { m_config.zOrder = zOrder; }

        /**
         * @brief Get Z-order
         * @return Current Z-order
         */
        int GetZOrder() const { return m_config.zOrder; }

        /**
         * @brief Get element metrics
         * @return Performance metrics
         */
        const UIElementMetrics& GetMetrics() const { return m_metrics; }

        /**
         * @brief Reset metrics
         */
        void ResetMetrics();

        /**
         * @brief Get element bounds in screen coordinates
         * @return Screen bounds
         */
        virtual UIRect GetScreenBounds() const;

        /**
         * @brief Invalidate element (mark for redraw)
         */
        void Invalidate();

        /**
         * @brief Check if element needs redraw
         * @return true if needs redraw, false otherwise
         */
        bool NeedsRedraw() const { return m_needsRedraw; }

        /**
         * @brief Mark element as drawn
         */
        void MarkAsDrawn() { m_needsRedraw = false; }

        // Serialization

        /**
         * @brief Serialize element to data
         * @return Serialized data
         */
        virtual std::unordered_map<std::string, std::any> Serialize() const;

        /**
         * @brief Deserialize element from data
         * @param data Serialized data
         * @return true if successful, false otherwise
         */
        virtual bool Deserialize(const std::unordered_map<std::string, std::any>& data);

        // Custom properties

        /**
         * @brief Set custom property
         * @param key Property key
         * @param value Property value
         */
        void SetCustomProperty(const std::string& key, const std::any& value) {
            m_config.customProperties[key] = value;
        }

        /**
         * @brief Get custom property
         * @tparam T Property type
         * @param key Property key
         * @param defaultValue Default value
         * @return Property value or default
         */
        template<typename T>
        T GetCustomProperty(const std::string& key, const T& defaultValue = T{}) const {
            auto it = m_config.customProperties.find(key);
            if (it != m_config.customProperties.end()) {
                try {
                    return std::any_cast<T>(it->second);
                } catch (const std::bad_any_cast&) {
                    return defaultValue;
                }
            }
            return defaultValue;
        }

        /**
         * @brief Check if element is valid
         * @return true if valid, false otherwise
         */
        virtual bool Validate() const;

        /**
         * @brief Get element debug info
         * @return Debug information string
         */
        virtual std::string GetDebugInfo() const;

        /**
         * @brief Clone element
         * @return Cloned element
         */
        virtual std::shared_ptr<UIElement> Clone() const;

    protected:
        /**
         * @brief Update element bounds based on anchor and parent
         */
        virtual void UpdateBounds();

        /**
         * @brief Render element content
         */
        virtual void RenderContent();

        /**
         * @brief Render element children
         */
        virtual void RenderChildren();

        /**
         * @brief Handle state change
         * @param oldState Previous state
         * @param newState New state
         */
        virtual void OnStateChanged(UIElementState oldState, UIElementState newState);

        /**
         * @brief Handle focus change
         * @param hasFocus New focus state
         */
        virtual void OnFocusChanged(bool hasFocus);

        /**
         * @brief Handle parent change
         * @param oldParent Previous parent
         * @param newParent New parent
         */
        virtual void OnParentChanged(std::shared_ptr<UIElement> oldParent,
                                   std::shared_ptr<UIElement> newParent);

        /**
         * @brief Handle child added
         * @param child Added child
         */
        virtual void OnChildAdded(std::shared_ptr<UIElement> child);

        /**
         * @brief Handle child removed
         * @param child Removed child
         */
        virtual void OnChildRemoved(std::shared_ptr<UIElement> child);

        /**
         * @brief Update performance metrics
         * @param deltaTime Time elapsed
         * @param renderTime Render time
         */
        void UpdateMetrics(double deltaTime, double renderTime);

        /**
         * @brief Handle element errors
         * @param error Error message
         */
        void HandleError(const std::string& error);

        // Element data
        uint32_t m_id;                              ///< Element unique ID
        UIElementConfig m_config;                   ///< Element configuration
        UIElementState m_state;                     ///< Current element state
        UIRect m_bounds;                            ///< Element bounds
        UIRect m_originalBounds;                    ///< Original bounds (before anchoring)

        // Hierarchy
        std::weak_ptr<UIElement> m_parent;          ///< Parent element
        std::vector<std::shared_ptr<UIElement>> m_children; ///< Child elements

        // Content
        std::string m_text;                         ///< Element text content
        std::unordered_map<std::string, std::any> m_properties; ///< Element properties

        // State
        bool m_hasFocus;                            ///< Element has focus
        bool m_isDragging;                          ///< Element is being dragged
        bool m_needsRedraw;                         ///< Element needs redraw

        // Animation
        struct ElementAnimation {
            uint32_t animationId;                   ///< Animation ID
            std::string type;                       ///< Animation type
            double startTime;                       ///< Start time
            double duration;                        ///< Duration
            bool isPlaying;                         ///< Is playing
            std::unordered_map<std::string, float> startValues; ///< Start values
            std::unordered_map<std::string, float> endValues; ///< End values
            std::function<void()> completionCallback; ///< Completion callback
        };

        std::vector<ElementAnimation> m_animations; ///< Active animations

        // Event system
        std::unordered_map<uint32_t, std::function<void(const UIEvent&)>> m_eventListeners; ///< Event listeners

        // Metrics
        UIElementMetrics m_metrics;                 ///< Performance metrics

        // References
        UIManager* m_uiManager;                     ///< UI manager reference
        UIRenderer* m_renderer;                     ///< UI renderer reference

        static std::atomic<uint32_t> s_nextElementId; ///< Next element ID counter
        static std::atomic<uint32_t> s_nextListenerId; ///< Next listener ID counter
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_UI_UI_ELEMENT_HPP
