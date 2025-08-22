/**
 * @file Mob.hpp
 * @brief VoxelCraft Mob System - AI Entities
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines the Mob class that represents AI-controlled entities
 * in the VoxelCraft game world with advanced behaviors, pathfinding,
 * combat systems, and interaction capabilities.
 */

#ifndef VOXELCRAFT_AI_MOB_HPP
#define VOXELCRAFT_AI_MOB_HPP

#include <memory>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <any>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../entities/Entity.hpp"
#include "../physics/RigidBody.hpp"
#include "../ai/BehaviorTree.hpp"
#include "../ai/PathfindingSystem.hpp"
#include "../world/World.hpp"

namespace VoxelCraft {

    // Forward declarations
    class AISystem;
    class Blackboard;
    class SensorySystem;
    class BehaviorTree;
    class PathfindingSystem;

    /**
     * @enum MobState
     * @brief Current state of the mob
     */
    enum class MobState {
        Idle,                   ///< Mob is idle
        Patrolling,             ///< Mob is patrolling
        Investigating,          ///< Mob is investigating something
        Chasing,                ///< Mob is chasing a target
        Attacking,              ///< Mob is attacking
        Fleeing,                ///< Mob is fleeing
        Feeding,                ///< Mob is feeding/eating
        Resting,                ///< Mob is resting
        Dead,                   ///< Mob is dead
        Custom                 ///< Custom mob state
    };

    /**
     * @enum MobType
     * @brief Types of mobs available
     */
    enum class MobType {
        Passive,                ///< Passive mob (sheep, cow, etc.)
        Neutral,                ///< Neutral mob (pig, chicken, etc.)
        Hostile,                ///< Hostile mob (zombie, creeper, etc.)
        Boss,                   ///< Boss mob
        NPC,                    ///< Non-player character
        Custom                 ///< Custom mob type
    };

    /**
     * @enum MobBehavior
     * @brief Behavior flags for mobs
     */
    enum class MobBehavior : uint32_t {
        NONE                    = 0x0000,
        CAN_MOVE                = 0x0001,      ///< Can move around
        CAN_JUMP                = 0x0002,      ///< Can jump
        CAN_FLY                 = 0x0004,      ///< Can fly
        CAN_SWIM                = 0x0008,      ///< Can swim
        CAN_CLIMB               = 0x0010,      ///< Can climb
        CAN_ATTACK              = 0x0020,      ///< Can attack
        CAN_BE_ATTACKED         = 0x0040,      ///< Can be attacked
        CAN_BREED               = 0x0080,      ///< Can breed
        CAN_TAME                = 0x0100,      ///< Can be tamed
        CAN_SHEAR               = 0x0200,      ///< Can be sheared
        CAN_MILK                = 0x0400,      ///< Can be milked
        DROPS_ITEMS             = 0x0800,      ///< Drops items when killed
        HAS_INVENTORY           = 0x1000,      ///< Has inventory
        USES_EQUIPMENT          = 0x2000,      ///< Uses equipment
        HAS_SENSES              = 0x4000,      ///< Has sensory capabilities
        HAS_MEMORY              = 0x8000       ///< Has memory system
    };

    /**
     * @enum MobSense
     * @brief Sensory capabilities of mobs
     */
    enum class MobSense {
        Sight,                  ///< Visual perception
        Hearing,                ///< Auditory perception
        Smell,                  ///< Olfactory perception
        Touch,                  ///< Tactile perception
        Vibration,              ///< Vibration detection
        Custom                 ///< Custom sense
    };

    /**
     * @struct MobConfig
     * @brief Configuration for mob entities
     */
    struct MobConfig {
        // Basic properties
        std::string name;                       ///< Mob display name
        std::string typeName;                   ///< Mob type identifier
        MobType mobType;                        ///< Type of mob
        uint32_t behaviorFlags;                 ///< Behavior flags

        // Physical properties
        float width;                            ///< Mob width
        float height;                           ///< Mob height
        float mass;                             ///< Mob mass
        float maxHealth;                        ///< Maximum health
        float movementSpeed;                    ///< Base movement speed
        float jumpForce;                        ///< Jump force
        bool canFly;                            ///< Can fly
        bool canSwim;                           ///< Can swim

        // Combat properties
        float attackDamage;                     ///< Base attack damage
        float attackRange;                      ///< Attack range
        float attackCooldown;                   ///< Attack cooldown (seconds)
        float defense;                          ///< Defense value
        float armor;                            ///< Armor value

        // AI properties
        float sightRange;                       ///< Visual range
        float hearingRange;                     ///< Hearing range
        float smellRange;                       ///< Smell range
        float memorySpan;                       ///< Memory duration (seconds)
        float reactionTime;                     ///< Reaction time (seconds)
        int aggressionLevel;                    ///< Aggression level (0-10)

        // Survival properties
        float hungerRate;                       ///< Hunger increase rate
        float thirstRate;                       ///< Thirst increase rate
        float tirednessRate;                    ///< Tiredness increase rate
        float maxHunger;                        ///< Maximum hunger level
        float maxThirst;                        ///< Maximum thirst level
        float maxTiredness;                     ///< Maximum tiredness level

        // Social properties
        bool isSocial;                          ///< Is social creature
        float socialDistance;                   ///< Preferred social distance
        int maxGroupSize;                       ///< Maximum group size
        bool canLeadGroups;                     ///< Can lead groups

        // Reproduction properties
        bool canBreed;                          ///< Can breed
        float breedingCooldown;                 ///< Breeding cooldown (seconds)
        int maxOffspring;                       ///< Maximum offspring
        float matingRange;                      ///< Mating detection range

        // Loot properties
        std::unordered_map<std::string, float> lootTable; ///< Item drop chances
        float experienceDrop;                   ///< Experience dropped when killed
        bool dropsItemsOnDeath;                 ///< Drops items when killed

        // Visual properties
        std::string modelPath;                  ///< 3D model path
        std::string texturePath;                ///< Texture path
        std::string animationPath;              ///< Animation data path
        float scale;                            ///< Model scale
        bool hasCustomRendering;                ///< Has custom rendering

        // Audio properties
        std::string ambientSound;               ///< Ambient sound
        std::string hurtSound;                  ///< Hurt sound
        std::string deathSound;                 ///< Death sound
        std::string attackSound;                ///< Attack sound
        float soundVolume;                      ///< Sound volume multiplier

        // Advanced properties
        std::unordered_map<std::string, std::any> customProperties; ///< Custom properties
        std::vector<std::string> tags;          ///< Mob tags for categorization
        bool enableDebugLogging;                ///< Enable debug logging
        int spawnWeight;                        ///< Spawn weight for world generation
    };

    /**
     * @struct MobStats
     * @brief Dynamic statistics for mob entities
     */
    struct MobStats {
        // Health and damage
        float currentHealth;                    ///< Current health
        float maxHealth;                        ///< Maximum health
        float healthRegeneration;               ///< Health regeneration rate
        bool isInvulnerable;                    ///< Currently invulnerable
        double lastDamageTime;                  ///< Last time damaged
        double invulnerabilityDuration;         ///< Invulnerability duration

        // Combat stats
        float attackDamage;                     ///< Current attack damage
        float attackSpeed;                      ///< Current attack speed
        int killCount;                          ///< Number of kills
        int deathCount;                         ///< Number of deaths
        float totalDamageDealt;                 ///< Total damage dealt
        float totalDamageReceived;              ///< Total damage received

        // Movement stats
        float currentSpeed;                     ///< Current movement speed
        float maxSpeed;                         ///< Maximum speed reached
        float totalDistanceTraveled;            ///< Total distance traveled
        int jumpsPerformed;                     ///< Number of jumps
        double lastJumpTime;                    ///< Last jump time

        // Survival stats
        float hunger;                           ///< Current hunger level
        float thirst;                           ///< Current thirst level
        float tiredness;                        ///< Current tiredness level
        bool isHungry;                          ///< Is currently hungry
        bool isThirsty;                         ///< Is currently thirsty
        bool isTired;                           ///< Is currently tired

        // Social stats
        int groupSize;                          ///< Current group size
        bool isGroupLeader;                     ///< Is group leader
        Entity* groupLeader;                    ///< Group leader entity
        std::vector<Entity*> groupMembers;      ///< Group members
        float socialSatisfaction;               ///< Social satisfaction level

        // AI stats
        int decisionsMade;                      ///< Number of AI decisions
        int pathsFound;                         ///< Number of paths found
        int pathsFailed;                        ///< Number of pathfinding failures
        float averagePathLength;                ///< Average path length
        int stimuliProcessed;                   ///< Number of stimuli processed

        // Reproduction stats
        bool canBreed;                          ///< Can currently breed
        double lastBreedingTime;                ///< Last breeding time
        int offspringCount;                     ///< Number of offspring
        Entity* breedingPartner;                ///< Current breeding partner

        // Environmental stats
        bool isInWater;                         ///< Currently in water
        bool isOnFire;                          ///< Currently on fire
        bool isFrozen;                          ///< Currently frozen
        float temperature;                      ///< Current body temperature
        float wetness;                          ///< Current wetness level
    };

    /**
     * @struct MobMemory
     * @brief Memory system for mob entities
     */
    struct MobMemory {
        // Location memory
        std::vector<glm::vec3> knownLocations;  ///< Known interesting locations
        std::unordered_map<std::string, glm::vec3> locationTags; ///< Tagged locations
        glm::vec3 homeLocation;                 ///< Home location
        glm::vec3 lastKnownPlayerPosition;      ///< Last known player position

        // Entity memory
        std::unordered_map<uint32_t, Entity*> knownEntities; ///< Known entities
        std::vector<uint32_t> friendlyEntities; ///< Friendly entity IDs
        std::vector<uint32_t> hostileEntities;  ///< Hostile entity IDs
        std::vector<uint32_t> fearedEntities;   ///< Feared entity IDs

        // Event memory
        std::vector<std::string> recentEvents;  ///< Recent events
        std::unordered_map<std::string, double> eventTimestamps; ///< Event timestamps
        std::vector<glm::vec3> dangerLocations; ///< Dangerous locations

        // Item memory
        std::unordered_map<std::string, int> knownItems; ///< Known item types
        std::vector<glm::vec3> foodSources;   ///< Known food source locations
        std::vector<glm::vec3> waterSources;  ///< Known water source locations

        // Temporal memory
        double memoryDuration;                 ///< How long memories last
        double lastMemoryUpdate;               ///< Last memory update time
        int maxMemories;                       ///< Maximum number of memories
    };

    /**
     * @struct MobSenses
     * @brief Sensory information for mob entities
     */
    struct MobSenses {
        // Visual senses
        std::vector<Entity*> visibleEntities;  ///< Currently visible entities
        std::vector<glm::vec3> visibleLocations; ///< Visible interesting locations
        float currentSightRange;               ///< Current sight range
        bool hasLineOfSight;                   ///< Has line of sight to target

        // Auditory senses
        std::vector<std::pair<glm::vec3, float>> heardSounds; ///< Heard sounds (position, intensity)
        float currentHearingRange;             ///< Current hearing range
        glm::vec3 lastHeardSoundPosition;      ///< Last heard sound position

        // Olfactory senses
        std::vector<std::pair<glm::vec3, float>> smelledScents; ///< Smelled scents
        float currentSmellRange;               ///< Current smell range
        std::string dominantScent;             ///< Currently dominant scent

        // Tactile senses
        std::vector<Entity*> touchingEntities; ///< Entities currently touching
        bool isBeingTouched;                   ///< Is being touched by something
        float touchPressure;                   ///< Current touch pressure

        // Vibration senses
        std::vector<glm::vec3> feltVibrations; ///< Felt vibrations
        float vibrationIntensity;              ///< Current vibration intensity

        // Processing
        double lastSenseUpdate;                ///< Last sense update time
        float senseUpdateFrequency;            ///< Sense update frequency
        bool sensesActive;                     ///< Senses are currently active
    };

    /**
     * @struct MobMetrics
     * @brief Performance metrics for mob entities
     */
    struct MobMetrics {
        // Performance metrics
        uint64_t updateCount;                  ///< Number of updates performed
        double totalUpdateTime;                ///< Total update time (ms)
        double averageUpdateTime;              ///< Average update time (ms)
        double maxUpdateTime;                  ///< Maximum update time (ms)

        // AI metrics
        uint64_t behaviorExecutions;           ///< Behavior tree executions
        uint64_t pathfindingRequests;          ///< Pathfinding requests made
        uint64_t sensoryUpdates;               ///< Sensory system updates
        uint64_t decisionCount;                ///< Number of decisions made

        // Movement metrics
        float totalDistanceTraveled;           ///< Total distance traveled
        int jumpsPerformed;                    ///< Number of jumps
        int pathFollowingSuccess;              ///< Successful path following
        int pathFollowingFailures;             ///< Path following failures

        // Combat metrics
        int attacksPerformed;                  ///< Attacks performed
        int attacksHit;                        ///< Successful attacks
        int damageDealt;                       ///< Total damage dealt
        int damageReceived;                    ///< Total damage received

        // Survival metrics
        int foodConsumed;                      ///< Food items consumed
        int waterDrank;                        ///< Water consumed
        int restPeriods;                       ///< Rest periods taken
        float averageHunger;                   ///< Average hunger level

        // Social metrics
        int interactions;                      ///< Social interactions
        int groupJoins;                        ///< Group joins
        int groupLeaves;                       ///< Group leaves
        float socialTime;                      ///< Time spent in social situations
    };

    /**
     * @class Mob
     * @brief AI-controlled entity with advanced behaviors
     *
     * The Mob class represents AI-controlled entities in the VoxelCraft game world.
     * Mobs have complex behaviors, sensory systems, memory, pathfinding capabilities,
     * and can interact with the environment and other entities in sophisticated ways.
     *
     * Key Features:
     * - Advanced AI behavior system with behavior trees
     * - Multi-sensory perception (sight, hearing, smell, touch)
     * - Memory system for learning and adaptation
     * - Pathfinding and navigation capabilities
     * - Combat system with different attack types
     * - Survival mechanics (hunger, thirst, tiredness)
     * - Social behaviors and group dynamics
     * - Reproduction system
     * - Dynamic stat system with buffs/debuffs
     * - Customizable behaviors and properties
     *
     * Mobs can range from simple passive creatures to complex boss entities
     * with unique behaviors and abilities.
     */
    class Mob : public Entity {
    public:
        /**
         * @brief Constructor
         * @param config Mob configuration
         */
        explicit Mob(const MobConfig& config);

        /**
         * @brief Destructor
         */
        ~Mob() override;

        /**
         * @brief Deleted copy constructor
         */
        Mob(const Mob&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        Mob& operator=(const Mob&) = delete;

        // Entity interface implementation

        /**
         * @brief Update mob entity
         * @param deltaTime Time elapsed since last update
         */
        void Update(double deltaTime) override;

        /**
         * @brief Render mob entity
         */
        void Render() override;

        /**
         * @brief Get entity type
         * @return Entity type
         */
        EntityType GetType() const override { return EntityType::Mob; }

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

        // Mob identification and configuration

        /**
         * @brief Get mob name
         * @return Mob display name
         */
        const std::string& GetName() const { return m_config.name; }

        /**
         * @brief Set mob name
         * @param name New mob name
         */
        void SetName(const std::string& name) { m_config.name = name; }

        /**
         * @brief Get mob type
         * @return Mob type
         */
        MobType GetMobType() const { return m_config.mobType; }

        /**
         * @brief Get mob configuration
         * @return Mob configuration
         */
        const MobConfig& GetConfig() const { return m_config; }

        /**
         * @brief Set mob configuration
         * @param config New configuration
         */
        void SetConfig(const MobConfig& config);

        // Mob state and stats

        /**
         * @brief Get current mob state
         * @return Mob state
         */
        MobState GetState() const { return m_state; }

        /**
         * @brief Set mob state
         * @param state New mob state
         */
        void SetState(MobState state);

        /**
         * @brief Get mob stats
         * @return Mob statistics
         */
        const MobStats& GetStats() const { return m_stats; }

        /**
         * @brief Get current health
         * @return Current health
         */
        float GetHealth() const { return m_stats.currentHealth; }

        /**
         * @brief Set health
         * @param health New health value
         */
        void SetHealth(float health);

        /**
         * @brief Get maximum health
         * @return Maximum health
         */
        float GetMaxHealth() const { return m_stats.maxHealth; }

        /**
         * @brief Check if mob is alive
         * @return true if alive, false otherwise
         */
        bool IsAlive() const { return m_stats.currentHealth > 0.0f; }

        /**
         * @brief Check if mob is dead
         * @return true if dead, false otherwise
         */
        bool IsDead() const { return m_stats.currentHealth <= 0.0f; }

        /**
         * @brief Damage mob
         * @param damage Amount of damage
         * @param damageType Type of damage
         * @param attacker Entity that caused damage
         * @return true if damage was applied, false otherwise
         */
        bool TakeDamage(float damage, const std::string& damageType = "generic", Entity* attacker = nullptr);

        /**
         * @brief Heal mob
         * @param healAmount Amount to heal
         * @return true if healing was applied, false otherwise
         */
        bool Heal(float healAmount);

        /**
         * @brief Kill mob
         * @param killer Entity that killed the mob
         */
        void Kill(Entity* killer = nullptr);

        // Behavior and AI

        /**
         * @brief Get behavior tree
         * @return Behavior tree instance
         */
        BehaviorTree* GetBehaviorTree() const { return m_behaviorTree.get(); }

        /**
         * @brief Set behavior tree
         * @param behaviorTree New behavior tree
         * @return true if set, false otherwise
         */
        bool SetBehaviorTree(std::shared_ptr<BehaviorTree> behaviorTree);

        /**
         * @brief Get blackboard
         * @return Blackboard instance
         */
        Blackboard* GetBlackboard() const { return m_blackboard.get(); }

        /**
         * @brief Get AI system
         * @return AI system instance
         */
        AISystem* GetAISystem() const { return m_aiSystem; }

        /**
         * @brief Set AI system
         * @param aiSystem New AI system
         */
        void SetAISystem(AISystem* aiSystem) { m_aiSystem = aiSystem; }

        // Movement and navigation

        /**
         * @brief Move mob to position
         * @param position Target position
         * @param speed Movement speed
         * @return true if movement started, false otherwise
         */
        bool MoveTo(const glm::vec3& position, float speed = -1.0f);

        /**
         * @brief Follow entity
         * @param target Entity to follow
         * @param distance Following distance
         * @return true if following started, false otherwise
         */
        bool Follow(Entity* target, float distance = 3.0f);

        /**
         * @brief Stop movement
         */
        void StopMovement();

        /**
         * @brief Check if mob is moving
         * @return true if moving, false otherwise
         */
        bool IsMoving() const;

        /**
         * @brief Get current path
         * @return Current path or empty vector
         */
        const std::vector<glm::vec3>& GetCurrentPath() const { return m_currentPath; }

        /**
         * @brief Get pathfinding system
         * @return Pathfinding system instance
         */
        PathfindingSystem* GetPathfindingSystem() const;

        // Combat and interaction

        /**
         * @brief Attack target
         * @param target Entity to attack
         * @return true if attack started, false otherwise
         */
        bool Attack(Entity* target);

        /**
         * @brief Stop attacking
         */
        void StopAttack();

        /**
         * @brief Check if mob is attacking
         * @return true if attacking, false otherwise
         */
        bool IsAttacking() const;

        /**
         * @brief Get current attack target
         * @return Attack target or nullptr
         */
        Entity* GetAttackTarget() const { return m_attackTarget; }

        /**
         * @brief Interact with entity
         * @param target Entity to interact with
         * @return true if interaction occurred, false otherwise
         */
        bool Interact(Entity* target);

        // Senses and perception

        /**
         * @brief Get mob senses
         * @return Mob sensory information
         */
        const MobSenses& GetSenses() const { return m_senses; }

        /**
         * @brief Check if mob can see entity
         * @param entity Entity to check
         * @return true if visible, false otherwise
         */
        bool CanSee(Entity* entity) const;

        /**
         * @brief Check if mob can hear position
         * @param position Sound position
         * @param intensity Sound intensity
         * @return true if audible, false otherwise
         */
        bool CanHear(const glm::vec3& position, float intensity = 1.0f) const;

        /**
         * @brief Check if mob can smell position
         * @param position Scent position
         * @param intensity Scent intensity
         * @return true if smellable, false otherwise
         */
        bool CanSmell(const glm::vec3& position, float intensity = 1.0f) const;

        /**
         * @brief Update senses
         * @param deltaTime Time elapsed
         */
        void UpdateSenses(double deltaTime);

        // Memory system

        /**
         * @brief Get mob memory
         * @return Mob memory information
         */
        const MobMemory& GetMemory() const { return m_memory; }

        /**
         * @brief Remember location
         * @param position Position to remember
         * @param tag Location tag
         */
        void RememberLocation(const glm::vec3& position, const std::string& tag = "");

        /**
         * @brief Remember entity
         * @param entity Entity to remember
         * @param relationship Relationship type
         */
        void RememberEntity(Entity* entity, const std::string& relationship);

        /**
         * @brief Forget memory
         * @param memoryType Type of memory to forget
         */
        void Forget(const std::string& memoryType);

        /**
         * @brief Clear all memories
         */
        void ClearMemory();

        // Behavior flags

        /**
         * @brief Check if mob has behavior flag
         * @param flag Behavior flag to check
         * @return true if has flag, false otherwise
         */
        bool HasBehaviorFlag(MobBehavior flag) const {
            return (m_config.behaviorFlags & static_cast<uint32_t>(flag)) != 0;
        }

        /**
         * @brief Set behavior flag
         * @param flag Behavior flag to set
         */
        void SetBehaviorFlag(MobBehavior flag);

        /**
         * @brief Clear behavior flag
         * @param flag Behavior flag to clear
         */
        void ClearBehaviorFlag(MobBehavior flag);

        /**
         * @brief Get behavior flags
         * @return Current behavior flags
         */
        uint32_t GetBehaviorFlags() const { return m_config.behaviorFlags; }

        /**
         * @brief Set behavior flags
         * @param flags New behavior flags
         */
        void SetBehaviorFlags(uint32_t flags);

        // Survival mechanics

        /**
         * @brief Feed mob
         * @param foodAmount Amount of food
         * @return true if fed, false otherwise
         */
        bool Feed(float foodAmount);

        /**
         * @brief Give water to mob
         * @param waterAmount Amount of water
         * @return true if given water, false otherwise
         */
        bool GiveWater(float waterAmount);

        /**
         * @brief Rest mob
         * @param restAmount Amount of rest
         * @return true if rested, false otherwise
         */
        bool Rest(float restAmount);

        /**
         * @brief Check if mob needs food
         * @return true if hungry, false otherwise
         */
        bool NeedsFood() const { return m_stats.hunger > m_config.maxHunger * 0.7f; }

        /**
         * @brief Check if mob needs water
         * @return true if thirsty, false otherwise
         */
        bool NeedsWater() const { return m_stats.thirst > m_config.maxThirst * 0.7f; }

        /**
         * @brief Check if mob needs rest
         * @return true if tired, false otherwise
         */
        bool NeedsRest() const { return m_stats.tiredness > m_config.maxTiredness * 0.7f; }

        // Social behaviors

        /**
         * @brief Join group with leader
         * @param leader Group leader
         * @return true if joined, false otherwise
         */
        bool JoinGroup(Entity* leader);

        /**
         * @brief Leave current group
         */
        void LeaveGroup();

        /**
         * @brief Check if mob is in group
         * @return true if in group, false otherwise
         */
        bool IsInGroup() const { return m_stats.groupLeader != nullptr; }

        /**
         * @brief Get group leader
         * @return Group leader or nullptr
         */
        Entity* GetGroupLeader() const { return m_stats.groupLeader; }

        /**
         * @brief Get group members
         * @return Vector of group members
         */
        const std::vector<Entity*>& GetGroupMembers() const { return m_stats.groupMembers; }

        // Reproduction

        /**
         * @brief Breed with partner
         * @param partner Breeding partner
         * @return true if breeding successful, false otherwise
         */
        bool Breed(Entity* partner);

        /**
         * @brief Check if mob can breed
         * @return true if can breed, false otherwise
         */
        bool CanBreed() const;

        /**
         * @brief Get breeding partner
         * @return Breeding partner or nullptr
         */
        Entity* GetBreedingPartner() const { return m_stats.breedingPartner; }

        // Physics and collision

        /**
         * @brief Get rigid body
         * @return Rigid body instance
         */
        RigidBody* GetRigidBody() const { return m_rigidBody; }

        /**
         * @brief Set rigid body
         * @param rigidBody New rigid body
         */
        void SetRigidBody(RigidBody* rigidBody) { m_rigidBody = rigidBody; }

        // World integration

        /**
         * @brief Get world the mob is in
         * @return World instance
         */
        World* GetWorld() const { return m_world; }

        /**
         * @brief Set world for mob
         * @param world New world
         */
        void SetWorld(World* world);

        // Metrics and debugging

        /**
         * @brief Get mob metrics
         * @return Performance metrics
         */
        const MobMetrics& GetMetrics() const { return m_metrics; }

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
         * @brief Validate mob state
         * @return true if valid, false otherwise
         */
        bool Validate() const;

        // Event system

        /**
         * @brief Add mob event listener
         * @param eventType Event type
         * @param listener Listener function
         */
        void AddEventListener(const std::string& eventType, std::function<void(const std::string&)> listener);

        /**
         * @brief Remove mob event listener
         * @param eventType Event type
         */
        void RemoveEventListener(const std::string& eventType);

        /**
         * @brief Trigger mob event
         * @param eventType Event type
         * @param eventData Event data
         */
        void TriggerEvent(const std::string& eventType, const std::string& eventData = "");

    private:
        /**
         * @brief Initialize mob subsystems
         * @return true if successful, false otherwise
         */
        bool InitializeSubsystems();

        /**
         * @brief Update mob logic
         * @param deltaTime Time elapsed
         */
        void UpdateLogic(double deltaTime);

        /**
         * @brief Update mob physics
         * @param deltaTime Time elapsed
         */
        void UpdatePhysics(double deltaTime);

        /**
         * @brief Update mob AI
         * @param deltaTime Time elapsed
         */
        void UpdateAI(double deltaTime);

        /**
         * @brief Update mob survival
         * @param deltaTime Time elapsed
         */
        void UpdateSurvival(double deltaTime);

        /**
         * @brief Update mob animation
         * @param deltaTime Time elapsed
         */
        void UpdateAnimation(double deltaTime);

        /**
         * @brief Handle mob death
         * @param killer Entity that killed the mob
         */
        void HandleDeath(Entity* killer = nullptr);

        /**
         * @brief Handle mob damage
         * @param damage Amount of damage
         * @param damageType Type of damage
         * @param attacker Entity that caused damage
         */
        void HandleDamage(float damage, const std::string& damageType, Entity* attacker);

        /**
         * @brief Handle mob healing
         * @param healAmount Amount healed
         */
        void HandleHealing(float healAmount);

        /**
         * @brief Update mob metrics
         * @param deltaTime Time elapsed
         */
        void UpdateMetrics(double deltaTime);

        /**
         * @brief Handle mob errors
         * @param error Error message
         */
        void HandleMobError(const std::string& error);

        // Mob data
        uint32_t m_mobId;                                   ///< Unique mob ID
        MobConfig m_config;                                 ///< Mob configuration
        MobStats m_stats;                                   ///< Mob statistics
        MobState m_state;                                   ///< Current mob state
        MobMemory m_memory;                                 ///< Mob memory system
        MobSenses m_senses;                                 ///< Mob sensory system
        MobMetrics m_metrics;                               ///< Performance metrics

        // AI systems
        std::shared_ptr<BehaviorTree> m_behaviorTree;       ///< Behavior tree
        std::unique_ptr<Blackboard> m_blackboard;           ///< Blackboard system
        AISystem* m_aiSystem;                               ///< AI system reference

        // Movement and navigation
        std::vector<glm::vec3> m_currentPath;              ///< Current navigation path
        size_t m_pathIndex;                                 ///< Current path index
        glm::vec3 m_targetPosition;                         ///< Current target position
        Entity* m_followTarget;                             ///< Entity being followed
        float m_followDistance;                             ///< Following distance

        // Combat system
        Entity* m_attackTarget;                             ///< Current attack target
        double m_lastAttackTime;                            ///< Last attack time
        double m_attackCooldownEnd;                         ///< Attack cooldown end time
        bool m_isAttacking;                                 ///< Attack in progress flag

        // Physics integration
        RigidBody* m_rigidBody;                             ///< Mob rigid body
        World* m_world;                                     ///< Mob's world

        // Event system
        std::unordered_map<std::string, std::vector<std::function<void(const std::string&)>>> m_eventListeners;
        mutable std::shared_mutex m_eventMutex;             ///< Event synchronization

        // State flags
        bool m_isInitialized;                               ///< Mob is initialized
        bool m_isDead;                                      ///< Mob is dead
        bool m_needsRespawn;                                ///< Mob needs respawn
        double m_creationTime;                              ///< Mob creation timestamp
        double m_lastUpdateTime;                            ///< Last update timestamp

        static std::atomic<uint32_t> s_nextMobId;          ///< Next mob ID counter
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_AI_MOB_HPP
