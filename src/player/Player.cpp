/**
 * @file Player.cpp
 * @brief VoxelCraft Player Entity - Player character implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#include "Player.hpp"
#include <algorithm>
#include <cmath>

namespace VoxelCraft {

// Simple vector structures (should be shared with other files)
struct Vec2 {
    float x, y;
    Vec2(float x = 0, float y = 0) : x(x), y(y) {}
};

struct Vec3 {
    float x, y, z;
    Vec3(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {}

    Vec3 operator+(const Vec3& other) const {
        return Vec3(x + other.x, y + other.y, z + other.z);
    }

    Vec3 operator-(const Vec3& other) const {
        return Vec3(x - other.x, y - other.y, z - other.z);
    }

    Vec3 operator*(float scalar) const {
        return Vec3(x * scalar, y * scalar, z * scalar);
    }

    Vec3& operator+=(const Vec3& other) {
        x += other.x; y += other.y; z += other.z;
        return *this;
    }

    float length() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    Vec3 normalized() const {
        float len = length();
        if (len > 0) {
            return Vec3(x / len, y / len, z / len);
        }
        return Vec3(0, 0, 0);
    }
};

Player::Player(const std::string& name)
    : Entity(name)
    , m_state(PlayerState::IDLE)
    , m_velocity(0, 0, 0)
    , m_isJumping(false)
    , m_eyeHeight(1.62f) // Minecraft eye height
{
    // Set player dimensions (Minecraft-like)
    SetPosition(Vec3(0, m_eyeHeight, 0));

    // Create player inventory
    m_inventory = std::make_shared<Inventory>(InventoryType::PLAYER);
    m_inventory->Initialize();
}

Player::~Player() {
    // Cleanup will be handled by smart pointers
}

bool Player::Initialize() {
    // Initialize entity base
    if (!Entity::Initialize()) {
        return false;
    }

    // Set player-specific components
    // TODO: Add PlayerComponent if needed

    return true;
}

void Player::Update(float deltaTime) {
    // Update entity base
    Entity::Update(deltaTime);

    // Update player-specific logic
    UpdateMovement(deltaTime);
    UpdateState();
    ApplyPhysics(deltaTime);

    // Update camera if attached
    if (m_camera) {
        UpdateCamera();
    }
}

void Player::HandleInput(const PlayerInput& input, float deltaTime) {
    m_currentInput = input;

    // Handle movement input
    if (std::abs(input.moveForward) > 0.1f || std::abs(input.moveRight) > 0.1f) {
        UpdateMovement(deltaTime);
    }

    // Handle jumping
    if (input.jumpPressed && CanJump()) {
        HandleJumping();
    }

    // Handle sneaking
    if (input.sneakPressed != m_stats.isSneaking) {
        m_stats.isSneaking = input.sneakPressed;
        if (m_stats.isSneaking) {
            m_eyeHeight = 1.42f; // Lower eye height when sneaking
        } else {
            m_eyeHeight = 1.62f; // Normal eye height
        }
    }

    // Handle sprinting
    m_stats.isSprinting = input.sprintPressed && CanSprint();
}

void Player::UpdateMovement(float deltaTime) {
    // Calculate movement direction based on input
    Vec3 moveDir(0, 0, 0);

    // Get camera forward/right vectors for relative movement
    if (m_camera) {
        Vec3 forward = m_camera->GetForwardVector();
        Vec3 right = m_camera->GetRightVector();

        // Flatten vectors for horizontal movement
        forward.y = 0;
        right.y = 0;

        // Normalize vectors
        forward = forward.normalized();
        right = right.normalized();

        // Apply input
        moveDir = forward * m_currentInput.moveForward + right * m_currentInput.moveRight;
    } else {
        // Simple world-space movement if no camera
        moveDir = Vec3(m_currentInput.moveRight, 0, m_currentInput.moveForward);
    }

    // Normalize movement direction
    if (moveDir.length() > 0) {
        moveDir = moveDir.normalized();
    }

    // Calculate movement speed
    float speed = CalculateMovementSpeed();
    Vec3 targetVelocity = moveDir * speed;

    // Apply movement with smooth acceleration
    float acceleration = 20.0f; // Units per second
    Vec3 velocityDiff = targetVelocity - m_velocity;
    velocityDiff.y = 0; // Don't accelerate vertically here

    if (velocityDiff.length() > 0) {
        Vec3 accelerationVec = velocityDiff.normalized() * acceleration * deltaTime;
        if (accelerationVec.length() > velocityDiff.length()) {
            m_velocity.x = targetVelocity.x;
            m_velocity.z = targetVelocity.z;
        } else {
            m_velocity += accelerationVec;
        }
    }

    // Update position
    Vec3 newPosition = GetPosition() + m_velocity * deltaTime;
    SetPosition(newPosition);
}

void Player::UpdateState() {
    // Update grounded state (simple check for now)
    // In a full implementation, this would check collision with ground
    if (GetPosition().y <= m_eyeHeight) {
        m_stats.isGrounded = true;
        m_velocity.y = 0;
        if (m_isJumping) {
            m_isJumping = false;
        }
    } else {
        m_stats.isGrounded = false;
    }

    // Update player state based on movement
    float horizontalSpeed = Vec3(m_velocity.x, 0, m_velocity.z).length();

    if (!IsAlive()) {
        m_state = PlayerState::DEAD;
    } else if (m_stats.isSneaking) {
        m_state = PlayerState::SNEAKING;
    } else if (!m_stats.isGrounded) {
        m_state = m_velocity.y > 0 ? PlayerState::JUMPING : PlayerState::FALLING;
    } else if (horizontalSpeed > m_stats.runSpeed * 0.8f) {
        m_state = PlayerState::RUNNING;
    } else if (horizontalSpeed > 0.1f) {
        m_state = PlayerState::WALKING;
    } else {
        m_state = PlayerState::IDLE;
    }
}

void Player::ApplyPhysics(float deltaTime) {
    // Apply gravity if not grounded and not flying
    if (!m_stats.isGrounded && !m_stats.isFlying) {
        float gravity = 9.81f * 3.0f; // Minecraft-like gravity
        m_velocity.y -= gravity * deltaTime;

        // Terminal velocity
        float terminalVelocity = -78.4f;
        if (m_velocity.y < terminalVelocity) {
            m_velocity.y = terminalVelocity;
        }
    }

    // Apply friction
    ApplyFriction(deltaTime);
}

void Player::HandleJumping() {
    if (CanJump()) {
        m_velocity.y = std::sqrt(2.0f * 9.81f * 3.0f * m_stats.jumpHeight);
        m_isJumping = true;
        m_stats.isGrounded = false;
    }
}

void Player::ApplyFriction(float deltaTime) {
    if (m_stats.isGrounded) {
        // Ground friction
        float friction = 0.91f; // Minecraft ground friction
        m_velocity.x *= friction;
        m_velocity.z *= friction;

        // Stop very small velocities
        if (std::abs(m_velocity.x) < 0.01f) m_velocity.x = 0;
        if (std::abs(m_velocity.z) < 0.01f) m_velocity.z = 0;
    } else {
        // Air friction (less friction)
        float airFriction = 0.98f;
        m_velocity.x *= airFriction;
        m_velocity.z *= airFriction;
    }
}

void Player::UpdateCamera() {
    if (!m_camera) return;

    // Update camera position to follow player
    Vec3 playerPos = GetPosition();
    Vec3 cameraPos = Vec3(playerPos.x, playerPos.y + m_eyeHeight, playerPos.z);

    m_camera->position = cameraPos;
}

float Player::CalculateMovementSpeed() const {
    if (m_stats.isSneaking) {
        return m_stats.walkSpeed * 0.3f; // Sneak speed multiplier
    } else if (m_stats.isSprinting) {
        return m_stats.runSpeed;
    } else {
        return m_stats.walkSpeed;
    }
}

bool Player::CanJump() const {
    return m_stats.isGrounded && IsAlive() && !m_stats.isSneaking;
}

bool Player::CanSprint() const {
    return m_stats.isGrounded && IsAlive() && !m_stats.isSneaking &&
           Vec3(m_velocity.x, 0, m_velocity.z).length() > m_stats.walkSpeed * 0.8f;
}

void Player::SetCamera(std::shared_ptr<Camera> camera) {
    m_camera = camera;
}

void Player::SetWorld(std::shared_ptr<World> world) {
    m_world = world;
}

void Player::Teleport(const Vec3& position) {
    SetPosition(position);
    m_velocity = Vec3(0, 0, 0);
    if (m_camera) {
        UpdateCamera();
    }
}

void Player::TakeDamage(float damage) {
    if (!IsAlive()) return;

    m_stats.health -= damage;
    if (m_stats.health <= 0) {
        m_stats.health = 0;
        // Handle death
    }
}

void Player::Heal(float amount) {
    if (!IsAlive()) return;

    m_stats.health = std::min(m_stats.health + amount, m_stats.maxHealth);
}

} // namespace VoxelCraft
