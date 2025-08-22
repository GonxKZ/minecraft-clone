/**
 * @file BlockSystem.hpp
 * @brief VoxelCraft Block System Main Coordinator
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines the BlockSystem class that coordinates all block-related
 * functionality in the voxel world, including block types, behaviors, rendering,
 * and physics interactions.
 */

#ifndef VOXELCRAFT_BLOCKS_BLOCK_SYSTEM_HPP
#define VOXELCRAFT_BLOCKS_BLOCK_SYSTEM_HPP

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
#include "../world/World.hpp"
#include "Block.hpp"
#include "BlockRegistry.hpp"
#include "BlockMeshGenerator.hpp"
#include "TextureAtlas.hpp"
#include "BlockBehavior.hpp"

namespace VoxelCraft {

    // Forward declarations
    class World;
    class Chunk;
    class Entity;
    class Player;
    class BlockMeshGenerator;
    class TextureAtlas;
    class BlockRegistry;
    class BlockBehaviorManager;

    /**
     * @struct BlockSystemConfig
     * @brief Configuration for the block system
     */
    struct BlockSystemConfig {
        // Core settings
        size_t maxBlockTypes;                   ///< Maximum number of block types
        size_t maxBlockBehaviors;               ///< Maximum number of block behaviors
        bool enableCustomBlocks;                ///< Enable custom block types
        bool enableBlockOverrides;              ///< Enable block type overrides

        // Performance settings
        bool enableMultithreading;              ///< Enable multithreaded block operations
        int blockWorkerThreads;                 ///< Number of block worker threads
        size_t maxPendingBlockUpdates;          ///< Maximum pending block updates
        float blockUpdateInterval;              ///< Block update interval (seconds)

        // Rendering settings
        bool enableAdvancedRendering;           ///< Enable advanced block rendering
        bool enableMeshOptimization;            ///< Enable mesh optimization
        bool enableTextureStreaming;            ///< Enable texture streaming
        int maxTextureSize;                     ///< Maximum texture size

        // Physics settings
        bool enableBlockPhysics;                ///< Enable block physics
        bool enableBlockCollisions;             ///< Enable block collisions
        bool enableBlockDestruction;            ///< Enable block destruction
        float blockHardnessMultiplier;          ///< Block hardness multiplier

        // Behavior settings
        bool enableBlockBehaviors;              ///< Enable block behaviors
        bool enableInteractiveBlocks;           ///< Enable interactive blocks
        bool enableAnimatedBlocks;              ///< Enable animated blocks
        int maxBehaviorChainLength;             ///< Maximum behavior chain length

        // Memory settings
        size_t maxBlockMemoryUsage;             ///< Maximum memory usage (MB)
        bool enableBlockCompression;            ///< Enable block data compression
        bool enableBlockCaching;                ///< Enable block caching
        size_t blockCacheSize;                  ///< Block cache size

        // Debug settings
        bool enableBlockDebugging;              ///< Enable block debugging
        bool showBlockInfo;                     ///< Show block information
        bool enableBlockValidation;             ///< Enable block validation
    };

    /**
     * @struct BlockSystemMetrics
     * @brief Performance metrics for the block system
     */
    struct BlockSystemMetrics {
        // Block metrics
        uint64_t totalBlocks;                   ///< Total blocks in world
        uint32_t activeBlockTypes;              ///< Number of active block types
        uint32_t loadedBlockTypes;              ///< Number of loaded block types
        uint64_t blockUpdates;                  ///< Total block updates performed

        // Performance metrics
        double averageBlockUpdateTime;          ///< Average block update time (ms)
        double averageMeshGenerationTime;       ///< Average mesh generation time (ms)
        double averageTextureLoadTime;          ///< Average texture load time (ms)
        uint64_t meshGenerations;               ///< Total mesh generations
        uint64_t textureLoads;                  ///< Total texture loads

        // Memory metrics
        size_t blockMemoryUsage;                ///< Block system memory usage
        size_t textureMemoryUsage;              ///< Texture memory usage
        size_t meshMemoryUsage;                 ///< Mesh memory usage
        size_t behaviorMemoryUsage;             ///< Behavior memory usage

        // System metrics
        uint32_t activeBehaviors;               ///< Number of active behaviors
        uint32_t pendingUpdates;                ///< Pending block updates
        uint32_t activeThreads;                 ///< Active worker threads
        double systemLoad;                      ///< Block system load (0.0 - 1.0)

        // Error metrics
        uint64_t blockLoadErrors;               ///< Block loading errors
        uint64_t behaviorExecutionErrors;       ///< Behavior execution errors
        uint64_t meshGenerationErrors;          ///< Mesh generation errors
        uint64_t textureLoadErrors;             ///< Texture loading errors
    };

    /**
     * @struct BlockUpdateInfo
     * @brief Information about a block update
     */
    struct BlockUpdateInfo {
        int x, y, z;                            ///< Block coordinates
        BlockID oldBlockId;                     ///< Previous block ID
        BlockMetadata oldMetadata;              ///< Previous metadata
        BlockID newBlockId;                     ///< New block ID
        BlockMetadata newMetadata;              ///< New metadata
        double timestamp;                       ///< Update timestamp
        std::string source;                     ///< Update source
        int priority;                           ///< Update priority
    };

    /**
     * @struct BlockInteractionInfo
     * @brief Information about a block interaction
     */
    struct BlockInteractionInfo {
        int x, y, z;                            ///< Block coordinates
        BlockID blockId;                        ///< Block ID
        BlockMetadata metadata;                 ///< Block metadata
        Player* player;                         ///< Player who interacted
        std::string interactionType;            ///< Type of interaction
        double timestamp;                       ///< Interaction timestamp
        std::unordered_map<std::string, std::any> data; ///< Interaction data
    };

    /**
     * @class BlockSystem
     * @brief Main block system coordinator
     *
     * The BlockSystem is the central hub for all block-related functionality:
     * - Block type management and registration
     * - Block behavior coordination
     * - Block mesh generation and rendering
     * - Texture atlas management
     * - Block physics and collision detection
     * - Block interaction handling
     * - Performance optimization and caching
     *
     * Key Features:
     * - Unified interface for all block operations
     * - Advanced caching and memory management
     * - Multi-threaded block processing
     * - Real-time block updates and synchronization
     * - Comprehensive debugging and profiling
     */
    class BlockSystem {
    public:
        /**
         * @brief Constructor
         * @param config Block system configuration
         */
        explicit BlockSystem(const BlockSystemConfig& config);

        /**
         * @brief Destructor
         */
        ~BlockSystem();

        /**
         * @brief Deleted copy constructor
         */
        BlockSystem(const BlockSystem&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        BlockSystem& operator=(const BlockSystem&) = delete;

        // System lifecycle

        /**
         * @brief Initialize the block system
         * @param world Parent world
         * @return true if successful, false otherwise
         */
        bool Initialize(World* world);

        /**
         * @brief Shutdown the block system
         */
        void Shutdown();

        /**
         * @brief Update block system
         * @param deltaTime Time elapsed since last update
         */
        void Update(double deltaTime);

        // Block operations

        /**
         * @brief Get block at world coordinates
         * @param world World instance
         * @param x World X coordinate
         * @param y World Y coordinate
         * @param z World Z coordinate
         * @return Block ID or 0 if not found
         */
        uint32_t GetBlock(World* world, int x, int y, int z) const;

        /**
         * @brief Set block at world coordinates
         * @param world World instance
         * @param x World X coordinate
         * @param y World Y coordinate
         * @param z World Z coordinate
         * @param blockId Block ID to set
         * @param metadata Block metadata
         * @param player Player who placed the block (optional)
         * @return true if successful, false otherwise
         */
        bool SetBlock(World* world, int x, int y, int z, uint32_t blockId,
                     BlockMetadata metadata = 0, Player* player = nullptr);

        /**
         * @brief Get block metadata
         * @param world World instance
         * @param x World X coordinate
         * @param y World Y coordinate
         * @param z World Z coordinate
         * @return Block metadata
         */
        BlockMetadata GetBlockMetadata(World* world, int x, int y, int z) const;

        /**
         * @brief Set block metadata
         * @param world World instance
         * @param x World X coordinate
         * @param y World Y coordinate
         * @param z World Z coordinate
         * @param metadata New metadata
         * @return true if successful, false otherwise
         */
        bool SetBlockMetadata(World* world, int x, int y, int z, BlockMetadata metadata);

        // Block information

        /**
         * @brief Get block by ID
         * @param blockId Block ID
         * @return Block pointer or nullptr if not found
         */
        Block* GetBlockById(BlockID blockId) const;

        /**
         * @brief Get block by name
         * @param name Block name
         * @return Block pointer or nullptr if not found
         */
        Block* GetBlockByName(const std::string& name) const;

        /**
         * @brief Get block name by ID
         * @param blockId Block ID
         * @return Block name or empty string if not found
         */
        std::string GetBlockName(BlockID blockId) const;

        /**
         * @brief Get block ID by name
         * @param name Block name
         * @return Block ID or 0 if not found
         */
        BlockID GetBlockId(const std::string& name) const;

        /**
         * @brief Check if block exists
         * @param blockId Block ID
         * @return true if exists, false otherwise
         */
        bool BlockExists(BlockID blockId) const;

        /**
         * @brief Check if block name exists
         * @param name Block name
         * @return true if exists, false otherwise
         */
        bool BlockNameExists(const std::string& name) const;

        // Block registration

        /**
         * @brief Register a new block type
         * @param block Block to register
         * @return true if registered, false otherwise
         */
        bool RegisterBlock(std::unique_ptr<Block> block);

        /**
         * @brief Unregister a block type
         * @param blockId Block ID to unregister
         * @return true if unregistered, false otherwise
         */
        bool UnregisterBlock(BlockID blockId);

        /**
         * @brief Load block definitions from file
         * @param filename Block definition file
         * @return Number of blocks loaded
         */
        size_t LoadBlockDefinitions(const std::string& filename);

        /**
         * @brief Save block definitions to file
         * @param filename Output filename
         * @return true if successful, false otherwise
         */
        bool SaveBlockDefinitions(const std::string& filename) const;

        // Block interactions

        /**
         * @brief Handle block interaction
         * @param world World instance
         * @param x Block X coordinate
         * @param y Block Y coordinate
         * @param z Block Z coordinate
         * @param player Player interacting with block
         * @param interactionType Type of interaction
         * @return true if interaction handled, false otherwise
         */
        bool HandleBlockInteraction(World* world, int x, int y, int z,
                                   Player* player, const std::string& interactionType);

        /**
         * @brief Handle block placement
         * @param world World instance
         * @param x Block X coordinate
         * @param y Block Y coordinate
         * @param z Block Z coordinate
         * @param blockId Block ID being placed
         * @param player Player placing the block
         * @return true if placement successful, false otherwise
         */
        bool HandleBlockPlacement(World* world, int x, int y, int z,
                                 BlockID blockId, Player* player);

        /**
         * @brief Handle block destruction
         * @param world World instance
         * @param x Block X coordinate
         * @param y Block Y coordinate
         * @param z Block Z coordinate
         * @param player Player destroying the block
         * @return true if destruction successful, false otherwise
         */
        bool HandleBlockDestruction(World* world, int x, int y, int z, Player* player);

        // Block updates

        /**
         * @brief Queue block update
         * @param world World instance
         * @param x Block X coordinate
         * @param y Block Y coordinate
         * @param z Block Z coordinate
         * @param priority Update priority
         * @param delay Update delay in seconds
         * @return Update ID or 0 on failure
         */
        uint64_t QueueBlockUpdate(World* world, int x, int y, int z,
                                 int priority = 0, double delay = 0.0);

        /**
         * @brief Cancel block update
         * @param updateId Update ID to cancel
         * @return true if cancelled, false otherwise
         */
        bool CancelBlockUpdate(uint64_t updateId);

        /**
         * @brief Process pending block updates
         * @param deltaTime Time elapsed
         * @return Number of updates processed
         */
        size_t ProcessBlockUpdates(double deltaTime);

        // Block behaviors

        /**
         * @brief Register block behavior
         * @param behavior Behavior to register
         * @return true if registered, false otherwise
         */
        bool RegisterBlockBehavior(std::unique_ptr<BlockBehavior> behavior);

        /**
         * @brief Add behavior to block
         * @param blockId Block ID
         * @param behaviorName Behavior name
         * @return true if added, false otherwise
         */
        bool AddBehaviorToBlock(BlockID blockId, const std::string& behaviorName);

        /**
         * @brief Remove behavior from block
         * @param blockId Block ID
         * @param behaviorName Behavior name
         * @return true if removed, false otherwise
         */
        bool RemoveBehaviorFromBlock(BlockID blockId, const std::string& behaviorName);

        /**
         * @brief Trigger block behaviors
         * @param trigger Behavior trigger
         * @param world World instance
         * @param x Block X coordinate
         * @param y Block Y coordinate
         * @param z Block Z coordinate
         * @param context Additional context
         * @return Number of behaviors triggered
         */
        size_t TriggerBlockBehaviors(BehaviorTrigger trigger, World* world,
                                    int x, int y, int z,
                                    const std::unordered_map<std::string, std::any>& context = {});

        // Rendering system

        /**
         * @brief Get texture coordinates for block face
         * @param blockId Block ID
         * @param face Block face
         * @param metadata Block metadata
         * @return Texture coordinates
         */
        BlockTextureCoords GetTextureCoords(BlockID blockId, BlockFace face, BlockMetadata metadata) const;

        /**
         * @brief Generate mesh for chunk
         * @param chunk Chunk to generate mesh for
         * @return true if successful, false otherwise
         */
        bool GenerateChunkMesh(Chunk* chunk);

        /**
         * @brief Update block in chunk mesh
         * @param chunk Chunk containing the block
         * @param x Local X coordinate
         * @param y Local Y coordinate
         * @param z Local Z coordinate
         * @return true if updated, false otherwise
         */
        bool UpdateBlockInMesh(Chunk* chunk, int x, int y, int z);

        // Physics and collisions

        /**
         * @brief Check block collision at position
         * @param world World instance
         * @param x X coordinate
         * @param y Y coordinate
         * @param z Z coordinate
         * @return true if collidable, false otherwise
         */
        bool IsBlockCollidable(World* world, int x, int y, int z) const;

        /**
         * @brief Get block collision bounds
         * @param world World instance
         * @param x X coordinate
         * @param y Y coordinate
         * @param z Z coordinate
         * @return Collision bounds
         */
        BlockBounds GetBlockCollisionBounds(World* world, int x, int y, int z) const;

        /**
         * @brief Handle entity collision with block
         * @param world World instance
         * @param x Block X coordinate
         * @param y Block Y coordinate
         * @param z Block Z coordinate
         * @param entity Entity that collided
         * @return true if collision handled, false otherwise
         */
        bool HandleEntityCollision(World* world, int x, int y, int z, Entity* entity);

        // Configuration and metrics

        /**
         * @brief Get block system configuration
         * @return Current configuration
         */
        const BlockSystemConfig& GetConfig() const { return m_config; }

        /**
         * @brief Set block system configuration
         * @param config New configuration
         */
        void SetConfig(const BlockSystemConfig& config);

        /**
         * @brief Get block system metrics
         * @return Performance metrics
         */
        const BlockSystemMetrics& GetMetrics() const { return m_metrics; }

        /**
         * @brief Reset metrics
         */
        void ResetMetrics();

        // System access

        /**
         * @brief Get block registry
         * @return Block registry reference
         */
        BlockRegistry& GetBlockRegistry() { return *m_blockRegistry; }

        /**
         * @brief Get texture atlas
         * @return Texture atlas reference
         */
        TextureAtlas& GetTextureAtlas() { return *m_textureAtlas; }

        /**
         * @brief Get mesh generator
         * @return Mesh generator reference
         */
        BlockMeshGenerator& GetMeshGenerator() { return *m_meshGenerator; }

        /**
         * @brief Get behavior manager
         * @return Behavior manager reference
         */
        BlockBehaviorManager& GetBehaviorManager() { return *m_behaviorManager; }

        // Utilities

        /**
         * @brief Get all registered block types
         * @return Vector of block IDs
         */
        std::vector<BlockID> GetAllBlockTypes() const;

        /**
         * @brief Get block information summary
         * @return Block system status report
         */
        std::string GetSystemStatusReport() const;

        /**
         * @brief Validate block system integrity
         * @return Vector of validation errors
         */
        std::vector<std::string> ValidateSystem() const;

        /**
         * @brief Optimize block system performance
         * @return Optimization results
         */
        std::unordered_map<std::string, size_t> OptimizeSystem();

    private:
        /**
         * @brief Initialize subsystems
         * @return true if successful, false otherwise
         */
        bool InitializeSubsystems();

        /**
         * @brief Initialize default blocks
         * @return Number of blocks initialized
         */
        size_t InitializeDefaultBlocks();

        /**
         * @brief Update block system
         * @param deltaTime Time elapsed
         */
        void UpdateSystem(double deltaTime);

        /**
         * @brief Process block update queue
         * @param deltaTime Time elapsed
         * @return Number of updates processed
         */
        size_t ProcessUpdateQueue(double deltaTime);

        /**
         * @brief Update system metrics
         * @param deltaTime Time elapsed
         */
        void UpdateMetrics(double deltaTime);

        /**
         * @brief Handle block update
         * @param updateInfo Block update information
         * @return true if update handled, false otherwise
         */
        bool HandleBlockUpdate(const BlockUpdateInfo& updateInfo);

        /**
         * @brief Notify neighbors of block change
         * @param world World instance
         * @param x Block X coordinate
         * @param y Block Y coordinate
         * @param z Block Z coordinate
         */
        void NotifyBlockNeighbors(World* world, int x, int y, int z);

        /**
         * @brief Process block behavior triggers
         * @param trigger Behavior trigger
         * @param world World instance
         * @param x Block X coordinate
         * @param y Block Y coordinate
         * @param z Block Z coordinate
         * @param context Additional context
         */
        void ProcessBehaviorTriggers(BehaviorTrigger trigger, World* world,
                                   int x, int y, int z,
                                   const std::unordered_map<std::string, std::any>& context);

        // Configuration and state
        BlockSystemConfig m_config;                      ///< System configuration
        BlockSystemMetrics m_metrics;                    ///< Performance metrics
        World* m_world;                                  ///< Parent world

        // Core subsystems
        std::unique_ptr<BlockRegistry> m_blockRegistry;  ///< Block type registry
        std::unique_ptr<TextureAtlas> m_textureAtlas;    ///< Texture atlas system
        std::unique_ptr<BlockMeshGenerator> m_meshGenerator; ///< Mesh generation system
        std::unique_ptr<BlockBehaviorManager> m_behaviorManager; ///< Behavior system

        // Block update system
        struct PendingUpdate {
            uint64_t id;                                ///< Update ID
            BlockUpdateInfo info;                       ///< Update information
            double executeTime;                         ///< Execution time
            int retryCount;                             ///< Retry count
        };

        std::vector<PendingUpdate> m_pendingUpdates;    ///< Pending updates queue
        std::atomic<uint64_t> m_nextUpdateId;           ///< Next update ID

        // Threading and synchronization
        mutable std::shared_mutex m_systemMutex;        ///< System synchronization
        mutable std::shared_mutex m_updateMutex;        ///< Update synchronization
        mutable std::shared_mutex m_metricsMutex;       ///< Metrics synchronization
    };

    /**
     * @class BlockSystemFactory
     * @brief Factory for creating block system configurations
     */
    class BlockSystemFactory {
    public:
        /**
         * @brief Create default block system
         * @return Default block system configuration
         */
        static BlockSystemConfig CreateDefaultConfig();

        /**
         * @brief Create high-performance block system
         * @return High-performance configuration
         */
        static BlockSystemConfig CreateHighPerformanceConfig();

        /**
         * @brief Create low-memory block system
         * @return Low-memory configuration
         */
        static BlockSystemConfig CreateLowMemoryConfig();

        /**
         * @brief Create custom block system
         * @param baseConfig Base configuration
         * @param overrides Configuration overrides
         * @return Custom configuration
         */
        static BlockSystemConfig CreateCustomConfig(
            const BlockSystemConfig& baseConfig,
            const std::unordered_map<std::string, std::any>& overrides);

        /**
         * @brief Load configuration from file
         * @param filename Configuration file
         * @return Loaded configuration
         */
        static BlockSystemConfig LoadConfigFromFile(const std::string& filename);

        /**
         * @brief Save configuration to file
         * @param config Configuration to save
         * @param filename Output filename
         * @return true if successful, false otherwise
         */
        static bool SaveConfigToFile(const BlockSystemConfig& config, const std::string& filename);
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_BLOCKS_BLOCK_SYSTEM_HPP
