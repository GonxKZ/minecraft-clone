/**
 * @file ECSExample.hpp
 * @brief VoxelCraft ECS - Example Usage
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * Example demonstrating how to use the complete ECS system.
 */

#ifndef VOXELCRAFT_ENTITIES_ECS_EXAMPLE_HPP
#define VOXELCRAFT_ENTITIES_ECS_EXAMPLE_HPP

#include "Entity.hpp"
#include "EntityManager.hpp"
#include "TransformComponent.hpp"
#include "RenderComponent.hpp"
#include "PhysicsComponent.hpp"
#include "PlayerComponent.hpp"
#include "RenderSystem.hpp"
#include "Logger.hpp"

#include <memory>

namespace VoxelCraft {

    /**
     * @class ECSExample
     * @brief Example class demonstrating ECS usage
     *
     * This class shows how to:
     * - Create entities with components
     * - Set up systems
     * - Process game logic
     * - Handle rendering
     */
    class ECSExample {
    public:
        /**
         * @brief Constructor
         */
        ECSExample();

        /**
         * @brief Destructor
         */
        ~ECSExample();

        /**
         * @brief Initialize the example
         * @return true if successful, false otherwise
         */
        bool Initialize();

        /**
         * @brief Update the example
         * @param deltaTime Time elapsed since last update
         */
        void Update(double deltaTime);

        /**
         * @brief Render the example
         */
        void Render();

        /**
         * @brief Shutdown the example
         */
        void Shutdown();

        /**
         * @brief Create a simple cube entity
         * @param position Position of the cube
         * @param name Name of the entity
         * @return Created entity or nullptr
         */
        Entity* CreateCubeEntity(const glm::vec3& position, const std::string& name = "Cube");

        /**
         * @brief Create a player entity
         * @param position Position of the player
         * @param name Name of the player
         * @return Created entity or nullptr
         */
        Entity* CreatePlayerEntity(const glm::vec3& position, const std::string& name = "Player");

        /**
         * @brief Create a camera entity
         * @param position Position of the camera
         * @return Created entity or nullptr
         */
        Entity* CreateCameraEntity(const glm::vec3& position);

        /**
         * @brief Get entity manager
         * @return Entity manager
         */
        EntityManager* GetEntityManager() const { return m_entityManager.get(); }

        /**
         * @brief Get render system
         * @return Render system
         */
        RenderSystem* GetRenderSystem() const { return m_renderSystem.get(); }

        /**
         * @brief Get player entity
         * @return Player entity
         */
        Entity* GetPlayer() const { return m_player; }

        /**
         * @brief Get camera entity
         * @return Camera entity
         */
        Entity* GetCamera() const { return m_camera; }

    private:
        /**
         * @brief Setup systems
         */
        void SetupSystems();

        /**
         * @brief Create example entities
         */
        void CreateExampleEntities();

        /**
         * @brief Update game logic
         * @param deltaTime Time elapsed since last update
         */
        void UpdateGameLogic(double deltaTime);

        std::unique_ptr<EntityManager> m_entityManager;    ///< Entity manager
        std::unique_ptr<RenderSystem> m_renderSystem;      ///< Render system

        Entity* m_player;                                  ///< Player entity
        Entity* m_camera;                                  ///< Camera entity

        bool m_initialized;                                ///< Initialization flag
        double m_totalTime;                                ///< Total time elapsed
    };

    /**
     * @brief Global ECS example instance
     */
    extern std::unique_ptr<ECSExample> g_ECSExample;

    /**
     * @brief Initialize global ECS example
     * @return true if successful, false otherwise
     */
    bool InitializeECSExample();

    /**
     * @brief Shutdown global ECS example
     */
    void ShutdownECSExample();

    /**
     * @brief Update global ECS example
     * @param deltaTime Time elapsed since last update
     */
    void UpdateECSExample(double deltaTime);

    /**
     * @brief Render global ECS example
     */
    void RenderECSExample();

} // namespace VoxelCraft

#endif // VOXELCRAFT_ENTITIES_ECS_EXAMPLE_HPP
