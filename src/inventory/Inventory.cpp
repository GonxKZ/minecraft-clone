#include "Inventory.hpp"
#include "entities/PlayerEntity.hpp"
#include <algorithm>
#include <sstream>
#include <iostream>

namespace VoxelCraft {

    // ItemStack implementation
    ItemStack::ItemStack(int id, int cnt, int maxStack, int meta, const std::string& name)
        : itemID(id), count(cnt), maxStackSize(maxStack), metadata(meta), itemName(name)
    {
    }

    bool ItemStack::CanStackWith(const ItemStack& other) const {
        if (itemID != other.itemID) return false;
        if (metadata != other.metadata) return false;
        return count + other.count <= maxStackSize;
    }

    int ItemStack::GetRemainingSpace() const {
        return maxStackSize - count;
    }

    std::string ItemStack::ToString() const {
        std::stringstream ss;
        ss << "ItemStack[ID=" << itemID << ", Count=" << count << "/" << maxStackSize << "]";
        return ss.str();
    }

    // Inventory implementation
    Inventory::Inventory(InventoryType type, int size)
        : m_type(type), m_size(size), m_maxStackSize(64), m_slots(size) {
        for (int i = 0; i < size; ++i) {
            m_slots[i].slotIndex = i;
        }
    }

    Inventory::~Inventory() {
        ClearAll();
    }

    bool Inventory::IsValidSlot(int slot) const {
        return slot >= 0 && slot < m_size;
    }

    bool Inventory::SetItem(int slot, const ItemStack& itemStack) {
        if (!IsValidSlot(slot)) return false;

        auto oldStack = m_slots[slot].itemStack;
        m_slots[slot].itemStack = itemStack;
        NotifyChange(slot, oldStack, itemStack);

        return true;
    }

    ItemStack Inventory::GetItem(int slot) const {
        if (!IsValidSlot(slot)) return ItemStack();
        return m_slots[slot].itemStack;
    }

    bool Inventory::AddItem(const ItemStack& itemStack) {
        if (itemStack.IsEmpty()) return true;

        // First, try to stack with existing items
        for (int i = 0; i < m_size; ++i) {
            auto& slot = m_slots[i];
            if (slot.itemStack.CanStackWith(itemStack)) {
                int space = slot.itemStack.GetRemainingSpace();
                if (space > 0) {
                    auto oldStack = slot.itemStack;
                    slot.itemStack.count += std::min(itemStack.count, space);
                    NotifyChange(i, oldStack, slot.itemStack);
                    return true;
                }
            }
        }

        // Then, add to empty slots
        for (int i = 0; i < m_size; ++i) {
            auto& slot = m_slots[i];
            if (slot.IsEmpty()) {
                slot.itemStack = itemStack;
                NotifyChange(i, ItemStack(), itemStack);
                return true;
            }
        }

        return false;
    }

    bool Inventory::RemoveItem(int itemID, int count) {
        int remaining = count;

        for (int i = 0; i < m_size && remaining > 0; ++i) {
            auto& slot = m_slots[i];
            if (slot.itemStack.itemID == itemID && !slot.IsEmpty()) {
                int toRemove = std::min(remaining, slot.itemStack.count);
                auto oldStack = slot.itemStack;

                slot.itemStack.count -= toRemove;
                if (slot.itemStack.IsEmpty()) {
                    slot.Clear();
                }

                NotifyChange(i, oldStack, slot.itemStack);
                remaining -= toRemove;
            }
        }

        return remaining == 0;
    }

    int Inventory::GetItemCount(int itemID) const {
        int total = 0;
        for (const auto& slot : m_slots) {
            if (slot.itemStack.itemID == itemID) {
                total += slot.itemStack.count;
            }
        }
        return total;
    }

    bool Inventory::ClearSlot(int slot) {
        if (!IsValidSlot(slot)) return false;

        auto oldStack = m_slots[slot].itemStack;
        m_slots[slot].Clear();
        NotifyChange(slot, oldStack, m_slots[slot].itemStack);

        return true;
    }

    void Inventory::ClearAll() {
        for (int i = 0; i < m_size; ++i) {
            ClearSlot(i);
        }
    }

    int Inventory::FindEmptySlot() const {
        for (int i = 0; i < m_size; ++i) {
            if (m_slots[i].IsEmpty()) {
                return i;
            }
        }
        return -1;
    }

    void Inventory::NotifyChange(int slot, const ItemStack& oldStack, const ItemStack& newStack) {
        for (const auto& callback : m_changeListeners) {
            if (callback) {
                callback(slot, oldStack, newStack);
            }
        }
    }

    std::string Inventory::ToString() const {
        std::stringstream ss;
        ss << "Inventory[Size=" << m_size << ", Items=" << GetItemCount(-1) << "]";
        return ss.str();
    }

    // PlayerInventory implementation
    PlayerInventory::PlayerInventory()
        : Inventory(InventoryType::PLAYER, TOTAL_SIZE), m_selectedHotbarSlot(0) {
    }

    bool PlayerInventory::SetHotbarSlot(int hotbarSlot, const ItemStack& itemStack) {
        if (hotbarSlot < 0 || hotbarSlot >= HOTBAR_SIZE) return false;
        return SetItem(hotbarSlot, itemStack);
    }

    ItemStack PlayerInventory::GetHotbarSlot(int hotbarSlot) const {
        if (hotbarSlot < 0 || hotbarSlot >= HOTBAR_SIZE) return ItemStack();
        return GetItem(hotbarSlot);
    }

    bool PlayerInventory::SelectHotbarSlot(int slot) {
        if (slot >= 0 && slot < HOTBAR_SIZE) {
            m_selectedHotbarSlot = slot;
            return true;
        }
        return false;
    }

    ItemStack PlayerInventory::GetSelectedItem() const {
        return GetHotbarSlot(m_selectedHotbarSlot);
    }

    // InventoryManager implementation
    InventoryManager& InventoryManager::GetInstance() {
        static InventoryManager instance;
        return instance;
    }

    std::shared_ptr<Inventory> InventoryManager::CreateInventory(InventoryType type, int size) {
        return std::make_shared<Inventory>(type, size);
    }

    std::shared_ptr<PlayerInventory> InventoryManager::CreatePlayerInventory() {
        return std::make_shared<PlayerInventory>();
    }

    bool InventoryManager::RegisterItem(int itemID, const std::string& name, int maxStackSize) {
        if (m_itemRegistry.find(itemID) != m_itemRegistry.end()) {
            return false;
        }
        m_itemRegistry[itemID] = std::make_pair(name, maxStackSize);
        return true;
    }

    std::string InventoryManager::GetItemName(int itemID) const {
        auto it = m_itemRegistry.find(itemID);
        return (it != m_itemRegistry.end()) ? it->second.first : "Unknown Item";
    }

    int InventoryManager::GetItemMaxStackSize(int itemID) const {
        auto it = m_itemRegistry.find(itemID);
        return (it != m_itemRegistry.end()) ? it->second.second : 64;
    }

    bool InventoryManager::IsValidItem(int itemID) const {
        return m_itemRegistry.find(itemID) != m_itemRegistry.end();
    }

}