/**
 * @file Inventory.cpp
 * @brief VoxelCraft Inventory System - Inventory Implementation
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#include "Inventory.hpp"
#include "../crafting/CraftingTable.hpp"
#include <algorithm>

namespace VoxelCraft {

    // Inventory Implementation
    Inventory::Inventory(InventoryType type, int size)
        : m_type(type)
        , m_slots(size)
    {
        if (size <= 0) {
            throw std::invalid_argument("Inventory size must be positive");
        }
    }

    const ItemStack& Inventory::GetItem(int slot) const {
        if (!IsValidSlot(slot)) {
            static ItemStack empty;
            return empty;
        }
        return m_slots[slot].item;
    }

    bool Inventory::SetItem(int slot, const ItemStack& item) {
        if (!IsValidSlot(slot)) {
            return false;
        }

        if (m_slots[slot].locked && !item.IsEmpty()) {
            return false;
        }

        ItemStack oldItem = m_slots[slot].item;
        m_slots[slot].item = item;

        NotifySlotChange(slot, oldItem, item);
        return true;
    }

    int Inventory::AddItem(const ItemStack& item) {
        if (item.IsEmpty()) {
            return 0;
        }

        int remaining = item.count;

        // First, try to add to existing stacks
        for (int i = 0; i < GetSize() && remaining > 0; ++i) {
            if (m_slots[i].CanHoldItem(ItemStack(item.item, remaining, item.nbt))) {
                int space = m_slots[i].maxStackSize - m_slots[i].item.count;
                int toAdd = std::min(remaining, space);

                if (m_slots[i].IsEmpty()) {
                    m_slots[i].item = ItemStack(item.item, toAdd, item.nbt);
                } else {
                    m_slots[i].item.count += toAdd;
                }

                remaining -= toAdd;
                NotifySlotChange(i, ItemStack(), m_slots[i].item);
            }
        }

        // Then, try to add to empty slots
        for (int i = 0; i < GetSize() && remaining > 0; ++i) {
            if (m_slots[i].IsEmpty()) {
                int toAdd = std::min(remaining, m_slots[i].maxStackSize);
                m_slots[i].item = ItemStack(item.item, toAdd, item.nbt);
                remaining -= toAdd;
                NotifySlotChange(i, ItemStack(), m_slots[i].item);
            }
        }

        return remaining;
    }

    ItemStack Inventory::RemoveItem(int slot, int count) {
        if (!IsValidSlot(slot) || m_slots[slot].IsEmpty()) {
            return ItemStack();
        }

        ItemStack removed = m_slots[slot].item;
        int toRemove = std::min(count, removed.count);

        removed.count = toRemove;
        m_slots[slot].item.count -= toRemove;

        if (m_slots[slot].item.count <= 0) {
            m_slots[slot].item = ItemStack();
        }

        NotifySlotChange(slot, removed, m_slots[slot].item);
        return removed;
    }

    bool Inventory::HasSpaceFor(const ItemStack& item) const {
        if (item.IsEmpty()) {
            return true;
        }

        int remaining = item.count;

        // Check existing stacks
        for (const auto& slot : m_slots) {
            if (!slot.locked && !slot.IsEmpty() &&
                slot.item.item == item.item && slot.item.nbt == item.nbt) {
                int space = slot.maxStackSize - slot.item.count;
                remaining -= space;
                if (remaining <= 0) return true;
            }
        }

        // Check empty slots
        for (const auto& slot : m_slots) {
            if (!slot.locked && slot.IsEmpty()) {
                remaining -= slot.maxStackSize;
                if (remaining <= 0) return true;
            }
        }

        return false;
    }

    std::vector<int> Inventory::FindItem(BlockType itemType) const {
        std::vector<int> result;
        for (int i = 0; i < GetSize(); ++i) {
            if (!m_slots[i].IsEmpty() && m_slots[i].item.item == itemType) {
                result.push_back(i);
            }
        }
        return result;
    }

    int Inventory::CountItem(BlockType itemType) const {
        int count = 0;
        for (const auto& slot : m_slots) {
            if (!slot.IsEmpty() && slot.item.item == itemType) {
                count += slot.item.count;
            }
        }
        return count;
    }

    void Inventory::ClearSlot(int slot) {
        if (IsValidSlot(slot)) {
            ItemStack oldItem = m_slots[slot].item;
            m_slots[slot].item = ItemStack();
            NotifySlotChange(slot, oldItem, ItemStack());
        }
    }

    void Inventory::Clear() {
        for (int i = 0; i < GetSize(); ++i) {
            ClearSlot(i);
        }
    }

    bool Inventory::IsEmpty() const {
        for (const auto& slot : m_slots) {
            if (!slot.IsEmpty()) {
                return false;
            }
        }
        return true;
    }

    const InventorySlot& Inventory::GetSlot(int slot) const {
        if (!IsValidSlot(slot)) {
            static InventorySlot emptySlot;
            return emptySlot;
        }
        return m_slots[slot];
    }

    bool Inventory::SwapSlots(int slot1, int slot2) {
        if (!IsValidSlot(slot1) || !IsValidSlot(slot2)) {
            return false;
        }

        if (m_slots[slot1].locked || m_slots[slot2].locked) {
            return false;
        }

        ItemStack item1 = m_slots[slot1].item;
        ItemStack item2 = m_slots[slot2].item;

        m_slots[slot1].item = item2;
        m_slots[slot2].item = item1;

        NotifySlotChange(slot1, item1, item2);
        NotifySlotChange(slot2, item2, item1);

        return true;
    }

    int Inventory::MoveItem(int fromSlot, int toSlot, int count) {
        if (!IsValidSlot(fromSlot) || !IsValidSlot(toSlot)) {
            return 0;
        }

        if (m_slots[fromSlot].IsEmpty() || m_slots[toSlot].locked) {
            return 0;
        }

        ItemStack fromItem = m_slots[fromSlot].item;
        int toMove = (count == -1) ? fromItem.count : std::min(count, fromItem.count);

        if (m_slots[toSlot].IsEmpty()) {
            // Moving to empty slot
            int canMove = std::min(toMove, m_slots[toSlot].maxStackSize);
            m_slots[toSlot].item = ItemStack(fromItem.item, canMove, fromItem.nbt);
            m_slots[fromSlot].item.count -= canMove;

            if (m_slots[fromSlot].item.count <= 0) {
                m_slots[fromSlot].item = ItemStack();
            }

            NotifySlotChange(fromSlot, fromItem, m_slots[fromSlot].item);
            NotifySlotChange(toSlot, ItemStack(), m_slots[toSlot].item);

            return canMove;
        } else if (m_slots[toSlot].item.item == fromItem.item &&
                   m_slots[toSlot].item.nbt == fromItem.nbt) {
            // Stacking items
            int space = m_slots[toSlot].maxStackSize - m_slots[toSlot].item.count;
            int canMove = std::min(toMove, space);

            m_slots[toSlot].item.count += canMove;
            m_slots[fromSlot].item.count -= canMove;

            if (m_slots[fromSlot].item.count <= 0) {
                m_slots[fromSlot].item = ItemStack();
            }

            NotifySlotChange(fromSlot, fromItem, m_slots[fromSlot].item);
            NotifySlotChange(toSlot, ItemStack(fromItem.item, m_slots[toSlot].item.count - canMove, fromItem.nbt), m_slots[toSlot].item);

            return canMove;
        }

        return 0;
    }

    void Inventory::AddChangeCallback(std::function<void(int slot, const ItemStack& oldItem, const ItemStack& newItem)> callback) {
        m_callbacks.push_back(callback);
    }

    void Inventory::ClearChangeCallbacks() {
        m_callbacks.clear();
    }

    void Inventory::NotifySlotChange(int slot, const ItemStack& oldItem, const ItemStack& newItem) {
        for (const auto& callback : m_callbacks) {
            callback(slot, oldItem, newItem);
        }
    }

    // PlayerInventory Implementation
    PlayerInventory::PlayerInventory()
        : Inventory(InventoryType::PLAYER, TOTAL_SLOTS)
        , m_selectedHotbarSlot(0)
    {
        // Initialize slot names and properties
        for (int i = 0; i < HOTBAR_SLOTS; ++i) {
            m_slots[i].slotName = "Hotbar " + std::to_string(i + 1);
        }

        for (int row = 0; row < MAIN_INVENTORY_ROWS; ++row) {
            for (int col = 0; col < MAIN_INVENTORY_COLS; ++col) {
                int slot = GetMainInventorySlotIndex(row, col);
                m_slots[slot].slotName = "Inventory " + std::to_string(row + 1) + "," + std::to_string(col + 1);
            }
        }

        // Armor slots
        m_slots[GetArmorSlotIndex(ArmorSlot::HELMET)].slotName = "Helmet";
        m_slots[GetArmorSlotIndex(ArmorSlot::CHESTPLATE)].slotName = "Chestplate";
        m_slots[GetArmorSlotIndex(ArmorSlot::LEGGINGS)].slotName = "Leggings";
        m_slots[GetArmorSlotIndex(ArmorSlot::BOOTS)].slotName = "Boots";

        // Offhand slot
        m_slots[GetOffhandSlotIndex()].slotName = "Offhand";

        // Crafting slots
        for (int i = 0; i < CRAFTING_SLOTS; ++i) {
            int slot = GetCraftingSlotIndex(i);
            m_slots[slot].slotName = "Crafting " + std::to_string(i + 1);
        }

        // Create crafting table
        m_craftingTable = std::make_shared<PlayerCraftingTable>();

        // Connect crafting table to inventory
        auto updateCrafting = [this](int slot, const ItemStack& oldItem, const ItemStack& newItem) {
            if (slot >= GetCraftingSlotIndex(0) && slot <= GetCraftingSlotIndex(CRAFTING_SLOTS - 1)) {
                int craftingSlot = slot - GetCraftingSlotIndex(0);
                int row = craftingSlot / 2;
                int col = craftingSlot % 2;
                m_craftingTable->SetInputItem(col, row, newItem);
            }
        };

        AddChangeCallback(updateCrafting);
    }

    const ItemStack& PlayerInventory::GetHotbarItem(int slot) const {
        if (slot < 0 || slot >= HOTBAR_SLOTS) {
            static ItemStack empty;
            return empty;
        }
        return GetItem(GetHotbarSlotIndex(slot));
    }

    bool PlayerInventory::SetHotbarItem(int slot, const ItemStack& item) {
        if (slot < 0 || slot >= HOTBAR_SLOTS) {
            return false;
        }
        return SetItem(GetHotbarSlotIndex(slot), item);
    }

    const ItemStack& PlayerInventory::GetMainInventoryItem(int row, int col) const {
        if (row < 0 || row >= MAIN_INVENTORY_ROWS || col < 0 || col >= MAIN_INVENTORY_COLS) {
            static ItemStack empty;
            return empty;
        }
        return GetItem(GetMainInventorySlotIndex(row, col));
    }

    bool PlayerInventory::SetMainInventoryItem(int row, int col, const ItemStack& item) {
        if (row < 0 || row >= MAIN_INVENTORY_ROWS || col < 0 || col >= MAIN_INVENTORY_COLS) {
            return false;
        }
        return SetItem(GetMainInventorySlotIndex(row, col), item);
    }

    const ItemStack& PlayerInventory::GetArmorItem(ArmorSlot slot) const {
        return GetItem(GetArmorSlotIndex(slot));
    }

    bool PlayerInventory::SetArmorItem(ArmorSlot slot, const ItemStack& item) {
        return SetItem(GetArmorSlotIndex(slot), item);
    }

    const ItemStack& PlayerInventory::GetOffhandItem() const {
        return GetItem(GetOffhandSlotIndex());
    }

    bool PlayerInventory::SetOffhandItem(const ItemStack& item) {
        return SetItem(GetOffhandSlotIndex(), item);
    }

    bool PlayerInventory::SetSelectedHotbarSlot(int slot) {
        if (slot < 0 || slot >= HOTBAR_SLOTS) {
            return false;
        }
        m_selectedHotbarSlot = slot;
        return true;
    }

    const ItemStack& PlayerInventory::GetSelectedItem() const {
        return GetHotbarItem(m_selectedHotbarSlot);
    }

    int PlayerInventory::PickUpItem(const ItemStack& item) {
        return AddItem(item);
    }

    ItemStack PlayerInventory::DropItem(int slot, int count) {
        return RemoveItem(slot, count);
    }

    // ContainerInventory Implementation
    ContainerInventory::ContainerInventory(int size, const std::string& name)
        : Inventory(InventoryType::CONTAINER, size)
        , m_name(name)
    {
        // Set slot names
        for (int i = 0; i < size; ++i) {
            m_slots[i].slotName = name + " Slot " + std::to_string(i + 1);
        }
    }

} // namespace VoxelCraft
