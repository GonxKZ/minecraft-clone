#include "Component.hpp"
#include "Entity.hpp"
#include <sstream>
#include <atomic>

namespace VoxelCraft {

    // Variable estática para generar IDs únicos
    std::atomic<Component::ComponentID> Component::s_NextID{1};

    Component::Component()
        : m_ID(GenerateID())
        , m_Name("Component")
        , m_Owner(nullptr)
        , m_State(ComponentState::ENABLED)
    {
    }

    Component::~Component() {
        OnDestroy();
    }

    Component::Component(Component&& other) noexcept
        : m_ID(other.m_ID)
        , m_Name(std::move(other.m_Name))
        , m_Owner(other.m_Owner)
        , m_State(other.m_State)
    {
        other.m_State = ComponentState::DESTROYED;
        other.m_Owner = nullptr;
    }

    Component& Component::operator=(Component&& other) noexcept {
        if (this != &other) {
            OnDestroy();

            m_ID = other.m_ID;
            m_Name = std::move(other.m_Name);
            m_Owner = other.m_Owner;
            m_State = other.m_State;

            other.m_State = ComponentState::DESTROYED;
            other.m_Owner = nullptr;
        }

        return *this;
    }

    void Component::SetEnabled(bool enabled) {
        if (m_State == ComponentState::DESTROYED) {
            return;
        }

        ComponentState newState = enabled ? ComponentState::ENABLED : ComponentState::DISABLED;

        if (m_State != newState) {
            m_State = newState;

            if (enabled) {
                OnEnable();
            } else {
                OnDisable();
            }
        }
    }

    void Component::Destroy() {
        if (m_State != ComponentState::DESTROYED) {
            m_State = ComponentState::DESTROYED;
            OnDestroy();
        }
    }

    std::string Component::ToString() const {
        std::stringstream ss;
        ss << "Component[ID=" << m_ID << ", Name='" << m_Name << "', Type=" << GetType().name();

        switch (m_State) {
            case ComponentState::ENABLED: ss << ", State=ENABLED"; break;
            case ComponentState::DISABLED: ss << ", State=DISABLED"; break;
            case ComponentState::DESTROYED: ss << ", State=DESTROYED"; break;
        }

        ss << ", Owner=" << (m_Owner ? m_Owner->GetName() : "null") << "]";
        return ss.str();
    }

    Component::ComponentID Component::GenerateID() {
        return s_NextID.fetch_add(1, std::memory_order_relaxed);
    }

}