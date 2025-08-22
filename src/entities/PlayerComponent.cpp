/**
 * @file PlayerComponent.cpp
 * @brief VoxelCraft ECS - Player Component Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#include "PlayerComponent.hpp"
#include "Entity.hpp"
#include "TransformComponent.hpp"
#include "PhysicsComponent.hpp"
#include "Logger.hpp"

#include <algorithm>

namespace VoxelCraft {

    PlayerComponent::PlayerComponent(Entity* owner)
        : Component(owner, "PlayerComponent")
        , m_playerName("Player")
        , m_playerType(PlayerType::Human)
        , m_state(PlayerState::Idle)
        , m_isGrounded(true)
        , m_isCrouching(false)
        , m_isFlying(false)
        , m_staminaRegenTimer(0.0f)
        , m_jumpTimer(0.0f)
        , m_aiTarget(0.0f)
        , m_aiUpdateTimer(0.0f)
    {
        VOXELCRAFT_TRACE("PlayerComponent created for entity '{}'",
                        owner ? owner->GetName() : "null");
    }

    PlayerComponent::PlayerComponent(Entity* owner, PlayerType playerType)
        : Component(owner, "PlayerComponent")
        , m_playerName("Player")
        , m_playerType(playerType)
        , m_state(PlayerState::Idle)
        , m_isGrounded(true)
        , m_isCrouching(false)
        , m_isFlying(false)
        , m_staminaRegenTimer(0.0f)
        , m_jumpTimer(0.0f)
        , m_aiTarget(0.0f)
        , m_aiUpdateTimer(0.0f)
    {
        VOXELCRAFT_TRACE("PlayerComponent created for entity '{}' with type {}",
                        owner ? owner->GetName() : "null", static_cast<int>(playerType));
    }

    PlayerComponent::PlayerComponent(Entity* owner, PlayerType playerType, const std::string& playerName)
        : Component(owner, "PlayerComponent")
        , m_playerName(playerName)
        , m_playerType(playerType)
        , m_state(PlayerState::Idle)
        , m_isGrounded(true)
        , m_isCrouching(false)
        , m_isFlying(false)
        , m_staminaRegenTimer(0.0f)
        , m_jumpTimer(0.0f)
        , m_aiTarget(0.0f)
        , m_aiUpdateTimer(0.0f)
    {
        VOXELCRAFT_TRACE("PlayerComponent created for entity '{}' with name '{}' and type {}",
                        owner ? owner->GetName() : "null", playerName, static_cast<int>(playerType));
    }

    PlayerComponent::~PlayerComponent() {
        VOXELCRAFT_TRACE("PlayerComponent destroyed for entity '{}'",
                        GetOwner() ? GetOwner()->GetName() : "null");
    }

    bool PlayerComponent::Initialize() {
        VOXELCRAFT_TRACE("PlayerComponent initialized for entity '{}'",
                        GetOwner() ? GetOwner()->GetName() : "null");

        // Initialize based on player type
        switch (m_playerType) {
            case PlayerType::Human:
                InitializeHuman();
                break;
            case PlayerType::AI:
                InitializeAI();
                break;
            case PlayerType::Remote:
                InitializeRemote();
                break;
        }

        return true;
    }

    void PlayerComponent::Update(double deltaTime) {
        float dt = static_cast<float>(deltaTime);

        // Update timers
        m_staminaRegenTimer = std::max(0.0f, m_staminaRegenTimer - dt);
        m_jumpTimer = std::max(0.0f, m_jumpTimer - dt);
        m_aiUpdateTimer += dt;

        // Update based on player type
        switch (m_playerType) {
            case PlayerType::Human:
                UpdateMovement(dt);
                break;
            case PlayerType::AI:
                UpdateAI(dt);
                break;
            case PlayerType::Remote:
                // Remote player updates come from network
                break;
        }

        UpdateState();

        // Regenerate stamina if not moving
        if (m_staminaRegenTimer <= 0.0f && m_stats.stamina < m_stats.maxStamina) {
            m_stats.stamina = std::min(m_stats.maxStamina, m_stats.stamina + 10.0f * dt);
        }

        // Update hunger
        if (m_stats.hunger > 0.0f) {
            m_stats.hunger = std::max(0.0f, m_stats.hunger - 2.0f * dt);
        }
    }

    void PlayerComponent::Render() {
        // Render player-specific debug information
        VOXELCRAFT_TRACE("Player debug render for entity '{}': State={}, Health={}/{}, Stamina={}/{}",
                        GetOwner() ? GetOwner()->GetName() : "null",
                        static_cast<int>(m_state),
                        m_stats.health, m_stats.maxHealth,
                        m_stats.stamina, m_stats.maxStamina);
    }

    void PlayerComponent::SetPlayerName(const std::string& name) {
        m_playerName = name;
        VOXELCRAFT_INFO("Player name changed to '{}' for entity '{}'",
                      name, GetOwner() ? GetOwner()->GetName() : "null");
    }

    void PlayerComponent::SetState(PlayerState state) {
        if (m_state != state) {
            PlayerState oldState = m_state;
            m_state = state;
            VOXELCRAFT_INFO("Player state changed from {} to {} for entity '{}'",
                          static_cast<int>(oldState), static_cast<int>(state),
                          GetOwner() ? GetOwner()->GetName() : "null");
        }
    }

    void PlayerComponent::ProcessInput(const PlayerInput& input) {
        if (m_playerType != PlayerType::Human || !IsAlive()) {
            return;
        }

        m_currentInput = input;

        // Handle input actions
        if (input.jumpPressed && CanMove() && m_isGrounded && m_jumpTimer <= 0.0f) {
            Jump();
        }

        if (input.crouchPressed != m_isCrouching) {
            Crouch(input.crouchPressed);
        }

        if (input.flyPressed && m_movement.canFly) {
            ToggleFly();
        }

        if (input.attackPressed) {
            Attack();
        }

        if (input.usePressed) {
            Use();
        }

        // Handle looking
        if (glm::length(input.lookInput) > 0.001f) {
            Look(input.lookInput.x, input.lookInput.y);
        }
    }

    void PlayerComponent::SetStats(const PlayerStats& stats) {
        m_stats = stats;

        // Ensure health doesn't exceed max
        m_stats.health = std::min(m_stats.health, m_stats.maxHealth);
        m_stats.stamina = std::min(m_stats.stamina, m_stats.maxStamina);
        m_stats.hunger = std::min(m_stats.hunger, m_stats.maxHunger);
    }

    bool PlayerComponent::ModifyHealth(float amount) {
        if (amount == 0.0f) {
            return false;
        }

        float oldHealth = m_stats.health;
        m_stats.health = std::max(0.0f, std::min(m_stats.maxHealth, m_stats.health + amount));

        if (m_stats.health != oldHealth) {
            VOXELCRAFT_INFO("Player '{}' health changed from {} to {}",
                          m_playerName, oldHealth, m_stats.health);

            if (m_stats.health <= 0.0f) {
                SetState(PlayerState::Dead);
            }

            return true;
        }

        return false;
    }

    bool PlayerComponent::ModifyStamina(float amount) {
        if (amount == 0.0f) {
            return false;
        }

        float oldStamina = m_stats.stamina;
        m_stats.stamina = std::max(0.0f, std::min(m_stats.maxStamina, m_stats.stamina + amount));

        if (m_stats.stamina != oldStamina) {
            m_staminaRegenTimer = 2.0f; // Reset regeneration timer
            return true;
        }

        return false;
    }

    bool PlayerComponent::AddExperience(int amount) {
        if (amount <= 0) {
            return false;
        }

        m_stats.experience += amount;

        // Simple level up formula: level * 100 XP required
        int xpForNextLevel = m_stats.level * 100;
        bool leveledUp = false;

        while (m_stats.experience >= xpForNextLevel) {
            m_stats.experience -= xpForNextLevel;
            m_stats.level++;
            leveledUp = true;
            xpForNextLevel = m_stats.level * 100;

            HandleLevelUp(m_stats.level);
        }

        return leveledUp;
    }

    void PlayerComponent::SetMovement(const PlayerMovement& movement) {
        m_movement = movement;

        // Update physics component if exists
        auto physics = GetOwner()->GetComponent<PhysicsComponent>();
        if (physics) {
            physics->SetMass(70.0f); // Average human mass
        }
    }

    bool PlayerComponent::CanMove() const {
        return IsAlive() && m_state != PlayerState::Respawning;
    }

    void PlayerComponent::Move(const glm::vec3& direction, double deltaTime) {
        if (!CanMove()) {
            return;
        }

        float dt = static_cast<float>(deltaTime);
        auto transform = GetOwner()->GetComponent<TransformComponent>();
        auto physics = GetOwner()->GetComponent<PhysicsComponent>();

        if (!transform) {
            return;
        }

        // Calculate movement speed
        float speed = m_movement.walkSpeed;
        if (m_currentInput.runPressed && m_stats.stamina > 10.0f) {
            speed = m_movement.runSpeed;
            ModifyStamina(-5.0f * dt); // Running costs stamina
        }

        // Calculate movement direction
        glm::vec3 moveDir = direction * speed * dt;

        if (physics && !m_isFlying) {
            // Use physics for movement
            physics->ApplyForce(moveDir / dt, glm::vec3(0.0f));
        } else {
            // Direct transform movement (flying or no physics)
            transform->Translate(moveDir);
        }

        // Update state based on movement
        float moveMagnitude = glm::length(direction);
        if (moveMagnitude > 0.1f) {
            if (m_currentInput.runPressed && m_stats.stamina > 10.0f) {
                SetState(PlayerState::Running);
            } else {
                SetState(PlayerState::Walking);
            }
        } else {
            SetState(PlayerState::Idle);
        }
    }

    void PlayerComponent::Jump() {
        if (!CanMove() || !m_isGrounded || m_jumpTimer > 0.0f) {
            return;
        }

        auto physics = GetOwner()->GetComponent<PhysicsComponent>();
        if (physics) {
            physics->ApplyImpulse(glm::vec3(0.0f, m_movement.jumpForce, 0.0f));
        }

        m_jumpTimer = 0.5f; // Jump cooldown
        SetState(PlayerState::Jumping);
        ModifyStamina(-10.0f); // Jumping costs stamina

        VOXELCRAFT_TRACE("Player '{}' jumped", m_playerName);
    }

    void PlayerComponent::Crouch(bool crouched) {
        if (m_isCrouching == crouched) {
            return;
        }

        m_isCrouching = crouched;

        auto transform = GetOwner()->GetComponent<TransformComponent>();
        if (transform) {
            if (crouched) {
                transform->SetScale(1.0f, 0.5f, 1.0f); // Half height
            } else {
                transform->SetScale(1.0f, 1.0f, 1.0f); // Normal height
            }
        }

        VOXELCRAFT_TRACE("Player '{}' {} crouched", m_playerName, crouched ? "is now" : "stood up from");
    }

    void PlayerComponent::ToggleFly() {
        if (!m_movement.canFly) {
            return;
        }

        m_isFlying = !m_isFlying;

        auto physics = GetOwner()->GetComponent<PhysicsComponent>();
        if (physics) {
            if (m_isFlying) {
                physics->SetGravityEnabled(false);
                physics->SetKinematic(true);
                SetState(PlayerState::Flying);
            } else {
                physics->SetGravityEnabled(true);
                physics->SetKinematic(false);
                SetState(PlayerState::Falling);
            }
        }

        VOXELCRAFT_INFO("Player '{}' {} flying", m_playerName, m_isFlying ? "started" : "stopped");
    }

    void PlayerComponent::Look(float yaw, float pitch) {
        if (!CanMove()) {
            return;
        }

        auto transform = GetOwner()->GetComponent<TransformComponent>();
        if (!transform) {
            return;
        }

        // Get current rotation
        glm::vec3 euler = glm::eulerAngles(transform->GetRotation());

        // Apply mouse input with sensitivity
        euler.y -= yaw * m_movement.mouseSensitivity;
        euler.x = glm::clamp(euler.x - pitch * m_movement.mouseSensitivity,
                           -glm::pi<float>() * 0.5f,
                           glm::pi<float>() * 0.5f);

        // Set new rotation
        transform->SetRotation(euler.x, euler.y, euler.z);
    }

    void PlayerComponent::Attack() {
        if (!CanMove()) {
            return;
        }

        VOXELCRAFT_TRACE("Player '{}' attacked", m_playerName);

        // TODO: Implement attack logic
        // - Raycast from camera
        // - Check for hit
        // - Apply damage
        // - Play animation/sound
    }

    void PlayerComponent::Use() {
        if (!CanMove()) {
            return;
        }

        VOXELCRAFT_TRACE("Player '{}' used/interacted", m_playerName);

        // TODO: Implement use/interaction logic
        // - Raycast for interactable objects
        // - Trigger interaction
        // - Play animation/sound
    }

    void PlayerComponent::UpdateMovement(double deltaTime) {
        if (!CanMove()) {
            return;
        }

        // Handle movement input
        glm::vec3 moveDir(0.0f);
        if (glm::length(m_currentInput.moveInput) > 0.001f) {
            // Calculate movement direction based on input
            glm::vec2 input = glm::normalize(m_currentInput.moveInput);

            auto transform = GetOwner()->GetComponent<TransformComponent>();
            if (transform) {
                glm::vec3 forward = transform->GetForward();
                glm::vec3 right = transform->GetRight();

                // Remove Y component for ground movement
                forward.y = 0.0f;
                right.y = 0.0f;

                if (glm::length(forward) > 0.001f) {
                    forward = glm::normalize(forward);
                }
                if (glm::length(right) > 0.001f) {
                    right = glm::normalize(right);
                }

                moveDir = forward * input.y + right * input.x;
            }
        }

        if (glm::length(moveDir) > 0.001f) {
            Move(moveDir, deltaTime);
        }
    }

    void PlayerComponent::UpdateState() {
        if (!IsAlive()) {
            return;
        }

        auto physics = GetOwner()->GetComponent<PhysicsComponent>();
        if (!physics) {
            return;
        }

        glm::vec3 velocity = physics->GetLinearVelocity();

        // Check if falling
        if (velocity.y < -1.0f && !m_isGrounded) {
            SetState(PlayerState::Falling);
        }
        // Check if swimming (TODO: implement water detection)
        else if (false) { // Placeholder for water detection
            SetState(PlayerState::Swimming);
        }
        // Idle state is set in UpdateMovement
    }

    void PlayerComponent::UpdateAI(double deltaTime) {
        // Simple AI behavior
        if (m_aiUpdateTimer >= 1.0f) { // Update AI every second
            m_aiUpdateTimer = 0.0f;

            auto transform = GetOwner()->GetComponent<TransformComponent>();
            if (transform) {
                glm::vec3 currentPos = transform->GetPosition();

                // Simple wandering AI
                if (glm::distance(currentPos, m_aiTarget) < 1.0f) {
                    // Set new random target
                    m_aiTarget = currentPos + glm::vec3(
                        (rand() % 21 - 10), // -10 to +10
                        0.0f,
                        (rand() % 21 - 10)  // -10 to +10
                    );
                }

                // Move towards target
                glm::vec3 direction = glm::normalize(m_aiTarget - currentPos);
                Move(direction, deltaTime);
            }
        }
    }

    void PlayerComponent::InitializeHuman() {
        VOXELCRAFT_INFO("Initialized human player '{}'", m_playerName);

        // Human players start with default movement
        m_movement = PlayerMovement();
    }

    void PlayerComponent::InitializeAI() {
        VOXELCRAFT_INFO("Initialized AI player '{}'", m_playerName);

        // AI players have different properties
        m_movement.walkSpeed *= 0.8f; // Slower than humans
        m_movement.runSpeed *= 0.7f;
        m_movement.canFly = false; // AI can't fly by default
    }

    void PlayerComponent::InitializeRemote() {
        VOXELCRAFT_INFO("Initialized remote player '{}'", m_playerName);

        // Remote players are similar to human but controlled by network
        m_movement = PlayerMovement();
    }

    void PlayerComponent::HandleLevelUp(int newLevel) {
        VOXELCRAFT_INFO("Player '{}' leveled up to level {}!", m_playerName, newLevel);

        // Increase stats on level up
        m_stats.maxHealth += 10.0f;
        m_stats.health = m_stats.maxHealth; // Heal on level up
        m_stats.maxStamina += 5.0f;

        // TODO: Show level up effects, sounds, etc.
    }

} // namespace VoxelCraft
