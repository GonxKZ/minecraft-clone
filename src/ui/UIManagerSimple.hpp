/**
 * @file UIManagerSimple.hpp
 * @brief VoxelCraft Simple UI Manager - Basic HUD and Menus
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines a simple UI system for the VoxelCraft game engine,
 * providing basic HUD elements and menus integrated with the ECS.
 */

#ifndef VOXELCRAFT_UI_UI_MANAGER_SIMPLE_HPP
#define VOXELCRAFT_UI_UI_MANAGER_SIMPLE_HPP

#include <memory>
#include <vector>
#include <string>
#include <functional>

#include "../core/Config.hpp"
#include "../player/Player.hpp"
#include "../window/Window.hpp"
#include "../entities/System.hpp"

namespace VoxelCraft {

    /**
     * @enum UIState
     * @brief Current UI state
     */
    enum class UIState {
        NONE,           // No UI shown
        HUD,            // In-game HUD
        PAUSE_MENU,     // Pause menu
        INVENTORY,      // Player inventory
        MAIN_MENU       // Main menu
    };

    /**
     * @class UIManagerSimple
     * @brief Simple UI system integrated with ECS
     */
    class UIManagerSimple : public System {
    public:
        /**
         * @brief Constructor
         * @param window Game window
         * @param config Configuration system
         */
        UIManagerSimple(std::shared_ptr<Window> window, std::shared_ptr<Config> config);

        /**
         * @brief Destructor
         */
        ~UIManagerSimple();

        /**
         * @brief Initialize UI system
         * @return true if successful
         */
        bool Initialize();

        /**
         * @brief Update UI system
         * @param deltaTime Time since last update
         */
        void Update(float deltaTime) override;

        /**
         * @brief Render UI elements
         */
        void Render();

        /**
         * @brief Handle input events
         * @param key Key code
         * @param action Key action
         */
        void HandleInput(int key, int action);

        /**
         * @brief Set player reference for HUD
         * @param player Player entity
         */
        void SetPlayer(std::shared_ptr<Player> player);

        /**
         * @brief Get current UI state
         * @return UI state
         */
        UIState GetState() const { return m_state; }

        /**
         * @brief Set UI state
         * @param state New UI state
         */
        void SetState(UIState state);

        /**
         * @brief Show/hide debug info
         * @param show Whether to show debug info
         */
        void ShowDebugInfo(bool show) { m_showDebugInfo = show; }

    private:
        std::shared_ptr<Window> m_window;
        std::shared_ptr<Config> m_config;
        std::shared_ptr<Player> m_player;

        UIState m_state;
        bool m_showDebugInfo;

        /**
         * @brief Render HUD elements
         */
        void RenderHUD();

        /**
         * @brief Render debug information
         */
        void RenderDebugInfo();

        /**
         * @brief Render pause menu
         */
        void RenderPauseMenu();

        /**
         * @brief Render main menu
         */
        void RenderMainMenu();

        /**
         * @brief Simple text rendering (placeholder)
         * @param text Text to render
         * @param x X position
         * @param y Y position
         */
        void RenderText(const std::string& text, float x, float y);
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_UI_UI_MANAGER_SIMPLE_HPP
