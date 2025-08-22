/**
 * @file System.cpp
 * @brief VoxelCraft Entity System - System Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file implements the System base class for the VoxelCraft ECS system.
 */

#include "System.hpp"
#include "EntityManager.hpp"
#include "Entity.hpp"
#include "Logger.hpp"

namespace VoxelCraft {

    // Static member initialization
    std::atomic<SystemID> System::s_nextId{1};

    System::System(const std::string& name, SystemPriority priority)
        : m_id(GenerateId())
        , m_name(name)
        , m_state(SystemState::Enabled)
        , m_priority(priority)
        , m_entityManager(nullptr)
    {
        VOXELCRAFT_TRACE("System '{}' (ID: {}) created", name, m_id);
    }

    System::~System() {
        VOXELCRAFT_TRACE("System '{}' (ID: {}) destroyed", m_name, m_id);
    }

    SystemID System::GenerateId() {
        return s_nextId.fetch_add(1, std::memory_order_relaxed);
    }

} // namespace VoxelCraft
