/**
 * @file TransformComponent.cpp
 * @brief VoxelCraft ECS - Transform Component Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#include "TransformComponent.hpp"
#include "Entity.hpp"
#include "Logger.hpp"

#include <algorithm>

namespace VoxelCraft {

    TransformComponent::TransformComponent(Entity* owner)
    : Component(owner, "TransformComponent")
    , m_position(Vec3::zero())
    , m_rotation(Quat()) // Identity quaternion
    , m_scale(1.0f, 1.0f, 1.0f)
    , m_localMatrix(Mat4::identity())
    , m_worldMatrix(Mat4::identity())
    , m_isDirty(true)
    , m_parent(nullptr)
{
    VOXELCRAFT_TRACE("TransformComponent created for entity '{}'",
                    owner ? owner->GetName() : "null");
}

TransformComponent::TransformComponent(Entity* owner, const Vec3& position)
    : Component(owner, "TransformComponent")
    , m_position(position)
    , m_rotation(Quat()) // Identity quaternion
    , m_scale(1.0f, 1.0f, 1.0f)
    , m_localMatrix(Mat4::identity())
    , m_worldMatrix(Mat4::identity())
    , m_isDirty(true)
    , m_parent(nullptr)
{
    VOXELCRAFT_TRACE("TransformComponent created for entity '{}' at position ({}, {}, {})",
                    owner ? owner->GetName() : "null", position.x, position.y, position.z);
}

TransformComponent::TransformComponent(Entity* owner,
                                     const Vec3& position,
                                     const Quat& rotation,
                                     const Vec3& scale)
    : Component(owner, "TransformComponent")
    , m_position(position)
    , m_rotation(rotation)
    , m_scale(scale)
    , m_localMatrix(Mat4::identity())
    , m_worldMatrix(Mat4::identity())
    , m_isDirty(true)
    , m_parent(nullptr)
{
    VOXELCRAFT_TRACE("TransformComponent created for entity '{}' with full transform",
                    owner ? owner->GetName() : "null");
}

    TransformComponent::~TransformComponent() {
        VOXELCRAFT_TRACE("TransformComponent destroyed for entity '{}'",
                        GetOwner() ? GetOwner()->GetName() : "null");

        // Remove from parent
        if (m_parent) {
            m_parent->RemoveChild(this);
        }

        // Orphan children
        for (TransformComponent* child : m_children) {
            child->m_parent = nullptr;
        }
        m_children.clear();
    }

    bool TransformComponent::Initialize() {
        VOXELCRAFT_TRACE("TransformComponent initialized for entity '{}'",
                        GetOwner() ? GetOwner()->GetName() : "null");
        return true;
    }

    void TransformComponent::Update(double deltaTime) {
        // Transform components typically don't need per-frame updates
        // unless they have velocity, interpolation, or other dynamic behavior
        // This can be overridden in derived classes if needed
    }

    // Position operations

    void TransformComponent::SetPosition(const Vec3& position) {
    if (m_position.x != position.x || m_position.y != position.y || m_position.z != position.z) {
        m_position = position;
        MarkDirty();
    }
}

void TransformComponent::SetPosition(float x, float y, float z) {
    SetPosition(Vec3(x, y, z));
}

void TransformComponent::Translate(const Vec3& translation) {
    SetPosition(m_position + translation);
}

void TransformComponent::Translate(float x, float y, float z) {
    Translate(Vec3(x, y, z));
}

    // Rotation operations

    void TransformComponent::SetRotation(const Quat& rotation) {
        if (m_rotation.w != rotation.w || m_rotation.x != rotation.x ||
            m_rotation.y != rotation.y || m_rotation.z != rotation.z) {
            m_rotation = rotation;
            MarkDirty();
        }
    }

    void TransformComponent::SetRotation(float pitch, float yaw, float roll) {
        Quat rotation = Quat::fromEuler(pitch, yaw, roll);
        SetRotation(rotation);
    }

    void TransformComponent::Rotate(const Quat& rotation) {
        SetRotation(m_rotation * rotation);
    }

    void TransformComponent::Rotate(const Vec3& axis, float angle) {
        // Create quaternion for rotation around axis
        Vec3 normalizedAxis = axis.normalize();
        float halfAngle = angle * 0.0174532925f * 0.5f; // degrees to radians
        float sinHalf = std::sin(halfAngle);
        float cosHalf = std::cos(halfAngle);

        Quat axisRotation(
            cosHalf,
            normalizedAxis.x * sinHalf,
            normalizedAxis.y * sinHalf,
            normalizedAxis.z * sinHalf
        );

        Rotate(axisRotation);
    }

    Vec3 TransformComponent::GetForward() const {
        return (m_rotation * Vec3::forward()).normalize();
    }

    Vec3 TransformComponent::GetRight() const {
        return (m_rotation * Vec3::right()).normalize();
    }

    Vec3 TransformComponent::GetUp() const {
        return (m_rotation * Vec3::up()).normalize();
    }

    void TransformComponent::LookAt(const Vec3& target, const Vec3& up) {
        Vec3 direction = (target - m_position).normalize();
        Vec3 right = up.cross(direction).normalize();
        Vec3 newUp = direction.cross(right).normalize();

        // Build rotation matrix
        Mat4 rotationMatrix;
        rotationMatrix.data[0] = right.x;
        rotationMatrix.data[1] = right.y;
        rotationMatrix.data[2] = right.z;

        rotationMatrix.data[4] = newUp.x;
        rotationMatrix.data[5] = newUp.y;
        rotationMatrix.data[6] = newUp.z;

        rotationMatrix.data[8] = direction.x;
        rotationMatrix.data[9] = direction.y;
        rotationMatrix.data[10] = direction.z;

        // Convert to quaternion (simplified)
        float trace = rotationMatrix.data[0] + rotationMatrix.data[5] + rotationMatrix.data[10];
        Quat q;

        if (trace > 0.0f) {
            float s = 0.5f / std::sqrt(trace + 1.0f);
            q.w = 0.25f / s;
            q.x = (rotationMatrix.data[6] - rotationMatrix.data[9]) * s;
            q.y = (rotationMatrix.data[8] - rotationMatrix.data[2]) * s;
            q.z = (rotationMatrix.data[1] - rotationMatrix.data[4]) * s;
        } else {
            if (rotationMatrix.data[0] > rotationMatrix.data[5] && rotationMatrix.data[0] > rotationMatrix.data[10]) {
                float s = 2.0f * std::sqrt(1.0f + rotationMatrix.data[0] - rotationMatrix.data[5] - rotationMatrix.data[10]);
                q.w = (rotationMatrix.data[6] - rotationMatrix.data[9]) / s;
                q.x = 0.25f * s;
                q.y = (rotationMatrix.data[1] + rotationMatrix.data[4]) / s;
                q.z = (rotationMatrix.data[8] + rotationMatrix.data[2]) / s;
            } else if (rotationMatrix.data[5] > rotationMatrix.data[10]) {
                float s = 2.0f * std::sqrt(1.0f + rotationMatrix.data[5] - rotationMatrix.data[0] - rotationMatrix.data[10]);
                q.w = (rotationMatrix.data[8] - rotationMatrix.data[2]) / s;
                q.x = (rotationMatrix.data[1] + rotationMatrix.data[4]) / s;
                q.y = 0.25f * s;
                q.z = (rotationMatrix.data[6] + rotationMatrix.data[9]) / s;
            } else {
                float s = 2.0f * std::sqrt(1.0f + rotationMatrix.data[10] - rotationMatrix.data[0] - rotationMatrix.data[5]);
                q.w = (rotationMatrix.data[1] - rotationMatrix.data[4]) / s;
                q.x = (rotationMatrix.data[8] + rotationMatrix.data[2]) / s;
                q.y = (rotationMatrix.data[6] + rotationMatrix.data[9]) / s;
                q.z = 0.25f * s;
            }
        }

        SetRotation(q.normalize());
    }

    // Scale operations

    void TransformComponent::SetScale(const Vec3& scale) {
        if (m_scale.x != scale.x || m_scale.y != scale.y || m_scale.z != scale.z) {
            m_scale = scale;
            MarkDirty();
        }
    }

    void TransformComponent::SetScale(float scale) {
        SetScale(Vec3(scale, scale, scale));
    }

    void TransformComponent::SetScale(float x, float y, float z) {
        SetScale(Vec3(x, y, z));
    }

    void TransformComponent::Scale(const Vec3& scale) {
        SetScale(Vec3(m_scale.x * scale.x, m_scale.y * scale.y, m_scale.z * scale.z));
    }

    void TransformComponent::Scale(float scale) {
        SetScale(Vec3(m_scale.x * scale, m_scale.y * scale, m_scale.z * scale));
    }

    // Matrix operations

    Mat4 TransformComponent::GetWorldMatrix() const {
        UpdateMatrices();
        return m_worldMatrix;
    }

    Mat4 TransformComponent::GetLocalMatrix() const {
        UpdateMatrices();
        return m_localMatrix;
    }

    void TransformComponent::MarkDirty() {
        m_isDirty = true;

        // Mark all children as dirty
        for (TransformComponent* child : m_children) {
            child->MarkDirty();
        }
    }

    void TransformComponent::UpdateMatrices() const {
        if (!m_isDirty) {
            return;
        }

        // Build local transformation matrix
        // Order: Scale -> Rotate -> Translate (TRS order)
        Mat4 scaleMatrix = Mat4::scale(m_scale);
        Mat4 rotationMatrix = Mat4::rotate(m_rotation);
        Mat4 translationMatrix = Mat4::translate(m_position);

        // Combine transformations: T * R * S
        m_localMatrix = translationMatrix * rotationMatrix * scaleMatrix;

        // Calculate world matrix
        if (m_parent) {
            m_worldMatrix = m_parent->GetWorldMatrix() * m_localMatrix;
        } else {
            m_worldMatrix = m_localMatrix;
        }

        m_isDirty = false;
    }

    // Hierarchy operations

    void TransformComponent::SetParent(TransformComponent* parent) {
        if (m_parent == parent) {
            return;
        }

        // Remove from current parent
        if (m_parent) {
            m_parent->RemoveChild(this);
        }

        m_parent = parent;

        // Add to new parent
        if (parent) {
            parent->AddChild(this);
        }

        MarkDirty();
    }

    void TransformComponent::AddChild(TransformComponent* child) {
        if (!child || child == this) {
            return;
        }

        // Check if already a child
        auto it = std::find(m_children.begin(), m_children.end(), child);
        if (it == m_children.end()) {
            m_children.push_back(child);
            child->m_parent = this;
            child->MarkDirty();
        }
    }

    void TransformComponent::RemoveChild(TransformComponent* child) {
        if (!child) {
            return;
        }

        auto it = std::find(m_children.begin(), m_children.end(), child);
        if (it != m_children.end()) {
            m_children.erase(it);
            child->m_parent = nullptr;
            child->MarkDirty();
        }
    }

    Vec3 TransformComponent::GetWorldPosition() const {
        if (m_parent) {
            return m_parent->GetWorldMatrix() * m_position;
        }
        return m_position;
    }

    Quat TransformComponent::GetWorldRotation() const {
        if (m_parent) {
            return m_parent->GetWorldRotation() * m_rotation;
        }
        return m_rotation;
    }

    Vec3 TransformComponent::GetWorldScale() const {
        if (m_parent) {
            Vec3 parentScale = m_parent->GetWorldScale();
            return Vec3(parentScale.x * m_scale.x, parentScale.y * m_scale.y, parentScale.z * m_scale.z);
        }
        return m_scale;
    }

    Mat4 TransformComponent::GetWorldMatrixHierarchy() const {
        UpdateMatrices();
        return m_worldMatrix;
    }

} // namespace VoxelCraft
