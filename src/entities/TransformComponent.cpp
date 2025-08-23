#include "TransformComponent.hpp"
#include <glm/gtc/matrix_transform.hpp>

namespace VoxelCraft {

    TransformComponent::TransformComponent(const glm::vec3& pos,
                                         const glm::quat& rot,
                                         const glm::vec3& scl)
        : position(pos), rotation(rot), scale(scl)
    {
        m_Name = "TransformComponent";
    }

    void TransformComponent::Translate(const glm::vec3& delta) {
        position += delta;
    }

    void TransformComponent::Rotate(const glm::vec3& eulerAngles) {
        glm::quat deltaRotation = glm::quat(eulerAngles);
        rotation = deltaRotation * rotation;
    }

    void TransformComponent::Scale(const glm::vec3& delta) {
        scale *= delta;
    }

    glm::mat4 TransformComponent::GetModelMatrix() const {
        glm::mat4 modelMatrix = glm::mat4(1.0f);

        // Aplicar transformación: escala -> rotación -> traslación
        modelMatrix = glm::translate(modelMatrix, position);
        modelMatrix = modelMatrix * glm::mat4_cast(rotation);
        modelMatrix = glm::scale(modelMatrix, scale);

        return modelMatrix;
    }

    std::string TransformComponent::ToString() const {
        std::stringstream ss;
        ss << "TransformComponent[Position=(" << position.x << "," << position.y << "," << position.z
           << "), Scale=(" << scale.x << "," << scale.y << "," << scale.z << ")]";
        return ss.str();
    }

}