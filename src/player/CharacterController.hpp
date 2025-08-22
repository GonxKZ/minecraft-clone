/**
 * @file CharacterController.hpp
 * @brief VoxelCraft Character Controller System
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines the CharacterController class that provides advanced
 * character movement, collision detection, and interaction with the game world.
 */

#ifndef VOXELCRAFT_PLAYER_CHARACTER_CONTROLLER_HPP
#define VOXELCRAFT_PLAYER_CHARACTER_CONTROLLER_HPP

#include <memory>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <atomic>

#include "../core/Config.hpp"
#include "../physics/RigidBody.hpp"
#include "../physics/PhysicsTypes.hpp"
#include "../world/World.hpp"

namespace VoxelCraft {

    // Forward declarations
    class Player;
    class Entity;
    class World;

    /**
     * @enum CharacterState
     * @brief Character movement and action states
     */
    enum class CharacterState {
        Idle,                   ///< Character is idle
        Walking,                ///< Character is walking
        Running,                ///< Character is running
        Jumping,                ///< Character is jumping
        Falling,                ///< Character is falling
        Landing,                ///< Character is landing
        Sneaking,               ///< Character is sneaking
        Swimming,               ///< Character is swimming
        Flying,                 ///< Character is flying
        Climbing,               ///< Character is climbing
        Sliding,                ///< Character is sliding
        Rolling,                ///< Character is rolling
        Attacking,              ///< Character is attacking
        Defending,              ///< Character is defending
        Interacting,            ///< Character is interacting
        Dead,                   ///< Character is dead
        Custom                  ///< Custom character state
    };

    /**
     * @enum MovementFlags
     * @brief Character movement capability flags
     */
    enum class MovementFlags : uint32_t {
        NONE                    = 0x0000,
        CAN_WALK                = 0x0001,      ///< Can walk on ground
        CAN_RUN                 = 0x0002,      ///< Can run
        CAN_JUMP                = 0x0004,      ///< Can jump
        CAN_SNEAK               = 0x0008,      ///< Can sneak/crouch
        CAN_FLY                 = 0x0010,      ///< Can fly
        CAN_SWIM                = 0x0020,      ///< Can swim
        CAN_CLIMB               = 0x0040,      ///< Can climb
        CAN_SLIDE               = 0x0080,      ///< Can slide
        CAN_ROLL                = 0x0100,      ///< Can roll
        CAN_WALL_JUMP           = 0x0200,      ///< Can wall jump
        CAN_DOUBLE_JUMP         = 0x0400,      ///< Can double jump
        CAN_GLIDE               = 0x0800,      ///< Can glide
        CAN_TELEPORT            = 0x1000,      ///< Can teleport
        CAN_DASH                = 0x2000,      ///< Can dash
        ENABLE_GRAVITY          = 0x4000,      ///< Affected by gravity
        ENABLE_FRICTION         = 0x8000       ///< Affected by friction
    };

    /**
     * @enum GroundState
     * @brief Character ground contact state
     */
    enum class GroundState {
        InAir,                  ///< Not touching ground
        OnGround,               ///< Standing on ground
        OnSlope,                ///< On a slope
        Sliding,                ///< Sliding down slope
        OnIce,                  ///< On ice surface
        OnWater,                ///< On water surface
        OnLadder                ///< On ladder
    };

    /**
     * @struct CharacterConfig
     * @brief Character controller configuration
     */
    struct CharacterConfig {
        // Physical properties
        float height;                           ///< Character height
        float radius;                           ///< Character radius
        float mass;                             ///< Character mass
        float stepHeight;                       ///< Maximum step height
        float slopeLimit;                       ///< Maximum slope angle (degrees)

        // Movement speeds
        float walkSpeed;                        ///< Walking speed
        float runSpeed;                         ///< Running speed
        float sneakSpeed;                       ///< Sneaking speed
        float flySpeed;                         ///< Flying speed
        float swimSpeed;                        ///< Swimming speed
        float climbSpeed;                       ///< Climbing speed
        float slideSpeed;                       ///< Sliding speed

        // Jump properties
        float jumpHeight;                       ///< Jump height
        float jumpForce;                        ///< Jump force
        int maxJumpCount;                       ///< Maximum jump count (for double jump)
        float coyoteTime;                       ///< Coyote time (seconds)
        float jumpBufferTime;                   ///< Jump buffer time (seconds)

        // Advanced movement
        float dashDistance;                     ///< Dash distance
        float dashDuration;                     ///< Dash duration (seconds)
        float rollDistance;                     ///< Roll distance
        float rollDuration;                     ///< Roll duration (seconds)
        float wallJumpForce;                    ///< Wall jump force
        float glideSpeed;                       ///< Gliding speed
        float glideDuration;                    ///< Maximum glide duration

        // Collision settings
        float collisionMargin;                  ///< Collision margin
        float groundCheckDistance;              ///< Ground check ray distance
        float wallCheckDistance;                ///< Wall check distance
        int maxCollisionIterations;             ///< Maximum collision resolution iterations

        // Physics settings
        bool enableGravity;                     ///< Enable gravity
        bool enableFriction;                    ///< Enable friction
        float gravityMultiplier;                ///< Gravity multiplier
        float frictionMultiplier;               ///< Friction multiplier
        float airControl;                       ///< Air control factor (0.0 - 1.0)

        // Animation settings
        bool enableAnimations;                  ///< Enable animation system
        float animationBlendTime;               ///< Animation blend time
        bool enableFootsteps;                   ///< Enable footstep sounds
        bool enableLandingEffects;              ///< Enable landing effects

        // Interaction settings
        float interactionRange;                 ///< Maximum interaction range
        float pickupRange;                      ///< Maximum pickup range
        float attackRange;                      ///< Attack range
        float reachDistance;                    ///< Maximum reach distance

        // Debug settings
        bool enableDebugDrawing;                ///< Enable debug visualization
        bool showCollisionShapes;               ///< Show collision shapes
        bool showMovementVectors;               ///< Show movement vectors
        bool showGroundState;                   ///< Show ground state info
    };

    /**
     * @struct CharacterStateData
     * @brief Current character state and movement data
     */
    struct CharacterStateData {
        // Position and movement
        glm::vec3 position;                     ///< Current world position
        glm::vec3 velocity;                     ///< Current velocity
        glm::vec3 acceleration;                 ///< Current acceleration
        glm::vec3 inputDirection;               ///< Input movement direction
        glm::vec3 movementDirection;            ///< Actual movement direction

        // Rotation and orientation
        float yaw;                              ///< Yaw rotation (degrees)
        float pitch;                            ///< Pitch rotation (degrees)
        glm::quat orientation;                  ///< Current orientation
        glm::vec3 forwardDirection;             ///< Forward direction
        glm::vec3 upDirection;                  ///< Up direction
        glm::vec3 rightDirection;               ///< Right direction

        // Ground and collision
        GroundState groundState;                ///< Current ground state
        glm::vec3 groundNormal;                 ///< Ground surface normal
        float groundDistance;                   ///< Distance to ground
        bool isOnGround;                        ///< Is character on ground
        bool isInWater;                         ///< Is character in water
        bool isOnLadder;                        ///< Is character on ladder
        bool isTouchingWall;                    ///< Is touching wall
        glm::vec3 wallNormal;                   ///< Wall surface normal

        // Movement state
        CharacterState characterState;          ///< Current character state
        bool isMoving;                          ///< Is character moving
        bool isJumping;                         ///< Is character jumping
        bool isFalling;                         ///< Is character falling
        bool isSneaking;                        ///< Is character sneaking
        bool isFlying;                          ///< Is character flying
        bool isSwimming;                        ///< Is character swimming

        // Advanced movement
        bool isDashing;                         ///< Is character dashing
        bool isRolling;                         ///< Is character rolling
        bool isGliding;                         ///< Is character gliding
        bool isWallJumping;                     ///< Is wall jumping
        int jumpCount;                          ///< Current jump count
        float dashTimeRemaining;                ///< Dash time remaining
        float rollTimeRemaining;                ///< Roll time remaining
        float glideTimeRemaining;               ///< Glide time remaining

        // Timing
        double currentTime;                     ///< Current time
        double deltaTime;                       ///< Time delta
        double lastGroundTime;                  ///< Last time on ground
        double lastJumpTime;                    ///< Last jump time
        double lastDashTime;                    ///< Last dash time
        double lastRollTime;                    ///< Last roll time
    };

    /**
     * @struct CharacterMetrics
     * @brief Performance and gameplay metrics for character controller
     */
    struct CharacterMetrics {
        // Performance metrics
        uint64_t updateCount;                   ///< Number of updates performed
        double totalUpdateTime;                 ///< Total update time (ms)
        double averageUpdateTime;               ///< Average update time (ms)
        double maxUpdateTime;                   ///< Maximum update time (ms)

        // Movement metrics
        uint64_t jumpsPerformed;                ///< Total jumps performed
        uint64_t fallsTaken;                    ///< Total falls taken
        uint64_t dashesPerformed;               ///< Total dashes performed
        uint64_t rollsPerformed;                ///< Total rolls performed
        float totalDistanceTraveled;            ///< Total distance traveled
        float averageSpeed;                     ///< Average movement speed
        float maxSpeed;                         ///< Maximum speed reached

        // Collision metrics
        uint64_t groundCollisions;              ///< Ground collision events
        uint64_t wallCollisions;                ///< Wall collision events
        uint64_t slopeCollisions;               ///< Slope collision events
        uint64_t stepUpEvents;                  ///< Step-up events
        uint64_t collisionFailures;             ///< Collision resolution failures

        // State metrics
        double totalAirTime;                    ///< Total time in air
        double totalGroundTime;                 ///< Total time on ground
        double totalWaterTime;                  ///< Total time in water
        double totalClimbTime;                  ///< Total time climbing
        double totalFlyTime;                    ///< Total time flying

        // Interaction metrics
        uint64_t interactions;                  ///< Total interactions
        uint64_t pickups;                       ///< Total item pickups
        uint64_t attacks;                       ///< Total attacks performed
        uint64_t hits;                          ///< Total successful hits

        // Error metrics
        uint64_t physicsErrors;                 ///< Physics-related errors
        uint64_t movementErrors;                ///< Movement-related errors
        uint64_t stateErrors;                   ///< State transition errors
        uint64_t collisionErrors;               ///< Collision-related errors
    };

    /**
     * @struct CharacterCollisionData
     * @brief Character collision detection and resolution data
     */
    struct CharacterCollisionData {
        // Collision shapes
        AABB boundingBox;                       ///< Character bounding box
        BoundingSphere collisionSphere;         ///< Collision sphere
        std::vector<AABB> collisionBoxes;       ///< Multiple collision boxes

        // Ground detection
        std::vector<Ray> groundRays;            ///< Ground detection rays
        std::vector<Ray> wallRays;              ///< Wall detection rays
        std::vector<Ray> stepRays;              ///< Step-up detection rays

        // Collision results
        std::vector<ContactPoint> groundContacts; ///< Ground contact points
        std::vector<ContactPoint> wallContacts;  ///< Wall contact points
        ContactManifold groundManifold;         ///< Ground contact manifold

        // Collision state
        bool hasGroundContact;                  ///< Has ground contact
        bool hasWallContact;                    ///< Has wall contact
        bool canStepUp;                         ///< Can step up obstacle
        float penetrationDepth;                 ///< Current penetration depth
        glm::vec3 separationVector;             ///< Separation vector
    };

    /**
     * @class CharacterController
     * @brief Advanced character controller with physics integration
     *
     * The CharacterController provides a sophisticated character movement system
     * with full physics integration, collision detection, and advanced movement
     * capabilities. It handles walking, running, jumping, swimming, flying, and
     * climbing with smooth transitions and realistic physics.
     *
     * Key Features:
     * - Physics-based character movement
     * - Advanced collision detection and resolution
     * - Multiple movement modes (walk, run, sneak, fly, swim, climb)
     * - Jump mechanics (single, double, wall jump)
     * - Slope and stair handling
     * - Swimming and buoyancy simulation
     * - Ladder climbing system
     * - Dash and roll mechanics
     * - Glide and air control
     * - Ground state detection
     * - Collision prediction and avoidance
     * - Smooth animation integration
     *
     * Movement Pipeline:
     * 1. Input processing and state updates
     * 2. Ground state detection
     * 3. Collision detection and prediction
     * 4. Movement force calculation
     * 5. Physics integration
     * 6. Collision resolution
     * 7. Animation state updates
     * 8. Visual effect generation
     */
    class CharacterController {
    public:
        /**
         * @brief Constructor
         * @param config Character controller configuration
         */
        explicit CharacterController(const CharacterConfig& config);

        /**
         * @brief Destructor
         */
        ~CharacterController();

        /**
         * @brief Deleted copy constructor
         */
        CharacterController(const CharacterController&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        CharacterController& operator=(const CharacterController&) = delete;

        // Character controller lifecycle

        /**
         * @brief Initialize character controller
         * @param player Parent player
         * @param rigidBody Player's rigid body
         * @return true if successful, false otherwise
         */
        bool Initialize(Player* player, RigidBody* rigidBody);

        /**
         * @brief Shutdown character controller
         */
        void Shutdown();

        /**
         * @brief Update character controller
         * @param deltaTime Time elapsed since last update
         */
        void Update(double deltaTime);

        /**
         * @brief Render debug information
         */
        void RenderDebug();

        // Configuration and state

        /**
         * @brief Get character configuration
         * @return Current configuration
         */
        const CharacterConfig& GetConfig() const { return m_config; }

        /**
         * @brief Set character configuration
         * @param config New configuration
         */
        void SetConfig(const CharacterConfig& config);

        /**
         * @brief Get character state data
         * @return Current state data
         */
        const CharacterStateData& GetState() const { return m_state; }

        /**
         * @brief Get character metrics
         * @return Performance metrics
         */
        const CharacterMetrics& GetMetrics() const { return m_metrics; }

        // Movement control

        /**
         * @brief Move character
         * @param direction Movement direction (normalized)
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
         * @brief Sneak/crouch
         * @param enable Enable sneaking
         */
        void SetSneaking(bool enable);

        /**
         * @brief Fly
         * @param enable Enable flying
         */
        void SetFlying(bool enable);

        /**
         * @brief Swim
         * @param direction Swim direction
         */
        void Swim(const glm::vec3& direction);

        /**
         * @brief Climb
         * @param direction Climb direction
         */
        void Climb(const glm::vec3& direction);

        /**
         * @brief Dash in direction
         * @param direction Dash direction (normalized)
         * @param distance Dash distance
         * @return true if dash was performed, false otherwise
         */
        bool Dash(const glm::vec3& direction, float distance = 5.0f);

        /**
         * @brief Roll in direction
         * @param direction Roll direction (normalized)
         * @param distance Roll distance
         * @return true if roll was performed, false otherwise
         */
        bool Roll(const glm::vec3& direction, float distance = 3.0f);

        /**
         * @brief Wall jump
         * @return true if wall jump was performed, false otherwise
         */
        bool WallJump();

        /**
         * @brief Glide
         * @param enable Enable gliding
         */
        void SetGliding(bool enable);

        // State queries

        /**
         * @brief Get character state
         * @return Current character state
         */
        CharacterState GetCharacterState() const { return m_state.characterState; }

        /**
         * @brief Get ground state
         * @return Current ground state
         */
        GroundState GetGroundState() const { return m_state.groundState; }

        /**
         * @brief Check if character is on ground
         * @return true if on ground, false otherwise
         */
        bool IsOnGround() const { return m_state.isOnGround; }

        /**
         * @brief Check if character is in air
         * @return true if in air, false otherwise
         */
        bool IsInAir() const { return m_state.groundState == GroundState::InAir; }

        /**
         * @brief Check if character is moving
         * @return true if moving, false otherwise
         */
        bool IsMoving() const { return m_state.isMoving; }

        /**
         * @brief Check if character can jump
         * @return true if can jump, false otherwise
         */
        bool CanJump() const;

        /**
         * @brief Check if character can dash
         * @return true if can dash, false otherwise
         */
        bool CanDash() const;

        /**
         * @brief Check if character can roll
         * @return true if can roll, false otherwise
         */
        bool CanRoll() const;

        /**
         * @brief Check if character can wall jump
         * @return true if can wall jump, false otherwise
         */
        bool CanWallJump() const;

        // Movement properties

        /**
         * @brief Get current velocity
         * @return Current velocity vector
         */
        const glm::vec3& GetVelocity() const { return m_state.velocity; }

        /**
         * @brief Set velocity
         * @param velocity New velocity vector
         */
        void SetVelocity(const glm::vec3& velocity);

        /**
         * @brief Get movement speed for current state
         * @return Current movement speed
         */
        float GetCurrentMovementSpeed() const;

        /**
         * @brief Get jump height
         * @return Current jump height
         */
        float GetJumpHeight() const { return m_config.jumpHeight; }

        /**
         * @brief Set jump height
         * @param height New jump height
         */
        void SetJumpHeight(float height) { m_config.jumpHeight = height; }

        // Collision and physics

        /**
         * @brief Get collision data
         * @return Current collision data
         */
        const CharacterCollisionData& GetCollisionData() const { return m_collisionData; }

        /**
         * @brief Perform ground detection
         * @return true if ground detected, false otherwise
         */
        bool DetectGround();

        /**
         * @brief Perform wall detection
         * @return true if wall detected, false otherwise
         */
        bool DetectWalls();

        /**
         * @brief Check if can step up obstacle
         * @param obstacleHeight Obstacle height
         * @return true if can step up, false otherwise
         */
        bool CanStepUp(float obstacleHeight) const;

        /**
         * @brief Handle collision with environment
         * @param contactPoint Contact point
         * @param normal Surface normal
         */
        void HandleCollision(const glm::vec3& contactPoint, const glm::vec3& normal);

        /**
         * @brief Resolve character penetration
         * @return true if resolved, false otherwise
         */
        bool ResolvePenetration();

        // Movement flags

        /**
         * @brief Check if movement flag is set
         * @param flag Movement flag to check
         * @return true if set, false otherwise
         */
        bool HasMovementFlag(MovementFlags flag) const {
            return (m_movementFlags & static_cast<uint32_t>(flag)) != 0;
        }

        /**
         * @brief Set movement flag
         * @param flag Movement flag to set
         */
        void SetMovementFlag(MovementFlags flag);

        /**
         * @brief Clear movement flag
         * @param flag Movement flag to clear
         */
        void ClearMovementFlag(MovementFlags flag);

        /**
         * @brief Get movement flags
         * @return Current movement flags
         */
        uint32_t GetMovementFlags() const { return m_movementFlags; }

        /**
         * @brief Set movement flags
         * @param flags New movement flags
         */
        void SetMovementFlags(uint32_t flags);

        // World interaction

        /**
         * @brief Get world the character is in
         * @return Character's world
         */
        World* GetWorld() const { return m_world; }

        /**
         * @brief Set world for character
         * @param world New world
         */
        void SetWorld(World* world);

        /**
         * @brief Check if position is valid for character
         * @param position Position to check
         * @return true if valid, false otherwise
         */
        bool IsValidPosition(const glm::vec3& position) const;

        /**
         * @brief Find safe position near current location
         * @param currentPos Current position
         * @param searchRadius Search radius
         * @return Safe position or original position if none found
         */
        glm::vec3 FindSafePosition(const glm::vec3& currentPos, float searchRadius = 5.0f) const;

        // Animation and effects

        /**
         * @brief Get animation state
         * @return Current animation state
         */
        std::string GetAnimationState() const;

        /**
         * @brief Play movement effect
         * @param effectType Effect type
         */
        void PlayMovementEffect(const std::string& effectType);

        /**
         * @brief Update character animation
         * @param deltaTime Time elapsed
         */
        void UpdateAnimation(double deltaTime);

        // Metrics and debugging

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
         * @brief Validate character controller
         * @return true if valid, false otherwise
         */
        bool Validate() const;

        /**
         * @brief Get debug render data
         * @return Debug render data
         */
        const std::vector<DebugRenderData>& GetDebugRenderData() const;

        /**
         * @brief Clear debug render data
         */
        void ClearDebugRenderData();

    private:
        /**
         * @brief Initialize movement systems
         * @return true if successful, false otherwise
         */
        bool InitializeMovementSystems();

        /**
         * @brief Update movement state
         * @param deltaTime Time elapsed
         */
        void UpdateMovementState(double deltaTime);

        /**
         * @brief Update ground state
         * @param deltaTime Time elapsed
         */
        void UpdateGroundState(double deltaTime);

        /**
         * @brief Update collision detection
         * @param deltaTime Time elapsed
         */
        void UpdateCollisionDetection(double deltaTime);

        /**
         * @brief Update movement forces
         * @param deltaTime Time elapsed
         */
        void UpdateMovementForces(double deltaTime);

        /**
         * @brief Apply movement to rigid body
         * @param deltaTime Time elapsed
         */
        void ApplyMovement(double deltaTime);

        /**
         * @brief Handle state transitions
         */
        void HandleStateTransitions();

        /**
         * @brief Calculate movement direction
         * @param inputDirection Input direction
         * @return Movement direction
         */
        glm::vec3 CalculateMovementDirection(const glm::vec3& inputDirection) const;

        /**
         * @brief Calculate movement speed
         * @return Movement speed
         */
        float CalculateMovementSpeed() const;

        /**
         * @brief Apply gravity to character
         * @param deltaTime Time elapsed
         */
        void ApplyGravity(double deltaTime);

        /**
         * @brief Apply friction to character
         * @param deltaTime Time elapsed
         */
        void ApplyFriction(double deltaTime);

        /**
         * @brief Handle jumping mechanics
         * @param deltaTime Time elapsed
         */
        void HandleJumping(double deltaTime);

        /**
         * @brief Handle advanced movement (dash, roll, etc.)
         * @param deltaTime Time elapsed
         */
        void HandleAdvancedMovement(double deltaTime);

        /**
         * @brief Update collision shapes
         */
        void UpdateCollisionShapes();

        /**
         * @brief Update collision rays
         */
        void UpdateCollisionRays();

        /**
         * @brief Perform raycast for ground detection
         * @param ray Ground detection ray
         * @return Raycast hit or empty optional
         */
        std::optional<RaycastHit> PerformGroundRaycast(const Ray& ray) const;

        /**
         * @brief Perform raycast for wall detection
         * @param ray Wall detection ray
         * @return Raycast hit or empty optional
         */
        std::optional<RaycastHit> PerformWallRaycast(const Ray& ray) const;

        /**
         * @brief Update character metrics
         * @param deltaTime Time elapsed
         */
        void UpdateMetrics(double deltaTime);

        /**
         * @brief Collect debug render data
         */
        void CollectDebugRenderData();

        /**
         * @brief Handle character errors
         * @param error Error message
         */
        void HandleCharacterError(const std::string& error);

        // Character data
        CharacterConfig m_config;                      ///< Character configuration
        CharacterStateData m_state;                    ///< Current character state
        CharacterMetrics m_metrics;                    ///< Performance metrics
        CharacterCollisionData m_collisionData;       ///< Collision detection data

        // Associated objects
        Player* m_player;                              ///< Parent player
        RigidBody* m_rigidBody;                        ///< Player's rigid body
        World* m_world;                                ///< Player's world

        // Movement state
        uint32_t m_movementFlags;                      ///< Movement capability flags
        glm::vec3 m_inputDirection;                    ///< Current input direction
        glm::vec3 m_desiredVelocity;                   ///< Desired velocity
        glm::vec3 m_externalForces;                    ///< External forces applied

        // Advanced movement state
        bool m_canDoubleJump;                          ///< Can perform double jump
        bool m_canWallJump;                            ///< Can perform wall jump
        bool m_isCoyoteTimeActive;                     ///< Coyote time is active
        bool m_isJumpBuffered;                         ///< Jump is buffered
        double m_coyoteTimeStart;                      ///< Coyote time start time
        double m_jumpBufferStart;                      ///< Jump buffer start time

        // Collision state
        std::vector<ContactPoint> m_activeContacts;   ///< Active contact points
        mutable std::shared_mutex m_collisionMutex;   ///< Collision synchronization

        // Debug data
        std::vector<DebugRenderData> m_debugRenderData;
        mutable std::shared_mutex m_debugMutex;       ///< Debug synchronization

        // Initialization state
        bool m_isInitialized;                          ///< Character controller initialized
        double m_lastUpdateTime;                       ///< Last update timestamp
        double m_creationTime;                         ///< Character creation timestamp
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_PLAYER_CHARACTER_CONTROLLER_HPP
