#pragma once

#include "Component.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace VoxelCraft {

    /**
     * @brief Componente de transformación básico
     */
    class TransformComponent : public Component {
    public:
        glm::vec3 position{0.0f};
        glm::quat rotation{1.0f, 0.0f, 0.0f, 0.0f};
        glm::vec3 scale{1.0f};

        TransformComponent(const glm::vec3& pos = glm::vec3(0.0f),
                          const glm::quat& rot = glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
                          const glm::vec3& scl = glm::vec3(1.0f));

        // Transformaciones
        void Translate(const glm::vec3& delta);
        void Rotate(const glm::vec3& eulerAngles);
        void Scale(const glm::vec3& delta);

        // Utilidades
        glm::mat4 GetModelMatrix() const;

        std::string ToString() const override;
    };

}