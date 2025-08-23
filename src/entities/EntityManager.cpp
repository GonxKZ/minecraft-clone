#include "EntityManager.hpp"
#include "Entity.hpp"
#include "Component.hpp"
#include "System.hpp"
#include <algorithm>
#include <sstream>
#include <iostream>

namespace VoxelCraft {

    // Variable estática para generar IDs únicos de entidades
    std::atomic<Entity::EntityID> EntityManager::s_NextEntityID{1};

    EntityManager& EntityManager::GetInstance() {
        static EntityManager instance;
        return instance;
    }

    Entity* EntityManager::CreateEntity(const std::string& name) {
        std::unique_lock lock(m_EntityMutex);

        EntityID id = GenerateEntityID();
        auto entity = std::make_unique<Entity>(id, name, this);
        Entity* rawPtr = entity.get();

        m_Entities[id] = std::move(entity);
        AddEntityToActive(rawPtr);

        m_Stats.totalEntitiesCreated++;

        return rawPtr;
    }

    Entity* EntityManager::GetEntity(EntityID id) const {
        std::shared_lock lock(m_EntityMutex);

        auto it = m_Entities.find(id);
        return (it != m_Entities.end()) ? it->second.get() : nullptr;
    }

    bool EntityManager::DestroyEntity(EntityID id) {
        std::unique_lock lock(m_EntityMutex);

        auto it = m_Entities.find(id);
        if (it == m_Entities.end()) {
            return false;
        }

        Entity* entity = it->second.get();
        entity->Destroy();
        AddEntityToPendingDestroy(entity);

        m_Stats.totalEntitiesDestroyed++;
        return true;
    }

    void EntityManager::DestroyAllEntities() {
        std::unique_lock lock(m_EntityMutex);

        for (auto& pair : m_Entities) {
            pair.second->Destroy();
            AddEntityToPendingDestroy(pair.second.get());
        }

        m_Stats.totalEntitiesDestroyed += m_Entities.size();
    }

    size_t EntityManager::GetEntityCount() const {
        std::shared_lock lock(m_EntityMutex);
        return m_Entities.size();
    }

    void EntityManager::Update(float deltaTime) {
        // Procesar destrucciones pendientes
        ProcessPendingDestroys();

        // Actualizar entidades activas
        {
            std::shared_lock lock(m_EntityMutex);
            for (Entity* entity : m_ActiveEntities) {
                if (entity && entity->IsActive()) {
                    entity->Update(deltaTime);
                }
            }
        }

        // Actualizar sistemas
        UpdateSystems(deltaTime);
    }

    void EntityManager::FixedUpdate(float fixedDeltaTime) {
        // Actualizar entidades activas
        {
            std::shared_lock lock(m_EntityMutex);
            for (Entity* entity : m_ActiveEntities) {
                if (entity && entity->IsActive()) {
                    entity->FixedUpdate(fixedDeltaTime);
                }
            }
        }

        // Actualizar sistemas
        FixedUpdateSystems(fixedDeltaTime);
    }

    void EntityManager::LateUpdate(float deltaTime) {
        // Actualizar entidades activas
        {
            std::shared_lock lock(m_EntityMutex);
            for (Entity* entity : m_ActiveEntities) {
                if (entity && entity->IsActive()) {
                    entity->LateUpdate(deltaTime);
                }
            }
        }

        // Actualizar sistemas
        LateUpdateSystems(deltaTime);
    }

    void EntityManager::ProcessPendingDestroys() {
        std::unique_lock lock(m_EntityMutex);

        for (Entity* entity : m_PendingDestroyEntities) {
            if (entity) {
                RemoveEntityFromActive(entity);
                RemoveEntityFromInactive(entity);
                m_Entities.erase(entity->GetID());
            }
        }

        m_PendingDestroyEntities.clear();
    }

    void EntityManager::CleanupDestroyedEntities() {
        std::unique_lock lock(m_EntityMutex);

        // Limpiar entidades marcadas como destruidas
        for (auto it = m_Entities.begin(); it != m_Entities.end();) {
            if (it->second->IsDestroyed()) {
                it = m_Entities.erase(it);
            } else {
                ++it;
            }
        }
    }

    std::string EntityManager::ToString() const {
        std::shared_lock lock(m_EntityMutex);
        std::shared_lock systemLock(m_SystemMutex);

        std::stringstream ss;
        ss << "EntityManager[Entities=" << m_Entities.size()
           << ", Active=" << m_ActiveEntities.size()
           << ", Inactive=" << m_InactiveEntities.size()
           << ", PendingDestroy=" << m_PendingDestroyEntities.size()
           << ", Systems=" << m_Systems.size() << "]";

        return ss.str();
    }

    void EntityManager::LogStatistics() const {
        auto now = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(
            now - m_Stats.startTime).count();

        std::cout << "=== EntityManager Statistics ===" << std::endl;
        std::cout << "Total Entities Created: " << m_Stats.totalEntitiesCreated << std::endl;
        std::cout << "Total Entities Destroyed: " << m_Stats.totalEntitiesDestroyed << std::endl;
        std::cout << "Total Components Created: " << m_Stats.totalComponentsCreated << std::endl;
        std::cout << "Total Components Destroyed: " << m_Stats.totalComponentsDestroyed << std::endl;
        std::cout << "Current Entity Count: " << m_Entities.size() << std::endl;
        std::cout << "Active Entities: " << m_ActiveEntities.size() << std::endl;
        std::cout << "Systems Count: " << m_Systems.size() << std::endl;
        std::cout << "Uptime: " << duration << " seconds" << std::endl;
        std::cout << "===============================" << std::endl;
    }

    // Métodos privados
    Entity::EntityID EntityManager::GenerateEntityID() {
        return s_NextEntityID.fetch_add(1, std::memory_order_relaxed);
    }

    void EntityManager::AddEntityToActive(Entity* entity) {
        if (std::find(m_ActiveEntities.begin(), m_ActiveEntities.end(), entity) == m_ActiveEntities.end()) {
            m_ActiveEntities.push_back(entity);
        }
    }

    void EntityManager::RemoveEntityFromActive(Entity* entity) {
        auto it = std::remove(m_ActiveEntities.begin(), m_ActiveEntities.end(), entity);
        if (it != m_ActiveEntities.end()) {
            m_ActiveEntities.erase(it, m_ActiveEntities.end());
        }
    }

    void EntityManager::AddEntityToInactive(Entity* entity) {
        if (std::find(m_InactiveEntities.begin(), m_InactiveEntities.end(), entity) == m_InactiveEntities.end()) {
            m_InactiveEntities.push_back(entity);
        }
    }

    void EntityManager::RemoveEntityFromInactive(Entity* entity) {
        auto it = std::remove(m_InactiveEntities.begin(), m_InactiveEntities.end(), entity);
        if (it != m_InactiveEntities.end()) {
            m_InactiveEntities.erase(it, m_InactiveEntities.end());
        }
    }

    void EntityManager::AddEntityToPendingDestroy(Entity* entity) {
        if (std::find(m_PendingDestroyEntities.begin(), m_PendingDestroyEntities.end(), entity) == m_PendingDestroyEntities.end()) {
            m_PendingDestroyEntities.push_back(entity);
        }
    }

    void EntityManager::RemoveEntityFromPendingDestroy(Entity* entity) {
        auto it = std::remove(m_PendingDestroyEntities.begin(), m_PendingDestroyEntities.end(), entity);
        if (it != m_PendingDestroyEntities.end()) {
            m_PendingDestroyEntities.erase(it, m_PendingDestroyEntities.end());
        }
    }

    void EntityManager::RegisterComponent(Component* component) {
        std::unique_lock lock(m_ComponentMutex);

        auto typeIndex = component->GetType();
        m_ComponentsByType[typeIndex].push_back(component);

        m_Stats.totalComponentsCreated++;
    }

    void EntityManager::UnregisterComponent(Component* component) {
        std::unique_lock lock(m_ComponentMutex);

        auto typeIndex = component->GetType();
        auto& components = m_ComponentsByType[typeIndex];

        auto it = std::remove(components.begin(), components.end(), component);
        if (it != components.end()) {
            components.erase(it, components.end());
        }

        m_Stats.totalComponentsDestroyed++;
    }

    void EntityManager::UpdateSystems(float deltaTime) {
        std::shared_lock lock(m_SystemMutex);

        for (const auto& system : m_Systems) {
            if (system) {
                system->Update(deltaTime);
            }
        }
    }

    void EntityManager::FixedUpdateSystems(float fixedDeltaTime) {
        std::shared_lock lock(m_SystemMutex);

        for (const auto& system : m_Systems) {
            if (system) {
                system->FixedUpdate(fixedDeltaTime);
            }
        }
    }

    void EntityManager::LateUpdateSystems(float deltaTime) {
        std::shared_lock lock(m_SystemMutex);

        for (const auto& system : m_Systems) {
            if (system) {
                system->LateUpdate(deltaTime);
            }
        }
    }

}