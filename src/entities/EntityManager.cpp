/**
 * @file EntityManager.cpp
 * @brief VoxelCraft Entity System - Entity Manager Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file implements the EntityManager class for managing entities in the ECS system.
 */

#include "EntityManager.hpp"
#include "Logger.hpp"

#include <algorithm>

namespace VoxelCraft {

    // Static member initialization
    std::atomic<EntityID> EntityManager::s_nextEntityId{1};

    EntityManager::EntityManager() {
        VOXELCRAFT_TRACE("EntityManager instance created");
        ResetStatistics();
    }

    EntityManager::~EntityManager() {
        VOXELCRAFT_TRACE("EntityManager instance destroyed");

        // Clear all entities
        ClearAllEntities(true);
    }

    Entity* EntityManager::CreateEntity(const std::string& name) {
        std::unique_lock<std::shared_mutex> lock(m_entityMutex);

        EntityID entityId = GenerateEntityId();
        std::string entityName = name.empty() ? "Entity_" + std::to_string(entityId) : name;

        // Check if name already exists
        if (m_entityNames.count(entityName) > 0) {
            entityName += "_" + std::to_string(entityId);
        }

        auto entity = std::make_unique<Entity>(entityId, entityName, this);
        Entity* entityPtr = entity.get();

        AddEntity(entityPtr);
        m_entities[entityId] = std::move(entity);

        // Update statistics
        {
            std::lock_guard<std::mutex> statsLock(m_statsMutex);
            m_stats.totalEntities++;
            m_stats.activeEntities++;
            m_stats.createdEntities++;
        }

        VOXELCRAFT_INFO("Entity '{}' (ID: {}) created", entityName, entityId);
        return entityPtr;
    }

    bool EntityManager::DestroyEntity(Entity* entity) {
        if (!entity) {
            return false;
        }

        return DestroyEntity(entity->GetID());
    }

    bool EntityManager::DestroyEntity(EntityID entityId) {
        std::unique_lock<std::shared_mutex> lock(m_entityMutex);

        auto it = m_entities.find(entityId);
        if (it == m_entities.end()) {
            return false;
        }

        Entity* entity = it->second.get();
        std::string name = entity->GetName();

        // Mark entity as destroyed
        entity->SetState(EntityState::Destroyed);

        // Remove from collections
        RemoveEntity(entityId);
        m_entities.erase(it);

        // Update statistics
        {
            std::lock_guard<std::mutex> statsLock(m_statsMutex);
            m_stats.totalEntities--;
            m_stats.destroyedEntities++;

            if (entity->IsActive()) {
                m_stats.activeEntities--;
            } else {
                m_stats.inactiveEntities--;
            }
        }

        VOXELCRAFT_INFO("Entity '{}' (ID: {}) destroyed", name, entityId);
        return true;
    }

    bool EntityManager::MarkEntityForDestruction(Entity* entity) {
        if (!entity) {
            return false;
        }

        std::unique_lock<std::shared_mutex> lock(m_entityMutex);

        if (entity->GetState() != EntityState::PendingDestroy) {
            entity->SetState(EntityState::PendingDestroy);
            m_pendingDestroy.push(entity->GetID());

            // Update statistics
            {
                std::lock_guard<std::mutex> statsLock(m_statsMutex);
                m_stats.pendingDestroyEntities++;
                if (entity->IsActive()) {
                    m_stats.activeEntities--;
                }
            }

            VOXELCRAFT_TRACE("Entity '{}' (ID: {}) marked for destruction",
                           entity->GetName(), entity->GetID());
            return true;
        }

        return false;
    }

    Entity* EntityManager::GetEntity(EntityID entityId) {
        std::shared_lock<std::shared_mutex> lock(m_entityMutex);
        auto it = m_entities.find(entityId);
        return it != m_entities.end() ? it->second.get() : nullptr;
    }

    const Entity* EntityManager::GetEntity(EntityID entityId) const {
        std::shared_lock<std::shared_mutex> lock(m_entityMutex);
        auto it = m_entities.find(entityId);
        return it != m_entities.end() ? it->second.get() : nullptr;
    }

    Entity* EntityManager::FindEntity(const std::string& name) {
        std::shared_lock<std::shared_mutex> lock(m_entityMutex);
        auto it = m_entityNames.find(name);
        if (it != m_entityNames.end()) {
            return GetEntity(it->second);
        }
        return nullptr;
    }

    bool EntityManager::EntityExists(EntityID entityId) const {
        std::shared_lock<std::shared_mutex> lock(m_entityMutex);
        return m_entities.count(entityId) > 0;
    }

    std::vector<Entity*> EntityManager::GetAllEntities() {
        std::shared_lock<std::shared_mutex> lock(m_entityMutex);
        std::vector<Entity*> entities;
        entities.reserve(m_entities.size());

        for (const auto& pair : m_entities) {
            entities.push_back(pair.second.get());
        }

        return entities;
    }

    std::vector<Entity*> EntityManager::GetActiveEntities() {
        std::shared_lock<std::shared_mutex> lock(m_entityMutex);
        std::vector<Entity*> activeEntities;

        for (const auto& pair : m_entities) {
            if (pair.second->IsActive()) {
                activeEntities.push_back(pair.second.get());
            }
        }

        return activeEntities;
    }

    std::vector<Entity*> EntityManager::GetEntities(const EntityFilter& filter) {
        std::shared_lock<std::shared_mutex> lock(m_entityMutex);
        std::vector<Entity*> filteredEntities;

        for (const auto& pair : m_entities) {
            if (filter(pair.second.get())) {
                filteredEntities.push_back(pair.second.get());
            }
        }

        return filteredEntities;
    }

    void EntityManager::ProcessActiveEntities(const EntityProcessor& processor) {
        std::vector<Entity*> activeEntities = GetActiveEntities();

        for (Entity* entity : activeEntities) {
            if (entity->IsActive()) {
                processor(entity);
            }
        }
    }

    void EntityManager::ProcessEntities(const EntityFilter& filter, const EntityProcessor& processor) {
        std::vector<Entity*> filteredEntities = GetEntities(filter);

        for (Entity* entity : filteredEntities) {
            processor(entity);
        }
    }

    void EntityManager::UpdateEntities(double deltaTime) {
        std::vector<Entity*> activeEntities = GetActiveEntities();

        for (Entity* entity : activeEntities) {
            if (entity->IsActive()) {
                entity->Update(deltaTime);
            }
        }
    }

    void EntityManager::RenderEntities() {
        std::vector<Entity*> activeEntities = GetActiveEntities();

        for (Entity* entity : activeEntities) {
            if (entity->IsActive()) {
                entity->Render();
            }
        }
    }

    size_t EntityManager::CleanupDestroyedEntities() {
        size_t cleanedCount = 0;

        while (!m_pendingDestroy.empty()) {
            EntityID entityId = m_pendingDestroy.front();
            m_pendingDestroy.pop();

            if (DestroyEntity(entityId)) {
                cleanedCount++;
            }
        }

        if (cleanedCount > 0) {
            VOXELCRAFT_INFO("Cleaned up {} destroyed entities", cleanedCount);
        }

        return cleanedCount;
    }

    void EntityManager::ClearAllEntities(bool force) {
        std::unique_lock<std::shared_mutex> lock(m_entityMutex);

        size_t entityCount = m_entities.size();

        if (force) {
            // Force destroy all entities
            for (auto& pair : m_entities) {
                pair.second->SetState(EntityState::Destroyed);
            }
        } else {
            // Mark all for destruction
            for (auto& pair : m_entities) {
                if (pair.second->GetState() != EntityState::PendingDestroy) {
                    pair.second->SetState(EntityState::PendingDestroy);
                    m_pendingDestroy.push(pair.first);
                }
            }
        }

        // Clear collections
        m_entities.clear();
        m_entityNames.clear();

        while (!m_pendingDestroy.empty()) {
            m_pendingDestroy.pop();
        }

        // Reset statistics
        ResetStatistics();

        VOXELCRAFT_INFO("Cleared all {} entities", entityCount);
    }

    EntityManagerStats EntityManager::GetStatistics() const {
        std::lock_guard<std::mutex> statsLock(m_statsMutex);

        // Update current statistics
        EntityManagerStats stats = m_stats;
        stats.totalEntities = m_entities.size();
        stats.pendingDestroyEntities = m_pendingDestroy.size();

        // Count active/inactive entities
        size_t activeCount = 0;
        size_t inactiveCount = 0;
        size_t totalComponents = 0;

        {
            std::shared_lock<std::shared_mutex> entityLock(m_entityMutex);
            for (const auto& pair : m_entities) {
                const Entity* entity = pair.second.get();
                if (entity->IsActive()) {
                    activeCount++;
                } else {
                    inactiveCount++;
                }
                totalComponents += entity->GetComponentCount();
            }
        }

        stats.activeEntities = activeCount;
        stats.inactiveEntities = inactiveCount;
        stats.totalComponents = totalComponents;
        stats.averageComponentsPerEntity = stats.totalEntities > 0 ?
            static_cast<double>(totalComponents) / stats.totalEntities : 0.0;

        return stats;
    }

    void EntityManager::ResetStatistics() {
        std::lock_guard<std::mutex> statsLock(m_statsMutex);
        m_stats = EntityManagerStats{};
    }

    size_t EntityManager::GetEntityCount() const {
        std::shared_lock<std::shared_mutex> lock(m_entityMutex);
        return m_entities.size();
    }

    size_t EntityManager::GetActiveEntityCount() const {
        std::shared_lock<std::shared_mutex> lock(m_entityMutex);
        return std::count_if(m_entities.begin(), m_entities.end(),
            [](const auto& pair) { return pair.second->IsActive(); });
    }

    size_t EntityManager::GetPendingDestroyCount() const {
        return m_pendingDestroy.size();
    }

    EntityID EntityManager::GenerateEntityId() {
        return s_nextEntityId.fetch_add(1, std::memory_order_relaxed);
    }

    void EntityManager::AddEntity(Entity* entity) {
        m_entities[entity->GetID()] = std::unique_ptr<Entity>(entity);
        m_entityNames[entity->GetName()] = entity->GetID();
    }

    void EntityManager::RemoveEntity(EntityID entityId) {
        auto it = m_entities.find(entityId);
        if (it != m_entities.end()) {
            std::string name = it->second->GetName();
            m_entityNames.erase(name);
            m_entities.erase(it);
        }
    }

} // namespace VoxelCraft
