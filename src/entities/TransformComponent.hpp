/**
 * @file TransformComponent.hpp
 * @brief VoxelCraft ECS - Transform Component
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * Transform component for position, rotation, and scale in 3D space.
 */

#ifndef VOXELCRAFT_ENTITIES_TRANSFORM_COMPONENT_HPP
#define VOXELCRAFT_ENTITIES_TRANSFORM_COMPONENT_HPP

#include "Component.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace VoxelCraft {

    class TransformComponent : public Component {
    public:
        /**
         * @brief Constructor
         * @param owner Entity that owns this component
         */
        TransformComponent(Entity* owner);

        /**
         * @brief Constructor with initial position
         * @param owner Entity that owns this component
         * @param position Initial position
         */
        TransformComponent(Entity* owner, const glm::vec3& position);

        /**
         * @brief Constructor with full transform
         * @param owner Entity that owns this component
         * @param position Initial position
         * @param rotation Initial rotation
         * @param scale Initial scale
         */
        TransformComponent(Entity* owner,
                          const glm::vec3& position,
                          const glm::quat& rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
                          const glm::vec3& scale = glm::vec3(1.0f));

        /**
         * @brief Destructor
         */
        ~TransformComponent() override;

        /**
         * @brief Get component type name
         * @return Component type name
         */
        std::string GetTypeName() const override { return "TransformComponent"; }

        /**
         * @brief Initialize the component
         * @return true if successful, false otherwise
         */
        bool Initialize() override;

        /**
         * @brief Update the component
         * @param deltaTime Time elapsed since last update
         */
        void Update(double deltaTime) override;

        // Position operations

        /**
         * @brief Set position
         * @param position New position
         */
        void SetPosition(const glm::vec3& position);

        /**
         * @brief Set position with individual coordinates
         * @param x X coordinate
         * @param y Y coordinate
         * @param z Z coordinate
         */
        void SetPosition(float x, float y, float z);

        /**
         * @brief Get current position
         * @return Current position
         */
        const glm::vec3& GetPosition() const { return m_position; }

        /**
         * @brief Translate by vector
         * @param translation Translation vector
         */
        void Translate(const glm::vec3& translation);

        /**
         * @brief Translate by individual coordinates
         * @param x X translation
         * @param y Y translation
         * @param z Z translation
         */
        void Translate(float x, float y, float z);

        // Rotation operations

        /**
         * @brief Set rotation using quaternion
         * @param rotation New rotation quaternion
         */
        void SetRotation(const glm::quat& rotation);

        /**
         * @brief Set rotation using Euler angles (degrees)
         * @param pitch Pitch angle
         * @param yaw Yaw angle
         * @param roll Roll angle
         */
        void SetRotation(float pitch, float yaw, float roll);

        /**
         * @brief Rotate by quaternion
         * @param rotation Rotation quaternion to apply
         */
        void Rotate(const glm::quat& rotation);

        /**
         * @brief Rotate around axis by angle
         * @param axis Rotation axis
         * @param angle Rotation angle in degrees
         */
        void Rotate(const glm::vec3& axis, float angle);

        /**
         * @brief Get current rotation
         * @return Current rotation quaternion
         */
        const glm::quat& GetRotation() const { return m_rotation; }

        /**
         * @brief Get forward direction vector
         * @return Forward direction
         */
        glm::vec3 GetForward() const;

        /**
         * @brief Get right direction vector
         * @return Right direction
         */
        glm::vec3 GetRight() const;

        /**
         * @brief Get up direction vector
         * @return Up direction
         */
        glm::vec3 GetUp() const;

        /**
         * @brief Look at target position
         * @param target Target position to look at
         * @param up Up vector for orientation
         */
        void LookAt(const glm::vec3& target, const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.0f));

        // Scale operations

        /**
         * @brief Set scale
         * @param scale New scale vector
         */
        void SetScale(const glm::vec3& scale);

        /**
         * @brief Set uniform scale
         * @param scale Uniform scale value
         */
        void SetScale(float scale);

        /**
         * @brief Set scale with individual coordinates
         * @param x X scale
         * @param y Y scale
         * @param z Z scale
         */
        void SetScale(float x, float y, float z);

        /**
         * @brief Get current scale
         * @return Current scale vector
         */
        const glm::vec3& GetScale() const { return m_scale; }

        /**
         * @brief Scale by vector
         * @param scale Scale vector to apply
         */
        void Scale(const glm::vec3& scale);

        /**
         * @brief Scale uniformly
         * @param scale Scale factor to apply
         */
        void Scale(float scale);

        // Matrix operations

        /**
         * @brief Get world transformation matrix
         * @return 4x4 transformation matrix
         */
        glm::mat4 GetWorldMatrix() const;

        /**
         * @brief Get local transformation matrix
         * @return 4x4 transformation matrix
         */
        glm::mat4 GetLocalMatrix() const;

        /**
         * @brief Mark transform as dirty (forces matrix recalculation)
         */
        void MarkDirty();

        /**
         * @brief Check if transform is dirty
         * @return true if dirty, false otherwise
         */
        bool IsDirty() const { return m_isDirty; }

        // Hierarchy operations (if needed)

        /**
         * @brief Set parent transform
         * @param parent Parent transform component
         */
        void SetParent(TransformComponent* parent);

        /**
         * @brief Get parent transform
         * @return Parent transform component or nullptr
         */
        TransformComponent* GetParent() const { return m_parent; }

        /**
         * @brief Add child transform
         * @param child Child transform component
         */
        void AddChild(TransformComponent* child);

        /**
         * @brief Remove child transform
         * @param child Child transform component to remove
         */
        void RemoveChild(TransformComponent* child);

        /**
         * @brief Get children transforms
         * @return Vector of child transform components
         */
        const std::vector<TransformComponent*>& GetChildren() const { return m_children; }

        /**
         * @brief Get world position (including parent transforms)
         * @return World position
         */
        glm::vec3 GetWorldPosition() const;

        /**
         * @brief Get world rotation (including parent transforms)
         * @return World rotation quaternion
         */
        glm::quat GetWorldRotation() const;

        /**
         * @brief Get world scale (including parent transforms)
         * @return World scale
         */
        glm::vec3 GetWorldScale() const;

        /**
         * @brief Get world transformation matrix (including parent transforms)
         * @return World 4x4 transformation matrix
         */
        glm::mat4 GetWorldMatrixHierarchy() const;

    private:
        /**
         * @brief Update transformation matrices if dirty
         */
        void UpdateMatrices() const;

        /**
         * @brief Recalculate world matrix recursively
         * @param parentWorldMatrix Parent world matrix
         * @return Updated world matrix
         */
        glm::mat4 CalculateWorldMatrix(const glm::mat4& parentWorldMatrix) const;

        glm::vec3 m_position;                    ///< Local position
        glm::quat m_rotation;                    ///< Local rotation (quaternion)
        glm::vec3 m_scale;                       ///< Local scale

        mutable glm::mat4 m_localMatrix;         ///< Cached local transformation matrix
        mutable glm::mat4 m_worldMatrix;         ///< Cached world transformation matrix
        mutable bool m_isDirty;                  ///< Dirty flag for matrix recalculation

        // Hierarchy support
        TransformComponent* m_parent;            ///< Parent transform
        std::vector<TransformComponent*> m_children; ///< Child transforms
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_ENTITIES_TRANSFORM_COMPONENT_HPP
