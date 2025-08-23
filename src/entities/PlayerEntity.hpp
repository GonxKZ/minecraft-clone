#pragma once

#include "Entity.hpp"
#include "TransformComponent.hpp"
#include "VelocityComponent.hpp"
#include "HealthComponent.hpp"
#include "ColliderComponent.hpp"
#include "RenderComponent.hpp"
#include <string>
#include <unordered_map>
#include <memory>

namespace VoxelCraft {

    /**
     * @brief Entidad del jugador principal
     */
    class PlayerEntity : public Entity {
    public:
        enum class PlayerState {
            IDLE,
            WALKING,
            RUNNING,
            JUMPING,
            FALLING,
            SWIMMING,
            FLYING,
            SLEEPING,
            DEAD
        };

        PlayerEntity(const std::string& playerName = "Player");
        ~PlayerEntity() override = default;

        // Getters de componentes principales
        TransformComponent* GetTransform() const { return m_Transform; }
        VelocityComponent* GetVelocity() const { return m_Velocity; }
        HealthComponent* GetHealth() const { return m_Health; }
        ColliderComponent* GetCollider() const { return m_Collider; }
        RenderComponent* GetRender() const { return m_Render; }

        // Gestión de estado del jugador
        PlayerState GetState() const { return m_State; }
        void SetState(PlayerState state);
        std::string GetPlayerName() const { return m_PlayerName; }

        // Movimiento del jugador
        void Move(const glm::vec3& direction, float speed);
        void Jump(float force = 5.0f);
        void Sprint(bool enable);
        void Crouch(bool enable);
        void Fly(bool enable);

        // Interacción
        void BreakBlock(const glm::vec3& blockPosition);
        void PlaceBlock(const glm::vec3& blockPosition, const std::string& blockType);
        void UseItem();

        // Inventario
        void AddItem(const std::string& itemType, int count);
        void RemoveItem(const std::string& itemType, int count);
        int GetItemCount(const std::string& itemType) const;
        void SelectHotbarSlot(int slot);
        int GetSelectedHotbarSlot() const { return m_SelectedHotbarSlot; }

        // Estadísticas
        void UpdateStatistics();
        int GetBlocksBroken() const { return m_BlocksBroken; }
        int GetBlocksPlaced() const { return m_BlocksPlaced; }
        float GetDistanceWalked() const { return m_DistanceWalked; }
        int GetMobsKilled() const { return m_MobsKilled; }

        // Callbacks de ciclo de vida
        void OnCreate() override;
        void Update(float deltaTime) override;
        void FixedUpdate(float fixedDeltaTime) override;

        // Serialización
        std::string ToString() const override;

    private:
        std::string m_PlayerName;
        PlayerState m_State{PlayerState::IDLE};

        // Componentes principales
        TransformComponent* m_Transform{nullptr};
        VelocityComponent* m_Velocity{nullptr};
        HealthComponent* m_Health{nullptr};
        ColliderComponent* m_Collider{nullptr};
        RenderComponent* m_Render{nullptr};

        // Estado del jugador
        bool m_IsSprinting{false};
        bool m_IsCrouching{false};
        bool m_IsFlying{false};
        bool m_IsOnGround{true};
        bool m_IsInWater{false};
        float m_JumpCooldown{0.0f};

        // Inventario
        std::unordered_map<std::string, int> m_Inventory;
        int m_SelectedHotbarSlot{0};
        std::array<std::string, 9> m_Hotbar;

        // Estadísticas
        int m_BlocksBroken{0};
        int m_BlocksPlaced{0};
        float m_DistanceWalked{0.0f};
        int m_MobsKilled{0};
        float m_TimePlayed{0.0f};

        // Constantes de movimiento
        static constexpr float WALK_SPEED = 4.3f;
        static constexpr float SPRINT_SPEED = 5.6f;
        static constexpr float JUMP_FORCE = 5.0f;
        static constexpr float FLY_SPEED = 10.0f;
        static constexpr float GRAVITY = -9.81f;
        static constexpr float MAX_FALL_SPEED = -53.0f;

        // Métodos auxiliares
        void InitializeComponents();
        void UpdateMovement(float deltaTime);
        void UpdatePhysics(float fixedDeltaTime);
        void UpdateAnimation();
        void HandleInput();
        void CheckGroundCollision();
        void CheckWaterCollision();

        // Callbacks de componentes
        void OnPlayerDamage(float damage, Entity* attacker = nullptr);
        void OnPlayerDeath();
        void OnPlayerRevive();
    };

    /**
     * @brief Componente específico del jugador
     */
    class PlayerComponent : public Component {
    public:
        std::string playerName;
        std::string skinName;
        int experienceLevel{0};
        float experiencePoints{0.0f};
        int foodLevel{20};
        float saturation{5.0f};

        // Habilidades
        bool canFly{false};
        bool canBuild{true};
        bool isCreativeMode{false};

        PlayerComponent(const std::string& name = "Player");
        ~PlayerComponent() override = default;

        // Gestión de experiencia
        void AddExperience(int amount);
        void AddExperiencePoints(float points);
        bool CanLevelUp() const;

        // Gestión de comida
        void EatFood(int foodValue, float saturationValue);
        void LoseHunger(int amount);
        bool IsHungry() const { return foodLevel <= 6; }
        bool IsStarving() const { return foodLevel <= 0; }

        // Modos de juego
        void SetCreativeMode(bool enable);
        void SetSurvivalMode(bool enable);

        void Update(float deltaTime) override;

        std::string ToString() const override;
    };

}
