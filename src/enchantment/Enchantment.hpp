/**
 * @file Enchantment.hpp
 * @brief VoxelCraft Enchantment System - Enchantments and Effects
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_ENCHANTMENT_ENCHANTMENT_HPP
#define VOXELCRAFT_ENCHANTMENT_ENCHANTMENT_HPP

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>
#include <array>
#include <chrono>

namespace VoxelCraft {

    class Entity;
    class Player;
    class Item;

    /**
     * @enum EnchantmentType
     * @brief Types of enchantments available
     */
    enum class EnchantmentType {
        // Weapon Enchantments
        SHARPNESS = 0,      ///< Increases melee damage
        SMITE,              ///< Extra damage to undead
        BANE_OF_ARTHROPODS, ///< Extra damage to arthropods
        KNOCKBACK,          ///< Knockback enemies
        FIRE_ASPECT,        ///< Sets targets on fire
        LOOTING,            ///< Increases mob drops
        SWEEPING_EDGE,      ///< Increases sweeping attack damage

        // Armor Enchantments
        PROTECTION,         ///< Reduces damage
        FIRE_PROTECTION,    ///< Reduces fire damage
        FEATHER_FALLING,    ///< Reduces fall damage
        BLAST_PROTECTION,   ///< Reduces explosion damage
        PROJECTILE_PROTECTION, ///< Reduces projectile damage
        RESPIRATION,        ///< Increases underwater breathing time
        AQUA_AFFINITY,      ///< Increases underwater mining speed
        THORNS,             ///< Damages attackers
        DEPTH_STRIDER,      ///< Increases underwater movement speed
        FROST_WALKER,       ///< Freezes water when walking
        SOUL_SPEED,         ///< Increases speed on soul sand/soul soil

        // Tool Enchantments
        EFFICIENCY,         ///< Increases mining speed
        SILK_TOUCH,         ///< Mines blocks with original form
        UNBREAKING,         ///< Increases item durability
        FORTUNE,            ///< Increases block drops
        LUCK_OF_THE_SEA,    ///< Increases fishing luck
        LURE,               ///< Decreases fishing time

        // Bow Enchantments
        POWER,              ///< Increases arrow damage
        PUNCH,              ///< Knockback with arrows
        FLAME,              ///< Sets targets on fire with arrows
        INFINITY,           ///< Infinite arrows

        // Trident Enchantments
        IMPALING,           ///< Extra damage to aquatic mobs
        RIPTIDE,            ///< Propels player with trident
        LOYALTY,            ///< Trident returns to player
        CHANNELING,         ///< Trident can summon lightning

        // Fishing Rod Enchantments
        LUCK_OF_THE_SEA,    ///< Already defined above
        LURE,               ///< Already defined above

        // Crossbow Enchantments
        MULTISHOT,          ///< Shoots multiple arrows
        PIERCING,           ///< Arrows pass through entities
        QUICK_CHARGE,       ///< Faster crossbow charging

        // Curses
        CURSE_OF_BINDING,   ///< Prevents item removal
        CURSE_OF_VANISHING, ///< Destroys item on death
        CURSE_OF_LOSS,      ///< Damages item when taking damage
    };

    /**
     * @enum EnchantmentRarity
     * @brief Rarity levels for enchantments
     */
    enum class EnchantmentRarity {
        COMMON = 0,         ///< Common enchantments
        UNCOMMON,           ///< Uncommon enchantments
        RARE,               ///< Rare enchantments
        VERY_RARE,          ///< Very rare enchantments
        TREASURE,           ///< Treasure enchantments (books only)
    };

    /**
     * @enum EnchantmentTarget
     * @brief Item types that can be enchanted
     */
    enum class EnchantmentTarget {
        ALL = 0,            ///< All items
        WEAPON,             ///< Weapons (swords, axes)
        ARMOR,              ///< Armor pieces
        TOOL,               ///< Tools (pickaxes, shovels, etc.)
        BOW,                ///< Bows
        CROSSBOW,           ///< Crossbows
        TRIDENT,            ///< Tridents
        FISHING_ROD,        ///< Fishing rods
        BREAKABLE,          ///< Breakable items
        WEARABLE,           ///< Wearable items
        HELMET,             ///< Helmets only
        CHESTPLATE,         ///< Chestplates only
        LEGGINGS,           ///< Leggings only
        BOOTS,              ///< Boots only
    };

    /**
     * @struct EnchantmentLevel
     * @brief Single enchantment level with its properties
     */
    struct EnchantmentLevel {
        int level;                          ///< Enchantment level (1-10 typically)
        int minXPLevel;                     ///< Minimum XP level required
        int maxXPLevel;                     ///< Maximum XP level required
        float baseCost;                     ///< Base enchantment cost
        float levelCost;                    ///< Cost per level
        std::unordered_map<std::string, float> properties; ///< Level-specific properties
    };

    /**
     * @struct EnchantmentDefinition
     * @brief Complete definition of an enchantment
     */
    struct EnchantmentDefinition {
        EnchantmentType type;
        std::string name;
        std::string displayName;
        std::string description;
        EnchantmentRarity rarity;
        EnchantmentTarget target;
        bool isCurse;                       ///< Whether this is a curse
        bool isTreasure;                    ///< Whether this is a treasure enchantment
        int maxLevel;                       ///< Maximum level for this enchantment
        std::vector<EnchantmentLevel> levels;
        std::vector<EnchantmentType> incompatibleEnchantments; ///< Enchantments that conflict
        std::function<void(Entity*, Entity*, int)> onHitEffect; ///< Effect when hitting entities
        std::function<void(Entity*, int)> onDamageEffect; ///< Effect when taking damage
        std::function<void(Entity*, int)> onTickEffect; ///< Effect every tick
        std::function<void(Entity*, Entity*, int)> onKillEffect; ///< Effect when killing entities
        std::function<void(Entity*, int)> onBreakEffect; ///< Effect when breaking blocks
        std::unordered_map<std::string, std::any> customProperties;
    };

    /**
     * @struct EnchantmentInstance
     * @brief Instance of an enchantment on an item
     */
    struct EnchantmentInstance {
        EnchantmentType type;
        int level;
        std::chrono::steady_clock::time_point appliedTime;
        Entity* appliedBy;                  ///< Entity that applied the enchantment
        std::unordered_map<std::string, float> modifiers;

        EnchantmentInstance() : appliedTime(std::chrono::steady_clock::now()) {}

        /**
         * @brief Get enchantment age in seconds
         * @return Age in seconds
         */
        float GetAge() const {
            auto now = std::chrono::steady_clock::now();
            return std::chrono::duration<float>(now - appliedTime).count();
        }
    };

    /**
     * @struct EnchantedItem
     * @brief Item with enchantments applied
     */
    struct EnchantedItem {
        int itemID;
        int itemData;
        std::vector<EnchantmentInstance> enchantments;
        int repairCost;                     ///< XP cost to repair/rename in anvil
        std::string customName;             ///< Custom item name

        /**
         * @brief Check if item has specific enchantment
         * @param type Enchantment type to check
         * @return true if has enchantment
         */
        bool HasEnchantment(EnchantmentType type) const {
            for (const auto& enchantment : enchantments) {
                if (enchantment.type == type) return true;
            }
            return false;
        }

        /**
         * @brief Get enchantment level
         * @param type Enchantment type
         * @return Level or 0 if not found
         */
        int GetEnchantmentLevel(EnchantmentType type) const {
            for (const auto& enchantment : enchantments) {
                if (enchantment.type == type) return enchantment.level;
            }
            return 0;
        }

        /**
         * @brief Get total enchantment power
         * @return Sum of all enchantment levels
         */
        int GetTotalEnchantmentPower() const {
            int total = 0;
            for (const auto& enchantment : enchantments) {
                total += enchantment.level;
            }
            return total;
        }
    };

    /**
     * @class Enchantment
     * @brief Base class for all enchantments
     */
    class Enchantment {
    public:
        /**
         * @brief Constructor
         * @param definition Enchantment definition
         */
        Enchantment(const EnchantmentDefinition& definition);

        /**
         * @brief Destructor
         */
        virtual ~Enchantment() = default;

        /**
         * @brief Get enchantment type
         * @return Enchantment type
         */
        EnchantmentType GetType() const { return m_definition.type; }

        /**
         * @brief Get enchantment name
         * @return Enchantment name
         */
        const std::string& GetName() const { return m_definition.name; }

        /**
         * @brief Get enchantment display name
         * @return Display name
         */
        const std::string& GetDisplayName() const { return m_definition.displayName; }

        /**
         * @brief Get enchantment definition
         * @return Enchantment definition
         */
        const EnchantmentDefinition& GetDefinition() const { return m_definition; }

        /**
         * @brief Check if enchantment can be applied to item
         * @param itemID Item ID
         * @param existingEnchantments Current enchantments on item
         * @return true if can be applied
         */
        virtual bool CanApplyToItem(int itemID, const std::vector<EnchantmentInstance>& existingEnchantments) const;

        /**
         * @brief Get minimum level for enchantment
         * @return Minimum level
         */
        int GetMinLevel() const { return 1; }

        /**
         * @brief Get maximum level for enchantment
         * @return Maximum level
         */
        int GetMaxLevel() const { return m_definition.maxLevel; }

        /**
         * @brief Get enchantment rarity
         * @return Enchantment rarity
         */
        EnchantmentRarity GetRarity() const { return m_definition.rarity; }

        /**
         * @brief Check if this is a curse
         * @return true if curse
         */
        bool IsCurse() const { return m_definition.isCurse; }

        /**
         * @brief Check if this is a treasure enchantment
         * @return true if treasure
         */
        bool IsTreasure() const { return m_definition.isTreasure; }

        /**
         * @brief Apply enchantment effect
         * @param target Target entity
         * @param source Source of the effect
         * @param level Enchantment level
         */
        virtual void ApplyEffect(Entity* target, Entity* source, int level) {}

        /**
         * @brief Remove enchantment effect
         * @param target Target entity
         * @param level Enchantment level
         */
        virtual void RemoveEffect(Entity* target, int level) {}

    protected:
        EnchantmentDefinition m_definition;
    };

    /**
     * @class DamageEnchantment
     * @brief Enchantment that deals extra damage
     */
    class DamageEnchantment : public Enchantment {
    public:
        DamageEnchantment(const EnchantmentDefinition& definition);

        void ApplyEffect(Entity* target, Entity* source, int level) override;
        bool CanApplyToItem(int itemID, const std::vector<EnchantmentInstance>& existingEnchantments) const override;

    private:
        float m_damageMultiplier;
    };

    /**
     * @class ProtectionEnchantment
     * @brief Enchantment that provides damage protection
     */
    class ProtectionEnchantment : public Enchantment {
    public:
        ProtectionEnchantment(const EnchantmentDefinition& definition);

        void ApplyEffect(Entity* target, Entity* source, int level) override;
        bool CanApplyToItem(int itemID, const std::vector<EnchantmentInstance>& existingEnchantments) const override;

    private:
        float m_protectionMultiplier;
    };

    /**
     * @class EfficiencyEnchantment
     * @brief Enchantment that increases mining speed
     */
    class EfficiencyEnchantment : public Enchantment {
    public:
        EfficiencyEnchantment(const EnchantmentDefinition& definition);

        void ApplyEffect(Entity* target, Entity* source, int level) override;
        bool CanApplyToItem(int itemID, const std::vector<EnchantmentInstance>& existingEnchantments) const override;

    private:
        float m_speedMultiplier;
    };

    /**
     * @class EnchantmentManager
     * @brief Manager for all enchantments
     */
    class EnchantmentManager {
    public:
        /**
         * @brief Get singleton instance
         * @return EnchantmentManager instance
         */
        static EnchantmentManager& GetInstance();

        /**
         * @brief Initialize enchantment manager
         * @return true if successful
         */
        bool Initialize();

        /**
         * @brief Shutdown enchantment manager
         */
        void Shutdown();

        /**
         * @brief Register enchantment
         * @param enchantment Enchantment to register
         */
        void RegisterEnchantment(std::shared_ptr<Enchantment> enchantment);

        /**
         * @brief Get enchantment by type
         * @param type Enchantment type
         * @return Enchantment instance or nullptr
         */
        std::shared_ptr<Enchantment> GetEnchantment(EnchantmentType type) const;

        /**
         * @brief Get all available enchantments
         * @return Vector of all enchantments
         */
        std::vector<std::shared_ptr<Enchantment>> GetAllEnchantments() const;

        /**
         * @brief Get compatible enchantments for item
         * @param itemID Item ID
         * @param existingEnchantments Current enchantments
         * @return Vector of compatible enchantments
         */
        std::vector<EnchantmentType> GetCompatibleEnchantments(int itemID,
                                                              const std::vector<EnchantmentInstance>& existingEnchantments) const;

        /**
         * @brief Calculate enchantment cost
         * @param enchantmentType Enchantment type
         * @param level Enchantment level
         * @param itemID Item ID
         * @return XP cost
         */
        int CalculateEnchantmentCost(EnchantmentType enchantmentType, int level, int itemID) const;

        /**
         * @brief Check if enchantments are compatible
         * @param type1 First enchantment type
         * @param type2 Second enchantment type
         * @return true if compatible
         */
        bool AreEnchantmentsCompatible(EnchantmentType type1, EnchantmentType type2) const;

        /**
         * @brief Get enchantment display color
         * @param rarity Enchantment rarity
         * @return Color code
         */
        static std::string GetEnchantmentColor(EnchantmentRarity rarity);

        /**
         * @brief Get enchantment stats
         * @return Enchantment statistics
         */
        const EnchantmentStats& GetStats() const { return m_stats; }

    private:
        EnchantmentManager() = default;
        ~EnchantmentManager() = default;

        // Prevent copying
        EnchantmentManager(const EnchantmentManager&) = delete;
        EnchantmentManager& operator=(const EnchantmentManager&) = delete;

        std::unordered_map<EnchantmentType, std::shared_ptr<Enchantment>> m_enchantments;
        EnchantmentStats m_stats;
        bool m_initialized = false;

        void RegisterDefaultEnchantments();
    };

    /**
     * @struct EnchantmentStats
     * @brief Statistics for enchantment system
     */
    struct EnchantmentStats {
        int totalEnchantments = 0;
        int itemsEnchanted = 0;
        int totalEnchantmentLevels = 0;
        int cursesApplied = 0;
        int treasureEnchantments = 0;
        std::unordered_map<EnchantmentType, int> enchantmentsApplied;
        std::unordered_map<EnchantmentRarity, int> rarityDistribution;
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_ENCHANTMENT_ENCHANTMENT_HPP
