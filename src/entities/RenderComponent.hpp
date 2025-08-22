/**
 * @file RenderComponent.hpp
 * @brief VoxelCraft ECS - Render Component
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * Render component for entities that need to be visually represented.
 */

#ifndef VOXELCRAFT_ENTITIES_RENDER_COMPONENT_HPP
#define VOXELCRAFT_ENTITIES_RENDER_COMPONENT_HPP

#include "Component.hpp"
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

namespace VoxelCraft {

    class Mesh;
    class Material;
    class Shader;

    /**
     * @enum RenderMode
     * @brief Rendering mode for the component
     */
    enum class RenderMode {
        Opaque,         ///< Opaque rendering (default)
        Transparent,    ///< Transparent rendering
        Wireframe,      ///< Wireframe rendering
        Billboard       ///< Billboard rendering (always faces camera)
    };

    /**
     * @enum RenderLayer
     * @brief Rendering layer for sorting
     */
    enum class RenderLayer {
        Background = 0,     ///< Background layer
        Terrain = 1,        ///< Terrain layer
        Water = 2,          ///< Water layer
        Objects = 3,        ///< General objects layer
        Particles = 4,      ///< Particle effects layer
        UI = 5,             ///< UI layer
        Overlay = 6         ///< Overlay layer
    };

    /**
     * @struct RenderInstance
     * @brief Data for a single render instance
     */
    struct RenderInstance {
        std::shared_ptr<Mesh> mesh;              ///< Mesh to render
        std::shared_ptr<Material> material;      ///< Material to use
        RenderMode mode;                         ///< Rendering mode
        RenderLayer layer;                       ///< Rendering layer
        bool visible;                            ///< Visibility flag
        bool castShadows;                        ///< Cast shadows flag
        bool receiveShadows;                     ///< Receive shadows flag
        float distanceCulling;                   ///< Distance culling distance
        std::unordered_map<std::string, float> customProperties; ///< Custom shader properties

        RenderInstance()
            : mode(RenderMode::Opaque)
            , layer(RenderLayer::Objects)
            , visible(true)
            , castShadows(true)
            , receiveShadows(true)
            , distanceCulling(1000.0f)
        {}
    };

    /**
     * @class RenderComponent
     * @brief Component for entities that need visual representation
     *
     * The RenderComponent manages all visual aspects of an entity including
     * meshes, materials, rendering modes, and culling.
     */
    class RenderComponent : public Component {
    public:
        /**
         * @brief Constructor
         * @param owner Entity that owns this component
         */
        RenderComponent(Entity* owner);

        /**
         * @brief Constructor with mesh and material
         * @param owner Entity that owns this component
         * @param mesh Mesh to render
         * @param material Material to use
         */
        RenderComponent(Entity* owner,
                       std::shared_ptr<Mesh> mesh,
                       std::shared_ptr<Material> material);

        /**
         * @brief Destructor
         */
        ~RenderComponent() override;

        /**
         * @brief Get component type name
         * @return Component type name
         */
        std::string GetTypeName() const override { return "RenderComponent"; }

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

        /**
         * @brief Render the component
         */
        void Render() override;

        // Mesh operations

        /**
         * @brief Add render instance
         * @param instance Render instance to add
         * @return Index of the added instance
         */
        size_t AddInstance(const RenderInstance& instance);

        /**
         * @brief Remove render instance
         * @param index Index of instance to remove
         * @return true if removed, false if index invalid
         */
        bool RemoveInstance(size_t index);

        /**
         * @brief Get render instance
         * @param index Instance index
         * @return Render instance or nullptr if invalid
         */
        RenderInstance* GetInstance(size_t index);

        /**
         * @brief Get render instance (const version)
         * @param index Instance index
         * @return Render instance or nullptr if invalid
         */
        const RenderInstance* GetInstance(size_t index) const;

        /**
         * @brief Get number of render instances
         * @return Number of instances
         */
        size_t GetInstanceCount() const { return m_instances.size(); }

        /**
         * @brief Clear all render instances
         */
        void ClearInstances();

        // Visibility and culling

        /**
         * @brief Set overall visibility
         * @param visible Visibility state
         */
        void SetVisible(bool visible);

        /**
         * @brief Check if component is visible
         * @return true if visible, false otherwise
         */
        bool IsVisible() const { return m_isVisible; }

        /**
         * @brief Set distance culling
         * @param distance Culling distance
         */
        void SetCullingDistance(float distance) { m_cullingDistance = distance; }

        /**
         * @brief Get distance culling
         * @return Culling distance
         */
        float GetCullingDistance() const { return m_cullingDistance; }

        /**
         * @brief Check if should be culled based on distance
         * @param cameraPosition Camera position
         * @param cameraDirection Camera direction
         * @param cameraFOV Camera field of view
         * @return true if should be culled, false otherwise
         */
        bool ShouldBeCulled(const glm::vec3& cameraPosition,
                           const glm::vec3& cameraDirection,
                           float cameraFOV) const;

        // Bounds and collision

        /**
         * @brief Set bounding box
         * @param min Minimum corner
         * @param max Maximum corner
         */
        void SetBoundingBox(const glm::vec3& min, const glm::vec3& max);

        /**
         * @brief Get bounding box minimum
         * @return Minimum corner
         */
        const glm::vec3& GetBoundingBoxMin() const { return m_boundingBoxMin; }

        /**
         * @brief Get bounding box maximum
         * @return Maximum corner
         */
        const glm::vec3& GetBoundingBoxMax() const { return m_boundingBoxMax; }

        /**
         * @brief Get bounding box center
         * @return Center point
         */
        glm::vec3 GetBoundingBoxCenter() const;

        /**
         * @brief Get bounding box size
         * @return Size vector
         */
        glm::vec3 GetBoundingBoxSize() const;

        // Render state

        /**
         * @brief Set render layer
         * @param layer Render layer
         */
        void SetRenderLayer(RenderLayer layer) { m_renderLayer = layer; }

        /**
         * @brief Get render layer
         * @return Render layer
         */
        RenderLayer GetRenderLayer() const { return m_renderLayer; }

        /**
         * @brief Enable/disable shadow casting
         * @param castShadows Shadow casting state
         */
        void SetCastShadows(bool castShadows) { m_castShadows = castShadows; }

        /**
         * @brief Check if casts shadows
         * @return true if casts shadows, false otherwise
         */
        bool CastsShadows() const { return m_castShadows; }

        /**
         * @brief Enable/disable shadow receiving
         * @param receiveShadows Shadow receiving state
         */
        void SetReceiveShadows(bool receiveShadows) { m_receiveShadows = receiveShadows; }

        /**
         * @brief Check if receives shadows
         * @return true if receives shadows, false otherwise
         */
        bool ReceivesShadows() const { return m_receiveShadows; }

        // Performance monitoring

        /**
         * @brief Get render statistics
         * @return Statistics string
         */
        std::string GetRenderStats() const;

        /**
         * @brief Reset render statistics
         */
        void ResetRenderStats();

    private:
        std::vector<RenderInstance> m_instances;     ///< Render instances
        bool m_isVisible;                            ///< Overall visibility
        float m_cullingDistance;                     ///< Distance culling distance
        RenderLayer m_renderLayer;                   ///< Render layer
        bool m_castShadows;                          ///< Cast shadows flag
        bool m_receiveShadows;                       ///< Receive shadows flag

        // Bounds
        glm::vec3 m_boundingBoxMin;                  ///< Bounding box minimum
        glm::vec3 m_boundingBoxMax;                  ///< Bounding box maximum

        // Statistics
        mutable uint64_t m_renderCount;              ///< Number of times rendered
        mutable uint64_t m_culledCount;              ///< Number of times culled
        mutable double m_lastRenderTime;             ///< Last render time
        mutable double m_totalRenderTime;            ///< Total render time
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_ENTITIES_RENDER_COMPONENT_HPP
