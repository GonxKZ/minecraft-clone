#include "PlayerEntity.hpp"
#include <algorithm>
#include <cmath>

namespace VoxelCraft {

    PlayerEntity::PlayerEntity(const std::string& playerName)
        : Entity(0, "PlayerEntity", nullptr), m_PlayerName(playerName)
    {
        InitializeComponents();
    }

    void PlayerEntity::InitializeComponents() {
        // Crear componentes principales
        m_Transform = AddComponent<TransformComponent>();
        m_Velocity = AddComponent<VelocityComponent>();
        m_Health = AddComponent<HealthComponent>(20.0f); // 10 corazones
        m_Collider = AddComponent<ColliderComponent>(ColliderComponent::ColliderType::CAPSULE);
        m_Render = AddComponent<RenderComponent>("player_model", "player_material", "steve_skin");

        // Configurar colisionador del jugador (1.8 bloques de alto, 0.6 de ancho)
        m_Collider->SetAsCapsule(0.3f, 1.8f);
        m_Collider->isTrigger = false;

        // Configurar componente de jugador
        auto playerComponent = AddComponent<PlayerComponent>(m_PlayerName);
        playerComponent->skinName = "default";

        // Configurar velocidad del jugador
        m_Velocity->maxSpeed = WALK_SPEED;
        m_Velocity->acceleration = 50.0f;
        m_Velocity->friction = 0.91f;
        m_Velocity->mass = 70.0f; // Peso promedio de una persona

        VOXELCRAFT_LOG_INFO("Player entity '{}' initialized with {} components", m_PlayerName, GetComponentCount());
    }

    void PlayerEntity::SetState(PlayerState state) {
        if (m_State != state) {
            PlayerState oldState = m_State;
            m_State = state;

            // Callbacks de cambio de estado
            switch (state) {
                case PlayerState::WALKING:
                    m_Velocity->maxSpeed = m_IsSprinting ? SPRINT_SPEED : WALK_SPEED;
                    break;
                case PlayerState::RUNNING:
                    m_Velocity->maxSpeed = SPRINT_SPEED;
                    break;
                case PlayerState::JUMPING:
                    if (m_IsOnGround) {
                        Jump();
                    }
                    break;
                case PlayerState::FALLING:
                    break;
                case PlayerState::SWIMMING:
                    m_Velocity->maxSpeed = WALK_SPEED * 0.5f;
                    break;
                case PlayerState::FLYING:
                    m_Velocity->maxSpeed = FLY_SPEED;
                    break;
                case PlayerState::DEAD:
                    m_Velocity->linearVelocity = glm::vec3(0.0f);
                    break;
                default:
                    break;
            }

            VOXELCRAFT_LOG_DEBUG("Player state changed: {} -> {}",
                               static_cast<int>(oldState), static_cast<int>(state));
        }
    }

    void PlayerEntity::Move(const glm::vec3& direction, float speed) {
        if (m_State == PlayerState::DEAD) {
            return;
        }

        // Normalizar dirección
        glm::vec3 moveDir = glm::normalize(direction);
        float moveSpeed = speed > 0.0f ? speed : m_Velocity->maxSpeed;

        // Aplicar movimiento basado en el estado
        if (m_IsFlying) {
            m_Velocity->linearVelocity = moveDir * moveSpeed;
        } else {
            // Movimiento en tierra
            glm::vec3 horizontalVelocity = glm::vec3(moveDir.x, 0.0f, moveDir.z) * moveSpeed;
            m_Velocity->linearVelocity.x = horizontalVelocity.x;
            m_Velocity->linearVelocity.z = horizontalVelocity.z;
        }

        // Actualizar estado basado en el movimiento
        if (glm::length(direction) > 0.1f) {
            if (m_IsFlying) {
                SetState(PlayerState::FLYING);
            } else if (m_IsInWater) {
                SetState(PlayerState::SWIMMING);
            } else if (m_IsSprinting) {
                SetState(PlayerState::RUNNING);
            } else {
                SetState(PlayerState::WALKING);
            }
        } else {
            SetState(PlayerState::IDLE);
        }
    }

    void PlayerEntity::Jump(float force) {
        if (m_State == PlayerState::DEAD || m_JumpCooldown > 0.0f) {
            return;
        }

        if (m_IsOnGround || m_IsInWater || m_IsFlying) {
            m_Velocity->linearVelocity.y = force;
            m_IsOnGround = false;
            m_JumpCooldown = 0.5f; // 500ms cooldown

            SetState(PlayerState::JUMPING);

            // Efecto de sonido de salto
            // AudioManager::PlaySound("player_jump", GetTransform()->position);
        }
    }

    void PlayerEntity::Sprint(bool enable) {
        if (m_IsSprinting != enable) {
            m_IsSprinting = enable;

            if (enable) {
                m_Velocity->maxSpeed = SPRINT_SPEED;
            } else {
                m_Velocity->maxSpeed = WALK_SPEED;
            }

            // Solo actualizar estado si estamos moviéndonos
            if (glm::length(m_Velocity->linearVelocity) > 0.1f) {
                SetState(m_IsSprinting ? PlayerState::RUNNING : PlayerState::WALKING);
            }
        }
    }

    void PlayerEntity::Crouch(bool enable) {
        m_IsCrouching = enable;

        if (enable) {
            m_Velocity->maxSpeed = WALK_SPEED * 0.3f; // Movimiento lento agachado
            // Reducir altura del colisionador
            m_Collider->dimensions.capsule.height = 1.5f;
        } else {
            m_Velocity->maxSpeed = m_IsSprinting ? SPRINT_SPEED : WALK_SPEED;
            m_Collider->dimensions.capsule.height = 1.8f;
        }
    }

    void PlayerEntity::Fly(bool enable) {
        if (m_IsFlying != enable) {
            m_IsFlying = enable;

            if (enable) {
                // Desactivar gravedad cuando volamos
                m_Velocity->linearVelocity.y = 0.0f;
                SetState(PlayerState::FLYING);
            } else {
                SetState(PlayerState::IDLE);
            }
        }
    }

    void PlayerEntity::BreakBlock(const glm::vec3& blockPosition) {
        // Lógica para romper bloque
        m_BlocksBroken++;

        // Efectos de partículas
        // ParticleSystem::EmitBlockBreakParticles(blockPosition);

        // Efecto de sonido
        // AudioManager::PlaySound("block_break", blockPosition);

        VOXELCRAFT_LOG_DEBUG("Player {} broke block at ({}, {}, {})",
                           m_PlayerName, blockPosition.x, blockPosition.y, blockPosition.z);
    }

    void PlayerEntity::PlaceBlock(const glm::vec3& blockPosition, const std::string& blockType) {
        if (GetItemCount(blockType) > 0) {
            // Lógica para colocar bloque
            m_BlocksPlaced++;
            RemoveItem(blockType, 1);

            // Efecto de sonido
            // AudioManager::PlaySound("block_place", blockPosition);

            VOXELCRAFT_LOG_DEBUG("Player {} placed {} block at ({}, {}, {})",
                               m_PlayerName, blockType, blockPosition.x, blockPosition.y, blockPosition.z);
        }
    }

    void PlayerEntity::UseItem() {
        // Lógica para usar el item seleccionado
        auto selectedItem = m_Hotbar[m_SelectedHotbarSlot];
        if (!selectedItem.empty()) {
            VOXELCRAFT_LOG_DEBUG("Player {} used item: {}", m_PlayerName, selectedItem);
        }
    }

    void PlayerEntity::AddItem(const std::string& itemType, int count) {
        m_Inventory[itemType] += count;
        VOXELCRAFT_LOG_DEBUG("Player {} added {}x {}", m_PlayerName, count, itemType);
    }

    void PlayerEntity::RemoveItem(const std::string& itemType, int count) {
        if (m_Inventory.count(itemType)) {
            m_Inventory[itemType] = std::max(0, m_Inventory[itemType] - count);
            if (m_Inventory[itemType] == 0) {
                m_Inventory.erase(itemType);
            }
        }
    }

    int PlayerEntity::GetItemCount(const std::string& itemType) const {
        auto it = m_Inventory.find(itemType);
        return (it != m_Inventory.end()) ? it->second : 0;
    }

    void PlayerEntity::SelectHotbarSlot(int slot) {
        if (slot >= 0 && slot < 9) {
            m_SelectedHotbarSlot = slot;
            VOXELCRAFT_LOG_DEBUG("Player {} selected hotbar slot {}", m_PlayerName, slot);
        }
    }

    void PlayerEntity::UpdateStatistics() {
        // Actualizar estadísticas de juego
        if (m_State == PlayerState::WALKING || m_State == PlayerState::RUNNING) {
            float distance = glm::length(m_Velocity->linearVelocity) * 0.016f; // Aproximado
            m_DistanceWalked += distance;
        }

        m_TimePlayed += 0.016f; // 16ms por frame
    }

    void PlayerEntity::OnCreate() {
        VOXELCRAFT_LOG_INFO("Player entity '{}' created at position ({}, {}, {})",
                           m_PlayerName,
                           m_Transform->position.x,
                           m_Transform->position.y,
                           m_Transform->position.z);
    }

    void PlayerEntity::Update(float deltaTime) {
        if (m_State == PlayerState::DEAD) {
            return;
        }

        // Actualizar cooldowns
        if (m_JumpCooldown > 0.0f) {
            m_JumpCooldown -= deltaTime;
        }

        // Actualizar estadísticas
        UpdateStatistics();

        // Actualizar animación
        UpdateAnimation();

        // Manejar entrada del jugador
        HandleInput();
    }

    void PlayerEntity::FixedUpdate(float fixedDeltaTime) {
        if (m_State == PlayerState::DEAD) {
            return;
        }

        // Actualizar física
        UpdatePhysics(fixedDeltaTime);

        // Verificar colisiones
        CheckGroundCollision();
        CheckWaterCollision();
    }

    void PlayerEntity::UpdateMovement(float deltaTime) {
        // Aplicar fricción
        if (m_IsOnGround && !m_IsFlying) {
            m_Velocity->linearVelocity.x *= m_Velocity->friction;
            m_Velocity->linearVelocity.z *= m_Velocity->friction;
        }

        // Aplicar gravedad si no estamos volando
        if (!m_IsFlying && !m_IsOnGround) {
            m_Velocity->linearVelocity.y += GRAVITY * deltaTime;

            // Limitar velocidad de caída
            if (m_Velocity->linearVelocity.y < MAX_FALL_SPEED) {
                m_Velocity->linearVelocity.y = MAX_FALL_SPEED;
            }
        }

        // Actualizar posición
        m_Transform->position += m_Velocity->linearVelocity * deltaTime;
    }

    void PlayerEntity::UpdatePhysics(float fixedDeltaTime) {
        // Integrar movimiento
        UpdateMovement(fixedDeltaTime);

        // Aplicar restricciones de velocidad
        m_Velocity->ClampSpeed();
    }

    void PlayerEntity::UpdateAnimation() {
        // Actualizar animación basada en el estado
        // Esto se integraría con el sistema de animación
    }

    void PlayerEntity::HandleInput() {
        // Esta función se conectaría con el InputManager
        // Por ahora es solo un placeholder
    }

    void PlayerEntity::CheckGroundCollision() {
        // Verificar si el jugador está en el suelo
        // Esta sería una verificación de colisión con el mundo
        float groundCheckDistance = 0.1f;
        // Implementar raycast hacia abajo

        // Por ahora, asumir que siempre está en el suelo si la velocidad Y es 0
        if (std::abs(m_Velocity->linearVelocity.y) < 0.01f) {
            m_IsOnGround = true;
            if (m_State == PlayerState::JUMPING || m_State == PlayerState::FALLING) {
                SetState(PlayerState::IDLE);
            }
        } else {
            m_IsOnGround = false;
            if (m_Velocity->linearVelocity.y > 0.1f) {
                SetState(PlayerState::JUMPING);
            } else if (m_Velocity->linearVelocity.y < -0.1f) {
                SetState(PlayerState::FALLING);
            }
        }
    }

    void PlayerEntity::CheckWaterCollision() {
        // Verificar si el jugador está en agua
        // Implementar verificación de colisión con bloques de agua

        // Por ahora, es solo un placeholder
        m_IsInWater = false;
    }

    void PlayerEntity::OnPlayerDamage(float damage, Entity* attacker) {
        VOXELCRAFT_LOG_DEBUG("Player {} took {} damage", m_PlayerName, damage);

        // Efectos visuales de daño
        // ParticleSystem::EmitDamageParticles(m_Transform->position);

        // Efecto de sonido de daño
        // AudioManager::PlaySound("player_hurt", m_Transform->position);
    }

    void PlayerEntity::OnPlayerDeath() {
        VOXELCRAFT_LOG_INFO("Player {} died", m_PlayerName);

        SetState(PlayerState::DEAD);

        // Efectos de muerte
        // ParticleSystem::EmitDeathParticles(m_Transform->position);
        // AudioManager::PlaySound("player_death", m_Transform->position);
    }

    void PlayerEntity::OnPlayerRevive() {
        VOXELCRAFT_LOG_INFO("Player {} revived", m_PlayerName);

        SetState(PlayerState::IDLE);
        m_Health->currentHealth = m_Health->maxHealth;
    }

    std::string PlayerEntity::ToString() const {
        std::stringstream ss;
        ss << "PlayerEntity[Name='" << m_PlayerName << "', State=";
        switch (m_State) {
            case PlayerState::IDLE: ss << "IDLE"; break;
            case PlayerState::WALKING: ss << "WALKING"; break;
            case PlayerState::RUNNING: ss << "RUNNING"; break;
            case PlayerState::JUMPING: ss << "JUMPING"; break;
            case PlayerState::FALLING: ss << "FALLING"; break;
            case PlayerState::SWIMMING: ss << "SWIMMING"; break;
            case PlayerState::FLYING: ss << "FLYING"; break;
            case PlayerState::DEAD: ss << "DEAD"; break;
            default: ss << "UNKNOWN"; break;
        }
        ss << ", Health=" << m_Health->currentHealth << "/" << m_Health->maxHealth;
        ss << ", Position=(" << m_Transform->position.x << "," << m_Transform->position.y << "," << m_Transform->position.z << ")]";
        return ss.str();
    }

    // Implementación de PlayerComponent
    PlayerComponent::PlayerComponent(const std::string& name) : playerName(name) {
        m_Name = "PlayerComponent";
    }

    void PlayerComponent::AddExperience(int amount) {
        experiencePoints += amount;
        while (CanLevelUp()) {
            experienceLevel++;
            // Reset experience points for next level
            experiencePoints -= GetExperienceForNextLevel();
            VOXELCRAFT_LOG_DEBUG("Player {} leveled up to level {}", playerName, experienceLevel);
        }
    }

    void PlayerComponent::AddExperiencePoints(float points) {
        experiencePoints += points;
    }

    bool PlayerComponent::CanLevelUp() const {
        return experiencePoints >= GetExperienceForNextLevel();
    }

    int PlayerComponent::GetExperienceForNextLevel() const {
        // Minecraft experience formula: 7 + (level * 7) / 2
        return 7 + (experienceLevel * 7) / 2;
    }

    void PlayerComponent::EatFood(int foodValue, float saturationValue) {
        foodLevel = std::min(20, foodLevel + foodValue);
        saturation = std::min(20.0f, saturation + saturationValue);
    }

    void PlayerComponent::LoseHunger(int amount) {
        if (saturation > 0.0f) {
            saturation = std::max(0.0f, saturation - amount);
        } else {
            foodLevel = std::max(0, foodLevel - amount);
        }
    }

    void PlayerComponent::SetCreativeMode(bool enable) {
        isCreativeMode = enable;
        canFly = enable;
        canBuild = true;
    }

    void PlayerComponent::SetSurvivalMode(bool enable) {
        isCreativeMode = !enable;
        canFly = false;
        canBuild = true;
    }

    void PlayerComponent::Update(float deltaTime) {
        // Regeneración de salud basada en comida
        if (foodLevel >= 18 && GetOwner() && GetOwner()->GetComponent<HealthComponent>()) {
            auto health = GetOwner()->GetComponent<HealthComponent>();
            if (health->currentHealth < health->maxHealth) {
                // Regenerar 1 HP cada 4 segundos si foodLevel >= 18
                static float regenTimer = 0.0f;
                regenTimer += deltaTime;
                if (regenTimer >= 4.0f) {
                    health->Heal(1.0f);
                    LoseHunger(1);
                    regenTimer = 0.0f;
                }
            }
        }
    }

    std::string PlayerComponent::ToString() const {
        std::stringstream ss;
        ss << "PlayerComponent[Name='" << playerName << "', Level=" << experienceLevel
           << ", XP=" << experiencePoints << ", Food=" << foodLevel << ", Saturation=" << saturation
           << ", Mode=" << (isCreativeMode ? "Creative" : "Survival") << "]";
        return ss.str();
    }

}
