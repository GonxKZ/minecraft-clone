/**
 * @file BlockRegistry.cpp
 * @brief VoxelCraft Block Registry Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#include "BlockRegistry.hpp"
#include <algorithm>
#include <sstream>
#include <iostream>

namespace VoxelCraft {

    BlockRegistry::BlockRegistry()
        : m_cachingEnabled(true)
        , m_defaultsInitialized(false)
        , m_nextBlockId(1) // Start from 1, 0 is reserved for air
    {
        // Initialize metrics
        ResetMetrics();
    }

    BlockRegistry::~BlockRegistry() {
        Clear();
    }

    bool BlockRegistry::RegisterBlock(std::unique_ptr<Block> block, const std::string& category) {
        if (!block) {
            return false;
        }

        BlockID id = block->GetType();
        const std::string& name = block->GetName();

        std::unique_lock<std::shared_mutex> lock(m_blocksMutex);

        // Check if already registered
        if (m_blocks.find(id) != m_blocks.end()) {
            return false;
        }

        if (m_nameToId.find(name) != m_nameToId.end()) {
            return false;
        }

        // Register the block
        m_blocks[id] = std::move(block);
        m_nameToId[name] = id;

        // Create registration info
        BlockRegistrationInfo info;
        info.id = id;
        info.name = name;
        info.displayName = m_blocks[id]->GetName();
        info.category = category;
        info.isDefault = false;
        info.registrationTime = 0.0; // TODO: Add timestamp
        info.memoryUsage = sizeof(Block); // Estimate

        m_registrationInfo[id] = info;

        // Update cache
        if (m_cachingEnabled) {
            UpdateCache(id, name, "add");
        }

        // Update category
        UpdateCategory(category, id, "add");

        // Update metrics
        UpdateMetrics("register");

        return true;
    }

    bool BlockRegistry::RegisterBlock(std::unique_ptr<Block> block,
                                     const std::string& category,
                                     const std::unordered_map<std::string, std::any>& properties) {
        // For now, ignore properties and just register normally
        return RegisterBlock(std::move(block), category);
    }

    bool BlockRegistry::UnregisterBlock(BlockID id) {
        std::unique_lock<std::shared_mutex> lock(m_blocksMutex);

        auto it = m_blocks.find(id);
        if (it == m_blocks.end()) {
            return false;
        }

        std::string name = it->second->GetName();

        // Remove from name mapping
        m_nameToId.erase(name);

        // Remove registration info
        auto infoIt = m_registrationInfo.find(id);
        if (infoIt != m_registrationInfo.end()) {
            std::string category = infoIt->second.category;
            UpdateCategory(category, id, "remove");
            m_registrationInfo.erase(infoIt);
        }

        // Remove block
        m_blocks.erase(it);

        // Update cache
        if (m_cachingEnabled) {
            UpdateCache(id, name, "remove");
        }

        // Update metrics
        UpdateMetrics("unregister");

        return true;
    }

    bool BlockRegistry::UnregisterBlock(const std::string& name) {
        auto it = m_nameToId.find(name);
        if (it == m_nameToId.end()) {
            return false;
        }

        return UnregisterBlock(it->second);
    }

    Block* BlockRegistry::GetBlock(BlockID id) const {
        // Check cache first
        if (m_cachingEnabled) {
            std::shared_lock<std::shared_mutex> cacheLock(m_cacheMutex);
            auto cacheIt = m_idCache.find(id);
            if (cacheIt != m_idCache.end()) {
                m_metrics.cacheHits++;
                return cacheIt->second;
            }
        }

        // Check main storage
        std::shared_lock<std::shared_mutex> lock(m_blocksMutex);
        auto it = m_blocks.find(id);
        if (it != m_blocks.end()) {
            Block* block = it->second.get();

            // Update cache
            if (m_cachingEnabled) {
                std::unique_lock<std::shared_mutex> cacheLock(m_cacheMutex);
                m_idCache[id] = block;
                m_metrics.cacheMisses++;
            }

            return block;
        }

        return nullptr;
    }

    Block* BlockRegistry::GetBlock(const std::string& name) const {
        // Check cache first
        if (m_cachingEnabled) {
            std::shared_lock<std::shared_mutex> cacheLock(m_cacheMutex);
            auto cacheIt = m_nameCache.find(name);
            if (cacheIt != m_nameCache.end()) {
                m_metrics.cacheHits++;
                return cacheIt->second;
            }
        }

        // Get ID from name
        std::shared_lock<std::shared_mutex> lock(m_blocksMutex);
        auto nameIt = m_nameToId.find(name);
        if (nameIt != m_nameToId.end()) {
            BlockID id = nameIt->second;
            auto blockIt = m_blocks.find(id);
            if (blockIt != m_blocks.end()) {
                Block* block = blockIt->second.get();

                // Update cache
                if (m_cachingEnabled) {
                    std::unique_lock<std::shared_mutex> cacheLock(m_cacheMutex);
                    m_nameCache[name] = block;
                    m_metrics.cacheMisses++;
                }

                return block;
            }
        }

        return nullptr;
    }

    BlockID BlockRegistry::GetBlockId(const std::string& name) const {
        std::shared_lock<std::shared_mutex> lock(m_blocksMutex);
        auto it = m_nameToId.find(name);
        return (it != m_nameToId.end()) ? it->second : 0;
    }

    std::string BlockRegistry::GetBlockName(BlockID id) const {
        std::shared_lock<std::shared_mutex> lock(m_blocksMutex);
        auto it = m_blocks.find(id);
        return (it != m_blocks.end()) ? it->second->GetName() : "";
    }

    bool BlockRegistry::IsRegistered(BlockID id) const {
        std::shared_lock<std::shared_mutex> lock(m_blocksMutex);
        return m_blocks.find(id) != m_blocks.end();
    }

    bool BlockRegistry::IsRegistered(const std::string& name) const {
        std::shared_lock<std::shared_mutex> lock(m_blocksMutex);
        return m_nameToId.find(name) != m_nameToId.end();
    }

    std::vector<Block*> BlockRegistry::GetAllBlocks() const {
        std::shared_lock<std::shared_mutex> lock(m_blocksMutex);
        std::vector<Block*> result;
        result.reserve(m_blocks.size());

        for (const auto& pair : m_blocks) {
            result.push_back(pair.second.get());
        }

        return result;
    }

    std::vector<BlockID> BlockRegistry::GetAllBlockIds() const {
        std::shared_lock<std::shared_mutex> lock(m_blocksMutex);
        std::vector<BlockID> result;
        result.reserve(m_blocks.size());

        for (const auto& pair : m_blocks) {
            result.push_back(pair.first);
        }

        return result;
    }

    size_t BlockRegistry::GetBlockCount() const {
        std::shared_lock<std::shared_mutex> lock(m_blocksMutex);
        return m_blocks.size();
    }

    bool BlockRegistry::CreateCategory(const std::string& name,
                                      const std::string& displayName,
                                      const std::string& description) {
        std::unique_lock<std::shared_mutex> lock(m_categoriesMutex);

        if (m_categories.find(name) != m_categories.end()) {
            return false;
        }

        BlockCategory category;
        category.name = name;
        category.displayName = displayName;
        category.description = description;
        category.isVisible = true;
        category.sortOrder = m_categories.size();

        m_categories[name] = category;
        return true;
    }

    size_t BlockRegistry::GetMemoryUsage() const {
        std::shared_lock<std::shared_mutex> lock(m_blocksMutex);
        size_t usage = 0;

        for (const auto& pair : m_blocks) {
            usage += sizeof(Block) + pair.second->GetName().capacity();
        }

        return usage;
    }

    void BlockRegistry::Clear() {
        std::unique_lock<std::shared_mutex> lock(m_blocksMutex);
        std::unique_lock<std::shared_mutex> catLock(m_categoriesMutex);
        std::unique_lock<std::shared_mutex> cacheLock(m_cacheMutex);

        m_blocks.clear();
        m_nameToId.clear();
        m_registrationInfo.clear();
        m_categories.clear();
        m_idCache.clear();
        m_nameCache.clear();

        m_nextBlockId = 1;
        m_defaultsInitialized = false;

        ResetMetrics();
    }

    size_t BlockRegistry::InitializeDefaults() {
        if (m_defaultsInitialized) {
            return 0;
        }

        size_t count = 0;

        // Create default categories
        CreateCategory("building", "Building Blocks", "Basic building materials");
        CreateCategory("nature", "Natural Blocks", "Naturally occurring blocks");
        CreateCategory("redstone", "Redstone", "Redstone components");
        CreateCategory("decorative", "Decorative", "Decorative blocks");
        CreateCategory("tools", "Tools", "Tools and equipment");

        // Air block (always ID 0)
        auto airBlock = Block::CreateBlock(BlockType::AIR);
        if (RegisterBlock(std::move(airBlock), "nature")) {
            count++;
        }

        // Stone block
        auto stoneBlock = Block::CreateBlock(BlockType::STONE);
        if (RegisterBlock(std::move(stoneBlock), "building")) {
            count++;
        }

        // Dirt block
        auto dirtBlock = Block::CreateBlock(BlockType::DIRT);
        if (RegisterBlock(std::move(dirtBlock), "nature")) {
            count++;
        }

        // Grass block
        auto grassBlock = Block::CreateBlock(BlockType::GRASS_BLOCK);
        if (RegisterBlock(std::move(grassBlock), "nature")) {
            count++;
        }

        // Other basic blocks
        std::vector<BlockType> basicBlocks = {
            BlockType::COBBLESTONE, BlockType::WOOD_PLANKS, BlockType::BEDROCK,
            BlockType::SAND, BlockType::GRAVEL, BlockType::OAK_LOG,
            BlockType::OAK_LEAVES, BlockType::GLASS, BlockType::BRICKS,
            BlockType::IRON_BLOCK, BlockType::GOLD_BLOCK, BlockType::DIAMOND_BLOCK
        };

        for (auto type : basicBlocks) {
            auto block = Block::CreateBlock(type);
            if (RegisterBlock(std::move(block), "building")) {
                count++;
            }
        }

        m_defaultsInitialized = true;
        return count;
    }

    bool BlockRegistry::AreDefaultsInitialized() const {
        return m_defaultsInitialized;
    }

    const BlockRegistryMetrics& BlockRegistry::GetMetrics() const {
        return m_metrics;
    }

    void BlockRegistry::ResetMetrics() {
        m_metrics.totalRegistrations = 0;
        m_metrics.totalUnregistrations = 0;
        m_metrics.activeBlocks = 0;
        m_metrics.totalCategories = 0;
        m_metrics.averageRegistrationTime = 0.0;
        m_metrics.averageLookupTime = 0.0;
        m_metrics.totalMemoryUsage = 0;
        m_metrics.peakMemoryUsage = 0;
        m_metrics.cacheHits = 0;
        m_metrics.cacheMisses = 0;
        m_metrics.cacheHitRate = 0.0;
    }

    bool BlockRegistry::SetCachingEnabled(bool enabled) {
        bool wasEnabled = m_cachingEnabled;
        m_cachingEnabled = enabled;

        if (!enabled) {
            ClearCache();
        }

        return wasEnabled;
    }

    void BlockRegistry::ClearCache() {
        std::unique_lock<std::shared_mutex> lock(m_cacheMutex);
        m_idCache.clear();
        m_nameCache.clear();
    }

    void BlockRegistry::UpdateCache(BlockID id, const std::string& name, const std::string& operation) {
        if (!m_cachingEnabled) {
            return;
        }

        std::unique_lock<std::shared_mutex> lock(m_cacheMutex);

        if (operation == "remove") {
            m_idCache.erase(id);
            m_nameCache.erase(name);
        }
        // For "add", cache will be populated on next lookup
    }

    void BlockRegistry::UpdateCategory(const std::string& category, BlockID blockId, const std::string& operation) {
        std::unique_lock<std::shared_mutex> lock(m_categoriesMutex);

        auto catIt = m_categories.find(category);
        if (catIt != m_categories.end()) {
            auto& cat = catIt->second;

            if (operation == "add") {
                if (std::find(cat.blocks.begin(), cat.blocks.end(), blockId) == cat.blocks.end()) {
                    cat.blocks.push_back(blockId);
                }
            } else if (operation == "remove") {
                cat.blocks.erase(std::remove(cat.blocks.begin(), cat.blocks.end(), blockId), cat.blocks.end());
            }
        }
    }

    void BlockRegistry::UpdateMetrics(const std::string& operation) {
        std::lock_guard<std::mutex> lock(m_metricsMutex);

        if (operation == "register") {
            m_metrics.totalRegistrations++;
            m_metrics.activeBlocks = m_blocks.size();
        } else if (operation == "unregister") {
            m_metrics.totalUnregistrations++;
            m_metrics.activeBlocks = m_blocks.size();
        }

        m_metrics.totalCategories = m_categories.size();
        m_metrics.totalMemoryUsage = GetMemoryUsage();

        // Update cache hit rate
        uint64_t totalRequests = m_metrics.cacheHits + m_metrics.cacheMisses;
        if (totalRequests > 0) {
            m_metrics.cacheHitRate = static_cast<double>(m_metrics.cacheHits) / totalRequests;
        }
    }

    // Factory methods
    std::unique_ptr<BlockRegistry> BlockRegistryFactory::CreateDefaultRegistry() {
        auto registry = std::make_unique<BlockRegistry>();
        registry->InitializeDefaults();
        return registry;
    }

} // namespace VoxelCraft
