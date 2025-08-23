#pragma once

#include <memory>
#include <string>
#include <typeinfo>
#include <chrono>

namespace VoxelCraft {

    class Entity;

    /**
     * @brief Clase base para todos los componentes del ECS
     */
    class Component {
    public:
        using ComponentID = uint32_t;

        enum class ComponentState {
            ENABLED,
            DISABLED,
            DESTROYED
        };

        Component();
        virtual ~Component();

        // Getters básicos
        ComponentID GetID() const { return m_ID; }
        const std::string& GetName() const { return m_Name; }
        Entity* GetOwner() const { return m_Owner; }
        ComponentState GetState() const { return m_State; }
        bool IsEnabled() const { return m_State == ComponentState::ENABLED; }

        // Gestión de estado
        void SetEnabled(bool enabled);
        void Destroy();
        void SetName(const std::string& name) { m_Name = name; }

        // Gestión de owner
        void SetOwner(Entity* owner) { m_Owner = owner; }

        // Métodos de actualización
        virtual void Update(float deltaTime) {}
        virtual void FixedUpdate(float fixedDeltaTime) {}
        virtual void LateUpdate(float deltaTime) {}

        // Callbacks de ciclo de vida
        virtual void OnAttach() {}
        virtual void OnDetach() {}
        virtual void OnEnable() {}
        virtual void OnDisable() {}
        virtual void OnDestroy() {}

        // Utilidades
        std::string ToString() const;
        virtual std::type_index GetType() const { return typeid(*this); }

    protected:
        ComponentID m_ID;
        std::string m_Name;
        Entity* m_Owner;
        ComponentState m_State;

        static ComponentID GenerateID();
        static ComponentID s_NextID;
    };

}