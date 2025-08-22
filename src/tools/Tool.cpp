/**
 * @file Tool.cpp
 * @brief VoxelCraft Tool System - Tool Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#include "Tool.hpp"
#include <algorithm>

namespace VoxelCraft {

    Tool::Tool(ToolType type, ToolMaterial material)
        : m_type(type)
        , m_material(material)
    {
        InitializeStats();
    }

    Tool::~Tool() {
        // Cleanup if needed
    }

    std::string Tool::GetName() const {
        return GetMaterialName() + " " + GetBaseName();
    }

    bool Tool::UseTool(int amount) {
        if (m_stats.currentDurability > 0) {
            m_stats.currentDurability = std::max(0, m_stats.currentDurability - amount);
            return m_stats.currentDurability == 0; // Return true if tool broke
        }
        return false;
    }

    void Tool::RepairTool(int amount) {
        m_stats.currentDurability = std::min(m_stats.maxDurability,
                                           m_stats.currentDurability + amount);
    }

    bool Tool::IsEffectiveAgainst(int materialType) const {
        // Basic effectiveness check - can be expanded based on tool type
        switch (m_type) {
            case ToolType::WOODEN_PICKAXE:
            case ToolType::STONE_PICKAXE:
            case ToolType::IRON_PICKAXE:
            case ToolType::DIAMOND_PICKAXE:
                // Pickaxes are effective against stone-based materials
                return materialType == 1; // Stone material type
            case ToolType::WOODEN_AXE:
            case ToolType::STONE_AXE:
            case ToolType::IRON_AXE:
            case ToolType::DIAMOND_AXE:
                // Axes are effective against wood-based materials
                return materialType == 2; // Wood material type
            case ToolType::WOODEN_SHOVEL:
            case ToolType::STONE_SHOVEL:
            case ToolType::IRON_SHOVEL:
            case ToolType::DIAMOND_SHOVEL:
                // Shovels are effective against dirt-based materials
                return materialType == 3; // Dirt material type
            default:
                return false;
        }
    }

    float Tool::GetMiningSpeedFor(int materialType) const {
        if (IsEffectiveAgainst(materialType)) {
            return m_stats.miningSpeed * 2.0f; // Double speed for effective tools
        }
        return m_stats.miningSpeed;
    }

    void Tool::AddEnchantment(ToolEnchantment enchantment, int level) {
        m_enchantments[enchantment] = level;
        m_stats.isEnchanted = !m_enchantments.empty();
    }

    void Tool::RemoveEnchantment(ToolEnchantment enchantment) {
        m_enchantments.erase(enchantment);
        m_stats.isEnchanted = !m_enchantments.empty();
    }

    bool Tool::HasEnchantment(ToolEnchantment enchantment) const {
        return m_enchantments.find(enchantment) != m_enchantments.end();
    }

    int Tool::GetEnchantmentLevel(ToolEnchantment enchantment) const {
        auto it = m_enchantments.find(enchantment);
        return (it != m_enchantments.end()) ? it->second : 0;
    }

    void Tool::InitializeStats() {
        // Set base stats based on material
        switch (m_material) {
            case ToolMaterial::WOOD:
                m_stats.miningSpeed = 2.0f;
                m_stats.attackDamage = 1.0f;
                m_stats.maxDurability = 59;
                break;
            case ToolMaterial::STONE:
                m_stats.miningSpeed = 4.0f;
                m_stats.attackDamage = 2.0f;
                m_stats.maxDurability = 131;
                break;
            case ToolMaterial::IRON:
                m_stats.miningSpeed = 6.0f;
                m_stats.attackDamage = 3.0f;
                m_stats.maxDurability = 250;
                break;
            case ToolMaterial::DIAMOND:
                m_stats.miningSpeed = 8.0f;
                m_stats.attackDamage = 4.0f;
                m_stats.maxDurability = 1561;
                break;
            case ToolMaterial::GOLD:
                m_stats.miningSpeed = 12.0f; // Gold is fast but weak
                m_stats.attackDamage = 1.0f;
                m_stats.maxDurability = 32;
                break;
            case ToolMaterial::NETHERITE:
                m_stats.miningSpeed = 9.0f;
                m_stats.attackDamage = 5.0f;
                m_stats.maxDurability = 2031;
                break;
        }

        m_stats.currentDurability = m_stats.maxDurability;

        // Adjust stats based on tool type
        switch (m_type) {
            case ToolType::WOODEN_SWORD:
            case ToolType::STONE_SWORD:
            case ToolType::IRON_SWORD:
            case ToolType::DIAMOND_SWORD:
                m_stats.attackSpeed = 1.6f;
                m_stats.miningSpeed = 1.5f; // Swords are not great for mining
                break;
            case ToolType::WOODEN_AXE:
            case ToolType::STONE_AXE:
            case ToolType::IRON_AXE:
            case ToolType::DIAMOND_AXE:
                m_stats.attackDamage += 2.0f; // Axes do more damage
                m_stats.attackSpeed = 0.8f;
                break;
            case ToolType::WOODEN_SHOVEL:
            case ToolType::STONE_SHOVEL:
            case ToolType::IRON_SHOVEL:
            case ToolType::DIAMOND_SHOVEL:
                m_stats.miningSpeed *= 0.5f; // Shovels are slower
                break;
            default:
                break;
        }
    }

    std::string Tool::GetBaseName() const {
        switch (m_type) {
            case ToolType::WOODEN_SWORD:
            case ToolType::STONE_SWORD:
            case ToolType::IRON_SWORD:
            case ToolType::DIAMOND_SWORD:
                return "Sword";
            case ToolType::WOODEN_AXE:
            case ToolType::STONE_AXE:
            case ToolType::IRON_AXE:
            case ToolType::DIAMOND_AXE:
                return "Axe";
            case ToolType::WOODEN_PICKAXE:
            case ToolType::STONE_PICKAXE:
            case ToolType::IRON_PICKAXE:
            case ToolType::DIAMOND_PICKAXE:
                return "Pickaxe";
            case ToolType::WOODEN_SHOVEL:
            case ToolType::STONE_SHOVEL:
            case ToolType::IRON_SHOVEL:
            case ToolType::DIAMOND_SHOVEL:
                return "Shovel";
            case ToolType::WOODEN_HOE:
            case ToolType::STONE_HOE:
            case ToolType::IRON_HOE:
            case ToolType::DIAMOND_HOE:
                return "Hoe";
            case ToolType::SHEARS:
                return "Shears";
            case ToolType::FISHING_ROD:
                return "Fishing Rod";
            case ToolType::FLINT_AND_STEEL:
                return "Flint and Steel";
            default:
                return "Tool";
        }
    }

    std::string Tool::GetMaterialName() const {
        switch (m_material) {
            case ToolMaterial::WOOD:
                return "Wooden";
            case ToolMaterial::STONE:
                return "Stone";
            case ToolMaterial::IRON:
                return "Iron";
            case ToolMaterial::DIAMOND:
                return "Diamond";
            case ToolMaterial::GOLD:
                return "Golden";
            case ToolMaterial::NETHERITE:
                return "Netherite";
            default:
                return "Unknown";
        }
    }

} // namespace VoxelCraft
