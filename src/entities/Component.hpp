/**
 * @file Component.hpp
 * @brief VoxelCraft Entity System - Component Header
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines the Component base class for the VoxelCraft ECS system.
 */

#ifndef VOXELCRAFT_ENTITIES_COMPONENT_HPP
#define VOXELCRAFT_ENTITIES_COMPONENT_HPP

#include <memory>
#include <string>
#include <atomic>

namespace VoxelCraft {

    class Entity;

    /**
     * @typedef ComponentID
     * @brief Unique identifier for components
     */
    using ComponentID = uint64_t;

    /**
     * @enum ComponentState
     * @brief Current state of a component
     */
    enum class ComponentState {
        Enabled,        ///< Component is enabled and active
        Disabled,       ///< Component is disabled but still exists
        PendingDestroy  ///< Component is marked for destruction
    };

    /**
     * @class Component
     * @brief Base class for all entity components
     *
     * Components define the behavior and properties of entities in the ECS system.
     * Each component type provides specific functionality to the entity it belongs to.
     */
    class Component {
    public:
        /**
         * @brief Constructor
         * @param owner Entity that owns this component
         * @param name Component name
         */
        Component(Entity* owner, const std::string& name = "");

        /**
         * @brief Virtual destructor
         */
        virtual ~Component();

        /**
         * @brief Deleted copy constructor
         */
        Component(const Component&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        Component& operator=(const Component&) = delete;

        /**
         * @brief Initialize the component
         * @return true if initialization successful, false otherwise
         */
        virtual bool Initialize() { return true; }

        /**
         * @brief Update the component
         * @param deltaTime Time elapsed since last update
         */
        virtual void Update(double deltaTime) {}

        /**
         * @brief Render the component
         */
        virtual void Render() {}

        /**
         * @brief Shutdown the component
         */
        virtual void Shutdown() {}

        /**
         * @brief Get component ID
         * @return Component ID
         */
        ComponentID GetID() const { return m_id; }

        /**
         * @brief Get component name
         * @return Component name
         */
        const std::string& GetName() const { return m_name; }

        /**
         * @brief Set component name
         * @param name New component name
         */
        void SetName(const std::string& name) { m_name = name; }

        /**
         * @brief Get component type name
         * @return Component type name
         */
        virtual std::string GetTypeName() const { return "Component"; }

        /**
         * @brief Get owner entity
         * @return Owner entity pointer
         */
        Entity* GetOwner() const { return m_owner; }

        /**
         * @brief Get component state
         * @return Component state
         */
        ComponentState GetState() const { return m_state; }

        /**
         * @brief Set component state
         * @param state New component state
         */
        void SetState(ComponentState state) { m_state = state; }

        /**
         * @brief Check if component is enabled
         * @return true if enabled, false otherwise
         */
        bool IsEnabled() const { return m_state == ComponentState::Enabled; }

        /**
         * @brief Enable the component
         */
        void Enable() { m_state = ComponentState::Enabled; }

        /**
         * @brief Disable the component
         */
        void Disable() { m_state = ComponentState::Disabled; }

        /**
         * @brief Mark component for destruction
         */
        void Destroy() { m_state = ComponentState::PendingDestroy; }

        /**
         * @brief Check if component is marked for destruction
         * @return true if pending destruction, false otherwise
         */
        bool IsPendingDestroy() const { return m_state == ComponentState::PendingDestroy; }

    protected:
        ComponentID m_id;                    ///< Unique component ID
        std::string m_name;                  ///< Component name
        Entity* m_owner;                     ///< Owner entity
        ComponentState m_state;              ///< Current component state

        /**
         * @brief Generate unique component ID
         * @return Unique ID
         */
        static ComponentID GenerateId();

        static std::atomic<ComponentID> s_nextId; ///< Next ID counter
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_ENTITIES_COMPONENT_HPP