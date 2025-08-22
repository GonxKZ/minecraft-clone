/**
 * @file Component.cpp
 * @brief VoxelCraft Entity System - Component Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file implements the Component base class for the VoxelCraft ECS system.
 */

#include "Component.hpp"
#include "Entity.hpp"
#include "Logger.hpp"

namespace VoxelCraft {

    // Static member initialization
    std::atomic<ComponentID> Component::s_nextId{1};

    Component::Component(Entity* owner, const std::string& name)
        : m_id(GenerateId())
        , m_name(name.empty() ? GetTypeName() : name)
        , m_owner(owner)
        , m_state(ComponentState::Enabled)
    {
        if (!owner) {
            VOXELCRAFT_ERROR("Component created with null owner");
        }

        VOXELCRAFT_TRACE("Component '{}' (ID: {}) created for entity '{}'",
                        m_name, m_id, owner ? owner->GetName() : "null");
    }

    Component::~Component() {
        VOXELCRAFT_TRACE("Component '{}' (ID: {}) destroyed", m_name, m_id);

        // Notify owner entity if it still exists
        if (m_owner) {
            // The entity will handle cleanup through RemoveComponentInternal
        }
    }

    ComponentID Component::GenerateId() {
        return s_nextId.fetch_add(1, std::memory_order_relaxed);
    }

} // namespace VoxelCraft
