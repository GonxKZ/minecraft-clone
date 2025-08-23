/**
 * @file UIInventory.hpp
 * @brief VoxelCraft Inventory UI System
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_UI_UI_INVENTORY_HPP
#define VOXELCRAFT_UI_UI_INVENTORY_HPP

#include "UIWidgets.hpp"
#include "../inventory/Inventory.hpp"
#include <memory>

namespace VoxelCraft {

    class PlayerInventory;

    /**
     * @class UIInventoryPanel
     * @brief Complete inventory UI panel
     */
    class UIInventoryPanel : public UIElement {
    public:
        UIInventoryPanel(const std::string& id);
        ~UIInventoryPanel() override = default;

        void SetInventory(std::shared_ptr<PlayerInventory> inventory) { m_inventory = inventory; }
        std::shared_ptr<PlayerInventory> GetInventory() const { return m_inventory; }

        void SetVisible(bool visible) override;
        void Refresh();

        void Update(float deltaTime) override;
        void Render() override;
        bool HandleInput(const UIEvent& event) override;

    private:
        std::shared_ptr<PlayerInventory> m_inventory;
        std::vector<std::shared_ptr<UIInventorySlot>> m_slots;
        std::shared_ptr<UIElement> m_craftingGrid;
        std::shared_ptr<UIInventorySlot> m_craftingResult;
        std::shared_ptr<UIButton> m_closeButton;

        void InitializeSlots();
        void UpdateSlots();
        void HandleSlotClick(const std::string& slotId);
        void HandleCrafting();
    };

    /**
     * @class UICraftingTable
     * @brief Crafting table interface
     */
    class UICraftingTable : public UIElement {
    public:
        UICraftingTable(const std::string& id);
        ~UICraftingTable() override = default;

        void SetVisible(bool visible) override;
        void Refresh();

        void Update(float deltaTime) override;
        void Render() override;
        bool HandleInput(const UIEvent& event) override;

    private:
        std::vector<std::shared_ptr<UIInventorySlot>> m_craftingSlots;
        std::shared_ptr<UIInventorySlot> m_resultSlot;
        std::shared_ptr<UIButton> m_closeButton;

        void InitializeCraftingGrid();
        void HandleCraftingSlotClick(const std::string& slotId);
        void UpdateRecipe();
    };

    /**
     * @class UIFurnace
     * @brief Furnace interface
     */
    class UIFurnace : public UIElement {
    public:
        UIFurnace(const std::string& id);
        ~UIFurnace() override = default;

        void SetVisible(bool visible) override;
        void Refresh();

        void Update(float deltaTime) override;
        void Render() override;
        bool HandleInput(const UIEvent& event) override;

    private:
        std::shared_ptr<UIInventorySlot> m_inputSlot;
        std::shared_ptr<UIInventorySlot> m_fuelSlot;
        std::shared_ptr<UIInventorySlot> m_outputSlot;
        std::shared_ptr<UIProgressBar> m_progressBar;
        std::shared_ptr<UIButton> m_closeButton;

        void HandleSlotClick(const std::string& slotId);
        void UpdateSmelting();
    };

    /**
     * @class UIChest
     * @brief Chest interface
     */
    class UIChest : public UIElement {
    public:
        UIChest(const std::string& id);
        ~UIChest() override = default;

        void SetChestInventory(std::shared_ptr<Inventory> chestInventory) {
            m_chestInventory = chestInventory;
        }

        void SetPlayerInventory(std::shared_ptr<PlayerInventory> playerInventory) {
            m_playerInventory = playerInventory;
        }

        void SetVisible(bool visible) override;
        void Refresh();

        void Update(float deltaTime) override;
        void Render() override;
        bool HandleInput(const UIEvent& event) override;

    private:
        std::shared_ptr<Inventory> m_chestInventory;
        std::shared_ptr<PlayerInventory> m_playerInventory;
        std::vector<std::shared_ptr<UIInventorySlot>> m_chestSlots;
        std::vector<std::shared_ptr<UIInventorySlot>> m_playerSlots;
        std::shared_ptr<UIButton> m_closeButton;

        void InitializeSlots();
        void UpdateSlots();
        void HandleSlotClick(const std::string& slotId);
    };

    /**
     * @class UIDragDropManager
     * @brief Manages drag and drop operations for inventory
     */
    class UIDragDropManager {
    public:
        static UIDragDropManager& GetInstance();

        void StartDrag(const std::string& itemId, int count, const Vec2& startPos);
        void UpdateDrag(const Vec2& position);
        void EndDrag();

        bool IsDragging() const { return m_isDragging; }
        const std::string& GetDraggedItem() const { return m_draggedItem; }
        int GetDraggedCount() const { return m_draggedCount; }
        Vec2 GetDragPosition() const { return m_dragPosition; }

        void SetOnDropCallback(std::function<void(const std::string&, int, const Vec2&)> callback) {
            m_onDropCallback = callback;
        }

    private:
        UIDragDropManager() = default;

        bool m_isDragging = false;
        std::string m_draggedItem;
        int m_draggedCount = 0;
        Vec2 m_dragStartPos;
        Vec2 m_dragPosition;
        std::function<void(const std::string&, int, const Vec2&)> m_onDropCallback;
    };

    /**
     * @class UITooltip
     * @brief Item tooltip display
     */
    class UITooltip : public UIElement {
    public:
        UITooltip(const std::string& id);
        ~UITooltip() override = default;

        void Show(const std::string& itemId, const Vec2& position);
        void Hide();

        void Update(float deltaTime) override;
        void Render() override;

    private:
        std::string m_itemId;
        Vec2 m_targetPosition;
        float m_showTimer = 0.0f;
        bool m_isVisible = false;

        void UpdateContent();
        void UpdatePosition();
    };

    /**
     * @class UIItemSelector
     * @brief Item selection overlay (creative mode)
     */
    class UIItemSelector : public UIElement {
    public:
        UIItemSelector(const std::string& id);
        ~UIItemSelector() override = default;

        void SetItemList(const std::vector<std::string>& items) { m_items = items; }
        void SetOnItemSelected(std::function<void(const std::string&)> callback) {
            m_onItemSelected = callback;
        }

        void Show() { SetVisible(true); }
        void Hide() { SetVisible(false); }

        void Update(float deltaTime) override;
        void Render() override;
        bool HandleInput(const UIEvent& event) override;

    private:
        std::vector<std::string> m_items;
        int m_selectedIndex = 0;
        int m_scrollOffset = 0;
        std::function<void(const std::string&)> m_onItemSelected;

        void UpdateSelection();
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_UI_UI_INVENTORY_HPP
