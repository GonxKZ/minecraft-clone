/**
 * @file EnchantmentTable.hpp
 * @brief VoxelCraft Enchantment Table System - Enchanting Interface
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_ENCHANTMENT_ENCHANTMENT_TABLE_HPP
#define VOXELCRAFT_ENCHANTMENT_ENCHANTMENT_TABLE_HPP

#include <memory>
#include <vector>
#include <array>
#include <functional>
#include <unordered_map>
#include <chrono>
#include <glm/glm.hpp>

#include "Enchantment.hpp"

namespace VoxelCraft {

    class Player;
    class World;
    class Inventory;

    /**
     * @enum EnchantmentSlot
     * @brief Slots in the enchantment table
     */
    enum class EnchantmentSlot {
        INPUT = 0,           ///< Item to enchant
        LAPIS_LAZULI,        ///< Lapis lazuli for enchanting
        OUTPUT              ///< Enchanted result
    };

    /**
     * @struct EnchantmentOption
     * @brief Single enchantment option shown to player
     */
    struct EnchantmentOption {
        std::vector<EnchantmentInstance> enchantments;
        int xpCost;
        int lapisCost;
        int levelRequirement;
        float enchantmentPower;               ///< 0-3, determines glow intensity
        std::string description;

        EnchantmentOption() : xpCost(0), lapisCost(0), levelRequirement(0), enchantmentPower(0.0f) {}
    };

    /**
     * @struct EnchantmentTableState
     * @brief Current state of the enchantment table
     */
    struct EnchantmentTableState {
        std::array<int, 3> slots;             ///< Item IDs in each slot
        std::array<int, 3> slotCounts;        ///< Item counts in each slot
        std::vector<EnchantmentOption> options;
        int selectedOption;                   ///< Currently selected option (-1 if none)
        int bookshelfCount;                   ///< Number of bookshelves around table
        float enchantmentSeed;                ///< Random seed for enchantment generation
        bool isEnchanting;                    ///< Whether enchanting is in progress

        EnchantmentTableState() : selectedOption(-1), bookshelfCount(0), enchantmentSeed(0.0f), isEnchanting(false) {
            slots.fill(0);
            slotCounts.fill(0);
        }
    };

    /**
     * @struct EnchantmentTableConfig
     * @brief Configuration for enchantment tables
     */
    struct EnchantmentTableConfig {
        bool enableEnchantmentTable = true;
        int maxBookshelves = 15;              ///< Maximum bookshelves that affect enchanting
        int minXPLevel = 1;                   ///< Minimum XP level for enchanting
        int maxXPLevel = 30;                  ///< Maximum XP level for enchanting
        float baseEnchantmentCost = 1.0f;     ///< Base XP cost multiplier
        float bookshelfMultiplier = 1.15f;    ///< XP cost increase per bookshelf
        int minLapisCost = 1;                 ///< Minimum lapis lazuli required
        int maxLapisCost = 3;                 ///< Maximum lapis lazuli required
        float enchantmentTime = 2.0f;         ///< Time to enchant in seconds
        bool requireBookshelves = false;      ///< Whether bookshelves are required
        bool enableTreasureEnchantments = true;
    };

    /**
     * @class EnchantmentTable
     * @brief Enchantment table block and interface
     */
    class EnchantmentTable {
    public:
        /**
         * @brief Constructor
         * @param position World position
         * @param world World reference
         */
        EnchantmentTable(const glm::ivec3& position, World* world);

        /**
         * @brief Destructor
         */
        ~EnchantmentTable();

        /**
         * @brief Update enchantment table
         * @param deltaTime Time since last update
         */
        void Update(float deltaTime);

        /**
         * @brief Open enchantment interface for player
         * @param player Player opening the table
         * @return true if interface opened
         */
        bool OpenInterface(Player* player);

        /**
         * @brief Close enchantment interface
         */
        void CloseInterface();

        /**
         * @brief Get current table state
         * @return Current state
         */
        const EnchantmentTableState& GetState() const { return m_state; }

        /**
         * @brief Set item in slot
         * @param slot Slot to set
         * @param itemID Item ID
         * @param count Item count
         * @return true if item was set
         */
        bool SetItem(EnchantmentSlot slot, int itemID, int count);

        /**
         * @brief Get item from slot
         * @param slot Slot to get from
         * @return Pair of itemID and count
         */
        std::pair<int, int> GetItem(EnchantmentSlot slot) const;

        /**
         * @brief Select enchantment option
         * @param optionIndex Index of option to select
         * @return true if option was selected
         */
        bool SelectOption(int optionIndex);

        /**
         * @brief Perform enchanting
         * @return true if enchanting was successful
         */
        bool PerformEnchanting();

        /**
         * @brief Calculate enchantment power based on bookshelves
         * @return Enchantment power (0-3)
         */
        float CalculateEnchantmentPower() const;

        /**
         * @brief Generate enchantment options
         */
        void GenerateEnchantmentOptions();

        /**
         * @brief Check if item can be enchanted
         * @param itemID Item ID to check
         * @return true if can be enchanted
         */
        bool CanEnchantItem(int itemID) const;

        /**
         * @brief Get required lapis lazuli for item
         * @param itemID Item ID
         * @return Required lapis count
         */
        int GetRequiredLapis(int itemID) const;

        /**
         * @brief Get XP cost for enchantment
         * @param option Enchantment option
         * @return XP cost
         */
        int GetXPCost(const EnchantmentOption& option) const;

        /**
         * @brief Check if player can enchant
         * @param player Player to check
         * @return true if can enchant
         */
        bool CanPlayerEnchant(Player* player) const;

        /**
         * @brief Get table position
         * @return World position
         */
        const glm::ivec3& GetPosition() const { return m_position; }

        /**
         * @brief Get current player using table
         * @return Current player or nullptr
         */
        Player* GetCurrentPlayer() const { return m_currentPlayer; }

        /**
         * @brief Check if table is in use
         * @return true if in use
         */
        bool IsInUse() const { return m_currentPlayer != nullptr; }

    private:
        glm::ivec3 m_position;
        World* m_world;
        Player* m_currentPlayer;
        EnchantmentTableState m_state;
        EnchantmentTableConfig m_config;

        // Animation and effects
        float m_enchantmentProgress;
        float m_glowIntensity;
        float m_bookshelfScanTimer;
        std::chrono::steady_clock::time_point m_lastUpdate;

        /**
         * @brief Count bookshelves around table
         * @return Number of bookshelves
         */
        int CountBookshelves() const;

        /**
         * @brief Scan for bookshelves periodically
         */
        void ScanForBookshelves();

        /**
         * @brief Generate random enchantment seed
         * @return Random seed
         */
        float GenerateEnchantmentSeed() const;

        /**
         * @brief Calculate base enchantment level
         * @param playerXP Player XP level
         * @return Base enchantment level
         */
        int CalculateBaseEnchantmentLevel(int playerXP) const;

        /**
         * @brief Create enchantment option
         * @param level Base enchantment level
         * @param itemID Item to enchant
         * @return Enchantment option
         */
        EnchantmentOption CreateEnchantmentOption(int level, int itemID);

        /**
         * @brief Apply enchantments to item
         * @param option Selected enchantment option
         * @return Enchanted item
         */
        EnchantedItem ApplyEnchantments(const EnchantmentOption& option);

        /**
         * @brief Consume enchanting materials
         * @param option Selected option
         * @return true if materials consumed successfully
         */
        bool ConsumeMaterials(const EnchantmentOption& option);

        /**
         * @brief Play enchantment effects
         */
        void PlayEnchantmentEffects();

        /**
         * @brief Update visual effects
         * @param deltaTime Time since last update
         */
        void UpdateVisualEffects(float deltaTime);

        /**
         * @brief Validate enchantment table setup
         * @return true if setup is valid
         */
        bool ValidateSetup() const;
    };

    /**
     * @class EnchantmentTableManager
     * @brief Manager for all enchantment tables in the world
     */
    class EnchantmentTableManager {
    public:
        /**
         * @brief Get singleton instance
         * @return EnchantmentTableManager instance
         */
        static EnchantmentTableManager& GetInstance();

        /**
         * @brief Initialize manager
         * @param world World reference
         * @return true if successful
         */
        bool Initialize(World* world);

        /**
         * @brief Shutdown manager
         */
        void Shutdown();

        /**
         * @brief Update all enchantment tables
         * @param deltaTime Time since last update
         */
        void Update(float deltaTime);

        /**
         * @brief Create enchantment table at position
         * @param position Position to create table
         * @return true if created successfully
         */
        bool CreateEnchantmentTable(const glm::ivec3& position);

        /**
         * @brief Remove enchantment table at position
         * @param position Position to remove table
         * @return true if removed successfully
         */
        bool RemoveEnchantmentTable(const glm::ivec3& position);

        /**
         * @brief Get enchantment table at position
         * @param position Position to check
         * @return Enchantment table or nullptr
         */
        std::shared_ptr<EnchantmentTable> GetEnchantmentTable(const glm::ivec3& position) const;

        /**
         * @brief Get all enchantment tables
         * @return Vector of all tables
         */
        std::vector<std::shared_ptr<EnchantmentTable>> GetAllEnchantmentTables() const;

        /**
         * @brief Find nearest enchantment table to position
         * @param position Search position
         * @param maxDistance Maximum search distance
         * @return Nearest table or nullptr
         */
        std::shared_ptr<EnchantmentTable> FindNearestEnchantmentTable(const glm::vec3& position, float maxDistance = 32.0f);

        /**
         * @brief Get enchantment table statistics
         * @return Table statistics
         */
        const EnchantmentTableStats& GetStats() const { return m_stats; }

    private:
        EnchantmentTableManager() = default;
        ~EnchantmentTableManager() = default;

        // Prevent copying
        EnchantmentTableManager(const EnchantmentTableManager&) = delete;
        EnchantmentTableManager& operator=(const EnchantmentTableManager&) = delete;

        World* m_world;
        std::unordered_map<glm::ivec3, std::shared_ptr<EnchantmentTable>> m_tables;
        EnchantmentTableStats m_stats;
        bool m_initialized = false;
    };

    /**
     * @struct EnchantmentTableStats
     * @brief Statistics for enchantment tables
     */
    struct EnchantmentTableStats {
        int totalTables = 0;
        int activeTables = 0;
        int itemsEnchanted = 0;
        int totalEnchantmentsApplied = 0;
        int totalXPSpent = 0;
        int totalLapisUsed = 0;
        float averageBookshelves = 0.0f;
        std::unordered_map<EnchantmentType, int> enchantmentsByType;
        std::unordered_map<int, int> enchantmentLevelDistribution;
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_ENCHANTMENT_ENCHANTMENT_TABLE_HPP
