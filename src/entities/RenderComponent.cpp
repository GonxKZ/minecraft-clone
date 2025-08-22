/**
 * @file RenderComponent.cpp
 * @brief VoxelCraft ECS - Render Component Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#include "RenderComponent.hpp"
#include "Entity.hpp"
#include "TransformComponent.hpp"
#include "Logger.hpp"

#include <algorithm>

namespace VoxelCraft {

    // Forward declarations for graphics classes
    class Mesh {
    public:
        std::string name;
        size_t vertexCount;
        size_t indexCount;

        Mesh(const std::string& meshName = "default")
            : name(meshName), vertexCount(0), indexCount(0) {}
    };

    class Material {
    public:
        std::string name;
        std::string shaderName;
        std::unordered_map<std::string, glm::vec4> colors;
        std::unordered_map<std::string, float> floats;
        std::unordered_map<std::string, std::string> textures;

        Material(const std::string& materialName = "default")
            : name(materialName), shaderName("default") {}
    };

    class Shader {
    public:
        std::string name;
        std::string vertexSource;
        std::string fragmentSource;

        Shader(const std::string& shaderName = "default")
            : name(shaderName) {}
    };

    RenderComponent::RenderComponent(Entity* owner)
        : Component(owner, "RenderComponent")
        , m_isVisible(true)
        , m_cullingDistance(1000.0f)
        , m_renderLayer(RenderLayer::Objects)
        , m_castShadows(true)
        , m_receiveShadows(true)
        , m_boundingBoxMin(0.0f)
        , m_boundingBoxMax(0.0f)
        , m_renderCount(0)
        , m_culledCount(0)
        , m_lastRenderTime(0.0)
        , m_totalRenderTime(0.0)
    {
        VOXELCRAFT_TRACE("RenderComponent created for entity '{}'",
                        owner ? owner->GetName() : "null");
    }

    RenderComponent::RenderComponent(Entity* owner,
                                   std::shared_ptr<Mesh> mesh,
                                   std::shared_ptr<Material> material)
        : Component(owner, "RenderComponent")
        , m_isVisible(true)
        , m_cullingDistance(1000.0f)
        , m_renderLayer(RenderLayer::Objects)
        , m_castShadows(true)
        , m_receiveShadows(true)
        , m_boundingBoxMin(0.0f)
        , m_boundingBoxMax(0.0f)
        , m_renderCount(0)
        , m_culledCount(0)
        , m_lastRenderTime(0.0)
        , m_totalRenderTime(0.0)
    {
        VOXELCRAFT_TRACE("RenderComponent created for entity '{}' with mesh and material",
                        owner ? owner->GetName() : "null");

        // Add the initial instance
        RenderInstance instance;
        instance.mesh = mesh;
        instance.material = material;
        AddInstance(instance);
    }

    RenderComponent::~RenderComponent() {
        VOXELCRAFT_TRACE("RenderComponent destroyed for entity '{}'",
                        GetOwner() ? GetOwner()->GetName() : "null");
    }

    bool RenderComponent::Initialize() {
        VOXELCRAFT_TRACE("RenderComponent initialized for entity '{}'",
                        GetOwner() ? GetOwner()->GetName() : "null");

        // Auto-calculate bounding box if we have instances
        if (!m_instances.empty() && m_boundingBoxMin == m_boundingBoxMax) {
            // For now, set a default bounding box
            SetBoundingBox(glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(1.0f, 1.0f, 1.0f));
        }

        return true;
    }

    void RenderComponent::Update(double deltaTime) {
        // Update render instances
        for (auto& instance : m_instances) {
            // Update any animated properties
            // This could include UV animation, morphing, etc.
        }
    }

    void RenderComponent::Render() {
        if (!m_isVisible || m_instances.empty()) {
            return;
        }

        auto startTime = std::chrono::high_resolution_clock::now();

        // Get transform component for world matrix
        auto transform = GetOwner()->GetComponent<TransformComponent>();
        if (!transform) {
            VOXELCRAFT_WARNING("RenderComponent on entity '{}' has no TransformComponent",
                             GetOwner()->GetName());
            return;
        }

        // Render each instance
        for (const auto& instance : m_instances) {
            if (!instance.visible || !instance.mesh || !instance.material) {
                continue;
            }

            // In a real implementation, this would:
            // 1. Bind the material/shader
            // 2. Set uniforms (world matrix, etc.)
            // 3. Bind the mesh
            // 4. Draw the mesh

            VOXELCRAFT_TRACE("Rendering instance of entity '{}' with mesh '{}' and material '{}'",
                           GetOwner()->GetName(),
                           instance.mesh->name,
                           instance.material->name);
        }

        auto endTime = std::chrono::high_resolution_clock::now();
        m_lastRenderTime = std::chrono::duration_cast<std::chrono::duration<double>>(
            endTime - startTime
        ).count();
        m_totalRenderTime += m_lastRenderTime;
        m_renderCount++;
    }

    size_t RenderComponent::AddInstance(const RenderInstance& instance) {
        m_instances.push_back(instance);
        VOXELCRAFT_TRACE("Added render instance to entity '{}', total instances: {}",
                        GetOwner() ? GetOwner()->GetName() : "null", m_instances.size());
        return m_instances.size() - 1;
    }

    bool RenderComponent::RemoveInstance(size_t index) {
        if (index >= m_instances.size()) {
            return false;
        }

        m_instances.erase(m_instances.begin() + index);
        VOXELCRAFT_TRACE("Removed render instance from entity '{}', remaining instances: {}",
                        GetOwner() ? GetOwner()->GetName() : "null", m_instances.size());
        return true;
    }

    RenderInstance* RenderComponent::GetInstance(size_t index) {
        if (index >= m_instances.size()) {
            return nullptr;
        }
        return &m_instances[index];
    }

    const RenderInstance* RenderComponent::GetInstance(size_t index) const {
        if (index >= m_instances.size()) {
            return nullptr;
        }
        return &m_instances[index];
    }

    void RenderComponent::ClearInstances() {
        m_instances.clear();
        VOXELCRAFT_TRACE("Cleared all render instances from entity '{}'",
                        GetOwner() ? GetOwner()->GetName() : "null");
    }

    void RenderComponent::SetVisible(bool visible) {
        if (m_isVisible != visible) {
            m_isVisible = visible;
            VOXELCRAFT_TRACE("Entity '{}' visibility changed to {}",
                           GetOwner() ? GetOwner()->GetName() : "null", visible ? "visible" : "hidden");
        }
    }

    bool RenderComponent::ShouldBeCulled(const glm::vec3& cameraPosition,
                                       const glm::vec3& cameraDirection,
                                       float cameraFOV) const {
        if (!m_isVisible) {
            m_culledCount++;
            return true;
        }

        // Get transform component for position
        auto transform = GetOwner()->GetComponent<TransformComponent>();
        if (!transform) {
            m_culledCount++;
            return true;
        }

        glm::vec3 worldPosition = transform->GetWorldPosition();

        // Distance culling
        float distance = glm::distance(cameraPosition, worldPosition);
        if (distance > m_cullingDistance) {
            m_culledCount++;
            return true;
        }

        // Frustum culling (simplified)
        // In a real implementation, this would check against the camera frustum
        // For now, we'll just do a simple distance check

        return false;
    }

    void RenderComponent::SetBoundingBox(const glm::vec3& min, const glm::vec3& max) {
        m_boundingBoxMin = min;
        m_boundingBoxMax = max;
        VOXELCRAFT_TRACE("Bounding box set for entity '{}' from ({}, {}, {}) to ({}, {}, {})",
                        GetOwner() ? GetOwner()->GetName() : "null",
                        min.x, min.y, min.z, max.x, max.y, max.z);
    }

    glm::vec3 RenderComponent::GetBoundingBoxCenter() const {
        return (m_boundingBoxMin + m_boundingBoxMax) * 0.5f;
    }

    glm::vec3 RenderComponent::GetBoundingBoxSize() const {
        return m_boundingBoxMax - m_boundingBoxMin;
    }

    std::string RenderComponent::GetRenderStats() const {
        std::stringstream ss;
        ss << "Render Stats for entity '" << (GetOwner() ? GetOwner()->GetName() : "null") << "':\n";
        ss << "  Instances: " << m_instances.size() << "\n";
        ss << "  Render Count: " << m_renderCount << "\n";
        ss << "  Culled Count: " << m_culledCount << "\n";
        ss << "  Last Render Time: " << (m_lastRenderTime * 1000.0) << "ms\n";
        ss << "  Total Render Time: " << (m_totalRenderTime * 1000.0) << "ms\n";
        ss << "  Average Render Time: " << (m_renderCount > 0 ? (m_totalRenderTime * 1000.0) / m_renderCount : 0.0) << "ms\n";
        return ss.str();
    }

    void RenderComponent::ResetRenderStats() {
        m_renderCount = 0;
        m_culledCount = 0;
        m_lastRenderTime = 0.0;
        m_totalRenderTime = 0.0;
    }

} // namespace VoxelCraft
