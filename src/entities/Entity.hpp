#pragma once

#include <memory>
#include <unordered_map>
#include <typeindex>
#include <string>
#include <vector>
#include <functional>
#include <chrono>
#include <mutex>
#include <shared_mutex>

namespace VoxelCraft {

    class Component;
    class EntityManager;

    /**
     * @brief Clase base para todas las entidades del juego
     *
     * Representa una entidad en el ECS (Entity Component System) con capacidades
     * para agregar, remover y gestionar componentes dinámicamente.
     */
    class Entity {
    public:
        using EntityID = uint64_t;
        using ComponentMap = std::unordered_map<std::type_index, std::unique_ptr<Component>>;

        enum class EntityState {
            ACTIVE,
            INACTIVE,
            DESTROYED,
            PENDING_DESTROY
        };

        Entity(EntityID id, const std::string& name, EntityManager* manager = nullptr);
        virtual ~Entity();

        // Eliminamos la copia
        Entity(const Entity&) = delete;
        Entity& operator=(const Entity&) = delete;

        // Permitimos el movimiento
        Entity(Entity&& other) noexcept;
        Entity& operator=(Entity&& other) noexcept;

        // Getters básicos
        EntityID GetID() const { return m_ID; }
        const std::string& GetName() const { return m_Name; }
        EntityState GetState() const { return m_State; }
        bool IsActive() const { return m_State == EntityState::ACTIVE; }
        bool IsDestroyed() const { return m_State == EntityState::DESTROYED; }

        // Gestión de estado
        void SetActive(bool active);
        void Destroy();
        void SetName(const std::string& name) { m_Name = name; }

        // Gestión de componentes
        template<typename T, typename... Args>
        T* AddComponent(Args&&... args);

        template<typename T>
        T* GetComponent() const;

        template<typename T>
        bool HasComponent() const;

        template<typename T>
        void RemoveComponent();

        // Iteración de componentes
        template<typename Func>
        void ForEachComponent(Func&& func);

        // Gestión de EntityManager
        EntityManager* GetManager() const { return m_Manager; }
        void SetManager(EntityManager* manager) { m_Manager = manager; }

        // Métodos de actualización
        virtual void Update(float deltaTime);
        virtual void FixedUpdate(float fixedDeltaTime);
        virtual void LateUpdate(float deltaTime);

        // Callbacks de ciclo de vida
        virtual void OnCreate() {}
        virtual void OnDestroy() {}
        virtual void OnActivate() {}
        virtual void OnDeactivate() {}

        // Utilidades
        std::string ToString() const;
        size_t GetComponentCount() const { return m_Components.size(); }

    protected:
        EntityID m_ID;
        std::string m_Name;
        EntityState m_State;
        EntityManager* m_Manager;
        ComponentMap m_Components;
        mutable std::shared_mutex m_ComponentMutex;

        // Métodos auxiliares
        void Initialize();
        void Cleanup();
    };

    // Template implementations
    template<typename T, typename... Args>
    T* Entity::AddComponent(Args&&... args) {
        std::unique_lock lock(m_ComponentMutex);

        auto typeIndex = std::type_index(typeid(T));

        // Verificar si ya existe
        if (m_Components.find(typeIndex) != m_Components.end()) {
            return static_cast<T*>(m_Components[typeIndex].get());
        }

        // Crear el componente
        auto component = std::make_unique<T>(std::forward<Args>(args)...);
        auto* rawPtr = component.get();

        // Configurar el componente
        component->SetOwner(this);
        component->OnAttach();

        // Agregar al mapa
        m_Components[typeIndex] = std::move(component);

        return rawPtr;
    }

    template<typename T>
    T* Entity::GetComponent() const {
        std::shared_lock lock(m_ComponentMutex);

        auto typeIndex = std::type_index(typeid(T));
        auto it = m_Components.find(typeIndex);

        if (it != m_Components.end()) {
            return static_cast<T*>(it->second.get());
        }

        return nullptr;
    }

    template<typename T>
    bool Entity::HasComponent() const {
        std::shared_lock lock(m_ComponentMutex);
        return m_Components.find(std::type_index(typeid(T))) != m_Components.end();
    }

    template<typename T>
    void Entity::RemoveComponent() {
        std::unique_lock lock(m_ComponentMutex);

        auto typeIndex = std::type_index(typeid(T));
        auto it = m_Components.find(typeIndex);

        if (it != m_Components.end()) {
            it->second->OnDetach();
            m_Components.erase(it);
        }
    }

    template<typename Func>
    void Entity::ForEachComponent(Func&& func) {
        std::shared_lock lock(m_ComponentMutex);

        for (auto& pair : m_Components) {
            func(pair.second.get());
        }
    }

}