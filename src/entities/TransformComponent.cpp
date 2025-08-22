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
        , m_position(0.0f, 0.0f, 0.0f)
        , m_rotation(1.0f, 0.0f, 0.0f, 0.0f) // Identity quaternion
        , m_scale(1.0f, 1.0f, 1.0f)
        , m_localMatrix(1.0f)
        , m_worldMatrix(1.0f)
        , m_isDirty(true)
        , m_parent(nullptr)
    {
        VOXELCRAFT_TRACE("TransformComponent created for entity '{}'",
                        owner ? owner->GetName() : "null");
    }

    TransformComponent::TransformComponent(Entity* owner, const glm::vec3& position)
        : Component(owner, "TransformComponent")
        , m_position(position)
        , m_rotation(1.0f, 0.0f, 0.0f, 0.0f) // Identity quaternion
        , m_scale(1.0f, 1.0f, 1.0f)
        , m_localMatrix(1.0f)
        , m_worldMatrix(1.0f)
        , m_isDirty(true)
        , m_parent(nullptr)
    {
        VOXELCRAFT_TRACE("TransformComponent created for entity '{}' at position ({}, {}, {})",
                        owner ? owner->GetName() : "null", position.x, position.y, position.z);
    }

    TransformComponent::TransformComponent(Entity* owner,
                                         const glm::vec3& position,
                                         const glm::quat& rotation,
                                         const glm::vec3& scale)
        : Component(owner, "TransformComponent")
        , m_position(position)
        , m_rotation(rotation)
        , m_scale(scale)
        , m_localMatrix(1.0f)
        , m_worldMatrix(1.0f)
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

    void TransformComponent::SetPosition(const glm::vec3& position) {
        if (m_position != position) {
            m_position = position;
            MarkDirty();
        }
    }

    void TransformComponent::SetPosition(float x, float y, float z) {
        SetPosition(glm::vec3(x, y, z));
    }

    void TransformComponent::Translate(const glm::vec3& translation) {
        SetPosition(m_position + translation);
    }

    void TransformComponent::Translate(float x, float y, float z) {
        Translate(glm::vec3(x, y, z));
    }

    // Rotation operations

    void TransformComponent::SetRotation(const glm::quat& rotation) {
        if (m_rotation != rotation) {
            m_rotation = rotation;
            MarkDirty();
        }
    }

    void TransformComponent::SetRotation(float pitch, float yaw, float roll) {
        glm::quat rotation = glm::quat(glm::vec3(
            glm::radians(pitch),
            glm::radians(yaw),
            glm::radians(roll)
        ));
        SetRotation(rotation);
    }

    void TransformComponent::Rotate(const glm::quat& rotation) {
        SetRotation(m_rotation * rotation);
    }

    void TransformComponent::Rotate(const glm::vec3& axis, float angle) {
        Rotate(glm::angleAxis(glm::radians(angle), glm::normalize(axis)));
    }

    glm::vec3 TransformComponent::GetForward() const {
        return glm::normalize(m_rotation * glm::vec3(0.0f, 0.0f, -1.0f));
    }

    glm::vec3 TransformComponent::GetRight() const {
        return glm::normalize(m_rotation * glm::vec3(1.0f, 0.0f, 0.0f));
    }

    glm::vec3 TransformComponent::GetUp() const {
        return glm::normalize(m_rotation * glm::vec3(0.0f, 1.0f, 0.0f));
    }

    void TransformComponent::LookAt(const glm::vec3& target, const glm::vec3& up) {
        glm::vec3 direction = glm::normalize(target - m_position);
        glm::quat rotation = glm::quatLookAt(direction, up);
        SetRotation(rotation);
    }

    // Scale operations

    void TransformComponent::SetScale(const glm::vec3& scale) {
        if (m_scale != scale) {
            m_scale = scale;
            MarkDirty();
        }
    }

    void TransformComponent::SetScale(float scale) {
        SetScale(glm::vec3(scale, scale, scale));
    }

    void TransformComponent::SetScale(float x, float y, float z) {
        SetScale(glm::vec3(x, y, z));
    }

    void TransformComponent::Scale(const glm::vec3& scale) {
        SetScale(m_scale * scale);
    }

    void TransformComponent::Scale(float scale) {
        SetScale(m_scale * scale);
    }

    // Matrix operations

    glm::mat4 TransformComponent::GetWorldMatrix() const {
        UpdateMatrices();
        return m_worldMatrix;
    }

    glm::mat4 TransformComponent::GetLocalMatrix() const {
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
        m_localMatrix = glm::mat4(1.0f);
        m_localMatrix = glm::translate(m_localMatrix, m_position);
        m_localMatrix = m_localMatrix * glm::mat4_cast(m_rotation);
        m_localMatrix = glm::scale(m_localMatrix, m_scale);

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

    glm::vec3 TransformComponent::GetWorldPosition() const {
        if (m_parent) {
            return glm::vec3(m_parent->GetWorldMatrix() * glm::vec4(m_position, 1.0f));
        }
        return m_position;
    }

    glm::quat TransformComponent::GetWorldRotation() const {
        if (m_parent) {
            return m_parent->GetWorldRotation() * m_rotation;
        }
        return m_rotation;
    }

    glm::vec3 TransformComponent::GetWorldScale() const {
        if (m_parent) {
            return m_parent->GetWorldScale() * m_scale;
        }
        return m_scale;
    }

    glm::mat4 TransformComponent::GetWorldMatrixHierarchy() const {
        UpdateMatrices();
        return m_worldMatrix;
    }

} // namespace VoxelCraft
