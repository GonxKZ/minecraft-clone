/**
 * @file RenderSystem.cpp
 * @brief VoxelCraft ECS - Render System Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#include "RenderSystem.hpp"
#include "Entity.hpp"
#include "RenderComponent.hpp"
#include "TransformComponent.hpp"
#include "Logger.hpp"

#include <algorithm>
#include <chrono>

namespace VoxelCraft {

    RenderSystem::RenderSystem()
        : System("RenderSystem", SystemPriority::Normal)
        , m_activeCamera(nullptr)
        , m_frustumCullingEnabled(true)
        , m_occlusionCullingEnabled(false)
        , m_debugRenderingEnabled(false)
        , m_renderDistance(1000.0f)
        , m_lastFrameTime(0.0)
    {
        VOXELCRAFT_TRACE("RenderSystem created");
    }

    RenderSystem::RenderSystem(const std::string& name)
        : System(name, SystemPriority::Normal)
        , m_activeCamera(nullptr)
        , m_frustumCullingEnabled(true)
        , m_occlusionCullingEnabled(false)
        , m_debugRenderingEnabled(false)
        , m_renderDistance(1000.0f)
        , m_lastFrameTime(0.0)
    {
        VOXELCRAFT_TRACE("RenderSystem created with name '{}'", name);
    }

    RenderSystem::~RenderSystem() {
        VOXELCRAFT_TRACE("RenderSystem destroyed");
        ClearRenderQueue();
    }

    bool RenderSystem::Initialize() {
        VOXELCRAFT_TRACE("RenderSystem initialized");

        // Find default camera
        if (GetEntityManager()) {
            auto entities = GetEntityManager()->GetAllEntities();
            for (auto entity : entities) {
                // Look for entity with "Camera" in name or with camera component
                if (entity->GetName().find("Camera") != std::string::npos) {
                    m_activeCamera = entity;
                    VOXELCRAFT_INFO("Found default camera: {}", entity->GetName());
                    break;
                }
            }
        }

        return true;
    }

    void RenderSystem::Update(double deltaTime) {
        // Clear previous frame data
        ClearRenderQueue();
        m_statistics.frameCount++;

        // Update render components
        if (GetEntityManager()) {
            auto entities = GetEntityManager()->GetActiveEntities();
            for (auto entity : entities) {
                ProcessEntity(entity, deltaTime);
            }
        }

        // Sort render queue for optimal rendering
        SortRenderQueue();
    }

    void RenderSystem::Render() {
        auto startTime = std::chrono::high_resolution_clock::now();

        // Process different render passes
        ProcessRenderQueue(RenderPass::Opaque);
        ProcessRenderQueue(RenderPass::Transparent);
        ProcessRenderQueue(RenderPass::UI);

        // Render debug information if enabled
        if (m_debugRenderingEnabled) {
            RenderDebug();
        }

        auto endTime = std::chrono::high_resolution_clock::now();
        m_statistics.renderTime = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(
            endTime - startTime
        ).count();
    }

    void RenderSystem::Shutdown() {
        VOXELCRAFT_TRACE("RenderSystem shutdown");
        ClearRenderQueue();
        m_activeCamera = nullptr;
    }

    void RenderSystem::ProcessEntity(Entity* entity, double deltaTime) {
        if (!entity || !entity->IsActive()) {
            return;
        }

        auto renderComponent = entity->GetComponent<RenderComponent>();
        if (!renderComponent) {
            return;
        }

        // Calculate distance from camera
        float distance = CalculateCameraDistance(entity);

        // Perform culling
        bool shouldCull = false;
        auto cullStart = std::chrono::high_resolution_clock::now();

        if (m_frustumCullingEnabled) {
            shouldCull = !PerformFrustumCulling(entity);
        }

        if (!shouldCull && distance > 0.0f) {
            shouldCull = !PerformDistanceCulling(entity, distance);
        }

        auto cullEnd = std::chrono::high_resolution_clock::now();
        m_statistics.cullTime += std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(
            cullEnd - cullStart
        ).count();

        if (shouldCull) {
            m_statistics.culledEntities++;
            return;
        }

        // Add to render queue
        AddToRenderQueue(entity, distance);
        m_statistics.visibleEntities++;
    }

    bool RenderSystem::ShouldProcessEntity(Entity* entity) const {
        if (!entity) {
            return false;
        }

        // Entity must have a render component and be active
        auto renderComponent = entity->GetComponent<RenderComponent>();
        return renderComponent && entity->IsActive() && renderComponent->IsVisible();
    }

    void RenderSystem::SetActiveCamera(Entity* camera) {
        if (m_activeCamera != camera) {
            m_activeCamera = camera;
            VOXELCRAFT_INFO("Active camera changed to: {}",
                          camera ? camera->GetName() : "null");
        }
    }

    void RenderSystem::AddToRenderQueue(Entity* entity, float distance) {
        if (!entity) {
            return;
        }

        auto renderComponent = entity->GetComponent<RenderComponent>();
        if (!renderComponent) {
            return;
        }

        RenderQueueEntry entry;
        entry.entity = entity;
        entry.distance = distance;
        entry.renderComponent = renderComponent;

        m_renderQueue.push_back(entry);
    }

    void RenderSystem::ClearRenderQueue() {
        m_renderQueue.clear();
    }

    void RenderSystem::SortRenderQueue() {
        // Sort by distance (back to front for transparency, front to back for opaque)
        std::sort(m_renderQueue.begin(), m_renderQueue.end());
    }

    void RenderSystem::ProcessRenderQueue(RenderPass pass) {
        for (const auto& entry : m_renderQueue) {
            if (!entry.entity || !entry.renderComponent) {
                continue;
            }

            // Check if entity should be rendered in this pass
            // For now, we'll render all entities in the opaque pass
            // TODO: Implement proper render pass filtering

            entry.renderComponent->Render();
            m_statistics.totalDrawCalls++;
        }
    }

    bool RenderSystem::PerformFrustumCulling(Entity* entity) {
        if (!m_activeCamera) {
            return true; // No camera, everything is visible
        }

        // TODO: Implement proper frustum culling
        // For now, just return true (no culling)
        return true;
    }

    bool RenderSystem::PerformDistanceCulling(Entity* entity, float distance) {
        if (distance > m_renderDistance) {
            return false; // Cull if too far
        }

        // Check entity-specific culling distance
        auto renderComponent = entity->GetComponent<RenderComponent>();
        if (renderComponent && distance > renderComponent->GetCullingDistance()) {
            return false; // Cull if beyond entity-specific distance
        }

        return true; // Don't cull
    }

    float RenderSystem::CalculateCameraDistance(Entity* entity) {
        if (!m_activeCamera || !entity) {
            return 0.0f;
        }

        auto entityTransform = entity->GetComponent<TransformComponent>();
        auto cameraTransform = m_activeCamera->GetComponent<TransformComponent>();

        if (!entityTransform || !cameraTransform) {
            return 0.0f;
        }

        glm::vec3 entityPos = entityTransform->GetWorldPosition();
        glm::vec3 cameraPos = cameraTransform->GetWorldPosition();

        return glm::distance(cameraPos, entityPos);
    }

    void RenderSystem::RenderDebug() {
        // Render debug information
        VOXELCRAFT_TRACE("RenderSystem debug: Queue={}, Visible={}, Culled={}, DrawCalls={}",
                        m_renderQueue.size(),
                        m_statistics.visibleEntities,
                        m_statistics.culledEntities,
                        m_statistics.totalDrawCalls);
    }

    void RenderSystem::ResetStatistics() {
        m_statistics = RenderStatistics();
    }

} // namespace VoxelCraft
