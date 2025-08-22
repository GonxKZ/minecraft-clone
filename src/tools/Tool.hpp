/**
 * @file Tool.hpp
 * @brief VoxelCraft Tool System - Tool Definitions
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_TOOLS_TOOL_HPP
#define VOXELCRAFT_TOOLS_TOOL_HPP

#include <string>
#include <unordered_map>

namespace VoxelCraft {

    /**
     * @enum ToolType
     * @brief Types of tools available in VoxelCraft
     */
    enum class ToolType {
        NONE = 0,
        WOODEN_SWORD,
        STONE_SWORD,
        IRON_SWORD,
        DIAMOND_SWORD,
        WOODEN_AXE,
        STONE_AXE,
        IRON_AXE,
        DIAMOND_AXE,
        WOODEN_PICKAXE,
        STONE_PICKAXE,
        IRON_PICKAXE,
        DIAMOND_PICKAXE,
        WOODEN_SHOVEL,
        STONE_SHOVEL,
        IRON_SHOVEL,
        DIAMOND_SHOVEL,
        WOODEN_HOE,
        STONE_HOE,
        IRON_HOE,
        DIAMOND_HOE,
        SHEARS,
        FISHING_ROD,
        FLINT_AND_STEEL
    };

    /**
     * @enum ToolMaterial
     * @brief Material types for tools
     */
    enum class ToolMaterial {
        WOOD = 0,
        STONE,
        IRON,
        DIAMOND,
        GOLD,
        NETHERITE
    };

    /**
     * @enum ToolEnchantment
     * @brief Available tool enchantments
     */
    enum class ToolEnchantment {
        EFFICIENCY = 0,
        SILK_TOUCH,
        FORTUNE,
        UNBREAKING,
        SHARPNESS,
        SMITE,
        BANE_OF_ARTHROPODS,
        LOOTING,
        FIRE_ASPECT,
        KNOCKBACK,
        MENDING,
        CURSE_OF_VANISHING
    };

    /**
     * @struct ToolStats
     * @brief Statistics and properties of a tool
     */
    struct ToolStats {
        float miningSpeed;           ///< Base mining speed multiplier
        float attackDamage;          ///< Base attack damage
        float attackSpeed;           ///< Attack speed multiplier
        int maxDurability;           ///< Maximum durability
        int currentDurability;       ///< Current durability
        ToolMaterial material;       ///< Tool material
        bool isEnchanted;            ///< Whether tool has enchantments

        ToolStats()
            : miningSpeed(1.0f)
            , attackDamage(1.0f)
            , attackSpeed(1.0f)
            , maxDurability(0)
            , currentDurability(0)
            , material(ToolMaterial::WOOD)
            , isEnchanted(false)
        {}
    };

    /**
     * @class Tool
     * @brief Represents a tool item in VoxelCraft
     */
    class Tool {
    public:
        /**
         * @brief Constructor
         * @param type Tool type
         * @param material Tool material
         */
        Tool(ToolType type = ToolType::NONE, ToolMaterial material = ToolMaterial::WOOD);

        /**
         * @brief Destructor
         */
        ~Tool();

        /**
         * @brief Get tool type
         * @return Tool type
         */
        ToolType GetType() const { return m_type; }

        /**
         * @brief Get tool material
         * @return Tool material
         */
        ToolMaterial GetMaterial() const { return m_material; }

        /**
         * @brief Get tool name
         * @return Tool name
         */
        std::string GetName() const;

        /**
         * @brief Get tool statistics
         * @return Tool statistics
         */
        const ToolStats& GetStats() const { return m_stats; }

        /**
         * @brief Check if tool is broken
         * @return true if tool is broken
         */
        bool IsBroken() const { return m_stats.currentDurability <= 0; }

        /**
         * @brief Use tool (reduce durability)
         * @param amount Amount to reduce durability by
         * @return true if tool broke
         */
        bool UseTool(int amount = 1);

        /**
         * @brief Repair tool
         * @param amount Amount to repair
         */
        void RepairTool(int amount);

        /**
         * @brief Check if tool is effective against material
         * @param materialType Material type to check
         * @return true if tool is effective
         */
        bool IsEffectiveAgainst(int materialType) const;

        /**
         * @brief Get mining speed multiplier for material
         * @param materialType Material type
         * @return Mining speed multiplier
         */
        float GetMiningSpeedFor(int materialType) const;

        /**
         * @brief Add enchantment to tool
         * @param enchantment Enchantment to add
         * @param level Enchantment level
         */
        void AddEnchantment(ToolEnchantment enchantment, int level);

        /**
         * @brief Remove enchantment from tool
         * @param enchantment Enchantment to remove
         */
        void RemoveEnchantment(ToolEnchantment enchantment);

        /**
         * @brief Check if tool has enchantment
         * @param enchantment Enchantment to check
         * @return true if tool has enchantment
         */
        bool HasEnchantment(ToolEnchantment enchantment) const;

        /**
         * @brief Get enchantment level
         * @param enchantment Enchantment to get level for
         * @return Enchantment level (0 if not enchanted)
         */
        int GetEnchantmentLevel(ToolEnchantment enchantment) const;

    private:
        ToolType m_type;
        ToolMaterial m_material;
        ToolStats m_stats;
        std::unordered_map<ToolEnchantment, int> m_enchantments;

        /**
         * @brief Initialize tool statistics based on type and material
         */
        void InitializeStats();

        /**
         * @brief Get base name for tool type
         * @return Base name
         */
        std::string GetBaseName() const;

        /**
         * @brief Get material name
         * @return Material name
         */
        std::string GetMaterialName() const;
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_TOOLS_TOOL_HPP
