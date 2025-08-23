/**
 * @file UIHUD.hpp
 * @brief VoxelCraft HUD System
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_UI_UI_HUD_HPP
#define VOXELCRAFT_UI_UI_HUD_HPP

#include "UIWidgets.hpp"
#include <memory>

namespace VoxelCraft {

    class Player;
    class World;

    /**
     * @class UIHUD
     * @brief Main Heads-Up Display
     */
    class UIHUD : public UIElement {
    public:
        UIHUD(const std::string& id);
        ~UIHUD() override = default;

        void SetPlayer(std::shared_ptr<Player> player) { m_player = player; }
        void SetWorld(std::shared_ptr<World> world) { m_world = world; }

        void SetVisible(bool visible) override;

        void Update(float deltaTime) override;
        void Render() override;
        bool HandleInput(const UIEvent& event) override;

    private:
        std::shared_ptr<Player> m_player;
        std::shared_ptr<World> m_world;

        // Core HUD elements
        std::shared_ptr<UICrosshair> m_crosshair;
        std::shared_ptr<UIHotbar> m_hotbar;
        std::shared_ptr<UIStatusBar> m_healthBar;
        std::shared_ptr<UIStatusBar> m_hungerBar;
        std::shared_ptr<UIStatusBar> m_experienceBar;
        std::shared_ptr<UIStatusBar> m_armorBar;
        std::shared_ptr<UIStatusBar> m_breathBar;

        // Additional elements
        std::shared_ptr<UIText> m_coordinatesText;
        std::shared_ptr<UIText> m_biomeText;
        std::shared_ptr<UIProgressBar> m_mountHealthBar;
        std::shared_ptr<UIText> m_debugText;

        // Effects and overlays
        std::shared_ptr<UIElement> m_damageOverlay;
        std::shared_ptr<UIElement> m_hungerOverlay;
        std::shared_ptr<UIElement> m_vignetteOverlay;
        std::shared_ptr<UIElement> m_bossHealthBar;

        void InitializeHUD();
        void UpdatePlayerStats();
        void UpdateWorldInfo();
        void UpdateEffects();
        void HandleHotbarInput(const UIEvent& event);
    };

    /**
     * @class UIMiniMap
     * @brief Mini-map display
     */
    class UIMiniMap : public UIElement {
    public:
        UIMiniMap(const std::string& id);
        ~UIMiniMap() override = default;

        void SetWorld(std::shared_ptr<World> world) { m_world = world; }
        void SetPlayer(std::shared_ptr<Player> player) { m_player = player; }

        void SetSize(float size) { m_size = size; }
        void SetZoom(float zoom) { m_zoom = zoom; }

        void SetVisible(bool visible) override;

        void Update(float deltaTime) override;
        void Render() override;

    private:
        std::shared_ptr<World> m_world;
        std::shared_ptr<Player> m_player;

        float m_size = 128.0f;
        float m_zoom = 1.0f;
        bool m_showCoordinates = false;
        bool m_showEntities = true;

        void RenderMap();
        void RenderPlayerPosition();
        void RenderEntities();
        void RenderWaypoints();
    };

    /**
     * @class UIActionBar
     * @brief Action bar for temporary messages and actions
     */
    class UIActionBar : public UIElement {
    public:
        UIActionBar(const std::string& id);
        ~UIActionBar() override = default;

        void ShowMessage(const std::string& message, float duration = 3.0f);
        void ShowAction(const std::string& action, float duration = 1.0f);
        void ClearMessage();

        void Update(float deltaTime) override;
        void Render() override;

    private:
        struct ActionMessage {
            std::string text;
            float duration;
            float elapsedTime;
            bool isAction;
        };

        std::vector<ActionMessage> m_messages;
        std::shared_ptr<UIText> m_textElement;

        void UpdateMessages(float deltaTime);
    };

    /**
     * @class UIEffectOverlay
     * @brief Screen effect overlays (damage, potion effects, etc.)
     */
    class UIEffectOverlay : public UIElement {
    public:
        UIEffectOverlay(const std::string& id);
        ~UIEffectOverlay() override = default;

        void AddEffect(const std::string& effectType, float intensity = 1.0f, float duration = -1.0f);
        void RemoveEffect(const std::string& effectType);
        void ClearEffects();

        void Update(float deltaTime) override;
        void Render() override;

    private:
        struct Effect {
            std::string type;
            float intensity;
            float duration;
            float elapsedTime;
            Vec4 color;
        };

        std::vector<Effect> m_effects;
        std::unordered_map<std::string, Effect> m_activeEffects;

        void UpdateEffects(float deltaTime);
        void RenderEffects();
        Vec4 GetEffectColor(const std::string& effectType);
    };

    /**
     * @class UIBossHealthBar
     * @brief Boss health display
     */
    class UIBossHealthBar : public UIElement {
    public:
        UIBossHealthBar(const std::string& id);
        ~UIBossHealthBar() override = default;

        void SetBossName(const std::string& name) { m_bossName = name; }
        void SetHealth(float current, float max);
        void Show() { SetVisible(true); }
        void Hide() { SetVisible(false); }

        void Update(float deltaTime) override;
        void Render() override;

    private:
        std::string m_bossName;
        float m_currentHealth = 0.0f;
        float m_maxHealth = 0.0f;
        float m_displayHealth = 0.0f;

        std::shared_ptr<UIText> m_nameText;
        std::shared_ptr<UIProgressBar> m_healthBar;

        void InitializeBar();
    };

    /**
     * @class UIScoreboard
     * @brief Scoreboard display for multiplayer
     */
    class UIScoreboard : public UIElement {
    public:
        UIScoreboard(const std::string& id);
        ~UIScoreboard() override = default;

        void AddPlayer(const std::string& playerName, int score, int ping);
        void RemovePlayer(const std::string& playerName);
        void UpdatePlayer(const std::string& playerName, int score, int ping);
        void ClearPlayers();

        void Show() { SetVisible(true); }
        void Hide() { SetVisible(false); }

        void Update(float deltaTime) override;
        void Render() override;

    private:
        struct PlayerInfo {
            std::string name;
            int score;
            int ping;
            bool isCurrentPlayer;
        };

        std::vector<PlayerInfo> m_players;
        std::vector<std::shared_ptr<UIText>> m_playerTexts;

        void SortPlayers();
        void UpdateDisplay();
    };

    /**
     * @class UITooltipManager
     * @brief Manages item and UI tooltips
     */
    class UITooltipManager {
    public:
        static UITooltipManager& GetInstance();

        void ShowTooltip(const std::string& text, const Vec2& position, float duration = -1.0f);
        void ShowItemTooltip(const std::string& itemId, const Vec2& position);
        void HideTooltip();
        bool IsTooltipVisible() const { return m_tooltipVisible; }

        void Update(float deltaTime);
        void Render();

    private:
        UITooltipManager() = default;

        bool m_tooltipVisible = false;
        std::string m_tooltipText;
        Vec2 m_tooltipPosition;
        float m_tooltipDuration = -1.0f;
        float m_tooltipTimer = 0.0f;
        std::shared_ptr<UIElement> m_tooltipElement;
        std::shared_ptr<UIText> m_tooltipTextElement;

        void CreateTooltipElement();
        void UpdateTooltipPosition();
    };

    /**
     * @class UIDebugOverlay
     * @brief Debug information overlay
     */
    class UIDebugOverlay : public UIElement {
    public:
        UIDebugOverlay(const std::string& id);
        ~UIDebugOverlay() override = default;

        void SetPlayer(std::shared_ptr<Player> player) { m_player = player; }
        void SetWorld(std::shared_ptr<World> world) { m_world = world; }

        void SetVisible(bool visible) override;

        void Update(float deltaTime) override;
        void Render() override;

    private:
        std::shared_ptr<Player> m_player;
        std::shared_ptr<World> m_world;

        std::shared_ptr<UIText> m_fpsText;
        std::shared_ptr<UIText> m_positionText;
        std::shared_ptr<UIText> m_chunkText;
        std::shared_ptr<UIText> m_memoryText;
        std::shared_ptr<UIText> m_networkText;

        float m_updateTimer = 0.0f;

        void UpdateDebugInfo();
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_UI_UI_HUD_HPP
