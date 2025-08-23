#include "Entity.hpp"
#include "Component.hpp"
#include <algorithm>
#include <sstream>

namespace VoxelCraft {

    Entity::Entity(EntityID id, const std::string& name, EntityManager* manager)
        : m_ID(id)
        , m_Name(name)
        , m_State(EntityState::ACTIVE)
        , m_Manager(manager)
    {
        Initialize();
    }

    Entity::~Entity() {
        Cleanup();
    }

    Entity::Entity(Entity&& other) noexcept
        : m_ID(other.m_ID)
        , m_Name(std::move(other.m_Name))
        , m_State(other.m_State)
        , m_Manager(other.m_Manager)
        , m_Components(std::move(other.m_Components))
    {
        other.m_State = EntityState::DESTROYED;
        other.m_Manager = nullptr;

        // Actualizar el owner de todos los componentes
        for (auto& pair : m_Components) {
            pair.second->SetOwner(this);
        }
    }

    Entity& Entity::operator=(Entity&& other) noexcept {
        if (this != &other) {
            Cleanup();

            m_ID = other.m_ID;
            m_Name = std::move(other.m_Name);
            m_State = other.m_State;
            m_Manager = other.m_Manager;
            m_Components = std::move(other.m_Components);

            other.m_State = EntityState::DESTROYED;
            other.m_Manager = nullptr;

            // Actualizar el owner de todos los componentes
            for (auto& pair : m_Components) {
                pair.second->SetOwner(this);
            }
        }

        return *this;
    }

    void Entity::SetActive(bool active) {
        if (m_State == EntityState::DESTROYED) {
            return;
        }

        EntityState newState = active ? EntityState::ACTIVE : EntityState::INACTIVE;

        if (m_State != newState) {
            m_State = newState;

            if (active) {
                OnActivate();
            } else {
                OnDeactivate();
            }
        }
    }

    void Entity::Destroy() {
        if (m_State != EntityState::DESTROYED) {
            m_State = EntityState::PENDING_DESTROY;
            OnDestroy();
        }
    }

    void Entity::Update(float deltaTime) {
        if (m_State != EntityState::ACTIVE) {
            return;
        }

        ForEachComponent([deltaTime](Component* component) {
            if (component->IsEnabled()) {
                component->Update(deltaTime);
            }
        });
    }

    void Entity::FixedUpdate(float fixedDeltaTime) {
        if (m_State != EntityState::ACTIVE) {
            return;
        }

        ForEachComponent([fixedDeltaTime](Component* component) {
            if (component->IsEnabled()) {
                component->FixedUpdate(fixedDeltaTime);
            }
        });
    }

    void Entity::LateUpdate(float deltaTime) {
        if (m_State != EntityState::ACTIVE) {
            return;
        }

        ForEachComponent([deltaTime](Component* component) {
            if (component->IsEnabled()) {
                component->LateUpdate(deltaTime);
            }
        });
    }

    std::string Entity::ToString() const {
        std::stringstream ss;
        ss << "Entity[ID=" << m_ID << ", Name='" << m_Name << "', State=";

        switch (m_State) {
            case EntityState::ACTIVE: ss << "ACTIVE"; break;
            case EntityState::INACTIVE: ss << "INACTIVE"; break;
            case EntityState::DESTROYED: ss << "DESTROYED"; break;
            case EntityState::PENDING_DESTROY: ss << "PENDING_DESTROY"; break;
        }

        ss << ", Components=" << m_Components.size() << "]";
        return ss.str();
    }

    void Entity::Initialize() {
        OnCreate();
    }

    void Entity::Cleanup() {
        if (m_State != EntityState::DESTROYED) {
            m_State = EntityState::DESTROYED;

            // Desvincular todos los componentes
            ForEachComponent([](Component* component) {
                component->OnDetach();
            });

            m_Components.clear();
            OnDestroy();
        }
    }

}