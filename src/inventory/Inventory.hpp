/**
 * @file Inventory.hpp
 * @brief VoxelCraft Inventory System - Player and Container Inventories
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * Complete inventory system inspired by Minecraft with slots, stacking, and crafting integration
 */

#ifndef VOXELCRAFT_INVENTORY_INVENTORY_HPP
#define VOXELCRAFT_INVENTORY_INVENTORY_HPP

#include <vector>
#include <memory>
#include <unordered_map>
#include <functional>
#include "../crafting/CraftingRecipe.hpp"
#include "../blocks/Block.hpp"

namespace VoxelCraft {

    // Forward declarations
    class CraftingTable;
    struct ItemStack;

    /**
     * @enum InventoryType
     * @brief Types of inventories in the game
     */
    enum class InventoryType {
        PLAYER,           // Player's main inventory
        PLAYER_HOTBAR,    // Player's hotbar
        CRAFTING,         // Crafting result slots
        ARMOR,            // Armor slots
        OFFHAND,          // Offhand slot
        CONTAINER,        // Generic container (chest, etc.)
        FURNACE,          // Furnace inventory
        ENCHANTING,       // Enchanting table
        BREWING,          // Brewing stand
        ANVIL,            // Anvil
        BEACON,           // Beacon
        HOPPER,           // Hopper
        DISPENSER,        // Dispenser/Dropper
        SHULKER_BOX,      // Shulker box
        ENDER_CHEST,      // Ender chest
        HORSE,            // Horse inventory
        VILLAGER,         // Villager trading
        MINECART,         // Minecart with chest
        CUSTOM            // Custom inventory type
    };

    /**
     * @enum ArmorSlot
     * @brief Armor equipment slots
     */
    enum class ArmorSlot {
        HELMET = 0,    // Head slot
        CHESTPLATE,    // Chest slot
        LEGGINGS,      // Legs slot
        BOOTS,         // Feet slot
        COUNT          // Number of armor slots
    };

    /**
     * @struct InventorySlot
     * @brief A single slot in an inventory
     */
    struct InventorySlot {
        ItemStack item;           ///< Item in the slot
        int maxStackSize;        ///< Maximum stack size for this slot
        bool locked;             ///< Whether the slot is locked
        std::string slotName;    ///< Name/description of the slot

        InventorySlot()
            : maxStackSize(64)
            , locked(false)
            , slotName("")
        {}

        InventorySlot(int maxStack, const std::string& name = "")
            : maxStackSize(maxStack)
            , locked(false)
            , slotName(name)
        {}

        bool IsEmpty() const {
            return item.IsEmpty();
        }

        bool CanHoldItem(const ItemStack& newItem) const {
            if (locked) return false;
            if (IsEmpty()) return newItem.count <= maxStackSize;
            if (item.item == newItem.item && item.nbt == newItem.nbt) {
                return (item.count + newItem.count) <= maxStackSize;
            }
            return false;
        }
    };

    /**
     * @class Inventory
     * @brief Base class for all inventory types
     */
    class Inventory {
    public:
        /**
         * @brief Constructor
         * @param type Inventory type
         * @param size Number of slots
         */
        Inventory(InventoryType type, int size);

        /**
         * @brief Virtual destructor
         */
        virtual ~Inventory() = default;

        /**
         * @brief Get inventory type
         * @return Inventory type
         */
        InventoryType GetType() const { return m_type; }

        /**
         * @brief Get inventory size
         * @return Number of slots
         */
        int GetSize() const { return m_slots.size(); }

        /**
         * @brief Get item from slot
         * @param slot Slot index
         * @return Item in slot
         */
        const ItemStack& GetItem(int slot) const;

        /**
         * @brief Set item in slot
         * @param slot Slot index
         * @param item Item to set
         * @return true if successful
         */
        bool SetItem(int slot, const ItemStack& item);

        /**
         * @brief Add item to inventory
         * @param item Item to add
         * @return Number of items that couldn't be added
         */
        int AddItem(const ItemStack& item);

        /**
         * @brief Remove item from slot
         * @param slot Slot index
         * @param count Number of items to remove
         * @return Items removed
         */
        ItemStack RemoveItem(int slot, int count = 1);

        /**
         * @brief Check if inventory has space for item
         * @param item Item to check
         * @return true if can add item
         */
        bool HasSpaceFor(const ItemStack& item) const;

        /**
         * @brief Find item in inventory
         * @param itemType Item to find
         * @return Vector of slot indices containing the item
         */
        std::vector<int> FindItem(BlockType itemType) const;

        /**
         * @brief Count total items of type
         * @param itemType Item to count
         * @return Total count
         */
        int CountItem(BlockType itemType) const;

        /**
         * @brief Clear slot
         * @param slot Slot index
         */
        void ClearSlot(int slot);

        /**
         * @brief Clear entire inventory
         */
        void Clear();

        /**
         * @brief Check if inventory is empty
         * @return true if empty
         */
        bool IsEmpty() const;

        /**
         * @brief Get slot information
         * @param slot Slot index
         * @return Slot information
         */
        const InventorySlot& GetSlot(int slot) const;

        /**
         * @brief Check if slot is valid
         * @param slot Slot index
         * @return true if valid
         */
        bool IsValidSlot(int slot) const {
            return slot >= 0 && slot < GetSize();
        }

        /**
         * @brief Swap items between slots
         * @param slot1 First slot
         * @param slot2 Second slot
         * @return true if successful
         */
        bool SwapSlots(int slot1, int slot2);

        /**
         * @brief Move item from one slot to another
         * @param fromSlot Source slot
         * @param toSlot Destination slot
         * @param count Number of items to move
         * @return Items actually moved
         */
        int MoveItem(int fromSlot, int toSlot, int count = -1);

        /**
         * @brief Add inventory change callback
         * @param callback Function called when inventory changes
         */
        void AddChangeCallback(std::function<void(int slot, const ItemStack& oldItem, const ItemStack& newItem)> callback);

        /**
         * @brief Remove all change callbacks
         */
        void ClearChangeCallbacks();

        /**
         * @brief Get all slots
         * @return Vector of all slots
         */
        const std::vector<InventorySlot>& GetAllSlots() const { return m_slots; }

    protected:
        InventoryType m_type;                                    ///< Inventory type
        std::vector<InventorySlot> m_slots;                      ///< Inventory slots
        std::vector<std::function<void(int slot, const ItemStack& oldItem, const ItemStack& newItem)>> m_callbacks;  ///< Change callbacks

        /**
         * @brief Notify about slot change
         * @param slot Changed slot
         * @param oldItem Previous item
         * @param newItem New item
         */
        void NotifySlotChange(int slot, const ItemStack& oldItem, const ItemStack& newItem);
    };

    /**
     * @class PlayerInventory
     * @brief Player's complete inventory system (hotbar + main inventory + armor + offhand)
     */
    class PlayerInventory : public Inventory {
    public:
        /**
         * @brief Constructor
         */
        PlayerInventory();

        /**
         * @brief Get hotbar item
         * @param slot Hotbar slot (0-8)
         * @return Item in hotbar slot
         */
        const ItemStack& GetHotbarItem(int slot) const;

        /**
         * @brief Set hotbar item
         * @param slot Hotbar slot (0-8)
         * @param item Item to set
         * @return true if successful
         */
        bool SetHotbarItem(int slot, const ItemStack& item);

        /**
         * @brief Get main inventory item
         * @param row Row (0-3)
         * @param col Column (0-8)
         * @return Item in main inventory
         */
        const ItemStack& GetMainInventoryItem(int row, int col) const;

        /**
         * @brief Set main inventory item
         * @param row Row (0-3)
         * @param col Column (0-8)
         * @param item Item to set
         * @return true if successful
         */
        bool SetMainInventoryItem(int row, int col, const ItemStack& item);

        /**
         * @brief Get armor item
         * @param slot Armor slot
         * @return Item in armor slot
         */
        const ItemStack& GetArmorItem(ArmorSlot slot) const;

        /**
         * @brief Set armor item
         * @param slot Armor slot
         * @param item Item to set
         * @return true if successful
         */
        bool SetArmorItem(ArmorSlot slot, const ItemStack& item);

        /**
         * @brief Get offhand item
         * @return Item in offhand slot
         */
        const ItemStack& GetOffhandItem() const;

        /**
         * @brief Set offhand item
         * @param item Item to set
         * @return true if successful
         */
        bool SetOffhandItem(const ItemStack& item);

        /**
         * @brief Get selected hotbar slot
         * @return Selected slot (0-8)
         */
        int GetSelectedHotbarSlot() const { return m_selectedHotbarSlot; }

        /**
         * @brief Set selected hotbar slot
         * @param slot Selected slot (0-8)
         * @return true if successful
         */
        bool SetSelectedHotbarSlot(int slot);

        /**
         * @brief Get currently selected item
         * @return Selected item
         */
        const ItemStack& GetSelectedItem() const;

        /**
         * @brief Get 2x2 crafting table for player inventory
         * @return Player crafting table
         */
        std::shared_ptr<CraftingTable> GetCraftingTable() const { return m_craftingTable; }

        /**
         * @brief Pick up item from world
         * @param item Item to pick up
         * @return Number of items that couldn't be picked up
         */
        int PickUpItem(const ItemStack& item);

        /**
         * @brief Drop item from slot
         * @param slot Slot to drop from
         * @param count Number of items to drop
         * @return Items dropped
         */
        ItemStack DropItem(int slot, int count = 1);

        // Constants for slot layout
        static constexpr int HOTBAR_SLOTS = 9;
        static constexpr int MAIN_INVENTORY_ROWS = 3;
        static constexpr int MAIN_INVENTORY_COLS = 9;
        static constexpr int ARMOR_SLOTS = 4;
        static constexpr int OFFHAND_SLOTS = 1;
        static constexpr int CRAFTING_SLOTS = 4; // 2x2 crafting grid
        static constexpr int TOTAL_SLOTS = HOTBAR_SLOTS + MAIN_INVENTORY_ROWS * MAIN_INVENTORY_COLS +
                                          ARMOR_SLOTS + OFFHAND_SLOTS + CRAFTING_SLOTS;

    private:
        int m_selectedHotbarSlot;                    ///< Currently selected hotbar slot (0-8)
        std::shared_ptr<CraftingTable> m_craftingTable;  ///< Player's crafting table

        /**
         * @brief Get hotbar slot index from hotbar slot number
         * @param hotbarSlot Hotbar slot (0-8)
         * @return Internal slot index
         */
        int GetHotbarSlotIndex(int hotbarSlot) const { return hotbarSlot; }

        /**
         * @brief Get main inventory slot index
         * @param row Row (0-3)
         * @param col Column (0-8)
         * @return Internal slot index
         */
        int GetMainInventorySlotIndex(int row, int col) const {
            return HOTBAR_SLOTS + row * MAIN_INVENTORY_COLS + col;
        }

        /**
         * @brief Get armor slot index
         * @param slot Armor slot
         * @return Internal slot index
         */
        int GetArmorSlotIndex(ArmorSlot slot) const {
            return HOTBAR_SLOTS + MAIN_INVENTORY_ROWS * MAIN_INVENTORY_COLS + static_cast<int>(slot);
        }

        /**
         * @brief Get offhand slot index
         * @return Internal slot index
         */
        int GetOffhandSlotIndex() const {
            return HOTBAR_SLOTS + MAIN_INVENTORY_ROWS * MAIN_INVENTORY_COLS + ARMOR_SLOTS;
        }

        /**
         * @brief Get crafting slot index
         * @param index Crafting slot (0-3)
         * @return Internal slot index
         */
        int GetCraftingSlotIndex(int index) const {
            return HOTBAR_SLOTS + MAIN_INVENTORY_ROWS * MAIN_INVENTORY_COLS + ARMOR_SLOTS + OFFHAND_SLOTS + index;
        }
    };

    /**
     * @class ContainerInventory
     * @brief Generic container inventory (chest, furnace, etc.)
     */
    class ContainerInventory : public Inventory {
    public:
        /**
         * @brief Constructor
         * @param size Number of slots
         * @param name Container name
         */
        ContainerInventory(int size, const std::string& name = "Container");

        /**
         * @brief Get container name
         * @return Container name
         */
        const std::string& GetName() const { return m_name; }

        /**
         * @brief Set container name
         * @param name New name
         */
        void SetName(const std::string& name) { m_name = name; }

    private:
        std::string m_name;  ///< Container name
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_INVENTORY_INVENTORY_HPP