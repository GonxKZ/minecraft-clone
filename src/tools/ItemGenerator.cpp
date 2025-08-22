/**
 * @file ItemGenerator.cpp
 * @brief VoxelCraft Procedural Item and Tool Generator - Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#include "ItemGenerator.hpp"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <cmath>

namespace VoxelCraft {

    ItemGenerator::ItemGenerator() {
        m_seed = std::chrono::system_clock::now().time_since_epoch().count();
        m_randomEngine.seed(m_seed);
    }

    ItemGenerator::~ItemGenerator() {
        // Cleanup any generated data
    }

    void ItemGenerator::Initialize(uint64_t seed) {
        m_seed = seed;
        m_randomEngine.seed(seed);
        InitializeMaterialTemplates();
        InitializeToolTemplates();
        InitializeGenerators();
        InitializeModelGenerators();
        InitializeTextureGenerators();
        InitializeSoundGenerators();
    }

    void ItemGenerator::InitializeMaterialTemplates() {
        // Wood material
        ItemStats woodStats;
        woodStats.durability = 59.0f;
        woodStats.currentDurability = 59.0f;
        woodStats.attackDamage = 2.0f;
        woodStats.attackSpeed = 1.6f;
        woodStats.miningSpeed = 2.0f;
        woodStats.efficiency = 1.0f;
        woodStats.armorValue = 1.0f;
        woodStats.maxStackSize = 64;
        woodStats.isStackable = true;
        woodStats.isEnchantable = true;
        woodStats.isRepairable = true;
        woodStats.isConsumable = false;
        m_materialTemplates[MaterialType::WOOD] = woodStats;

        // Stone material
        ItemStats stoneStats;
        stoneStats.durability = 131.0f;
        stoneStats.currentDurability = 131.0f;
        stoneStats.attackDamage = 3.0f;
        stoneStats.attackSpeed = 1.6f;
        stoneStats.miningSpeed = 4.0f;
        stoneStats.efficiency = 1.0f;
        stoneStats.armorValue = 2.0f;
        stoneStats.maxStackSize = 64;
        stoneStats.isStackable = true;
        stoneStats.isEnchantable = true;
        stoneStats.isRepairable = true;
        stoneStats.isConsumable = false;
        m_materialTemplates[MaterialType::STONE] = stoneStats;

        // Iron material
        ItemStats ironStats;
        ironStats.durability = 250.0f;
        ironStats.currentDurability = 250.0f;
        ironStats.attackDamage = 4.0f;
        ironStats.attackSpeed = 1.6f;
        ironStats.miningSpeed = 6.0f;
        ironStats.efficiency = 1.0f;
        ironStats.armorValue = 3.0f;
        ironStats.maxStackSize = 64;
        ironStats.isStackable = true;
        ironStats.isEnchantable = true;
        ironStats.isRepairable = true;
        ironStats.isConsumable = false;
        m_materialTemplates[MaterialType::IRON] = ironStats;

        // Diamond material
        ItemStats diamondStats;
        diamondStats.durability = 1561.0f;
        diamondStats.currentDurability = 1561.0f;
        diamondStats.attackDamage = 5.0f;
        diamondStats.attackSpeed = 1.6f;
        diamondStats.miningSpeed = 8.0f;
        diamondStats.efficiency = 1.0f;
        diamondStats.armorValue = 4.0f;
        diamondStats.maxStackSize = 64;
        diamondStats.isStackable = true;
        diamondStats.isEnchantable = true;
        diamondStats.isRepairable = true;
        diamondStats.isConsumable = false;
        m_materialTemplates[MaterialType::DIAMOND] = diamondStats;

        // Netherite material
        ItemStats netheriteStats;
        netheriteStats.durability = 2031.0f;
        netheriteStats.currentDurability = 2031.0f;
        netheriteStats.attackDamage = 6.0f;
        netheriteStats.attackSpeed = 1.6f;
        netheriteStats.miningSpeed = 9.0f;
        netheriteStats.efficiency = 1.0f;
        netheriteStats.armorValue = 5.0f;
        netheriteStats.maxStackSize = 64;
        netheriteStats.isStackable = true;
        netheriteStats.isEnchantable = true;
        netheriteStats.isRepairable = true;
        netheriteStats.isConsumable = false;
        m_materialTemplates[MaterialType::NETHERITE] = netheriteStats;

        // Crystal material (magical)
        ItemStats crystalStats;
        crystalStats.durability = 800.0f;
        crystalStats.currentDurability = 800.0f;
        crystalStats.attackDamage = 4.0f;
        crystalStats.attackSpeed = 1.8f;
        crystalStats.miningSpeed = 7.0f;
        crystalStats.efficiency = 1.2f;
        crystalStats.armorValue = 3.0f;
        crystalStats.maxStackSize = 32;
        crystalStats.isStackable = true;
        crystalStats.isEnchantable = true;
        crystalStats.isRepairable = true;
        crystalStats.isConsumable = false;
        m_materialTemplates[MaterialType::CRYSTAL] = crystalStats;

        // Bone material
        ItemStats boneStats;
        boneStats.durability = 100.0f;
        boneStats.currentDurability = 100.0f;
        boneStats.attackDamage = 2.5f;
        boneStats.attackSpeed = 1.4f;
        boneStats.miningSpeed = 2.5f;
        boneStats.efficiency = 0.8f;
        boneStats.armorValue = 1.5f;
        boneStats.maxStackSize = 64;
        boneStats.isStackable = true;
        boneStats.isEnchantable = true;
        boneStats.isRepairable = true;
        boneStats.isConsumable = false;
        m_materialTemplates[MaterialType::BONE] = boneStats;
    }

    void ItemGenerator::InitializeToolTemplates() {
        // Sword template
        ItemStats swordStats;
        swordStats.attackDamage = 4.0f; // Base damage, modified by material
        swordStats.attackSpeed = 1.6f;
        swordStats.miningSpeed = 1.5f;
        swordStats.efficiency = 1.0f;
        swordStats.isStackable = false;
        swordStats.maxStackSize = 1;
        m_toolTemplates[ToolType::SWORD] = swordStats;

        // Pickaxe template
        ItemStats pickaxeStats;
        pickaxeStats.attackDamage = 2.0f;
        pickaxeStats.attackSpeed = 1.2f;
        pickaxeStats.miningSpeed = 1.0f; // Base mining speed, modified by material
        pickaxeStats.efficiency = 1.0f;
        pickaxeStats.isStackable = false;
        pickaxeStats.maxStackSize = 1;
        m_toolTemplates[ToolType::PICKAXE] = pickaxeStats;

        // Axe template
        ItemStats axeStats;
        axeStats.attackDamage = 3.0f; // Higher damage for axes
        axeStats.attackSpeed = 0.8f; // Slower attack speed
        axeStats.miningSpeed = 1.0f;
        axeStats.efficiency = 1.0f;
        axeStats.isStackable = false;
        axeStats.maxStackSize = 1;
        m_toolTemplates[ToolType::AXE] = axeStats;

        // Shovel template
        ItemStats shovelStats;
        shovelStats.attackDamage = 2.5f;
        shovelStats.attackSpeed = 1.0f;
        shovelStats.miningSpeed = 1.0f;
        shovelStats.efficiency = 1.0f;
        shovelStats.isStackable = false;
        shovelStats.maxStackSize = 1;
        m_toolTemplates[ToolType::SHOVEL] = shovelStats;

        // Bow template
        ItemStats bowStats;
        bowStats.attackDamage = 1.0f; // Base damage, modified by material
        bowStats.attackSpeed = 1.0f;
        bowStats.isStackable = false;
        bowStats.maxStackSize = 1;
        m_toolTemplates[ToolType::BOW] = bowStats;

        // Staff template
        ItemStats staffStats;
        staffStats.attackDamage = 3.0f;
        staffStats.attackSpeed = 1.0f;
        staffStats.isStackable = false;
        staffStats.maxStackSize = 1;
        m_toolTemplates[ToolType::STAFF] = staffStats;
    }

    void ItemGenerator::InitializeGenerators() {
        // Tool generators
        m_toolGenerators["sword"] = [this](MaterialType material, ItemRarity rarity) -> GeneratedItem {
            return GenerateSword(material, rarity);
        };

        m_toolGenerators["pickaxe"] = [this](MaterialType material, ItemRarity rarity) -> GeneratedItem {
            return GeneratePickaxe(material, rarity);
        };

        m_toolGenerators["axe"] = [this](MaterialType material, ItemRarity rarity) -> GeneratedItem {
            return GenerateAxe(material, rarity);
        };

        m_toolGenerators["shovel"] = [this](MaterialType material, ItemRarity rarity) -> GeneratedItem {
            return GenerateShovel(material, rarity);
        };

        m_toolGenerators["hoe"] = [this](MaterialType material, ItemRarity rarity) -> GeneratedItem {
            return GenerateHoe(material, rarity);
        };

        m_toolGenerators["bow"] = [this](MaterialType material, ItemRarity rarity) -> GeneratedItem {
            return GenerateBow(material, rarity);
        };

        m_toolGenerators["staff"] = [this](MaterialType material, ItemRarity rarity) -> GeneratedItem {
            return GenerateStaff(material, rarity);
        };

        m_toolGenerators["hammer"] = [this](MaterialType material, ItemRarity rarity) -> GeneratedItem {
            return GenerateHammer(material, rarity);
        };

        // Armor generators
        m_armorGenerators["helmet"] = [this](MaterialType material, ItemRarity rarity) -> GeneratedItem {
            return GenerateHelmet(material, rarity);
        };

        m_armorGenerators["chestplate"] = [this](MaterialType material, ItemRarity rarity) -> GeneratedItem {
            return GenerateChestplate(material, rarity);
        };

        m_armorGenerators["leggings"] = [this](MaterialType material, ItemRarity rarity) -> GeneratedItem {
            return GenerateLeggings(material, rarity);
        };

        m_armorGenerators["boots"] = [this](MaterialType material, ItemRarity rarity) -> GeneratedItem {
            return GenerateBoots(material, rarity);
        };

        // Consumable generators
        m_consumableGenerators["food"] = [this](ItemRarity rarity) -> GeneratedItem {
            return GenerateFood(rarity);
        };

        m_consumableGenerators["potion"] = [this](ItemRarity rarity) -> GeneratedItem {
            return GeneratePotion(rarity);
        };

        m_consumableGenerators["scroll"] = [this](ItemRarity rarity) -> GeneratedItem {
            return GenerateScroll(rarity);
        };

        // Special item generators
        m_specialGenerators["artifact"] = [this](ItemRarity rarity) -> GeneratedItem {
            return GenerateArtifact(rarity);
        };

        m_specialGenerators["relic"] = [this](ItemRarity rarity) -> GeneratedItem {
            return GenerateRelic(rarity);
        };

        m_specialGenerators["totem"] = [this](ItemRarity rarity) -> GeneratedItem {
            return GenerateTotem(rarity);
        };
    }

    void ItemGenerator::InitializeModelGenerators() {
        m_modelGenerators["tool"] = [this](const GeneratedItem& item) -> void* {
            return GenerateToolModel(item);
        };

        m_modelGenerators["weapon"] = [this](const GeneratedItem& item) -> void* {
            return GenerateWeaponModel(item);
        };

        m_modelGenerators["armor"] = [this](const GeneratedItem& item) -> void* {
            return GenerateArmorModel(item);
        };

        m_modelGenerators["consumable"] = [this](const GeneratedItem& item) -> void* {
            return GenerateConsumableModel(item);
        };

        m_modelGenerators["special"] = [this](const GeneratedItem& item) -> void* {
            return GenerateSpecialModel(item);
        };
    }

    void ItemGenerator::InitializeTextureGenerators() {
        m_textureGenerators["tool"] = [this](const GeneratedItem& item) -> void* {
            return GenerateToolTexture(item);
        };

        m_textureGenerators["weapon"] = [this](const GeneratedItem& item) -> void* {
            return GenerateWeaponTexture(item);
        };

        m_textureGenerators["armor"] = [this](const GeneratedItem& item) -> void* {
            return GenerateArmorTexture(item);
        };

        m_textureGenerators["consumable"] = [this](const GeneratedItem& item) -> void* {
            return GenerateConsumableTexture(item);
        };

        m_textureGenerators["special"] = [this](const GeneratedItem& item) -> void* {
            return GenerateSpecialTexture(item);
        };
    }

    void ItemGenerator::InitializeSoundGenerators() {
        m_soundGenerators["tool"] = [this](const GeneratedItem& item) -> void* {
            return GenerateToolSounds(item);
        };

        m_soundGenerators["weapon"] = [this](const GeneratedItem& item) -> void* {
            return GenerateWeaponSounds(item);
        };

        m_soundGenerators["armor"] = [this](const GeneratedItem& item) -> void* {
            return GenerateArmorSounds(item);
        };

        m_soundGenerators["consumable"] = [this](const GeneratedItem& item) -> void* {
            return GenerateConsumableSounds(item);
        };

        m_soundGenerators["special"] = [this](const GeneratedItem& item) -> void* {
            return GenerateSpecialSounds(item);
        };
    }

    GeneratedItem ItemGenerator::GenerateTool(ToolType toolType, MaterialType materialType, ItemRarity rarity) {
        std::string toolName;
        switch (toolType) {
            case ToolType::SWORD: toolName = "sword"; break;
            case ToolType::PICKAXE: toolName = "pickaxe"; break;
            case ToolType::AXE: toolName = "axe"; break;
            case ToolType::SHOVEL: toolName = "shovel"; break;
            case ToolType::HOE: toolName = "hoe"; break;
            case ToolType::BOW: toolName = "bow"; break;
            case ToolType::STAFF: toolName = "staff"; break;
            case ToolType::HAMMER: toolName = "hammer"; break;
            default: toolName = "tool"; break;
        }

        auto it = m_toolGenerators.find(toolName);
        if (it != m_toolGenerators.end()) {
            GeneratedItem item = it->second(materialType, rarity);
            ApplyRarityModifiers(item, rarity);
            return item;
        }

        // Fallback to generic tool generation
        return GenerateGenericTool(toolType, materialType, rarity);
    }

    GeneratedItem ItemGenerator::GenerateRandomTool(ItemRarity minRarity, ItemRarity maxRarity) {
        std::vector<ToolType> toolTypes = {
            ToolType::SWORD, ToolType::PICKAXE, ToolType::AXE, ToolType::SHOVEL,
            ToolType::HOE, ToolType::BOW, ToolType::STAFF, ToolType::HAMMER
        };

        std::vector<MaterialType> materialTypes = {
            MaterialType::WOOD, MaterialType::STONE, MaterialType::IRON,
            MaterialType::DIAMOND, MaterialType::NETHERITE, MaterialType::CRYSTAL
        };

        std::uniform_int_distribution<size_t> toolDist(0, toolTypes.size() - 1);
        std::uniform_int_distribution<size_t> materialDist(0, materialTypes.size() - 1);
        std::uniform_int_distribution<int> rarityDist(static_cast<int>(minRarity), static_cast<int>(maxRarity));

        ToolType toolType = toolTypes[toolDist(m_randomEngine)];
        MaterialType materialType = materialTypes[materialDist(m_randomEngine)];
        ItemRarity rarity = static_cast<ItemRarity>(rarityDist(m_randomEngine));

        return GenerateTool(toolType, materialType, rarity);
    }

    GeneratedItem ItemGenerator::GenerateWeapon(ToolType weaponType, MaterialType materialType, ItemRarity rarity) {
        // Weapons are similar to tools but with different stats
        GeneratedItem item = GenerateTool(weaponType, materialType, rarity);
        item.type = ItemType::TOOL;

        // Increase damage for weapons
        item.stats.attackDamage *= 1.5f;
        item.stats.attackSpeed *= 1.2f;

        return item;
    }

    GeneratedItem ItemGenerator::GenerateArmor(const std::string& armorSlot, MaterialType materialType, ItemRarity rarity) {
        auto it = m_armorGenerators.find(armorSlot);
        if (it != m_armorGenerators.end()) {
            GeneratedItem item = it->second(materialType, rarity);
            ApplyRarityModifiers(item, rarity);
            return item;
        }

        // Fallback to generic armor generation
        return GenerateGenericArmor(armorSlot, materialType, rarity);
    }

    GeneratedItem ItemGenerator::GenerateConsumable(const std::string& consumableType, ItemRarity rarity) {
        auto it = m_consumableGenerators.find(consumableType);
        if (it != m_consumableGenerators.end()) {
            GeneratedItem item = it->second(rarity);
            ApplyRarityModifiers(item, rarity);
            return item;
        }

        // Fallback to generic consumable generation
        return GenerateGenericConsumable(consumableType, rarity);
    }

    GeneratedItem ItemGenerator::GenerateMaterial(MaterialType materialType, ItemRarity rarity) {
        GeneratedItem item;
        item.type = ItemType::MATERIAL;
        item.materialType = materialType;
        item.rarity = rarity;

        std::string materialName = GetMaterialName(materialType);
        item.name = materialName + "_material";
        item.displayName = GenerateRarityFormattedName(materialName + " Material", rarity);

        item.stats = m_materialTemplates[materialType];
        item.stats.maxStackSize = 64;
        item.stats.isStackable = true;

        // Generate ID
        std::stringstream ss;
        ss << "material_" << materialName << "_" << static_cast<int>(rarity);
        item.id = ss.str();

        ApplyRarityModifiers(item, rarity);

        // Generate appearance
        item.appearance.scale = 1.0f;
        item.appearance.variant = std::uniform_int_distribution<int>(0, 5)(m_randomEngine);

        // Generate procedural content
        item.modelData = GenerateItemModel(item);
        item.textureData = GenerateItemTexture(item);
        item.soundData = GenerateItemSounds(item);

        return item;
    }

    GeneratedItem ItemGenerator::GenerateSpecialItem(const std::string& specialType, ItemRarity rarity) {
        auto it = m_specialGenerators.find(specialType);
        if (it != m_specialGenerators.end()) {
            GeneratedItem item = it->second(rarity);
            ApplyRarityModifiers(item, rarity);
            return item;
        }

        // Fallback to generic special item generation
        return GenerateGenericSpecialItem(specialType, rarity);
    }

    GeneratedItem ItemGenerator::GenerateRandomItem(ItemType itemType, ItemRarity minRarity, ItemRarity maxRarity) {
        std::uniform_int_distribution<int> rarityDist(static_cast<int>(minRarity), static_cast<int>(maxRarity));
        ItemRarity rarity = static_cast<ItemRarity>(rarityDist(m_randomEngine));

        switch (itemType) {
            case ItemType::TOOL:
                return GenerateRandomTool(minRarity, maxRarity);
            case ItemType::ARMOR: {
                std::vector<std::string> armorSlots = {"helmet", "chestplate", "leggings", "boots"};
                std::uniform_int_distribution<size_t> slotDist(0, armorSlots.size() - 1);
                std::vector<MaterialType> materials = {MaterialType::LEATHER, MaterialType::IRON, MaterialType::DIAMOND, MaterialType::NETHERITE};
                std::uniform_int_distribution<size_t> materialDist(0, materials.size() - 1);
                return GenerateArmor(armorSlots[slotDist(m_randomEngine)], materials[materialDist(m_randomEngine)], rarity);
            }
            case ItemType::CONSUMABLE: {
                std::vector<std::string> consumableTypes = {"food", "potion", "scroll"};
                std::uniform_int_distribution<size_t> typeDist(0, consumableTypes.size() - 1);
                return GenerateConsumable(consumableTypes[typeDist(m_randomEngine)], rarity);
            }
            case ItemType::MATERIAL: {
                std::vector<MaterialType> materials = {MaterialType::IRON, MaterialType::DIAMOND, MaterialType::CRYSTAL, MaterialType::EMERALD};
                std::uniform_int_distribution<size_t> materialDist(0, materials.size() - 1);
                return GenerateMaterial(materials[materialDist(m_randomEngine)], rarity);
            }
            case ItemType::SPECIAL: {
                std::vector<std::string> specialTypes = {"artifact", "relic", "totem"};
                std::uniform_int_distribution<size_t> typeDist(0, specialTypes.size() - 1);
                return GenerateSpecialItem(specialTypes[typeDist(m_randomEngine)], rarity);
            }
            default:
                return GenerateRandomTool(minRarity, maxRarity);
        }
    }

    void* ItemGenerator::GenerateItemModel(const GeneratedItem& item) {
        std::string modelType;
        switch (item.type) {
            case ItemType::TOOL: modelType = "tool"; break;
            case ItemType::ARMOR: modelType = "armor"; break;
            case ItemType::CONSUMABLE: modelType = "consumable"; break;
            case ItemType::SPECIAL: modelType = "special"; break;
            default: modelType = "tool"; break;
        }

        auto it = m_modelGenerators.find(modelType);
        if (it != m_modelGenerators.end()) {
            return it->second(item);
        }
        return nullptr;
    }

    void* ItemGenerator::GenerateItemTexture(const GeneratedItem& item) {
        std::string textureType;
        switch (item.type) {
            case ItemType::TOOL: textureType = "tool"; break;
            case ItemType::ARMOR: textureType = "armor"; break;
            case ItemType::CONSUMABLE: textureType = "consumable"; break;
            case ItemType::SPECIAL: textureType = "special"; break;
            default: textureType = "tool"; break;
        }

        auto it = m_textureGenerators.find(textureType);
        if (it != m_textureGenerators.end()) {
            return it->second(item);
        }
        return nullptr;
    }

    void* ItemGenerator::GenerateItemSounds(const GeneratedItem& item) {
        std::string soundType;
        switch (item.type) {
            case ItemType::TOOL: soundType = "tool"; break;
            case ItemType::ARMOR: soundType = "armor"; break;
            case ItemType::CONSUMABLE: soundType = "consumable"; break;
            case ItemType::SPECIAL: soundType = "special"; break;
            default: soundType = "tool"; break;
        }

        auto it = m_soundGenerators.find(soundType);
        if (it != m_soundGenerators.end()) {
            return it->second(item);
        }
        return nullptr;
    }

    void ItemGenerator::ApplyRandomEnchantments(GeneratedItem& item, int maxEnchantments) {
        if (!item.stats.isEnchantable) return;

        std::uniform_int_distribution<int> enchantCountDist(0, maxEnchantments);
        int numEnchantments = enchantCountDist(m_randomEngine);

        for (int i = 0; i < numEnchantments; ++i) {
            std::uniform_int_distribution<int> levelDist(1, 5);
            std::string enchantment = GenerateRandomEnchantment(item.type, levelDist(m_randomEngine));

            if (!enchantment.empty() && IsEnchantmentCompatible(enchantment, item.type)) {
                item.enchantments.push_back(enchantment);

                // Apply enchantment effects
                ApplyEnchantmentEffects(item, enchantment);
            }
        }
    }

    // Tool generation implementations
    GeneratedItem ItemGenerator::GenerateSword(MaterialType material, ItemRarity rarity) {
        GeneratedItem item;
        item.type = ItemType::TOOL;
        item.toolType = ToolType::SWORD;
        item.materialType = material;
        item.rarity = rarity;

        std::string materialName = GetMaterialName(material);
        item.name = materialName + "_sword";
        item.displayName = GenerateRarityFormattedName(materialName + " Sword", rarity);

        // Combine material and tool stats
        item.stats = m_materialTemplates[material];
        ItemStats toolStats = m_toolTemplates[ToolType::SWORD];
        item.stats.attackDamage = toolStats.attackDamage;
        item.stats.attackSpeed = toolStats.attackSpeed;
        item.stats.miningSpeed = toolStats.miningSpeed;

        // Generate ID
        std::stringstream ss;
        ss << "sword_" << materialName << "_" << static_cast<int>(rarity);
        item.id = ss.str();

        // Generate appearance
        item.appearance.scale = 1.0f + (std::uniform_real_distribution<float>(-0.1f, 0.2f)(m_randomEngine));
        item.appearance.variant = std::uniform_int_distribution<int>(0, 10)(m_randomEngine);

        // Generate procedural content
        item.modelData = GenerateItemModel(item);
        item.textureData = GenerateItemTexture(item);
        item.soundData = GenerateItemSounds(item);

        // Generate lore
        item.lore = GenerateItemLore(item);

        return item;
    }

    GeneratedItem ItemGenerator::GeneratePickaxe(MaterialType material, ItemRarity rarity) {
        GeneratedItem item;
        item.type = ItemType::TOOL;
        item.toolType = ToolType::PICKAXE;
        item.materialType = material;
        item.rarity = rarity;

        std::string materialName = GetMaterialName(material);
        item.name = materialName + "_pickaxe";
        item.displayName = GenerateRarityFormattedName(materialName + " Pickaxe", rarity);

        // Combine material and tool stats
        item.stats = m_materialTemplates[material];
        ItemStats toolStats = m_toolTemplates[ToolType::PICKAXE];
        item.stats.attackDamage = toolStats.attackDamage;
        item.stats.attackSpeed = toolStats.attackSpeed;
        item.stats.miningSpeed = toolStats.miningSpeed;

        // Generate ID
        std::stringstream ss;
        ss << "pickaxe_" << materialName << "_" << static_cast<int>(rarity);
        item.id = ss.str();

        // Generate appearance
        item.appearance.scale = 1.0f + (std::uniform_real_distribution<float>(-0.1f, 0.2f)(m_randomEngine));
        item.appearance.variant = std::uniform_int_distribution<int>(0, 8)(m_randomEngine);

        // Generate procedural content
        item.modelData = GenerateItemModel(item);
        item.textureData = GenerateItemTexture(item);
        item.soundData = GenerateItemSounds(item);

        // Generate lore
        item.lore = GenerateItemLore(item);

        return item;
    }

    GeneratedItem ItemGenerator::GenerateAxe(MaterialType material, ItemRarity rarity) {
        GeneratedItem item;
        item.type = ItemType::TOOL;
        item.toolType = ToolType::AXE;
        item.materialType = material;
        item.rarity = rarity;

        std::string materialName = GetMaterialName(material);
        item.name = materialName + "_axe";
        item.displayName = GenerateRarityFormattedName(materialName + " Axe", rarity);

        // Combine material and tool stats
        item.stats = m_materialTemplates[material];
        ItemStats toolStats = m_toolTemplates[ToolType::AXE];
        item.stats.attackDamage = toolStats.attackDamage;
        item.stats.attackSpeed = toolStats.attackSpeed;
        item.stats.miningSpeed = toolStats.miningSpeed;

        // Generate ID
        std::stringstream ss;
        ss << "axe_" << materialName << "_" << static_cast<int>(rarity);
        item.id = ss.str();

        // Generate appearance
        item.appearance.scale = 1.0f + (std::uniform_real_distribution<float>(-0.1f, 0.2f)(m_randomEngine));
        item.appearance.variant = std::uniform_int_distribution<int>(0, 6)(m_randomEngine);

        // Generate procedural content
        item.modelData = GenerateItemModel(item);
        item.textureData = GenerateItemTexture(item);
        item.soundData = GenerateItemSounds(item);

        // Generate lore
        item.lore = GenerateItemLore(item);

        return item;
    }

    GeneratedItem ItemGenerator::GenerateShovel(MaterialType material, ItemRarity rarity) {
        GeneratedItem item;
        item.type = ItemType::TOOL;
        item.toolType = ToolType::SHOVEL;
        item.materialType = material;
        item.rarity = rarity;

        std::string materialName = GetMaterialName(material);
        item.name = materialName + "_shovel";
        item.displayName = GenerateRarityFormattedName(materialName + " Shovel", rarity);

        // Combine material and tool stats
        item.stats = m_materialTemplates[material];
        ItemStats toolStats = m_toolTemplates[ToolType::SHOVEL];
        item.stats.attackDamage = toolStats.attackDamage;
        item.stats.attackSpeed = toolStats.attackSpeed;
        item.stats.miningSpeed = toolStats.miningSpeed;

        // Generate ID
        std::stringstream ss;
        ss << "shovel_" << materialName << "_" << static_cast<int>(rarity);
        item.id = ss.str();

        // Generate appearance
        item.appearance.scale = 1.0f + (std::uniform_real_distribution<float>(-0.1f, 0.2f)(m_randomEngine));
        item.appearance.variant = std::uniform_int_distribution<int>(0, 5)(m_randomEngine);

        // Generate procedural content
        item.modelData = GenerateItemModel(item);
        item.textureData = GenerateItemTexture(item);
        item.soundData = GenerateItemSounds(item);

        // Generate lore
        item.lore = GenerateItemLore(item);

        return item;
    }

    GeneratedItem ItemGenerator::GenerateHoe(MaterialType material, ItemRarity rarity) {
        GeneratedItem item;
        item.type = ItemType::TOOL;
        item.toolType = ToolType::HOE;
        item.materialType = material;
        item.rarity = rarity;

        std::string materialName = GetMaterialName(material);
        item.name = materialName + "_hoe";
        item.displayName = GenerateRarityFormattedName(materialName + " Hoe", rarity);

        // Combine material and tool stats
        item.stats = m_materialTemplates[material];
        ItemStats toolStats = m_toolTemplates[ToolType::HOE];
        item.stats.attackDamage = toolStats.attackDamage;
        item.stats.attackSpeed = toolStats.attackSpeed;
        item.stats.miningSpeed = toolStats.miningSpeed;

        // Generate ID
        std::stringstream ss;
        ss << "hoe_" << materialName << "_" << static_cast<int>(rarity);
        item.id = ss.str();

        // Generate appearance
        item.appearance.scale = 1.0f + (std::uniform_real_distribution<float>(-0.1f, 0.2f)(m_randomEngine));
        item.appearance.variant = std::uniform_int_distribution<int>(0, 4)(m_randomEngine);

        // Generate procedural content
        item.modelData = GenerateItemModel(item);
        item.textureData = GenerateItemTexture(item);
        item.soundData = GenerateItemSounds(item);

        // Generate lore
        item.lore = GenerateItemLore(item);

        return item;
    }

    GeneratedItem ItemGenerator::GenerateBow(MaterialType material, ItemRarity rarity) {
        GeneratedItem item;
        item.type = ItemType::TOOL;
        item.toolType = ToolType::BOW;
        item.materialType = material;
        item.rarity = rarity;

        std::string materialName = GetMaterialName(material);
        item.name = materialName + "_bow";
        item.displayName = GenerateRarityFormattedName(materialName + " Bow", rarity);

        // Combine material and tool stats
        item.stats = m_materialTemplates[material];
        ItemStats toolStats = m_toolTemplates[ToolType::BOW];
        item.stats.attackDamage = toolStats.attackDamage;
        item.stats.attackSpeed = toolStats.attackSpeed;

        // Generate ID
        std::stringstream ss;
        ss << "bow_" << materialName << "_" << static_cast<int>(rarity);
        item.id = ss.str();

        // Generate appearance
        item.appearance.scale = 1.0f + (std::uniform_real_distribution<float>(-0.1f, 0.2f)(m_randomEngine));
        item.appearance.variant = std::uniform_int_distribution<int>(0, 7)(m_randomEngine);

        // Generate procedural content
        item.modelData = GenerateItemModel(item);
        item.textureData = GenerateItemTexture(item);
        item.soundData = GenerateItemSounds(item);

        // Generate lore
        item.lore = GenerateItemLore(item);

        return item;
    }

    GeneratedItem ItemGenerator::GenerateStaff(MaterialType material, ItemRarity rarity) {
        GeneratedItem item;
        item.type = ItemType::TOOL;
        item.toolType = ToolType::STAFF;
        item.materialType = material;
        item.rarity = rarity;

        std::string materialName = GetMaterialName(material);
        item.name = materialName + "_staff";
        item.displayName = GenerateRarityFormattedName(materialName + " Staff", rarity);

        // Combine material and tool stats
        item.stats = m_materialTemplates[material];
        ItemStats toolStats = m_toolTemplates[ToolType::STAFF];
        item.stats.attackDamage = toolStats.attackDamage;
        item.stats.attackSpeed = toolStats.attackSpeed;

        // Generate ID
        std::stringstream ss;
        ss << "staff_" << materialName << "_" << static_cast<int>(rarity);
        item.id = ss.str();

        // Generate appearance
        item.appearance.scale = 1.0f + (std::uniform_real_distribution<float>(-0.1f, 0.2f)(m_randomEngine));
        item.appearance.variant = std::uniform_int_distribution<int>(0, 12)(m_randomEngine);

        // Add special effects for staffs
        item.appearance.hasGlow = true;
        item.appearance.hasParticles = true;
        item.appearance.specialEffects.push_back("magical_aura");

        // Generate procedural content
        item.modelData = GenerateItemModel(item);
        item.textureData = GenerateItemTexture(item);
        item.soundData = GenerateItemSounds(item);

        // Generate lore
        item.lore = GenerateItemLore(item);

        return item;
    }

    GeneratedItem ItemGenerator::GenerateHammer(MaterialType material, ItemRarity rarity) {
        GeneratedItem item;
        item.type = ItemType::TOOL;
        item.toolType = ToolType::HAMMER;
        item.materialType = material;
        item.rarity = rarity;

        std::string materialName = GetMaterialName(material);
        item.name = materialName + "_hammer";
        item.displayName = GenerateRarityFormattedName(materialName + " Hammer", rarity);

        // Combine material and tool stats
        item.stats = m_materialTemplates[material];
        item.stats.attackDamage *= 2.5f; // Hammers do more damage
        item.stats.attackSpeed *= 0.7f;  // But slower
        item.stats.miningSpeed *= 1.5f;  // Better mining

        // Generate ID
        std::stringstream ss;
        ss << "hammer_" << materialName << "_" << static_cast<int>(rarity);
        item.id = ss.str();

        // Generate appearance
        item.appearance.scale = 1.2f + (std::uniform_real_distribution<float>(-0.1f, 0.2f)(m_randomEngine));
        item.appearance.variant = std::uniform_int_distribution<int>(0, 8)(m_randomEngine);

        // Generate procedural content
        item.modelData = GenerateItemModel(item);
        item.textureData = GenerateItemTexture(item);
        item.soundData = GenerateItemSounds(item);

        // Generate lore
        item.lore = GenerateItemLore(item);

        return item;
    }

    // Armor generation methods
    GeneratedItem ItemGenerator::GenerateHelmet(MaterialType material, ItemRarity rarity) {
        GeneratedItem item;
        item.type = ItemType::ARMOR;
        item.materialType = material;
        item.rarity = rarity;

        std::string materialName = GetMaterialName(material);
        item.name = materialName + "_helmet";
        item.displayName = GenerateRarityFormattedName(materialName + " Helmet", rarity);

        item.stats = m_materialTemplates[material];
        item.stats.armorValue *= 0.15f; // 15% of total armor for helmet
        item.stats.isStackable = false;
        item.stats.maxStackSize = 1;

        std::stringstream ss;
        ss << "helmet_" << materialName << "_" << static_cast<int>(rarity);
        item.id = ss.str();

        item.appearance.scale = 1.0f;
        item.appearance.variant = std::uniform_int_distribution<int>(0, 6)(m_randomEngine);

        item.modelData = GenerateItemModel(item);
        item.textureData = GenerateItemTexture(item);
        item.soundData = GenerateItemSounds(item);

        item.lore = GenerateItemLore(item);

        return item;
    }

    GeneratedItem ItemGenerator::GenerateChestplate(MaterialType material, ItemRarity rarity) {
        GeneratedItem item;
        item.type = ItemType::ARMOR;
        item.materialType = material;
        item.rarity = rarity;

        std::string materialName = GetMaterialName(material);
        item.name = materialName + "_chestplate";
        item.displayName = GenerateRarityFormattedName(materialName + " Chestplate", rarity);

        item.stats = m_materialTemplates[material];
        item.stats.armorValue *= 0.4f; // 40% of total armor for chestplate
        item.stats.isStackable = false;
        item.stats.maxStackSize = 1;

        std::stringstream ss;
        ss << "chestplate_" << materialName << "_" << static_cast<int>(rarity);
        item.id = ss.str();

        item.appearance.scale = 1.0f;
        item.appearance.variant = std::uniform_int_distribution<int>(0, 8)(m_randomEngine);

        item.modelData = GenerateItemModel(item);
        item.textureData = GenerateItemTexture(item);
        item.soundData = GenerateItemSounds(item);

        item.lore = GenerateItemLore(item);

        return item;
    }

    GeneratedItem ItemGenerator::GenerateLeggings(MaterialType material, ItemRarity rarity) {
        GeneratedItem item;
        item.type = ItemType::ARMOR;
        item.materialType = material;
        item.rarity = rarity;

        std::string materialName = GetMaterialName(material);
        item.name = materialName + "_leggings";
        item.displayName = GenerateRarityFormattedName(materialName + " Leggings", rarity);

        item.stats = m_materialTemplates[material];
        item.stats.armorValue *= 0.3f; // 30% of total armor for leggings
        item.stats.isStackable = false;
        item.stats.maxStackSize = 1;

        std::stringstream ss;
        ss << "leggings_" << materialName << "_" << static_cast<int>(rarity);
        item.id = ss.str();

        item.appearance.scale = 1.0f;
        item.appearance.variant = std::uniform_int_distribution<int>(0, 5)(m_randomEngine);

        item.modelData = GenerateItemModel(item);
        item.textureData = GenerateItemTexture(item);
        item.soundData = GenerateItemSounds(item);

        item.lore = GenerateItemLore(item);

        return item;
    }

    GeneratedItem ItemGenerator::GenerateBoots(MaterialType material, ItemRarity rarity) {
        GeneratedItem item;
        item.type = ItemType::ARMOR;
        item.materialType = material;
        item.rarity = rarity;

        std::string materialName = GetMaterialName(material);
        item.name = materialName + "_boots";
        item.displayName = GenerateRarityFormattedName(materialName + " Boots", rarity);

        item.stats = m_materialTemplates[material];
        item.stats.armorValue *= 0.15f; // 15% of total armor for boots
        item.stats.movementSpeedBonus = 0.01f; // Small speed bonus
        item.stats.isStackable = false;
        item.stats.maxStackSize = 1;

        std::stringstream ss;
        ss << "boots_" << materialName << "_" << static_cast<int>(rarity);
        item.id = ss.str();

        item.appearance.scale = 1.0f;
        item.appearance.variant = std::uniform_int_distribution<int>(0, 4)(m_randomEngine);

        item.modelData = GenerateItemModel(item);
        item.textureData = GenerateItemTexture(item);
        item.soundData = GenerateItemSounds(item);

        item.lore = GenerateItemLore(item);

        return item;
    }

    // Consumable generation methods
    GeneratedItem ItemGenerator::GenerateFood(ItemRarity rarity) {
        GeneratedItem item;
        item.type = ItemType::CONSUMABLE;
        item.rarity = rarity;

        std::vector<std::string> foodTypes = {
            "apple", "bread", "carrot", "potato", "beef", "porkchop", "chicken", "fish",
            "golden_apple", "enchanted_golden_apple", "mystery_meat", "dragon_fruit"
        };

        std::uniform_int_distribution<size_t> foodDist(0, foodTypes.size() - 1);
        std::string foodType = foodTypes[foodDist(m_randomEngine)];

        item.name = foodType;
        item.displayName = GenerateRarityFormattedName(GetFoodDisplayName(foodType), rarity);

        item.stats.isConsumable = true;
        item.stats.foodValue = GetFoodValue(foodType);
        item.stats.saturationValue = GetSaturationValue(foodType);
        item.stats.maxStackSize = 64;
        item.stats.isStackable = true;

        std::stringstream ss;
        ss << "food_" << foodType << "_" << static_cast<int>(rarity);
        item.id = ss.str();

        item.appearance.scale = 1.0f;
        item.appearance.variant = std::uniform_int_distribution<int>(0, 3)(m_randomEngine);

        item.modelData = GenerateItemModel(item);
        item.textureData = GenerateItemTexture(item);
        item.soundData = GenerateItemSounds(item);

        item.lore = GenerateItemLore(item);

        return item;
    }

    GeneratedItem ItemGenerator::GeneratePotion(ItemRarity rarity) {
        GeneratedItem item;
        item.type = ItemType::CONSUMABLE;
        item.rarity = rarity;

        std::vector<std::string> potionTypes = {
            "healing", "strength", "speed", "regeneration", "invisibility", "fire_resistance",
            "water_breathing", "night_vision", "poison", "weakness", "slowness", "luck"
        };

        std::uniform_int_distribution<size_t> potionDist(0, potionTypes.size() - 1);
        std::string potionType = potionTypes[potionDist(m_randomEngine)];

        item.name = potionType + "_potion";
        item.displayName = GenerateRarityFormattedName(GetPotionDisplayName(potionType), rarity);

        item.stats.isConsumable = true;
        item.stats.maxStackSize = 1;
        item.stats.isStackable = true;

        std::stringstream ss;
        ss << "potion_" << potionType << "_" << static_cast<int>(rarity);
        item.id = ss.str();

        item.appearance.scale = 1.0f;
        item.appearance.variant = std::uniform_int_distribution<int>(0, 5)(m_randomEngine);

        item.modelData = GenerateItemModel(item);
        item.textureData = GenerateItemTexture(item);
        item.soundData = GenerateItemSounds(item);

        item.lore = GenerateItemLore(item);

        return item;
    }

    GeneratedItem ItemGenerator::GenerateScroll(ItemRarity rarity) {
        GeneratedItem item;
        item.type = ItemType::CONSUMABLE;
        item.rarity = rarity;

        std::vector<std::string> scrollTypes = {
            "teleport", "fireball", "lightning", "healing", "shield", "summon",
            "invisibility", "strength", "speed", "wisdom", "luck", "curse_removal"
        };

        std::uniform_int_distribution<size_t> scrollDist(0, scrollTypes.size() - 1);
        std::string scrollType = scrollTypes[scrollDist(m_randomEngine)];

        item.name = scrollType + "_scroll";
        item.displayName = GenerateRarityFormattedName("Scroll of " + GetScrollDisplayName(scrollType), rarity);

        item.stats.isConsumable = true;
        item.stats.maxStackSize = 16;
        item.stats.isStackable = true;

        std::stringstream ss;
        ss << "scroll_" << scrollType << "_" << static_cast<int>(rarity);
        item.id = ss.str();

        item.appearance.scale = 1.0f;
        item.appearance.variant = std::uniform_int_distribution<int>(0, 4)(m_randomEngine);

        item.modelData = GenerateItemModel(item);
        item.textureData = GenerateItemTexture(item);
        item.soundData = GenerateItemSounds(item);

        item.lore = GenerateItemLore(item);

        return item;
    }

    // Special item generation methods
    GeneratedItem ItemGenerator::GenerateArtifact(ItemRarity rarity) {
        GeneratedItem item;
        item.type = ItemType::SPECIAL;
        item.rarity = rarity;

        std::vector<std::string> artifactTypes = {
            "ancient_sword", "cursed_amulet", "dragon_scale", "phoenix_feather",
            "time_crystal", "void_gem", "lightning_rod", "storm_bringer"
        };

        std::uniform_int_distribution<size_t> artifactDist(0, artifactTypes.size() - 1);
        std::string artifactType = artifactTypes[artifactDist(m_randomEngine)];

        item.name = artifactType;
        item.displayName = GenerateRarityFormattedName(GetArtifactDisplayName(artifactType), rarity);

        item.stats.isStackable = false;
        item.stats.maxStackSize = 1;
        item.stats.isEnchantable = false;
        item.appearance.hasGlow = true;

        std::stringstream ss;
        ss << "artifact_" << artifactType << "_" << static_cast<int>(rarity);
        item.id = ss.str();

        item.appearance.scale = 1.0f + (std::uniform_real_distribution<float>(-0.1f, 0.3f)(m_randomEngine));
        item.appearance.variant = std::uniform_int_distribution<int>(0, 15)(m_randomEngine);

        item.modelData = GenerateItemModel(item);
        item.textureData = GenerateItemTexture(item);
        item.soundData = GenerateItemSounds(item);

        item.lore = GenerateItemLore(item);

        return item;
    }

    GeneratedItem ItemGenerator::GenerateRelic(ItemRarity rarity) {
        GeneratedItem item;
        item.type = ItemType::SPECIAL;
        item.rarity = rarity;

        std::vector<std::string> relicTypes = {
            "lost_crown", "ancient_tome", "forgotten_ring", "cursed_idol",
            "sacred_relic", "demon_horn", "angel_wing", "chaos_orb"
        };

        std::uniform_int_distribution<size_t> relicDist(0, relicTypes.size() - 1);
        std::string relicType = relicTypes[relicDist(m_randomEngine)];

        item.name = relicType;
        item.displayName = GenerateRarityFormattedName(GetRelicDisplayName(relicType), rarity);

        item.stats.isStackable = false;
        item.stats.maxStackSize = 1;
        item.stats.isEnchantable = false;
        item.appearance.hasGlow = (rarity >= ItemRarity::RARE);

        std::stringstream ss;
        ss << "relic_" << relicType << "_" << static_cast<int>(rarity);
        item.id = ss.str();

        item.appearance.scale = 1.0f + (std::uniform_real_distribution<float>(-0.1f, 0.3f)(m_randomEngine));
        item.appearance.variant = std::uniform_int_distribution<int>(0, 10)(m_randomEngine);

        item.modelData = GenerateItemModel(item);
        item.textureData = GenerateItemTexture(item);
        item.soundData = GenerateItemSounds(item);

        item.lore = GenerateItemLore(item);

        return item;
    }

    GeneratedItem ItemGenerator::GenerateTotem(ItemRarity rarity) {
        GeneratedItem item;
        item.type = ItemType::SPECIAL;
        item.rarity = rarity;

        std::vector<std::string> totemTypes = {
            "undying", "resurrection", "protection", "strength", "speed",
            "healing", "fire", "water", "earth", "air"
        };

        std::uniform_int_distribution<size_t> totemDist(0, totemTypes.size() - 1);
        std::string totemType = totemTypes[totemDist(m_randomEngine)];

        item.name = totemType + "_totem";
        item.displayName = GenerateRarityFormattedName("Totem of " + GetTotemDisplayName(totemType), rarity);

        item.stats.isStackable = false;
        item.stats.maxStackSize = 1;
        item.stats.isEnchantable = false;
        item.appearance.hasGlow = true;
        item.appearance.hasParticles = true;

        std::stringstream ss;
        ss << "totem_" << totemType << "_" << static_cast<int>(rarity);
        item.id = ss.str();

        item.appearance.scale = 1.0f + (std::uniform_real_distribution<float>(-0.1f, 0.3f)(m_randomEngine));
        item.appearance.variant = std::uniform_int_distribution<int>(0, 8)(m_randomEngine);

        item.modelData = GenerateItemModel(item);
        item.textureData = GenerateItemTexture(item);
        item.soundData = GenerateItemSounds(item);

        item.lore = GenerateItemLore(item);

        return item;
    }

    // Generic generation methods for fallback
    GeneratedItem ItemGenerator::GenerateGenericTool(ToolType toolType, MaterialType materialType, ItemRarity rarity) {
        GeneratedItem item;
        item.type = ItemType::TOOL;
        item.toolType = toolType;
        item.materialType = materialType;
        item.rarity = rarity;

        std::string materialName = GetMaterialName(materialType);
        std::string toolName = GetToolName(toolType);

        item.name = materialName + "_" + toolName;
        item.displayName = GenerateRarityFormattedName(materialName + " " + toolName, rarity);

        item.stats = m_materialTemplates[materialType];
        if (m_toolTemplates.count(toolType)) {
            ItemStats toolStats = m_toolTemplates[toolType];
            item.stats.attackDamage = toolStats.attackDamage;
            item.stats.attackSpeed = toolStats.attackSpeed;
        }

        std::stringstream ss;
        ss << "tool_" << materialName << "_" << toolName << "_" << static_cast<int>(rarity);
        item.id = ss.str();

        item.appearance.scale = 1.0f + (std::uniform_real_distribution<float>(-0.1f, 0.2f)(m_randomEngine));
        item.appearance.variant = std::uniform_int_distribution<int>(0, 10)(m_randomEngine);

        item.modelData = GenerateItemModel(item);
        item.textureData = GenerateItemTexture(item);
        item.soundData = GenerateItemSounds(item);

        item.lore = GenerateItemLore(item);

        return item;
    }

    GeneratedItem ItemGenerator::GenerateGenericArmor(const std::string& armorSlot, MaterialType materialType, ItemRarity rarity) {
        GeneratedItem item;
        item.type = ItemType::ARMOR;
        item.materialType = materialType;
        item.rarity = rarity;

        std::string materialName = GetMaterialName(materialType);
        item.name = materialName + "_" + armorSlot;
        item.displayName = GenerateRarityFormattedName(materialName + " " + armorSlot, rarity);

        item.stats = m_materialTemplates[materialType];
        item.stats.isStackable = false;
        item.stats.maxStackSize = 1;

        std::stringstream ss;
        ss << "armor_" << materialName << "_" << armorSlot << "_" << static_cast<int>(rarity);
        item.id = ss.str();

        item.appearance.scale = 1.0f;
        item.appearance.variant = std::uniform_int_distribution<int>(0, 6)(m_randomEngine);

        item.modelData = GenerateItemModel(item);
        item.textureData = GenerateItemTexture(item);
        item.soundData = GenerateItemSounds(item);

        item.lore = GenerateItemLore(item);

        return item;
    }

    GeneratedItem ItemGenerator::GenerateGenericConsumable(const std::string& consumableType, ItemRarity rarity) {
        GeneratedItem item;
        item.type = ItemType::CONSUMABLE;
        item.rarity = rarity;

        item.name = consumableType;
        item.displayName = GenerateRarityFormattedName(consumableType, rarity);

        item.stats.isConsumable = true;
        item.stats.maxStackSize = 64;
        item.stats.isStackable = true;

        std::stringstream ss;
        ss << "consumable_" << consumableType << "_" << static_cast<int>(rarity);
        item.id = ss.str();

        item.appearance.scale = 1.0f;
        item.appearance.variant = std::uniform_int_distribution<int>(0, 3)(m_randomEngine);

        item.modelData = GenerateItemModel(item);
        item.textureData = GenerateItemTexture(item);
        item.soundData = GenerateItemSounds(item);

        item.lore = GenerateItemLore(item);

        return item;
    }

    GeneratedItem ItemGenerator::GenerateGenericSpecialItem(const std::string& specialType, ItemRarity rarity) {
        GeneratedItem item;
        item.type = ItemType::SPECIAL;
        item.rarity = rarity;

        item.name = specialType;
        item.displayName = GenerateRarityFormattedName(specialType, rarity);

        item.stats.isStackable = false;
        item.stats.maxStackSize = 1;
        item.stats.isEnchantable = false;

        std::stringstream ss;
        ss << "special_" << specialType << "_" << static_cast<int>(rarity);
        item.id = ss.str();

        item.appearance.scale = 1.0f + (std::uniform_real_distribution<float>(-0.1f, 0.3f)(m_randomEngine));
        item.appearance.variant = std::uniform_int_distribution<int>(0, 10)(m_randomEngine);

        item.modelData = GenerateItemModel(item);
        item.textureData = GenerateItemTexture(item);
        item.soundData = GenerateItemSounds(item);

        item.lore = GenerateItemLore(item);

        return item;
    }

    // Model generation implementations
    void* ItemGenerator::GenerateToolModel(const GeneratedItem& item) {
        ToolModel* model = new ToolModel();

        // Generate basic tool shape based on tool type
        switch (item.toolType) {
            case ToolType::SWORD:
                // Generate sword model
                model->vertices = GenerateSwordVertices(item);
                model->indices = GenerateSwordIndices();
                break;
            case ToolType::PICKAXE:
                // Generate pickaxe model
                model->vertices = GeneratePickaxeVertices(item);
                model->indices = GeneratePickaxeIndices();
                break;
            case ToolType::AXE:
                // Generate axe model
                model->vertices = GenerateAxeVertices(item);
                model->indices = GenerateAxeIndices();
                break;
            case ToolType::SHOVEL:
                // Generate shovel model
                model->vertices = GenerateShovelVertices(item);
                model->indices = GenerateShovelIndices();
                break;
            default:
                // Generate generic tool model
                model->vertices = GenerateGenericToolVertices(item);
                model->indices = GenerateGenericToolIndices();
                break;
        }

        // Set colors based on material
        SetMaterialColors(model, item.materialType);

        model->scale = item.appearance.scale;
        model->detailLevel = 1 + (item.rarity >= ItemRarity::RARE ? 1 : 0);

        return model;
    }

    void* ItemGenerator::GenerateWeaponModel(const GeneratedItem& item) {
        WeaponModel* model = new WeaponModel();

        // Generate weapon-specific model
        model->vertices = GenerateWeaponVertices(item);
        model->indices = GenerateWeaponIndices();

        // Set colors based on material
        SetWeaponMaterialColors(model, item.materialType);

        return model;
    }

    void* ItemGenerator::GenerateArmorModel(const GeneratedItem& item) {
        ArmorModel* model = new ArmorModel();

        // Generate armor-specific model based on type
        model->vertices = GenerateArmorVertices(item);
        model->indices = GenerateArmorIndices();

        // Set colors based on material
        SetArmorMaterialColors(model, item.materialType);

        return model;
    }

    void* ItemGenerator::GenerateConsumableModel(const GeneratedItem& item) {
        ConsumableModel* model = new ConsumableModel();

        // Generate consumable-specific model
        model->vertices = GenerateConsumableVertices(item);
        model->indices = GenerateConsumableIndices();

        // Set appropriate colors
        SetConsumableColors(model, item);

        return model;
    }

    void* ItemGenerator::GenerateSpecialModel(const GeneratedItem& item) {
        SpecialModel* model = new SpecialModel();

        // Generate special item model with effects
        model->vertices = GenerateSpecialVertices(item);
        model->indices = GenerateSpecialIndices();

        // Set special colors and effects
        SetSpecialColors(model, item);
        model->hasGlow = item.appearance.hasGlow;
        model->hasParticles = item.appearance.hasParticles;

        return model;
    }

    // Texture generation implementations
    void* ItemGenerator::GenerateToolTexture(const GeneratedItem& item) {
        ItemTexture* texture = new ItemTexture();

        // Generate tool texture based on material and type
        texture->width = 32;
        texture->height = 32;
        texture->data.resize(texture->width * texture->height * 4); // RGBA

        // Generate procedural texture data
        GenerateToolTextureData(texture, item);

        return texture;
    }

    void* ItemGenerator::GenerateWeaponTexture(const GeneratedItem& item) {
        ItemTexture* texture = new ItemTexture();

        texture->width = 32;
        texture->height = 32;
        texture->data.resize(texture->width * texture->height * 4);

        GenerateWeaponTextureData(texture, item);

        return texture;
    }

    void* ItemGenerator::GenerateArmorTexture(const GeneratedItem& item) {
        ItemTexture* texture = new ItemTexture();

        texture->width = 64;
        texture->height = 32;
        texture->data.resize(texture->width * texture->height * 4);

        GenerateArmorTextureData(texture, item);

        return texture;
    }

    void* ItemGenerator::GenerateConsumableTexture(const GeneratedItem& item) {
        ItemTexture* texture = new ItemTexture();

        texture->width = 16;
        texture->height = 16;
        texture->data.resize(texture->width * texture->height * 4);

        GenerateConsumableTextureData(texture, item);

        return texture;
    }

    void* ItemGenerator::GenerateSpecialTexture(const GeneratedItem& item) {
        ItemTexture* texture = new ItemTexture();

        texture->width = 32;
        texture->height = 32;
        texture->data.resize(texture->width * texture->height * 4);

        GenerateSpecialTextureData(texture, item);
        texture->hasGlow = item.appearance.hasGlow;

        return texture;
    }

    // Sound generation implementations
    void* ItemGenerator::GenerateToolSounds(const GeneratedItem& item) {
        ItemSounds* sounds = new ItemSounds();
        sounds->volume = 1.0f;
        sounds->pitch = 0.9f + (static_cast<float>(item.materialType) / 10.0f) * 0.3f;
        sounds->variant = static_cast<int>(item.materialType) % 3;
        return sounds;
    }

    void* ItemGenerator::GenerateWeaponSounds(const GeneratedItem& item) {
        ItemSounds* sounds = new ItemSounds();
        sounds->volume = 1.2f;
        sounds->pitch = 0.8f + (static_cast<float>(item.materialType) / 10.0f) * 0.4f;
        sounds->variant = static_cast<int>(item.materialType) % 4;
        return sounds;
    }

    void* ItemGenerator::GenerateArmorSounds(const GeneratedItem& item) {
        ItemSounds* sounds = new ItemSounds();
        sounds->volume = 0.8f;
        sounds->pitch = 1.0f + (static_cast<float>(item.materialType) / 10.0f) * 0.2f;
        sounds->variant = static_cast<int>(item.materialType) % 2;
        return sounds;
    }

    void* ItemGenerator::GenerateConsumableSounds(const GeneratedItem& item) {
        ItemSounds* sounds = new ItemSounds();
        sounds->volume = 0.6f;
        sounds->pitch = 1.0f;
        sounds->variant = std::uniform_int_distribution<int>(0, 2)(m_randomEngine);
        return sounds;
    }

    void* ItemGenerator::GenerateSpecialSounds(const GeneratedItem& item) {
        ItemSounds* sounds = new ItemSounds();
        sounds->volume = 1.0f;
        sounds->pitch = 0.5f + (static_cast<float>(item.rarity) / 10.0f) * 1.0f;
        sounds->variant = static_cast<int>(item.rarity) % 5;
        return sounds;
    }

    // Helper methods
    std::string ItemGenerator::GetMaterialName(MaterialType materialType) {
        switch (materialType) {
            case MaterialType::WOOD: return "Wooden";
            case MaterialType::STONE: return "Stone";
            case MaterialType::IRON: return "Iron";
            case MaterialType::DIAMOND: return "Diamond";
            case MaterialType::NETHERITE: return "Netherite";
            case MaterialType::CRYSTAL: return "Crystal";
            case MaterialType::BONE: return "Bone";
            case MaterialType::LEATHER: return "Leather";
            case MaterialType::CLOTH: return "Cloth";
            case MaterialType::OBSIDIAN: return "Obsidian";
            case MaterialType::EMERALD: return "Emerald";
            default: return "Unknown";
        }
    }

    std::string ItemGenerator::GetToolName(ToolType toolType) {
        switch (toolType) {
            case ToolType::SWORD: return "Sword";
            case ToolType::PICKAXE: return "Pickaxe";
            case ToolType::AXE: return "Axe";
            case ToolType::SHOVEL: return "Shovel";
            case ToolType::HOE: return "Hoe";
            case ToolType::BOW: return "Bow";
            case ToolType::STAFF: return "Staff";
            case ToolType::HAMMER: return "Hammer";
            default: return "Tool";
        }
    }

    std::string ItemGenerator::GenerateRarityFormattedName(const std::string& baseName, ItemRarity rarity) {
        switch (rarity) {
            case ItemRarity::COMMON: return baseName;
            case ItemRarity::UNCOMMON: return "§e" + baseName + "§r";
            case ItemRarity::RARE: return "§b" + baseName + "§r";
            case ItemRarity::EPIC: return "§d" + baseName + "§r";
            case ItemRarity::LEGENDARY: return "§6" + baseName + "§r";
            case ItemRarity::MYTHIC: return "§c" + baseName + "§r";
            default: return baseName;
        }
    }

    std::vector<std::string> ItemGenerator::GenerateItemLore(const GeneratedItem& item) {
        std::vector<std::string> lore;

        // Add rarity description
        switch (item.rarity) {
            case ItemRarity::COMMON:
                lore.push_back("§7A common item");
                break;
            case ItemRarity::UNCOMMON:
                lore.push_back("§eAn uncommon item");
                break;
            case ItemRarity::RARE:
                lore.push_back("§bA rare and valuable item");
                break;
            case ItemRarity::EPIC:
                lore.push_back("§dAn epic item of great power");
                break;
            case ItemRarity::LEGENDARY:
                lore.push_back("§6A legendary item of myth");
                break;
            case ItemRarity::MYTHIC:
                lore.push_back("§cA mythic item beyond legend");
                break;
        }

        // Add material description
        std::string materialDesc = "Made of " + GetMaterialName(item.materialType);
        lore.push_back("§7" + materialDesc);

        // Add stats if applicable
        if (item.stats.attackDamage > 0) {
            lore.push_back("§7Attack Damage: §c" + std::to_string(static_cast<int>(item.stats.attackDamage)));
        }

        if (item.stats.armorValue > 0) {
            lore.push_back("§7Armor: §a" + std::to_string(static_cast<int>(item.stats.armorValue)));
        }

        if (item.stats.durability > 0) {
            lore.push_back("§7Durability: §e" + std::to_string(static_cast<int>(item.stats.durability)));
        }

        // Add special properties
        if (item.appearance.hasGlow) {
            lore.push_back("§eGlowing");
        }

        if (!item.enchantments.empty()) {
            lore.push_back("§9Enchanted");
        }

        return lore;
    }

    void ItemGenerator::ApplyRarityModifiers(GeneratedItem& item, ItemRarity rarity) {
        float multiplier = 1.0f;

        switch (rarity) {
            case ItemRarity::COMMON: multiplier = 1.0f; break;
            case ItemRarity::UNCOMMON: multiplier = 1.1f; break;
            case ItemRarity::RARE: multiplier = 1.25f; break;
            case ItemRarity::EPIC: multiplier = 1.5f; break;
            case ItemRarity::LEGENDARY: multiplier = 2.0f; break;
            case ItemRarity::MYTHIC: multiplier = 3.0f; break;
        }

        item.stats.durability *= multiplier;
        item.stats.attackDamage *= multiplier;
        item.stats.armorValue *= multiplier;
        item.stats.miningSpeed *= multiplier;
    }

    std::string ItemGenerator::GenerateRandomEnchantment(ItemType itemType, int level) {
        std::vector<std::string> possibleEnchantments;

        switch (itemType) {
            case ItemType::TOOL:
                possibleEnchantments = {
                    "Efficiency " + std::to_string(level),
                    "Unbreaking " + std::to_string(level),
                    "Fortune " + std::to_string(level),
                    "Silk Touch"
                };
                break;
            case ItemType::ARMOR:
                possibleEnchantments = {
                    "Protection " + std::to_string(level),
                    "Fire Protection " + std::to_string(level),
                    "Blast Protection " + std::to_string(level),
                    "Projectile Protection " + std::to_string(level),
                    "Thorns " + std::to_string(level)
                };
                break;
            case ItemType::WEAPON:
                possibleEnchantments = {
                    "Sharpness " + std::to_string(level),
                    "Smite " + std::to_string(level),
                    "Bane of Arthropods " + std::to_string(level),
                    "Knockback " + std::to_string(level),
                    "Fire Aspect " + std::to_string(level),
                    "Looting " + std::to_string(level)
                };
                break;
            default:
                possibleEnchantments = {"Unknown Enchantment " + std::to_string(level)};
                break;
        }

        std::uniform_int_distribution<size_t> dist(0, possibleEnchantments.size() - 1);
        return possibleEnchantments[dist(m_randomEngine)];
    }

    bool ItemGenerator::IsEnchantmentCompatible(const std::string& enchantment, ItemType itemType) {
        // Basic compatibility check
        return true; // Simplified for now
    }

    void ItemGenerator::ApplyEnchantmentEffects(GeneratedItem& item, const std::string& enchantment) {
        if (enchantment.find("Efficiency") != std::string::npos) {
            item.stats.miningSpeed *= 1.2f;
        } else if (enchantment.find("Sharpness") != std::string::npos) {
            item.stats.attackDamage *= 1.2f;
        } else if (enchantment.find("Protection") != std::string::npos) {
            item.stats.armorValue *= 1.1f;
        }
    }

    // Display name helpers
    std::string ItemGenerator::GetFoodDisplayName(const std::string& foodType) {
        if (foodType == "golden_apple") return "Golden Apple";
        if (foodType == "enchanted_golden_apple") return "Enchanted Golden Apple";
        if (foodType == "mystery_meat") return "Mystery Meat";
        if (foodType == "dragon_fruit") return "Dragon Fruit";
        // Capitalize first letter
        std::string result = foodType;
        if (!result.empty()) {
            result[0] = std::toupper(result[0]);
        }
        return result;
    }

    std::string ItemGenerator::GetPotionDisplayName(const std::string& potionType) {
        std::string displayName = "Potion of ";
        displayName += potionType;
        displayName[0] = std::toupper(displayName[0]);
        return displayName;
    }

    std::string ItemGenerator::GetScrollDisplayName(const std::string& scrollType) {
        std::string displayName = scrollType;
        displayName[0] = std::toupper(displayName[0]);
        return displayName;
    }

    std::string ItemGenerator::GetArtifactDisplayName(const std::string& artifactType) {
        std::string displayName = artifactType;
        std::replace(displayName.begin(), displayName.end(), '_', ' ');
        displayName[0] = std::toupper(displayName[0]);
        return displayName;
    }

    std::string ItemGenerator::GetRelicDisplayName(const std::string& relicType) {
        std::string displayName = relicType;
        std::replace(displayName.begin(), displayName.end(), '_', ' ');
        displayName[0] = std::toupper(displayName[0]);
        return displayName;
    }

    std::string ItemGenerator::GetTotemDisplayName(const std::string& totemType) {
        std::string displayName = totemType;
        displayName[0] = std::toupper(displayName[0]);
        return displayName;
    }

    // Food value helpers
    int ItemGenerator::GetFoodValue(const std::string& foodType) {
        if (foodType == "golden_apple") return 4;
        if (foodType == "enchanted_golden_apple") return 4;
        if (foodType == "bread") return 5;
        if (foodType == "apple") return 4;
        if (foodType == "carrot" || foodType == "potato") return 3;
        if (foodType == "beef" || foodType == "porkchop" || foodType == "chicken") return 3;
        if (foodType == "fish") return 2;
        if (foodType == "mystery_meat") return 2;
        if (foodType == "dragon_fruit") return 8;
        return 2; // Default
    }

    float ItemGenerator::GetSaturationValue(const std::string& foodType) {
        if (foodType == "golden_apple") return 9.6f;
        if (foodType == "enchanted_golden_apple") return 9.6f;
        if (foodType == "bread") return 6.0f;
        if (foodType == "apple") return 2.4f;
        if (foodType == "carrot" || foodType == "potato") return 3.6f;
        if (foodType == "beef" || foodType == "porkchop" || foodType == "chicken") return 3.6f;
        if (foodType == "fish") return 1.2f;
        if (foodType == "mystery_meat") return 1.2f;
        if (foodType == "dragon_fruit") return 12.0f;
        return 1.2f; // Default
    }

    // Model generation helper methods (simplified implementations)
    std::vector<float> ItemGenerator::GenerateSwordVertices(const GeneratedItem& item) {
        // Generate sword vertices based on material and variant
        std::vector<float> vertices;
        float bladeLength = 1.0f + (item.appearance.variant % 3) * 0.2f;
        float bladeWidth = 0.1f + (item.appearance.variant % 2) * 0.05f;

        // Blade vertices
        vertices = {
            // Blade
            -bladeWidth, 0.0f, 0.0f,
            bladeWidth, 0.0f, 0.0f,
            bladeWidth, bladeLength, 0.0f,
            -bladeWidth, bladeLength, 0.0f,
            // Guard
            -0.3f, 0.1f, 0.0f,
            0.3f, 0.1f, 0.0f,
            0.3f, 0.0f, 0.0f,
            -0.3f, 0.0f, 0.0f,
            // Handle
            -0.05f, -0.5f, 0.0f,
            0.05f, -0.5f, 0.0f,
            0.05f, 0.0f, 0.0f,
            -0.05f, 0.0f, 0.0f
        };

        return vertices;
    }

    std::vector<unsigned int> ItemGenerator::GenerateSwordIndices() {
        return {
            0, 1, 2, 0, 2, 3,    // Blade
            4, 5, 6, 4, 6, 7,    // Guard
            8, 9, 10, 8, 10, 11 // Handle
        };
    }

    std::vector<float> ItemGenerator::GeneratePickaxeVertices(const GeneratedItem& item) {
        std::vector<float> vertices;
        float headSize = 0.3f + (item.appearance.variant % 2) * 0.1f;

        vertices = {
            // Pickaxe head
            -headSize, 0.0f, -0.1f,
            headSize, 0.0f, -0.1f,
            headSize, 0.0f, 0.1f,
            -headSize, 0.0f, 0.1f,
            0.0f, headSize, 0.0f,
            // Handle
            -0.05f, -0.8f, -0.05f,
            0.05f, -0.8f, -0.05f,
            0.05f, -0.8f, 0.05f,
            -0.05f, -0.8f, 0.05f,
            0.05f, 0.0f, 0.05f,
            -0.05f, 0.0f, 0.05f,
            0.05f, 0.0f, -0.05f,
            -0.05f, 0.0f, -0.05f
        };

        return vertices;
    }

    std::vector<unsigned int> ItemGenerator::GeneratePickaxeIndices() {
        return {
            0, 1, 4, 1, 2, 4, 2, 3, 4, 3, 0, 4, // Pickaxe head
            5, 6, 9, 5, 9, 8, 6, 7, 10, 6, 10, 9, // Handle
            8, 9, 11, 8, 11, 12, 9, 10, 13, 9, 13, 11
        };
    }

    std::vector<float> ItemGenerator::GenerateAxeVertices(const GeneratedItem& item) {
        std::vector<float> vertices;
        float bladeSize = 0.4f + (item.appearance.variant % 3) * 0.1f;

        vertices = {
            // Axe head
            -0.1f, 0.0f, -0.1f,
            bladeSize, 0.0f, 0.0f,
            bladeSize, 0.0f, 0.0f,
            0.0f, bladeSize, 0.0f,
            -0.1f, 0.0f, 0.1f,
            // Handle
            -0.05f, -0.8f, -0.05f,
            0.05f, -0.8f, -0.05f,
            0.05f, -0.8f, 0.05f,
            -0.05f, -0.8f, 0.05f,
            0.05f, 0.0f, 0.05f,
            -0.05f, 0.0f, 0.05f,
            0.05f, 0.0f, -0.05f,
            -0.05f, 0.0f, -0.05f
        };

        return vertices;
    }

    std::vector<unsigned int> ItemGenerator::GenerateAxeIndices() {
        return {
            0, 1, 3, 1, 2, 3, 2, 4, 3, 4, 0, 3, // Axe head
            5, 6, 9, 5, 9, 8, 6, 7, 10, 6, 10, 9, // Handle
            8, 9, 11, 8, 11, 12, 9, 10, 13, 9, 13, 11
        };
    }

    std::vector<float> ItemGenerator::GenerateShovelVertices(const GeneratedItem& item) {
        std::vector<float> vertices;
        float bladeSize = 0.25f + (item.appearance.variant % 2) * 0.1f;

        vertices = {
            // Shovel blade
            -bladeSize, 0.0f, -0.05f,
            bladeSize, 0.0f, -0.05f,
            bladeSize, bladeSize, 0.05f,
            -bladeSize, bladeSize, 0.05f,
            // Handle
            -0.05f, -0.7f, -0.05f,
            0.05f, -0.7f, -0.05f,
            0.05f, -0.7f, 0.05f,
            -0.05f, -0.7f, 0.05f,
            0.05f, 0.0f, 0.05f,
            -0.05f, 0.0f, 0.05f,
            0.05f, 0.0f, -0.05f,
            -0.05f, 0.0f, -0.05f
        };

        return vertices;
    }

    std::vector<unsigned int> ItemGenerator::GenerateShovelIndices() {
        return {
            0, 1, 2, 0, 2, 3, // Shovel blade
            4, 5, 8, 4, 8, 7, 5, 6, 9, 5, 9, 8, // Handle
            7, 8, 10, 7, 10, 11, 8, 9, 12, 8, 12, 10
        };
    }

    std::vector<float> ItemGenerator::GenerateGenericToolVertices(const GeneratedItem& item) {
        // Generic tool shape
        return {
            -0.1f, -0.5f, -0.05f,
            0.1f, -0.5f, -0.05f,
            0.1f, 0.0f, -0.05f,
            -0.1f, 0.0f, -0.05f,
            -0.1f, -0.5f, 0.05f,
            0.1f, -0.5f, 0.05f,
            0.1f, 0.0f, 0.05f,
            -0.1f, 0.0f, 0.05f
        };
    }

    std::vector<unsigned int> ItemGenerator::GenerateGenericToolIndices() {
        return {
            0, 1, 2, 0, 2, 3, // Front face
            4, 5, 6, 4, 6, 7, // Back face
            0, 1, 5, 0, 5, 4, // Bottom face
            3, 2, 6, 3, 6, 7, // Top face
            0, 3, 7, 0, 7, 4, // Left face
            1, 2, 6, 1, 6, 5  // Right face
        };
    }

    std::vector<float> ItemGenerator::GenerateWeaponVertices(const GeneratedItem& item) {
        return GenerateSwordVertices(item); // Default to sword shape
    }

    std::vector<unsigned int> ItemGenerator::GenerateWeaponIndices() {
        return GenerateSwordIndices();
    }

    std::vector<float> ItemGenerator::GenerateArmorVertices(const GeneratedItem& item) {
        // Simple armor piece shape
        return {
            -0.3f, -0.5f, -0.1f,
            0.3f, -0.5f, -0.1f,
            0.3f, 0.5f, -0.1f,
            -0.3f, 0.5f, -0.1f,
            -0.3f, -0.5f, 0.1f,
            0.3f, -0.5f, 0.1f,
            0.3f, 0.5f, 0.1f,
            -0.3f, 0.5f, 0.1f
        };
    }

    std::vector<unsigned int> ItemGenerator::GenerateArmorIndices() {
        return {
            0, 1, 2, 0, 2, 3, // Front face
            4, 5, 6, 4, 6, 7, // Back face
            0, 1, 5, 0, 5, 4, // Bottom face
            3, 2, 6, 3, 6, 7, // Top face
            0, 3, 7, 0, 7, 4, // Left face
            1, 2, 6, 1, 6, 5  // Right face
        };
    }

    std::vector<float> ItemGenerator::GenerateConsumableVertices(const GeneratedItem& item) {
        // Simple cube shape for consumables
        return {
            -0.2f, -0.2f, -0.2f,
            0.2f, -0.2f, -0.2f,
            0.2f, 0.2f, -0.2f,
            -0.2f, 0.2f, -0.2f,
            -0.2f, -0.2f, 0.2f,
            0.2f, -0.2f, 0.2f,
            0.2f, 0.2f, 0.2f,
            -0.2f, 0.2f, 0.2f
        };
    }

    std::vector<unsigned int> ItemGenerator::GenerateConsumableIndices() {
        return {
            0, 1, 2, 0, 2, 3, // Front face
            4, 5, 6, 4, 6, 7, // Back face
            0, 1, 5, 0, 5, 4, // Bottom face
            3, 2, 6, 3, 6, 7, // Top face
            0, 3, 7, 0, 7, 4, // Left face
            1, 2, 6, 1, 6, 5  // Right face
        };
    }

    std::vector<float> ItemGenerator::GenerateSpecialVertices(const GeneratedItem& item) {
        // Special item shape with more vertices
        return {
            -0.3f, -0.3f, -0.3f,
            0.3f, -0.3f, -0.3f,
            0.3f, 0.3f, -0.3f,
            -0.3f, 0.3f, -0.3f,
            -0.3f, -0.3f, 0.3f,
            0.3f, -0.3f, 0.3f,
            0.3f, 0.3f, 0.3f,
            -0.3f, 0.3f, 0.3f,
            0.0f, 0.4f, 0.0f,   // Top point
            0.0f, -0.4f, 0.0f   // Bottom point
        };
    }

    std::vector<unsigned int> ItemGenerator::GenerateSpecialIndices() {
        return {
            0, 1, 2, 0, 2, 3, // Front face
            4, 5, 6, 4, 6, 7, // Back face
            0, 1, 5, 0, 5, 4, // Bottom face
            3, 2, 6, 3, 6, 7, // Top face
            0, 3, 7, 0, 7, 4, // Left face
            1, 2, 6, 1, 6, 5, // Right face
            8, 3, 2, 8, 2, 6, 8, 6, 7, 8, 7, 3, // Top pyramid
            9, 0, 1, 9, 1, 5, 9, 5, 4, 9, 4, 0  // Bottom pyramid
        };
    }

    // Material color setting methods
    void ItemGenerator::SetMaterialColors(ToolModel* model, MaterialType materialType) {
        switch (materialType) {
            case MaterialType::WOOD:
                model->handleColor = Vec3(0.6f, 0.4f, 0.2f);
                model->headColor = Vec3(0.4f, 0.3f, 0.1f);
                model->accentColor = Vec3(0.5f, 0.35f, 0.15f);
                break;
            case MaterialType::STONE:
                model->handleColor = Vec3(0.5f, 0.5f, 0.5f);
                model->headColor = Vec3(0.4f, 0.4f, 0.4f);
                model->accentColor = Vec3(0.3f, 0.3f, 0.3f);
                break;
            case MaterialType::IRON:
                model->handleColor = Vec3(0.8f, 0.8f, 0.8f);
                model->headColor = Vec3(0.7f, 0.7f, 0.7f);
                model->accentColor = Vec3(0.6f, 0.6f, 0.6f);
                break;
            case MaterialType::DIAMOND:
                model->handleColor = Vec3(0.3f, 0.7f, 0.9f);
                model->headColor = Vec3(0.2f, 0.6f, 0.8f);
                model->accentColor = Vec3(0.1f, 0.5f, 0.7f);
                break;
            case MaterialType::NETHERITE:
                model->handleColor = Vec3(0.2f, 0.2f, 0.2f);
                model->headColor = Vec3(0.3f, 0.3f, 0.3f);
                model->accentColor = Vec3(0.4f, 0.4f, 0.4f);
                break;
            default:
                model->handleColor = Vec3(0.5f, 0.5f, 0.5f);
                model->headColor = Vec3(0.4f, 0.4f, 0.4f);
                model->accentColor = Vec3(0.3f, 0.3f, 0.3f);
                break;
        }
    }

    void ItemGenerator::SetWeaponMaterialColors(WeaponModel* model, MaterialType materialType) {
        // Similar to tool colors but for weapons
        SetMaterialColors(reinterpret_cast<ToolModel*>(model), materialType);
    }

    void ItemGenerator::SetArmorMaterialColors(ArmorModel* model, MaterialType materialType) {
        switch (materialType) {
            case MaterialType::LEATHER:
                model->primaryColor = Vec3(0.6f, 0.4f, 0.2f);
                model->secondaryColor = Vec3(0.7f, 0.5f, 0.3f);
                model->trimColor = Vec3(0.5f, 0.3f, 0.1f);
                break;
            case MaterialType::IRON:
                model->primaryColor = Vec3(0.7f, 0.7f, 0.7f);
                model->secondaryColor = Vec3(0.8f, 0.8f, 0.8f);
                model->trimColor = Vec3(0.6f, 0.6f, 0.6f);
                break;
            case MaterialType::DIAMOND:
                model->primaryColor = Vec3(0.3f, 0.7f, 0.9f);
                model->secondaryColor = Vec3(0.4f, 0.8f, 1.0f);
                model->trimColor = Vec3(0.2f, 0.6f, 0.8f);
                break;
            case MaterialType::NETHERITE:
                model->primaryColor = Vec3(0.2f, 0.2f, 0.2f);
                model->secondaryColor = Vec3(0.3f, 0.3f, 0.3f);
                model->trimColor = Vec3(0.4f, 0.4f, 0.4f);
                break;
            default:
                model->primaryColor = Vec3(0.5f, 0.5f, 0.5f);
                model->secondaryColor = Vec3(0.6f, 0.6f, 0.6f);
                model->trimColor = Vec3(0.4f, 0.4f, 0.4f);
                break;
        }
    }

    void ItemGenerator::SetConsumableColors(ConsumableModel* model, const GeneratedItem& item) {
        if (item.name.find("potion") != std::string::npos) {
            model->mainColor = Vec3(0.3f, 0.3f, 0.3f);
            model->accentColor = Vec3(0.4f, 0.4f, 0.4f);
            model->liquidColor = Vec3(0.2f, 0.5f, 0.8f);
            model->containerType = 1; // Bottle
        } else if (item.name.find("apple") != std::string::npos) {
            model->mainColor = Vec3(0.8f, 0.2f, 0.1f);
            model->accentColor = Vec3(0.6f, 0.1f, 0.1f);
            model->containerType = 0; // No container
        } else {
            model->mainColor = Vec3(0.6f, 0.6f, 0.6f);
            model->accentColor = Vec3(0.5f, 0.5f, 0.5f);
            model->containerType = 0;
        }
    }

    void ItemGenerator::SetSpecialColors(SpecialModel* model, const GeneratedItem& item) {
        model->primaryColor = Vec3(0.8f, 0.8f, 0.8f);
        model->secondaryColor = Vec3(0.6f, 0.6f, 0.6f);
        model->effectColor = Vec3(1.0f, 1.0f, 0.5f);

        if (item.rarity >= ItemRarity::RARE) {
            model->effectColor = Vec3(1.0f, 0.8f, 0.2f); // Gold
        }
        if (item.rarity >= ItemRarity::EPIC) {
            model->effectColor = Vec3(0.8f, 0.2f, 1.0f); // Purple
        }
        if (item.rarity >= ItemRarity::LEGENDARY) {
            model->effectColor = Vec3(1.0f, 0.5f, 0.1f); // Orange
        }
    }

    // Texture generation methods (simplified implementations)
    void ItemGenerator::GenerateToolTextureData(ItemTexture* texture, const GeneratedItem& item) {
        // Simple procedural texture generation
        for (int y = 0; y < texture->height; ++y) {
            for (int x = 0; x < texture->width; ++x) {
                int index = (y * texture->width + x) * 4;
                float noise = static_cast<float>((x + y + item.appearance.variant) % 256) / 255.0f;

                texture->data[index] = static_cast<unsigned char>(texture->primaryColor.x * 255 * noise);
                texture->data[index + 1] = static_cast<unsigned char>(texture->primaryColor.y * 255 * noise);
                texture->data[index + 2] = static_cast<unsigned char>(texture->primaryColor.z * 255 * noise);
                texture->data[index + 3] = 255; // Alpha
            }
        }
    }

    void ItemGenerator::GenerateWeaponTextureData(ItemTexture* texture, const GeneratedItem& item) {
        GenerateToolTextureData(texture, item);
    }

    void ItemGenerator::GenerateArmorTextureData(ItemTexture* texture, const GeneratedItem& item) {
        GenerateToolTextureData(texture, item);
    }

    void ItemGenerator::GenerateConsumableTextureData(ItemTexture* texture, const GeneratedItem& item) {
        GenerateToolTextureData(texture, item);
    }

    void ItemGenerator::GenerateSpecialTextureData(ItemTexture* texture, const GeneratedItem& item) {
        GenerateToolTextureData(texture, item);
        texture->hasGlow = item.appearance.hasGlow;
        texture->glowIntensity = 0.5f + static_cast<float>(item.rarity) / 10.0f;
    }

    // Entity creation
    std::shared_ptr<Entity> ItemGenerator::CreateItemEntity(const GeneratedItem& item, const Vec3& position, EntityManager* entityManager) {
        auto entity = entityManager->CreateEntity(item.name + "_dropped");

        // Add transform component
        auto transform = entity->AddComponent<TransformComponent>();
        transform->SetPosition(position);
        transform->SetScale(Vec3(item.appearance.scale, item.appearance.scale, item.appearance.scale));

        // Add physics component for dropped item behavior
        auto physics = entity->AddComponent<PhysicsComponent>();
        physics->SetMass(0.1f); // Light weight for items
        physics->SetVelocity(Vec3(0, 0, 0));

        // Add render component if model data exists
        if (item.modelData != nullptr) {
            auto render = entity->AddComponent<RenderComponent>();
            render->SetRenderType(RenderType::STATIC_MESH);
            // Set custom model and texture data
        }

        return entity;
    }

} // namespace VoxelCraft
