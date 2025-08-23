/**
 * @file EnchantmentTable.cpp
 * @brief VoxelCraft Enchantment Table System Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#include "EnchantmentTable.hpp"
#include "../world/World.hpp"
#include "../player/Player.hpp"
#include "../inventory/Inventory.hpp"
#include <algorithm>
#include <random>
#include <cmath>

namespace VoxelCraft {

// EnchantmentTable implementation
EnchantmentTable::EnchantmentTable(const glm::ivec3& position, World* world)
    : m_position(position)
    , m_world(world)
    , m_currentPlayer(nullptr)
    , m_enchantmentProgress(0.0f)
    , m_glowIntensity(0.0f)
    , m_bookshelfScanTimer(0.0f)
    , m_lastUpdate(std::chrono::steady_clock::now())
{
    m_config.enableEnchantmentTable = true;
    m_config.maxBookshelves = 15;
    m_config.minXPLevel = 1;
    m_config.maxXPLevel = 30;
    m_config.baseEnchantmentCost = 1.0f;
    m_config.bookshelfMultiplier = 1.15f;
    m_config.minLapisCost = 1;
    m_config.maxLapisCost = 3;
    m_config.enchantmentTime = 2.0f;
    m_config.requireBookshelves = false;
    m_config.enableTreasureEnchantments = true;
}

EnchantmentTable::~EnchantmentTable() {
    if (m_currentPlayer) {
        CloseInterface();
    }
}

void EnchantmentTable::Update(float deltaTime) {
    auto now = std::chrono::steady_clock::now();
    auto timeSinceLastUpdate = std::chrono::duration<float>(now - m_lastUpdate).count();
    m_lastUpdate = now;

    // Update enchantment progress
    if (m_state.isEnchanting) {
        m_enchantmentProgress += deltaTime;
        if (m_enchantmentProgress >= m_config.enchantmentTime) {
            m_state.isEnchanting = false;
            m_enchantmentProgress = 0.0f;
        }
    }

    // Update visual effects
    UpdateVisualEffects(deltaTime);

    // Scan for bookshelves periodically
    m_bookshelfScanTimer += deltaTime;
    if (m_bookshelfScanTimer >= 5.0f) {
        m_bookshelfScanTimer = 0.0f;
        ScanForBookshelves();
    }

    // Update enchantment options if item changed
    if (m_currentPlayer && m_state.slots[static_cast<int>(EnchantmentSlot::INPUT)] != 0) {
        GenerateEnchantmentOptions();
    }
}

bool EnchantmentTable::OpenInterface(Player* player) {
    if (!player || m_currentPlayer) {
        return false;
    }

    m_currentPlayer = player;
    m_state = EnchantmentTableState(); // Reset state
    m_state.enchantmentSeed = GenerateEnchantmentSeed();

    ScanForBookshelves();

    return true;
}

void EnchantmentTable::CloseInterface() {
    if (m_currentPlayer) {
        // Return any items in the table
        for (int i = 0; i < 3; ++i) {
            if (m_state.slots[i] != 0 && m_state.slotCounts[i] > 0) {
                // Return item to player inventory
                // This would typically interact with the player's inventory system
            }
        }

        m_currentPlayer = nullptr;
        m_state = EnchantmentTableState();
    }
}

bool EnchantmentTable::SetItem(EnchantmentSlot slot, int itemID, int count) {
    int slotIndex = static_cast<int>(slot);

    if (slotIndex < 0 || slotIndex >= 3) return false;

    // Validate item placement
    switch (slot) {
        case EnchantmentSlot::INPUT:
            if (!CanEnchantItem(itemID)) return false;
            break;
        case EnchantmentSlot::LAPIS_LAZULI:
            if (itemID != 351) return false; // Lapis lazuli
            break;
        case EnchantmentSlot::OUTPUT:
            return false; // Output slot is read-only
            break;
    }

    m_state.slots[slotIndex] = itemID;
    m_state.slotCounts[slotIndex] = count;

    // Clear output slot when input changes
    if (slot == EnchantmentSlot::INPUT) {
        m_state.slots[static_cast<int>(EnchantmentSlot::OUTPUT)] = 0;
        m_state.slotCounts[static_cast<int>(EnchantmentSlot::OUTPUT)] = 0;
        m_state.selectedOption = -1;
    }

    return true;
}

std::pair<int, int> EnchantmentTable::GetItem(EnchantmentSlot slot) const {
    int slotIndex = static_cast<int>(slot);

    if (slotIndex < 0 || slotIndex >= 3) {
        return {0, 0};
    }

    return {m_state.slots[slotIndex], m_state.slotCounts[slotIndex]};
}

bool EnchantmentTable::SelectOption(int optionIndex) {
    if (optionIndex < 0 || optionIndex >= static_cast<int>(m_state.options.size())) {
        return false;
    }

    m_state.selectedOption = optionIndex;

    // Create output item preview
    const auto& option = m_state.options[optionIndex];
    int inputItemID = m_state.slots[static_cast<int>(EnchantmentSlot::INPUT)];

    // This would create an enchanted version of the input item
    // For now, just set a placeholder enchanted item
    m_state.slots[static_cast<int>(EnchantmentSlot::OUTPUT)] = inputItemID;
    m_state.slotCounts[static_cast<int>(EnchantmentSlot::OUTPUT)] = 1;

    return true;
}

bool EnchantmentTable::PerformEnchanting() {
    if (!m_currentPlayer || m_state.selectedOption < 0) {
        return false;
    }

    if (m_state.isEnchanting) {
        return false;
    }

    const auto& option = m_state.options[m_state.selectedOption];

    // Check if player has enough XP
    if (m_currentPlayer->GetXPLevel() < option.levelRequirement) {
        return false;
    }

    // Check if player has enough lapis
    auto [lapisID, lapisCount] = GetItem(EnchantmentSlot::LAPIS_LAZULI);
    if (lapisCount < option.lapisCost) {
        return false;
    }

    // Consume materials
    if (!ConsumeMaterials(option)) {
        return false;
    }

    // Spend XP
    m_currentPlayer->SetXPLevel(m_currentPlayer->GetXPLevel() - option.xpCost);

    // Start enchanting animation
    m_state.isEnchanting = true;
    m_enchantmentProgress = 0.0f;

    // Play effects
    PlayEnchantmentEffects();

    return true;
}

float EnchantmentTable::CalculateEnchantmentPower() const {
    // Base power calculation based on bookshelves
    float power = 0.0f;

    if (m_state.bookshelfCount >= 15) {
        power = 3.0f; // Maximum power
    } else if (m_state.bookshelfCount >= 8) {
        power = 2.0f;
    } else if (m_state.bookshelfCount >= 1) {
        power = 1.0f;
    }

    return power;
}

void EnchantmentTable::GenerateEnchantmentOptions() {
    m_state.options.clear();

    int inputItemID = m_state.slots[static_cast<int>(EnchantmentSlot::INPUT)];
    if (inputItemID == 0) return;

    int playerXP = m_currentPlayer ? m_currentPlayer->GetXPLevel() : 0;
    int baseLevel = CalculateBaseEnchantmentLevel(playerXP);

    // Generate 3 enchantment options
    for (int i = 0; i < 3; ++i) {
        EnchantmentOption option = CreateEnchantmentOption(baseLevel, inputItemID);

        // Adjust option based on slot position
        if (i == 0) {
            // First option is always available
        } else if (i == 1) {
            // Second option requires more bookshelves
            if (m_state.bookshelfCount < 8) {
                option.enchantments.clear();
            }
        } else if (i == 2) {
            // Third option requires maximum bookshelves
            if (m_state.bookshelfCount < 15) {
                option.enchantments.clear();
            }
        }

        m_state.options.push_back(option);
    }
}

bool EnchantmentTable::CanEnchantItem(int itemID) const {
    // Check if item is enchantable
    if (itemID == 0) return false;

    // Tools, weapons, armor, and some special items can be enchanted
    bool isTool = (itemID >= 256 && itemID <= 294);
    bool isWeapon = (itemID >= 267 && itemID <= 279) || (itemID >= 283 && itemID <= 286);
    bool isArmor = (itemID >= 298 && itemID <= 317);
    bool isBow = (itemID == 261);
    bool isFishingRod = (itemID == 346);

    return isTool || isWeapon || isArmor || isBow || isFishingRod;
}

int EnchantmentTable::GetRequiredLapis(int itemID) const {
    // Base lapis cost depends on item type
    int baseCost = 1;

    if (itemID >= 298 && itemID <= 317) { // Armor
        baseCost = 2;
    } else if (itemID == 261) { // Bow
        baseCost = 1;
    } else if (itemID >= 267 && itemID <= 279) { // Swords
        baseCost = 1;
    }

    // Increase cost with more bookshelves
    int bookshelfMultiplier = std::min(m_state.bookshelfCount / 8, 2);
    return std::min(baseCost + bookshelfMultiplier, m_config.maxLapisCost);
}

int EnchantmentTable::GetXPCost(const EnchantmentOption& option) const {
    float baseCost = m_config.baseEnchantmentCost * option.levelRequirement;
    float bookshelfMultiplier = std::pow(m_config.bookshelfMultiplier, std::min(m_state.bookshelfCount, 15));
    return static_cast<int>(baseCost * bookshelfMultiplier);
}

bool EnchantmentTable::CanPlayerEnchant(Player* player) const {
    if (!player) return false;

    return player->GetXPLevel() >= m_config.minXPLevel;
}

int EnchantmentTable::CountBookshelves() const {
    if (!m_world) return 0;

    int count = 0;
    const int radius = 2; // 2 blocks in each direction

    // Check all positions within 2 blocks horizontally and 1 block vertically
    for (int x = -radius; x <= radius; ++x) {
        for (int z = -radius; z <= radius; ++z) {
            for (int y = 0; y <= 1; ++y) {
                if (x == 0 && z == 0 && y == 0) continue; // Skip the table itself

                glm::ivec3 checkPos = m_position + glm::ivec3(x, y, z);
                int blockID = m_world->GetBlock(checkPos.x, checkPos.y, checkPos.z);

                if (blockID == 47) { // Bookshelf block ID
                    count++;
                }
            }
        }
    }

    return std::min(count, m_config.maxBookshelves);
}

void EnchantmentTable::ScanForBookshelves() {
    m_state.bookshelfCount = CountBookshelves();
}

float EnchantmentTable::GenerateEnchantmentSeed() const {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0.0f, 1000.0f);
    return dis(gen);
}

int EnchantmentTable::CalculateBaseEnchantmentLevel(int playerXP) const {
    // Minecraft's enchantment level calculation
    int level = playerXP;

    // Add bonus from bookshelves
    level += m_state.bookshelfCount / 2;

    // Add some randomness
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(-level / 4, level / 4);
    level += dis(gen);

    return std::max(1, std::min(level, m_config.maxXPLevel));
}

EnchantmentOption EnchantmentTable::CreateEnchantmentOption(int level, int itemID) {
    EnchantmentOption option;
    option.levelRequirement = level;
    option.xpCost = GetXPCost(option);
    option.lapisCost = GetRequiredLapis(itemID);
    option.enchantmentPower = CalculateEnchantmentPower();

    // Get compatible enchantments for this item
    auto& manager = EnchantmentManager::GetInstance();
    std::vector<EnchantmentInstance> existingEnchantments; // Empty for new items

    auto compatibleEnchantments = manager.GetCompatibleEnchantments(itemID, existingEnchantments);

    if (compatibleEnchantments.empty()) {
        return option;
    }

    // Select random enchantments based on level
    std::random_device rd;
    std::mt19937 gen(rd());

    int numEnchantments = std::min(3, static_cast<int>(compatibleEnchantments.size()));
    if (level < 10) numEnchantments = 1;
    else if (level < 20) numEnchantments = std::max(1, numEnchantments / 2);

    std::vector<EnchantmentType> selectedEnchantments;

    for (int i = 0; i < numEnchantments; ++i) {
        std::uniform_int_distribution<size_t> dis(0, compatibleEnchantments.size() - 1);
        size_t index = dis(gen);
        EnchantmentType type = compatibleEnchantments[index];

        // Check if already selected
        if (std::find(selectedEnchantments.begin(), selectedEnchantments.end(), type) == selectedEnchantments.end()) {
            selectedEnchantments.push_back(type);

            // Create enchantment instance
            auto enchantment = manager.GetEnchantment(type);
            if (enchantment) {
                int maxLevel = enchantment->GetMaxLevel();
                int enchantmentLevel = std::min(maxLevel, level / 5 + 1);

                EnchantmentInstance instance;
                instance.type = type;
                instance.level = enchantmentLevel;

                option.enchantments.push_back(instance);
            }
        }
    }

    return option;
}

EnchantedItem EnchantmentTable::ApplyEnchantments(const EnchantmentOption& option) {
    EnchantedItem item;
    item.itemID = m_state.slots[static_cast<int>(EnchantmentSlot::INPUT)];
    item.itemData = 0; // Could store additional data
    item.enchantments = option.enchantments;
    item.repairCost = option.xpCost;
    item.customName = "";

    return item;
}

bool EnchantmentTable::ConsumeMaterials(const EnchantmentOption& option) {
    // Consume lapis lazuli
    auto [lapisID, lapisCount] = GetItem(EnchantmentSlot::LAPIS_LAZULI);
    if (lapisCount < option.lapisCost) return false;

    m_state.slotCounts[static_cast<int>(EnchantmentSlot::LAPIS_LAZULI)] -= option.lapisCost;
    if (m_state.slotCounts[static_cast<int>(EnchantmentSlot::LAPIS_LAZULI)] <= 0) {
        m_state.slots[static_cast<int>(EnchantmentSlot::LAPIS_LAZULI)] = 0;
    }

    return true;
}

void EnchantmentTable::PlayEnchantmentEffects() {
    // This would play particle effects, sounds, etc.
    // For now, just update the glow effect
    m_glowIntensity = 1.0f;
}

void EnchantmentTable::UpdateVisualEffects(float deltaTime) {
    // Update glow intensity
    if (m_glowIntensity > 0.0f) {
        m_glowIntensity -= deltaTime * 0.5f;
        if (m_glowIntensity < 0.0f) {
            m_glowIntensity = 0.0f;
        }
    }
}

bool EnchantmentTable::ValidateSetup() const {
    // Check if table is properly placed
    if (!m_world) return false;

    int blockBelow = m_world->GetBlock(m_position.x, m_position.y - 1, m_position.z);
    return blockBelow != 0; // Must have solid block below
}

// EnchantmentTableManager implementation
EnchantmentTableManager& EnchantmentTableManager::GetInstance() {
    static EnchantmentTableManager instance;
    return instance;
}

bool EnchantmentTableManager::Initialize(World* world) {
    if (!world) return false;

    m_world = world;
    m_initialized = true;

    return true;
}

void EnchantmentTableManager::Shutdown() {
    m_tables.clear();
    m_world = nullptr;
    m_initialized = false;
}

void EnchantmentTableManager::Update(float deltaTime) {
    for (auto& pair : m_tables) {
        if (pair.second) {
            pair.second->Update(deltaTime);
        }
    }
}

bool EnchantmentTableManager::CreateEnchantmentTable(const glm::ivec3& position) {
    if (m_tables.find(position) != m_tables.end()) {
        return false;
    }

    auto table = std::make_shared<EnchantmentTable>(position, m_world);
    m_tables[position] = table;
    m_stats.totalTables++;

    return true;
}

bool EnchantmentTableManager::RemoveEnchantmentTable(const glm::ivec3& position) {
    auto it = m_tables.find(position);
    if (it == m_tables.end()) {
        return false;
    }

    m_tables.erase(it);
    m_stats.totalTables--;

    return true;
}

std::shared_ptr<EnchantmentTable> EnchantmentTableManager::GetEnchantmentTable(const glm::ivec3& position) const {
    auto it = m_tables.find(position);
    return (it != m_tables.end()) ? it->second : nullptr;
}

std::vector<std::shared_ptr<EnchantmentTable>> EnchantmentTableManager::GetAllEnchantmentTables() const {
    std::vector<std::shared_ptr<EnchantmentTable>> tables;
    tables.reserve(m_tables.size());

    for (const auto& pair : m_tables) {
        tables.push_back(pair.second);
    }

    return tables;
}

std::shared_ptr<EnchantmentTable> EnchantmentTableManager::FindNearestEnchantmentTable(const glm::vec3& position, float maxDistance) {
    std::shared_ptr<EnchantmentTable> nearest = nullptr;
    float nearestDistance = maxDistance * maxDistance;

    for (const auto& pair : m_tables) {
        if (pair.second) {
            glm::vec3 tablePos = glm::vec3(pair.second->GetPosition());
            float distance = glm::distance(position, tablePos);
            if (distance < nearestDistance) {
                nearestDistance = distance;
                nearest = pair.second;
            }
        }
    }

    return nearest;
}

} // namespace VoxelCraft
