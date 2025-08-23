#pragma once

#include <vector>
#include <unordered_map>
#include <memory>
#include <string>
#include <functional>
#include <mutex>
#include <atomic>

namespace VoxelCraft {

    class Item;
    class ItemStack;
    class PlayerEntity;

    /**
     * @enum InventoryType
     * @brief Types of inventories in the game
     */
    enum class InventoryType {
        PLAYER = 0,           ///< Player main inventory
        HOTBAR,               ///< Player hotbar (subset of player inventory)
        CRAFTING,             ///< Crafting table inventory
        FURNACE,              ///< Furnace inventory
        CHEST,                ///< Chest inventory
        ENDER_CHEST,          ///< Ender chest inventory
        SHULKER_BOX,          ///< Shulker box inventory
        BARREL,               ///< Barrel inventory
        HOPPER,               ///< Hopper inventory
        DISPENSER,            ///< Dispenser inventory
        DROPPER,              ///< Dropper inventory
        BREWING_STAND,        ///< Brewing stand inventory
        ENCHANTMENT_TABLE,    ///< Enchantment table inventory
        ANVIL,                ///< Anvil inventory
        BEACON,               ///< Beacon inventory
        HOPPER_MINECART,      ///< Hopper minecart inventory
        CHEST_MINECART,       ///< Chest minecart inventory
        VILLAGER_TRADING,     ///< Villager trading inventory
        HORSE_INVENTORY       ///< Horse inventory
    };

    /**
     * @struct ItemStack
     * @brief Represents a stack of items in the inventory
     */
    struct ItemStack {
        int itemID;              ///< Item/block ID
        int count;               ///< Number of items in stack
        int maxStackSize;        ///< Maximum stack size for this item
        int metadata;            ///< Item metadata (durability, enchantments, etc.)
        std::string itemName;    ///< Item display name

        ItemStack(int id = 0, int cnt = 0, int maxStack = 64,
                 int meta = 0, const std::string& name = "");

        // Stack operations
        bool CanStackWith(const ItemStack& other) const;
        int GetRemainingSpace() const;
        bool IsEmpty() const { return count <= 0; }
        bool IsFull() const { return count >= maxStackSize; }
        void Clear() { count = 0; }
        std::string ToString() const;
    };

    /**
     * @struct InventorySlot
     * @brief Represents a single inventory slot
     */
    struct InventorySlot {
        int slotIndex;           ///< Slot index in inventory
        ItemStack itemStack;     ///< Item stack in this slot
        bool locked;             ///< Whether slot is locked
        std::string customName;  ///< Custom slot name

        InventorySlot(int index = 0);
        bool IsEmpty() const { return itemStack.IsEmpty(); }
        void Clear();
    };

    /**
     * @class Inventory
     * @brief Base inventory system for the game
     */
    class Inventory {
    public:
        /**
         * @brief Constructor
         * @param type Inventory type
         * @param size Number of slots
         */
        Inventory(InventoryType type = InventoryType::PLAYER, int size = 36);

        /**
         * @brief Destructor
         */
        virtual ~Inventory();

        // Getters
        InventoryType GetType() const { return m_type; }
        int GetSize() const { return m_size; }
        int GetMaxStackSize() const { return m_maxStackSize; }
        const std::string& GetName() const { return m_name; }

        // Setters
        void SetName(const std::string& name) { m_name = name; }
        void SetMaxStackSize(int maxStack) { m_maxStackSize = maxStack; }

        // Slot operations
        bool IsValidSlot(int slot) const;
        const InventorySlot* GetSlot(int slot) const;
        InventorySlot* GetSlot(int slot);

        // Item operations
        bool SetItem(int slot, const ItemStack& itemStack);
        bool SetItem(int slot, int itemID, int count = 1);
        ItemStack GetItem(int slot) const;
        bool RemoveItem(int slot, int count = 1);
        bool ClearSlot(int slot);
        void ClearAll();

        // Stack operations
        bool AddItem(const ItemStack& itemStack);
        bool AddItem(int itemID, int count = 1);
        bool RemoveItem(int itemID, int count = 1);
        int GetItemCount(int itemID) const;
        bool HasItem(int itemID, int count = 1) const;

        // Inventory operations
        bool SwapSlots(int slot1, int slot2);
        bool MoveItem(int fromSlot, int toSlot, int count = -1);
        bool SplitStack(int slot);
        bool MergeStacks(int fromSlot, int toSlot);

        // Search operations
        int FindEmptySlot() const;
        int FindItem(int itemID) const;
        std::vector<int> FindAllItems(int itemID) const;
        int GetFirstEmptySlot() const;
        int GetTotalItemCount(int itemID) const;

        // Advanced operations
        bool CanAddItem(const ItemStack& itemStack) const;
        bool CanAddItem(int itemID, int count = 1) const;
        int GetRemainingSpace(int itemID) const;

        // Event callbacks
        using InventoryChangeCallback = std::function<void(int slot, const ItemStack& oldStack, const ItemStack& newStack)>;
        void AddChangeListener(const InventoryChangeCallback& callback);
        void RemoveChangeListener(const InventoryChangeCallback& callback);

        // Serialization
        std::string Serialize() const;
        bool Deserialize(const std::string& data);

        // Debug
        void PrintInventory() const;
        std::string ToString() const;

    protected:
        InventoryType m_type;
        int m_size;
        int m_maxStackSize;
        std::string m_name;
        std::vector<InventorySlot> m_slots;
        mutable std::shared_mutex m_inventoryMutex;
        std::vector<InventoryChangeCallback> m_changeListeners;

        // Protected methods
        void NotifyChange(int slot, const ItemStack& oldStack, const ItemStack& newStack);
        bool IsValidItem(int itemID) const;
        int GetMaxStackSizeForItem(int itemID) const;
        virtual bool CanPlaceItem(int slot, const ItemStack& itemStack) const;
        virtual void OnItemChanged(int slot, const ItemStack& oldStack, const ItemStack& newStack);
    };

    /**
     * @class PlayerInventory
     * @brief Specialized inventory for players with hotbar and armor slots
     */
    class PlayerInventory : public Inventory {
    public:
        static const int HOTBAR_SIZE = 9;
        static const int ARMOR_SIZE = 4;
        static const int OFFHAND_SIZE = 1;
        static const int CRAFTING_SIZE = 4;
        static const int MAIN_INVENTORY_SIZE = 27;
        static const int TOTAL_SIZE = HOTBAR_SIZE + ARMOR_SIZE + OFFHAND_SIZE + CRAFTING_SIZE + MAIN_INVENTORY_SIZE;

        enum class PlayerInventorySlot {
            // Hotbar slots (0-8)
            HOTBAR_START = 0,
            HOTBAR_END = HOTBAR_START + HOTBAR_SIZE - 1,

            // Armor slots (9-12)
            HELMET = HOTBAR_END + 1,
            CHESTPLATE = HELMET + 1,
            LEGGINGS = CHESTPLATE + 1,
            BOOTS = LEGGINGS + 1,
            ARMOR_END = BOOTS,

            // Offhand slot (13)
            OFFHAND = ARMOR_END + 1,

            // Crafting slots (14-17)
            CRAFTING_START = OFFHAND + 1,
            CRAFTING_END = CRAFTING_START + CRAFTING_SIZE - 1,

            // Main inventory (18-44)
            MAIN_INVENTORY_START = CRAFTING_END + 1,
            MAIN_INVENTORY_END = MAIN_INVENTORY_START + MAIN_INVENTORY_SIZE - 1
        };

        /**
         * @brief Constructor
         */
        PlayerInventory();

        /**
         * @brief Destructor
         */
        ~PlayerInventory() override;

        // Hotbar operations
        bool SetHotbarSlot(int hotbarSlot, const ItemStack& itemStack);
        ItemStack GetHotbarSlot(int hotbarSlot) const;
        bool SelectHotbarSlot(int slot);
        int GetSelectedHotbarSlot() const { return m_selectedHotbarSlot; }
        ItemStack GetSelectedItem() const;

        // Armor operations
        bool SetArmorSlot(int armorSlot, const ItemStack& itemStack);
        ItemStack GetArmorSlot(int armorSlot) const;
        bool IsWearingArmor(int armorSlot) const;

        // Offhand operations
        bool SetOffhandItem(const ItemStack& itemStack);
        ItemStack GetOffhandItem() const;

        // Crafting operations
        bool SetCraftingSlot(int craftingSlot, const ItemStack& itemStack);
        ItemStack GetCraftingSlot(int craftingSlot) const;
        void ClearCraftingGrid();
        bool IsValidArmor(int itemID, int armorSlot) const;

        // Player-specific operations
        void UpdatePlayerStats();
        int GetTotalProtection() const;
        float GetMovementSpeedModifier() const;
        bool HasItemInHotbar(int itemID) const;

        // Armor durability
        bool DamageArmor(int damage);
        void RepairArmor(int repairAmount);

        // Quick access methods
        int GetHotbarIndex(int hotbarSlot) const;
        int GetArmorIndex(int armorSlot) const;
        int GetCraftingIndex(int craftingSlot) const;

        // Serialization for player data
        std::string SerializePlayerInventory() const;
        bool DeserializePlayerInventory(const std::string& data);

    protected:
        int m_selectedHotbarSlot;

        bool CanPlaceItem(int slot, const ItemStack& itemStack) const override;
        void OnItemChanged(int slot, const ItemStack& oldStack, const ItemStack& newStack) override;
    };

    /**
     * @class InventoryManager
     * @brief Manager for all inventories in the game
     */
    class InventoryManager {
    public:
        static InventoryManager& GetInstance();

        // Inventory creation and management
        std::shared_ptr<Inventory> CreateInventory(InventoryType type, int size = 27);
        std::shared_ptr<PlayerInventory> CreatePlayerInventory();
        bool DestroyInventory(std::shared_ptr<Inventory> inventory);

        // Player inventory management
        std::shared_ptr<PlayerInventory> GetPlayerInventory(PlayerEntity* player);
        bool SetPlayerInventory(PlayerEntity* player, std::shared_ptr<PlayerInventory> inventory);

        // Inventory operations
        bool TransferItem(std::shared_ptr<Inventory> from, int fromSlot,
                         std::shared_ptr<Inventory> to, int toSlot, int count = -1);
        bool TransferAllItems(std::shared_ptr<Inventory> from, std::shared_ptr<Inventory> to);

        // Item management
        bool RegisterItem(int itemID, const std::string& name, int maxStackSize = 64);
        bool UnregisterItem(int itemID);
        std::string GetItemName(int itemID) const;
        int GetItemMaxStackSize(int itemID) const;
        bool IsValidItem(int itemID) const;

        // Statistics
        size_t GetTotalInventoryCount() const;
        size_t GetTotalItemCount() const;

        // Cleanup
        void CleanupEmptyInventories();

    private:
        InventoryManager() = default;
        ~InventoryManager() = default;

        // Prevent copying
        InventoryManager(const InventoryManager&) = delete;
        InventoryManager& operator=(const InventoryManager&) = delete;

        // Data structures
        std::unordered_map<PlayerEntity*, std::shared_ptr<PlayerInventory>> m_playerInventories;
        std::vector<std::shared_ptr<Inventory>> m_inventories;
        std::unordered_map<int, std::pair<std::string, int>> m_itemRegistry;

        // Thread safety
        mutable std::shared_mutex m_managerMutex;
    };

} // namespace VoxelCraft