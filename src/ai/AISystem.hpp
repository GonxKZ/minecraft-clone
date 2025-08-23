/**
 * @file AISystem.hpp
 * @brief VoxelCraft Advanced AI System
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_AI_AI_SYSTEM_HPP
#define VOXELCRAFT_AI_AI_SYSTEM_HPP

#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
#include <atomic>
#include <mutex>
#include <functional>
#include <queue>

#include "../math/Vec3.hpp"
#include "../entities/Entity.hpp"

namespace VoxelCraft {

    // Forward declarations
    class World;
    class Player;
    class EntityManager;
    class PathfindingSystem;
    class BehaviorTree;
    class UtilityAI;
    class NeuralNetwork;
    class GeneticAlgorithm;
    struct AIConfig;
    struct AIStats;
    struct EntityMemory;
    struct SocialRelationship;

    /**
     * @enum AIType
     * @brief Types of AI behaviors
     */
    enum class AIType {
        PASSIVE,           ///< Peaceful creatures (cows, pigs, etc.)
        NEUTRAL,           ///< Neutral mobs (spiders, endermen)
        AGGRESSIVE,        ///< Hostile mobs (zombies, skeletons)
        BOSS,              ///< Boss enemies (dragon, wither)
        NPC,               ///< Non-player characters (villagers)
        PET,               ///< Tamable pets (wolves, cats)
        MOUNT,             ///< Mountable creatures (horses, llamas)
        CUSTOM             ///< Custom AI behavior
    };

    /**
     * @enum AIState
     * @brief Current state of AI entity
     */
    enum class AIState {
        IDLE,              ///< Doing nothing
        WANDERING,         ///< Random movement
        FOLLOWING,         ///< Following target
        FLEEING,           ///< Fleeing from threat
        ATTACKING,         ///< Attacking target
        HUNTING,           ///< Hunting for food
        BREEDING,          ///< Looking for mate
        EATING,            ///< Eating food
        SLEEPING,          ///< Sleeping/resting
        PATROL,            ///< Patrolling area
        GUARD,             ///< Guarding position
        CUSTOM             ///< Custom state
    };

    /**
     * @enum AISensorType
     * @brief Types of AI sensors
     */
    enum class AISensorType {
        VISION,            ///< Visual detection
        HEARING,           ///< Audio detection
        SMELL,             ///< Scent detection
        TOUCH,             ///< Physical contact
        MAGNETIC,          ///< Magnetic field detection
        ELECTRIC,          ///< Electric field detection
        THERMAL,           ///< Heat detection
        CHEMICAL,          ///< Chemical detection
        CUSTOM             ///< Custom sensor
    };

    /**
     * @struct EntityMemory
     * @brief Memory system for AI entities
     */
    struct EntityMemory {
        struct MemoryEntry {
            std::string targetId;
            Vec3 lastKnownPosition;
            float threatLevel;
            float familiarity;
            uint64_t lastSeenTime;
            uint64_t firstSeenTime;
            std::unordered_map<std::string, std::string> metadata;
        };

        std::unordered_map<std::string, MemoryEntry> memories;
        uint32_t maxMemories = 100;
        float forgetTime = 3600.0f; // 1 hour in seconds
        float memoryDecay = 0.01f;

        void AddMemory(const std::string& targetId, const Vec3& position, float threat, float familiarity);
        void UpdateMemories(float deltaTime);
        void ForgetOldMemories();
        MemoryEntry* GetMemory(const std::string& targetId);
    };

    /**
     * @struct SocialRelationship
     * @brief Social relationships between AI entities
     */
    struct SocialRelationship {
        std::string targetId;
        float trustLevel;
        float fearLevel;
        float respectLevel;
        float loveLevel;
        float hateLevel;
        std::string relationshipType; // "friend", "enemy", "mate", "parent", "child", etc.
        uint64_t lastInteraction;
        std::unordered_map<std::string, float> customAttributes;
    };

    /**
     * @struct AIConfig
     * @brief Configuration for AI system
     */
    struct AIConfig {
        bool enableAI = true;
        bool enablePathfinding = true;
        bool enableMemory = true;
        bool enableSocialBehavior = true;
        bool enableLearning = true;
        bool enableNeuralNetworks = false; // Experimental
        bool enableGeneticEvolution = false; // Experimental

        // Performance settings
        int maxAIEntities = 1000;
        int maxPathfindingQueries = 100;
        float aiUpdateInterval = 1.0f / 30.0f; // 30 FPS AI updates
        float pathfindingUpdateInterval = 1.0f / 10.0f; // 10 FPS pathfinding
        int maxPathLength = 1000;

        // AI behavior settings
        float detectionRange = 32.0f;
        float attackRange = 2.0f;
        float fleeRange = 8.0f;
        float socialRange = 16.0f;
        float memoryTime = 3600.0f;
        float learningRate = 0.01f;

        // Advanced settings
        int neuralNetworkLayers = 3;
        int neuralNetworkNeurons = 64;
        int geneticPopulationSize = 100;
        int geneticGenerations = 10;
    };

    /**
     * @struct AIStats
     * @brief Performance statistics for AI system
     */
    struct AIStats {
        int activeEntities = 0;
        int pathfindingQueries = 0;
        int successfulPaths = 0;
        int failedPaths = 0;
        float averagePathLength = 0.0f;
        float cpuTime = 0.0f;
        float pathfindingTime = 0.0f;
        int memoryEntries = 0;
        int socialRelationships = 0;
        int neuralNetworkEvaluations = 0;
        int geneticGenerations = 0;
    };

    /**
     * @class AISystem
     * @brief Advanced AI system for VoxelCraft
     *
     * Features:
     * - Behavior trees for complex decision making
     * - Utility-based AI for goal selection
     * - Neural networks for learning and adaptation
     * - Genetic algorithms for evolution
     * - Advanced pathfinding with A*, RVO, and navigation meshes
     * - Memory systems for persistent behavior
     * - Social AI with relationships and group behavior
     * - Sensor systems for perception
     * - Emotion and personality systems
     * - Learning and adaptation systems
     * - Multi-threaded AI processing
     * - Performance optimization with spatial partitioning
     * - Debug visualization and profiling
     * - Serialization for save/load
     */
    class AISystem {
    public:
        static AISystem& GetInstance();

        /**
         * @brief Initialize the AI system
         * @param config AI configuration
         * @return true if successful
         */
        bool Initialize(const AIConfig& config);

        /**
         * @brief Shutdown the AI system
         */
        void Shutdown();

        /**
         * @brief Update all AI entities
         * @param deltaTime Time since last update
         */
        void Update(float deltaTime);

        // Entity AI Management
        /**
         * @brief Register entity with AI system
         * @param entity Entity to register
         * @param aiType Type of AI behavior
         * @return true if successful
         */
        bool RegisterEntity(Entity* entity, AIType aiType);

        /**
         * @brief Unregister entity from AI system
         * @param entity Entity to unregister
         * @return true if successful
         */
        bool UnregisterEntity(Entity* entity);

        /**
         * @brief Set AI behavior for entity
         * @param entity Entity
         * @param behaviorTree Behavior tree to use
         * @return true if successful
         */
        bool SetEntityBehavior(Entity* entity, std::shared_ptr<BehaviorTree> behaviorTree);

        /**
         * @brief Get AI state of entity
         * @param entity Entity
         * @return Current AI state
         */
        AIState GetEntityState(Entity* entity) const;

        /**
         * @brief Set AI state of entity
         * @param entity Entity
         * @param state New AI state
         * @return true if successful
         */
        bool SetEntityState(Entity* entity, AIState state);

        // Pathfinding
        /**
         * @brief Find path between two points
         * @param start Starting position
         * @param end Ending position
         * @param path Output path
         * @return true if path found
         */
        bool FindPath(const Vec3& start, const Vec3& end, std::vector<Vec3>& path);

        /**
         * @brief Find path for entity with constraints
         * @param entity Entity finding path
         * @param target Target position
         * @param path Output path
         * @return true if path found
         */
        bool FindEntityPath(Entity* entity, const Vec3& target, std::vector<Vec3>& path);

        /**
         * @brief Check if position is walkable
         * @param position Position to check
         * @return true if walkable
         */
        bool IsWalkable(const Vec3& position) const;

        // Memory System
        /**
         * @brief Get entity memory
         * @param entity Entity
         * @return Entity memory
         */
        EntityMemory* GetEntityMemory(Entity* entity);

        /**
         * @brief Add memory entry for entity
         * @param entity Entity
         * @param targetId Target entity ID
         * @param position Last known position
         * @param threat Threat level
         * @param familiarity Familiarity level
         * @return true if successful
         */
        bool AddEntityMemory(Entity* entity, const std::string& targetId,
                           const Vec3& position, float threat, float familiarity);

        // Social System
        /**
         * @brief Get social relationship between entities
         * @param entity1 First entity
         * @param entity2 Second entity
         * @return Social relationship
         */
        SocialRelationship* GetSocialRelationship(Entity* entity1, Entity* entity2);

        /**
         * @brief Update social relationship
         * @param entity1 First entity
         * @param entity2 Second entity
         * @param relationship Updated relationship
         * @return true if successful
         */
        bool UpdateSocialRelationship(Entity* entity1, Entity* entity2,
                                    const SocialRelationship& relationship);

        // Learning and Adaptation
        /**
         * @brief Enable learning for entity
         * @param entity Entity
         * @param enable Enable learning
         * @return true if successful
         */
        bool EnableEntityLearning(Entity* entity, bool enable);

        /**
         * @brief Train entity on specific behavior
         * @param entity Entity
         * @param behavior Behavior to train
         * @param reward Reward value
         * @return true if successful
         */
        bool TrainEntity(Entity* entity, const std::string& behavior, float reward);

        // Neural Networks (Experimental)
        /**
         * @brief Create neural network for entity
         * @param entity Entity
         * @param inputSize Input layer size
         * @param hiddenSize Hidden layer size
         * @param outputSize Output layer size
         * @return true if successful
         */
        bool CreateEntityNeuralNetwork(Entity* entity, int inputSize, int hiddenSize, int outputSize);

        /**
         * @brief Train neural network
         * @param entity Entity
         * @param inputs Input data
         * @param outputs Expected outputs
         * @param learningRate Learning rate
         * @return true if successful
         */
        bool TrainEntityNeuralNetwork(Entity* entity, const std::vector<float>& inputs,
                                    const std::vector<float>& outputs, float learningRate);

        // Configuration
        /**
         * @brief Set AI configuration
         * @param config New configuration
         */
        void SetConfig(const AIConfig& config) { m_config = config; }

        /**
         * @brief Get current configuration
         * @return Current configuration
         */
        const AIConfig& GetConfig() const { return m_config; }

        // Statistics
        /**
         * @brief Get AI system statistics
         * @return Current statistics
         */
        const AIStats& GetStats() const { return m_stats; }

        /**
         * @brief Reset statistics
         */
        void ResetStats();

        // Debug
        /**
         * @brief Enable debug visualization
         * @param enable Enable state
         */
        void EnableDebugMode(bool enable) { m_debugMode = enable; }

        /**
         * @brief Render debug information
         */
        void RenderDebugInfo();

        /**
         * @brief Get debug information string
         * @return Debug info
         */
        std::string GetDebugInfo() const;

        /**
         * @brief Get entity debug info
         * @param entity Entity
         * @return Debug info for entity
         */
        std::string GetEntityDebugInfo(Entity* entity) const;

    private:
        AISystem() = default;
        ~AISystem();

        // Prevent copying
        AISystem(const AISystem&) = delete;
        AISystem& operator=(const AISystem&) = delete;

        // Core AI processing
        void UpdateEntityAI(float deltaTime);
        void UpdatePathfinding(float deltaTime);
        void UpdateMemorySystems(float deltaTime);
        void UpdateSocialSystems(float deltaTime);
        void UpdateLearningSystems(float deltaTime);

        // AI decision making
        void ProcessBehaviorTrees(float deltaTime);
        void ProcessUtilityAI(float deltaTime);
        void ProcessNeuralNetworks(float deltaTime);
        void ProcessGeneticAlgorithms(float deltaTime);

        // Pathfinding
        void ProcessPathfindingQueue();
        bool AStarPathfinding(const Vec3& start, const Vec3& end, std::vector<Vec3>& path);
        void OptimizePath(std::vector<Vec3>& path);
        void SmoothPath(std::vector<Vec3>& path);

        // Perception and sensors
        void UpdateEntityPerception(Entity* entity);
        bool CanEntitySee(Entity* entity, const Vec3& target);
        bool CanEntityHear(Entity* entity, const Vec3& source);
        bool CanEntityDetect(Entity* entity, const Vec3& target, AISensorType sensorType);

        // Decision making
        void MakeEntityDecision(Entity* entity);
        void EvaluateEntityGoals(Entity* entity);
        void SelectEntityAction(Entity* entity);

        // Utility functions
        float CalculateEntityUtility(Entity* entity, const std::string& action);
        float CalculateThreatLevel(Entity* entity, Entity* threat);
        float CalculateEntityDistance(Entity* entity1, Entity* entity2) const;
        Vec3 GetRandomWanderPosition(Entity* entity) const;

        // Entity management
        void CleanupDeadEntities();
        void OptimizeEntityMemory();
        void UpdateEntityStats();

        // Member variables
        AIConfig m_config;
        bool m_initialized = false;
        bool m_debugMode = false;

        // Entity AI data
        std::unordered_map<Entity*, AIType> m_entityTypes;
        std::unordered_map<Entity*, AIState> m_entityStates;
        std::unordered_map<Entity*, EntityMemory> m_entityMemories;
        std::unordered_map<Entity*, std::shared_ptr<BehaviorTree>> m_behaviorTrees;
        std::unordered_map<Entity*, std::shared_ptr<UtilityAI>> m_utilityAIs;
        std::unordered_map<Entity*, std::shared_ptr<NeuralNetwork>> m_neuralNetworks;

        // Social relationships
        std::unordered_map<std::string, std::unordered_map<std::string, SocialRelationship>> m_socialRelationships;

        // Pathfinding
        std::unique_ptr<PathfindingSystem> m_pathfindingSystem;
        std::queue<std::function<void()>> m_pathfindingQueue;

        // Statistics and performance
        AIStats m_stats;

        // Thread safety
        mutable std::shared_mutex m_aiMutex;
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_AI_AI_SYSTEM_HPP
