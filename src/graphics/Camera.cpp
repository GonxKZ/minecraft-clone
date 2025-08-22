/**
 * @file Camera.cpp
 * @brief VoxelCraft Camera Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#include "Camera.hpp"
#include "Logger.hpp"
#include <cmath>

namespace VoxelCraft {

Camera::Camera()
    : m_position(0, 0, 0)
    , m_pitch(0)
    , m_yaw(0)
    , position(0, 0, 0)
{
}

Camera::~Camera() {
}

void Camera::Update(double deltaTime) {
    // Update camera logic here
    m_position = position;
}

void Camera::SetPosition(const Vec3& pos) {
    position = pos;
    m_position = pos;
}

void Camera::SetRotation(float pitch, float yaw) {
    m_pitch = pitch;
    m_yaw = yaw;
}

Vec3 Camera::GetForward() const {
    Vec3 forward;
    forward.x = cos(m_yaw) * cos(m_pitch);
    forward.y = sin(m_pitch);
    forward.z = sin(m_yaw) * cos(m_pitch);
    return forward;
}

Vec3 Camera::GetRight() const {
    Vec3 right;
    right.x = -sin(m_yaw);
    right.y = 0;
    right.z = cos(m_yaw);
    return right;
}

Vec3 Camera::GetUp() const {
    Vec3 up = GetRight();
    Vec3 forward = GetForward();

    up.x = forward.y * up.z - forward.z * up.y;
    up.y = forward.z * up.x - forward.x * up.z;
    up.z = forward.x * up.y - forward.y * up.x;

    return up;
}

} // namespace VoxelCraft
