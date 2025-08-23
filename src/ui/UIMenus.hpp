/**
 * @file UIMenus.hpp
 * @brief VoxelCraft Main Menu System
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_UI_UI_MENUS_HPP
#define VOXELCRAFT_UI_UI_MENUS_HPP

#include "UISystem.hpp"
#include <memory>

namespace VoxelCraft {

    /**
     * @class UIMainMenu
     * @brief Main game menu
     */
    class UIMainMenu : public UIElement {
    public:
        UIMainMenu(const std::string& id);
        ~UIMainMenu() override = default;

        void Show() { SetVisible(true); }
        void Hide() { SetVisible(false); }

        void SetOnSinglePlayer(std::function<void()> callback) { m_onSinglePlayer = callback; }
        void SetOnMultiPlayer(std::function<void()> callback) { m_onMultiPlayer = callback; }
        void SetOnOptions(std::function<void()> callback) { m_onOptions = callback; }
        void SetOnQuit(std::function<void()> callback) { m_onQuit = callback; }

        void Update(float deltaTime) override;
        void Render() override;
        bool HandleInput(const UIEvent& event) override;

    private:
        std::shared_ptr<UIButton> m_singlePlayerButton;
        std::shared_ptr<UIButton> m_multiPlayerButton;
        std::shared_ptr<UIButton> m_optionsButton;
        std::shared_ptr<UIButton> m_quitButton;
        std::shared_ptr<UIImage> m_logo;
        std::shared_ptr<UIText> m_versionText;

        std::function<void()> m_onSinglePlayer;
        std::function<void()> m_onMultiPlayer;
        std::function<void()> m_onOptions;
        std::function<void()> m_onQuit;

        void InitializeMenu();
    };

    /**
     * @class UIWorldSelectionMenu
     * @brief World selection menu
     */
    class UIWorldSelectionMenu : public UIElement {
    public:
        UIWorldSelectionMenu(const std::string& id);
        ~UIWorldSelectionMenu() override = default;

        void SetWorlds(const std::vector<std::string>& worlds) { m_worlds = worlds; }
        void RefreshWorldList();

        void SetOnWorldSelected(std::function<void(const std::string&)> callback) {
            m_onWorldSelected = callback;
        }
        void SetOnCreateWorld(std::function<void()> callback) { m_onCreateWorld = callback; }
        void SetOnBack(std::function<void()> callback) { m_onBack = callback; }

        void Show() { SetVisible(true); RefreshWorldList(); }
        void Hide() { SetVisible(false); }

        void Update(float deltaTime) override;
        void Render() override;
        bool HandleInput(const UIEvent& event) override;

    private:
        std::vector<std::string> m_worlds;
        std::vector<std::shared_ptr<UIButton>> m_worldButtons;
        std::shared_ptr<UIButton> m_createWorldButton;
        std::shared_ptr<UIButton> m_backButton;
        std::shared_ptr<UIText> m_titleText;

        std::function<void(const std::string&)> m_onWorldSelected;
        std::function<void()> m_onCreateWorld;
        std::function<void()> m_onBack;

        void InitializeMenu();
    };

    /**
     * @class UICreateWorldMenu
     * @brief Create new world menu
     */
    class UICreateWorldMenu : public UIElement {
    public:
        UICreateWorldMenu(const std::string& id);
        ~UICreateWorldMenu() override = default;

        void SetOnCreate(std::function<void(const std::string&, const std::string&)> callback) {
            m_onCreate = callback;
        }
        void SetOnCancel(std::function<void()> callback) { m_onCancel = callback; }

        void Show() { SetVisible(true); }
        void Hide() { SetVisible(false); }

        void Update(float deltaTime) override;
        void Render() override;
        bool HandleInput(const UIEvent& event) override;

    private:
        std::shared_ptr<UIText> m_worldNameLabel;
        std::shared_ptr<UIText> m_worldTypeLabel;
        std::shared_ptr<UIButton> m_createButton;
        std::shared_ptr<UIButton> m_cancelButton;
        std::shared_ptr<UIText> m_titleText;

        std::string m_worldName;
        std::string m_worldType = "default";

        std::function<void(const std::string&, const std::string&)> m_onCreate;
        std::function<void()> m_onCancel;

        void InitializeMenu();
        bool ValidateWorldName(const std::string& name);
    };

    /**
     * @class UIOptionsMenu
     * @brief Game options menu
     */
    class UIOptionsMenu : public UIElement {
    public:
        UIOptionsMenu(const std::string& id);
        ~UIOptionsMenu() override = default;

        void LoadSettings();
        void SaveSettings();

        void SetOnBack(std::function<void()> callback) { m_onBack = callback; }

        void Show() { SetVisible(true); LoadSettings(); }
        void Hide() { SetVisible(false); SaveSettings(); }

        void Update(float deltaTime) override;
        void Render() override;
        bool HandleInput(const UIEvent& event) override;

    private:
        // Video settings
        std::shared_ptr<UIText> m_videoLabel;
        std::shared_ptr<UISlider> m_resolutionSlider;
        std::shared_ptr<UISlider> m_brightnessSlider;
        std::shared_ptr<UICheckbox> m_fullscreenCheckbox;
        std::shared_ptr<UICheckbox> m_vsyncCheckbox;

        // Audio settings
        std::shared_ptr<UIText> m_audioLabel;
        std::shared_ptr<UISlider> m_masterVolumeSlider;
        std::shared_ptr<UISlider> m_musicVolumeSlider;
        std::shared_ptr<UISlider> m_sfxVolumeSlider;

        // Controls
        std::shared_ptr<UIText> m_controlsLabel;
        std::shared_ptr<UISlider> m_mouseSensitivitySlider;
        std::shared_ptr<UICheckbox> m_invertMouseCheckbox;

        // Buttons
        std::shared_ptr<UIButton> m_applyButton;
        std::shared_ptr<UIButton> m_backButton;
        std::shared_ptr<UIButton> m_resetButton;

        std::function<void()> m_onBack;

        void InitializeMenu();
        void ApplySettings();
        void ResetSettings();
    };

    /**
     * @class UIPauseMenu
     * @brief In-game pause menu
     */
    class UIPauseMenu : public UIElement {
    public:
        UIPauseMenu(const std::string& id);
        ~UIPauseMenu() override = default;

        void SetOnResume(std::function<void()> callback) { m_onResume = callback; }
        void SetOnOptions(std::function<void()> callback) { m_onOptions = callback; }
        void SetOnSaveAndQuit(std::function<void()> callback) { m_onSaveAndQuit = callback; }
        void SetOnQuitToTitle(std::function<void()> callback) { m_onQuitToTitle = callback; }

        void Show() { SetVisible(true); }
        void Hide() { SetVisible(false); }

        void Update(float deltaTime) override;
        void Render() override;
        bool HandleInput(const UIEvent& event) override;

    private:
        std::shared_ptr<UIButton> m_resumeButton;
        std::shared_ptr<UIButton> m_optionsButton;
        std::shared_ptr<UIButton> m_saveAndQuitButton;
        std::shared_ptr<UIButton> m_quitToTitleButton;
        std::shared_ptr<UIText> m_titleText;

        std::function<void()> m_onResume;
        std::function<void()> m_onOptions;
        std::function<void()> m_onSaveAndQuit;
        std::function<void()> m_onQuitToTitle;

        void InitializeMenu();
    };

    /**
     * @class UIDeathScreen
     * @brief Game over screen
     */
    class UIDeathScreen : public UIElement {
    public:
        UIDeathScreen(const std::string& id);
        ~UIDeathScreen() override = default;

        void SetDeathMessage(const std::string& message) { m_deathMessage = message; }

        void SetOnRespawn(std::function<void()> callback) { m_onRespawn = callback; }
        void SetOnTitleScreen(std::function<void()> callback) { m_onTitleScreen = callback; }

        void Show() { SetVisible(true); }
        void Hide() { SetVisible(false); }

        void Update(float deltaTime) override;
        void Render() override;
        bool HandleInput(const UIEvent& event) override;

    private:
        std::shared_ptr<UIText> m_deathMessageText;
        std::shared_ptr<UIButton> m_respawnButton;
        std::shared_ptr<UIButton> m_titleScreenButton;
        std::shared_ptr<UIText> m_titleText;

        std::string m_deathMessage = "You died!";
        float m_animationTimer = 0.0f;

        std::function<void()> m_onRespawn;
        std::function<void()> m_onTitleScreen;

        void InitializeScreen();
    };

    /**
     * @class UILoadingScreen
     * @brief Loading screen with progress
     */
    class UILoadingScreen : public UIElement {
    public:
        UILoadingScreen(const std::string& id);
        ~UILoadingScreen() override = default;

        void SetProgress(float progress) { m_progress = std::max(0.0f, std::min(1.0f, progress)); }
        float GetProgress() const { return m_progress; }

        void SetLoadingText(const std::string& text) { m_loadingText = text; }
        void SetTipText(const std::string& tip) { m_tipText = tip; }

        void Show() { SetVisible(true); }
        void Hide() { SetVisible(false); }

        void Update(float deltaTime) override;
        void Render() override;

    private:
        float m_progress = 0.0f;
        std::string m_loadingText = "Loading...";
        std::string m_tipText;
        std::shared_ptr<UIProgressBar> m_progressBar;
        std::shared_ptr<UIText> m_loadingTextElement;
        std::shared_ptr<UIText> m_tipTextElement;
        std::shared_ptr<UIText> m_percentageText;

        void InitializeScreen();
        std::string GetRandomTip();
    };

    /**
     * @class UIMultiplayerMenu
     * @brief Multiplayer connection menu
     */
    class UIMultiplayerMenu : public UIElement {
    public:
        UIMultiplayerMenu(const std::string& id);
        ~UIMultiplayerMenu() override = default;

        void SetOnConnect(std::function<void(const std::string&, int)> callback) {
            m_onConnect = callback;
        }
        void SetOnDirectConnect(std::function<void(const std::string&, int)> callback) {
            m_onDirectConnect = callback;
        }
        void SetOnBack(std::function<void()> callback) { m_onBack = callback; }

        void Show() { SetVisible(true); }
        void Hide() { SetVisible(false); }

        void Update(float deltaTime) override;
        void Render() override;
        bool HandleInput(const UIEvent& event) override;

    private:
        std::shared_ptr<UIButton> m_directConnectButton;
        std::shared_ptr<UIButton> m_serverListButton;
        std::shared_ptr<UIButton> m_backButton;
        std::shared_ptr<UIText> m_titleText;

        std::string m_serverAddress = "localhost";
        int m_serverPort = 25565;

        std::function<void(const std::string&, int)> m_onConnect;
        std::function<void(const std::string&, int)> m_onDirectConnect;
        std::function<void()> m_onBack;

        void InitializeMenu();
    };

    /**
     * @class UIDirectConnectMenu
     * @brief Direct server connection menu
     */
    class UIDirectConnectMenu : public UIElement {
    public:
        UIDirectConnectMenu(const std::string& id);
        ~UIDirectConnectMenu() override = default;

        void SetOnConnect(std::function<void(const std::string&, int)> callback) {
            m_onConnect = callback;
        }
        void SetOnCancel(std::function<void()> callback) { m_onCancel = callback; }

        void Show() { SetVisible(true); }
        void Hide() { SetVisible(false); }

        void Update(float deltaTime) override;
        void Render() override;
        bool HandleInput(const UIEvent& event) override;

    private:
        std::shared_ptr<UIText> m_addressLabel;
        std::shared_ptr<UIText> m_portLabel;
        std::shared_ptr<UIButton> m_connectButton;
        std::shared_ptr<UIButton> m_cancelButton;

        std::string m_serverAddress = "localhost";
        std::string m_serverPort = "25565";

        std::function<void(const std::string&, int)> m_onConnect;
        std::function<void()> m_onCancel;

        void InitializeMenu();
        bool ValidateConnectionInfo();
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_UI_UI_MENUS_HPP
