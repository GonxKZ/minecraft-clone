/**
 * @file HUD.hpp
 * @brief VoxelCraft HUD System - Heads-Up Display
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines the HUD class that manages the game's heads-up display,
 * including health bars, inventory slots, minimap, action bars, and various
 * gameplay indicators with modern UI design and smooth animations.
 */

#ifndef VOXELCRAFT_UI_HUD_HPP
#define VOXELCRAFT_UI_HUD_HPP

#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <array>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../core/Config.hpp"
#include "UIElement.hpp"

namespace VoxelCraft {

    // Forward declarations
    class UIManager;
    class Player;
    class Inventory;
    class World;
    class FontManager;
    class TextureManager;

    /**
     * @enum HUDComponent
     * @brief HUD component types
     */
    enum class HUDComponent {
        HealthBar,              ///< Health indicator
        HungerBar,              ///< Hunger indicator
        StaminaBar,             ///< Stamina indicator
        ExperienceBar,          ///< Experience indicator
        Hotbar,                 ///< Quick access inventory
        Minimap,                ///< Mini map display
        Crosshair,              ///< Crosshair/targeting indicator
        StatusEffects,          ///< Active status effects
        QuestTracker,           ///< Quest objectives
        Coordinates,            ///< Position coordinates
        FPSCounter,             ///< Frame rate display
        DebugInfo,              ///< Debug information
        ChatOverlay,            ///< Chat messages overlay
        ActionBar,              ///< Action/ability bar
        Compass,                ///< Directional compass
        Clock,                  ///< Game time display
        WeatherInfo,            ///< Weather information
        Custom                  ///< Custom HUD component
    };

    /**
     * @enum HUDPosition
     * @brief HUD positioning options
     */
    enum class HUDPosition {
        TopLeft,                ///< Top-left corner
        TopCenter,              ///< Top center
        TopRight,               ///< Top-right corner
        MiddleLeft,             ///< Middle left
        MiddleCenter,           ///< Center screen
        MiddleRight,            ///< Middle right
        BottomLeft,             ///< Bottom-left corner
        BottomCenter,           ///< Bottom center
        BottomRight,            ///< Bottom-right corner
        Custom                  ///< Custom position
    };

    /**
     * @enum HUDAnimation
     * @brief HUD animation types
     */
    enum class HUDAnimation {
        Fade,                   ///< Fade in/out
        Slide,                  ///< Slide in/out
        Scale,                  ///< Scale animation
        Pulse,                  ///< Pulse effect
        Shake,                  ///< Shake effect
        Glow,                   ///< Glow effect
        None                    ///< No animation
    };

    /**
     * @struct HUDComponentConfig
     * @brief Configuration for individual HUD components
     */
    struct HUDComponentConfig {
        HUDComponent component;                 ///< Component type
        HUDPosition position;                   ///< Component position
        glm::vec2 offset;                      ///< Position offset
        glm::vec2 size;                        ///< Component size
        bool visible;                          ///< Component visibility
        bool enabled;                          ///< Component enabled state
        float opacity;                         ///< Component opacity (0.0 - 1.0)
        HUDAnimation animation;                ///< Animation type
        float animationSpeed;                  ///< Animation speed
        std::string style;                     ///< Visual style
        std::unordered_map<std::string, std::any> properties; ///< Custom properties

        HUDComponentConfig()
            : component(HUDComponent::Custom)
            , position(HUDPosition::TopLeft)
            , offset(0.0f, 0.0f)
            , size(100.0f, 20.0f)
            , visible(true)
            , enabled(true)
            , opacity(1.0f)
            , animation(HUDAnimation::None)
            , animationSpeed(1.0f)
        {}
    };

    /**
     * @struct HUDConfig
     * @brief Main HUD configuration
     */
    struct HUDConfig {
        // Basic settings
        bool enabled;                          ///< HUD is enabled
        bool showInMenus;                      ///< Show HUD in menus
        bool showInInventory;                  ///< Show HUD in inventory
        float globalScale;                     ///< Global HUD scale
        float globalOpacity;                   ///< Global HUD opacity

        // Layout settings
        float screenMargin;                    ///< Screen margin for components
        bool autoArrange;                      ///< Auto-arrange components
        bool snapToGrid;                       ///< Snap components to grid
        glm::vec2 gridSize;                    ///< Grid size for snapping

        // Animation settings
        bool enableAnimations;                 ///< Enable HUD animations
        float fadeInDuration;                  ///< Fade in duration
        float fadeOutDuration;                 ///< Fade out duration
        float transitionSpeed;                 ///< Transition animation speed

        // Visual settings
        std::string theme;                     ///< HUD theme
        bool useCustomColors;                  ///< Use custom colors
        glm::vec4 primaryColor;                ///< Primary color
        glm::vec4 secondaryColor;              ///< Secondary color
        glm::vec4 accentColor;                 ///< Accent color
        glm::vec4 warningColor;                ///< Warning color

        // Component settings
        bool showHealthBar;                    ///< Show health bar
        bool showHungerBar;                    ///< Show hunger bar
        bool showStaminaBar;                   ///< Show stamina bar
        bool showExperienceBar;                ///< Show experience bar
        bool showHotbar;                       ///< Show hotbar
        bool showMinimap;                      ///< Show minimap
        bool showCrosshair;                    ///< Show crosshair
        bool showStatusEffects;                ///< Show status effects
        bool showQuestTracker;                 ///< Show quest tracker
        bool showCoordinates;                  ///< Show coordinates
        bool showFPSCounter;                   ///< Show FPS counter
        bool showDebugInfo;                    ///< Show debug info
        bool showChatOverlay;                  ///< Show chat overlay
        bool showActionBar;                    ///< Show action bar
        bool showCompass;                      ///< Show compass
        bool showClock;                        ///< Show clock
        bool showWeatherInfo;                  ///< Show weather info

        // Performance settings
        float updateInterval;                  ///< HUD update interval
        bool enableCulling;                    ///< Enable HUD culling
        float maxRenderDistance;               ///< Maximum render distance
        int maxVisibleComponents;              ///< Maximum visible components

        // Accessibility settings
        bool highContrast;                     ///< High contrast mode
        bool largeText;                        ///< Large text mode
        bool screenReader;                     ///< Screen reader support
        float textScale;                       ///< Text scale factor
    };

    /**
     * @struct HUDMetrics
     * @brief Performance metrics for HUD system
     */
    struct HUDMetrics {
        uint64_t updateCount;                  ///< Number of updates performed
        double totalUpdateTime;                ///< Total update time (ms)
        double averageUpdateTime;              ///< Average update time (ms)
        double maxUpdateTime;                  ///< Maximum update time (ms)

        uint32_t activeComponents;             ///< Number of active components
        uint32_t visibleComponents;            ///< Number of visible components
        uint32_t animatedComponents;           ///< Number of animated components

        uint32_t drawCalls;                    ///< Number of draw calls
        uint32_t verticesDrawn;                ///< Number of vertices drawn
        uint32_t textureSwitches;              ///< Number of texture switches

        size_t memoryUsage;                    ///< Memory usage (bytes)
        float averageFPS;                      ///< Average FPS
        float minFPS;                          ///< Minimum FPS
        float maxFPS;                          ///< Maximum FPS
    };

    /**
     * @class HUD
     * @brief Heads-Up Display system for VoxelCraft
     *
     * The HUD class manages the game's heads-up display, providing essential
     * gameplay information through various visual components. It includes
     * health bars, inventory indicators, minimap, and other gameplay elements
     * with modern UI design and smooth animations.
     *
     * Key Features:
     * - Modular component system with easy customization
     * - Advanced health, hunger, and stamina indicators
     * - Interactive hotbar with drag & drop support
     * - Real-time minimap with zoom and filtering
     * - Status effect indicators with detailed information
     * - Quest and objective tracking
     * - Performance monitoring and debug displays
     * - Accessibility features and screen reader support
     * - Smooth animations and transitions
     * - Theme system with customizable appearance
     * - Performance optimization with culling and batching
     *
     * The HUD system is designed to be highly configurable and extensible,
     * allowing for easy addition of new components and customization of
     * existing ones to match different gameplay styles and visual themes.
     */
    class HUD {
    public:
        /**
         * @brief Constructor
         * @param config HUD configuration
         */
        explicit HUD(const HUDConfig& config);

        /**
         * @brief Destructor
         */
        ~HUD();

        /**
         * @brief Deleted copy constructor
         */
        HUD(const HUD&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        HUD& operator=(const HUD&) = delete;

        // HUD lifecycle

        /**
         * @brief Initialize HUD system
         * @param uiManager UI manager
         * @return true if successful, false otherwise
         */
        bool Initialize(UIManager* uiManager);

        /**
         * @brief Shutdown HUD system
         */
        void Shutdown();

        /**
         * @brief Update HUD system
         * @param deltaTime Time elapsed since last update
         */
        void Update(double deltaTime);

        /**
         * @brief Render HUD elements
         */
        void Render();

        /**
         * @brief Get HUD configuration
         * @return Current configuration
         */
        const HUDConfig& GetConfig() const { return m_config; }

        /**
         * @brief Set HUD configuration
         * @param config New configuration
         */
        void SetConfig(const HUDConfig& config);

        // Component management

        /**
         * @brief Enable HUD component
         * @param component Component to enable
         * @return true if enabled, false otherwise
         */
        bool EnableComponent(HUDComponent component);

        /**
         * @brief Disable HUD component
         * @param component Component to disable
         * @return true if disabled, false otherwise
         */
        bool DisableComponent(HUDComponent component);

        /**
         * @brief Check if component is enabled
         * @param component Component to check
         * @return true if enabled, false otherwise
         */
        bool IsComponentEnabled(HUDComponent component) const;

        /**
         * @brief Show HUD component
         * @param component Component to show
         * @return true if shown, false otherwise
         */
        bool ShowComponent(HUDComponent component);

        /**
         * @brief Hide HUD component
         * @param component Component to hide
         * @return true if hidden, false otherwise
         */
        bool HideComponent(HUDComponent component);

        /**
         * @brief Check if component is visible
         * @param component Component to check
         * @return true if visible, false otherwise
         */
        bool IsComponentVisible(HUDComponent component) const;

        /**
         * @brief Get component configuration
         * @param component Component type
         * @return Component configuration
         */
        const HUDComponentConfig& GetComponentConfig(HUDComponent component) const;

        /**
         * @brief Set component configuration
         * @param component Component type
         * @param config New configuration
         * @return true if set, false otherwise
         */
        bool SetComponentConfig(HUDComponent component, const HUDComponentConfig& config);

        // Player integration

        /**
         * @brief Set player reference
         * @param player Player instance
         */
        void SetPlayer(Player* player);

        /**
         * @brief Get player reference
         * @return Player instance
         */
        Player* GetPlayer() const { return m_player; }

        /**
         * @brief Update player information
         * @param player Player instance
         */
        void UpdatePlayerInfo(Player* player);

        // Health system

        /**
         * @brief Set health value
         * @param current Current health
         * @param max Maximum health
         */
        void SetHealth(float current, float max);

        /**
         * @brief Get current health
         * @return Current health value
         */
        float GetCurrentHealth() const { return m_currentHealth; }

        /**
         * @brief Get maximum health
         * @return Maximum health value
         */
        float GetMaxHealth() const { return m_maxHealth; }

        /**
         * @brief Set hunger value
         * @param current Current hunger
         * @param max Maximum hunger
         */
        void SetHunger(float current, float max);

        /**
         * @brief Set stamina value
         * @param current Current stamina
         * @param max Maximum stamina
         */
        void SetStamina(float current, float max);

        /**
         * @brief Set experience value
         * @param current Current experience
         * @param max Maximum experience
         * @param level Current level
         */
        void SetExperience(float current, float max, int level);

        // Hotbar system

        /**
         * @brief Set hotbar size
         * @param size New size
         * @return true if successful, false otherwise
         */
        bool SetHotbarSize(int size);

        /**
         * @brief Get hotbar size
         * @return Current hotbar size
         */
        int GetHotbarSize() const { return m_hotbarSize; }

        /**
         * @brief Set hotbar selection
         * @param slot Selected slot
         * @return true if successful, false otherwise
         */
        bool SetHotbarSelection(int slot);

        /**
         * @brief Get hotbar selection
         * @return Current selection
         */
        int GetHotbarSelection() const { return m_hotbarSelection; }

        /**
         * @brief Set hotbar item
         * @param slot Slot index
         * @param itemId Item ID
         * @param count Item count
         * @param texture Texture name
         * @return true if successful, false otherwise
         */
        bool SetHotbarItem(int slot, uint32_t itemId, int count, const std::string& texture);

        // Minimap system

        /**
         * @brief Set minimap position
         * @param position Player position
         */
        void SetMinimapPosition(const glm::vec3& position);

        /**
         * @brief Set minimap zoom
         * @param zoom Zoom level
         * @return true if successful, false otherwise
         */
        bool SetMinimapZoom(float zoom);

        /**
         * @brief Get minimap zoom
         * @return Current zoom level
         */
        float GetMinimapZoom() const { return m_minimapZoom; }

        /**
         * @brief Add minimap marker
         * @param position Marker position
         * @param type Marker type
         * @param color Marker color
         * @param size Marker size
         * @return Marker ID or 0 if failed
         */
        uint32_t AddMinimapMarker(const glm::vec3& position, const std::string& type,
                                 const glm::vec4& color, float size);

        /**
         * @brief Remove minimap marker
         * @param markerId Marker ID
         * @return true if removed, false otherwise
         */
        bool RemoveMinimapMarker(uint32_t markerId);

        // Status effects

        /**
         * @brief Add status effect
         * @param effectId Effect ID
         * @param name Effect name
         * @param icon Icon texture
         * @param duration Duration (seconds)
         * @param color Effect color
         * @return true if added, false otherwise
         */
        bool AddStatusEffect(uint32_t effectId, const std::string& name,
                           const std::string& icon, float duration,
                           const glm::vec4& color);

        /**
         * @brief Remove status effect
         * @param effectId Effect ID
         * @return true if removed, false otherwise
         */
        bool RemoveStatusEffect(uint32_t effectId);

        /**
         * @brief Update status effect
         * @param effectId Effect ID
         * @param duration New duration
         * @return true if updated, false otherwise
         */
        bool UpdateStatusEffect(uint32_t effectId, float duration);

        // Quest system

        /**
         * @brief Add quest objective
         * @param questId Quest ID
         * @param objective Objective text
         * @param progress Current progress
         * @param maxProgress Maximum progress
         * @return true if added, false otherwise
         */
        bool AddQuestObjective(uint32_t questId, const std::string& objective,
                              int progress, int maxProgress);

        /**
         * @brief Update quest objective
         * @param questId Quest ID
         * @param progress New progress
         * @return true if updated, false otherwise
         */
        bool UpdateQuestObjective(uint32_t questId, int progress);

        /**
         * @brief Remove quest objective
         * @param questId Quest ID
         * @return true if removed, false otherwise
         */
        bool RemoveQuestObjective(uint32_t questId);

        // Crosshair system

        /**
         * @brief Set crosshair type
         * @param type Crosshair type
         * @return true if successful, false otherwise
         */
        bool SetCrosshairType(const std::string& type);

        /**
         * @brief Set crosshair color
         * @param color Crosshair color
         */
        void SetCrosshairColor(const glm::vec4& color);

        /**
         * @brief Set crosshair size
         * @param size Crosshair size
         */
        void SetCrosshairSize(float size);

        /**
         * @brief Show crosshair
         */
        void ShowCrosshair();

        /**
         * @brief Hide crosshair
         */
        void HideCrosshair();

        /**
         * @brief Toggle crosshair visibility
         */
        void ToggleCrosshair();

        // Notification system

        /**
         * @brief Show notification
         * @param message Notification message
         * @param duration Display duration
         * @param color Text color
         * @param position Display position
         * @return Notification ID
         */
        uint32_t ShowNotification(const std::string& message, float duration = 3.0f,
                                const glm::vec4& color = glm::vec4(1.0f),
                                HUDPosition position = HUDPosition::TopCenter);

        /**
         * @brief Hide notification
         * @param notificationId Notification ID
         * @return true if hidden, false otherwise
         */
        bool HideNotification(uint32_t notificationId);

        /**
         * @brief Clear all notifications
         */
        void ClearNotifications();

        // Animation system

        /**
         * @brief Play component animation
         * @param component Component to animate
         * @param animation Animation type
         * @param duration Animation duration
         * @return true if successful, false otherwise
         */
        bool PlayComponentAnimation(HUDComponent component, HUDAnimation animation, float duration);

        /**
         * @brief Stop component animation
         * @param component Component to stop
         * @return true if stopped, false otherwise
         */
        bool StopComponentAnimation(HUDComponent component);

        // Performance monitoring

        /**
         * @brief Set FPS value
         * @param fps Current FPS
         */
        void SetFPS(float fps);

        /**
         * @brief Set debug info
         * @param info Debug information
         */
        void SetDebugInfo(const std::string& info);

        /**
         * @brief Toggle debug info visibility
         */
        void ToggleDebugInfo();

        // Configuration

        /**
         * @brief Load HUD layout
         * @param layoutName Layout name
         * @return true if successful, false otherwise
         */
        bool LoadLayout(const std::string& layoutName);

        /**
         * @brief Save current layout
         * @param layoutName Layout name
         * @return true if successful, false otherwise
         */
        bool SaveLayout(const std::string& layoutName);

        /**
         * @brief Reset HUD to default layout
         */
        void ResetToDefaultLayout();

        // Metrics and monitoring

        /**
         * @brief Get HUD metrics
         * @return Performance metrics
         */
        const HUDMetrics& GetMetrics() const { return m_metrics; }

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
         * @brief Get HUD element by component
         * @param component Component type
         * @return UI element or nullptr
         */
        std::shared_ptr<UIElement> GetComponentElement(HUDComponent component) const;

        /**
         * @brief Check if HUD is visible
         * @return true if visible, false otherwise
         */
        bool IsVisible() const { return m_config.enabled; }

        /**
         * @brief Set HUD visibility
         * @param visible New visibility state
         */
        void SetVisible(bool visible);

        /**
         * @brief Check if HUD is valid
         * @return true if valid, false otherwise
         */
        bool Validate() const;

        /**
         * @brief Get HUD status report
         * @return Detailed status report
         */
        std::string GetStatusReport() const;

        /**
         * @brief Optimize HUD performance
         * @return Optimization results
         */
        std::unordered_map<std::string, size_t> Optimize();

    private:
        /**
         * @brief Initialize HUD components
         * @return true if successful, false otherwise
         */
        bool InitializeComponents();

        /**
         * @brief Create component elements
         */
        void CreateComponentElements();

        /**
         * @brief Update HUD layout
         */
        void UpdateLayout();

        /**
         * @brief Update component positions
         */
        void UpdateComponentPositions();

        /**
         * @brief Update health bar display
         */
        void UpdateHealthBar();

        /**
         * @brief Update hunger bar display
         */
        void UpdateHungerBar();

        /**
         * @brief Update stamina bar display
         */
        void UpdateStaminaBar();

        /**
         * @brief Update experience bar display
         */
        void UpdateExperienceBar();

        /**
         * @brief Update hotbar display
         */
        void UpdateHotbar();

        /**
         * @brief Update minimap display
         */
        void UpdateMinimap();

        /**
         * @brief Update status effects display
         */
        void UpdateStatusEffects();

        /**
         * @brief Update quest tracker display
         */
        void UpdateQuestTracker();

        /**
         * @brief Update notifications
         * @param deltaTime Time elapsed
         */
        void UpdateNotifications(double deltaTime);

        /**
         * @brief Render HUD components
         */
        void RenderComponents();

        /**
         * @brief Handle component events
         * @param component Component type
         * @param event Event data
         */
        void HandleComponentEvent(HUDComponent component, const std::unordered_map<std::string, std::any>& event);

        /**
         * @brief Update performance metrics
         * @param deltaTime Time elapsed
         */
        void UpdateMetrics(double deltaTime);

        /**
         * @brief Handle HUD errors
         * @param error Error message
         */
        void HandleError(const std::string& error);

        // HUD data
        HUDConfig m_config;                                    ///< HUD configuration
        HUDMetrics m_metrics;                                  ///< Performance metrics

        // Core systems
        UIManager* m_uiManager;                               ///< UI manager reference
        Player* m_player;                                     ///< Player reference
        FontManager* m_fontManager;                           ///< Font manager reference
        TextureManager* m_textureManager;                     ///< Texture manager reference

        // HUD components
        std::unordered_map<HUDComponent, HUDComponentConfig> m_componentConfigs; ///< Component configurations
        std::unordered_map<HUDComponent, std::shared_ptr<UIElement>> m_componentElements; ///< Component elements

        // Health system
        float m_currentHealth;                                ///< Current health
        float m_maxHealth;                                    ///< Maximum health
        float m_currentHunger;                                ///< Current hunger
        float m_maxHunger;                                    ///< Maximum hunger
        float m_currentStamina;                               ///< Current stamina
        float m_maxStamina;                                   ///< Maximum stamina
        float m_currentExperience;                            ///< Current experience
        float m_maxExperience;                                ///< Maximum experience
        int m_currentLevel;                                   ///< Current level

        // Hotbar system
        int m_hotbarSize;                                     ///< Hotbar size
        int m_hotbarSelection;                                ///< Selected slot
        struct HotbarItem {
            uint32_t itemId;                                 ///< Item ID
            int count;                                       ///< Item count
            std::string texture;                             ///< Item texture
            std::shared_ptr<UIElement> element;             ///< UI element
        };
        std::vector<HotbarItem> m_hotbarItems;               ///< Hotbar items

        // Minimap system
        float m_minimapZoom;                                 ///< Minimap zoom level
        glm::vec3 m_minimapPosition;                         ///< Minimap center position
        struct MinimapMarker {
            uint32_t markerId;                              ///< Marker ID
            glm::vec3 position;                             ///< Marker position
            std::string type;                               ///< Marker type
            glm::vec4 color;                                ///< Marker color
            float size;                                     ///< Marker size
            std::shared_ptr<UIElement> element;            ///< UI element
        };
        std::unordered_map<uint32_t, MinimapMarker> m_minimapMarkers; ///< Minimap markers

        // Status effects
        struct StatusEffect {
            uint32_t effectId;                              ///< Effect ID
            std::string name;                               ///< Effect name
            std::string icon;                               ///< Effect icon
            float duration;                                 ///< Effect duration
            float remainingTime;                            ///< Remaining time
            glm::vec4 color;                                ///< Effect color
            std::shared_ptr<UIElement> element;            ///< UI element
        };
        std::unordered_map<uint32_t, StatusEffect> m_statusEffects; ///< Active status effects

        // Quest system
        struct QuestObjective {
            uint32_t questId;                               ///< Quest ID
            std::string objective;                          ///< Objective text
            int progress;                                   ///< Current progress
            int maxProgress;                                ///< Maximum progress
            std::shared_ptr<UIElement> element;            ///< UI element
        };
        std::unordered_map<uint32_t, QuestObjective> m_questObjectives; ///< Quest objectives

        // Notification system
        struct Notification {
            uint32_t notificationId;                        ///< Notification ID
            std::string message;                            ///< Notification message
            float duration;                                 ///< Display duration
            float remainingTime;                            ///< Remaining time
            glm::vec4 color;                                ///< Text color
            HUDPosition position;                           ///< Display position
            std::shared_ptr<UIElement> element;            ///< UI element
        };
        std::unordered_map<uint32_t, Notification> m_notifications; ///< Active notifications

        // Crosshair system
        std::string m_crosshairType;                        ///< Crosshair type
        glm::vec4 m_crosshairColor;                         ///< Crosshair color
        float m_crosshairSize;                              ///< Crosshair size
        std::shared_ptr<UIElement> m_crosshairElement;     ///< Crosshair element

        // Performance monitoring
        float m_currentFPS;                                 ///< Current FPS
        std::string m_debugInfo;                            ///< Debug information

        // State flags
        bool m_isInitialized;                               ///< HUD is initialized
        bool m_needsLayoutUpdate;                           ///< Layout needs update

        static std::atomic<uint32_t> s_nextMarkerId;       ///< Next marker ID counter
        static std::atomic<uint32_t> s_nextNotificationId;  ///< Next notification ID counter
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_UI_HUD_HPP
