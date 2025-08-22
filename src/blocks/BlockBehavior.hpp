/**
 * @file BlockBehavior.hpp
 * @brief VoxelCraft Block Behavior System
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines the BlockBehavior system that allows blocks to have
 * custom interactive behaviors, animations, and dynamic properties.
 */

#ifndef VOXELCRAFT_BLOCKS_BLOCK_BEHAVIOR_HPP
#define VOXELCRAFT_BLOCKS_BLOCK_BEHAVIOR_HPP

#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
#include <functional>
#include <any>
#include <chrono>

#include "../core/Config.hpp"
#include "Block.hpp"
#include "../world/World.hpp"
#include "../entities/Entity.hpp"

namespace VoxelCraft {

    // Forward declarations
    class Block;
    class World;
    class Entity;
    class Player;

    /**
     * @enum BehaviorTrigger
     * @brief Events that can trigger block behaviors
     */
    enum class BehaviorTrigger {
        OnPlace,             ///< Block is placed
        OnBreak,             ///< Block is broken
        OnInteract,          ///< Player interacts with block
        OnRandomTick,        ///< Random tick update
        OnScheduledTick,     ///< Scheduled tick update
        OnNeighborChange,    ///< Neighbor block changes
        OnEntityCollide,     ///< Entity collides with block
        OnWorldLoad,         ///< World containing block is loaded
        OnWorldUnload,       ///< World containing block is unloaded
        OnLightChange,       ///< Light level changes
        OnWeatherChange,     ///< Weather changes
        OnTimeChange,        ///< Time of day changes
        OnRedstoneChange,    ///< Redstone signal changes
        Custom               ///< Custom trigger
    };

    /**
     * @enum BehaviorAction
     * @brief Actions that block behaviors can perform
     */
    enum class BehaviorAction {
        ChangeBlock,         ///< Change this block to another type
        SpawnEntity,         ///< Spawn an entity
        PlaySound,           ///< Play a sound effect
        EmitParticles,       ///< Emit particle effects
        ApplyPhysics,        ///< Apply physics effects
        ModifyLight,         ///< Modify light level
        SendRedstone,        ///< Send redstone signal
        ExecuteCommand,      ///< Execute a command
        TriggerAnimation,    ///< Trigger block animation
        UpdateNeighbors,     ///< Update neighboring blocks
        ScheduleTick,        ///< Schedule a future tick
        Custom               ///< Custom action
    };

    /**
     * @struct BehaviorContext
     * @brief Context information for behavior execution
     */
    struct BehaviorContext {
        World* world;                    ///< World containing the block
        int x, y, z;                    ///< Block coordinates
        BlockID blockId;                ///< Block ID
        BlockMetadata metadata;         ///< Block metadata
        Entity* entity;                 ///< Entity involved (if any)
        Player* player;                 ///< Player involved (if any)
        BehaviorTrigger trigger;        ///< Trigger that caused this behavior
        double currentTime;             ///< Current game time
        std::unordered_map<std::string, std::any> data; ///< Additional context data
    };

    /**
     * @struct BehaviorActionData
     * @brief Data for behavior actions
     */
    struct BehaviorActionData {
        BehaviorAction action;          ///< Action to perform
        std::unordered_map<std::string, std::any> parameters; ///< Action parameters
        double delay;                   ///< Delay before executing (seconds)
        bool cancelable;               ///< Action can be canceled
        std::function<bool(const BehaviorContext&)> condition; ///< Execution condition
    };

    /**
     * @struct BehaviorAnimation
     * @brief Animation data for block behaviors
     */
    struct BehaviorAnimation {
        std::string name;               ///< Animation name
        std::vector<std::string> frames; ///< Animation frame names
        float duration;                 ///< Animation duration (seconds)
        float frameRate;                ///< Frames per second
        bool loop;                      ///< Animation loops
        std::function<void(const BehaviorContext&, float)> updateCallback; ///< Update callback
        std::function<void(const BehaviorContext&)> finishCallback; ///< Finish callback
    };

    /**
     * @struct BehaviorParticleEffect
     * @brief Particle effect data for behaviors
     */
    struct BehaviorParticleEffect {
        std::string particleType;       ///< Type of particle
        glm::vec3 position;             ///< Effect position offset
        glm::vec3 velocity;             ///< Initial velocity
        glm::vec3 acceleration;         ///< Acceleration
        float lifetime;                 ///< Particle lifetime (seconds)
        int count;                      ///< Number of particles
        float spread;                   ///< Random spread factor
        std::unordered_map<std::string, std::any> properties; ///< Particle properties
    };

    /**
     * @struct BlockBehaviorConfig
     * @brief Configuration for block behavior system
     */
    struct BlockBehaviorConfig {
        // Performance settings
        int maxActiveBehaviors;         ///< Maximum active behaviors per block
        int maxBehaviorChainLength;     ///< Maximum behavior chain length
        float maxBehaviorExecutionTime; ///< Maximum execution time per behavior
        bool enableBehaviorCaching;     ///< Enable behavior result caching
        int behaviorCacheSize;          ///< Behavior cache size

        // Execution settings
        bool enableMultithreading;      ///< Enable multithreaded behavior execution
        int behaviorWorkerThreads;      ///< Number of behavior worker threads
        bool enableBehaviorPriorities;  ///< Enable behavior priorities
        float maxBehaviorDelay;         ///< Maximum behavior delay (seconds)

        // Debugging settings
        bool enableBehaviorLogging;     ///< Enable behavior execution logging
        bool enableBehaviorProfiling;   ///< Enable behavior profiling
        bool enableBehaviorValidation;  ///< Enable behavior validation
    };

    /**
     * @struct BlockBehaviorMetrics
     * @brief Performance metrics for block behaviors
     */
    struct BlockBehaviorMetrics {
        uint64_t totalBehaviorsExecuted;    ///< Total behaviors executed
        uint64_t activeBehaviors;           ///< Currently active behaviors
        double averageExecutionTime;        ///< Average execution time (ms)
        double maxExecutionTime;            ///< Maximum execution time (ms)
        uint64_t behaviorChainsExecuted;    ///< Behavior chains executed
        uint64_t delayedBehaviors;          ///< Delayed behaviors scheduled
        uint64_t cachedBehaviors;           ///< Cached behavior results used
        uint64_t behaviorErrors;            ///< Behavior execution errors
        size_t memoryUsage;                 ///< Memory usage (bytes)
        double behaviorLoad;                ///< Behavior system load (0.0 - 1.0)
    };

    /**
     * @class BlockBehavior
     * @brief Base class for block behaviors
     *
     * BlockBehavior provides the interface for creating custom block behaviors.
     * Behaviors can respond to various triggers and perform different actions.
     *
     * Key Features:
     * - Multiple trigger types (place, break, interact, etc.)
     * - Action chaining and conditional execution
     * - Animation and particle effects
     * - Delayed execution and scheduling
     * - Performance monitoring and validation
     */
    class BlockBehavior {
    public:
        /**
         * @brief Constructor
         * @param name Behavior name
         */
        explicit BlockBehavior(const std::string& name);

        /**
         * @brief Destructor
         */
        virtual ~BlockBehavior() = default;

        /**
         * @brief Deleted copy constructor
         */
        BlockBehavior(const BlockBehavior&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        BlockBehavior& operator=(const BlockBehavior&) = delete;

        // Behavior identification

        /**
         * @brief Get behavior name
         * @return Behavior name
         */
        const std::string& GetName() const { return m_name; }

        /**
         * @brief Get behavior description
         * @return Behavior description
         */
        virtual std::string GetDescription() const { return "Block behavior"; }

        /**
         * @brief Check if behavior is enabled
         * @return true if enabled, false otherwise
         */
        bool IsEnabled() const { return m_enabled; }

        /**
         * @brief Enable/disable behavior
         * @param enabled Enable state
         */
        void SetEnabled(bool enabled) { m_enabled = enabled; }

        // Trigger management

        /**
         * @brief Add trigger to behavior
         * @param trigger Trigger type
         * @param priority Trigger priority
         */
        void AddTrigger(BehaviorTrigger trigger, int priority = 0);

        /**
         * @brief Remove trigger from behavior
         * @param trigger Trigger type
         */
        void RemoveTrigger(BehaviorTrigger trigger);

        /**
         * @brief Check if behavior responds to trigger
         * @param trigger Trigger type
         * @return true if responds, false otherwise
         */
        bool HasTrigger(BehaviorTrigger trigger) const;

        /**
         * @brief Get trigger priority
         * @param trigger Trigger type
         * @return Priority or 0 if not found
         */
        int GetTriggerPriority(BehaviorTrigger trigger) const;

        // Behavior execution

        /**
         * @brief Execute behavior
         * @param context Behavior context
         * @return true if successful, false otherwise
         */
        virtual bool Execute(const BehaviorContext& context) = 0;

        /**
         * @brief Check if behavior can execute in context
         * @param context Behavior context
         * @return true if can execute, false otherwise
         */
        virtual bool CanExecute(const BehaviorContext& context) const;

        /**
         * @brief Get behavior priority
         * @return Priority value
         */
        virtual int GetPriority() const { return 0; }

        /**
         * @brief Get behavior execution cost estimate
         * @return Cost estimate (0.0 - 1.0)
         */
        virtual float GetExecutionCost() const { return 0.1f; }

        // Action management

        /**
         * @brief Add action to behavior
         * @param action Action data
         */
        void AddAction(const BehaviorActionData& action);

        /**
         * @brief Remove action from behavior
         * @param actionType Action type
         */
        void RemoveAction(BehaviorAction actionType);

        /**
         * @brief Get actions for behavior
         * @return Vector of action data
         */
        const std::vector<BehaviorActionData>& GetActions() const { return m_actions; }

        // Animation management

        /**
         * @brief Add animation to behavior
         * @param animation Animation data
         */
        void AddAnimation(const BehaviorAnimation& animation);

        /**
         * @brief Remove animation from behavior
         * @param name Animation name
         */
        void RemoveAnimation(const std::string& name);

        /**
         * @brief Get animation by name
         * @param name Animation name
         * @return Animation data or empty optional
         */
        std::optional<BehaviorAnimation> GetAnimation(const std::string& name) const;

        // Particle effects

        /**
         * @brief Add particle effect to behavior
         * @param effect Particle effect data
         */
        void AddParticleEffect(const BehaviorParticleEffect& effect);

        /**
         * @brief Remove particle effect from behavior
         * @param particleType Particle type
         */
        void RemoveParticleEffect(const std::string& particleType);

        // Properties

        /**
         * @brief Set behavior property
         * @tparam T Property type
         * @param key Property key
         * @param value Property value
         */
        template<typename T>
        void SetProperty(const std::string& key, const T& value);

        /**
         * @brief Get behavior property
         * @tparam T Property type
         * @param key Property key
         * @param defaultValue Default value
         * @return Property value or default
         */
        template<typename T>
        T GetProperty(const std::string& key, const T& defaultValue = T{}) const;

        /**
         * @brief Check if behavior has property
         * @param key Property key
         * @return true if has property, false otherwise
         */
        bool HasProperty(const std::string& key) const;

        // Validation

        /**
         * @brief Validate behavior configuration
         * @return Vector of validation errors
         */
        virtual std::vector<std::string> Validate() const;

        /**
         * @brief Get behavior dependencies
         * @return Vector of required behavior names
         */
        virtual std::vector<std::string> GetDependencies() const { return {}; }

    protected:
        /**
         * @brief Execute actions for behavior
         * @param context Behavior context
         * @return Number of actions executed
         */
        size_t ExecuteActions(const BehaviorContext& context);

        /**
         * @brief Start animation
         * @param context Behavior context
         * @param animationName Animation name
         * @return true if started, false otherwise
         */
        bool StartAnimation(const BehaviorContext& context, const std::string& animationName);

        /**
         * @brief Emit particle effects
         * @param context Behavior context
         * @return Number of effects emitted
         */
        size_t EmitParticleEffects(const BehaviorContext& context);

        std::string m_name;                                       ///< Behavior name
        bool m_enabled;                                          ///< Behavior enabled flag

        std::unordered_map<BehaviorTrigger, int> m_triggers;     ///< Trigger priorities
        std::vector<BehaviorActionData> m_actions;              ///< Behavior actions
        std::unordered_map<std::string, BehaviorAnimation> m_animations; ///< Animations
        std::vector<BehaviorParticleEffect> m_particleEffects;  ///< Particle effects
        std::unordered_map<std::string, std::any> m_properties; ///< Custom properties
    };

    // Template implementations

    template<typename T>
    void BlockBehavior::SetProperty(const std::string& key, const T& value) {
        m_properties[key] = value;
    }

    template<typename T>
    T BlockBehavior::GetProperty(const std::string& key, const T& defaultValue) const {
        auto it = m_properties.find(key);
        if (it != m_properties.end()) {
            try {
                return std::any_cast<T>(it->second);
            } catch (const std::bad_any_cast&) {
                return defaultValue;
            }
        }
        return defaultValue;
    }

    /**
     * @class InteractiveBlockBehavior
     * @brief Behavior for interactive blocks (doors, levers, buttons, etc.)
     */
    class InteractiveBlockBehavior : public BlockBehavior {
    public:
        explicit InteractiveBlockBehavior(const std::string& name);

        /**
         * @brief Get interaction type
         * @return Interaction type string
         */
        virtual std::string GetInteractionType() const { return "default"; }

        /**
         * @brief Check if block can be interacted with
         * @param context Behavior context
         * @return true if can interact, false otherwise
         */
        virtual bool CanInteract(const BehaviorContext& context) const;

        /**
         * @brief Handle interaction
         * @param context Behavior context
         * @return true if interaction handled, false otherwise
         */
        virtual bool HandleInteraction(const BehaviorContext& context) = 0;

        /**
         * @brief Get interaction cooldown
         * @return Cooldown in seconds
         */
        virtual float GetInteractionCooldown() const { return 0.0f; }

        /**
         * @brief Get maximum interaction distance
         * @return Maximum distance
         */
        virtual float GetMaxInteractionDistance() const { return 5.0f; }

    protected:
        /**
         * @brief Check interaction cooldown
         * @param context Behavior context
         * @return true if cooldown active, false otherwise
         */
        bool IsOnCooldown(const BehaviorContext& context) const;

        /**
         * @brief Start interaction cooldown
         * @param context Behavior context
         */
        void StartCooldown(const BehaviorContext& context);

        std::unordered_map<std::string, double> m_lastInteractionTimes; ///< Last interaction times
    };

    /**
     * @class RedstoneBlockBehavior
     * @brief Behavior for redstone components
     */
    class RedstoneBlockBehavior : public BlockBehavior {
    public:
        explicit RedstoneBlockBehavior(const std::string& name);

        /**
         * @brief Get redstone signal strength
         * @param context Behavior context
         * @return Signal strength (0-15)
         */
        virtual int GetSignalStrength(const BehaviorContext& context) const { return 0; }

        /**
         * @brief Set redstone signal strength
         * @param context Behavior context
         * @param strength Signal strength (0-15)
         */
        virtual void SetSignalStrength(const BehaviorContext& context, int strength) {}

        /**
         * @brief Check if block is powered
         * @param context Behavior context
         * @return true if powered, false otherwise
         */
        virtual bool IsPowered(const BehaviorContext& context) const { return false; }

        /**
         * @brief Handle redstone signal change
         * @param context Behavior context
         * @param oldStrength Old signal strength
         * @param newStrength New signal strength
         */
        virtual void OnSignalChanged(const BehaviorContext& context, int oldStrength, int newStrength) {}

        /**
         * @brief Get redstone update delay
         * @return Delay in ticks
         */
        virtual int GetUpdateDelay() const { return 1; }
    };

    /**
     * @class AnimatedBlockBehavior
     * @brief Behavior for animated blocks
     */
    class AnimatedBlockBehavior : public BlockBehavior {
    public:
        explicit AnimatedBlockBehavior(const std::string& name);

        /**
         * @brief Update animation
         * @param context Behavior context
         * @param deltaTime Time elapsed
         */
        virtual void UpdateAnimation(const BehaviorContext& context, double deltaTime) {}

        /**
         * @brief Start animation
         * @param context Behavior context
         * @param animationName Animation name
         * @return true if started, false otherwise
         */
        virtual bool StartAnimation(const BehaviorContext& context, const std::string& animationName);

        /**
         * @brief Stop animation
         * @param context Behavior context
         */
        virtual void StopAnimation(const BehaviorContext& context);

        /**
         * @brief Get current animation frame
         * @param context Behavior context
         * @return Current frame index
         */
        virtual int GetCurrentFrame(const BehaviorContext& context) const { return 0; }

        /**
         * @brief Get animation speed multiplier
         * @return Speed multiplier
         */
        virtual float GetAnimationSpeed() const { return 1.0f; }

    protected:
        /**
         * @brief Animation state for each block instance
         */
        struct AnimationState {
            std::string currentAnimation;
            float currentTime;
            int currentFrame;
            bool isPlaying;
            bool isLooping;
        };

        std::unordered_map<std::string, AnimationState> m_animationStates; ///< Animation states
    };

    /**
     * @class BlockBehaviorManager
     * @brief Manager for block behaviors
     */
    class BlockBehaviorManager {
    public:
        /**
         * @brief Constructor
         * @param config Behavior configuration
         */
        explicit BlockBehaviorManager(const BlockBehaviorConfig& config);

        /**
         * @brief Destructor
         */
        ~BlockBehaviorManager();

        /**
         * @brief Deleted copy constructor
         */
        BlockBehaviorManager(const BlockBehaviorManager&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        BlockBehaviorManager& operator=(const BlockBehaviorManager&) = delete;

        // Behavior registration

        /**
         * @brief Register block behavior
         * @param behavior Behavior to register
         * @return true if registered, false if name exists
         */
        bool RegisterBehavior(std::unique_ptr<BlockBehavior> behavior);

        /**
         * @brief Register behavior for specific block
         * @param blockId Block ID
         * @param behaviorName Behavior name
         * @return true if registered, false otherwise
         */
        bool RegisterBehaviorForBlock(BlockID blockId, const std::string& behaviorName);

        /**
         * @brief Unregister behavior
         * @param name Behavior name
         * @return true if unregistered, false if not found
         */
        bool UnregisterBehavior(const std::string& name);

        /**
         * @brief Get behavior by name
         * @param name Behavior name
         * @return Behavior pointer or nullptr if not found
         */
        BlockBehavior* GetBehavior(const std::string& name) const;

        // Behavior execution

        /**
         * @brief Trigger behaviors for block
         * @param trigger Trigger type
         * @param context Behavior context
         * @return Number of behaviors executed
         */
        size_t TriggerBehaviors(BehaviorTrigger trigger, const BehaviorContext& context);

        /**
         * @brief Execute specific behavior
         * @param behaviorName Behavior name
         * @param context Behavior context
         * @return true if executed, false otherwise
         */
        bool ExecuteBehavior(const std::string& behaviorName, const BehaviorContext& context);

        /**
         * @brief Update behavior system
         * @param deltaTime Time elapsed
         */
        void Update(double deltaTime);

        // Configuration

        /**
         * @brief Get behavior configuration
         * @return Current configuration
         */
        const BlockBehaviorConfig& GetConfig() const { return m_config; }

        /**
         * @brief Set behavior configuration
         * @param config New configuration
         */
        void SetConfig(const BlockBehaviorConfig& config);

        /**
         * @brief Get behavior metrics
         * @return Performance metrics
         */
        const BlockBehaviorMetrics& GetMetrics() const { return m_metrics; }

        /**
         * @brief Reset metrics
         */
        void ResetMetrics();

        // Utilities

        /**
         * @brief Get behaviors for block
         * @param blockId Block ID
         * @return Vector of behavior names
         */
        std::vector<std::string> GetBehaviorsForBlock(BlockID blockId) const;

        /**
         * @brief Check if block has behavior
         * @param blockId Block ID
         * @param behaviorName Behavior name
         * @return true if has behavior, false otherwise
         */
        bool BlockHasBehavior(BlockID blockId, const std::string& behaviorName) const;

        /**
         * @brief Get all registered behaviors
         * @return Vector of behavior names
         */
        std::vector<std::string> GetAllBehaviors() const;

        /**
         * @brief Validate all behaviors
         * @return Vector of validation errors
         */
        std::vector<std::string> ValidateAllBehaviors() const;

    private:
        /**
         * @brief Execute behavior with error handling
         * @param behavior Behavior to execute
         * @param context Behavior context
         * @return true if successful, false otherwise
         */
        bool ExecuteBehaviorInternal(BlockBehavior* behavior, const BehaviorContext& context);

        /**
         * @brief Update behavior metrics
         * @param behavior Behavior that was executed
         * @param executionTime Time taken to execute
         * @param success Execution success flag
         */
        void UpdateMetrics(BlockBehavior* behavior, double executionTime, bool success);

        /**
         * @brief Process delayed behavior actions
         */
        void ProcessDelayedActions();

        /**
         * @brief Cleanup completed behaviors
         */
        void CleanupCompletedBehaviors();

        // Configuration and state
        BlockBehaviorConfig m_config;                        ///< Behavior configuration
        BlockBehaviorMetrics m_metrics;                      ///< Performance metrics

        // Behavior storage
        std::unordered_map<std::string, std::unique_ptr<BlockBehavior>> m_behaviors; ///< Registered behaviors
        std::unordered_map<BlockID, std::vector<std::string>> m_blockBehaviors; ///< Behaviors per block

        // Delayed actions
        struct DelayedAction {
            std::string behaviorName;
            BehaviorContext context;
            BehaviorActionData action;
            double executeTime;
        };
        std::vector<DelayedAction> m_delayedActions;        ///< Delayed actions queue

        // Threading and synchronization
        mutable std::shared_mutex m_behaviorsMutex;         ///< Behavior synchronization
        mutable std::shared_mutex m_metricsMutex;           ///< Metrics synchronization
    };

    /**
     * @class BlockBehaviorFactory
     * @brief Factory for creating block behaviors
     */
    class BlockBehaviorFactory {
    public:
        /**
         * @brief Create door behavior
         * @param name Behavior name
         * @return Door behavior instance
         */
        static std::unique_ptr<InteractiveBlockBehavior> CreateDoorBehavior(const std::string& name);

        /**
         * @brief Create button behavior
         * @param name Behavior name
         * @return Button behavior instance
         */
        static std::unique_ptr<InteractiveBlockBehavior> CreateButtonBehavior(const std::string& name);

        /**
         * @brief Create lever behavior
         * @param name Behavior name
         * @return Lever behavior instance
         */
        static std::unique_ptr<InteractiveBlockBehavior> CreateLeverBehavior(const std::string& name);

        /**
         * @brief Create pressure plate behavior
         * @param name Behavior name
         * @return Pressure plate behavior instance
         */
        static std::unique_ptr<BlockBehavior> CreatePressurePlateBehavior(const std::string& name);

        /**
         * @brief Create redstone torch behavior
         * @param name Behavior name
         * @return Redstone torch behavior instance
         */
        static std::unique_ptr<RedstoneBlockBehavior> CreateRedstoneTorchBehavior(const std::string& name);

        /**
         * @brief Create animated texture behavior
         * @param name Behavior name
         * @param frameCount Number of animation frames
         * @return Animated behavior instance
         */
        static std::unique_ptr<AnimatedBlockBehavior> CreateAnimatedBehavior(const std::string& name, int frameCount);

        /**
         * @brief Create custom behavior from configuration
         * @param name Behavior name
         * @param config Behavior configuration
         * @return Custom behavior instance
         */
        static std::unique_ptr<BlockBehavior> CreateCustomBehavior(
            const std::string& name,
            const std::unordered_map<std::string, std::any>& config);
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_BLOCKS_BLOCK_BEHAVIOR_HPP
