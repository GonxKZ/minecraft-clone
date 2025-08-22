/**
 * @file PlayerComponent.hpp
 * @brief VoxelCraft ECS - Player Component
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * Player component for entities that represent players (human or AI controlled).
 */

#ifndef VOXELCRAFT_ENTITIES_PLAYER_COMPONENT_HPP
#define VOXELCRAFT_ENTITIES_PLAYER_COMPONENT_HPP

#include "Component.hpp"
#include <memory>

namespace VoxelCraft {

    /**
     * @enum PlayerType
     * @brief Type of player control
     */
    enum class PlayerType {
        Human,          ///< Human player (keyboard/mouse/gamepad input)
        AI,             ///< AI controlled player
        Remote          ///< Remote/network player
    };

    /**
     * @enum PlayerState
     * @brief Current state of the player
     */
    enum class PlayerState {
        Idle,           ///< Player is idle
        Walking,        ///< Player is walking
        Running,        ///< Player is running
        Jumping,        ///< Player is jumping
        Falling,        ///< Player is falling
        Swimming,       ///< Player is swimming
        Flying,         ///< Player is flying
        Dead,           ///< Player is dead
        Respawning      ///< Player is respawning
    };

    /**
     * @struct PlayerInput
     * @brief Input state for the player
     */
    struct PlayerInput {
        glm::vec2 moveInput;            ///< Movement input (x: left/right, y: forward/back)
        glm::vec2 lookInput;            ///< Look input (x: yaw, y: pitch)
        bool jumpPressed;               ///< Jump button pressed
        bool runPressed;                ///< Run button pressed
        bool crouchPressed;             ///< Crouch button pressed
        bool flyPressed;                ///< Fly toggle pressed
        bool attackPressed;             ///< Attack button pressed
        bool usePressed;                ///< Use/interact button pressed

        PlayerInput()
            : moveInput(0.0f)
            , lookInput(0.0f)
            , jumpPressed(false)
            , runPressed(false)
            , crouchPressed(false)
            , flyPressed(false)
            , attackPressed(false)
            , usePressed(false)
        {}
    };

    /**
     * @struct PlayerStats
     * @brief Player statistics and attributes
     */
    struct PlayerStats {
        float health;                   ///< Current health
        float maxHealth;               ///< Maximum health
        float stamina;                 ///< Current stamina
        float maxStamina;              ///< Maximum stamina
        float hunger;                  ///< Current hunger level
        float maxHunger;               ///< Maximum hunger level
        int level;                     ///< Player level
        int experience;                ///< Current experience points
        int score;                     ///< Player score

        PlayerStats()
            : health(100.0f)
            , maxHealth(100.0f)
            , stamina(100.0f)
            , maxStamina(100.0f)
            , hunger(100.0f)
            , maxHunger(100.0f)
            , level(1)
            , experience(0)
            , score(0)
        {}
    };

    /**
     * @struct PlayerMovement
     * @brief Player movement properties
     */
    struct PlayerMovement {
        float walkSpeed;               ///< Walking speed (m/s)
        float runSpeed;                ///< Running speed (m/s)
        float jumpForce;               ///< Jump force
        float gravity;                 ///< Gravity strength
        float mouseSensitivity;        ///< Mouse sensitivity
        float gamepadSensitivity;      ///< Gamepad sensitivity
        bool canFly;                   ///< Can fly
        bool canSwim;                  ///< Can swim
        bool canCrouch;                ///< Can crouch
        float height;                  ///< Player height
        float width;                   ///< Player width
        float eyeHeight;               ///< Eye height from ground

        PlayerMovement()
            : walkSpeed(4.3f)
            , runSpeed(6.5f)
            , jumpForce(8.0f)
            , gravity(9.81f)
            , mouseSensitivity(0.5f)
            , gamepadSensitivity(1.0f)
            , canFly(false)
            , canSwim(true)
            , canCrouch(true)
            , height(1.8f)
            , width(0.6f)
            , eyeHeight(1.62f)
        {}
    };

    /**
     * @class PlayerComponent
     * @brief Component for player-controlled entities
     *
     * The PlayerComponent handles player input, movement, stats, and interactions.
     * It can work with both human players and AI-controlled entities.
     */
    class PlayerComponent : public Component {
    public:
        /**
         * @brief Constructor
         * @param owner Entity that owns this component
         */
        PlayerComponent(Entity* owner);

        /**
         * @brief Constructor with player type
         * @param owner Entity that owns this component
         * @param playerType Type of player control
         */
        PlayerComponent(Entity* owner, PlayerType playerType);

        /**
         * @brief Constructor with full configuration
         * @param owner Entity that owns this component
         * @param playerType Type of player control
         * @param playerName Player name
         */
        PlayerComponent(Entity* owner, PlayerType playerType, const std::string& playerName);

        /**
         * @brief Destructor
         */
        ~PlayerComponent() override;

        /**
         * @brief Get component type name
         * @return Component type name
         */
        std::string GetTypeName() const override { return "PlayerComponent"; }

        /**
         * @brief Initialize the component
         * @return true if successful, false otherwise
         */
        bool Initialize() override;

        /**
         * @brief Update the component
         * @param deltaTime Time elapsed since last update
         */
        void Update(double deltaTime) override;

        /**
         * @brief Render debug information
         */
        void Render() override;

        // Player identity

        /**
         * @brief Set player name
         * @param name New player name
         */
        void SetPlayerName(const std::string& name);

        /**
         * @brief Get player name
         * @return Player name
         */
        const std::string& GetPlayerName() const { return m_playerName; }

        /**
         * @brief Set player type
         * @param type New player type
         */
        void SetPlayerType(PlayerType type) { m_playerType = type; }

        /**
         * @brief Get player type
         * @return Player type
         */
        PlayerType GetPlayerType() const { return m_playerType; }

        // Input handling

        /**
         * @brief Process input for this frame
         * @param input Current input state
         */
        void ProcessInput(const PlayerInput& input);

        /**
         * @brief Get current input state
         * @return Current input state
         */
        const PlayerInput& GetCurrentInput() const { return m_currentInput; }

        // Player stats

        /**
         * @brief Set player stats
         * @param stats New player stats
         */
        void SetStats(const PlayerStats& stats);

        /**
         * @brief Get player stats
         * @return Current player stats
         */
        const PlayerStats& GetStats() const { return m_stats; }

        /**
         * @brief Modify health
         * @param amount Amount to modify (positive = heal, negative = damage)
         * @return true if health changed, false if invalid amount
         */
        bool ModifyHealth(float amount);

        /**
         * @brief Modify stamina
         * @param amount Amount to modify
         * @return true if stamina changed, false if invalid amount
         */
        bool ModifyStamina(float amount);

        /**
         * @brief Add experience points
         * @param amount Amount of XP to add
         * @return true if level up occurred, false otherwise
         */
        bool AddExperience(int amount);

        // Movement properties

        /**
         * @brief Set movement properties
         * @param movement New movement properties
         */
        void SetMovement(const PlayerMovement& movement);

        /**
         * @brief Get movement properties
         * @return Current movement properties
         */
        const PlayerMovement& GetMovement() const { return m_movement; }

        // State management

        /**
         * @brief Get current player state
         * @return Player state
         */
        PlayerState GetState() const { return m_state; }

        /**
         * @brief Set player state
         * @param state New player state
         */
        void SetState(PlayerState state);

        /**
         * @brief Check if player is alive
         * @return true if alive, false if dead
         */
        bool IsAlive() const { return m_state != PlayerState::Dead; }

        /**
         * @brief Check if player can move
         * @return true if can move, false otherwise
         */
        bool CanMove() const;

        // Movement operations

        /**
         * @brief Move the player
         * @param direction Movement direction
         * @param deltaTime Time elapsed since last update
         */
        void Move(const glm::vec3& direction, double deltaTime);

        /**
         * @brief Jump
         */
        void Jump();

        /**
         * @brief Crouch
         * @param crouched True to crouch, false to stand
         */
        void Crouch(bool crouched);

        /**
         * @brief Toggle fly mode
         */
        void ToggleFly();

        /**
         * @brief Look in direction
         * @param yaw Yaw angle change
         * @param pitch Pitch angle change
         */
        void Look(float yaw, float pitch);

        // Interaction

        /**
         * @brief Attack/interact with world
         */
        void Attack();

        /**
         * @brief Use/interact with objects
         */
        void Use();

        // Physics integration

        /**
         * @brief Update movement based on input
         * @param deltaTime Time elapsed since last update
         */
        void UpdateMovement(double deltaTime);

        /**
         * @brief Update player state based on movement
         */
        void UpdateState();

        /**
         * @brief Handle ground collision
         * @param isGrounded True if player is on ground
         */
        void SetGrounded(bool isGrounded) { m_isGrounded = isGrounded; }

        /**
         * @brief Check if player is grounded
         * @return true if grounded, false otherwise
         */
        bool IsGrounded() const { return m_isGrounded; }

        // AI specific (for AI players)

        /**
         * @brief Set AI target position
         * @param target Target position
         */
        void SetAITarget(const glm::vec3& target) { m_aiTarget = target; }

        /**
         * @brief Get AI target position
         * @return AI target position
         */
        const glm::vec3& GetAITarget() const { return m_aiTarget; }

        /**
         * @brief Update AI behavior
         * @param deltaTime Time elapsed since last update
         */
        void UpdateAI(double deltaTime);

    private:
        /**
         * @brief Initialize human player
         */
        void InitializeHuman();

        /**
         * @brief Initialize AI player
         */
        void InitializeAI();

        /**
         * @brief Initialize remote player
         */
        void InitializeRemote();

        /**
         * @brief Handle level up
         * @param newLevel New level reached
         */
        void HandleLevelUp(int newLevel);

        std::string m_playerName;               ///< Player name
        PlayerType m_playerType;                ///< Type of player control
        PlayerState m_state;                    ///< Current player state
        PlayerInput m_currentInput;             ///< Current input state
        PlayerStats m_stats;                    ///< Player statistics
        PlayerMovement m_movement;              ///< Movement properties

        bool m_isGrounded;                      ///< Is player on ground
        bool m_isCrouching;                     ///< Is player crouching
        bool m_isFlying;                        ///< Is player flying
        float m_staminaRegenTimer;              ///< Stamina regeneration timer
        float m_jumpTimer;                      ///< Jump cooldown timer

        // AI specific
        glm::vec3 m_aiTarget;                   ///< AI movement target
        float m_aiUpdateTimer;                  ///< AI update timer
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_ENTITIES_PLAYER_COMPONENT_HPP
