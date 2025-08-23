/**
 * @file AIController.hpp
 * @brief VoxelCraft AI Controller - Central AI System for Entities
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_AI_AI_CONTROLLER_HPP
#define VOXELCRAFT_AI_AI_CONTROLLER_HPP

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>
#include <chrono>
#include <glm/glm.hpp>
#include "BehaviorTree.hpp"
#include "Pathfinding.hpp"
#include "PerceptionSystem.hpp"
#include "MemorySystem.hpp"

namespace VoxelCraft {

    class Entity;
    class Player;
    class World;

    /**
     * @enum AIState
     * @brief Current state of AI entity
     */
    enum class AIState {
        IDLE = 0,           ///< Entity is idle
        WANDERING,          ///< Entity is wandering
        PATROLLING,         ///< Entity is patrolling
        CHASING,            ///< Entity is chasing target
        ATTACKING,          ///< Entity is attacking
        FLEEING,            ///< Entity is fleeing
        HUNTING,            ///< Entity is hunting for food
        FEEDING,            ///< Entity is eating
        RESTING,            ///< Entity is resting
        SOCIALIZING,        ///< Entity is socializing
        DEAD                ///< Entity is dead
    };

    /**
     * @enum AIGoal
     * @brief Goals that AI entities can pursue
     */
    enum class AIGoal {
        NONE = 0,
        SURVIVE,            ///< Stay alive, avoid danger
        HUNT,               ///< Hunt for food
        EXPLORE,            ///< Explore the world
        PATROL,             ///< Patrol an area
        GUARD,              ///< Guard a location
        FOLLOW,             ///< Follow another entity
        ATTACK,             ///< Attack enemies
        FLEE,               ///< Flee from danger
        SOCIALIZE,          ///< Interact with other entities
        GATHER,             ///< Gather resources
        BUILD,              ///< Build structures
        LEARN               ///< Learn new behaviors
    };

    /**
     * @enum AIEmotion
     * @brief Emotions that affect AI behavior
     */
    enum class AIEmotion {
        NEUTRAL = 0,
        HAPPY,
        ANGRY,
        FEARFUL,
        SAD,
        CURIOUS,
        HUNGRY,
        TIRED,
        EXCITED,
        BORED
    };

    /**
     * @struct AIStats
     * @brief Statistics for AI entity
     */
    struct AIStats {
        float health = 20.0f;
        float maxHealth = 20.0f;
        float hunger = 20.0f;
        float maxHunger = 20.0f;
        float energy = 100.0f;
        float maxEnergy = 100.0f;
        float fear = 0.0f;
        float aggression = 0.5f;
        float curiosity = 0.5f;
        float intelligence = 1.0f;
        float social = 0.5f;

        // Derived stats
        float movementSpeed = 2.0f;
        float attackDamage = 2.0f;
        float attackSpeed = 1.0f;
        float defense = 1.0f;
        float perceptionRange = 16.0f;
        float memoryCapacity = 1000.0f;
    };

    /**
     * @struct AIControllerConfig
     * @brief Configuration for AI controller
     */
    struct AIControllerConfig {
        std::string aiType = "generic";
        float updateInterval = 0.1f;
        float perceptionRange = 16.0f;
        float movementSpeed = 2.0f;
        float attackRange = 2.0f;
        float fleeThreshold = 0.3f;
        float attackThreshold = 0.7f;
        bool canFly = false;
        bool canSwim = true;
        bool isHostile = false;
        bool isPassive = false;
        bool canSocialize = true;
        int maxPathfindingDistance = 64;
    };

    /**
     * @class AIController
     * @brief Central AI controller for entities
     */
    class AIController {
    public:
        /**
         * @brief Constructor
         * @param entity Owner entity
         * @param config AI configuration
         */
        AIController(Entity* entity, const AIControllerConfig& config);

        /**
         * @brief Destructor
         */
        ~AIController();

        /**
         * @brief Update AI controller
         * @param deltaTime Time since last update
         */
        void Update(float deltaTime);

        /**
         * @brief Initialize AI systems
         * @return true if initialized successfully
         */
        bool Initialize();

        /**
         * @brief Get current AI state
         * @return Current state
         */
        AIState GetCurrentState() const { return m_currentState; }

        /**
         * @brief Set AI state
         * @param state New state
         */
        void SetState(AIState state);

        /**
         * @brief Get current goal
         * @return Current goal
         */
        AIGoal GetCurrentGoal() const { return m_currentGoal; }

        /**
         * @brief Set goal
         * @param goal New goal
         */
        void SetGoal(AIGoal goal);

        /**
         * @brief Get current emotion
         * @return Current emotion
         */
        AIEmotion GetCurrentEmotion() const { return m_currentEmotion; }

        /**
         * @brief Set emotion
         * @param emotion New emotion
         */
        void SetEmotion(AIEmotion emotion);

        /**
         * @brief Get AI statistics
         * @return AI stats
         */
        const AIStats& GetStats() const { return m_stats; }

        /**
         * @brief Get configuration
         * @return AI configuration
         */
        const AIControllerConfig& GetConfig() const { return m_config; }

        /**
         * @brief Set target entity
         * @param target Target entity
         */
        void SetTarget(Entity* target) { m_target = target; }

        /**
         * @brief Get target entity
         * @return Target entity
         */
        Entity* GetTarget() const { return m_target; }

        /**
         * @brief Get behavior tree
         * @return Behavior tree
         */
        BehaviorTree* GetBehaviorTree() const { return m_behaviorTree.get(); }

        /**
         * @brief Get perception system
         * @return Perception system
         */
        PerceptionSystem* GetPerceptionSystem() const { return m_perceptionSystem.get(); }

        /**
         * @brief Get memory system
         * @return Memory system
         */
        MemorySystem* GetMemorySystem() const { return m_memorySystem.get(); }

        /**
         * @brief Get pathfinding system
         * @return Pathfinding system
         */
        Pathfinding* GetPathfinding() const { return m_pathfinding.get(); }

        /**
         * @brief Move to position
         * @param position Target position
         * @return true if movement started
         */
        bool MoveToPosition(const glm::vec3& position);

        /**
         * @brief Attack target
         * @param target Target to attack
         * @return true if attack successful
         */
        bool Attack(Entity* target);

        /**
         * @brief Flee from position
         * @param threatPosition Position to flee from
         * @return true if fleeing started
         */
        bool FleeFrom(const glm::vec3& threatPosition);

        /**
         * @brief Check if can see target
         * @param target Target to check
         * @return true if visible
         */
        bool CanSee(Entity* target) const;

        /**
         * @brief Check if can hear target
         * @param target Target to check
         * @return true if audible
         */
        bool CanHear(Entity* target) const;

        /**
         * @brief Get distance to target
         * @param target Target entity
         * @return Distance or -1 if no target
         */
        float GetDistanceToTarget(Entity* target) const;

        /**
         * @brief Register AI event callback
         * @param callback Callback function
         */
        void RegisterEventCallback(std::function<void(const std::string&, Entity*)> callback);

    private:
        Entity* m_entity;
        AIControllerConfig m_config;
        AIState m_currentState = AIState::IDLE;
        AIGoal m_currentGoal = AIGoal::NONE;
        AIEmotion m_currentEmotion = AIEmotion::NEUTRAL;
        AIStats m_stats;

        // AI Systems
        std::unique_ptr<BehaviorTree> m_behaviorTree;
        std::unique_ptr<PerceptionSystem> m_perceptionSystem;
        std::unique_ptr<MemorySystem> m_memorySystem;
        std::unique_ptr<Pathfinding> m_pathfinding;

        // State
        Entity* m_target = nullptr;
        glm::vec3 m_homePosition = glm::vec3(0.0f);
        std::vector<glm::vec3> m_patrolPoints;
        size_t m_currentPatrolIndex = 0;

        // Timers
        float m_updateTimer = 0.0f;
        float m_stateTimer = 0.0f;
        float m_emotionTimer = 0.0f;
        float m_attackCooldown = 0.0f;

        // Callbacks
        std::vector<std::function<void(const std::string&, Entity*)>> m_eventCallbacks;

        // Pathfinding
        Path m_currentPath;
        glm::vec3 m_moveTarget = glm::vec3(0.0f);

        /**
         * @brief Initialize behavior tree
         */
        void InitializeBehaviorTree();

        /**
         * @brief Initialize perception system
         */
        void InitializePerceptionSystem();

        /**
         * @brief Initialize memory system
         */
        void InitializeMemorySystem();

        /**
         * @brief Update AI state logic
         * @param deltaTime Time since last update
         */
        void UpdateStateLogic(float deltaTime);

        /**
         * @brief Update emotion based on situation
         * @param deltaTime Time since last update
         */
        void UpdateEmotion(float deltaTime);

        /**
         * @brief Update stats based on actions and environment
         * @param deltaTime Time since last update
         */
        void UpdateStats(float deltaTime);

        /**
         * @brief Handle state transitions
         */
        void HandleStateTransitions();

        /**
         * @brief Execute current behavior
         */
        void ExecuteCurrentBehavior();

        /**
         * @brief Find best target for current goal
         * @return Best target entity or nullptr
         */
        Entity* FindBestTarget();

        /**
         * @brief Calculate threat level from entity
         * @param entity Entity to evaluate
         * @return Threat level (0.0 - 1.0)
         */
        float CalculateThreatLevel(Entity* entity) const;

        /**
         * @brief Calculate interest level in entity
         * @param entity Entity to evaluate
         * @return Interest level (0.0 - 1.0)
         */
        float CalculateInterestLevel(Entity* entity) const;

        /**
         * @brief Trigger AI event
         * @param event Event name
         */
        void TriggerEvent(const std::string& event);

        /**
         * @brief Update movement along current path
         * @param deltaTime Time since last update
         */
        void UpdateMovement(float deltaTime);
    };

    /**
     * @class AIControllerFactory
     * @brief Factory for creating AI controllers
     */
    class AIControllerFactory {
    public:
        /**
         * @brief Create AI controller for entity
         * @param entity Entity to create controller for
         * @param aiType Type of AI to create
         * @return AI controller
         */
        static std::unique_ptr<AIController> CreateController(Entity* entity, const std::string& aiType);

        /**
         * @brief Register AI type
         * @param type AI type name
         * @param creator Creator function
         */
        static void RegisterAIType(const std::string& type,
                                  std::function<std::unique_ptr<AIController>(Entity*)> creator);

    private:
        static std::unordered_map<std::string, std::function<std::unique_ptr<AIController>(Entity*)>> s_creators;
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_AI_AI_CONTROLLER_HPP
