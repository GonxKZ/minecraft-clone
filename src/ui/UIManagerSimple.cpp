/**
 * @file UIManagerSimple.cpp
 * @brief VoxelCraft Simple UI Manager Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#include "UIManagerSimple.hpp"
#include "../core/Logger.hpp"
#include <iostream>
#include <GLFW/glfw3.h>

namespace VoxelCraft {

    UIManagerSimple::UIManagerSimple(std::shared_ptr<Window> window, std::shared_ptr<Config> config)
        : m_window(window)
        , m_config(config)
        , m_state(UIState::HUD)
        , m_showDebugInfo(false)
    {
        VOXELCRAFT_TRACE("UIManagerSimple created");
    }

    UIManagerSimple::~UIManagerSimple() {
        VOXELCRAFT_TRACE("UIManagerSimple destroyed");
    }

    bool UIManagerSimple::Initialize() {
        VOXELCRAFT_INFO("Initializing Simple UI System");

        if (!m_window) {
            VOXELCRAFT_ERROR("No window provided to UIManagerSimple");
            return false;
        }

        VOXELCRAFT_INFO("Simple UI System initialized successfully");
        return true;
    }

    void UIManagerSimple::Update(float deltaTime) {
        // Update UI state based on game state
        // This would be expanded with actual game state management

        // For now, just ensure we have a valid state
        if (m_state == UIState::NONE) {
            m_state = UIState::HUD;
        }
    }

    void UIManagerSimple::Render() {
        switch (m_state) {
            case UIState::HUD:
                RenderHUD();
                if (m_showDebugInfo) {
                    RenderDebugInfo();
                }
                break;

            case UIState::PAUSE_MENU:
                RenderPauseMenu();
                break;

            case UIState::MAIN_MENU:
                RenderMainMenu();
                break;

            case UIState::INVENTORY:
                // Render inventory UI
                RenderHUD(); // Show HUD behind inventory
                break;

            case UIState::NONE:
            default:
                // No UI to render
                break;
        }
    }

    void UIManagerSimple::HandleInput(int key, int action) {
        if (action != GLFW_PRESS) return;

        switch (key) {
            case GLFW_KEY_ESCAPE:
                if (m_state == UIState::HUD) {
                    SetState(UIState::PAUSE_MENU);
                } else if (m_state == UIState::PAUSE_MENU) {
                    SetState(UIState::HUD);
                }
                break;

            case GLFW_KEY_F3:
                m_showDebugInfo = !m_showDebugInfo;
                break;

            case GLFW_KEY_E:
                if (m_state == UIState::HUD) {
                    SetState(UIState::INVENTORY);
                } else if (m_state == UIState::INVENTORY) {
                    SetState(UIState::HUD);
                }
                break;
        }
    }

    void UIManagerSimple::SetPlayer(std::shared_ptr<Player> player) {
        m_player = player;
    }

    void UIManagerSimple::SetState(UIState state) {
        m_state = state;
        VOXELCRAFT_INFO("UI state changed to: {}",
            state == UIState::HUD ? "HUD" :
            state == UIState::PAUSE_MENU ? "PAUSE_MENU" :
            state == UIState::INVENTORY ? "INVENTORY" :
            state == UIState::MAIN_MENU ? "MAIN_MENU" : "NONE");
    }

    void UIManagerSimple::RenderHUD() {
        // Simple HUD rendering
        // In a real implementation, this would use proper text rendering

        if (m_player) {
            auto& stats = m_player->GetStats();

            // Render health
            std::string healthText = "Health: " + std::to_string(static_cast<int>(stats.health)) +
                                   "/" + std::to_string(static_cast<int>(stats.maxHealth));
            RenderText(healthText, 10.0f, 10.0f);

            // Render position (simplified)
            auto pos = m_player->GetPosition();
            std::string posText = "Pos: " + std::to_string(static_cast<int>(pos.x)) + ", " +
                                std::to_string(static_cast<int>(pos.y)) + ", " +
                                std::to_string(static_cast<int>(pos.z));
            RenderText(posText, 10.0f, 30.0f);
        } else {
            RenderText("No player connected", 10.0f, 10.0f);
        }
    }

    void UIManagerSimple::RenderDebugInfo() {
        // Simple debug info
        RenderText("DEBUG MODE - F3 to toggle", 10.0f, 50.0f);

        if (m_player) {
            auto& stats = m_player->GetStats();
            std::string debugText = "Grounded: " + std::string(stats.isGrounded ? "Yes" : "No") +
                                  " Sneaking: " + std::string(stats.isSneaking ? "Yes" : "No") +
                                  " Sprinting: " + std::string(stats.isSprinting ? "Yes" : "No");
            RenderText(debugText, 10.0f, 70.0f);
        }
    }

    void UIManagerSimple::RenderPauseMenu() {
        // Simple pause menu
        RenderText("PAUSED", 400.0f, 200.0f);
        RenderText("Press ESC to resume", 350.0f, 230.0f);
        RenderText("Press Q to quit", 370.0f, 260.0f);
    }

    void UIManagerSimple::RenderMainMenu() {
        // Simple main menu
        RenderText("VOXELCRAFT", 350.0f, 150.0f);
        RenderText("Press ENTER to start", 320.0f, 200.0f);
        RenderText("Press ESC to exit", 330.0f, 230.0f);
    }

    void UIManagerSimple::RenderText(const std::string& text, float x, float y) {
        // Placeholder text rendering
        // In a real implementation, this would use OpenGL text rendering
        std::cout << "[UI] " << text << " (at " << x << ", " << y << ")" << std::endl;

        // For now, just log the text since we don't have a proper text renderer
        VOXELCRAFT_TRACE("UI Text: {} at ({}, {})", text, x, y);
    }

} // namespace VoxelCraft
