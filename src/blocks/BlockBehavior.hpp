/**
 * @file BlockBehavior.hpp
 * @brief VoxelCraft Block Behavior System
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_BLOCKS_BLOCK_BEHAVIOR_HPP
#define VOXELCRAFT_BLOCKS_BLOCK_BEHAVIOR_HPP

#include <string>
#include <memory>
#include <unordered_map>
#include <functional>
#include <any>

namespace VoxelCraft {

    // Forward declarations
    class World;
    class Player;
    class Entity;

    // Basic type definitions
    using BlockID = uint32_t;
    using BlockMetadata = uint16_t;

    /**
     * @enum BehaviorTrigger
     * @brief Types of events that can trigger block behaviors
     */
    enum class BehaviorTrigger {
        ON_PLACE,           ///< When block is placed
        ON_BREAK,           ///< When block is broken
        ON_INTERACT,        ///< When player interacts with block
        ON_NEIGHBOR_CHANGE, ///< When neighboring block changes
        ON_RANDOM_TICK,     ///< Random tick update
        ON_SCHEDULED_TICK,  ///< Scheduled tick update
        ON_ENTITY_COLLIDE,  ///< When entity collides with block
        ON_REDSTONE_UPDATE, ///< When redstone signal changes
        ON_LIGHT_UPDATE,    ///< When light level changes
        ON_WEATHER_CHANGE   ///< When weather changes
    };

    /**
     * @struct BehaviorContext
     * @brief Context information for behavior execution
     */
    struct BehaviorContext {
        World* world;                    ///< Game world
        int x, y, z;                    ///< Block coordinates
        BlockID blockId;                ///< Block ID
        BlockMetadata metadata;         ///< Block metadata
        Player* player;                 ///< Player involved (if any)
        Entity* entity;                 ///< Entity involved (if any)
        BehaviorTrigger trigger;        ///< Trigger type
        double timestamp;               ///< Event timestamp
        std::unordered_map<std::string, std::any> data; ///< Additional data
    };

    /**
     * @class BlockBehavior
     * @brief Base class for block behaviors
     */
    class BlockBehavior {
    public:
        BlockBehavior(const std::string& name);
        virtual ~BlockBehavior() = default;

        /**
         * @brief Get behavior name
         * @return Behavior name
         */
        const std::string& GetName() const { return m_name; }

        /**
         * @brief Execute behavior
         * @param context Behavior context
         * @return true if behavior executed successfully
         */
        virtual bool Execute(const BehaviorContext& context) = 0;

        /**
         * @brief Check if behavior can execute for given trigger
         * @param trigger Behavior trigger
         * @return true if can execute
         */
        virtual bool CanExecute(BehaviorTrigger trigger) const = 0;

        /**
         * @brief Get behavior priority (higher = executes first)
         * @return Priority value
         */
        virtual int GetPriority() const { return 0; }

        /**
         * @brief Check if behavior is enabled
         * @return true if enabled
         */
        bool IsEnabled() const { return m_enabled; }

        /**
         * @brief Set behavior enabled state
         * @param enabled Enabled state
         */
        void SetEnabled(bool enabled) { m_enabled = enabled; }

    protected:
        std::string m_name;    ///< Behavior name
        bool m_enabled;       ///< Enabled flag
    };

    /**
     * @class BlockBehaviorManager
     * @brief Manages block behaviors
     */
    class BlockBehaviorManager {
    public:
        BlockBehaviorManager();
        ~BlockBehaviorManager();

        /**
         * @brief Initialize the behavior manager
         * @return true if successful
         */
        bool Initialize();

        /**
         * @brief Shutdown the behavior manager
         */
        void Shutdown();

        /**
         * @brief Register a block behavior
         * @param behavior Behavior to register
         * @return true if registered successfully
         */
        bool RegisterBehavior(std::unique_ptr<BlockBehavior> behavior);

        /**
         * @brief Unregister a behavior by name
         * @param name Behavior name
         * @return true if unregistered
         */
        bool UnregisterBehavior(const std::string& name);

        /**
         * @brief Execute behaviors for a block
         * @param context Behavior context
         * @return Number of behaviors executed
         */
        size_t ExecuteBehaviors(const BehaviorContext& context);

        /**
         * @brief Get behavior by name
         * @param name Behavior name
         * @return Behavior pointer or nullptr
         */
        BlockBehavior* GetBehavior(const std::string& name) const;

        /**
         * @brief Check if behavior exists
         * @param name Behavior name
         * @return true if exists
         */
        bool HasBehavior(const std::string& name) const;

        /**
         * @brief Get all registered behaviors
         * @return Vector of behavior names
         */
        std::vector<std::string> GetAllBehaviors() const;

    private:
        std::unordered_map<std::string, std::unique_ptr<BlockBehavior>> m_behaviors;
        bool m_initialized;
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_BLOCKS_BLOCK_BEHAVIOR_HPP