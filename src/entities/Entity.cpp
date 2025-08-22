/**
 * @file Entity.cpp
 * @brief VoxelCraft Entity System - Entity Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file implements the Entity class for the VoxelCraft ECS system.
 */

#include "Entity.hpp"
#include "Component.hpp"
#include "Logger.hpp"

#include <algorithm>
#include <typeinfo>

namespace VoxelCraft {

    Entity::Entity(EntityID id, const std::string& name, EntityManager* manager)
        : m_id(id)
        , m_name(name)
        , m_state(EntityState::Active)
        , m_manager(manager)
    {
        VOXELCRAFT_TRACE("Entity '{}' (ID: {}) created", name, id);
    }

    Entity::~Entity() {
        VOXELCRAFT_TRACE("Entity '{}' (ID: {}) destroyed", m_name, m_id);

        // Clean up all components
        m_components.clear();
    }

    void Entity::Update(double deltaTime) {
        if (m_state != EntityState::Active) {
            return;
        }

        // Update all components
        for (auto& pair : m_components) {
            if (pair.second && pair.second->IsEnabled()) {
                pair.second->Update(deltaTime);
            }
        }
    }

    void Entity::Render() {
        if (m_state != EntityState::Active) {
            return;
        }

        // Render all components
        for (auto& pair : m_components) {
            if (pair.second && pair.second->IsEnabled()) {
                pair.second->Render();
            }
        }
    }

    void Entity::Destroy() {
        if (m_state != EntityState::PendingDestroy) {
            m_state = EntityState::PendingDestroy;
            VOXELCRAFT_INFO("Entity '{}' (ID: {}) marked for destruction", m_name, m_id);
        }
    }

    std::vector<Component*> Entity::GetComponents() {
        std::vector<Component*> components;
        components.reserve(m_components.size());

        for (auto& pair : m_components) {
            if (pair.second) {
                components.push_back(pair.second.get());
            }
        }

        return components;
    }

    void Entity::AddComponentInternal(Component* component) {
        if (!component) {
            return;
        }

        std::string typeName = typeid(*component).name();
        m_components[typeName] = std::unique_ptr<Component>(component);

        VOXELCRAFT_TRACE("Component '{}' added to entity '{}' (ID: {})",
                        typeName, m_name, m_id);
    }

    void Entity::RemoveComponentInternal(Component* component) {
        if (!component) {
            return;
        }

        std::string typeName = typeid(*component).name();
        auto it = m_components.find(typeName);
        if (it != m_components.end()) {
            m_components.erase(it);
            VOXELCRAFT_TRACE("Component '{}' removed from entity '{}' (ID: {})",
                            typeName, m_name, m_id);
        }
    }

    // Template implementations

    template<typename T, typename... Args>
    T* Entity::AddComponent(Args&&... args) {
        // Check if component already exists
        if (HasComponent<T>()) {
            VOXELCRAFT_WARNING("Entity '{}' already has component '{}'",
                              m_name, typeid(T).name());
            return GetComponent<T>();
        }

        // Create component
        T* component = new T(this, std::forward<Args>(args)...);
        AddComponentInternal(component);

        return component;
    }

    template<typename T>
    T* Entity::GetComponent() {
        std::string typeName = typeid(T).name();
        auto it = m_components.find(typeName);
        if (it != m_components.end()) {
            return static_cast<T*>(it->second.get());
        }
        return nullptr;
    }

    template<typename T>
    const T* Entity::GetComponent() const {
        std::string typeName = typeid(T).name();
        auto it = m_components.find(typeName);
        if (it != m_components.end()) {
            return static_cast<const T*>(it->second.get());
        }
        return nullptr;
    }

    template<typename T>
    bool Entity::HasComponent() const {
        std::string typeName = typeid(T).name();
        return m_components.count(typeName) > 0;
    }

    template<typename T>
    bool Entity::RemoveComponent() {
        std::string typeName = typeid(T).name();
        auto it = m_components.find(typeName);
        if (it != m_components.end()) {
            m_components.erase(it);
            VOXELCRAFT_TRACE("Component '{}' removed from entity '{}' (ID: {})",
                            typeName, m_name, m_id);
            return true;
        }
        return false;
    }

} // namespace VoxelCraft
