/**
 * @file Player.hpp
 * @brief VoxelCraft Player Character System
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines the Player class that represents the player character
 * in the VoxelCraft game world with full character functionality.
 */

#ifndef VOXELCRAFT_PLAYER_PLAYER_HPP
#define VOXELCRAFT_PLAYER_PLAYER_HPP

#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <atomic>

#include "../core/Config.hpp"
#include "../entities/Entity.hpp"
#include "../physics/RigidBody.hpp"
#include "../physics/PhysicsTypes.hpp"
#include "../world/World.hpp"

namespace VoxelCraft {

    // Forward declarations
    class World;
    class Camera;
    class CharacterController;
    class Inventory;
    class PlayerInput;
    class AnimationController;
    class MovementController;
    class PlayerPhysics;

    /**
     * @enum PlayerState
     * @brief Current state of the player
     */
    enum class PlayerState {
        Idle,                   ///< Player is idle
        Walking,                ///< Player is walking
        Running,                ///< Player is running
        Jumping,                ///< Player is jumping
        Falling,                ///< Player is falling
        Swimming,               ///< Player is swimming
        Flying,                 ///< Player is flying
        Sneaking,               ///< Player is sneaking
        Climbing,               ///< Player is climbing
        Dead,                   ///< Player is dead
        Spectating,             ///< Player is spectating
        Custom                  ///< Custom player state
    };

    /**
     * @enum PlayerAction
     * @brief Actions the player can perform
     */
    enum class PlayerAction {
        None,                   ///< No action
        Jump,                   ///< Jump action
        Attack,                 ///< Attack action
        Use,                    ///< Use/interact action
        Sneak,                  ///< Sneak/crouch action
        Sprint,                 ///< Sprint action
        Fly,                    ///< Toggle fly mode
        DropItem,               ///< Drop held item
        SwitchItem,             ///< Switch to next item
        OpenInventory,          ///< Open inventory
        Pause,                  ///< Pause game
        Custom1,                ///< Custom action 1
        Custom2,                ///< Custom action 2
        Custom3,                ///< Custom action 3
        Custom4                 ///< Custom action 4
    };

    /**
     * @enum PlayerMovementMode
     * @brief Movement modes available to the player
     */
    enum class PlayerMovementMode {
        Walk,                   ///< Normal walking
        Run,                    ///< Running
        Sneak,                  ///< Sneaking/crouching
        Fly,                    ///< Flying
        Swim,                   ///< Swimming
        Climb,                  ///< Climbing
        Spectator               ///< Spectator mode (no collision)
    };

    /**
     * @enum PlayerAbility
     * @brief Special abilities the player can have
     */
    enum class PlayerAbility {
        DoubleJump,             ///< Can jump twice
        WallJump,               ///< Can jump off walls
        Glide,                  ///< Can glide
        Teleport,               ///< Can teleport
        Invisibility,           ///< Can become invisible
        SuperSpeed,             ///< Can move very fast
        SuperJump,              ///< Can jump very high
        NightVision,            ///< Can see in the dark
        WaterBreathing,         ///< Can breathe underwater
        FireResistance,         ///< Resistant to fire
        FreezeResistance,       ///< Resistant to cold
        Custom1,                ///< Custom ability 1
        Custom2,                ///< Custom ability 2
        Custom3,                ///< Custom ability 3
        Custom4                 ///< Custom ability 4
    };

    /**
     * @struct PlayerStats
     * @brief Player character statistics
     */
    struct PlayerStats {
        // Health and damage
        float health;                           ///< Current health
        float maxHealth;                        ///< Maximum health
        float armor;                            ///< Armor protection
        float damageMultiplier;                 ///< Damage multiplier

        // Movement stats
        float walkSpeed;                        ///< Walking speed
        float runSpeed;                         ///< Running speed
        float sneakSpeed;                       ///< Sneaking speed
        float flySpeed;                         ///< Flying speed
        float swimSpeed;                        ///< Swimming speed
        float jumpHeight;                       ///< Jump height
        float jumpForce;                        ///< Jump force

        // Combat stats
        float attackDamage;                     ///< Base attack damage
        float attackSpeed;                      ///< Attack speed multiplier
        float attackRange;                      ///< Attack range
        float criticalHitChance;                ///< Critical hit chance (0.0 - 1.0)
        float criticalHitMultiplier;            ///< Critical hit damage multiplier

        // Survival stats
        float hunger;                           ///< Current hunger level
        float maxHunger;                        ///< Maximum hunger level
        float thirst;                           ///< Current thirst level
        float maxThirst;                        ///< Maximum thirst level
        float stamina;                          ///< Current stamina
        float maxStamina;                       ///< Maximum stamina
        float oxygen;                           ///< Current oxygen level
        float maxOxygen;                        ///< Maximum oxygen level

        // Experience and progression
        uint32_t level;                         ///< Player level
        uint64_t experience;                    ///< Current experience points
        uint64_t experienceToNextLevel;         ///< Experience needed for next level
        uint32_t skillPoints;                   ///< Available skill points

        // Environmental effects
        float temperature;                      ///< Body temperature
        float wetness;                          ///< Wetness level (0.0 - 1.0)
        bool isOnFire;                          ///< Player is on fire
        bool isPoisoned;                        ///< Player is poisoned
        bool isBleeding;                        ///< Player is bleeding
        bool isFrozen;                          ///< Player is frozen

        // Game statistics
        uint64_t blocksMined;                   ///< Total blocks mined
        uint64_t blocksPlaced;                  ///< Total blocks placed
        uint64_t mobsKilled;                    ///< Total mobs killed
        uint64_t distanceTraveled;              ///< Distance traveled (blocks)
        uint64_t playTime;                      ///< Total play time (seconds)
        uint64_t deaths;                        ///< Total deaths
    };

    /**
     * @struct PlayerConfig
     * @brief Player configuration settings
     */
    struct PlayerConfig {
        // Basic settings
        std::string playerName;                 ///< Player display name
        std::string skinPath;                   ///< Player skin texture path
        std::string modelPath;                  ///< Player model path

        // Movement settings
        float mouseSensitivity;                 ///< Mouse sensitivity (0.0 - 1.0)
        bool invertMouse;                       ///< Invert mouse Y axis
        float fov;                              ///< Field of view (degrees)
        bool enableAutoJump;                    ///< Enable automatic jumping

        // Control settings
        bool enableDoubleTapSprint;             ///< Enable double-tap to sprint
        bool enableCrouchToggle;                ///< Toggle crouch instead of hold
        bool enableFlyToggle;                   ///< Toggle fly instead of hold
        float controllerDeadzone;               ///< Controller deadzone (0.0 - 1.0)

        // Game settings
        bool enablePvP;                         ///< Enable player vs player combat
        bool enableDrowning;                    ///< Enable drowning damage
        bool enableHunger;                      ///< Enable hunger system
        bool enableStamina;                     ///< Enable stamina system

        // Graphics settings
        bool enablePlayerShadow;                ///< Enable player shadow
        bool enablePlayerReflections;           ///< Enable player reflections
        float playerModelScale;                 ///< Player model scale
        bool enableFirstPersonMode;             ///< Enable first person camera

        // Audio settings
        float masterVolume;                     ///< Master volume (0.0 - 1.0)
        float effectsVolume;                    ///< Sound effects volume
        float musicVolume;                      ///< Music volume
        float voiceVolume;                      ///< Voice chat volume

        // Advanced settings
        bool enableDebugMode;                   ///< Enable debug features
        bool enableCheats;                      ///< Enable cheat commands
        bool enableModSupport;                  ///< Enable mod support
        int renderDistance;                     ///< Player render distance
    };

    /**
     * @struct PlayerMetrics
     * @brief Performance and gameplay metrics for the player
     */
    struct PlayerMetrics {
        // Performance metrics
        uint64_t updateCount;                   ///< Number of updates performed
        double totalUpdateTime;                 ///< Total update time (ms)
        double averageUpdateTime;              ///< Average update time (ms)
        double maxUpdateTime;                  ///< Maximum update time (ms)

        // Input metrics
        uint64_t inputEvents;                   ///< Total input events processed
        uint64_t mouseEvents;                   ///< Mouse input events
        uint64_t keyboardEvents;                ///< Keyboard input events
        uint64_t controllerEvents;              ///< Controller input events

        // Movement metrics
        uint64_t jumpsPerformed;                ///< Total jumps performed
        uint64_t fallsTaken;                    ///< Total falls taken
        float maxFallDistance;                  ///< Maximum fall distance
        float totalDistanceTraveled;            ///< Total distance traveled

        // Combat metrics
        uint64_t attacksPerformed;              ///< Total attacks performed
        uint64_t damageDealt;                   ///< Total damage dealt
        uint64_t damageReceived;                ///< Total damage received
        uint64_t kills;                         ///< Total kills
        uint64_t deaths;                        ///< Total deaths

        // Interaction metrics
        uint64_t blocksInteracted;              ///< Blocks interacted with
        uint64_t itemsUsed;                     ///< Items used
        uint64_t itemsCrafted;                  ///< Items crafted
        uint64_t itemsCollected;                ///< Items collected

        // Camera metrics
        float averageCameraDistance;            ///< Average camera distance
        float maxCameraDistance;                ///< Maximum camera distance
        uint64_t cameraCollisions;              ///< Camera collision events

        // Network metrics (if applicable)
        uint64_t packetsSent;                   ///< Packets sent
        uint64_t packetsReceived;               ///< Packets received
        float averageLatency;                   ///< Average network latency
        float packetLoss;                       ///< Packet loss percentage
    };

    /**
     * @class Player
     * @brief Player character with full game functionality
     *
     * The Player class represents the player character in the VoxelCraft game world.
     * It provides comprehensive character functionality including movement, combat,
     * inventory management, camera control, and interaction with the game world.
     *
     * Key Features:
     * - Multiple movement modes (walk, run, sneak, fly, swim, climb)
     * - Advanced camera system with collision detection
     * - Physics-based character controller
     * - Comprehensive input handling
     * - Inventory and item management
     * - Health, hunger, and survival systems
     * - Combat and damage systems
     * - Animation and visual effects
     * - Multiplayer networking support
     * - Save/load functionality
     *
     * The player integrates seamlessly with the physics system, entity system,
     * and world management to provide a complete gaming experience.
     */
    class Player : public Entity {
    public:
        /**
         * @brief Constructor
         * @param config Player configuration
         */
        explicit Player(const PlayerConfig& config);

        /**
         * @brief Destructor
         */
        ~Player() override;

        /**
         * @brief Deleted copy constructor
         */
        Player(const Player&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        Player& operator=(const Player&) = delete;

        // Entity interface implementation

        /**
         * @brief Update player entity
         * @param deltaTime Time elapsed since last update
         */
        void Update(double deltaTime) override;

        /**
         * @brief Render player entity
         */
        void Render() override;

        /**
         * @brief Get entity type
         * @return Entity type
         */
        EntityType GetType() const override { return EntityType::Player; }

        /**
         * @brief Get entity position
         * @return World position
         */
        const glm::vec3& GetPosition() const override;

        /**
         * @brief Set entity position
         * @param position New world position
         */
        void SetPosition(const glm::vec3& position) override;

        /**
         * @brief Get entity rotation
         * @return World rotation quaternion
         */
        const glm::quat& GetRotation() const override;

        /**
         * @brief Set entity rotation
         * @param rotation New world rotation quaternion
         */
        void SetRotation(const glm::quat& rotation) override;

        /**
         * @brief Get entity scale
         * @return World scale
         */
        const glm::vec3& GetScale() const override;

        /**
         * @brief Set entity scale
         * @param scale New world scale
         */
        void SetScale(const glm::vec3& scale) override;

        // Player identification and configuration

        /**
         * @brief Get player name
         * @return Player name
         */
        const std::string& GetName() const { return m_config.playerName; }

        /**
         * @brief Set player name
         * @param name New player name
         */
        void SetName(const std::string& name);

        /**
         * @brief Get player ID
         * @return Unique player ID
         */
        uint32_t GetPlayerID() const { return m_playerId; }

        /**
         * @brief Get player configuration
         * @return Player configuration
         */
        const PlayerConfig& GetConfig() const { return m_config; }

        /**
         * @brief Set player configuration
         * @param config New configuration
         */
        void SetConfig(const PlayerConfig& config);

        // Player state and stats

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
         * @brief Get player stats
         * @return Player statistics
         */
        const PlayerStats& GetStats() const { return m_stats; }

        /**
         * @brief Set player stats
         * @param stats New player statistics
         */
        void SetStats(const PlayerStats& stats);

        /**
         * @brief Get current health
         * @return Current health
         */
        float GetHealth() const { return m_stats.health; }

        /**
         * @brief Set health
         * @param health New health value
         */
        void SetHealth(float health);

        /**
         * @brief Damage player
         * @param damage Amount of damage
         * @param damageType Type of damage
         * @param attacker Entity that caused damage (optional)
         * @return true if damage was applied, false otherwise
         */
        bool TakeDamage(float damage, const std::string& damageType = "generic", Entity* attacker = nullptr);

        /**
         * @brief Heal player
         * @param healAmount Amount to heal
         * @return true if healing was applied, false otherwise
         */
        bool Heal(float healAmount);

        /**
         * @brief Check if player is alive
         * @return true if alive, false otherwise
         */
        bool IsAlive() const { return m_stats.health > 0.0f; }

        /**
         * @brief Kill player
         * @param killer Entity that killed the player (optional)
         */
        void Kill(Entity* killer = nullptr);

        /**
         * @brief Respawn player
         * @param position Respawn position (optional)
         */
        void Respawn(const glm::vec3& position = glm::vec3(0.0f));

        // Movement and controls

        /**
         * @brief Get movement mode
         * @return Current movement mode
         */
        PlayerMovementMode GetMovementMode() const { return m_movementMode; }

        /**
         * @brief Set movement mode
         * @param mode New movement mode
         */
        void SetMovementMode(PlayerMovementMode mode);

        /**
         * @brief Move player
         * @param direction Movement direction
         * @param speed Movement speed multiplier
         */
        void Move(const glm::vec3& direction, float speed = 1.0f);

        /**
         * @brief Jump
         * @param force Jump force multiplier
         * @return true if jump was performed, false otherwise
         */
        bool Jump(float force = 1.0f);

        /**
         * @brief Perform action
         * @param action Player action to perform
         * @return true if action was performed, false otherwise
         */
        bool PerformAction(PlayerAction action);

        /**
         * @brief Check if player can perform action
         * @param action Action to check
         * @return true if can perform, false otherwise
         */
        bool CanPerformAction(PlayerAction action) const;

        // Camera and view

        /**
         * @brief Get player camera
         * @return Player camera instance
         */
        Camera* GetCamera() const { return m_camera.get(); }

        /**
         * @brief Get camera position
         * @return Camera world position
         */
        glm::vec3 GetCameraPosition() const;

        /**
         * @brief Get camera forward direction
         * @return Camera forward vector
         */
        glm::vec3 GetCameraForward() const;

        /**
         * @brief Get camera up direction
         * @return Camera up vector
         */
        glm::vec3 GetCameraUp() const;

        /**
         * @brief Get camera right direction
         * @return Camera right vector
         */
        glm::vec3 GetCameraRight() const;

        /**
         * @brief Add camera yaw rotation
         * @param yaw Yaw rotation in degrees
         */
        void AddCameraYaw(float yaw);

        /**
         * @brief Add camera pitch rotation
         * @param pitch Pitch rotation in degrees
         */
        void AddCameraPitch(float pitch);

        /**
         * @brief Set camera distance
         * @param distance New camera distance
         */
        void SetCameraDistance(float distance);

        /**
         * @brief Get camera distance
         * @return Current camera distance
         */
        float GetCameraDistance() const;

        // Inventory and items

        /**
         * @brief Get player inventory
         * @return Player inventory instance
         */
        Inventory* GetInventory() const { return m_inventory.get(); }

        /**
         * @brief Get currently held item
         * @return Currently held item ID
         */
        uint32_t GetHeldItem() const;

        /**
         * @brief Set held item
         * @param itemId Item ID to hold
         */
        void SetHeldItem(uint32_t itemId);

        /**
         * @brief Switch to next item
         */
        void NextItem();

        /**
         * @brief Switch to previous item
         */
        void PreviousItem();

        /**
         * @brief Use held item
         * @return true if item was used, false otherwise
         */
        bool UseHeldItem();

        // Physics and collision

        /**
         * @brief Get player rigid body
         * @return Player rigid body
         */
        RigidBody* GetRigidBody() const { return m_rigidBody; }

        /**
         * @brief Get player physics controller
         * @return Player physics controller
         */
        PlayerPhysics* GetPlayerPhysics() const { return m_playerPhysics.get(); }

        /**
         * @brief Check if player is grounded
         * @return true if grounded, false otherwise
         */
        bool IsGrounded() const;

        /**
         * @brief Check if player is in water
         * @return true if in water, false otherwise
         */
        bool IsInWater() const;

        /**
         * @brief Check if player is on ladder
         * @return true if on ladder, false otherwise
         */
        bool IsOnLadder() const;

        /**
         * @brief Get player velocity
         * @return Player velocity vector
         */
        glm::vec3 GetVelocity() const;

        /**
         * @brief Set player velocity
         * @param velocity New velocity vector
         */
        void SetVelocity(const glm::vec3& velocity);

        // Abilities and effects

        /**
         * @brief Check if player has ability
         * @param ability Ability to check
         * @return true if has ability, false otherwise
         */
        bool HasAbility(PlayerAbility ability) const;

        /**
         * @brief Grant ability to player
         * @param ability Ability to grant
         */
        void GrantAbility(PlayerAbility ability);

        /**
         * @brief Revoke ability from player
         * @param ability Ability to revoke
         */
        void RevokeAbility(PlayerAbility ability);

        /**
         * @brief Get all player abilities
         * @return Vector of player abilities
         */
        std::vector<PlayerAbility> GetAbilities() const;

        /**
         * @brief Apply effect to player
         * @param effectType Type of effect
         * @param duration Effect duration (seconds)
         * @param strength Effect strength
         */
        void ApplyEffect(const std::string& effectType, float duration, float strength);

        /**
         * @brief Remove effect from player
         * @param effectType Type of effect
         */
        void RemoveEffect(const std::string& effectType);

        /**
         * @brief Check if player has effect
         * @param effectType Type of effect
         * @return true if has effect, false otherwise
         */
        bool HasEffect(const std::string& effectType) const;

        // Input and controls

        /**
         * @brief Get player input handler
         * @return Player input instance
         */
        PlayerInput* GetInput() const { return m_input.get(); }

        /**
         * @brief Process player input
         * @param deltaTime Time elapsed
         */
        void ProcessInput(double deltaTime);

        // Animation and rendering

        /**
         * @brief Get animation controller
         * @return Animation controller instance
         */
        AnimationController* GetAnimationController() const { return m_animationController.get(); }

        /**
         * @brief Play animation
         * @param animationName Animation name
         * @param loop Whether to loop the animation
         * @param blendTime Blend time with current animation
         */
        void PlayAnimation(const std::string& animationName, bool loop = false, float blendTime = 0.2f);

        /**
         * @brief Stop current animation
         * @param blendTime Blend time for stopping
         */
        void StopAnimation(float blendTime = 0.2f);

        /**
         * @brief Get current animation name
         * @return Current animation name
         */
        std::string GetCurrentAnimation() const;

        // World interaction

        /**
         * @brief Get world the player is in
         * @return Player's world
         */
        World* GetWorld() const { return m_world; }

        /**
         * @brief Set world for player
         * @param world New world
         */
        void SetWorld(World* world);

        /**
         * @brief Interact with object at look position
         * @return true if interaction occurred, false otherwise
         */
        bool Interact();

        /**
         * @brief Get look position (where player is looking)
         * @param maxDistance Maximum distance to check
         * @return Look position or empty optional if nothing found
         */
        std::optional<glm::vec3> GetLookPosition(float maxDistance = 100.0f) const;

        /**
         * @brief Get look target (entity or block player is looking at)
         * @param maxDistance Maximum distance to check
         * @return Look target information
         */
        std::optional<RaycastHit> GetLookTarget(float maxDistance = 100.0f) const;

        // Save and load

        /**
         * @brief Save player state
         * @param filename Save file path
         * @return true if saved successfully, false otherwise
         */
        bool SaveState(const std::string& filename) const;

        /**
         * @brief Load player state
         * @param filename Save file path
         * @return true if loaded successfully, false otherwise
         */
        bool LoadState(const std::string& filename);

        /**
         * @brief Get save data as string
         * @return Serialized player data
         */
        std::string Serialize() const;

        /**
         * @brief Load from serialized data
         * @param data Serialized player data
         * @return true if loaded successfully, false otherwise
         */
        bool Deserialize(const std::string& data);

        // Metrics and debugging

        /**
         * @brief Get player metrics
         * @return Performance metrics
         */
        const PlayerMetrics& GetMetrics() const { return m_metrics; }

        /**
         * @brief Reset metrics
         */
        void ResetMetrics();

        /**
         * @brief Get debug information
         * @return Debug information string
         */
        std::string GetDebugInfo() const;

        /**
         * @brief Validate player state
         * @return true if valid, false otherwise
         */
        bool Validate() const;

        // Event system

        /**
         * @brief Add event listener
         * @param eventType Event type
         * @param listener Listener function
         */
        void AddEventListener(const std::string& eventType, std::function<void(const std::string&)> listener);

        /**
         * @brief Remove event listener
         * @param eventType Event type
         */
        void RemoveEventListener(const std::string& eventType);

        /**
         * @brief Trigger event
         * @param eventType Event type
         * @param eventData Event data
         */
        void TriggerEvent(const std::string& eventType, const std::string& eventData = "");

    private:
        /**
         * @brief Initialize player subsystems
         * @return true if successful, false otherwise
         */
        bool InitializeSubsystems();

        /**
         * @brief Update player logic
         * @param deltaTime Time elapsed
         */
        void UpdateLogic(double deltaTime);

        /**
         * @brief Update player physics
         * @param deltaTime Time elapsed
         */
        void UpdatePhysics(double deltaTime);

        /**
         * @brief Update player animations
         * @param deltaTime Time elapsed
         */
        void UpdateAnimations(double deltaTime);

        /**
         * @brief Update player camera
         * @param deltaTime Time elapsed
         */
        void UpdateCamera(double deltaTime);

        /**
         * @brief Update player stats and effects
         * @param deltaTime Time elapsed
         */
        void UpdateStats(double deltaTime);

        /**
         * @brief Handle player death
         * @param killer Entity that killed the player
         */
        void HandleDeath(Entity* killer = nullptr);

        /**
         * @brief Handle player respawn
         * @param position Respawn position
         */
        void HandleRespawn(const glm::vec3& position);

        /**
         * @brief Update player metrics
         * @param deltaTime Time elapsed
         */
        void UpdateMetrics(double deltaTime);

        /**
         * @brief Handle physics collision
         * @param other Other rigid body
         * @param contactPoint Contact point
         * @param normal Contact normal
         */
        void HandleCollision(RigidBody* other, const glm::vec3& contactPoint, const glm::vec3& normal);

        // Player data
        uint32_t m_playerId;                                    ///< Unique player ID
        PlayerConfig m_config;                                  ///< Player configuration
        PlayerStats m_stats;                                    ///< Player statistics
        PlayerState m_state;                                    ///< Current player state
        PlayerMovementMode m_movementMode;                      ///< Current movement mode
        PlayerMetrics m_metrics;                                ///< Performance metrics

        // Subsystems
        std::unique_ptr<Camera> m_camera;                       ///< Player camera
        std::unique_ptr<CharacterController> m_characterController; ///< Character controller
        std::unique_ptr<Inventory> m_inventory;                 ///< Player inventory
        std::unique_ptr<PlayerInput> m_input;                   ///< Player input handler
        std::unique_ptr<AnimationController> m_animationController; ///< Animation controller
        std::unique_ptr<MovementController> m_movementController; ///< Movement controller
        std::unique_ptr<PlayerPhysics> m_playerPhysics;         ///< Player physics

        // Physics integration
        RigidBody* m_rigidBody;                                 ///< Player rigid body
        World* m_world;                                         ///< Player's world

        // Abilities and effects
        std::unordered_set<PlayerAbility> m_abilities;          ///< Player abilities
        std::unordered_map<std::string, float> m_activeEffects; ///< Active effects (type -> remaining time)
        std::unordered_map<std::string, float> m_effectStrengths; ///< Effect strengths

        // Event system
        std::unordered_map<std::string, std::vector<std::function<void(const std::string&)>>> m_eventListeners;
        mutable std::shared_mutex m_eventMutex;                 ///< Event synchronization

        // State variables
        bool m_isInitialized;                                   ///< Player is initialized
        double m_lastUpdateTime;                                ///< Last update timestamp
        double m_creationTime;                                  ///< Player creation timestamp

        static std::atomic<uint32_t> s_nextPlayerId;           ///< Next player ID counter
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_PLAYER_PLAYER_HPP
