/**
 * @file RenderSystem.hpp
 * @brief VoxelCraft ECS - Render System
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * System for managing rendering of entities with RenderComponent.
 */

#ifndef VOXELCRAFT_ENTITIES_RENDER_SYSTEM_HPP
#define VOXELCRAFT_ENTITIES_RENDER_SYSTEM_HPP

#include "System.hpp"
#include <memory>
#include <vector>

namespace VoxelCraft {

    class RenderComponent;
    class Camera;

    /**
     * @enum RenderPass
     * @brief Different rendering passes
     */
    enum class RenderPass {
        Opaque,         ///< Opaque geometry pass
        Transparent,    ///< Transparent geometry pass
        Shadow,         ///< Shadow map generation
        PostProcess,    ///< Post-processing effects
        UI              ///< UI rendering
    };

    /**
     * @struct RenderStatistics
     * @brief Rendering performance statistics
     */
    struct RenderStatistics {
        uint64_t totalDrawCalls;               ///< Total number of draw calls
        uint64_t totalTriangles;               ///< Total number of triangles rendered
        uint64_t culledEntities;               ///< Number of entities culled
        uint64_t visibleEntities;              ///< Number of visible entities
        double renderTime;                     ///< Time spent rendering (ms)
        double cullTime;                       ///< Time spent culling (ms)
        uint32_t frameCount;                   ///< Frame counter

        RenderStatistics()
            : totalDrawCalls(0)
            , totalTriangles(0)
            , culledEntities(0)
            , visibleEntities(0)
            , renderTime(0.0)
            , cullTime(0.0)
            , frameCount(0)
        {}
    };

    /**
     * @class RenderSystem
     * @brief System for managing entity rendering
     *
     * The RenderSystem handles:
     * - Entity culling based on frustum and distance
     * - Render queue management
     * - Batch rendering optimization
     * - Render statistics collection
     */
    class RenderSystem : public System {
    public:
        /**
         * @brief Constructor
         */
        RenderSystem();

        /**
         * @brief Constructor with name
         * @param name System name
         */
        explicit RenderSystem(const std::string& name);

        /**
         * @brief Destructor
         */
        ~RenderSystem() override;

        /**
         * @brief Get system type name
         * @return System type name
         */
        std::string GetTypeName() const override { return "RenderSystem"; }

        /**
         * @brief Initialize the system
         * @return true if successful, false otherwise
         */
        bool Initialize() override;

        /**
         * @brief Update the system
         * @param deltaTime Time elapsed since last update
         */
        void Update(double deltaTime) override;

        /**
         * @brief Render the system
         */
        void Render() override;

        /**
         * @brief Shutdown the system
         */
        void Shutdown() override;

        /**
         * @brief Process a single entity
         * @param entity Entity to process
         * @param deltaTime Time elapsed since last update
         */
        void ProcessEntity(Entity* entity, double deltaTime) override;

        /**
         * @brief Check if system should process an entity
         * @param entity Entity to check
         * @return true if should process, false otherwise
         */
        bool ShouldProcessEntity(Entity* entity) const override;

        // Camera management

        /**
         * @brief Set active camera
         * @param camera Camera entity
         */
        void SetActiveCamera(Entity* camera);

        /**
         * @brief Get active camera
         * @return Active camera entity
         */
        Entity* GetActiveCamera() const { return m_activeCamera; }

        // Render queue management

        /**
         * @brief Add entity to render queue
         * @param entity Entity to add
         * @param distance Distance from camera
         */
        void AddToRenderQueue(Entity* entity, float distance);

        /**
         * @brief Clear render queue
         */
        void ClearRenderQueue();

        /**
         * @brief Sort render queue by distance
         */
        void SortRenderQueue();

        /**
         * @brief Process render queue for a specific pass
         * @param pass Render pass to process
         */
        void ProcessRenderQueue(RenderPass pass);

        // Culling and optimization

        /**
         * @brief Enable/disable frustum culling
         * @param enabled Culling state
         */
        void SetFrustumCullingEnabled(bool enabled) { m_frustumCullingEnabled = enabled; }

        /**
         * @brief Check if frustum culling is enabled
         * @return true if enabled, false otherwise
         */
        bool IsFrustumCullingEnabled() const { return m_frustumCullingEnabled; }

        /**
         * @brief Enable/disable occlusion culling
         * @param enabled Occlusion culling state
         */
        void SetOcclusionCullingEnabled(bool enabled) { m_occlusionCullingEnabled = enabled; }

        /**
         * @brief Check if occlusion culling is enabled
         * @return true if enabled, false otherwise
         */
        bool IsOcclusionCullingEnabled() const { return m_occlusionCullingEnabled; }

        /**
         * @brief Set global render distance
         * @param distance Render distance
         */
        void SetRenderDistance(float distance) { m_renderDistance = distance; }

        /**
         * @brief Get global render distance
         * @return Render distance
         */
        float GetRenderDistance() const { return m_renderDistance; }

        // Statistics and debugging

        /**
         * @brief Get render statistics
         * @return Current render statistics
         */
        const RenderStatistics& GetStatistics() const { return m_statistics; }

        /**
         * @brief Reset render statistics
         */
        void ResetStatistics();

        /**
         * @brief Enable/disable debug rendering
         * @param enabled Debug render state
         */
        void SetDebugRenderingEnabled(bool enabled) { m_debugRenderingEnabled = enabled; }

        /**
         * @brief Check if debug rendering is enabled
         * @return true if enabled, false otherwise
         */
        bool IsDebugRenderingEnabled() const { return m_debugRenderingEnabled; }

        /**
         * @brief Get render queue size
         * @return Number of entities in render queue
         */
        size_t GetRenderQueueSize() const { return m_renderQueue.size(); }

    private:
        /**
         * @struct RenderQueueEntry
         * @brief Entry in the render queue
         */
        struct RenderQueueEntry {
            Entity* entity;                    ///< Entity to render
            float distance;                    ///< Distance from camera
            RenderComponent* renderComponent;  ///< Render component

            bool operator<(const RenderQueueEntry& other) const {
                return distance < other.distance;
            }
        };

        /**
         * @brief Perform frustum culling
         * @param entity Entity to test
         * @return true if visible, false if culled
         */
        bool PerformFrustumCulling(Entity* entity);

        /**
         * @brief Perform distance culling
         * @param entity Entity to test
         * @param distance Distance from camera
         * @return true if visible, false if culled
         */
        bool PerformDistanceCulling(Entity* entity, float distance);

        /**
         * @brief Calculate distance from camera to entity
         * @param entity Entity to calculate distance for
         * @return Distance from camera
         */
        float CalculateCameraDistance(Entity* entity);

        /**
         * @brief Render debug information
         */
        void RenderDebug();

        Entity* m_activeCamera;                            ///< Active camera entity
        std::vector<RenderQueueEntry> m_renderQueue;       ///< Render queue

        bool m_frustumCullingEnabled;                      ///< Frustum culling state
        bool m_occlusionCullingEnabled;                    ///< Occlusion culling state
        bool m_debugRenderingEnabled;                      ///< Debug rendering state
        float m_renderDistance;                            ///< Global render distance

        RenderStatistics m_statistics;                     ///< Render statistics
        double m_lastFrameTime;                            ///< Last frame time for FPS calculation
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_ENTITIES_RENDER_SYSTEM_HPP
