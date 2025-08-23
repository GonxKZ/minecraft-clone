/**
 * @file Crop.cpp
 * @brief VoxelCraft Farming System - Crop Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#include "Crop.hpp"
#include "../world/World.hpp"
#include <algorithm>
#include <cmath>
#include <random>

namespace VoxelCraft {

// Crop base implementation
Crop::Crop(const CropProperties& properties)
    : m_properties(properties) {
}

void Crop::UpdateGrowth(CropInstance& instance, float deltaTime, World* world) {
    if (instance.stage == GrowthStage::WITHERED || instance.health <= 0.0f) {
        return;
    }

    // Handle environmental effects
    HandleEnvironmentalEffects(instance, world);

    // Check for diseases
    CheckForDiseases(instance, world);

    // Calculate growth rate
    float growthRate = CalculateGrowthRate(instance, world);
    float growthProgress = (deltaTime / 60.0f) * growthRate; // Convert to minutes

    // Apply growth
    instance.growthProgress += growthProgress;

    // Check if ready to advance stage
    while (instance.growthProgress >= 1.0f && instance.stage != GrowthStage::MATURE) {
        instance.growthProgress -= 1.0f;
        AdvanceGrowthStage(instance);
    }

    // Clamp growth progress
    instance.growthProgress = std::min(instance.growthProgress, 1.0f);

    instance.lastGrowthUpdate = std::chrono::steady_clock::now();
}

std::vector<std::pair<int, int>> Crop::Harvest(CropInstance& instance) {
    std::vector<std::pair<int, int>> items;

    if (!instance.IsReadyForHarvest()) {
        return items;
    }

    // Calculate yield based on health and conditions
    int yield = m_properties.minYield;
    if (instance.health > 0.8f && instance.fertilizerLevel > 0.5f) {
        yield = std::min(yield + 1, m_properties.maxYield);
    }

    // Add main harvest item
    items.push_back({m_properties.harvestItemID, yield});

    // Chance for bonus items (seeds)
    if (instance.health > 0.7f) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dis(0.0f, 1.0f);

        if (dis(gen) < 0.3f) { // 30% chance for seeds
            items.push_back({m_properties.seedItemID, 1});
        }
    }

    // Reset or remove crop
    if (m_properties.regrowsAfterHarvest) {
        instance.stage = GrowthStage::SEEDLING;
        instance.growthProgress = 0.0f;
        instance.lastGrowthUpdate = std::chrono::steady_clock::now();
    } else {
        instance.stage = GrowthStage::HARVESTED;
    }

    return items;
}

bool Crop::CanPlantAt(const glm::ivec3& position, World* world) const {
    if (!world) return false;

    // Check if position has valid soil
    int soilBlockID = world->GetBlock(position.x, position.y - 1, position.z);
    bool hasValidSoil = std::find(m_properties.compatibleSoils.begin(),
                                 m_properties.compatibleSoils.end(),
                                 soilBlockID) != m_properties.compatibleSoils.end();

    if (!hasValidSoil) return false;

    // Check if position is air
    int currentBlockID = world->GetBlock(position.x, position.y, position.z);
    if (currentBlockID != 0) return false; // Must be air

    // Check light level
    // This would need to be implemented based on the lighting system
    int lightLevel = 15; // Placeholder
    if (lightLevel < m_properties.requirements.minLightLevel) return false;

    return true;
}

int Crop::GetVisualGrowthStage(const CropInstance& instance) const {
    return static_cast<int>(instance.stage);
}

void Crop::ApplyFertilizer(CropInstance& instance, int fertilizerType, float amount) {
    instance.fertilizerLevel = std::min(instance.fertilizerLevel + amount, 1.0f);

    // Different fertilizer types have different effects
    switch (fertilizerType) {
        case 0: // Basic fertilizer - growth boost
            instance.growthModifiers["growth_rate"] = instance.growthModifiers.get("growth_rate", 1.0f) + 0.2f;
            break;
        case 1: // Premium fertilizer - growth + health boost
            instance.growthModifiers["growth_rate"] = instance.growthModifiers.get("growth_rate", 1.0f) + 0.4f;
            instance.health = std::min(instance.health + 0.2f, 1.0f);
            break;
        case 2: // Disease resistant fertilizer
            instance.growthModifiers["disease_resistance"] = instance.growthModifiers.get("disease_resistance", 1.0f) + 0.5f;
            break;
    }
}

void Crop::WaterCrop(CropInstance& instance, float amount) {
    instance.waterLevel = std::min(instance.waterLevel + static_cast<int>(amount * 3), 3);
    instance.lastWateredTime = std::chrono::steady_clock::now();

    // Water provides growth boost
    instance.growthModifiers["growth_rate"] = instance.growthModifiers.get("growth_rate", 1.0f) + 0.3f;
}

void Crop::CheckForDiseases(CropInstance& instance, World* world) {
    if (instance.isDiseased) return;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0.0f, 1.0f);

    float diseaseChance = 0.001f; // Base 0.1% chance per update

    // Environmental factors increase disease chance
    if (instance.waterLevel < 1) diseaseChance += 0.002f;
    if (instance.soilQuality == SoilQuality::POOR) diseaseChance += 0.001f;

    // Disease resistance reduces chance
    float resistance = m_properties.diseaseResistance *
                      instance.growthModifiers.get("disease_resistance", 1.0f);
    diseaseChance /= resistance;

    if (dis(gen) < diseaseChance) {
        instance.isDiseased = true;
        instance.health -= 0.3f;
    }
}

void Crop::TreatDisease(CropInstance& instance, int treatmentType) {
    if (!instance.isDiseased) return;

    switch (treatmentType) {
        case 0: // Basic treatment
            if (instance.health > 0.2f) {
                instance.isDiseased = false;
                instance.health = std::min(instance.health + 0.1f, 1.0f);
            }
            break;
        case 1: // Advanced treatment
            instance.isDiseased = false;
            instance.health = std::min(instance.health + 0.3f, 1.0f);
            break;
    }
}

float Crop::CalculateGrowthRate(const CropInstance& instance, World* world) const {
    float baseRate = 1.0f / m_properties.baseGrowthTime; // Growth per minute

    // Apply modifiers
    float modifiedRate = baseRate * m_properties.requirements.growthRateMultiplier;

    // Environmental factors
    if (instance.waterLevel > 0) {
        modifiedRate *= 1.2f + (instance.waterLevel * 0.1f);
    }

    if (instance.fertilizerLevel > 0) {
        modifiedRate *= 1.0f + (instance.fertilizerLevel * 0.5f);
    }

    // Health affects growth
    modifiedRate *= instance.health;

    // Soil quality
    switch (instance.soilQuality) {
        case SoilQuality::POOR: modifiedRate *= 0.7f; break;
        case SoilQuality::GOOD: modifiedRate *= 1.3f; break;
        case SoilQuality::EXCELLENT: modifiedRate *= 1.6f; break;
        case SoilQuality::PERFECT: modifiedRate *= 2.0f; break;
        default: break;
    }

    // Disease reduces growth
    if (instance.isDiseased) {
        modifiedRate *= 0.5f;
    }

    return modifiedRate;
}

void Crop::AdvanceGrowthStage(CropInstance& instance) {
    if (instance.stage == GrowthStage::MATURE) return;

    instance.stage = static_cast<GrowthStage>(static_cast<int>(instance.stage) + 1);

    // Special handling for certain stages
    if (instance.stage == GrowthStage::MATURE) {
        // Mature crop is more susceptible to environmental factors
        instance.growthModifiers["environmental_sensitivity"] = 1.2f;
    }
}

void Crop::HandleEnvironmentalEffects(CropInstance& instance, World* world) {
    if (!world) return;

    // Get environmental conditions
    float temperature = 20.0f; // Placeholder - would get from weather system
    float humidity = 0.5f;     // Placeholder
    int lightLevel = 15;       // Placeholder

    auto& req = m_properties.requirements;

    // Temperature effects
    if (temperature < req.minTemperature || temperature > req.maxTemperature) {
        instance.health -= 0.01f;
    } else if (std::abs(temperature - req.optimalTemperature) < 5.0f) {
        // Optimal temperature gives slight bonus
        instance.growthModifiers["growth_rate"] = instance.growthModifiers.get("growth_rate", 1.0f) + 0.1f;
    }

    // Light level effects
    if (req.needsSunlight) {
        if (lightLevel < req.minLightLevel) {
            instance.health -= 0.02f;
        } else if (lightLevel > req.maxLightLevel) {
            instance.health -= 0.01f;
        }
    }

    // Water level decreases over time
    if (instance.waterLevel > 0) {
        auto now = std::chrono::steady_clock::now();
        auto timeSinceWatered = std::chrono::duration_cast<std::chrono::minutes>(
            now - instance.lastWateredTime).count();

        if (timeSinceWatered > 10) { // Water lasts 10 minutes
            instance.waterLevel = std::max(instance.waterLevel - 1, 0);
        }
    }

    // Fertilizer level decreases over time
    if (instance.fertilizerLevel > 0) {
        instance.fertilizerLevel = std::max(instance.fertilizerLevel - 0.001f, 0.0f);
    }

    // Clamp health
    instance.health = std::max(0.0f, std::min(instance.health, 1.0f));
}

// WheatCrop implementation
WheatCrop::WheatCrop() : Crop({
    CropType::WHEAT,
    "wheat",
    "Wheat",
    7, 30.0f, 0.2f, 1, 3, 295, 296, false, 0, 1.0f, 1.0f, false,
    {2, 3, 60}, // Compatible soils: dirt, grass, farmland
    {
        10.0f, 30.0f, 20.0f,  // Temperature
        0.3f, 0.8f, 0.6f,     // Humidity
        8, 15, 12,            // Light
        SoilQuality::POOR, true, true, false, 1.0f
    }
}) {}

void WheatCrop::UpdateGrowth(CropInstance& instance, float deltaTime, World* world) {
    Crop::UpdateGrowth(instance, deltaTime, world);
}

std::vector<std::pair<int, int>> WheatCrop::Harvest(CropInstance& instance) {
    auto items = Crop::Harvest(instance);

    // Wheat specific: higher yield with good conditions
    if (instance.soilQuality >= SoilQuality::GOOD && instance.fertilizerLevel > 0.7f) {
        for (auto& item : items) {
            if (item.first == 296) { // Wheat item
                item.second += 1;
            }
        }
    }

    return items;
}

bool WheatCrop::CanPlantAt(const glm::ivec3& position, World* world) const {
    if (!Crop::CanPlantAt(position, world)) return false;

    // Wheat needs at least average soil quality
    int soilBlockID = world->GetBlock(position.x, position.y - 1, position.z);
    return soilBlockID == 60 || soilBlockID == 2 || soilBlockID == 3; // Farmland, dirt, grass
}

// CarrotCrop implementation
CarrotCrop::CarrotCrop() : Crop({
    CropType::CARROT,
    "carrot",
    "Carrot",
    7, 25.0f, 0.15f, 1, 4, 391, 391, false, 0, 1.0f, 1.0f, false,
    {2, 3, 60}, // Compatible soils
    {
        8.0f, 25.0f, 18.0f,   // Temperature
        0.2f, 0.7f, 0.5f,     // Humidity
        8, 15, 12,            // Light
        SoilQuality::AVERAGE, true, true, false, 1.0f
    }
}) {}

void CarrotCrop::UpdateGrowth(CropInstance& instance, float deltaTime, World* world) {
    Crop::UpdateGrowth(instance, deltaTime, world);
}

std::vector<std::pair<int, int>> CarrotCrop::Harvest(CropInstance& instance) {
    auto items = Crop::Harvest(instance);

    // Carrots can have multiple yields
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(1, 4);

    for (auto& item : items) {
        if (item.first == 391) { // Carrot item
            item.second = dis(gen);
            if (instance.fertilizerLevel > 0.8f) {
                item.second += 1;
            }
        }
    }

    return items;
}

bool CarrotCrop::CanPlantAt(const glm::ivec3& position, World* world) const {
    return Crop::CanPlantAt(position, world);
}

// TreeCrop implementation
TreeCrop::TreeCrop(const CropProperties& properties)
    : Crop(properties) {
}

void TreeCrop::UpdateGrowth(CropInstance& instance, float deltaTime, World* world) {
    Crop::UpdateGrowth(instance, deltaTime, world);

    // Trees need space to grow upward
    if (instance.stage >= GrowthStage::GROWING && world) {
        for (int y = 1; y <= m_maxTreeHeight; ++y) {
            glm::ivec3 checkPos = instance.position + glm::ivec3(0, y, 0);
            int blockID = world->GetBlock(checkPos.x, checkPos.y, checkPos.z);

            // If there's a solid block in the way, stop growth
            if (blockID != 0 && y < 3) {
                instance.health -= 0.05f;
                break;
            }
        }
    }
}

std::vector<std::pair<int, int>> TreeCrop::Harvest(CropInstance& instance) {
    auto items = Crop::Harvest(instance);

    // Trees drop wood when cut down
    if (instance.stage == GrowthStage::MATURE) {
        // This would drop logs based on tree type
        items.push_back({17, 4}); // Oak logs placeholder
    }

    return items;
}

bool TreeCrop::CanPlantAt(const glm::ivec3& position, World* world) const {
    if (!Crop::CanPlantAt(position, world)) return false;

    // Trees need space above
    for (int y = 1; y <= 3; ++y) {
        glm::ivec3 checkPos = position + glm::ivec3(0, y, 0);
        int blockID = world->GetBlock(checkPos.x, checkPos.y, checkPos.z);
        if (blockID != 0) return false;
    }

    return true;
}

int TreeCrop::GetVisualGrowthStage(const CropInstance& instance) const {
    // Trees have more complex visual stages
    float progress = instance.GetGrowthPercentage() / 100.0f;
    return static_cast<int>(progress * 5.0f); // 0-5 visual stages
}

// AppleTreeCrop implementation
AppleTreeCrop::AppleTreeCrop() : TreeCrop({
    CropType::APPLE,
    "apple_tree",
    "Apple Tree",
    5, 240.0f, 0.1f, 3, 8, 6, 260, true, 15, 1.0f, 1.0f, true,
    {2, 3}, // Compatible soils
    {
        15.0f, 25.0f, 20.0f,  // Temperature
        0.4f, 0.8f, 0.6f,     // Humidity
        10, 15, 13,           // Light
        SoilQuality::GOOD, false, true, false, 0.8f
    }
}) {
    m_maxTreeHeight = 8;
}

std::vector<std::pair<int, int>> AppleTreeCrop::Harvest(CropInstance& instance) {
    auto items = TreeCrop::Harvest(instance);

    // Apples drop from leaves
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(0, 5);

    int appleCount = dis(gen);
    if (instance.health > 0.8f) {
        appleCount += 2;
    }

    if (appleCount > 0) {
        items.push_back({260, appleCount}); // Apple
    }

    return items;
}

// CropManager implementation
CropManager& CropManager::GetInstance() {
    static CropManager instance;
    return instance;
}

bool CropManager::PlantCrop(CropType cropType, const glm::ivec3& position, World* world) {
    // Simple crop planting - always succeeds if block is air
    if (world && world->GetBlock(position.x, position.y, position.z) == 0) {
        CropInstance instance;
        instance.position = position;
        instance.type = cropType;
        instance.stage = GrowthStage::GROWING; // Start partially grown for gameplay
        instance.growthProgress = 0.5f;
        instance.health = 1.0f;
        instance.plantTime = std::chrono::steady_clock::now();
        instance.lastGrowthUpdate = std::chrono::steady_clock::now();

        m_cropInstances[position] = instance;
        return true;
    }
    return false;
}

std::vector<std::pair<int, int>> CropManager::HarvestCrop(const glm::ivec3& position, World* world) {
    auto it = m_cropInstances.find(position);
    if (it == m_cropInstances.end()) return {};

    // Simple harvest - give basic crop item
    std::vector<std::pair<int, int>> items;
    switch (it->second.type) {
        case CropType::WHEAT:
            items.push_back({296, 1}); // Wheat
            break;
        case CropType::CARROT:
            items.push_back({391, 1}); // Carrot
            break;
        case CropType::POTATO:
            items.push_back({392, 1}); // Potato
            break;
        default:
            items.push_back({296, 1}); // Default to wheat
            break;
    }

    // Remove crop after harvest
    m_cropInstances.erase(it);
    return items;
}

bool CropManager::HasCropAt(const glm::ivec3& position) const {
    return m_cropInstances.find(position) != m_cropInstances.end();
}

} // namespace VoxelCraft
