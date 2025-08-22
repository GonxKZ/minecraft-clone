/**
 * @file ItemGenerator.hpp
 * @brief VoxelCraft Procedural Item and Tool Generator
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_TOOLS_ITEM_GENERATOR_HPP
#define VOXELCRAFT_TOOLS_ITEM_GENERATOR_HPP

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <random>
#include <functional>
#include "ProceduralGenerator.hpp"

namespace VoxelCraft {

    // Forward declarations
    class EntityManager;
    class Entity;
    struct Vec3;

    /**
     * @enum ItemRarity
     * @brief Rarity levels for items
     */
    enum class ItemRarity {
        COMMON = 0,      ///< Basic items
        UNCOMMON,        ///< Slightly better items
        RARE,            ///< Rare and valuable items
        EPIC,            ///< Very rare items
        LEGENDARY,       ///< Extremely rare items
        MYTHIC,          ///< Unique legendary items
        CUSTOM           ///< Custom rarity
    };

    /**
     * @enum ToolType
     * @brief Types of tools that can be generated
     */
    enum class ToolType {
        SWORD = 0,
        PICKAXE,
        AXE,
        SHOVEL,
        HOE,
        BOW,
        CROSSBOW,
        STAFF,
        WAND,
        HAMMER,
        FISHING_ROD,
        SHEARS,
        FLINT_AND_STEEL,
        CUSTOM
    };

    /**
     * @enum ItemType
     * @brief Types of items that can be generated
     */
    enum class ItemType {
        TOOL = 0,        ///< Tools and weapons
        ARMOR,           ///< Armor pieces
        CONSUMABLE,      ///< Food and potions
        MATERIAL,        ///< Crafting materials
        BLOCK,           ///< Block items
        SPECIAL,         ///< Special items with unique effects
        CUSTOM           ///< Custom item types
    };

    /**
     * @enum MaterialType
     * @brief Material types for items
     */
    enum class MaterialType {
        WOOD = 0,        ///< Wooden items
        STONE,           ///< Stone items
        IRON,            ///< Iron items
        GOLD,            ///< Gold items
        DIAMOND,         ///< Diamond items
        NETHERITE,       ///< Netherite items
        CRYSTAL,         ///< Crystal/magic items
        BONE,            ///< Bone items
        LEATHER,         ///< Leather items
        CLOTH,           ///< Cloth items
        OBSIDIAN,        ///< Obsidian items
        EMERALD,         ///< Emerald items
        CUSTOM           ///< Custom materials
    };

    /**
     * @struct ItemStats
     * @brief Statistics and properties of an item
     */
    struct ItemStats {
        float durability;                 ///< Maximum durability
        float currentDurability;          ///< Current durability
        float attackDamage;               ///< Attack damage (for weapons)
        float attackSpeed;                ///< Attack speed (for weapons)
        float miningSpeed;                ///< Mining speed (for tools)
        float efficiency;                 ///< Tool efficiency multiplier
        float armorValue;                 ///< Armor protection value
        float knockbackResistance;        ///< Knockback resistance
        float movementSpeedBonus;         ///< Movement speed bonus
        float jumpHeightBonus;            ///< Jump height bonus
        int maxStackSize;                 ///< Maximum stack size
        int repairCost;                   ///< Cost to repair
        bool isStackable;                 ///< Can be stacked
        bool isEnchantable;               ///< Can be enchanted
        bool isRepairable;                ///< Can be repaired
        bool isConsumable;                ///< Can be consumed
        int foodValue;                    ///< Food points restored
        float saturationValue;            ///< Saturation restored
        std::vector<std::string> effects; ///< Special effects
    };

    /**
     * @struct ItemAppearance
     * @brief Visual appearance data for an item
     */
    struct ItemAppearance {
        std::string modelPath;            ///< 3D model path
        std::string texturePath;          ///< Texture path
        std::string iconPath;             ///< Icon path
        Vec3 primaryColor;                ///< Primary color
        Vec3 secondaryColor;              ///< Secondary color
        Vec3 accentColor;                 ///< Accent color
        float scale;                      ///< Size scale
        int variant;                      ///< Visual variant
        bool hasGlow;                     ///< Has glow effect
        bool hasParticles;                ///< Has particle effects
        std::vector<std::string> specialEffects; ///< Special visual effects
        std::string customModelData;      ///< Custom model data
    };

    /**
     * @struct GeneratedItem
     * @brief Complete item data generated procedurally
     */
    struct GeneratedItem {
        std::string id;                   ///< Unique item identifier
        std::string name;                 ///< Item name
        std::string displayName;          ///< Display name (with formatting)
        ItemType type;                    ///< Item type
        ToolType toolType;                ///< Tool type (if applicable)
        MaterialType materialType;        ///< Material type
        ItemRarity rarity;                ///< Item rarity
        ItemStats stats;                  ///< Item statistics
        ItemAppearance appearance;        ///< Visual appearance
        std::vector<std::string> lore;    ///< Item description/lore
        std::vector<std::string> enchantments; ///< Applied enchantments
        std::vector<std::string> tags;    ///< Item tags
        int count;                        ///< Stack count
        bool isUnbreakable;               ///< Cannot be broken
        bool isSoulbound;                 ///< Soulbound to player
        bool isCursed;                    ///< Has curse effects
        void* modelData;                  ///< Generated 3D model data
        void* textureData;                ///< Generated texture data
        void* soundData;                  ///< Generated sound data
    };

    /**
     * @class ItemGenerator
     * @brief Procedural item and tool generator
     */
    class ItemGenerator {
    public:
        /**
         * @brief Constructor
         */
        ItemGenerator();

        /**
         * @brief Destructor
         */
        ~ItemGenerator();

        /**
         * @brief Initialize the item generator
         * @param seed Random seed for generation
         */
        void Initialize(uint64_t seed);

        /**
         * @brief Generate a tool of specified type
         * @param toolType Type of tool to generate
         * @param materialType Material type
         * @param rarity Item rarity
         * @return Generated tool item
         */
        GeneratedItem GenerateTool(ToolType toolType, MaterialType materialType, ItemRarity rarity);

        /**
         * @brief Generate a random tool
         * @param minRarity Minimum rarity
         * @param maxRarity Maximum rarity
         * @return Generated random tool
         */
        GeneratedItem GenerateRandomTool(ItemRarity minRarity = ItemRarity::COMMON,
                                        ItemRarity maxRarity = ItemRarity::LEGENDARY);

        /**
         * @brief Generate a weapon
         * @param weaponType Type of weapon
         * @param materialType Material type
         * @param rarity Item rarity
         * @return Generated weapon item
         */
        GeneratedItem GenerateWeapon(ToolType weaponType, MaterialType materialType, ItemRarity rarity);

        /**
         * @brief Generate armor piece
         * @param armorSlot Armor slot (helmet, chestplate, etc.)
         * @param materialType Material type
         * @param rarity Item rarity
         * @return Generated armor item
         */
        GeneratedItem GenerateArmor(const std::string& armorSlot, MaterialType materialType, ItemRarity rarity);

        /**
         * @brief Generate consumable item
         * @param consumableType Type of consumable
         * @param rarity Item rarity
         * @return Generated consumable item
         */
        GeneratedItem GenerateConsumable(const std::string& consumableType, ItemRarity rarity);

        /**
         * @brief Generate material item
         * @param materialType Material type
         * @param rarity Item rarity
         * @return Generated material item
         */
        GeneratedItem GenerateMaterial(MaterialType materialType, ItemRarity rarity);

        /**
         * @brief Generate special item
         * @param specialType Type of special item
         * @param rarity Item rarity
         * @return Generated special item
         */
        GeneratedItem GenerateSpecialItem(const std::string& specialType, ItemRarity rarity);

        /**
         * @brief Generate random item
         * @param itemType Type of item to generate
         * @param minRarity Minimum rarity
         * @param maxRarity Maximum rarity
         * @return Generated random item
         */
        GeneratedItem GenerateRandomItem(ItemType itemType = ItemType::TOOL,
                                        ItemRarity minRarity = ItemRarity::COMMON,
                                        ItemRarity maxRarity = ItemRarity::LEGENDARY);

        /**
         * @brief Generate item model procedurally
         * @param item Generated item data
         * @return Generated 3D model data
         */
        void* GenerateItemModel(const GeneratedItem& item);

        /**
         * @brief Generate item texture procedurally
         * @param item Generated item data
         * @return Generated texture data
         */
        void* GenerateItemTexture(const GeneratedItem& item);

        /**
         * @brief Generate item sounds procedurally
         * @param item Generated item data
         * @return Generated sound data
         */
        void* GenerateItemSounds(const GeneratedItem& item);

        /**
         * @brief Apply random enchantments to item
         * @param item Item to enchant
         * @param maxEnchantments Maximum number of enchantments
         */
        void ApplyRandomEnchantments(GeneratedItem& item, int maxEnchantments = 3);

        /**
         * @brief Create entity from generated item (for dropped items)
         * @param item Generated item data
         * @param position Drop position
         * @param entityManager Entity manager to create in
         * @return Created entity
         */
        std::shared_ptr<Entity> CreateItemEntity(const GeneratedItem& item, const Vec3& position, EntityManager* entityManager);

        /**
         * @brief Get material properties
         * @param materialType Material type
         * @return Material properties
         */
        static ItemStats GetMaterialProperties(MaterialType materialType);

        /**
         * @brief Get tool properties
         * @param toolType Tool type
         * @return Tool properties
         */
        static ItemStats GetToolProperties(ToolType toolType);

    private:
        uint64_t m_seed;
        std::mt19937_64 m_randomEngine;

        // Material templates
        std::unordered_map<MaterialType, ItemStats> m_materialTemplates;
        std::unordered_map<ToolType, ItemStats> m_toolTemplates;

        // Generation functions
        std::unordered_map<std::string, std::function<GeneratedItem(MaterialType, ItemRarity)>> m_toolGenerators;
        std::unordered_map<std::string, std::function<GeneratedItem(MaterialType, ItemRarity)>> m_weaponGenerators;
        std::unordered_map<std::string, std::function<GeneratedItem(MaterialType, ItemRarity)>> m_armorGenerators;
        std::unordered_map<std::string, std::function<GeneratedItem(ItemRarity)>> m_consumableGenerators;
        std::unordered_map<std::string, std::function<GeneratedItem(ItemRarity)>> m_specialGenerators;

        // Model and texture generators
        std::unordered_map<std::string, std::function<void*(const GeneratedItem&)>> m_modelGenerators;
        std::unordered_map<std::string, std::function<void*(const GeneratedItem&)>> m_textureGenerators;
        std::unordered_map<std::string, std::function<void*(const GeneratedItem&)>> m_soundGenerators;

        /**
         * @brief Initialize material templates
         */
        void InitializeMaterialTemplates();

        /**
         * @brief Initialize tool templates
         */
        void InitializeToolTemplates();

        /**
         * @brief Initialize generation functions
         */
        void InitializeGenerators();

        /**
         * @brief Initialize model generators
         */
        void InitializeModelGenerators();

        /**
         * @brief Initialize texture generators
         */
        void InitializeTextureGenerators();

        /**
         * @brief Initialize sound generators
         */
        void InitializeSoundGenerators();

        /**
         * @brief Generate unique item name
         * @param baseName Base item name
         * @param rarity Item rarity
         * @param seed Generation seed
         * @return Unique name
         */
        std::string GenerateUniqueItemName(const std::string& baseName, ItemRarity rarity, uint64_t seed);

        /**
         * @brief Generate item lore/description
         * @param item Generated item
         * @return Lore lines
         */
        std::vector<std::string> GenerateItemLore(const GeneratedItem& item);

        /**
         * @brief Apply rarity modifiers to item
         * @param item Item to modify
         * @param rarity Item rarity
         */
        void ApplyRarityModifiers(GeneratedItem& item, ItemRarity rarity);

        /**
         * @brief Generate random enchantment
         * @param itemType Item type
         * @param level Enchantment level (1-10)
         * @return Enchantment string
         */
        std::string GenerateRandomEnchantment(ItemType itemType, int level);

        /**
         * @brief Check if enchantment is compatible with item
         * @param enchantment Enchantment name
         * @param itemType Item type
         * @return true if compatible
         */
        bool IsEnchantmentCompatible(const std::string& enchantment, ItemType itemType);

        // Tool generation methods
        GeneratedItem GenerateSword(MaterialType material, ItemRarity rarity);
        GeneratedItem GeneratePickaxe(MaterialType material, ItemRarity rarity);
        GeneratedItem GenerateAxe(MaterialType material, ItemRarity rarity);
        GeneratedItem GenerateShovel(MaterialType material, ItemRarity rarity);
        GeneratedItem GenerateHoe(MaterialType material, ItemRarity rarity);
        GeneratedItem GenerateBow(MaterialType material, ItemRarity rarity);
        GeneratedItem GenerateStaff(MaterialType material, ItemRarity rarity);
        GeneratedItem GenerateHammer(MaterialType material, ItemRarity rarity);

        // Armor generation methods
        GeneratedItem GenerateHelmet(MaterialType material, ItemRarity rarity);
        GeneratedItem GenerateChestplate(MaterialType material, ItemRarity rarity);
        GeneratedItem GenerateLeggings(MaterialType material, ItemRarity rarity);
        GeneratedItem GenerateBoots(MaterialType material, ItemRarity rarity);

        // Consumable generation methods
        GeneratedItem GenerateFood(ItemRarity rarity);
        GeneratedItem GeneratePotion(ItemRarity rarity);
        GeneratedItem GenerateScroll(ItemRarity rarity);

        // Special item generation methods
        GeneratedItem GenerateArtifact(ItemRarity rarity);
        GeneratedItem GenerateRelic(ItemRarity rarity);
        GeneratedItem GenerateTotem(ItemRarity rarity);

        // Model generation methods
        void* GenerateToolModel(const GeneratedItem& item);
        void* GenerateWeaponModel(const GeneratedItem& item);
        void* GenerateArmorModel(const GeneratedItem& item);
        void* GenerateConsumableModel(const GeneratedItem& item);
        void* GenerateSpecialModel(const GeneratedItem& item);

        // Texture generation methods
        void* GenerateToolTexture(const GeneratedItem& item);
        void* GenerateWeaponTexture(const GeneratedItem& item);
        void* GenerateArmorTexture(const GeneratedItem& item);
        void* GenerateConsumableTexture(const GeneratedItem& item);
        void* GenerateSpecialTexture(const GeneratedItem& item);

        // Sound generation methods
        void* GenerateToolSounds(const GeneratedItem& item);
        void* GenerateWeaponSounds(const GeneratedItem& item);
        void* GenerateArmorSounds(const GeneratedItem& item);
        void* GenerateConsumableSounds(const GeneratedItem& item);
        void* GenerateSpecialSounds(const GeneratedItem& item);
    };

    /**
     * @struct ToolModel
     * @brief 3D model data for tools
     */
    struct ToolModel {
        std::vector<float> vertices;      ///< Vertex data
        std::vector<float> normals;       ///< Normal data
        std::vector<float> texCoords;     ///< Texture coordinates
        std::vector<unsigned int> indices; ///< Index data
        Vec3 handleColor;                 ///< Handle color
        Vec3 headColor;                   ///< Head color
        Vec3 accentColor;                 ///< Accent color
        float scale;                      ///< Model scale
        int detailLevel;                  ///< Model detail level
    };

    /**
     * @struct WeaponModel
     * @brief 3D model data for weapons
     */
    struct WeaponModel {
        std::vector<float> vertices;      ///< Vertex data
        std::vector<float> normals;       ///< Normal data
        std::vector<float> texCoords;     ///< Texture coordinates
        std::vector<unsigned int> indices; ///< Index data
        Vec3 bladeColor;                  ///< Blade color
        Vec3 guardColor;                  ///< Guard color
        Vec3 handleColor;                 ///< Handle color
        Vec3 gemColor;                    ///< Gem color (if any)
        float scale;                      ///< Model scale
        int bladeType;                    ///< Blade design type
        int guardType;                    ///< Guard design type
    };

    /**
     * @struct ArmorModel
     * @brief 3D model data for armor
     */
    struct ArmorModel {
        std::vector<float> vertices;      ///< Vertex data
        std::vector<float> normals;       ///< Normal data
        std::vector<float> texCoords;     ///< Texture coordinates
        std::vector<unsigned int> indices; ///< Index data
        Vec3 primaryColor;                ///< Primary armor color
        Vec3 secondaryColor;              ///< Secondary armor color
        Vec3 trimColor;                   ///< Trim color
        Vec3 gemColor;                    ///< Gem color (if any)
        float scale;                      ///< Model scale
        int patternType;                  ///< Armor pattern type
        int trimType;                     ///< Trim design type
    };

    /**
     * @struct ConsumableModel
     * @brief 3D model data for consumables
     */
    struct ConsumableModel {
        std::vector<float> vertices;      ///< Vertex data
        std::vector<float> normals;       ///< Normal data
        std::vector<float> texCoords;     ///< Texture coordinates
        std::vector<unsigned int> indices; ///< Index data
        Vec3 mainColor;                   ///< Main color
        Vec3 accentColor;                 ///< Accent color
        Vec3 liquidColor;                 ///< Liquid color (for potions)
        float scale;                      ///< Model scale
        int shapeType;                    ///< Shape type
        int containerType;                ///< Container type (bottle, bowl, etc.)
    };

    /**
     * @struct SpecialModel
     * @brief 3D model data for special items
     */
    struct SpecialModel {
        std::vector<float> vertices;      ///< Vertex data
        std::vector<float> normals;       ///< Normal data
        std::vector<float> texCoords;     ///< Texture coordinates
        std::vector<unsigned int> indices; ///< Index data
        Vec3 primaryColor;                ///< Primary color
        Vec3 secondaryColor;              ///< Secondary color
        Vec3 effectColor;                 ///< Effect color
        float scale;                      ///< Model scale
        int modelType;                    ///< Model type
        int effectType;                   ///< Effect type
        bool hasGlow;                     ///< Has glow effect
        bool hasParticles;                ///< Has particle effects
    };

    /**
     * @struct ItemTexture
     * @brief Texture data for items
     */
    struct ItemTexture {
        int width;                        ///< Texture width
        int height;                       ///< Texture height
        std::vector<unsigned char> data;  ///< RGBA texture data
        Vec3 primaryColor;                ///< Primary color
        Vec3 secondaryColor;              ///< Secondary color
        Vec3 accentColor;                 ///< Accent color
        int patternType;                  ///< Pattern type
        int overlayType;                  ///< Overlay type
        bool hasGlow;                     ///< Has glow effect
        float glowIntensity;              ///< Glow intensity
    };

    /**
     * @struct ItemSounds
     * @brief Sound data for items
     */
    struct ItemSounds {
        void* useSound;                   ///< Use sound
        void* breakSound;                 ///< Break sound
        void* hitSound;                   ///< Hit sound
        void* specialSound;               ///< Special action sound
        float volume;                     ///< Base volume
        float pitch;                      ///< Base pitch
        int variant;                      ///< Sound variant
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_TOOLS_ITEM_GENERATOR_HPP
