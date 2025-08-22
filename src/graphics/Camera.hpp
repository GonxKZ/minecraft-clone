/**
 * @file Camera.hpp
 * @brief VoxelCraft Camera System
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_GRAPHICS_CAMERA_HPP
#define VOXELCRAFT_GRAPHICS_CAMERA_HPP

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>

namespace VoxelCraft {

struct Vec3 {
    float x, y, z;
    Vec3() : x(0), y(0), z(0) {}
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}
};

struct Vec2 {
    float x, y;
    Vec2() : x(0), y(0) {}
    Vec2(float x, float y) : x(x), y(y) {}
};

class Camera {
public:
    Camera();
    ~Camera();

    void Update(double deltaTime);
    void SetPosition(const Vec3& position);
    void SetRotation(float pitch, float yaw);
    Vec3 GetPosition() const { return m_position; }
    Vec3 GetRotation() const { return Vec3(m_pitch, m_yaw, 0); }
    Vec3 GetForward() const;
    Vec3 GetRight() const;
    Vec3 GetUp() const;

    Vec3 position;
    float m_pitch;
    float m_yaw;

private:
    Vec3 m_position;
};

} // namespace VoxelCraft

#endif // VOXELCRAFT_GRAPHICS_CAMERA_HPP
