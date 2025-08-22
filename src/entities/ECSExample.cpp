/**
 * @file ECSExample.cpp
 * @brief VoxelCraft ECS - Example Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#include "ECSExample.hpp"
#include "Logger.hpp"

#include <algorithm>

namespace VoxelCraft {

    // Global instance
    std::unique_ptr<ECSExample> g_ECSExample;

    ECSExample::ECSExample()
        : m_entityManager(nullptr)
        , m_renderSystem(nullptr)
        , m_player(nullptr)
        , m_camera(nullptr)
        , m_initialized(false)
        , m_totalTime(0.0)
    {
        VOXELCRAFT_TRACE("ECSExample created");
    }

    ECSExample::~ECSExample() {
        VOXELCRAFT_TRACE("ECSExample destroyed");
        Shutdown();
    }

    bool ECSExample::Initialize() {
        if (m_initialized) {
            return true;
        }

        VOXELCRAFT_INFO("Initializing ECS Example...");

        try {
            // Create entity manager
            m_entityManager = std::make_unique<EntityManager>();

            // Setup systems
            SetupSystems();

            // Create example entities
            CreateExampleEntities();

            m_initialized = true;
            VOXELCRAFT_INFO("ECS Example initialized successfully with {} entities",
                          m_entityManager->GetEntityCount());

            return true;

        } catch (const std::exception& e) {
            VOXELCRAFT_ERROR("Failed to initialize ECS Example: {}", e.what());
            return false;
        }
    }

    void ECSExample::Update(double deltaTime) {
        if (!m_initialized) {
            return;
        }

        m_totalTime += deltaTime;

        // Update entity manager
        if (m_entityManager) {
            m_entityManager->UpdateEntities(deltaTime);
        }

        // Update render system
        if (m_renderSystem) {
            m_renderSystem->Update(deltaTime);
        }

        // Update game logic
        UpdateGameLogic(deltaTime);
    }

    void ECSExample::Render() {
        if (!m_initialized) {
            return;
        }

        // Render through render system
        if (m_renderSystem) {
            m_renderSystem->Render();
        }
    }

    void ECSExample::Shutdown() {
        if (!m_initialized) {
            return;
        }

        VOXELCRAFT_INFO("Shutting down ECS Example...");

        m_player = nullptr;
        m_camera = nullptr;

        if (m_renderSystem) {
            m_renderSystem->Shutdown();
            m_renderSystem.reset();
        }

        m_entityManager.reset();

        m_initialized = false;
        VOXELCRAFT_INFO("ECS Example shutdown complete");
    }

    Entity* ECSExample::CreateCubeEntity(const glm::vec3& position, const std::string& name) {
        if (!m_entityManager) {
            return nullptr;
        }

        auto entity = m_entityManager->CreateEntity(name);
        if (!entity) {
            return nullptr;
        }

        // Add transform component
        auto transform = entity->AddComponent<TransformComponent>(position);
        if (transform) {
            VOXELCRAFT_TRACE("Added TransformComponent to {} at ({}, {}, {})",
                           name, position.x, position.y, position.z);
        }

        // Add render component
        auto render = entity->AddComponent<RenderComponent>();
        if (render) {
            VOXELCRAFT_TRACE("Added RenderComponent to {}", name);
        }

        // Add physics component
        auto physics = entity->AddComponent<PhysicsComponent>(PhysicsBodyType::Dynamic);
        if (physics) {
            VOXELCRAFT_TRACE("Added PhysicsComponent to {}", name);
        }

        VOXELCRAFT_INFO("Created cube entity '{}' at position ({}, {}, {})",
                      name, position.x, position.y, position.z);
        return entity;
    }

    Entity* ECSExample::CreatePlayerEntity(const glm::vec3& position, const std::string& name) {
        if (!m_entityManager) {
            return nullptr;
        }

        auto entity = m_entityManager->CreateEntity(name);
        if (!entity) {
            return nullptr;
        }

        // Add transform component
        auto transform = entity->AddComponent<TransformComponent>(position);
        if (transform) {
            VOXELCRAFT_TRACE("Added TransformComponent to player '{}' at ({}, {}, {})",
                           name, position.x, position.y, position.z);
        }

        // Add render component
        auto render = entity->AddComponent<RenderComponent>();
        if (render) {
            VOXELCRAFT_TRACE("Added RenderComponent to player '{}'", name);
        }

        // Add physics component
        auto physics = entity->AddComponent<PhysicsComponent>(PhysicsBodyType::Dynamic);
        if (physics) {
            VOXELCRAFT_TRACE("Added PhysicsComponent to player '{}'", name);
        }

        // Add player component
        auto player = entity->AddComponent<PlayerComponent>(PlayerType::Human, name);
        if (player) {
            VOXELCRAFT_TRACE("Added PlayerComponent to player '{}'", name);
        }

        m_player = entity;
        VOXELCRAFT_INFO("Created player entity '{}' at position ({}, {}, {})",
                      name, position.x, position.y, position.z);
        return entity;
    }

    Entity* ECSExample::CreateCameraEntity(const glm::vec3& position) {
        if (!m_entityManager) {
            return nullptr;
        }

        auto entity = m_entityManager->CreateEntity("Camera");
        if (!entity) {
            return nullptr;
        }

        // Add transform component
        auto transform = entity->AddComponent<TransformComponent>(position);
        if (transform) {
            // Look towards origin
            transform->LookAt(glm::vec3(0.0f, 0.0f, 0.0f));
            VOXELCRAFT_TRACE("Added TransformComponent to camera at ({}, {}, {})",
                           position.x, position.y, position.z);
        }

        m_camera = entity;
        VOXELCRAFT_INFO("Created camera entity at position ({}, {}, {})",
                      position.x, position.y, position.z);
        return entity;
    }

    void ECSExample::SetupSystems() {
        if (!m_entityManager) {
            return;
        }

        // Create render system
        m_renderSystem = std::make_unique<RenderSystem>("ECSExampleRenderSystem");
        m_renderSystem->SetEntityManager(m_entityManager.get());

        if (!m_renderSystem->Initialize()) {
            VOXELCRAFT_ERROR("Failed to initialize render system");
            return;
        }

        VOXELCRAFT_INFO("Render system initialized for ECS Example");
    }

    void ECSExample::CreateExampleEntities() {
        if (!m_entityManager) {
            return;
        }

        VOXELCRAFT_INFO("Creating example entities...");

        // Create camera
        CreateCameraEntity(glm::vec3(0.0f, 5.0f, 10.0f));

        // Create player
        CreatePlayerEntity(glm::vec3(0.0f, 2.0f, 0.0f), "ExamplePlayer");

        // Create some cube entities
        CreateCubeEntity(glm::vec3(3.0f, 1.0f, 3.0f), "Cube1");
        CreateCubeEntity(glm::vec3(-3.0f, 1.0f, 3.0f), "Cube2");
        CreateCubeEntity(glm::vec3(3.0f, 1.0f, -3.0f), "Cube3");
        CreateCubeEntity(glm::vec3(-3.0f, 1.0f, -3.0f), "Cube4");

        // Set camera for render system
        if (m_renderSystem && m_camera) {
            m_renderSystem->SetActiveCamera(m_camera);
        }

        VOXELCRAFT_INFO("Created {} example entities",
                      m_entityManager->GetEntityCount());
    }

    void ECSExample::UpdateGameLogic(double deltaTime) {
        // Simple game logic example

        // Make camera orbit around the scene
        if (m_camera) {
            auto transform = m_camera->GetComponent<TransformComponent>();
            if (transform) {
                float radius = 15.0f;
                float height = 5.0f;
                float speed = 0.5f;

                float x = std::sin(m_totalTime * speed) * radius;
                float z = std::cos(m_totalTime * speed) * radius;
                float y = height;

                transform->SetPosition(x, y, z);
                transform->LookAt(glm::vec3(0.0f, 2.0f, 0.0f));
            }
        }

        // Make cubes rotate
        auto entities = m_entityManager->GetAllEntities();
        for (auto entity : entities) {
            if (entity->GetName().find("Cube") != std::string::npos) {
                auto transform = entity->GetComponent<TransformComponent>();
                if (transform) {
                    // Rotate around Y axis
                    transform->Rotate(0.0f, static_cast<float>(deltaTime) * 45.0f, 0.0f);
                }
            }
        }

        // Log statistics every 5 seconds
        static double lastLogTime = 0.0;
        if (m_totalTime - lastLogTime >= 5.0) {
            lastLogTime = m_totalTime;

            if (m_renderSystem) {
                auto stats = m_renderSystem->GetStatistics();
                VOXELCRAFT_INFO("ECS Example Stats - Entities: {}, Visible: {}, Culled: {}, DrawCalls: {}",
                              m_entityManager->GetEntityCount(),
                              stats.visibleEntities,
                              stats.culledEntities,
                              stats.totalDrawCalls);
            }
        }
    }

    // Global functions

    bool InitializeECSExample() {
        if (g_ECSExample) {
            return true; // Already initialized
        }

        VOXELCRAFT_INFO("Initializing global ECS Example...");
        g_ECSExample = std::make_unique<ECSExample>();

        if (!g_ECSExample->Initialize()) {
            VOXELCRAFT_ERROR("Failed to initialize global ECS Example");
            g_ECSExample.reset();
            return false;
        }

        return true;
    }

    void ShutdownECSExample() {
        if (g_ECSExample) {
            VOXELCRAFT_INFO("Shutting down global ECS Example...");
            g_ECSExample->Shutdown();
            g_ECSExample.reset();
        }
    }

    void UpdateECSExample(double deltaTime) {
        if (g_ECSExample) {
            g_ECSExample->Update(deltaTime);
        }
    }

    void RenderECSExample() {
        if (g_ECSExample) {
            g_ECSExample->Render();
        }
    }

} // namespace VoxelCraft
