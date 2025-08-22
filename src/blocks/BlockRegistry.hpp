/**
 * @file BlockRegistry.hpp
 * @brief VoxelCraft Block Registry System
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines the BlockRegistry class that manages all block types,
 * their registration, lookup, and serialization for the voxel world system.
 */

#ifndef VOXELCRAFT_BLOCKS_BLOCK_REGISTRY_HPP
#define VOXELCRAFT_BLOCKS_BLOCK_REGISTRY_HPP

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <optional>
#include <deque>

#include "../core/Config.hpp"
#include "Block.hpp"

namespace VoxelCraft {

    /**
     * @struct BlockRegistrationInfo
     * @brief Information about a block registration
     */
    struct BlockRegistrationInfo {
        BlockID id;                    ///< Block ID
        std::string name;             ///< Block name
        std::string displayName;      ///< Display name
        std::string description;      ///< Block description
        std::string category;         ///< Block category
        bool isDefault;              ///< Is default block
        double registrationTime;      ///< Registration timestamp
        size_t memoryUsage;          ///< Memory usage estimate
    };

    /**
     * @struct BlockCategory
     * @brief Block category information
     */
    struct BlockCategory {
        std::string name;             ///< Category name
        std::string displayName;      ///< Display name
        std::string description;      ///< Category description
        std::vector<BlockID> blocks;  ///< Blocks in this category
        bool isVisible;              ///< Visible in creative inventory
        int sortOrder;               ///< Sort order for display
    };

    /**
     * @struct BlockSearchCriteria
     * @brief Criteria for searching blocks
     */
    struct BlockSearchCriteria {
        std::optional<std::string> namePattern;    ///< Name pattern (supports wildcards)
        std::optional<std::string> category;       ///< Block category
        std::optional<BlockMaterial> material;     ///< Block material
        std::optional<BlockRenderType> renderType; ///< Render type
        std::optional<BlockSoundType> soundType;   ///< Sound type
        std::optional<float> minHardness;          ///< Minimum hardness
        std::optional<float> maxHardness;          ///< Maximum hardness
        std::optional<bool> isOpaque;             ///< Opacity filter
        std::optional<bool> isSolid;              ///< Solidity filter
        std::optional<bool> hasLight;             ///< Has light emission
        std::optional<int> minLightLevel;         ///< Minimum light level
        std::optional<int> maxLightLevel;         ///< Maximum light level
        bool matchAll;                            ///< Match all criteria (AND) or any (OR)

        BlockSearchCriteria() : matchAll(true) {}
    };

    /**
     * @struct BlockRegistryMetrics
     * @brief Performance metrics for the block registry
     */
    struct BlockRegistryMetrics {
        uint64_t totalRegistrations;               ///< Total block registrations
        uint64_t totalUnregistrations;             ///< Total block unregistrations
        uint32_t activeBlocks;                     ///< Currently active blocks
        uint32_t totalCategories;                  ///< Total categories
        double averageRegistrationTime;            ///< Average registration time
        double averageLookupTime;                  ///< Average lookup time
        size_t totalMemoryUsage;                   ///< Total memory usage
        size_t peakMemoryUsage;                    ///< Peak memory usage
        uint64_t cacheHits;                        ///< Cache hits
        uint64_t cacheMisses;                      ///< Cache misses
        double cacheHitRate;                       ///< Cache hit rate (0.0 - 1.0)
    };

    /**
     * @class BlockRegistry
     * @brief Central registry for all block types in the voxel world
     *
     * The BlockRegistry provides:
     * - Registration and management of block types
     * - Fast lookup by ID and name
     * - Block categorization and searching
     * - Serialization and deserialization of blocks
     * - Memory management and optimization
     * - Performance monitoring and caching
     *
     * Key Features:
     * - Thread-safe operations with shared mutexes
     * - Efficient caching system for lookups
     * - Category-based organization
     * - Search and filtering capabilities
     * - Memory usage tracking
     * - Serialization support
     */
    class BlockRegistry {
    public:
        /**
         * @brief Constructor
         */
        BlockRegistry();

        /**
         * @brief Destructor
         */
        ~BlockRegistry();

        /**
         * @brief Deleted copy constructor
         */
        BlockRegistry(const BlockRegistry&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        BlockRegistry& operator=(const BlockRegistry&) = delete;

        // Block registration

        /**
         * @brief Register a block type
         * @param block Block to register
         * @param category Optional category for the block
         * @return true if registered successfully, false otherwise
         */
        bool RegisterBlock(std::unique_ptr<Block> block, const std::string& category = "general");

        /**
         * @brief Register block with custom properties
         * @param block Block to register
         * @param category Block category
         * @param properties Custom properties
         * @return true if registered successfully, false otherwise
         */
        bool RegisterBlock(std::unique_ptr<Block> block,
                          const std::string& category,
                          const std::unordered_map<std::string, std::any>& properties);

        /**
         * @brief Unregister a block type
         * @param id Block ID to unregister
         * @return true if unregistered, false if not found
         */
        bool UnregisterBlock(BlockID id);

        /**
         * @brief Unregister block by name
         * @param name Block name to unregister
         * @return true if unregistered, false if not found
         */
        bool UnregisterBlock(const std::string& name);

        // Block lookup

        /**
         * @brief Get block by ID
         * @param id Block ID
         * @return Block pointer or nullptr if not found
         */
        Block* GetBlock(BlockID id) const;

        /**
         * @brief Get block by name
         * @param name Block name
         * @return Block pointer or nullptr if not found
         */
        Block* GetBlock(const std::string& name) const;

        /**
         * @brief Get block ID by name
         * @param name Block name
         * @return Block ID or 0 if not found
         */
        BlockID GetBlockId(const std::string& name) const;

        /**
         * @brief Get block name by ID
         * @param id Block ID
         * @return Block name or empty string if not found
         */
        std::string GetBlockName(BlockID id) const;

        /**
         * @brief Check if block is registered
         * @param id Block ID
         * @return true if registered, false otherwise
         */
        bool IsRegistered(BlockID id) const;

        /**
         * @brief Check if block name is registered
         * @param name Block name
         * @return true if registered, false otherwise
         */
        bool IsRegistered(const std::string& name) const;

        /**
         * @brief Get block registration information
         * @param id Block ID
         * @return Registration info or empty optional if not found
         */
        std::optional<BlockRegistrationInfo> GetRegistrationInfo(BlockID id) const;

        // Block enumeration

        /**
         * @brief Get all registered blocks
         * @return Vector of block pointers
         */
        std::vector<Block*> GetAllBlocks() const;

        /**
         * @brief Get blocks in category
         * @param category Category name
         * @return Vector of block pointers
         */
        std::vector<Block*> GetBlocksInCategory(const std::string& category) const;

        /**
         * @brief Get all block IDs
         * @return Vector of block IDs
         */
        std::vector<BlockID> GetAllBlockIds() const;

        /**
         * @brief Get all block names
         * @return Vector of block names
         */
        std::vector<std::string> GetAllBlockNames() const;

        /**
         * @brief Get total number of registered blocks
         * @return Number of registered blocks
         */
        size_t GetBlockCount() const;

        // Block searching

        /**
         * @brief Search blocks by criteria
         * @param criteria Search criteria
         * @return Vector of matching block pointers
         */
        std::vector<Block*> SearchBlocks(const BlockSearchCriteria& criteria) const;

        /**
         * @brief Find blocks by name pattern
         * @param pattern Name pattern (supports wildcards)
         * @return Vector of matching block pointers
         */
        std::vector<Block*> FindBlocksByName(const std::string& pattern) const;

        /**
         * @brief Find blocks by properties
         * @param properties Property key-value pairs
         * @return Vector of matching block pointers
         */
        std::vector<Block*> FindBlocksByProperties(const std::unordered_map<std::string, std::any>& properties) const;

        // Category management

        /**
         * @brief Create a new block category
         * @param name Category name
         * @param displayName Display name
         * @param description Category description
         * @return true if created, false if already exists
         */
        bool CreateCategory(const std::string& name,
                           const std::string& displayName,
                           const std::string& description = "");

        /**
         * @brief Get category information
         * @param name Category name
         * @return Category info or empty optional if not found
         */
        std::optional<BlockCategory> GetCategory(const std::string& name) const;

        /**
         * @brief Get all categories
         * @return Vector of category names
         */
        std::vector<std::string> GetAllCategories() const;

        /**
         * @brief Get category count
         * @return Number of categories
         */
        size_t GetCategoryCount() const;

        /**
         * @brief Move block to different category
         * @param blockId Block ID
         * @param newCategory New category name
         * @return true if moved, false otherwise
         */
        bool MoveBlockToCategory(BlockID blockId, const std::string& newCategory);

        // Serialization

        /**
         * @brief Serialize registry to data stream
         * @param stream Output stream
         * @return true if successful, false otherwise
         */
        bool Serialize(std::ostream& stream) const;

        /**
         * @brief Deserialize registry from data stream
         * @param stream Input stream
         * @return true if successful, false otherwise
         */
        bool Deserialize(std::istream& stream);

        /**
         * @brief Export registry to JSON format
         * @return JSON string representation
         */
        std::string ExportToJson() const;

        /**
         * @brief Import registry from JSON format
         * @param json JSON string
         * @return true if successful, false otherwise
         */
        bool ImportFromJson(const std::string& json);

        // Memory management

        /**
         * @brief Get total memory usage
         * @return Memory usage in bytes
         */
        size_t GetMemoryUsage() const;

        /**
         * @brief Optimize memory usage
         * @return Number of optimizations performed
         */
        size_t OptimizeMemory();

        /**
         * @brief Clear all registered blocks
         */
        void Clear();

        // Default blocks

        /**
         * @brief Initialize default block types
         * @return Number of blocks initialized
         */
        size_t InitializeDefaults();

        /**
         * @brief Check if default blocks are initialized
         * @return true if initialized, false otherwise
         */
        bool AreDefaultsInitialized() const;

        // Performance monitoring

        /**
         * @brief Get registry metrics
         * @return Performance metrics
         */
        const BlockRegistryMetrics& GetMetrics() const;

        /**
         * @brief Reset metrics
         */
        void ResetMetrics();

        /**
         * @brief Get performance report
         * @return Detailed performance report
         */
        std::string GetPerformanceReport() const;

        // Cache management

        /**
         * @brief Enable/disable caching
         * @param enabled Enable state
         * @return Previous state
         */
        bool SetCachingEnabled(bool enabled);

        /**
         * @brief Check if caching is enabled
         * @return true if enabled, false otherwise
         */
        bool IsCachingEnabled() const { return m_cachingEnabled; }

        /**
         * @brief Clear lookup cache
         */
        void ClearCache();

        /**
         * @brief Get cache statistics
         * @return Cache statistics
         */
        std::unordered_map<std::string, size_t> GetCacheStatistics() const;

    private:
        /**
         * @brief Validate block before registration
         * @param block Block to validate
         * @return true if valid, false otherwise
         */
        bool ValidateBlock(const Block* block) const;

        /**
         * @brief Update cache after registration/unregistration
         * @param id Block ID
         * @param name Block name
         * @param operation Operation type (add/remove)
         */
        void UpdateCache(BlockID id, const std::string& name, const std::string& operation);

        /**
         * @brief Update category information
         * @param category Category name
         * @param blockId Block ID
         * @param operation Operation type (add/remove)
         */
        void UpdateCategory(const std::string& category, BlockID blockId, const std::string& operation);

        /**
         * @brief Update metrics after operation
         * @param operation Operation type
         */
        void UpdateMetrics(const std::string& operation);

        /**
         * @brief Get next available block ID
         * @return Next block ID
         */
        BlockID GetNextBlockId();

        // Block storage
        std::unordered_map<BlockID, std::unique_ptr<Block>> m_blocks;
        std::unordered_map<std::string, BlockID> m_nameToId;
        std::unordered_map<BlockID, BlockRegistrationInfo> m_registrationInfo;
        mutable std::shared_mutex m_blocksMutex;

        // Category management
        std::unordered_map<std::string, BlockCategory> m_categories;
        mutable std::shared_mutex m_categoriesMutex;

        // Cache system
        bool m_cachingEnabled;
        mutable std::unordered_map<BlockID, Block*> m_idCache;
        mutable std::unordered_map<std::string, Block*> m_nameCache;
        mutable std::shared_mutex m_cacheMutex;

        // Metrics
        BlockRegistryMetrics m_metrics;
        mutable std::mutex m_metricsMutex;

        // State
        bool m_defaultsInitialized;
        BlockID m_nextBlockId;

        // Default block IDs (for quick access)
        static const BlockID AIR_BLOCK_ID = 0;
        static const BlockID STONE_BLOCK_ID = 1;
        static const BlockID DIRT_BLOCK_ID = 2;
        static const BlockID GRASS_BLOCK_ID = 3;
        static const BlockID WATER_BLOCK_ID = 4;
    };

    /**
     * @class BlockRegistryFactory
     * @brief Factory for creating block registries with different configurations
     */
    class BlockRegistryFactory {
    public:
        /**
         * @brief Create default block registry
         * @return Default registry instance
         */
        static std::unique_ptr<BlockRegistry> CreateDefaultRegistry();

        /**
         * @brief Create registry with custom configuration
         * @param config Configuration options
         * @return Configured registry instance
         */
        static std::unique_ptr<BlockRegistry> CreateRegistry(
            const std::unordered_map<std::string, std::any>& config);

        /**
         * @brief Create registry from file
         * @param filename Registry configuration file
         * @return Loaded registry instance
         */
        static std::unique_ptr<BlockRegistry> CreateRegistryFromFile(const std::string& filename);

        /**
         * @brief Get default registry configuration
         * @return Default configuration
         */
        static std::unordered_map<std::string, std::any> GetDefaultConfig();
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_BLOCKS_BLOCK_REGISTRY_HPP
