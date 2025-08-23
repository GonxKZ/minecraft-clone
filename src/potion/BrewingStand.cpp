/**
 * @file BrewingStand.cpp
 * @brief VoxelCraft Brewing Stand System Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#include "BrewingStand.hpp"
#include "../world/World.hpp"
#include "../player/Player.hpp"
#include "../inventory/Inventory.hpp"
#include <algorithm>
#include <random>
#include <cmath>

namespace VoxelCraft {

// BrewingStand implementation
BrewingStand::BrewingStand(const glm::ivec3& position, World* world)
    : m_position(position)
    , m_world(world)
    , m_currentPlayer(nullptr)
    , m_brewingTimer(0.0f)
    , m_currentRecipeIndex(-1)
    , m_bubbleTimer(0.0f)
    , m_temperatureChangeTimer(0.0f)
{
    m_config.enableBrewingStand = true;
    m_config.maxFuelLevel = 20;
    m_config.baseBrewingTime = 20.0f;
    m_config.fuelEfficiency = 1.0f;
    m_config.maxSlots = 6;
    m_config.requireFuel = true;
    m_config.enableOverBrewing = false;
    m_config.temperatureEffect = 0.1f;

    LoadDefaultRecipes();
}

BrewingStand::~BrewingStand() {
    if (m_currentPlayer) {
        CloseInterface();
    }
}

void BrewingStand::Update(float deltaTime) {
    auto now = std::chrono::steady_clock::now();
    auto timeSinceLastUpdate = std::chrono::duration<float>(now - m_state.startTime).count();
    m_state.startTime = now;

    // Update temperature
    UpdateTemperature(deltaTime);

    // Update visual effects
    UpdateVisualEffects(deltaTime);

    // Update brewing process
    if (m_state.state == BrewingState::BREWING) {
        UpdateBrewing(deltaTime);
    } else if (m_state.state == BrewingState::COMPLETED || m_state.state == BrewingState::FAILED) {
        // Reset state after a delay
        static float resetTimer = 0.0f;
        resetTimer += deltaTime;
        if (resetTimer >= 2.0f) {
            resetTimer = 0.0f;
            if (m_state.state == BrewingState::COMPLETED) {
                m_state.state = BrewingState::IDLE;
                m_state.brewingProgress = 0.0f;
            } else {
                m_state.state = BrewingState::IDLE;
            }
        }
    }

    // Auto-start brewing if conditions are met
    if (m_state.state == BrewingState::IDLE && CanStartBrewing()) {
        StartBrewing();
    }
}

bool BrewingStand::OpenInterface(Player* player) {
    if (!player || m_currentPlayer) {
        return false;
    }

    m_currentPlayer = player;
    m_state.isActive = true;

    return true;
}

void BrewingStand::CloseInterface() {
    if (m_currentPlayer) {
        // Return any items in the brewing stand
        for (int i = 0; i < 6; ++i) {
            if (m_state.slots[i] != 0 && m_state.slotCounts[i] > 0) {
                // Return item to player inventory
                // This would typically interact with the player's inventory system
            }
        }

        m_currentPlayer = nullptr;
        m_state.isActive = false;
    }
}

bool BrewingStand::SetItem(BrewingSlot slot, int itemID, int count) {
    int slotIndex = static_cast<int>(slot);

    if (slotIndex < 0 || slotIndex >= 6) return false;

    // Validate item placement
    if (!CanUseItemInSlot(slot, itemID)) return false;

    m_state.slots[slotIndex] = itemID;
    m_state.slotCounts[slotIndex] = count;

    return true;
}

std::pair<int, int> BrewingStand::GetItem(BrewingSlot slot) const {
    int slotIndex = static_cast<int>(slot);

    if (slotIndex < 0 || slotIndex >= 6) {
        return {0, 0};
    }

    return {m_state.slots[slotIndex], m_state.slotCounts[slotIndex]};
}

bool BrewingStand::StartBrewing() {
    if (m_state.state != BrewingState::IDLE || !CanStartBrewing()) {
        return false;
    }

    m_currentRecipeIndex = FindMatchingRecipe();
    if (m_currentRecipeIndex < 0) {
        return false;
    }

    const auto& recipe = m_availableRecipes[m_currentRecipeIndex];

    // Consume ingredients
    if (!ConsumeIngredients(recipe)) {
        return false;
    }

    // Start brewing
    m_state.state = BrewingState::BREWING;
    m_state.brewingProgress = 0.0f;
    m_brewingTimer = 0.0f;

    PlayBrewingEffects();

    return true;
}

void BrewingStand::StopBrewing() {
    if (m_state.state == BrewingState::BREWING) {
        m_state.state = BrewingState::IDLE;
        m_state.brewingProgress = 0.0f;
        m_brewingTimer = 0.0f;
    }
}

bool BrewingStand::AddFuel(int amount) {
    if (m_state.fuelLevel >= m_config.maxFuelLevel) {
        return false;
    }

    m_state.fuelLevel = std::min(m_config.maxFuelLevel, m_state.fuelLevel + amount);
    return true;
}

bool BrewingStand::CanUseItemInSlot(BrewingSlot slot, int itemID) const {
    switch (slot) {
        case BrewingSlot::INPUT:
            // Input can be water bottles or potions
            return (itemID == 373 || itemID == 374 || itemID == 375 ||
                   (itemID >= 376 && itemID <= 378));
        case BrewingSlot::INGREDIENT:
            // Various brewing ingredients
            return (itemID >= 348 && itemID <= 350) || // Fish
                   (itemID >= 353 && itemID <= 355) || // Dyes
                   (itemID >= 370 && itemID <= 382) || // Brewing items
                   (itemID >= 375 && itemID <= 378) || // More brewing items
                   (itemID >= 396 && itemID <= 399) || // Golden items
                   (itemID >= 414 && itemID <= 415) || // Rabbit items
                   itemID == 470; // Phantom membrane
        case BrewingSlot::FUEL:
            // Only blaze powder
            return (itemID == 377);
        case BrewingSlot::OUTPUT1:
        case BrewingSlot::OUTPUT2:
        case BrewingSlot::OUTPUT3:
            return false; // Output slots are read-only
        default:
            return false;
    }
}

bool BrewingStand::CanPlayerUse(Player* player) const {
    if (!player) return false;

    // Check if player is in range
    float distance = glm::distance(glm::vec3(player->GetPosition()), glm::vec3(m_position));
    return distance <= 4.0f; // 4 block range
}

void BrewingStand::UpdateBrewing(float deltaTime) {
    if (m_currentRecipeIndex < 0) return;

    const auto& recipe = m_availableRecipes[m_currentRecipeIndex];
    float brewingTime = recipe.brewingTime / 20.0f; // Convert ticks to seconds

    // Apply temperature effect
    float temperatureMultiplier = 1.0f + (m_state.temperature - 20.0f) * m_config.temperatureEffect;
    brewingTime /= temperatureMultiplier;

    m_brewingTimer += deltaTime;
    m_state.brewingProgress = std::min(1.0f, m_brewingTimer / brewingTime);

    if (m_state.brewingProgress >= 1.0f) {
        // Brewing completed
        if (ProducePotions(recipe)) {
            m_state.state = BrewingState::COMPLETED;
        } else {
            m_state.state = BrewingState::FAILED;
        }
        m_brewingTimer = 0.0f;
    }
}

int BrewingStand::FindMatchingRecipe() const {
    int inputItemID = m_state.slots[static_cast<int>(BrewingSlot::INPUT)];
    int ingredientItemID = m_state.slots[static_cast<int>(BrewingSlot::INGREDIENT)];

    if (inputItemID == 0 || ingredientItemID == 0) return -1;

    for (size_t i = 0; i < m_availableRecipes.size(); ++i) {
        const auto& recipe = m_availableRecipes[i];
        if (recipe.inputItemID == inputItemID && recipe.ingredientItemID == ingredientItemID) {
            return static_cast<int>(i);
        }
    }

    return -1;
}

bool BrewingStand::CanStartBrewing() const {
    if (m_state.state != BrewingState::IDLE) return false;

    // Check fuel
    if (m_config.requireFuel && m_state.fuelLevel <= 0) return false;

    // Check if we have valid input and ingredient
    int inputItemID = m_state.slots[static_cast<int>(BrewingSlot::INPUT)];
    int ingredientItemID = m_state.slots[static_cast<int>(BrewingSlot::INGREDIENT)];

    if (inputItemID == 0 || ingredientItemID == 0) return false;

    // Check if output slots have space
    for (int i = 3; i < 6; ++i) {
        if (m_state.slots[i] == 0) return true;
    }

    return false;
}

bool BrewingStand::ConsumeIngredients(const BrewingRecipe& recipe) {
    // Consume input item
    int inputSlot = static_cast<int>(BrewingSlot::INPUT);
    if (m_state.slotCounts[inputSlot] <= 0) return false;
    m_state.slotCounts[inputSlot]--;

    if (m_state.slotCounts[inputSlot] <= 0) {
        m_state.slots[inputSlot] = 0;
    }

    // Consume ingredient
    int ingredientSlot = static_cast<int>(BrewingSlot::INGREDIENT);
    if (m_state.slotCounts[ingredientSlot] <= 0) return false;
    m_state.slotCounts[ingredientSlot]--;

    if (m_state.slotCounts[ingredientSlot] <= 0) {
        m_state.slots[ingredientSlot] = 0;
    }

    // Consume fuel
    if (m_config.requireFuel && recipe.fuelCost > 0) {
        if (m_state.fuelLevel < recipe.fuelCost) return false;
        m_state.fuelLevel -= recipe.fuelCost;
    }

    return true;
}

bool BrewingStand::ProducePotions(const BrewingRecipe& recipe) {
    // Find empty output slot
    for (int i = 3; i < 6; ++i) {
        if (m_state.slots[i] == 0) {
            m_state.slots[i] = recipe.resultItemID;
            m_state.slotCounts[i] = 1;
            return true;
        }
    }

    return false;
}

void BrewingStand::UpdateTemperature(float deltaTime) {
    m_temperatureChangeTimer += deltaTime;

    if (m_temperatureChangeTimer >= 5.0f) {
        m_temperatureChangeTimer = 0.0f;

        // Temperature changes based on activity
        if (m_state.state == BrewingState::BREWING) {
            m_state.temperature = std::min(100.0f, m_state.temperature + 5.0f);
        } else {
            m_state.temperature = std::max(20.0f, m_state.temperature - 2.0f);
        }
    }
}

void BrewingStand::UpdateVisualEffects(float deltaTime) {
    m_bubbleTimer += deltaTime;

    if (m_bubbleTimer >= 1.0f) {
        m_bubbleTimer = 0.0f;

        // Create bubble effects during brewing
        if (m_state.state == BrewingState::BREWING) {
            // This would create particle effects
        }
    }
}

void BrewingStand::PlayBrewingEffects() {
    // Play brewing sound effects
    // Create particle effects
    // This would typically play audio and visual effects
}

void BrewingStand::LoadDefaultRecipes() {
    m_availableRecipes.clear();

    // Water bottle + nether wart = awkward potion
    m_availableRecipes.push_back({
        373, // Water bottle
        372, // Nether wart
        373, // Awkward potion
        PotionType::AWKWARD,
        400, // 20 seconds
        1,   // 1 fuel
        false, // Doesn't require base potion
        "Nether Wart + Water Bottle = Awkward Potion"
    });

    // Awkward potion + golden carrot = night vision
    m_availableRecipes.push_back({
        373, // Awkward potion
        375, // Golden carrot
        373, // Night vision potion
        PotionType::NIGHT_VISION,
        400,
        1,
        true,
        "Golden Carrot + Awkward Potion = Night Vision"
    });

    // Awkward potion + fermented spider eye = invisibility
    m_availableRecipes.push_back({
        373, // Awkward potion
        376, // Fermented spider eye
        373, // Invisibility potion
        PotionType::INVISIBILITY,
        400,
        1,
        true,
        "Fermented Spider Eye + Awkward Potion = Invisibility"
    });

    // Awkward potion + rabbit foot = leaping
    m_availableRecipes.push_back({
        373, // Awkward potion
        414, // Rabbit foot
        373, // Leaping potion
        PotionType::LEAPING,
        400,
        1,
        true,
        "Rabbit Foot + Awkward Potion = Leaping"
    });

    // Awkward potion + magma cream = fire resistance
    m_availableRecipes.push_back({
        373, // Awkward potion
        378, // Magma cream
        373, // Fire resistance potion
        PotionType::FIRE_RESISTANCE,
        400,
        1,
        true,
        "Magma Cream + Awkward Potion = Fire Resistance"
    });

    // Awkward potion + sugar = swiftness
    m_availableRecipes.push_back({
        373, // Awkward potion
        353, // Sugar
        373, // Swiftness potion
        PotionType::SWIFTNESS,
        400,
        1,
        true,
        "Sugar + Awkward Potion = Swiftness"
    });

    // Awkward potion + glistering melon = healing
    m_availableRecipes.push_back({
        373, // Awkward potion
        382, // Glistering melon
        373, // Healing potion
        PotionType::HEALING,
        400,
        1,
        true,
        "Glistering Melon + Awkward Potion = Healing"
    });

    // Awkward potion + blaze powder = strength
    m_availableRecipes.push_back({
        373, // Awkward potion
        377, // Blaze powder
        373, // Strength potion
        PotionType::STRENGTH,
        400,
        1,
        true,
        "Blaze Powder + Awkward Potion = Strength"
    });

    // Awkward potion + spider eye = poison
    m_availableRecipes.push_back({
        373, // Awkward potion
        375, // Spider eye
        373, // Poison potion
        PotionType::POISON,
        400,
        1,
        true,
        "Spider Eye + Awkward Potion = Poison"
    });

    // Awkward potion + ghast tear = regeneration
    m_availableRecipes.push_back({
        373, // Awkward potion
        370, // Ghast tear
        373, // Regeneration potion
        PotionType::REGENERATION,
        400,
        1,
        true,
        "Ghast Tear + Awkward Potion = Regeneration"
    });

    // Awkward potion + pufferfish = water breathing
    m_availableRecipes.push_back({
        373, // Awkward potion
        349, // Pufferfish
        373, // Water breathing potion
        PotionType::WATER_BREATHING,
        400,
        1,
        true,
        "Pufferfish + Awkward Potion = Water Breathing"
    });

    // Water bottle + fermented spider eye = weakness
    m_availableRecipes.push_back({
        373, // Water bottle
        376, // Fermented spider eye
        373, // Weakness potion
        PotionType::WEAKNESS,
        400,
        1,
        false,
        "Fermented Spider Eye + Water Bottle = Weakness"
    });

    // Swiftness + fermented spider eye = slowness
    m_availableRecipes.push_back({
        373, // Swiftness potion
        376, // Fermented spider eye
        373, // Slowness potion
        PotionType::SLOWNESS,
        400,
        1,
        true,
        "Fermented Spider Eye + Swiftness = Slowness"
    });

    // Healing + fermented spider eye = harming
    m_availableRecipes.push_back({
        373, // Healing potion
        376, // Fermented spider eye
        373, // Harming potion
        PotionType::HARMING,
        400,
        1,
        true,
        "Fermented Spider Eye + Healing = Harming"
    });

    // Awkward potion + phantom membrane = slow falling
    m_availableRecipes.push_back({
        373, // Awkward potion
        470, // Phantom membrane
        373, // Slow falling potion
        PotionType::SLOW_FALLING,
        400,
        1,
        true,
        "Phantom Membrane + Awkward Potion = Slow Falling"
    });

    // Awkward potion + turtle shell = turtle master
    m_availableRecipes.push_back({
        373, // Awkward potion
        469, // Turtle shell
        373, // Turtle master potion
        PotionType::TURTLE_MASTER,
        400,
        1,
        true,
        "Turtle Shell + Awkward Potion = Turtle Master"
    });
}

bool BrewingStand::ValidateSetup() const {
    // Check if brewing stand is properly placed
    if (!m_world) return false;

    int blockBelow = m_world->GetBlock(m_position.x, m_position.y - 1, m_position.z);
    return blockBelow != 0; // Must have solid block below
}

// BrewingStandManager implementation
BrewingStandManager& BrewingStandManager::GetInstance() {
    static BrewingStandManager instance;
    return instance;
}

bool BrewingStandManager::Initialize(World* world) {
    if (!world) return false;

    m_world = world;
    m_initialized = true;

    LoadDefaultRecipes();

    return true;
}

void BrewingStandManager::Shutdown() {
    m_stands.clear();
    m_recipes.clear();
    m_world = nullptr;
    m_initialized = false;
}

void BrewingStandManager::Update(float deltaTime) {
    for (auto& pair : m_stands) {
        if (pair.second) {
            pair.second->Update(deltaTime);
        }
    }
}

bool BrewingStandManager::CreateBrewingStand(const glm::ivec3& position) {
    if (m_stands.find(position) != m_stands.end()) {
        return false;
    }

    auto stand = std::make_shared<BrewingStand>(position, m_world);
    m_stands[position] = stand;
    m_stats.totalStands++;

    return true;
}

bool BrewingStandManager::RemoveBrewingStand(const glm::ivec3& position) {
    auto it = m_stands.find(position);
    if (it == m_stands.end()) {
        return false;
    }

    m_stands.erase(it);
    m_stats.totalStands--;

    return true;
}

std::shared_ptr<BrewingStand> BrewingStandManager::GetBrewingStand(const glm::ivec3& position) const {
    auto it = m_stands.find(position);
    return (it != m_stands.end()) ? it->second : nullptr;
}

std::vector<std::shared_ptr<BrewingStand>> BrewingStandManager::GetAllBrewingStands() const {
    std::vector<std::shared_ptr<BrewingStand>> stands;
    stands.reserve(m_stands.size());

    for (const auto& pair : m_stands) {
        stands.push_back(pair.second);
    }

    return stands;
}

std::shared_ptr<BrewingStand> BrewingStandManager::FindNearestBrewingStand(const glm::vec3& position, float maxDistance) {
    std::shared_ptr<BrewingStand> nearest = nullptr;
    float nearestDistance = maxDistance * maxDistance;

    for (const auto& pair : m_stands) {
        if (pair.second) {
            glm::vec3 standPos = glm::vec3(pair.second->GetPosition());
            float distance = glm::distance(position, standPos);
            if (distance < nearestDistance) {
                nearestDistance = distance;
                nearest = pair.second;
            }
        }
    }

    return nearest;
}

const std::vector<BrewingRecipe>& BrewingStandManager::GetBrewingRecipes() const {
    return m_recipes;
}

bool BrewingStandManager::AddBrewingRecipe(const BrewingRecipe& recipe) {
    m_recipes.push_back(recipe);
    return true;
}

void BrewingStandManager::LoadDefaultRecipes() {
    // These recipes are already loaded in individual brewing stands
    // This could be used for global recipe management if needed
}

} // namespace VoxelCraft
