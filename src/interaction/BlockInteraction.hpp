/**
 * @file BlockInteraction.hpp
 * @brief VoxelCraft Block Interaction System - Player-World Interaction
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_INTERACTION_BLOCK_INTERACTION_HPP
#define VOXELCRAFT_INTERACTION_BLOCK_INTERACTION_HPP

#include <memory>
#include <functional>

#include "../world/World.hpp"
#include "../player/Player.hpp"
#include "../tools/Tool.hpp"
#include "../blocks/Block.hpp"
#include "../audio/AudioManager.hpp"

namespace VoxelCraft {

    // Forward declarations
    struct Vec3;

    /**
     * @enum InteractionType
     * @brief Type of player-world interaction
     */
    enum class InteractionType {
        DESTROY_BLOCK,     // Left click - destroy block
        PLACE_BLOCK,       // Right click - place block
        USE_ITEM,          // Right click on block - use item
        ATTACK,            // Left click on entity - attack
        INTERACT           // Right click on entity - interact
    };

    /**
     * @enum InteractionResult
     * @brief Result of an interaction attempt
     */
    enum class InteractionResult {
        SUCCESS,           // Interaction completed successfully
        FAILURE,           // Interaction failed
        BLOCKED,           // Interaction blocked by something
        OUT_OF_RANGE,      // Target too far away
        NO_TOOL,           // No appropriate tool available
        INSUFFICIENT_ITEMS, // Not enough items in inventory
        COOLDOWN,          // Action on cooldown
        INVALID_TARGET     // Invalid interaction target
    };

    /**
     * @struct InteractionContext
     * @brief Context information for an interaction
     */
    struct InteractionContext {
        InteractionType type;
        Vec3 playerPosition;
        Vec3 lookDirection;
        float maxDistance;
        std::shared_ptr<Player> player;
        std::shared_ptr<World> world;

        InteractionContext()
            : type(InteractionType::DESTROY_BLOCK)
            , maxDistance(5.0f)
        {}
    };

    /**
     * @struct BlockTarget
     * @brief Information about a targeted block
     */
    struct BlockTarget {
        Vec3 position;        // Block world position
        Vec3 normal;          // Face normal (direction of the face hit)
        BlockType blockType;  // Type of block targeted
        float distance;       // Distance from player to block
        bool valid;           // Whether this target is valid

        BlockTarget()
            : position(0, 0, 0)
            , normal(0, 0, 0)
            , blockType(BlockType::AIR)
            , distance(0.0f)
            , valid(false)
        {}
    };

    /**
     * @struct InteractionConfig
     * @brief Configuration for interaction system
     */
    struct InteractionConfig {
        float maxInteractionDistance;    // Maximum distance for interactions
        float blockPlaceCooldown;        // Cooldown between block placements
        float blockBreakCooldown;        // Cooldown between block breaks
        bool requireLineOfSight;         // Whether line of sight is required
        bool allowCreativePlace;         // Allow placing blocks in creative mode
        bool showInteractionOutline;     // Show outline around targeted block

        InteractionConfig()
            : maxInteractionDistance(5.0f)
            , blockPlaceCooldown(0.1f)
            , blockBreakCooldown(0.2f)
            , requireLineOfSight(true)
            , allowCreativePlace(true)
            , showInteractionOutline(true)
        {}
    };

    /**
     * @typedef InteractionCallback
     * @brief Callback for interaction events
     */
    using InteractionCallback = std::function<void(const InteractionContext&, InteractionResult)>;

    /**
     * @class BlockInteraction
     * @brief Handles all player-world block interactions
     */
    class BlockInteraction {
    public:
        /**
         * @brief Constructor
         * @param world Game world
         * @param player Player entity
         * @param audioManager Audio manager
         * @param config Interaction configuration
         */
        BlockInteraction(std::shared_ptr<World> world,
                        std::shared_ptr<Player> player,
                        std::shared_ptr<AudioManager> audioManager,
                        const InteractionConfig& config = InteractionConfig());

        /**
         * @brief Destructor
         */
        ~BlockInteraction();

        /**
         * @brief Update interaction system
         * @param deltaTime Time since last update
         */
        void Update(float deltaTime);

        /**
         * @brief Attempt to destroy a block
         * @param target Block target information
         * @return Interaction result
         */
        InteractionResult DestroyBlock(const BlockTarget& target);

        /**
         * @brief Attempt to place a block
         * @param target Block target information
         * @return Interaction result
         */
        InteractionResult PlaceBlock(const BlockTarget& target);

        /**
         * @brief Use item on block
         * @param target Block target information
         * @return Interaction result
         */
        InteractionResult UseItem(const BlockTarget& target);

        /**
         * @brief Find targeted block from player perspective
         * @param playerPosition Player eye position
         * @param lookDirection Player look direction
         * @return Block target information
         */
        BlockTarget FindTargetedBlock(const Vec3& playerPosition, const Vec3& lookDirection) const;

        /**
         * @brief Get current block target
         * @return Current block target
         */
        const BlockTarget& GetCurrentTarget() const { return m_currentTarget; }

        /**
         * @brief Check if player can interact with target
         * @param target Block target
         * @return true if interaction is possible
         */
        bool CanInteractWith(const BlockTarget& target) const;

        /**
         * @brief Register interaction callback
         * @param callback Callback function
         * @return Callback ID
         */
        int RegisterCallback(InteractionCallback callback);

        /**
         * @brief Unregister interaction callback
         * @param callbackId Callback ID
         */
        void UnregisterCallback(int callbackId);

        /**
         * @brief Set interaction configuration
         * @param config New configuration
         */
        void SetConfig(const InteractionConfig& config);

        /**
         * @brief Get interaction configuration
         * @return Current configuration
         */
        const InteractionConfig& GetConfig() const { return m_config; }

    private:
        std::shared_ptr<World> m_world;
        std::shared_ptr<Player> m_player;
        std::shared_ptr<AudioManager> m_audioManager;
        InteractionConfig m_config;
        BlockTarget m_currentTarget;

        // Timing and cooldowns
        float m_blockPlaceCooldownTimer;
        float m_blockBreakCooldownTimer;
        float m_lastInteractionTime;

        // Callbacks
        std::vector<InteractionCallback> m_callbacks;
        int m_nextCallbackId;

        // Internal methods

        /**
         * @brief Update current block target
         */
        void UpdateCurrentTarget();

        /**
         * @brief Check if action is on cooldown
         * @param actionType Type of action
         * @return true if on cooldown
         */
        bool IsOnCooldown(InteractionType actionType) const;

        /**
         * @brief Start cooldown for action
         * @param actionType Type of action
         */
        void StartCooldown(InteractionType actionType);

        /**
         * @brief Update cooldown timers
         * @param deltaTime Time since last update
         */
        void UpdateCooldowns(float deltaTime);

        /**
         * @brief Check if player has required tool for block
         * @param blockType Block type
         * @return true if has appropriate tool
         */
        bool HasRequiredTool(BlockType blockType) const;

        /**
         * @brief Calculate block breaking time
         * @param blockType Block type
         * @param tool Tool being used
         * @return Time to break block in seconds
         */
        float CalculateBreakTime(BlockType blockType, const Tool* tool) const;

        /**
         * @brief Get appropriate tool for block
         * @param blockType Block type
         * @return Best tool for the block, or nullptr
         */
        const Tool* GetAppropriateTool(BlockType blockType) const;

        /**
         * @brief Check if placement position is valid
         * @param position Position to place block
         * @param playerPos Player position
         * @return true if valid placement
         */
        bool IsValidPlacement(const Vec3& position, const Vec3& playerPos) const;

        /**
         * @brief Get block placement position from target
         * @param target Block target
         * @return Placement position
         */
        Vec3 GetPlacementPosition(const BlockTarget& target) const;

        /**
         * @brief Notify callbacks of interaction
         * @param context Interaction context
         * @param result Interaction result
         */
        void NotifyCallbacks(const InteractionContext& context, InteractionResult result);

        /**
         * @brief Handle special block interactions
         * @param blockType Block type
         * @param target Block target
         * @return Interaction result
         */
        InteractionResult HandleSpecialInteraction(BlockType blockType, const BlockTarget& target);
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_INTERACTION_BLOCK_INTERACTION_HPP
