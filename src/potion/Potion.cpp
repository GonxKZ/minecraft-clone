/**
 * @file Potion.cpp
 * @brief VoxelCraft Potion System Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#include "Potion.hpp"
#include <algorithm>
#include <random>
#include <sstream>

namespace VoxelCraft {

// PotionEffectManager implementation
bool PotionEffectManager::ApplyEffect(Entity* entity, const PotionEffect& effect) {
    if (!entity) return false;

    // Apply effect based on type
    switch (effect.type) {
        case PotionEffectType::SPEED:
            // Increase movement speed
            break;
        case PotionEffectType::SLOWNESS:
            // Decrease movement speed
            break;
        case PotionEffectType::STRENGTH:
            // Increase attack damage
            break;
        case PotionEffectType::INSTANT_HEALTH:
            if (entity->GetHealth() < entity->GetMaxHealth()) {
                float healAmount = 2.0f * (effect.amplifier + 1);
                entity->SetHealth(entity->GetHealth() + healAmount);
            }
            break;
        case PotionEffectType::INSTANT_DAMAGE:
            {
                float damageAmount = 3.0f * (effect.amplifier + 1);
                entity->TakeDamage(damageAmount);
            }
            break;
        case PotionEffectType::REGENERATION:
            // Start health regeneration
            break;
        case PotionEffectType::FIRE_RESISTANCE:
            // Grant fire immunity
            break;
        case PotionEffectType::WATER_BREATHING:
            // Allow underwater breathing
            break;
        case PotionEffectType::INVISIBILITY:
            // Make entity invisible
            break;
        case PotionEffectType::NIGHT_VISION:
            // Grant night vision
            break;
        case PotionEffectType::WEAKNESS:
            // Reduce attack damage
            break;
        case PotionEffectType::POISON:
            // Apply poison damage over time
            break;
        case PotionEffectType::JUMP_BOOST:
            // Increase jump height
            break;
        case PotionEffectType::SLOW_FALLING:
            // Reduce fall damage and speed
            break;
        default:
            break;
    }

    return true;
}

bool PotionEffectManager::RemoveEffect(Entity* entity, PotionEffectType effectType) {
    if (!entity) return false;

    // Remove effect from entity
    // This would typically remove the effect from the entity's active effects list
    return true;
}

bool PotionEffectManager::HasEffect(Entity* entity, PotionEffectType effectType) {
    if (!entity) return false;

    // Check if entity has the specified effect
    // This would typically check the entity's active effects list
    return false;
}

int PotionEffectManager::GetEffectAmplifier(Entity* entity, PotionEffectType effectType) {
    if (!entity) return 0;

    // Get effect amplifier
    // This would typically search the entity's active effects
    return 0;
}

int PotionEffectManager::GetEffectDuration(Entity* entity, PotionEffectType effectType) {
    if (!entity) return 0;

    // Get effect duration
    // This would typically search the entity's active effects
    return 0;
}

void PotionEffectManager::UpdateEntityEffects(Entity* entity, float deltaTime) {
    if (!entity) return;

    // Update all active effects on the entity
    // This would typically iterate through the entity's active effects and update them
}

void PotionEffectManager::ClearAllEffects(Entity* entity) {
    if (!entity) return;

    // Clear all effects from entity
    // This would typically clear the entity's active effects list
}

// Potion base implementation
Potion::Potion(const PotionDefinition& definition)
    : m_definition(definition) {
}

bool Potion::UsePotion(Entity* entity, PotionInstance& instance) {
    if (!entity || instance.IsEmpty()) return false;

    auto effects = GetPotionEffects(instance);
    ApplyPotionEffects(entity, effects);

    instance.usesRemaining--;
    return true;
}

bool Potion::ThrowSplashPotion(Entity* thrower, const glm::vec3& position,
                             const glm::vec3& direction, PotionInstance& instance) {
    if (!thrower || instance.IsEmpty()) return false;

    if (!CreateSplashPotionEntity(thrower, position, direction, instance)) {
        return false;
    }

    instance.usesRemaining--;
    return true;
}

bool Potion::CreateLingeringCloud(const glm::vec3& position, PotionInstance& instance) {
    if (instance.IsEmpty()) return false;

    // Create lingering area effect cloud
    // This would typically create a cloud entity at the specified position
    instance.usesRemaining--;
    return true;
}

std::vector<PotionEffect> Potion::GetPotionEffects(const PotionInstance& instance) const {
    std::vector<PotionEffect> effects = m_definition.baseEffects;

    // Apply instance modifiers
    for (auto& effect : effects) {
        auto it = instance.modifiers.find("duration_multiplier");
        if (it != instance.modifiers.end()) {
            effect.duration = static_cast<int>(effect.duration * it->second);
        }

        auto ampIt = instance.modifiers.find("amplifier_bonus");
        if (ampIt != instance.modifiers.end()) {
            effect.amplifier += static_cast<int>(ampIt->second);
        }
    }

    return effects;
}

bool Potion::CanBrew(const std::vector<int>& ingredients) const {
    auto required = GetBrewingIngredients();

    if (ingredients.size() != required.size()) return false;

    for (size_t i = 0; i < required.size(); ++i) {
        if (ingredients[i] != required[i]) return false;
    }

    return true;
}

std::vector<int> Potion::GetBrewingIngredients() const {
    // Default implementation - no brewing
    return {};
}

void Potion::ApplyPotionEffects(Entity* entity, const std::vector<PotionEffect>& effects) {
    for (const auto& effect : effects) {
        PotionEffectManager::ApplyEffect(entity, effect);
    }
}

bool Potion::CreateSplashPotionEntity(Entity* thrower, const glm::vec3& position,
                                    const glm::vec3& direction, PotionInstance& instance) {
    // Create splash potion projectile
    // This would typically create a projectile entity that applies effects on impact
    return true;
}

// HealingPotion implementation
HealingPotion::HealingPotion()
    : Potion(PotionDefinition{
        PotionType::HEALING,
        "healing_potion",
        "Healing Potion",
        "Restores health",
        PotionRarity::COMMON,
        373, // Potion item ID
        1,
        {
            PotionEffect{
                PotionEffectType::INSTANT_HEALTH,
                0, // amplifier
                1, // duration (instant)
                false, // ambient
                true, // show particles
                true, // show icon
                std::chrono::steady_clock::now(),
                nullptr, // onApply
                nullptr, // onTick
                nullptr  // onExpire
            }
        },
        false, // not splash
        false, // not lingering
        0xFF0000, // red color
        "healing_potion"
    }) {
}

bool HealingPotion::UsePotion(Entity* entity, PotionInstance& instance) {
    if (!entity || instance.IsEmpty()) return false;

    // Apply instant healing
    float healAmount = 4.0f; // 2 hearts
    if (entity->GetHealth() < entity->GetMaxHealth()) {
        entity->SetHealth(std::min(entity->GetMaxHealth(), entity->GetHealth() + healAmount));
    }

    instance.usesRemaining--;
    return true;
}

// SpeedPotion implementation
SpeedPotion::SpeedPotion()
    : Potion(PotionDefinition{
        PotionType::SWIFTNESS,
        "swiftness_potion",
        "Potion of Swiftness",
        "Increases movement speed",
        PotionRarity::COMMON,
        373, // Potion item ID
        1,
        {
            PotionEffect{
                PotionEffectType::SPEED,
                0, // amplifier
                1800, // 90 seconds
                false, // ambient
                true, // show particles
                true, // show icon
                std::chrono::steady_clock::now(),
                nullptr, // onApply
                nullptr, // onTick
                nullptr  // onExpire
            }
        },
        false, // not splash
        false, // not lingering
        0x7CAFC6, // light blue color
        "swiftness_potion"
    }) {
}

bool SpeedPotion::UsePotion(Entity* entity, PotionInstance& instance) {
    if (!entity || instance.IsEmpty()) return false;

    // Apply speed effect
    PotionEffect speedEffect;
    speedEffect.type = PotionEffectType::SPEED;
    speedEffect.amplifier = 0;
    speedEffect.duration = 1800; // 90 seconds
    speedEffect.isAmbient = false;
    speedEffect.showParticles = true;
    speedEffect.showIcon = true;

    PotionEffectManager::ApplyEffect(entity, speedEffect);

    instance.usesRemaining--;
    return true;
}

// StrengthPotion implementation
StrengthPotion::StrengthPotion()
    : Potion(PotionDefinition{
        PotionType::STRENGTH,
        "strength_potion",
        "Potion of Strength",
        "Increases attack damage",
        PotionRarity::COMMON,
        373, // Potion item ID
        1,
        {
            PotionEffect{
                PotionEffectType::STRENGTH,
                0, // amplifier
                1800, // 90 seconds
                false, // ambient
                true, // show particles
                true, // show icon
                std::chrono::steady_clock::now(),
                nullptr, // onApply
                nullptr, // onTick
                nullptr  // onExpire
            }
        },
        false, // not splash
        false, // not lingering
        0x932423, // red-brown color
        "strength_potion"
    }) {
}

bool StrengthPotion::UsePotion(Entity* entity, PotionInstance& instance) {
    if (!entity || instance.IsEmpty()) return false;

    // Apply strength effect
    PotionEffect strengthEffect;
    strengthEffect.type = PotionEffectType::STRENGTH;
    strengthEffect.amplifier = 0;
    strengthEffect.duration = 1800; // 90 seconds
    strengthEffect.isAmbient = false;
    strengthEffect.showParticles = true;
    strengthEffect.showIcon = true;

    PotionEffectManager::ApplyEffect(entity, strengthEffect);

    instance.usesRemaining--;
    return true;
}

// PotionManager implementation
PotionManager& PotionManager::GetInstance() {
    static PotionManager instance;
    return instance;
}

bool PotionManager::Initialize() {
    if (m_initialized) return true;

    RegisterDefaultPotions();
    RegisterBrewingRecipes();
    m_initialized = true;

    return true;
}

void PotionManager::Shutdown() {
    m_potions.clear();
    m_brewingRecipes.clear();
    m_stats = PotionStats();
    m_initialized = false;
}

void PotionManager::RegisterPotion(std::shared_ptr<Potion> potion) {
    if (potion) {
        m_potions[potion->GetType()] = potion;
        m_stats.totalPotions++;
    }
}

std::shared_ptr<Potion> PotionManager::GetPotion(PotionType type) const {
    auto it = m_potions.find(type);
    return (it != m_potions.end()) ? it->second : nullptr;
}

std::vector<std::shared_ptr<Potion>> PotionManager::GetAllPotions() const {
    std::vector<std::shared_ptr<Potion>> potions;
    potions.reserve(m_potions.size());

    for (const auto& pair : m_potions) {
        potions.push_back(pair.second);
    }

    return potions;
}

PotionInstance PotionManager::CreatePotionInstance(PotionType type, bool isSplash, bool isLingering) {
    PotionInstance instance;
    instance.type = type;

    auto potion = GetPotion(type);
    if (potion) {
        instance.usesRemaining = potion->GetDefinition().maxUses;
    } else {
        instance.usesRemaining = 1;
    }

    // Apply splash/lingering modifiers
    if (isSplash) {
        instance.modifiers["splash"] = 1.0f;
    }
    if (isLingering) {
        instance.modifiers["lingering"] = 1.0f;
        instance.usesRemaining = 4; // Lingering potions have 4 uses
    }

    return instance;
}

PotionType PotionManager::BrewPotion(const std::vector<int>& ingredients) {
    // Create recipe key
    std::stringstream ss;
    for (size_t i = 0; i < ingredients.size(); ++i) {
        if (i > 0) ss << ",";
        ss << ingredients[i];
    }
    std::string recipeKey = ss.str();

    // Find matching recipe
    auto it = m_brewingRecipes.find(recipeKey);
    if (it != m_brewingRecipes.end()) {
        m_stats.potionsBrewed++;
        return it->second;
    }

    return PotionType::WATER_BOTTLE; // Failed brewing
}

const std::unordered_map<std::string, PotionType>& PotionManager::GetBrewingRecipes() const {
    return m_brewingRecipes;
}

void PotionManager::UpdatePotionEffects(float deltaTime) {
    // Update all active potion effects in the world
    // This would typically iterate through all entities with active effects
}

void PotionManager::RegisterDefaultPotions() {
    // Basic potions
    RegisterPotion(std::make_shared<Potion>(PotionDefinition{
        PotionType::WATER_BOTTLE,
        "water_bottle",
        "Water Bottle",
        "Bottle of water",
        PotionRarity::COMMON,
        373,
        1,
        {},
        false, false,
        0x385DC6, // blue color
        "water_bottle"
    }));

    RegisterPotion(std::make_shared<Potion>(PotionDefinition{
        PotionType::MUNDANE,
        "mundane_potion",
        "Mundane Potion",
        "Has no effects",
        PotionRarity::COMMON,
        373,
        1,
        {},
        false, false,
        0x8B5A2B, // brown color
        "mundane_potion"
    }));

    RegisterPotion(std::make_shared<Potion>(PotionDefinition{
        PotionType::THICK,
        "thick_potion",
        "Thick Potion",
        "Has no effects",
        PotionRarity::COMMON,
        373,
        1,
        {},
        false, false,
        0x654335, // dark brown color
        "thick_potion"
    }));

    RegisterPotion(std::make_shared<Potion>(PotionDefinition{
        PotionType::AWKWARD,
        "awkward_potion",
        "Awkward Potion",
        "Base for other potions",
        PotionRarity::COMMON,
        373,
        1,
        {},
        false, false,
        0x9B5B4A, // reddish brown color
        "awkward_potion"
    }));

    // Effect potions
    RegisterPotion(std::make_shared<Potion>(PotionDefinition{
        PotionType::NIGHT_VISION,
        "night_vision_potion",
        "Potion of Night Vision",
        "Grants night vision",
        PotionRarity::COMMON,
        373,
        1,
        {
            PotionEffect{
                PotionEffectType::NIGHT_VISION,
                0, 3600, false, true, true,
                std::chrono::steady_clock::now()
            }
        },
        false, false,
        0x1F1FA1, // dark blue color
        "night_vision_potion"
    }));

    RegisterPotion(std::make_shared<Potion>(PotionDefinition{
        PotionType::INVISIBILITY,
        "invisibility_potion",
        "Potion of Invisibility",
        "Grants invisibility",
        PotionRarity::COMMON,
        373,
        1,
        {
            PotionEffect{
                PotionEffectType::INVISIBILITY,
                0, 3600, false, true, true,
                std::chrono::steady_clock::now()
            }
        },
        false, false,
        0x7F8392, // gray color
        "invisibility_potion"
    }));

    RegisterPotion(std::make_shared<Potion>(PotionDefinition{
        PotionType::LEAPING,
        "leaping_potion",
        "Potion of Leaping",
        "Increases jump height",
        PotionRarity::COMMON,
        373,
        1,
        {
            PotionEffect{
                PotionEffectType::JUMP_BOOST,
                0, 1800, false, true, true,
                std::chrono::steady_clock::now()
            }
        },
        false, false,
        0x786297, // purple color
        "leaping_potion"
    }));

    RegisterPotion(std::make_shared<Potion>(PotionDefinition{
        PotionType::FIRE_RESISTANCE,
        "fire_resistance_potion",
        "Potion of Fire Resistance",
        "Grants fire immunity",
        PotionRarity::COMMON,
        373,
        1,
        {
            PotionEffect{
                PotionEffectType::FIRE_RESISTANCE,
                0, 1800, false, true, true,
                std::chrono::steady_clock::now()
            }
        },
        false, false,
        0xE49A3A, // orange color
        "fire_resistance_potion"
    }));

    RegisterPotion(std::make_shared<SpeedPotion>());
    RegisterPotion(std::make_shared<StrengthPotion>());
    RegisterPotion(std::make_shared<HealingPotion>());

    RegisterPotion(std::make_shared<Potion>(PotionDefinition{
        PotionType::HARMING,
        "harming_potion",
        "Potion of Harming",
        "Deals instant damage",
        PotionRarity::COMMON,
        373,
        1,
        {
            PotionEffect{
                PotionEffectType::INSTANT_DAMAGE,
                0, 1, false, true, true,
                std::chrono::steady_clock::now()
            }
        },
        false, false,
        0x430A09, // dark red color
        "harming_potion"
    }));

    RegisterPotion(std::make_shared<Potion>(PotionDefinition{
        PotionType::POISON,
        "poison_potion",
        "Potion of Poison",
        "Applies poison effect",
        PotionRarity::COMMON,
        373,
        1,
        {
            PotionEffect{
                PotionEffectType::POISON,
                0, 900, false, true, true,
                std::chrono::steady_clock::now()
            }
        },
        false, false,
        0x4E9331, // green color
        "poison_potion"
    }));

    RegisterPotion(std::make_shared<Potion>(PotionDefinition{
        PotionType::REGENERATION,
        "regeneration_potion",
        "Potion of Regeneration",
        "Regenerates health over time",
        PotionRarity::COMMON,
        373,
        1,
        {
            PotionEffect{
                PotionEffectType::REGENERATION,
                0, 900, false, true, true,
                std::chrono::steady_clock::now()
            }
        },
        false, false,
        0xCD5CAB, // pink color
        "regeneration_potion"
    }));

    RegisterPotion(std::make_shared<Potion>(PotionDefinition{
        PotionType::WATER_BREATHING,
        "water_breathing_potion",
        "Potion of Water Breathing",
        "Allows underwater breathing",
        PotionRarity::COMMON,
        373,
        1,
        {
            PotionEffect{
                PotionEffectType::WATER_BREATHING,
                0, 1800, false, true, true,
                std::chrono::steady_clock::now()
            }
        },
        false, false,
        0x2E5299, // blue color
        "water_breathing_potion"
    }));

    RegisterPotion(std::make_shared<Potion>(PotionDefinition{
        PotionType::WEAKNESS,
        "weakness_potion",
        "Potion of Weakness",
        "Reduces attack damage",
        PotionRarity::COMMON,
        373,
        1,
        {
            PotionEffect{
                PotionEffectType::WEAKNESS,
                0, 1800, false, true, true,
                std::chrono::steady_clock::now()
            }
        },
        false, false,
        0x484D48, // dark gray color
        "weakness_potion"
    }));

    RegisterPotion(std::make_shared<Potion>(PotionDefinition{
        PotionType::SLOWNESS,
        "slowness_potion",
        "Potion of Slowness",
        "Reduces movement speed",
        PotionRarity::COMMON,
        373,
        1,
        {
            PotionEffect{
                PotionEffectType::SLOWNESS,
                0, 1800, false, true, true,
                std::chrono::steady_clock::now()
            }
        },
        false, false,
        0x5A6C81, // gray-blue color
        "slowness_potion"
    }));

    RegisterPotion(std::make_shared<Potion>(PotionDefinition{
        PotionType::LUCK,
        "luck_potion",
        "Potion of Luck",
        "Increases luck",
        PotionRarity::RARE,
        373,
        1,
        {
            PotionEffect{
                PotionEffectType::LUCK,
                0, 3000, false, true, true,
                std::chrono::steady_clock::now()
            }
        },
        false, false,
        0x339900, // green color
        "luck_potion"
    }));

    RegisterPotion(std::make_shared<Potion>(PotionDefinition{
        PotionType::SLOW_FALLING,
        "slow_falling_potion",
        "Potion of Slow Falling",
        "Reduces fall damage",
        PotionRarity::RARE,
        373,
        1,
        {
            PotionEffect{
                PotionEffectType::SLOW_FALLING,
                0, 1800, false, true, true,
                std::chrono::steady_clock::now()
            }
        },
        false, false,
        0xF3FFAE, // light yellow color
        "slow_falling_potion"
    }));
}

void PotionManager::RegisterBrewingRecipes() {
    // Basic brewing recipes
    // Format: "base_potion,ingredient1,ingredient2" -> result_potion

    // Night vision: Golden carrot + Awkward potion
    m_brewingRecipes["373,375"] = PotionType::NIGHT_VISION;

    // Invisibility: Fermented spider eye + Night vision potion
    m_brewingRecipes["373,376"] = PotionType::INVISIBILITY;

    // Leaping: Rabbit foot + Awkward potion
    m_brewingRecipes["373,414"] = PotionType::LEAPING;

    // Fire resistance: Magma cream + Awkward potion
    m_brewingRecipes["373,378"] = PotionType::FIRE_RESISTANCE;

    // Swiftness: Sugar + Awkward potion
    m_brewingRecipes["373,353"] = PotionType::SWIFTNESS;

    // Healing: Glistering melon + Awkward potion
    m_brewingRecipes["373,382"] = PotionType::HEALING;

    // Strength: Blaze powder + Awkward potion
    m_brewingRecipes["373,377"] = PotionType::STRENGTH;

    // Poison: Spider eye + Awkward potion
    m_brewingRecipes["373,375"] = PotionType::POISON;

    // Regeneration: Ghast tear + Awkward potion
    m_brewingRecipes["373,370"] = PotionType::REGENERATION;

    // Water breathing: Pufferfish + Awkward potion
    m_brewingRecipes["373,349"] = PotionType::WATER_BREATHING;

    // Weakness: Fermented spider eye + Water bottle
    m_brewingRecipes["373,376"] = PotionType::WEAKNESS;

    // Slowness: Fermented spider eye + Swiftness potion
    m_brewingRecipes["373,376"] = PotionType::SLOWNESS;

    // Harming: Fermented spider eye + Healing potion
    m_brewingRecipes["373,376"] = PotionType::HARMING;

    // Luck: Rabbit foot + Awkward potion (alternative)
    m_brewingRecipes["373,414"] = PotionType::LUCK;

    // Slow falling: Phantom membrane + Awkward potion
    m_brewingRecipes["373,470"] = PotionType::SLOW_FALLING;
}

} // namespace VoxelCraft
