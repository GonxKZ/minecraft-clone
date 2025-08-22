/**
 * @file Inventory.hpp
 * @brief VoxelCraft Player Inventory System
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines the Inventory class that manages player items, equipment,
 * crafting, and resource management with advanced features like stacking,
 * durability, and custom properties.
 */

#ifndef VOXELCRAFT_PLAYER_INVENTORY_HPP
#define VOXELCRAFT_PLAYER_INVENTORY_HPP

#include <memory>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <atomic>

#include "../core/Config.hpp"

namespace VoxelCraft {

    // Forward declarations
    class Player;
    class Item;

    /**
     * @enum InventorySlotType
     * @brief Types of inventory slots
     */
    enum class InventorySlotType {
        Regular,                ///< Regular inventory slot
        Hotbar,                 ///< Hotbar slot
        Equipment,              ///< Equipment slot (armor, tools)
        Crafting,               ///< Crafting grid slot
        Result,                 ///< Crafting result slot
        Offhand,                ///< Offhand slot
        Custom                  ///< Custom slot type
    };

    /**
     * @enum EquipmentSlot
     * @brief Equipment slot positions
     */
    enum class EquipmentSlot {
        Head,                   ///< Head slot (helmet)
        Chest,                  ///< Chest slot (chestplate)
        Legs,                   ///< Legs slot (leggings)
        Feet,                   ///< Feet slot (boots)
        MainHand,               ///< Main hand slot
        OffHand,                ///< Off hand slot
        Accessory1,             ///< Accessory slot 1
        Accessory2,             ///< Accessory slot 2
        Custom1,                ///< Custom equipment slot 1
        Custom2,                ///< Custom equipment slot 2
        Custom3,                ///< Custom equipment slot 3
        Custom4                 ///< Custom equipment slot 4
    };

    /**
     * @enum ItemRarity
     * @brief Item rarity levels
     */
    enum class ItemRarity {
        Common,                 ///< Common item
        Uncommon,               ///< Uncommon item
        Rare,                   ///< Rare item
        Epic,                   ///< Epic item
        Legendary,              ///< Legendary item
        Mythic,                 ///< Mythic item
        Unique                  ///< Unique item
    };

    /**
     * @enum ItemCategory
     * @brief Item categories
     */
    enum class ItemCategory {
        Tool,                   ///< Tool item
        Weapon,                 ///< Weapon item
        Armor,                  ///< Armor item
        Block,                  ///< Block item
        Material,               ///< Material item
        Food,                   ///< Food item
        Potion,                 ///< Potion item
        Book,                   ///< Book item
        Miscellaneous,          ///< Miscellaneous item
        Custom                  ///< Custom category
    };

    /**
     * @struct ItemProperties
     * @brief Properties of an item
     */
    struct ItemProperties {
        std::string id;                         ///< Unique item ID
        std::string name;                       ///< Item display name
        std::string description;                ///< Item description
        ItemCategory category;                  ///< Item category
        ItemRarity rarity;                      ///< Item rarity
        std::string iconPath;                   ///< Item icon texture path
        std::string modelPath;                  ///< Item 3D model path

        // Stack properties
        bool isStackable;                       ///< Can be stacked
        int maxStackSize;                       ///< Maximum stack size
        int currentStackSize;                   ///< Current stack size

        // Durability
        bool hasDurability;                     ///< Has durability
        int maxDurability;                      ///< Maximum durability
        int currentDurability;                  ///< Current durability

        // Physical properties
        float weight;                           ///< Item weight
        glm::vec3 dimensions;                   ///< Item dimensions

        // Usage properties
        bool isUsable;                          ///< Can be used
        bool isConsumable;                      ///< Can be consumed
        bool isEquippable;                      ///< Can be equipped
        float useCooldown;                      ///< Use cooldown (seconds)

        // Combat properties
        int attackDamage;                       ///< Attack damage
        float attackSpeed;                      ///< Attack speed multiplier
        float attackRange;                      ///< Attack range

        // Armor properties
        int armorValue;                         ///< Armor protection value
        int armorToughness;                     ///< Armor toughness

        // Tool properties
        float miningSpeed;                      ///< Mining speed multiplier
        int miningLevel;                        ///< Mining level required
        std::unordered_set<std::string> effectiveBlocks; ///< Effective block types

        // Food properties
        int hungerRestoration;                  ///< Hunger points restored
        float saturationRestoration;            ///< Saturation restored
        bool isEdible;                          ///< Can be eaten

        // Custom properties
        std::unordered_map<std::string, std::any> customProperties;
    };

    /**
     * @struct InventorySlot
     * @brief Inventory slot data
     */
    struct InventorySlot {
        int slotIndex;                          ///< Slot index
        InventorySlotType slotType;             ///< Slot type
        std::shared_ptr<Item> item;             ///< Item in slot
        int count;                              ///< Item count
        bool isLocked;                          ///< Slot is locked
        double lastUsedTime;                    ///< Last time slot was used
        std::unordered_map<std::string, std::any> customData; ///< Custom slot data
    };

    /**
     * @struct InventoryConfig
     * @brief Inventory system configuration
     */
    struct InventoryConfig {
        // Basic settings
        int mainInventorySize;                  ///< Main inventory size
        int hotbarSize;                         ///< Hotbar size (default 9)
        int equipmentSlotsCount;                ///< Number of equipment slots
        int craftingGridSize;                   ///< Crafting grid size (default 9)
        int maxStackSize;                       ///< Default maximum stack size

        // Advanced settings
        bool enableAutoStack;                   ///< Enable automatic stacking
        bool enableQuickMove;                   ///< Enable quick move (shift+click)
        bool enableItemDragDrop;                ///< Enable drag and drop
        bool enableItemFavorites;               ///< Enable item favorites
        bool enableItemSearch;                  ///< Enable item search

        // Weight and capacity
        bool enableWeightSystem;                ///< Enable weight system
        float maxCarryWeight;                   ///< Maximum carry weight
        int maxItemTypes;                       ///< Maximum different item types

        // Durability settings
        bool enableDurability;                  ///< Enable item durability
        bool showDurabilityBar;                 ///< Show durability bar
        bool enableRepairSystem;                ///< Enable item repair

        // Visual settings
        bool enableItemAnimations;              ///< Enable item animations
        bool enableStackAnimations;             ///< Enable stack number animations
        bool enableRarityColors;                ///< Enable rarity-based colors
        float iconSize;                         ///< Item icon size

        // Interaction settings
        float pickupRange;                      ///< Item pickup range
        bool enableAutoPickup;                  ///< Enable automatic item pickup
        bool enableItemThrowing;                ///< Enable item throwing
        float throwForce;                       ///< Item throw force

        // Network settings
        bool enableItemSync;                    ///< Enable item synchronization
        float syncInterval;                     ///< Item sync interval (seconds)
        bool enableItemValidation;              ///< Enable item validation

        // Custom settings
        std::unordered_map<std::string, std::any> customSettings;
    };

    /**
     * @struct InventoryMetrics
     * @brief Performance and usage metrics for inventory system
     */
    struct InventoryMetrics {
        // Performance metrics
        uint64_t updateCount;                   ///< Number of updates performed
        double totalUpdateTime;                 ///< Total update time (ms)
        double averageUpdateTime;               ///< Average update time (ms)
        double maxUpdateTime;                   ///< Maximum update time (ms)

        // Item metrics
        uint64_t itemsAdded;                    ///< Total items added
        uint64_t itemsRemoved;                  ///< Total items removed
        uint64_t itemsUsed;                     ///< Total items used
        uint64_t itemsCrafted;                  ///< Total items crafted
        uint64_t itemsRepaired;                 ///< Total items repaired

        // Slot metrics
        uint32_t occupiedSlots;                 ///< Currently occupied slots
        uint32_t totalSlots;                    ///< Total inventory slots
        uint64_t slotInteractions;              ///< Total slot interactions
        uint64_t hotbarSwitches;                ///< Hotbar slot switches

        // Stack metrics
        uint64_t stacksCreated;                 ///< Stacks created
        uint64_t stacksMerged;                  ///< Stacks merged
        uint64_t stacksSplit;                   ///< Stacks split
        uint32_t maxStackSize;                  ///< Maximum stack size reached

        // Durability metrics
        uint64_t itemsBroken;                   ///< Items broken due to durability
        uint64_t durabilityRepaired;            ///< Total durability repaired
        float averageDurability;                ///< Average item durability

        // Weight metrics
        float currentWeight;                    ///< Current inventory weight
        float maxWeight;                        ///< Maximum inventory weight
        uint64_t weightExceededEvents;          ///< Weight exceeded events

        // Error metrics
        uint64_t addItemErrors;                 ///< Item addition errors
        uint64_t removeItemErrors;              ///< Item removal errors
        uint64_t slotErrors;                    ///< Slot-related errors
        uint64_t validationErrors;              ///< Item validation errors
    };

    /**
     * @struct CraftingRecipe
     * @brief Crafting recipe definition
     */
    struct CraftingRecipe {
        std::string id;                         ///< Recipe unique ID
        std::string name;                       ///< Recipe display name
        std::vector<std::pair<std::string, int>> ingredients; ///< Required ingredients
        std::pair<std::string, int> result;     ///< Result item and count
        int craftingTime;                       ///< Crafting time (seconds)
        int requiredLevel;                      ///< Required crafting level
        std::string category;                   ///< Recipe category
        bool isShaped;                          ///< Shaped recipe (position matters)
        std::vector<std::vector<std::string>> pattern; ///< Crafting pattern
        std::unordered_map<std::string, std::any> customData; ///< Custom recipe data
    };

    /**
     * @class Item
     * @brief Game item with properties and behaviors
     */
    class Item {
    public:
        /**
         * @brief Constructor
         * @param properties Item properties
         */
        explicit Item(const ItemProperties& properties);

        /**
         * @brief Destructor
         */
        virtual ~Item() = default;

        /**
         * @brief Get item properties
         * @return Item properties
         */
        const ItemProperties& GetProperties() const { return m_properties; }

        /**
         * @brief Set item properties
         * @param properties New properties
         */
        void SetProperties(const ItemProperties& properties);

        /**
         * @brief Get item ID
         * @return Item ID
         */
        const std::string& GetId() const { return m_properties.id; }

        /**
         * @brief Get item name
         * @return Item name
         */
        const std::string& GetName() const { return m_properties.name; }

        /**
         * @brief Get item category
         * @return Item category
         */
        ItemCategory GetCategory() const { return m_properties.category; }

        /**
         * @brief Get item rarity
         * @return Item rarity
         */
        ItemRarity GetRarity() const { return m_properties.rarity; }

        /**
         * @brief Get current stack size
         * @return Current stack size
         */
        int GetStackSize() const { return m_properties.currentStackSize; }

        /**
         * @brief Set stack size
         * @param size New stack size
         */
        void SetStackSize(int size);

        /**
         * @brief Get maximum stack size
         * @return Maximum stack size
         */
        int GetMaxStackSize() const { return m_properties.maxStackSize; }

        /**
         * @brief Check if item has durability
         * @return true if has durability, false otherwise
         */
        bool HasDurability() const { return m_properties.hasDurability; }

        /**
         * @brief Get current durability
         * @return Current durability
         */
        int GetDurability() const { return m_properties.currentDurability; }

        /**
         * @brief Set durability
         * @param durability New durability
         */
        void SetDurability(int durability);

        /**
         * @brief Get maximum durability
         * @return Maximum durability
         */
        int GetMaxDurability() const { return m_properties.maxDurability; }

        /**
         * @brief Repair item
         * @param amount Repair amount
         * @return true if repaired, false otherwise
         */
        bool Repair(int amount);

        /**
         * @brief Damage item
         * @param amount Damage amount
         * @return true if item broke, false otherwise
         */
        bool Damage(int amount);

        /**
         * @brief Check if item is broken
         * @return true if broken, false otherwise
         */
        bool IsBroken() const;

        /**
         * @brief Use item
         * @param player Player using the item
         * @return true if used successfully, false otherwise
         */
        virtual bool Use(Player* player);

        /**
         * @brief Check if item can be used
         * @param player Player to check for
         * @return true if can be used, false otherwise
         */
        virtual bool CanUse(Player* player) const;

        /**
         * @brief Get use cooldown
         * @return Cooldown time (seconds)
         */
        float GetUseCooldown() const { return m_properties.useCooldown; }

        /**
         * @brief Check if item is on cooldown
         * @return true if on cooldown, false otherwise
         */
        bool IsOnCooldown() const;

        /**
         * @brief Get remaining cooldown time
         * @return Remaining cooldown (seconds)
         */
        float GetRemainingCooldown() const;

        /**
         * @brief Get item weight
         * @return Item weight
         */
        float GetWeight() const { return m_properties.weight; }

        /**
         * @brief Get item value (for trading, etc.)
         * @return Item value
         */
        virtual int GetValue() const;

        /**
         * @brief Get item tooltip text
         * @return Tooltip text
         */
        virtual std::string GetTooltip() const;

        /**
         * @brief Get item description
         * @return Item description
         */
        virtual std::string GetDescription() const { return m_properties.description; }

        /**
         * @brief Clone item
         * @return Cloned item
         */
        virtual std::shared_ptr<Item> Clone() const;

        /**
         * @brief Check if item can stack with another item
         * @param other Other item
         * @return true if can stack, false otherwise
         */
        virtual bool CanStackWith(const Item* other) const;

        /**
         * @brief Get custom property
         * @tparam T Property type
         * @param key Property key
         * @param defaultValue Default value
         * @return Property value or default
         */
        template<typename T>
        T GetCustomProperty(const std::string& key, const T& defaultValue = T{}) const;

        /**
         * @brief Set custom property
         * @tparam T Property type
         * @param key Property key
         * @param value Property value
         */
        template<typename T>
        void SetCustomProperty(const std::string& key, const T& value);

        /**
         * @brief Check if has custom property
         * @param key Property key
         * @return true if exists, false otherwise
         */
        bool HasCustomProperty(const std::string& key) const;

    protected:
        ItemProperties m_properties;                    ///< Item properties
        double m_lastUseTime;                           ///< Last use timestamp
        mutable std::shared_mutex m_propertyMutex;     ///< Property synchronization
    };

    /**
     * @class Inventory
     * @brief Player inventory management system
     *
     * The Inventory class provides a comprehensive item management system
     * with support for multiple inventory types, equipment, crafting, and
     * advanced features like stacking, durability, and item organization.
     *
     * Key Features:
     * - Multiple inventory types (main, hotbar, equipment, crafting)
     * - Item stacking and organization
     * - Equipment management with stat bonuses
     * - Crafting system with recipes
     * - Item durability and repair system
     * - Weight and capacity management
     * - Drag and drop interface support
     * - Item search and filtering
     * - Network synchronization
     * - Save/load functionality
     */
    class Inventory {
    public:
        /**
         * @brief Constructor
         * @param config Inventory configuration
         */
        explicit Inventory(const InventoryConfig& config);

        /**
         * @brief Destructor
         */
        ~Inventory();

        /**
         * @brief Deleted copy constructor
         */
        Inventory(const Inventory&) = delete;

        /**
         * @brief Deleted copy assignment operator
         */
        Inventory& operator=(const Inventory&) = delete;

        // Inventory lifecycle

        /**
         * @brief Initialize inventory
         * @param player Parent player
         * @return true if successful, false otherwise
         */
        bool Initialize(Player* player);

        /**
         * @brief Shutdown inventory
         */
        void Shutdown();

        /**
         * @brief Update inventory
         * @param deltaTime Time elapsed since last update
         */
        void Update(double deltaTime);

        // Configuration and state

        /**
         * @brief Get inventory configuration
         * @return Current configuration
         */
        const InventoryConfig& GetConfig() const { return m_config; }

        /**
         * @brief Set inventory configuration
         * @param config New configuration
         */
        void SetConfig(const InventoryConfig& config);

        /**
         * @brief Get inventory metrics
         * @return Performance metrics
         */
        const InventoryMetrics& GetMetrics() const { return m_metrics; }

        // Main inventory operations

        /**
         * @brief Add item to inventory
         * @param item Item to add
         * @param count Item count
         * @return Number of items actually added
         */
        int AddItem(std::shared_ptr<Item> item, int count = 1);

        /**
         * @brief Add item to specific slot
         * @param item Item to add
         * @param slotIndex Slot index
         * @param count Item count
         * @return true if added, false otherwise
         */
        bool AddItemToSlot(std::shared_ptr<Item> item, int slotIndex, int count = 1);

        /**
         * @brief Remove item from inventory
         * @param itemId Item ID to remove
         * @param count Number of items to remove
         * @return Number of items actually removed
         */
        int RemoveItem(const std::string& itemId, int count = 1);

        /**
         * @brief Remove item from specific slot
         * @param slotIndex Slot index
         * @param count Number of items to remove
         * @return Number of items actually removed
         */
        int RemoveItemFromSlot(int slotIndex, int count = 1);

        /**
         * @brief Get item from slot
         * @param slotIndex Slot index
         * @return Item in slot or nullptr
         */
        std::shared_ptr<Item> GetItem(int slotIndex) const;

        /**
         * @brief Set item in slot
         * @param slotIndex Slot index
         * @param item Item to set
         * @param count Item count
         * @return true if set, false otherwise
         */
        bool SetItem(int slotIndex, std::shared_ptr<Item> item, int count = 1);

        /**
         * @brief Check if slot is empty
         * @param slotIndex Slot index
         * @return true if empty, false otherwise
         */
        bool IsSlotEmpty(int slotIndex) const;

        /**
         * @brief Get item count in slot
         * @param slotIndex Slot index
         * @return Item count
         */
        int GetItemCount(int slotIndex) const;

        /**
         * @brief Find item in inventory
         * @param itemId Item ID to find
         * @return Vector of slot indices containing the item
         */
        std::vector<int> FindItem(const std::string& itemId) const;

        /**
         * @brief Get total item count
         * @param itemId Item ID
         * @return Total count of item in inventory
         */
        int GetTotalItemCount(const std::string& itemId) const;

        // Hotbar operations

        /**
         * @brief Get hotbar size
         * @return Hotbar size
         */
        int GetHotbarSize() const { return m_config.hotbarSize; }

        /**
         * @brief Get current hotbar selection
         * @return Selected hotbar slot index
         */
        int GetHotbarSelection() const { return m_hotbarSelection; }

        /**
         * @brief Set hotbar selection
         * @param slotIndex Hotbar slot index
         * @return true if set, false otherwise
         */
        bool SetHotbarSelection(int slotIndex);

        /**
         * @brief Get currently selected item
         * @return Selected item or nullptr
         */
        std::shared_ptr<Item> GetSelectedItem() const;

        /**
         * @brief Select next hotbar item
         */
        void SelectNextItem();

        /**
         * @brief Select previous hotbar item
         */
        void SelectPreviousItem();

        /**
         * @brief Select item by index
         * @param index Hotbar index
         * @return true if selected, false otherwise
         */
        bool SelectItemByIndex(int index);

        // Equipment operations

        /**
         * @brief Equip item
         * @param item Item to equip
         * @param slot Equipment slot
         * @return true if equipped, false otherwise
         */
        bool EquipItem(std::shared_ptr<Item> item, EquipmentSlot slot);

        /**
         * @brief Unequip item from slot
         * @param slot Equipment slot
         * @return Unequipped item or nullptr
         */
        std::shared_ptr<Item> UnequipItem(EquipmentSlot slot);

        /**
         * @brief Get equipped item
         * @param slot Equipment slot
         * @return Equipped item or nullptr
         */
        std::shared_ptr<Item> GetEquippedItem(EquipmentSlot slot) const;

        /**
         * @brief Get equipment stats
         * @return Equipment stat bonuses
         */
        std::unordered_map<std::string, int> GetEquipmentStats() const;

        // Crafting operations

        /**
         * @brief Set crafting grid item
         * @param gridIndex Grid index
         * @param item Item to place
         * @param count Item count
         * @return true if placed, false otherwise
         */
        bool SetCraftingGridItem(int gridIndex, std::shared_ptr<Item> item, int count = 1);

        /**
         * @brief Get crafting grid item
         * @param gridIndex Grid index
         * @return Item in grid or nullptr
         */
        std::shared_ptr<Item> GetCraftingGridItem(int gridIndex) const;

        /**
         * @brief Clear crafting grid
         */
        void ClearCraftingGrid();

        /**
         * @brief Get crafting result
         * @return Crafting result item or nullptr
         */
        std::shared_ptr<Item> GetCraftingResult() const;

        /**
         * @brief Craft item from grid
         * @return true if crafted, false otherwise
         */
        bool CraftItem();

        /**
         * @brief Add crafting recipe
         * @param recipe Crafting recipe
         * @return true if added, false otherwise
         */
        bool AddRecipe(const CraftingRecipe& recipe);

        /**
         * @brief Remove crafting recipe
         * @param recipeId Recipe ID
         * @return true if removed, false otherwise
         */
        bool RemoveRecipe(const std::string& recipeId);

        /**
         * @brief Get crafting recipe
         * @param recipeId Recipe ID
         * @return Recipe or empty optional
         */
        std::optional<CraftingRecipe> GetRecipe(const std::string& recipeId) const;

        // Utility operations

        /**
         * @brief Swap items between slots
         * @param slotA First slot index
         * @param slotB Second slot index
         * @return true if swapped, false otherwise
         */
        bool SwapItems(int slotA, int slotB);

        /**
         * @brief Split item stack
         * @param slotIndex Slot index
         * @param splitCount Number of items to split
         * @return true if split, false otherwise
         */
        bool SplitItemStack(int slotIndex, int splitCount);

        /**
         * @brief Merge item stacks
         * @param fromSlot Source slot
         * @param toSlot Destination slot
         * @return true if merged, false otherwise
         */
        bool MergeItemStacks(int fromSlot, int toSlot);

        /**
         * @brief Get free slot count
         * @return Number of free slots
         */
        int GetFreeSlotCount() const;

        /**
         * @brief Get occupied slot count
         * @return Number of occupied slots
         */
        int GetOccupiedSlotCount() const;

        /**
         * @brief Get total inventory size
         * @return Total inventory size
         */
        int GetTotalSize() const;

        /**
         * @brief Check if inventory is full
         * @return true if full, false otherwise
         */
        bool IsFull() const;

        /**
         * @brief Get current weight
         * @return Current inventory weight
         */
        float GetCurrentWeight() const;

        /**
         * @brief Get maximum weight
         * @return Maximum inventory weight
         */
        float GetMaxWeight() const { return m_config.maxCarryWeight; }

        /**
         * @brief Check if can carry more weight
         * @param additionalWeight Additional weight to check
         * @return true if can carry, false otherwise
         */
        bool CanCarryWeight(float additionalWeight) const;

        // Save and load

        /**
         * @brief Save inventory to file
         * @param filename File path
         * @return true if saved, false otherwise
         */
        bool SaveToFile(const std::string& filename) const;

        /**
         * @brief Load inventory from file
         * @param filename File path
         * @return true if loaded, false otherwise
         */
        bool LoadFromFile(const std::string& filename);

        /**
         * @brief Serialize inventory to string
         * @return Serialized inventory data
         */
        std::string Serialize() const;

        /**
         * @brief Deserialize inventory from string
         * @param data Serialized inventory data
         * @return true if loaded, false otherwise
         */
        bool Deserialize(const std::string& data);

        // Debug and utilities

        /**
         * @brief Get debug information
         * @return Debug information string
         */
        std::string GetDebugInfo() const;

        /**
         * @brief Validate inventory integrity
         * @return true if valid, false otherwise
         */
        bool Validate() const;

        /**
         * @brief Clear all items
         */
        void ClearAll();

        /**
         * @brief Reset metrics
         */
        void ResetMetrics();

    private:
        /**
         * @brief Initialize inventory slots
         */
        void InitializeSlots();

        /**
         * @brief Update crafting result
         */
        void UpdateCraftingResult();

        /**
         * @brief Check if recipe can be crafted
         * @param recipe Crafting recipe
         * @return true if can craft, false otherwise
         */
        bool CanCraftRecipe(const CraftingRecipe& recipe) const;

        /**
         * @brief Consume crafting ingredients
         * @param recipe Crafting recipe
         * @return true if consumed, false otherwise
         */
        bool ConsumeIngredients(const CraftingRecipe& recipe);

        /**
         * @brief Find empty slot
         * @param slotType Slot type preference
         * @return Empty slot index or -1 if none found
         */
        int FindEmptySlot(InventorySlotType slotType = InventorySlotType::Regular) const;

        /**
         * @brief Find slot with item
         * @param itemId Item ID to find
         * @param slotType Slot type preference
         * @return Slot index or -1 if not found
         */
        int FindSlotWithItem(const std::string& itemId, InventorySlotType slotType = InventorySlotType::Regular) const;

        /**
         * @brief Get slot type for index
         * @param slotIndex Slot index
         * @return Slot type
         */
        InventorySlotType GetSlotType(int slotIndex) const;

        /**
         * @brief Convert equipment slot to inventory index
         * @param equipmentSlot Equipment slot
         * @return Inventory index
         */
        int EquipmentSlotToIndex(EquipmentSlot equipmentSlot) const;

        /**
         * @brief Update inventory metrics
         * @param deltaTime Time elapsed
         */
        void UpdateMetrics(double deltaTime);

        /**
         * @brief Handle inventory errors
         * @param error Error message
         */
        void HandleInventoryError(const std::string& error);

        // Inventory data
        InventoryConfig m_config;                      ///< Inventory configuration
        InventoryMetrics m_metrics;                    ///< Performance metrics

        // Inventory slots
        std::vector<InventorySlot> m_slots;            ///< All inventory slots
        std::vector<int> m_hotbarSlots;                ///< Hotbar slot indices
        std::vector<int> m_equipmentSlots;             ///< Equipment slot indices
        std::vector<int> m_craftingSlots;              ///< Crafting grid slot indices
        int m_resultSlot;                              ///< Crafting result slot index
        int m_offhandSlot;                             ///< Offhand slot index

        // Current state
        int m_hotbarSelection;                         ///< Current hotbar selection
        Player* m_player;                              ///< Parent player

        // Crafting system
        std::vector<std::shared_ptr<Item>> m_craftingGrid; ///< Crafting grid items
        std::shared_ptr<Item> m_craftingResult;        ///< Current crafting result
        std::unordered_map<std::string, CraftingRecipe> m_recipes; ///< Available recipes

        // Equipment system
        std::unordered_map<EquipmentSlot, std::shared_ptr<Item>> m_equippedItems;
        std::unordered_map<std::string, int> m_equipmentStats; ///< Equipment stat bonuses

        // Synchronization
        mutable std::shared_mutex m_inventoryMutex;    ///< Inventory synchronization

        // Initialization state
        bool m_isInitialized;                          ///< Inventory initialized
        double m_lastUpdateTime;                       ///< Last update timestamp
    };

    // Template implementations

    template<typename T>
    T Item::GetCustomProperty(const std::string& key, const T& defaultValue) const {
        std::shared_lock<std::shared_mutex> lock(m_propertyMutex);
        auto it = m_properties.customProperties.find(key);
        if (it != m_properties.customProperties.end()) {
            try {
                return std::any_cast<T>(it->second);
            } catch (const std::bad_any_cast&) {
                return defaultValue;
            }
        }
        return defaultValue;
    }

    template<typename T>
    void Item::SetCustomProperty(const std::string& key, const T& value) {
        std::unique_lock<std::shared_mutex> lock(m_propertyMutex);
        m_properties.customProperties[key] = value;
    }

} // namespace VoxelCraft

#endif // VOXELCRAFT_PLAYER_INVENTORY_HPP
