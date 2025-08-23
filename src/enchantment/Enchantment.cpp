/**
 * @file Enchantment.cpp
 * @brief VoxelCraft Enchantment System Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#include "Enchantment.hpp"
#include <algorithm>
#include <cmath>
#include <random>

namespace VoxelCraft {

// Enchantment base implementation
Enchantment::Enchantment(const EnchantmentDefinition& definition)
    : m_definition(definition) {
}

bool Enchantment::CanApplyToItem(int itemID, const std::vector<EnchantmentInstance>& existingEnchantments) const {
    // Check if item type matches enchantment target
    bool itemMatches = false;
    switch (m_definition.target) {
        case EnchantmentTarget::ALL:
            itemMatches = true;
            break;
        case EnchantmentTarget::WEAPON:
            itemMatches = (itemID >= 267 && itemID <= 279) || (itemID >= 283 && itemID <= 286);
            break;
        case EnchantmentTarget::ARMOR:
            itemMatches = (itemID >= 298 && itemID <= 317);
            break;
        case EnchantmentTarget::TOOL:
            itemMatches = (itemID >= 256 && itemID <= 294);
            break;
        case EnchantmentTarget::BOW:
            itemMatches = (itemID == 261);
            break;
        case EnchantmentTarget::CROSSBOW:
            itemMatches = (itemID >= 445 && itemID <= 447);
            break;
        case EnchantmentTarget::TRIDENT:
            itemMatches = (itemID == 455);
            break;
        case EnchantmentTarget::FISHING_ROD:
            itemMatches = (itemID == 346);
            break;
        default:
            itemMatches = true;
            break;
    }

    if (!itemMatches) return false;

    // Check for incompatible enchantments
    for (const auto& existing : existingEnchantments) {
        if (std::find(m_definition.incompatibleEnchantments.begin(),
                     m_definition.incompatibleEnchantments.end(),
                     existing.type) != m_definition.incompatibleEnchantments.end()) {
            return false;
        }
    }

    return true;
}

// DamageEnchantment implementation
DamageEnchantment::DamageEnchantment(const EnchantmentDefinition& definition)
    : Enchantment(definition), m_damageMultiplier(0.5f + definition.type * 0.1f) {
}

void DamageEnchantment::ApplyEffect(Entity* target, Entity* source, int level) {
    if (!target || !source) return;

    float damage = m_damageMultiplier * level;
    target->TakeDamage(damage);
}

bool DamageEnchantment::CanApplyToItem(int itemID, const std::vector<EnchantmentInstance>& existingEnchantments) const {
    if (!Enchantment::CanApplyToItem(itemID, existingEnchantments)) return false;

    // Damage enchantments can only be applied to weapons
    return (itemID >= 267 && itemID <= 279) || (itemID >= 283 && itemID <= 286);
}

// ProtectionEnchantment implementation
ProtectionEnchantment::ProtectionEnchantment(const EnchantmentDefinition& definition)
    : Enchantment(definition), m_protectionMultiplier(0.04f) {
}

void ProtectionEnchantment::ApplyEffect(Entity* target, Entity* source, int level) {
    if (!target) return;

    // Reduce damage taken
    float protection = m_protectionMultiplier * level;
    // This would typically modify the damage calculation in the combat system
}

bool ProtectionEnchantment::CanApplyToItem(int itemID, const std::vector<EnchantmentInstance>& existingEnchantments) const {
    if (!Enchantment::CanApplyToItem(itemID, existingEnchantments)) return false;

    // Protection enchantments can only be applied to armor
    return (itemID >= 298 && itemID <= 317);
}

// EfficiencyEnchantment implementation
EfficiencyEnchantment::EfficiencyEnchantment(const EnchantmentDefinition& definition)
    : Enchantment(definition), m_speedMultiplier(0.3f) {
}

void EfficiencyEnchantment::ApplyEffect(Entity* target, Entity* source, int level) {
    if (!target) return;

    // Increase mining speed
    float speedBoost = m_speedMultiplier * level;
    // This would typically modify the tool's mining speed
}

bool EfficiencyEnchantment::CanApplyToItem(int itemID, const std::vector<EnchantmentInstance>& existingEnchantments) const {
    if (!Enchantment::CanApplyToItem(itemID, existingEnchantments)) return false;

    // Efficiency can only be applied to tools
    return (itemID >= 256 && itemID <= 294);
}

// EnchantmentManager implementation
EnchantmentManager& EnchantmentManager::GetInstance() {
    static EnchantmentManager instance;
    return instance;
}

bool EnchantmentManager::Initialize() {
    if (m_initialized) return true;

    RegisterDefaultEnchantments();
    m_initialized = true;

    return true;
}

void EnchantmentManager::Shutdown() {
    m_enchantments.clear();
    m_stats = EnchantmentStats();
    m_initialized = false;
}

void EnchantmentManager::RegisterEnchantment(std::shared_ptr<Enchantment> enchantment) {
    if (enchantment) {
        m_enchantments[enchantment->GetType()] = enchantment;
        m_stats.totalEnchantments++;
    }
}

std::shared_ptr<Enchantment> EnchantmentManager::GetEnchantment(EnchantmentType type) const {
    auto it = m_enchantments.find(type);
    return (it != m_enchantments.end()) ? it->second : nullptr;
}

std::vector<std::shared_ptr<Enchantment>> EnchantmentManager::GetAllEnchantments() const {
    std::vector<std::shared_ptr<Enchantment>> enchantments;
    enchantments.reserve(m_enchantments.size());

    for (const auto& pair : m_enchantments) {
        enchantments.push_back(pair.second);
    }

    return enchantments;
}

std::vector<EnchantmentType> EnchantmentManager::GetCompatibleEnchantments(
    int itemID, const std::vector<EnchantmentInstance>& existingEnchantments) const {

    std::vector<EnchantmentType> compatible;

    for (const auto& pair : m_enchantments) {
        auto enchantment = pair.second;
        if (enchantment && enchantment->CanApplyToItem(itemID, existingEnchantments)) {
            compatible.push_back(pair.first);
        }
    }

    return compatible;
}

int EnchantmentManager::CalculateEnchantmentCost(EnchantmentType enchantmentType, int level, int itemID) const {
    auto enchantment = GetEnchantment(enchantmentType);
    if (!enchantment) return 0;

    // Base cost calculation
    int baseCost = 1;
    float multiplier = 1.0f;

    switch (enchantment->GetRarity()) {
        case EnchantmentRarity::COMMON: baseCost = 1; break;
        case EnchantmentRarity::UNCOMMON: baseCost = 2; break;
        case EnchantmentRarity::RARE: baseCost = 4; break;
        case EnchantmentRarity::VERY_RARE: baseCost = 8; break;
        case EnchantmentRarity::TREASURE: baseCost = 16; break;
    }

    // Item material multiplier
    if (itemID >= 256 && itemID <= 279) { // Tools/weapons
        multiplier = 1.0f;
    } else if (itemID >= 298 && itemID <= 317) { // Armor
        multiplier = 1.2f;
    } else if (itemID == 261) { // Bow
        multiplier = 1.1f;
    }

    return static_cast<int>(baseCost * level * multiplier);
}

bool EnchantmentManager::AreEnchantmentsCompatible(EnchantmentType type1, EnchantmentType type2) const {
    auto enchantment1 = GetEnchantment(type1);
    auto enchantment2 = GetEnchantment(type2);

    if (!enchantment1 || !enchantment2) return true;

    const auto& incompatibles1 = enchantment1->GetDefinition().incompatibleEnchantments;
    const auto& incompatibles2 = enchantment2->GetDefinition().incompatibleEnchantments;

    return std::find(incompatibles1.begin(), incompatibles1.end(), type2) == incompatibles1.end() &&
           std::find(incompatibles2.begin(), incompatibles2.end(), type1) == incompatibles2.end();
}

std::string EnchantmentManager::GetEnchantmentColor(EnchantmentRarity rarity) {
    switch (rarity) {
        case EnchantmentRarity::COMMON: return "§7";      // Gray
        case EnchantmentRarity::UNCOMMON: return "§a";    // Green
        case EnchantmentRarity::RARE: return "§b";        // Aqua
        case EnchantmentRarity::VERY_RARE: return "§d";   // Magenta
        case EnchantmentRarity::TREASURE: return "§6";    // Gold
        default: return "§f";                            // White
    }
}

void EnchantmentManager::RegisterDefaultEnchantments() {
    // Weapon Enchantments
    RegisterEnchantment(std::make_shared<DamageEnchantment>(EnchantmentDefinition{
        EnchantmentType::SHARPNESS,
        "sharpness",
        "Sharpness",
        "Increases melee damage",
        EnchantmentRarity::COMMON,
        EnchantmentTarget::WEAPON,
        false, false, 5,
        {
            {1, 1, 11, 1.0f, 1.0f},
            {2, 12, 22, 2.0f, 2.0f},
            {3, 23, 33, 3.0f, 3.0f},
            {4, 34, 44, 4.0f, 4.0f},
            {5, 45, 55, 5.0f, 5.0f}
        },
        {EnchantmentType::SMITE, EnchantmentType::BANE_OF_ARTHROPODS}
    }));

    RegisterEnchantment(std::make_shared<DamageEnchantment>(EnchantmentDefinition{
        EnchantmentType::SMITE,
        "smite",
        "Smite",
        "Extra damage to undead mobs",
        EnchantmentRarity::UNCOMMON,
        EnchantmentTarget::WEAPON,
        false, false, 5,
        {
            {1, 5, 25, 2.0f, 2.0f},
            {2, 26, 41, 3.0f, 3.0f},
            {3, 42, 57, 4.0f, 4.0f},
            {4, 58, 73, 5.0f, 5.0f},
            {5, 74, 89, 6.0f, 6.0f}
        },
        {EnchantmentType::SHARPNESS, EnchantmentType::BANE_OF_ARTHROPODS}
    }));

    RegisterEnchantment(std::make_shared<DamageEnchantment>(EnchantmentDefinition{
        EnchantmentType::BANE_OF_ARTHROPODS,
        "bane_of_arthropods",
        "Bane of Arthropods",
        "Extra damage to arthropod mobs",
        EnchantmentRarity::UNCOMMON,
        EnchantmentTarget::WEAPON,
        false, false, 5,
        {
            {1, 5, 25, 2.0f, 2.0f},
            {2, 26, 41, 3.0f, 3.0f},
            {3, 42, 57, 4.0f, 4.0f},
            {4, 58, 73, 5.0f, 5.0f},
            {5, 74, 89, 6.0f, 6.0f}
        },
        {EnchantmentType::SHARPNESS, EnchantmentType::SMITE}
    }));

    // Armor Enchantments
    RegisterEnchantment(std::make_shared<ProtectionEnchantment>(EnchantmentDefinition{
        EnchantmentType::PROTECTION,
        "protection",
        "Protection",
        "Reduces most types of damage",
        EnchantmentRarity::COMMON,
        EnchantmentTarget::ARMOR,
        false, false, 4,
        {
            {1, 1, 12, 1.0f, 1.0f},
            {2, 13, 23, 2.0f, 2.0f},
            {3, 24, 34, 3.0f, 3.0f},
            {4, 35, 45, 4.0f, 4.0f}
        },
        {EnchantmentType::FIRE_PROTECTION, EnchantmentType::BLAST_PROTECTION, EnchantmentType::PROJECTILE_PROTECTION}
    }));

    RegisterEnchantment(std::make_shared<ProtectionEnchantment>(EnchantmentDefinition{
        EnchantmentType::FIRE_PROTECTION,
        "fire_protection",
        "Fire Protection",
        "Reduces fire damage",
        EnchantmentRarity::UNCOMMON,
        EnchantmentTarget::ARMOR,
        false, false, 4,
        {
            {1, 10, 22, 2.0f, 2.0f},
            {2, 23, 34, 3.0f, 3.0f},
            {3, 35, 46, 4.0f, 4.0f},
            {4, 47, 58, 5.0f, 5.0f}
        },
        {EnchantmentType::PROTECTION, EnchantmentType::BLAST_PROTECTION, EnchantmentType::PROJECTILE_PROTECTION}
    }));

    // Tool Enchantments
    RegisterEnchantment(std::make_shared<EfficiencyEnchantment>(EnchantmentDefinition{
        EnchantmentType::EFFICIENCY,
        "efficiency",
        "Efficiency",
        "Increases mining speed",
        EnchantmentRarity::COMMON,
        EnchantmentTarget::TOOL,
        false, false, 5,
        {
            {1, 1, 51, 1.0f, 1.0f},
            {2, 1, 51, 2.0f, 2.0f},
            {3, 1, 51, 3.0f, 3.0f},
            {4, 1, 51, 4.0f, 4.0f},
            {5, 1, 51, 5.0f, 5.0f}
        },
        {}
    }));

    RegisterEnchantment(std::make_shared<Enchantment>(EnchantmentDefinition{
        EnchantmentType::UNBREAKING,
        "unbreaking",
        "Unbreaking",
        "Increases item durability",
        EnchantmentRarity::UNCOMMON,
        EnchantmentTarget::BREAKABLE,
        false, false, 3,
        {
            {1, 5, 55, 2.0f, 2.0f},
            {2, 13, 71, 3.0f, 3.0f},
            {3, 21, 81, 4.0f, 4.0f}
        },
        {}
    }));

    // Bow Enchantments
    RegisterEnchantment(std::make_shared<Enchantment>(EnchantmentDefinition{
        EnchantmentType::POWER,
        "power",
        "Power",
        "Increases arrow damage",
        EnchantmentRarity::COMMON,
        EnchantmentTarget::BOW,
        false, false, 5,
        {
            {1, 1, 16, 1.0f, 1.0f},
            {2, 17, 26, 2.0f, 2.0f},
            {3, 27, 36, 3.0f, 3.0f},
            {4, 37, 46, 4.0f, 4.0f},
            {5, 47, 56, 5.0f, 5.0f}
        },
        {EnchantmentType::PUNCH}
    }));

    RegisterEnchantment(std::make_shared<Enchantment>(EnchantmentDefinition{
        EnchantmentType::INFINITY,
        "infinity",
        "Infinity",
        "Infinite arrows",
        EnchantmentRarity::UNCOMMON,
        EnchantmentTarget::BOW,
        false, false, 1,
        {
            {1, 20, 50, 6.0f, 6.0f}
        },
        {}
    }));

    // Curses
    RegisterEnchantment(std::make_shared<Enchantment>(EnchantmentDefinition{
        EnchantmentType::CURSE_OF_BINDING,
        "curse_of_binding",
        "Curse of Binding",
        "Prevents item removal",
        EnchantmentRarity::TREASURE,
        EnchantmentTarget::WEARABLE,
        true, true, 1,
        {
            {1, 25, 50, 8.0f, 8.0f}
        },
        {}
    }));

    RegisterEnchantment(std::make_shared<Enchantment>(EnchantmentDefinition{
        EnchantmentType::CURSE_OF_VANISHING,
        "curse_of_vanishing",
        "Curse of Vanishing",
        "Destroys item on death",
        EnchantmentRarity::TREASURE,
        EnchantmentTarget::ALL,
        true, true, 1,
        {
            {1, 25, 50, 8.0f, 8.0f}
        },
        {}
    }));
}

} // namespace VoxelCraft
