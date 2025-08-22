/**
 * @file UIManager.hpp
 * @brief VoxelCraft UI Management System
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_UI_UI_MANAGER_HPP
#define VOXELCRAFT_UI_UI_MANAGER_HPP

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>

namespace VoxelCraft {

// Forward declarations
class Window;
class Config;
class Player;
struct Vec3;

class UIManager {
public:
    UIManager();
    ~UIManager();

    bool Initialize();
    void Shutdown();
    void Update(double deltaTime);
    void Render();
    void HandleInput(int key, int action);

    void SetPlayer(Player* player);
    void SetWindow(Window* window);
    void SetConfig(const Config* config);

private:
    Window* m_window;
    const Config* m_config;
    Player* m_player;
    bool m_initialized;
};

} // namespace VoxelCraft

#endif // VOXELCRAFT_UI_UI_MANAGER_HPP