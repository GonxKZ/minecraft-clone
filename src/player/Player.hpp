/**
 * @file Player.hpp
 * @brief VoxelCraft Player Entity - Player character implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_PLAYER_PLAYER_HPP
#define VOXELCRAFT_PLAYER_PLAYER_HPP

#include <memory>
#include <string>
#include <unordered_map>

#include "../entities/Entity.hpp"
#include "../inventory/Inventory.hpp"
#include "../tools/Tool.hpp"

namespace VoxelCraft {

    // Forward declarations
    class World;
    class Camera;
    struct Vec3;
    struct Vec2;

    /**
     * @enum PlayerState
     * @brief Current state of the player
     */
    enum class PlayerState {
        IDLE,           // Standing still
        WALKING,        // Walking normally
        RUNNING,        // Running/sprinting
        JUMPING,        // In air after jumping
        FALLING,        // Falling down
        SWIMMING,       // In water
        CLIMBING,       // Climbing ladder/vine
        SNEAKING,       // Crouching
        FLYING,         // Creative flying
        DEAD            // Dead/dying
    };

    /**
     * @struct PlayerStats
     * @brief Player statistics and attributes
     */
    struct PlayerStats {
        float health;
        float maxHealth;
        float walkSpeed;
        float runSpeed;
        float jumpHeight;
        bool isGrounded;
        bool isSneaking;
        bool isSprinting;

        PlayerStats()
            : health(20.0f)
            , maxHealth(20.0f)
            , walkSpeed(4.317f)
            , runSpeed(5.612f)
            , jumpHeight(1.25f)
            , isGrounded(true)
            , isSneaking(false)
            , isSprinting(false)
        {}
    };

    /**
     * @struct PlayerInput
     * @brief Current player input state
     */
    struct PlayerInput {
        float moveForward;
        float moveRight;
        bool jumpPressed;
        bool sneakPressed;
        bool sprintPressed;

        PlayerInput()
            : moveForward(0.0f)
            , moveRight(0.0f)
            , jumpPressed(false)
            , sneakPressed(false)
            , sprintPressed(false)
        {}
    };

    /**
     * @class Player
     * @brief Player character entity with Minecraft-like behavior
     */
    class Player : public Entity {
    public:
        Player(const std::string& name = "Player");
        ~Player();

        bool Initialize();
        void Update(float deltaTime) override;
        void HandleInput(const PlayerInput& input, float deltaTime);

        const PlayerStats& GetStats() const { return m_stats; }
        std::shared_ptr<Inventory> GetInventory() const { return m_inventory; }
        PlayerState GetState() const { return m_state; }

        void SetCamera(std::shared_ptr<Camera> camera);
        void SetWorld(std::shared_ptr<World> world);

        void Teleport(const Vec3& position);
        void TakeDamage(float damage);
        void Heal(float amount);
        bool IsAlive() const { return m_stats.health > 0.0f; }

    private:
        PlayerStats m_stats;
        PlayerState m_state;
        PlayerInput m_currentInput;
        std::shared_ptr<Inventory> m_inventory;
        std::shared_ptr<Camera> m_camera;
        std::shared_ptr<World> m_world;

        Vec3 m_velocity;
        bool m_isJumping;
        float m_eyeHeight;

        void UpdateMovement(float deltaTime);
        void UpdateState();
        void ApplyPhysics(float deltaTime);
        void HandleJumping();
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_PLAYER_PLAYER_HPP