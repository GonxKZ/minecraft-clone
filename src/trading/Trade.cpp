/**
 * @file Trade.cpp
 * @brief VoxelCraft Trading System Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#include "Trade.hpp"
#include "../player/Player.hpp"
#include <algorithm>
#include <random>
#include <sstream>

namespace VoxelCraft {

// TradeManager implementation
TradeManager& TradeManager::GetInstance() {
    static TradeManager instance;
    return instance;
}

bool TradeManager::Initialize() {
    if (m_initialized) return true;

    m_config.enableTrading = true;
    m_config.enableWanderingTraders = true;
    m_config.enableVillagerTrading = true;
    m_config.enableTradeExperience = true;
    m_config.enableTradeLeveling = true;
    m_config.enableTradeRestocking = true;
    m_config.maxOffersPerMerchant = 6;
    m_config.maxUsesPerOffer = 12;
    m_config.restockTimeMinutes = 20;
    m_config.buyPriceMultiplier = 1.0f;
    m_config.sellPriceMultiplier = 1.0f;
    m_config.discountForReputation = 0.01f;
    m_config.maxMerchantLevel = 5;
    m_config.experiencePerTrade = 2;
    m_config.allowCustomTrades = true;

    m_nextOfferId = 1000;
    m_nextTransactionId = 1;

    InitializeDefaultOffers();
    m_initialized = true;

    return true;
}

void TradeManager::Shutdown() {
    m_defaultOffers.clear();
    m_customOffers.clear();
    m_transactionHistory.clear();
    m_stats = TradeStats();
    m_initialized = false;
}

void TradeManager::Update(float deltaTime) {
    // Update trade statistics
    m_stats.totalMerchants = 0; // This would be updated with actual merchant count
    m_stats.activeMerchants = 0; // This would be updated with active merchant count
}

MerchantProfile TradeManager::CreateMerchant(MerchantType type, int level) {
    MerchantProfile profile;
    profile.type = type;
    profile.profession = GetProfessionByMerchantType(type);
    profile.name = GenerateMerchantName(type);
    profile.level = std::max(1, std::min(level, m_config.maxMerchantLevel));
    profile.experience = 0;
    profile.maxExperience = GetExperienceForLevel(profile.level);
    profile.reputation = 0;
    profile.isWandering = (type == MerchantType::WANDERING_TRADER);

    // Generate initial offers
    profile.offers = GenerateOffersForMerchant(profile);

    return profile;
}

std::vector<TradeOffer> TradeManager::GetMerchantOffers(const MerchantProfile& merchant) const {
    std::vector<TradeOffer> availableOffers;

    for (const auto& offer : merchant.offers) {
        if (!offer.disabled && offer.uses < offer.maxUses &&
            merchant.level >= offer.minLevel && merchant.level <= offer.maxLevel) {
            availableOffers.push_back(offer);
        }
    }

    // Add unlocked custom offers
    for (int offerId : merchant.unlockedOffers) {
        for (const auto& offer : m_customOffers) {
            if (offer.offerId == offerId && !offer.disabled) {
                availableOffers.push_back(offer);
            }
        }
    }

    // Limit to maximum offers per merchant
    if (availableOffers.size() > m_config.maxOffersPerMerchant) {
        availableOffers.resize(m_config.maxOffersPerMerchant);
    }

    return availableOffers;
}

TradeTransaction TradeManager::ExecuteTrade(MerchantProfile& merchant, const TradeOffer& offer, Player* player) {
    TradeTransaction transaction;
    transaction.transactionId = GenerateTransactionId();
    transaction.offerId = offer.offerId;
    transaction.merchantType = merchant.type;
    transaction.merchantName = merchant.name;
    transaction.playerName = player ? player->GetName() : "Unknown";
    transaction.itemsGiven = offer.inputItems;
    transaction.itemsReceived = offer.outputItems;
    transaction.emeraldsSpent = CalculateOfferPrice(offer, merchant, player);
    transaction.experienceSpent = offer.experienceCost;
    transaction.timestamp = std::chrono::steady_clock::now();

    // Validate trade
    if (!ValidateTrade(offer, player)) {
        transaction.successful = false;
        transaction.failureReason = "Trade validation failed";
        m_stats.failedTrades++;
        return transaction;
    }

    // Process items
    if (!ProcessTradeItems(offer, player)) {
        transaction.successful = false;
        transaction.failureReason = "Insufficient items";
        m_stats.failedTrades++;
        return transaction;
    }

    // Spend emeralds/experience if required
    if (player) {
        // This would deduct emeralds and experience from player
        // player->RemoveEmeralds(transaction.emeraldsSpent);
        // player->RemoveExperience(transaction.experienceSpent);
    }

    // Update merchant
    UpdateMerchantAfterTrade(merchant, offer);

    // Update player reputation
    UpdatePlayerReputation(merchant, player, true);

    // Record transaction
    m_transactionHistory.push_back(transaction);
    m_stats.totalTrades++;
    m_stats.successfulTrades++;
    m_stats.totalEmeraldsTraded += transaction.emeraldsSpent;
    m_stats.totalExperienceGained += m_config.experiencePerTrade;

    return transaction;
}

bool TradeManager::CanPlayerUseOffer(const TradeOffer& offer, Player* player) const {
    if (!player) return false;

    // Check if player has required items
    for (const auto& item : offer.inputItems) {
        // This would check if player has the required items
        // For now, assume they do
    }

    // Check emerald cost
    int emeraldCost = CalculateOfferPrice(offer, MerchantProfile(), player);
    // if (player->GetEmeraldCount() < emeraldCost) return false;

    // Check experience cost
    // if (player->GetExperience() < offer.experienceCost) return false;

    return true;
}

bool TradeManager::LevelUpMerchant(MerchantProfile& merchant) {
    if (merchant.level >= m_config.maxMerchantLevel) return false;

    if (merchant.experience >= merchant.maxExperience) {
        merchant.level++;
        merchant.experience = 0;
        merchant.maxExperience = GetExperienceForLevel(merchant.level);

        // Generate new offers for higher level
        auto newOffers = GenerateOffersForMerchant(merchant);
        merchant.offers.insert(merchant.offers.end(), newOffers.begin(), newOffers.end());

        m_stats.merchantsLeveledUp++;
        return true;
    }

    return false;
}

void TradeManager::RestockMerchant(MerchantProfile& merchant) {
    auto now = std::chrono::steady_clock::now();
    auto timeSinceRestock = std::chrono::duration_cast<std::chrono::minutes>(
        now - merchant.lastRestock).count();

    if (timeSinceRestock >= m_config.restockTimeMinutes) {
        // Reset offer uses
        for (auto& offer : merchant.offers) {
            offer.uses = 0;
        }

        // Add new offers
        auto newOffers = GenerateOffersForMerchant(merchant);
        merchant.offers.insert(merchant.offers.end(), newOffers.begin(), newOffers.end());

        merchant.lastRestock = now;
        m_stats.offersRestocked++;
    }
}

int TradeManager::CalculateOfferPrice(const TradeOffer& offer, const MerchantProfile& merchant, Player* player) const {
    float basePrice = offer.emeraldCost * offer.priceMultiplier;

    // Apply merchant level multiplier
    float levelMultiplier = 1.0f + (merchant.level - 1) * 0.2f;
    basePrice *= levelMultiplier;

    // Apply reputation discount
    if (player) {
        float reputationDiscount = merchant.reputation * m_config.discountForReputation;
        basePrice *= (1.0f - reputationDiscount);
    }

    // Apply global multipliers
    basePrice *= m_config.buyPriceMultiplier;

    return static_cast<int>(std::max(1.0f, basePrice));
}

bool TradeManager::AddCustomTradeOffer(const TradeOffer& offer) {
    if (!m_config.allowCustomTrades) return false;

    m_customOffers.push_back(offer);
    m_stats.customOffers++;
    m_stats.totalOffers++;

    return true;
}

bool TradeManager::RemoveCustomTradeOffer(int offerId) {
    auto it = std::find_if(m_customOffers.begin(), m_customOffers.end(),
                          [offerId](const TradeOffer& offer) { return offer.offerId == offerId; });

    if (it != m_customOffers.end()) {
        m_customOffers.erase(it);
        m_stats.customOffers--;
        m_stats.totalOffers--;
        return true;
    }

    return false;
}

MerchantType TradeManager::GetMerchantTypeByProfession(const std::string& profession) {
    static std::unordered_map<std::string, MerchantType> professionMap = {
        {"Armorer", MerchantType::ARMORER},
        {"Butcher", MerchantType::BUTCHER},
        {"Cartographer", MerchantType::CARTOGRAPHER},
        {"Cleric", MerchantType::CLERIC},
        {"Farmer", MerchantType::FARMER},
        {"Fisherman", MerchantType::FISHERMAN},
        {"Fletcher", MerchantType::FLETCHER},
        {"Leatherworker", MerchantType::LEATHERWORKER},
        {"Librarian", MerchantType::LIBRARIAN},
        {"Mason", MerchantType::MASON},
        {"Nitwit", MerchantType::NITWIT},
        {"Shepherd", MerchantType::SHEPHERD},
        {"Toolsmith", MerchantType::TOOLSMITH},
        {"Weaponsmith", MerchantType::WEAPONSMITH},
        {"Wandering Trader", MerchantType::WANDERING_TRADER}
    };

    auto it = professionMap.find(profession);
    return (it != professionMap.end()) ? it->second : MerchantType::VILLAGER;
}

std::string TradeManager::GetProfessionByMerchantType(MerchantType type) {
    switch (type) {
        case MerchantType::ARMORER: return "Armorer";
        case MerchantType::BUTCHER: return "Butcher";
        case MerchantType::CARTOGRAPHER: return "Cartographer";
        case MerchantType::CLERIC: return "Cleric";
        case MerchantType::FARMER: return "Farmer";
        case MerchantType::FISHERMAN: return "Fisherman";
        case MerchantType::FLETCHER: return "Fletcher";
        case MerchantType::LEATHERWORKER: return "Leatherworker";
        case MerchantType::LIBRARIAN: return "Librarian";
        case MerchantType::MASON: return "Mason";
        case MerchantType::NITWIT: return "Nitwit";
        case MerchantType::SHEPHERD: return "Shepherd";
        case MerchantType::TOOLSMITH: return "Toolsmith";
        case MerchantType::WEAPONSMITH: return "Weaponsmith";
        case MerchantType::WANDERING_TRADER: return "Wandering Trader";
        default: return "Villager";
    }
}

void TradeManager::InitializeDefaultOffers() {
    // Initialize offers for each merchant type
    InitializeFarmerOffers();
    InitializeArmorerOffers();
    InitializeLibrarianOffers();
    InitializeClericOffers();
    InitializeWeaponsmithOffers();
    InitializeToolsmithOffers();
    InitializeLeatherworkerOffers();
    InitializeButcherOffers();
    InitializeCartographerOffers();
    InitializeMasonOffers();
    InitializeFletcherOffers();
    InitializeFishermanOffers();
    InitializeShepherdOffers();
    InitializeWanderingTraderOffers();
}

void TradeManager::InitializeFarmerOffers() {
    std::vector<TradeOffer> farmerOffers;

    // Level 1 offers
    farmerOffers.push_back(CreateOffer(TradeType::BUY_ITEM, TradeRarity::COMMON, 1, 1,
                                     {TradeItem(296, 20)}, {TradeItem(388, 1)}, 0, 0, 16)); // Wheat -> Emerald
    farmerOffers.push_back(CreateOffer(TradeType::SELL_ITEM, TradeRarity::COMMON, 1, 1,
                                     {TradeItem(388, 1)}, {TradeItem(391, 26)}, 0, 0, 16)); // Emerald -> Carrot

    // Level 2 offers
    farmerOffers.push_back(CreateOffer(TradeType::BUY_ITEM, TradeRarity::COMMON, 2, 2,
                                     {TradeItem(392, 18)}, {TradeItem(388, 1)}, 0, 0, 16)); // Potato -> Emerald
    farmerOffers.push_back(CreateOffer(TradeType::SELL_ITEM, TradeRarity::COMMON, 2, 2,
                                     {TradeItem(388, 1)}, {TradeItem(393, 10)}, 0, 0, 16)); // Emerald -> Beetroot

    m_defaultOffers[MerchantType::FARMER] = farmerOffers;
}

void TradeManager::InitializeArmorerOffers() {
    std::vector<TradeOffer> armorerOffers;

    // Level 1 offers
    armorerOffers.push_back(CreateOffer(TradeType::BUY_ITEM, TradeRarity::COMMON, 1, 1,
                                      {TradeItem(265, 15)}, {TradeItem(388, 1)}, 0, 0, 12)); // Iron -> Emerald
    armorerOffers.push_back(CreateOffer(TradeType::SELL_ITEM, TradeRarity::COMMON, 1, 1,
                                      {TradeItem(388, 1)}, {TradeItem(299, 1)}, 0, 0, 12)); // Emerald -> Leather Cap

    // Level 2 offers
    armorerOffers.push_back(CreateOffer(TradeType::BUY_ITEM, TradeRarity::COMMON, 2, 2,
                                      {TradeItem(264, 4)}, {TradeItem(388, 1)}, 0, 0, 12)); // Diamond -> Emerald
    armorerOffers.push_back(CreateOffer(TradeType::SELL_ITEM, TradeRarity::COMMON, 2, 2,
                                      {TradeItem(388, 1)}, {TradeItem(300, 1)}, 0, 0, 12)); // Emerald -> Leather Tunic

    m_defaultOffers[MerchantType::ARMORER] = armorerOffers;
}

void TradeManager::InitializeLibrarianOffers() {
    std::vector<TradeOffer> librarianOffers;

    // Level 1 offers
    librarianOffers.push_back(CreateOffer(TradeType::BUY_ITEM, TradeRarity::COMMON, 1, 1,
                                        {TradeItem(339, 24)}, {TradeItem(388, 1)}, 0, 0, 16)); // Paper -> Emerald
    librarianOffers.push_back(CreateOffer(TradeType::SELL_ITEM, TradeRarity::COMMON, 1, 1,
                                        {TradeItem(388, 1)}, {TradeItem(340, 1)}, 0, 0, 12)); // Emerald -> Book

    // Level 2 offers - Enchanted books
    librarianOffers.push_back(CreateOffer(TradeType::SELL_ITEM, TradeRarity::RARE, 2, 2,
                                        {TradeItem(388, 5)}, {TradeItem(403, 1)}, 0, 0, 6)); // Emerald -> Enchanted Book

    m_defaultOffers[MerchantType::LIBRARIAN] = librarianOffers;
}

void TradeManager::InitializeClericOffers() {
    std::vector<TradeOffer> clericOffers;

    // Level 1 offers
    clericOffers.push_back(CreateOffer(TradeType::BUY_ITEM, TradeRarity::COMMON, 1, 1,
                                     {TradeItem(352, 32)}, {TradeItem(388, 1)}, 0, 0, 16)); // Bone -> Emerald
    clericOffers.push_back(CreateOffer(TradeType::SELL_ITEM, TradeRarity::COMMON, 1, 1,
                                     {TradeItem(388, 1)}, {TradeItem(376, 1)}, 0, 0, 12)); // Emerald -> Redstone

    // Level 2 offers
    clericOffers.push_back(CreateOffer(TradeType::BUY_ITEM, TradeRarity::COMMON, 2, 2,
                                     {TradeItem(331, 2)}, {TradeItem(388, 1)}, 0, 0, 16)); // Redstone -> Emerald
    clericOffers.push_back(CreateOffer(TradeType::SELL_ITEM, TradeRarity::COMMON, 2, 2,
                                     {TradeItem(388, 1)}, {TradeItem(378, 1)}, 0, 0, 8)); // Emerald -> Magma Cream

    m_defaultOffers[MerchantType::CLERIC] = clericOffers;
}

void TradeManager::InitializeWeaponsmithOffers() {
    std::vector<TradeOffer> weaponsmithOffers;

    // Level 1 offers
    weaponsmithOffers.push_back(CreateOffer(TradeType::BUY_ITEM, TradeRarity::COMMON, 1, 1,
                                          {TradeItem(265, 7)}, {TradeItem(388, 1)}, 0, 0, 12)); // Iron -> Emerald
    weaponsmithOffers.push_back(CreateOffer(TradeType::SELL_ITEM, TradeRarity::COMMON, 1, 1,
                                          {TradeItem(388, 1)}, {TradeItem(267, 1)}, 0, 0, 12)); // Emerald -> Iron Sword

    // Level 2 offers
    weaponsmithOffers.push_back(CreateOffer(TradeType::BUY_ITEM, TradeRarity::COMMON, 2, 2,
                                          {TradeItem(264, 3)}, {TradeItem(388, 1)}, 0, 0, 12)); // Diamond -> Emerald
    weaponsmithOffers.push_back(CreateOffer(TradeType::SELL_ITEM, TradeRarity::COMMON, 2, 2,
                                          {TradeItem(388, 1)}, {TradeItem(276, 1)}, 0, 0, 3)); // Emerald -> Diamond Sword

    m_defaultOffers[MerchantType::WEAPONSMITH] = weaponsmithOffers;
}

void TradeManager::InitializeToolsmithOffers() {
    std::vector<TradeOffer> toolsmithOffers;

    // Level 1 offers
    toolsmithOffers.push_back(CreateOffer(TradeType::BUY_ITEM, TradeRarity::COMMON, 1, 1,
                                        {TradeItem(265, 7)}, {TradeItem(388, 1)}, 0, 0, 12)); // Iron -> Emerald
    toolsmithOffers.push_back(CreateOffer(TradeType::SELL_ITEM, TradeRarity::COMMON, 1, 1,
                                        {TradeItem(388, 1)}, {TradeItem(257, 1)}, 0, 0, 12)); // Emerald -> Iron Pickaxe

    // Level 2 offers
    toolsmithOffers.push_back(CreateOffer(TradeType::BUY_ITEM, TradeRarity::COMMON, 2, 2,
                                        {TradeItem(264, 3)}, {TradeItem(388, 1)}, 0, 0, 12)); // Diamond -> Emerald
    toolsmithOffers.push_back(CreateOffer(TradeType::SELL_ITEM, TradeRarity::COMMON, 2, 2,
                                        {TradeItem(388, 1)}, {TradeItem(278, 1)}, 0, 0, 3)); // Emerald -> Diamond Pickaxe

    m_defaultOffers[MerchantType::TOOLSMITH] = toolsmithOffers;
}

void TradeManager::InitializeLeatherworkerOffers() {
    std::vector<TradeOffer> leatherworkerOffers;

    // Level 1 offers
    leatherworkerOffers.push_back(CreateOffer(TradeType::BUY_ITEM, TradeRarity::COMMON, 1, 1,
                                            {TradeItem(334, 6)}, {TradeItem(388, 1)}, 0, 0, 16)); // Leather -> Emerald
    leatherworkerOffers.push_back(CreateOffer(TradeType::SELL_ITEM, TradeRarity::COMMON, 1, 1,
                                            {TradeItem(388, 1)}, {TradeItem(298, 1)}, 0, 0, 16)); // Emerald -> Leather Cap

    // Level 2 offers
    leatherworkerOffers.push_back(CreateOffer(TradeType::SELL_ITEM, TradeRarity::COMMON, 2, 2,
                                            {TradeItem(388, 1)}, {TradeItem(299, 1)}, 0, 0, 16)); // Emerald -> Leather Tunic

    m_defaultOffers[MerchantType::LEATHERWORKER] = leatherworkerOffers;
}

void TradeManager::InitializeButcherOffers() {
    std::vector<TradeOffer> butcherOffers;

    // Level 1 offers
    butcherOffers.push_back(CreateOffer(TradeType::BUY_ITEM, TradeRarity::COMMON, 1, 1,
                                      {TradeItem(363, 14)}, {TradeItem(388, 1)}, 0, 0, 16)); // Raw Beef -> Emerald
    butcherOffers.push_back(CreateOffer(TradeType::SELL_ITEM, TradeRarity::COMMON, 1, 1,
                                      {TradeItem(388, 1)}, {TradeItem(320, 6)}, 0, 0, 16)); // Emerald -> Cooked Beef

    // Level 2 offers
    butcherOffers.push_back(CreateOffer(TradeType::BUY_ITEM, TradeRarity::COMMON, 2, 2,
                                      {TradeItem(319, 7)}, {TradeItem(388, 1)}, 0, 0, 16)); // Raw Porkchop -> Emerald
    butcherOffers.push_back(CreateOffer(TradeType::SELL_ITEM, TradeRarity::COMMON, 2, 2,
                                      {TradeItem(388, 1)}, {TradeItem(364, 5)}, 0, 0, 16)); // Emerald -> Cooked Porkchop

    m_defaultOffers[MerchantType::BUTCHER] = butcherOffers;
}

void TradeManager::InitializeCartographerOffers() {
    std::vector<TradeOffer> cartographerOffers;

    // Level 1 offers
    cartographerOffers.push_back(CreateOffer(TradeType::BUY_ITEM, TradeRarity::COMMON, 1, 1,
                                           {TradeItem(339, 24)}, {TradeItem(388, 1)}, 0, 0, 16)); // Paper -> Emerald
    cartographerOffers.push_back(CreateOffer(TradeType::SELL_ITEM, TradeRarity::COMMON, 1, 1,
                                           {TradeItem(388, 1)}, {TradeItem(395, 1)}, 0, 0, 12)); // Emerald -> Empty Map

    // Level 2 offers
    cartographerOffers.push_back(CreateOffer(TradeType::BUY_ITEM, TradeRarity::COMMON, 2, 2,
                                           {TradeItem(19, 8)}, {TradeItem(388, 1)}, 0, 0, 12)); // Sponge -> Emerald
    cartographerOffers.push_back(CreateOffer(TradeType::SELL_ITEM, TradeRarity::COMMON, 2, 2,
                                           {TradeItem(388, 1)}, {TradeItem(358, 1)}, 0, 0, 8)); // Emerald -> Filled Map

    m_defaultOffers[MerchantType::CARTOGRAPHER] = cartographerOffers;
}

void TradeManager::InitializeMasonOffers() {
    std::vector<TradeOffer> masonOffers;

    // Level 1 offers
    masonOffers.push_back(CreateOffer(TradeType::BUY_ITEM, TradeRarity::COMMON, 1, 1,
                                    {TradeItem(4, 20)}, {TradeItem(388, 1)}, 0, 0, 16)); // Cobblestone -> Emerald
    masonOffers.push_back(CreateOffer(TradeType::SELL_ITEM, TradeRarity::COMMON, 1, 1,
                                    {TradeItem(388, 1)}, {TradeItem(1, 10)}, 0, 0, 16)); // Emerald -> Stone

    // Level 2 offers
    masonOffers.push_back(CreateOffer(TradeType::BUY_ITEM, TradeRarity::COMMON, 2, 2,
                                    {TradeItem(1, 16)}, {TradeItem(388, 1)}, 0, 0, 16)); // Stone -> Emerald
    masonOffers.push_back(CreateOffer(TradeType::SELL_ITEM, TradeRarity::COMMON, 2, 2,
                                    {TradeItem(388, 1)}, {TradeItem(98, 4)}, 0, 0, 16)); // Emerald -> Stone Bricks

    m_defaultOffers[MerchantType::MASON] = masonOffers;
}

void TradeManager::InitializeFletcherOffers() {
    std::vector<TradeOffer> fletcherOffers;

    // Level 1 offers
    fletcherOffers.push_back(CreateOffer(TradeType::BUY_ITEM, TradeRarity::COMMON, 1, 1,
                                       {TradeItem(280, 32)}, {TradeItem(388, 1)}, 0, 0, 16)); // Stick -> Emerald
    fletcherOffers.push_back(CreateOffer(TradeType::SELL_ITEM, TradeRarity::COMMON, 1, 1,
                                       {TradeItem(388, 1)}, {TradeItem(262, 16)}, 0, 0, 16)); // Emerald -> Arrow

    // Level 2 offers
    fletcherOffers.push_back(CreateOffer(TradeType::BUY_ITEM, TradeRarity::COMMON, 2, 2,
                                       {TradeItem(318, 26)}, {TradeItem(388, 1)}, 0, 0, 16)); // Flint -> Emerald
    fletcherOffers.push_back(CreateOffer(TradeType::SELL_ITEM, TradeRarity::COMMON, 2, 2,
                                       {TradeItem(388, 1)}, {TradeItem(261, 1)}, 0, 0, 12)); // Emerald -> Bow

    m_defaultOffers[MerchantType::FLETCHER] = fletcherOffers;
}

void TradeManager::InitializeFishermanOffers() {
    std::vector<TradeOffer> fishermanOffers;

    // Level 1 offers
    fishermanOffers.push_back(CreateOffer(TradeType::BUY_ITEM, TradeRarity::COMMON, 1, 1,
                                        {TradeItem(349, 6)}, {TradeItem(388, 1)}, 0, 0, 16)); // Raw Cod -> Emerald
    fishermanOffers.push_back(CreateOffer(TradeType::SELL_ITEM, TradeRarity::COMMON, 1, 1,
                                        {TradeItem(388, 1)}, {TradeItem(346, 1)}, 0, 0, 16)); // Emerald -> Fishing Rod

    // Level 2 offers
    fishermanOffers.push_back(CreateOffer(TradeType::BUY_ITEM, TradeRarity::COMMON, 2, 2,
                                        {TradeItem(350, 6)}, {TradeItem(388, 1)}, 0, 0, 16)); // Raw Salmon -> Emerald
    fishermanOffers.push_back(CreateOffer(TradeType::SELL_ITEM, TradeRarity::COMMON, 2, 2,
                                        {TradeItem(388, 1)}, {TradeItem(345, 1)}, 0, 0, 12)); // Emerald -> Enchanted Fishing Rod

    m_defaultOffers[MerchantType::FISHERMAN] = fishermanOffers;
}

void TradeManager::InitializeShepherdOffers() {
    std::vector<TradeOffer> shepherdOffers;

    // Level 1 offers
    shepherdOffers.push_back(CreateOffer(TradeType::BUY_ITEM, TradeRarity::COMMON, 1, 1,
                                       {TradeItem(35, 18)}, {TradeItem(388, 1)}, 0, 0, 16)); // Wool -> Emerald
    shepherdOffers.push_back(CreateOffer(TradeType::SELL_ITEM, TradeRarity::COMMON, 1, 1,
                                       {TradeItem(388, 1)}, {TradeItem(287, 1)}, 0, 0, 16)); // Emerald -> String

    // Level 2 offers
    shepherdOffers.push_back(CreateOffer(TradeType::SELL_ITEM, TradeRarity::COMMON, 2, 2,
                                       {TradeItem(388, 1)}, {TradeItem(35, 1)}, 0, 0, 16)); // Emerald -> Colored Wool

    m_defaultOffers[MerchantType::SHEPHERD] = shepherdOffers;
}

void TradeManager::InitializeWanderingTraderOffers() {
    std::vector<TradeOffer> wanderingTraderOffers;

    // Wandering traders have unique offers
    wanderingTraderOffers.push_back(CreateOffer(TradeType::SELL_ITEM, TradeRarity::RARE, 1, 1,
                                              {TradeItem(388, 5)}, {TradeItem(368, 1)}, 0, 0, 1)); // Emerald -> Ender Pearl
    wanderingTraderOffers.push_back(CreateOffer(TradeType::SELL_ITEM, TradeRarity::RARE, 1, 1,
                                              {TradeItem(388, 1)}, {TradeItem(50, 1)}, 0, 0, 1)); // Emerald -> Torch
    wanderingTraderOffers.push_back(CreateOffer(TradeType::SELL_ITEM, TradeRarity::EPIC, 1, 1,
                                              {TradeItem(388, 20)}, {TradeItem(38, 1)}, 0, 0, 1)); // Emerald -> Flower

    m_defaultOffers[MerchantType::WANDERING_TRADER] = wanderingTraderOffers;
}

TradeOffer TradeManager::CreateOffer(TradeType type, TradeRarity rarity, int minLevel, int maxLevel,
                                   const std::vector<TradeItem>& input, const std::vector<TradeItem>& output,
                                   int emeraldCost, int expCost, int maxUses) {
    TradeOffer offer;
    offer.offerId = GenerateOfferId();
    offer.type = type;
    offer.rarity = rarity;
    offer.inputItems = input;
    offer.outputItems = output;
    offer.emeraldCost = emeraldCost;
    offer.experienceCost = expCost;
    offer.maxUses = maxUses;
    offer.uses = 0;
    offer.minLevel = minLevel;
    offer.maxLevel = maxLevel;
    offer.priceMultiplier = 1.0f;

    return offer;
}

bool TradeManager::ValidateTrade(const TradeOffer& offer, Player* player) const {
    if (!player) return false;

    // Check if offer is disabled
    if (offer.disabled) return false;

    // Check if offer has uses left
    if (offer.uses >= offer.maxUses) return false;

    // Check if player has required items
    for (const auto& item : offer.inputItems) {
        // This would check player's inventory
        // For now, assume they have the items
    }

    return true;
}

bool TradeManager::ProcessTradeItems(const TradeOffer& offer, Player* player) {
    if (!player) return false;

    // Remove input items from player
    for (const auto& item : offer.inputItems) {
        // This would remove items from player's inventory
        // player->RemoveItem(item.itemID, item.count);
    }

    // Add output items to player
    for (const auto& item : offer.outputItems) {
        // This would add items to player's inventory
        // player->AddItem(item.itemID, item.count);
    }

    return true;
}

void TradeManager::UpdateMerchantAfterTrade(MerchantProfile& merchant, const TradeOffer& offer) {
    // Find the offer and update it
    for (auto& merchantOffer : merchant.offers) {
        if (merchantOffer.offerId == offer.offerId) {
            merchantOffer.uses++;
            merchantOffer.lastUsed = std::chrono::steady_clock::now();
            break;
        }
    }

    // Add experience to merchant
    merchant.experience += m_config.experiencePerTrade;

    // Check if merchant should level up
    if (m_config.enableTradeLeveling) {
        LevelUpMerchant(merchant);
    }
}

void TradeManager::UpdatePlayerReputation(MerchantProfile& merchant, Player* player, bool successful) {
    if (!player) return;

    if (successful) {
        merchant.reputation = std::min(100, merchant.reputation + 1);
    } else {
        merchant.reputation = std::max(-100, merchant.reputation - 1);
    }
}

std::vector<TradeOffer> TradeManager::GenerateOffersForMerchant(const MerchantProfile& merchant) const {
    auto it = m_defaultOffers.find(merchant.type);
    if (it == m_defaultOffers.end()) return {};

    std::vector<TradeOffer> offers;

    // Add offers that match the merchant's level
    for (const auto& offer : it->second) {
        if (merchant.level >= offer.minLevel && merchant.level <= offer.maxLevel) {
            offers.push_back(offer);
        }
    }

    // Limit to maximum offers per merchant
    if (offers.size() > m_config.maxOffersPerMerchant) {
        offers.resize(m_config.maxOffersPerMerchant);
    }

    return offers;
}

int TradeManager::GenerateOfferId() {
    return m_nextOfferId++;
}

int TradeManager::GenerateTransactionId() {
    return m_nextTransactionId++;
}

float TradeManager::GetOfferRarityWeight(TradeRarity rarity) const {
    switch (rarity) {
        case TradeRarity::COMMON: return 1.0f;
        case TradeRarity::UNCOMMON: return 0.5f;
        case TradeRarity::RARE: return 0.2f;
        case TradeRarity::EPIC: return 0.1f;
        case TradeRarity::LEGENDARY: return 0.05f;
        default: return 1.0f;
    }
}

int TradeManager::GetExperienceForLevel(int level) const {
    // Experience required for each level
    switch (level) {
        case 1: return 10;
        case 2: return 70;
        case 3: return 150;
        case 4: return 250;
        case 5: return 500;
        default: return 10;
    }
}

std::string TradeManager::GenerateMerchantName(MerchantType type) {
    static std::vector<std::string> firstNames = {
        "Steve", "Alex", "Bob", "Emma", "Oliver", "Sophia", "Liam", "Olivia", "Noah", "Ava"
    };

    static std::vector<std::string> lastNames = {
        "Smith", "Johnson", "Brown", "Williams", "Jones", "Garcia", "Miller", "Davis", "Wilson", "Martinez"
    };

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> firstDist(0, firstNames.size() - 1);
    std::uniform_int_distribution<size_t> lastDist(0, lastNames.size() - 1);

    return firstNames[firstDist(gen)] + " " + lastNames[lastDist(gen)];
}

} // namespace VoxelCraft
