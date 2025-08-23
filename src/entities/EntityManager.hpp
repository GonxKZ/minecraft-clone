#pragma once

#include <memory>
#include <unordered_map>
#include <vector>
#include <queue>
#include <typeindex>
#include <functional>
#include <shared_mutex>
#include <atomic>
#include <chrono>

namespace VoxelCraft {

    class Entity;
    class Component;
    class System;

    /**
     * @brief Gestor principal del sistema ECS
     *
     * Maneja la creación, destrucción y gestión de todas las entidades,
     * componentes y sistemas del juego.
     */
    class EntityManager {
    public:
        using EntityID = Entity::EntityID;
        using EntityPtr = std::unique_ptr<Entity>;
        using EntityMap = std::unordered_map<EntityID, EntityPtr>;
        using EntityVector = std::vector<Entity*>;
        using ComponentMap = std::unordered_map<std::type_index, std::vector<Component*>>;
        using SystemPtr = std::unique_ptr<System>;
        using SystemVector = std::vector<SystemPtr>;

        static EntityManager& GetInstance();

        // Creación y destrucción de entidades
        Entity* CreateEntity(const std::string& name = "");
        Entity* GetEntity(EntityID id) const;
        bool DestroyEntity(EntityID id);
        void DestroyAllEntities();
        size_t GetEntityCount() const;

        // Gestión de entidades por estado
        const EntityVector& GetActiveEntities() const { return m_ActiveEntities; }
        const EntityVector& GetInactiveEntities() const { return m_InactiveEntities; }
        const EntityVector& GetPendingDestroyEntities() const { return m_PendingDestroyEntities; }

        // Gestión de componentes
        template<typename T>
        std::vector<T*> GetComponentsOfType();

        template<typename T>
        size_t GetComponentCount();

        // Gestión de sistemas
        template<typename T, typename... Args>
        T* AddSystem(Args&&... args);

        template<typename T>
        T* GetSystem();

        template<typename T>
        bool HasSystem() const;

        template<typename T>
        void RemoveSystem();

        // Actualización
        void Update(float deltaTime);
        void FixedUpdate(float fixedDeltaTime);
        void LateUpdate(float deltaTime);

        // Limpieza
        void ProcessPendingDestroys();
        void CleanupDestroyedEntities();

        // Utilidades
        std::string ToString() const;
        void LogStatistics() const;

    private:
        EntityManager() = default;
        ~EntityManager() = default;

        // Eliminamos copia y asignación
        EntityManager(const EntityManager&) = delete;
        EntityManager& operator=(const EntityManager&) = delete;

        // Generación de IDs únicos
        static EntityID GenerateEntityID();

        // Gestión interna
        void AddEntityToActive(Entity* entity);
        void RemoveEntityFromActive(Entity* entity);
        void AddEntityToInactive(Entity* entity);
        void RemoveEntityFromInactive(Entity* entity);
        void AddEntityToPendingDestroy(Entity* entity);
        void RemoveEntityFromPendingDestroy(Entity* entity);

        // Component registration
        void RegisterComponent(Component* component);
        void UnregisterComponent(Component* component);

        // System updates
        void UpdateSystems(float deltaTime);
        void FixedUpdateSystems(float fixedDeltaTime);
        void LateUpdateSystems(float deltaTime);

        // Member variables
        EntityMap m_Entities;
        EntityVector m_ActiveEntities;
        EntityVector m_InactiveEntities;
        EntityVector m_PendingDestroyEntities;
        ComponentMap m_ComponentsByType;
        SystemVector m_Systems;

        // Thread safety
        mutable std::shared_mutex m_EntityMutex;
        mutable std::shared_mutex m_ComponentMutex;
        mutable std::shared_mutex m_SystemMutex;

        // ID generation
        static std::atomic<EntityID> s_NextEntityID;

        // Statistics
        struct Statistics {
            size_t totalEntitiesCreated = 0;
            size_t totalEntitiesDestroyed = 0;
            size_t totalComponentsCreated = 0;
            size_t totalComponentsDestroyed = 0;
            std::chrono::steady_clock::time_point startTime;
        } m_Stats;

        friend class Entity;
    };

    // Template implementations
    template<typename T>
    std::vector<T*> EntityManager::GetComponentsOfType() {
        std::shared_lock lock(m_ComponentMutex);
        std::vector<T*> result;

        auto typeIndex = std::type_index(typeid(T));
        auto it = m_ComponentsByType.find(typeIndex);

        if (it != m_ComponentsByType.end()) {
            for (Component* component : it->second) {
                if (component->IsEnabled()) {
                    T* casted = dynamic_cast<T*>(component);
                    if (casted) {
                        result.push_back(casted);
                    }
                }
            }
        }

        return result;
    }

    template<typename T>
    size_t EntityManager::GetComponentCount() {
        std::shared_lock lock(m_ComponentMutex);

        auto typeIndex = std::type_index(typeid(T));
        auto it = m_ComponentsByType.find(typeIndex);

        return (it != m_ComponentsByType.end()) ? it->second.size() : 0;
    }

    template<typename T, typename... Args>
    T* EntityManager::AddSystem(Args&&... args) {
        std::unique_lock lock(m_SystemMutex);

        // Verificar si ya existe
        for (const auto& system : m_Systems) {
            if (dynamic_cast<T*>(system.get())) {
                return static_cast<T*>(system.get());
            }
        }

        // Crear el sistema
        auto system = std::make_unique<T>(std::forward<Args>(args)...);
        auto* rawPtr = system.get();

        // Inicializar el sistema
        system->SetManager(this);
        system->OnInit();

        // Agregar al vector
        m_Systems.push_back(std::move(system));

        return rawPtr;
    }

    template<typename T>
    T* EntityManager::GetSystem() {
        std::shared_lock lock(m_SystemMutex);

        for (const auto& system : m_Systems) {
            T* casted = dynamic_cast<T*>(system.get());
            if (casted) {
                return casted;
            }
        }

        return nullptr;
    }

    template<typename T>
    bool EntityManager::HasSystem() const {
        std::shared_lock lock(m_SystemMutex);

        for (const auto& system : m_Systems) {
            if (dynamic_cast<T*>(system.get())) {
                return true;
            }
        }

        return false;
    }

    template<typename T>
    void EntityManager::RemoveSystem() {
        std::unique_lock lock(m_SystemMutex);

        for (auto it = m_Systems.begin(); it != m_Systems.end(); ++it) {
            if (dynamic_cast<T*>(it->get())) {
                (*it)->OnDestroy();
                m_Systems.erase(it);
                break;
            }
        }
    }

}