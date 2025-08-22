/**
 * @file RenderComponent.hpp
 * @brief VoxelCraft ECS - Render Component
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_ENTITIES_RENDER_COMPONENT_HPP
#define VOXELCRAFT_ENTITIES_RENDER_COMPONENT_HPP

#include <memory>
#include <vector>
#include <array>
#include "Component.hpp"

namespace VoxelCraft {

    /**
     * @enum RenderType
     * @brief Types of renderable objects
     */
    enum class RenderType {
        STATIC_MESH = 0,     ///< Static 3D mesh
        ANIMATED_MESH,       ///< Animated 3D mesh
        BILLBOARD,           ///< 2D billboard (particles, sprites)
        TERRAIN_CHUNK,       ///< Terrain chunk mesh
        WATER,               ///< Water surface
        SKYBOX,              ///< Skybox
        UI_ELEMENT           ///< UI element
    };

    /**
     * @enum RenderLayer
     * @brief Rendering layers for ordering
     */
    enum class RenderLayer {
        BACKGROUND = 0,      ///< Skybox, background
        TERRAIN,             ///< Terrain chunks
        STATIC_OBJECTS,      ///< Static objects
        ANIMATED_OBJECTS,    ///< Animated objects
        TRANSPARENT,         ///< Transparent objects
        PARTICLES,           ///< Particle effects
        UI                   ///< UI elements
    };

    /**
     * @struct RenderMaterial
     * @brief Material properties for rendering
     */
    struct RenderMaterial {
        std::string texturePath;          ///< Path to main texture
        std::string normalMapPath;        ///< Path to normal map
        std::string specularMapPath;      ///< Path to specular map
        std::array<float, 4> color;       ///< RGBA color
        float shininess;                  ///< Material shininess
        float transparency;               ///< Transparency (0-1)

        RenderMaterial()
            : color({1.0f, 1.0f, 1.0f, 1.0f})
            , shininess(32.0f)
            , transparency(1.0f)
        {}
    };

    /**
     * @struct RenderTransform
     * @brief Transform information for rendering
     */
    struct RenderTransform {
        std::array<float, 3> position;    ///< World position
        std::array<float, 3> rotation;    ///< Rotation (degrees)
        std::array<float, 3> scale;       ///< Scale factors

        RenderTransform()
            : position({0.0f, 0.0f, 0.0f})
            , rotation({0.0f, 0.0f, 0.0f})
            , scale({1.0f, 1.0f, 1.0f})
        {}
    };

    /**
     * @class RenderComponent
     * @brief Component containing rendering information for entities
     */
    class RenderComponent : public Component {
    public:
        /**
         * @brief Constructor
         */
        RenderComponent();

        /**
         * @brief Constructor with render type
         * @param type Render type
         */
        explicit RenderComponent(RenderType type);

        /**
         * @brief Destructor
         */
        ~RenderComponent() override;

        /**
         * @brief Get component type
         * @return Component type
         */
        ComponentType GetType() const override { return ComponentType::RENDER; }

        /**
         * @brief Update component
         * @param deltaTime Time since last update
         */
        void Update(float deltaTime) override;

        /**
         * @brief Check if component is enabled
         * @return true if enabled
         */
        bool IsEnabled() const { return m_enabled; }

        /**
         * @brief Enable or disable rendering
         * @param enabled Whether to enable rendering
         */
        void SetEnabled(bool enabled) { m_enabled = enabled; }

        /**
         * @brief Get render type
         * @return Render type
         */
        RenderType GetRenderType() const { return m_renderType; }

        /**
         * @brief Set render type
         * @param type New render type
         */
        void SetRenderType(RenderType type) { m_renderType = type; }

        /**
         * @brief Get render layer
         * @return Render layer
         */
        RenderLayer GetRenderLayer() const { return m_renderLayer; }

        /**
         * @brief Set render layer
         * @param layer New render layer
         */
        void SetRenderLayer(RenderLayer layer) { m_renderLayer = layer; }

        /**
         * @brief Get render material
         * @return Render material
         */
        const RenderMaterial& GetMaterial() const { return m_material; }

        /**
         * @brief Set render material
         * @param material New render material
         */
        void SetMaterial(const RenderMaterial& material) { m_material = material; }

        /**
         * @brief Get render transform
         * @return Render transform
         */
        const RenderTransform& GetTransform() const { return m_transform; }

        /**
         * @brief Set render transform
         * @param transform New render transform
         */
        void SetTransform(const RenderTransform& transform) { m_transform = transform; }

        /**
         * @brief Check if object is visible
         * @return true if visible
         */
        bool IsVisible() const { return m_visible; }

        /**
         * @brief Set visibility
         * @param visible Whether object is visible
         */
        void SetVisible(bool visible) { m_visible = visible; }

        /**
         * @brief Get bounding box min point
         * @return Min point
         */
        const std::array<float, 3>& GetBoundingBoxMin() const { return m_boundingBoxMin; }

        /**
         * @brief Get bounding box max point
         * @return Max point
         */
        const std::array<float, 3>& GetBoundingBoxMax() const { return m_boundingBoxMax; }

        /**
         * @brief Set bounding box
         * @param min Min point
         * @param max Max point
         */
        void SetBoundingBox(const std::array<float, 3>& min, const std::array<float, 3>& max);

        /**
         * @brief Check if bounding box is valid
         * @return true if valid
         */
        bool HasValidBoundingBox() const { return m_hasValidBoundingBox; }

        /**
         * @brief Get render distance
         * @return Render distance
         */
        float GetRenderDistance() const { return m_renderDistance; }

        /**
         * @brief Set render distance
         * @param distance New render distance
         */
        void SetRenderDistance(float distance) { m_renderDistance = distance; }

        /**
         * @brief Check if should cast shadows
         * @return true if casts shadows
         */
        bool CastsShadows() const { return m_castsShadows; }

        /**
         * @brief Set shadow casting
         * @param castsShadows Whether to cast shadows
         */
        void SetCastsShadows(bool castsShadows) { m_castsShadows = castsShadows; }

        /**
         * @brief Get custom render data
         * @return Custom render data pointer
         */
        void* GetCustomRenderData() const { return m_customRenderData; }

        /**
         * @brief Set custom render data
         * @param data Custom render data
         */
        void SetCustomRenderData(void* data) { m_customRenderData = data; }

    private:
        bool m_enabled;                              ///< Whether rendering is enabled
        RenderType m_renderType;                     ///< Type of renderable object
        RenderLayer m_renderLayer;                   ///< Rendering layer
        RenderMaterial m_material;                   ///< Material properties
        RenderTransform m_transform;                 ///< Transform information
        bool m_visible;                              ///< Visibility flag
        std::array<float, 3> m_boundingBoxMin;       ///< Bounding box minimum point
        std::array<float, 3> m_boundingBoxMax;       ///< Bounding box maximum point
        bool m_hasValidBoundingBox;                  ///< Whether bounding box is valid
        float m_renderDistance;                      ///< Maximum render distance
        bool m_castsShadows;                         ///< Whether object casts shadows
        void* m_customRenderData;                    ///< Custom render data pointer
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_ENTITIES_RENDER_COMPONENT_HPP
