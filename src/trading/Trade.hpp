/**
 * @file Trade.hpp
 * @brief VoxelCraft Trading System - Trading mechanics and offers
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_TRADING_TRADE_HPP
#define VOXELCRAFT_TRADING_TRADE_HPP

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>
#include <array>
#include <chrono>
#include <glm/glm.hpp>

#include "../entities/Entity.hpp"

namespace VoxelCraft {

    class Player;
    class Villager;
    class WanderingTrader;

    /**
     * @enum TradeType
     * @brief Types of trades available
     */
    enum class TradeType {
        BUY_ITEM = 0,              ///< Villager buys item from player
        SELL_ITEM,                 ///< Villager sells item to player
        ENCHANT_ITEM,              ///< Enchant an item
        CURE_ZOMBIE,               ///< Cure zombie villager
        BUY_MAP,                   ///< Buy exploration map
        SELL_MAP,                  ///< Sell exploration map
        BUY_EXPERIENCE,            ///< Buy experience
        SELL_EXPERIENCE,           ///< Sell experience
        CUSTOM_TRADE              ///< Custom trade logic
    };

    /**
     * @enum TradeRarity
     * @brief Rarity levels for trades
     */
    enum class TradeRarity {
        COMMON = 0,               ///< Common trades
        UNCOMMON,                 ///< Uncommon trades
        RARE,                    ///< Rare trades
        EPIC,                    ///< Epic trades
        LEGENDARY                ///< Legendary trades
    };

    /**
     * @enum MerchantType
     * @brief Types of merchants
     */
    enum class MerchantType {
        VILLAGER = 0,             ///< Regular villager
        WANDERING_TRADER,         ///< Wandering trader
        ARMORER,                  ///< Armorer villager
        BUTCHER,                  ///< Butcher villager
        CARTOGRAPHER,             ///< Cartographer villager
        CLERIC,                   ///< Cleric villager
        FARMER,                   ///< Farmer villager
        FISHERMAN,                ///< Fisherman villager
        FLETCHER,                 ///< Fletcher villager
        LEATHERWORKER,            ///< Leatherworker villager
        LIBRARIAN,                ///< Librarian villager
        MASON,                    ///< Mason villager
        NITWIT,                   ///< Nitwit villager
        SHEPHERD,                 ///< Shepherd villager
        TOOLSMITH,                ///< Toolsmith villager
        WEAPONSMITH              ///< Weaponsmith villager
    };

    /**
     * @struct TradeItem
     * @brief An item in a trade
     */
    struct TradeItem {
        int itemID;                       ///< Item ID
        int count;                        ///< Item count
        int data;                         ///< Item data/damage
        std::string itemName;             ///< Item display name
        std::unordered_map<std::string, int> enchantments; ///< Item enchantments
        std::unordered_map<std::string, std::any> customProperties;

        TradeItem() : itemID(0), count(1), data(0) {}
        TradeItem(int id, int cnt = 1, int dmg = 0, const std::string& name = "")
            : itemID(id), count(cnt), data(dmg), itemName(name) {}
    };

    /**
     * @struct TradeOffer
     * @brief A single trade offer
     */
    struct TradeOffer {
        int offerId;                      ///< Unique offer ID
        TradeType type;                   ///< Type of trade
        TradeRarity rarity;               ///< Rarity of the offer
        std::vector<TradeItem> inputItems; ///< Items player must give
        std::vector<TradeItem> outputItems; ///< Items player receives
        int emeraldCost;                  ///< Emerald cost (if applicable)
        int experienceCost;               ///< Experience cost (if applicable)
        int maxUses;                      ///< Maximum uses of this offer
        int uses;                         ///< Current uses
        bool disabled;                    ///< Whether offer is disabled
        int minLevel;                     ///< Minimum merchant level required
        int maxLevel;                     ///< Maximum merchant level for this offer
        float priceMultiplier;            ///< Price multiplier
        std::chrono::steady_clock::time_point lastUsed; ///< Last time offer was used
        std::unordered_map<std::string, std::any> customData;

        TradeOffer() : offerId(0), type(TradeType::BUY_ITEM), rarity(TradeRarity::COMMON),
                      emeraldCost(0), experienceCost(0), maxUses(12), uses(0), disabled(false),
                      minLevel(1), maxLevel(5), priceMultiplier(1.0f),
                      lastUsed(std::chrono::steady_clock::now()) {}
    };

    /**
     * @struct MerchantProfile
     * @brief Profile of a merchant
     */
    struct MerchantProfile {
        MerchantType type;               ///< Type of merchant
        std::string name;                ///< Merchant name
        std::string profession;          ///< Profession name
        int level;                       ///< Merchant level (1-5)
        int experience;                  ///< Current experience
        int maxExperience;               ///< Experience needed for next level
        int reputation;                  ///< Player reputation with this merchant
        std::vector<TradeOffer> offers;  ///< Available trade offers
        std::vector<int> unlockedOffers; ///< IDs of unlocked offers
        std::chrono::steady_clock::time_point lastRestock; ///< Last restock time
        bool isWandering;                ///< Whether this is a wandering trader
        glm::vec3 homePosition;          ///< Home position for villagers
        float homeRadius;                ///< Home radius
        std::unordered_map<std::string, int> tradeStatistics; ///< Trade statistics

        MerchantProfile() : type(MerchantType::VILLAGER), level(1), experience(0),
                           maxExperience(10), reputation(0), isWandering(false),
                           homeRadius(16.0f), lastRestock(std::chrono::steady_clock::now()) {}
    };

    /**
     * @struct TradeTransaction
     * @brief A completed trade transaction
     */
    struct TradeTransaction {
        int transactionId;               ///< Unique transaction ID
        int offerId;                     ///< Offer ID that was used
        MerchantType merchantType;       ///< Type of merchant
        std::string merchantName;        ///< Merchant name
        std::string playerName;          ///< Player name
        std::vector<TradeItem> itemsGiven; ///< Items player gave
        std::vector<TradeItem> itemsReceived; ///< Items player received
        int emeraldsSpent;               ///< Emeralds spent
        int experienceSpent;             ///< Experience spent
        std::chrono::steady_clock::time_point timestamp; ///< Transaction time
        bool successful;                 ///< Whether transaction was successful
        std::string failureReason;       ///< Reason for failure (if any)

        TradeTransaction() : transactionId(0), offerId(0), merchantType(MerchantType::VILLAGER),
                            emeraldsSpent(0), experienceSpent(0), successful(true),
                            timestamp(std::chrono::steady_clock::now()) {}
    };

    /**
     * @struct TradeConfiguration
     * @brief Configuration for trading system
     */
    struct TradeConfiguration {
        bool enableTrading;              ///< Whether trading is enabled
        bool enableWanderingTraders;     ///< Whether wandering traders spawn
        bool enableVillagerTrading;      ///< Whether villagers can trade
        bool enableTradeExperience;      ///< Whether trading gives experience
        bool enableTradeLeveling;        ///< Whether merchants level up
        bool enableTradeRestocking;      ///< Whether offers restock over time
        int maxOffersPerMerchant;        ///< Maximum offers per merchant
        int maxUsesPerOffer;             ///< Maximum uses per offer
        int restockTimeMinutes;          ///< Time between restocks (minutes)
        float buyPriceMultiplier;        ///< Multiplier for buy prices
        float sellPriceMultiplier;       ///< Multiplier for sell prices
        float discountForReputation;     ///< Price discount per reputation point
        int maxMerchantLevel;            ///< Maximum merchant level
        int experiencePerTrade;          ///< Experience gained per successful trade
        bool allowCustomTrades;          ///< Whether custom trades are allowed
        std::vector<MerchantType> disabledMerchants; ///< Disabled merchant types
    };

    /**
     * @class TradeManager
     * @brief Manager for all trade-related functionality
     */
    class TradeManager {
    public:
        /**
         * @brief Get singleton instance
         * @return TradeManager instance
         */
        static TradeManager& GetInstance();

        /**
         * @brief Initialize trade manager
         * @return true if successful
         */
        bool Initialize();

        /**
         * @brief Shutdown trade manager
         */
        void Shutdown();

        /**
         * @brief Update trade system
         * @param deltaTime Time since last update
         */
        void Update(float deltaTime);

        /**
         * @brief Create merchant profile
         * @param type Merchant type
         * @param level Starting level
         * @return New merchant profile
         */
        MerchantProfile CreateMerchant(MerchantType type, int level = 1);

        /**
         * @brief Get trade offers for merchant
         * @param merchant Merchant profile
         * @return Vector of available offers
         */
        std::vector<TradeOffer> GetMerchantOffers(const MerchantProfile& merchant) const;

        /**
         * @brief Execute trade
         * @param merchant Merchant profile
         * @param offer Trade offer
         * @param player Player executing trade
         * @return Trade transaction result
         */
        TradeTransaction ExecuteTrade(MerchantProfile& merchant, const TradeOffer& offer, Player* player);

        /**
         * @brief Check if player can use offer
         * @param offer Trade offer
         * @param player Player to check
         * @return true if can use
         */
        bool CanPlayerUseOffer(const TradeOffer& offer, Player* player) const;

        /**
         * @brief Level up merchant
         * @param merchant Merchant profile
         * @return true if leveled up
         */
        bool LevelUpMerchant(MerchantProfile& merchant);

        /**
         * @brief Restock merchant offers
         * @param merchant Merchant profile
         */
        void RestockMerchant(MerchantProfile& merchant);

        /**
         * @brief Calculate offer price
         * @param offer Trade offer
         * @param merchant Merchant profile
         * @param player Player (for reputation discount)
         * @return Final price in emeralds
         */
        int CalculateOfferPrice(const TradeOffer& offer, const MerchantProfile& merchant, Player* player) const;

        /**
         * @brief Get trade statistics
         * @return Trade statistics
         */
        const TradeStats& GetStats() const { return m_stats; }

        /**
         * @brief Set trading enabled/disabled
         * @param enabled Whether trading is enabled
         */
        void SetTradingEnabled(bool enabled) { m_config.enableTrading = enabled; }

        /**
         * @brief Check if trading is enabled
         * @return true if enabled
         */
        bool IsTradingEnabled() const { return m_config.enableTrading; }

        /**
         * @brief Add custom trade offer
         * @param offer Trade offer to add
         * @return true if added successfully
         */
        bool AddCustomTradeOffer(const TradeOffer& offer);

        /**
         * @brief Remove custom trade offer
         * @param offerId Offer ID to remove
         * @return true if removed successfully
         */
        bool RemoveCustomTradeOffer(int offerId);

        /**
         * @brief Get merchant type by profession
         * @param profession Profession name
         * @return Merchant type
         */
        static MerchantType GetMerchantTypeByProfession(const std::string& profession);

        /**
         * @brief Get profession by merchant type
         * @param type Merchant type
         * @return Profession name
         */
        static std::string GetProfessionByMerchantType(MerchantType type);

    private:
        TradeManager() = default;
        ~TradeManager() = default;

        // Prevent copying
        TradeManager(const TradeManager&) = delete;
        TradeManager& operator=(const TradeManager&) = delete;

        TradeConfiguration m_config;
        TradeStats m_stats;
        bool m_initialized;
        int m_nextOfferId;
        int m_nextTransactionId;

        std::unordered_map<MerchantType, std::vector<TradeOffer>> m_defaultOffers;
        std::vector<TradeOffer> m_customOffers;
        std::vector<TradeTransaction> m_transactionHistory;

        // Initialization
        void InitializeDefaultOffers();
        void InitializeConfiguration();

        // Trade execution helpers
        bool ValidateTrade(const TradeOffer& offer, Player* player) const;
        bool ProcessTradeItems(const TradeOffer& offer, Player* player);
        void UpdateMerchantAfterTrade(MerchantProfile& merchant, const TradeOffer& offer);
        void UpdatePlayerReputation(MerchantProfile& merchant, Player* player, bool successful);

        // Offer generation
        std::vector<TradeOffer> GenerateOffersForMerchant(const MerchantProfile& merchant) const;
        TradeOffer GenerateRandomOffer(MerchantType type, int level, TradeRarity rarity) const;
        bool IsOfferCompatible(const TradeOffer& offer, const MerchantProfile& merchant) const;

        // Utility functions
        int GenerateOfferId();
        int GenerateTransactionId();
        float GetOfferRarityWeight(TradeRarity rarity) const;
        int GetExperienceForLevel(int level) const;
    };

    /**
     * @struct TradeStats
     * @brief Statistics for trading system
     */
    struct TradeStats {
        size_t totalTrades = 0;
        size_t successfulTrades = 0;
        size_t failedTrades = 0;
        size_t totalMerchants = 0;
        size_t activeMerchants = 0;
        size_t totalOffers = 0;
        size_t customOffers = 0;
        int totalEmeraldsTraded = 0;
        int totalExperienceGained = 0;
        float averageTradeValue = 0.0f;
        std::unordered_map<MerchantType, size_t> tradesByMerchantType;
        std::unordered_map<TradeType, size_t> tradesByType;
        std::unordered_map<TradeRarity, size_t> tradesByRarity;
        std::vector<std::pair<std::string, size_t>> mostPopularItems;
        int merchantsLeveledUp = 0;
        int offersRestocked = 0;
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_TRADING_TRADE_HPP
