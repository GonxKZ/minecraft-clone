/**
 * @file RenderComponent.cpp
 * @brief VoxelCraft ECS - Render Component Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#include "RenderComponent.hpp"
#include "Entity.hpp"
#include "Logger.hpp"

namespace VoxelCraft {

    RenderComponent::RenderComponent()
        : m_enabled(true)
        , m_renderType(RenderType::STATIC_MESH)
        , m_renderLayer(RenderLayer::STATIC_OBJECTS)
        , m_visible(true)
        , m_boundingBoxMin({0.0f, 0.0f, 0.0f})
        , m_boundingBoxMax({0.0f, 0.0f, 0.0f})
        , m_hasValidBoundingBox(false)
        , m_renderDistance(100.0f)
        , m_castsShadows(true)
        , m_customRenderData(nullptr)
    {
    }

    RenderComponent::RenderComponent(RenderType type)
        : m_enabled(true)
        , m_renderType(type)
        , m_renderLayer(RenderLayer::STATIC_OBJECTS)
        , m_visible(true)
        , m_boundingBoxMin({0.0f, 0.0f, 0.0f})
        , m_boundingBoxMax({0.0f, 0.0f, 0.0f})
        , m_hasValidBoundingBox(false)
        , m_renderDistance(100.0f)
        , m_castsShadows(true)
        , m_customRenderData(nullptr)
    {
        // Set appropriate render layer based on type
        switch (type) {
            case RenderType::TERRAIN_CHUNK:
                m_renderLayer = RenderLayer::TERRAIN;
                break;
            case RenderType::WATER:
                m_renderLayer = RenderLayer::TRANSPARENT;
                break;
            case RenderType::SKYBOX:
                m_renderLayer = RenderLayer::BACKGROUND;
                break;
            case RenderType::UI_ELEMENT:
                m_renderLayer = RenderLayer::UI;
                break;
            case RenderType::BILLBOARD:
                m_renderLayer = RenderLayer::PARTICLES;
                break;
            case RenderType::ANIMATED_MESH:
                m_renderLayer = RenderLayer::ANIMATED_OBJECTS;
                break;
            default:
                m_renderLayer = RenderLayer::STATIC_OBJECTS;
                break;
        }
    }

    RenderComponent::~RenderComponent() {
        // Clean up custom render data if needed
        if (m_customRenderData) {
            // Note: Custom render data cleanup should be handled by the owner
            VOXELCRAFT_DEBUG("RenderComponent destroyed with custom render data");
        }
    }

    void RenderComponent::Update(float deltaTime) {
        // Update animations or other time-based render properties
        // This is called by the render system

        if (!m_enabled || !m_visible) {
            return;
        }

        // Update any animated properties based on deltaTime
        // This would be where animation updates would go
    }

    void RenderComponent::SetBoundingBox(const std::array<float, 3>& min,
                                       const std::array<float, 3>& max) {
        m_boundingBoxMin = min;
        m_boundingBoxMax = max;
        m_hasValidBoundingBox = true;

        // Validate bounding box
        for (int i = 0; i < 3; ++i) {
            if (min[i] > max[i]) {
                VOXELCRAFT_WARNING("Invalid bounding box: min[{}] > max[{}]", i, i);
                m_hasValidBoundingBox = false;
                break;
            }
        }
    }

} // namespace VoxelCraft
