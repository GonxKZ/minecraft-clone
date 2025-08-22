/**
 * @file System.hpp
 * @brief VoxelCraft Entity System - System Header
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines the System base class for the VoxelCraft ECS system.
 */

#ifndef VOXELCRAFT_ENTITIES_SYSTEM_HPP
#define VOXELCRAFT_ENTITIES_SYSTEM_HPP

#include <memory>
#include <string>
#include <vector>
#include <atomic>

namespace VoxelCraft {

    class EntityManager;
    class Entity;

    /**
     * @typedef SystemID
     * @brief Unique identifier for systems
     */
    using SystemID = uint64_t;

    /**
     * @enum SystemState
     * @brief Current state of a system
     */
    enum class SystemState {
        Enabled,        ///< System is enabled and active
        Disabled,       ///< System is disabled but still exists
        PendingDestroy  ///< System is marked for destruction
    };

    /**
     * @enum SystemPriority
     * @brief Execution priority for systems
     */
    enum class SystemPriority {
        Lowest = 0,     ///< Lowest priority (executed last)
        Low,            ///< Low priority
        Normal,         ///< Normal priority (default)
        High,           ///< High priority
        Highest         ///< Highest priority (executed first)
    };

    /**
     * @class System
     * @brief Base class for all entity systems
     *
     * Systems define the logic that operates on entities with specific components.
     * Each system processes entities that have the required components.
     */
    class System {
    public:
        /**
         * @brief Constructor
         * @param name System name
         * @param priority System execution priority
         */
        System(const std::string& name, SystemPriority priority = SystemPriority::Normal);

        /**
         * @brief Virtual destructor
         */
        virtual ~System();

        /**
         * @brief Deleted copy constructor
         */
        System(const System&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        System& operator=(const System&) = delete;

        /**
         * @brief Initialize the system
         * @return true if initialization successful, false otherwise
         */
        virtual bool Initialize() { return true; }

        /**
         * @brief Update the system
         * @param deltaTime Time elapsed since last update
         */
        virtual void Update(double deltaTime) {}

        /**
         * @brief Render the system
         */
        virtual void Render() {}

        /**
         * @brief Shutdown the system
         */
        virtual void Shutdown() {}

            /**
     * @brief Process a single entity
     * @param entity Entity to process
     * @param deltaTime Time elapsed since last update
     */
    virtual void ProcessEntity(Entity* entity, double deltaTime) {}

    /**
     * @brief Check if system should process an entity
     * @param entity Entity to check
     * @return true if should process, false otherwise
     */
    virtual bool ShouldProcessEntity(Entity* entity) const { return true; }

        /**
         * @brief Get system ID
         * @return System ID
         */
        SystemID GetID() const { return m_id; }

        /**
         * @brief Get system name
         * @return System name
         */
        const std::string& GetName() const { return m_name; }

        /**
         * @brief Set system name
         * @param name New system name
         */
        void SetName(const std::string& name) { m_name = name; }

        /**
         * @brief Get system type name
         * @return System type name
         */
        virtual std::string GetTypeName() const { return "System"; }

        /**
         * @brief Get system state
         * @return System state
         */
        SystemState GetState() const { return m_state; }

        /**
         * @brief Set system state
         * @param state New system state
         */
        void SetState(SystemState state) { m_state = state; }

        /**
         * @brief Get system priority
         * @return System priority
         */
        SystemPriority GetPriority() const { return m_priority; }

        /**
         * @brief Set system priority
         * @param priority New system priority
         */
        void SetPriority(SystemPriority priority) { m_priority = priority; }

        /**
         * @brief Check if system is enabled
         * @return true if enabled, false otherwise
         */
        bool IsEnabled() const { return m_state == SystemState::Enabled; }

        /**
         * @brief Enable the system
         */
        void Enable() { m_state = SystemState::Enabled; }

        /**
         * @brief Disable the system
         */
        void Disable() { m_state = SystemState::Disabled; }

        /**
         * @brief Mark system for destruction
         */
        void Destroy() { m_state = SystemState::PendingDestroy; }

        /**
         * @brief Check if system is marked for destruction
         * @return true if pending destruction, false otherwise
         */
        bool IsPendingDestroy() const { return m_state == SystemState::PendingDestroy; }

        /**
         * @brief Set entity manager for this system
         * @param manager Entity manager
         */
        void SetEntityManager(EntityManager* manager) { m_entityManager = manager; }

        /**
         * @brief Get entity manager
         * @return Entity manager pointer
         */
        EntityManager* GetEntityManager() const { return m_entityManager; }

    protected:
        SystemID m_id;                    ///< Unique system ID
        std::string m_name;               ///< System name
        SystemState m_state;              ///< Current system state
        SystemPriority m_priority;        ///< System execution priority
        EntityManager* m_entityManager;   ///< Entity manager reference

        /**
         * @brief Generate unique system ID
         * @return Unique ID
         */
        static SystemID GenerateId();

        static std::atomic<SystemID> s_nextId; ///< Next ID counter
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_ENTITIES_SYSTEM_HPP