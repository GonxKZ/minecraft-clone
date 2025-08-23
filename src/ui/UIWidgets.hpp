/**
 * @file UIWidgets.hpp
 * @brief VoxelCraft UI Widgets and Controls
 * @version 1.0.0
 * @author VoxelCraft Team
 */

#ifndef VOXELCRAFT_UI_UI_WIDGETS_HPP
#define VOXELCRAFT_UI_UI_WIDGETS_HPP

#include "UISystem.hpp"
#include <vector>
#include <string>
#include <functional>

namespace VoxelCraft {

    /**
     * @class UIButton
     * @brief Interactive button widget
     */
    class UIButton : public UIElement {
    public:
        UIButton(const std::string& id);
        ~UIButton() override = default;

        void SetText(const std::string& text) { m_text = text; }
        const std::string& GetText() const { return m_text; }

        void SetOnClick(std::function<void()> callback) { m_onClick = callback; }
        void SetOnHover(std::function<void(bool)> callback) { m_onHover = callback; }

        void Update(float deltaTime) override;
        void Render() override;
        bool HandleInput(const UIEvent& event) override;

    private:
        std::string m_text;
        std::function<void()> m_onClick;
        std::function<void(bool)> m_onHover;
        bool m_isHovered = false;
        bool m_isPressed = false;
    };

    /**
     * @class UIText
     * @brief Text display widget
     */
    class UIText : public UIElement {
    public:
        UIText(const std::string& id);
        ~UIText() override = default;

        void SetText(const std::string& text) { m_text = text; }
        const std::string& GetText() const { return m_text; }

        void SetFontSize(float size) { m_style.fontSize = size; }
        void SetColor(const Vec4& color) { m_style.textColor = color; }
        void SetAlignment(UIAnchor alignment) { m_alignment = alignment; }

        void Update(float deltaTime) override;
        void Render() override;

    private:
        std::string m_text;
        UIAnchor m_alignment = UIAnchor::TOP_LEFT;
    };

    /**
     * @class UIImage
     * @brief Image display widget
     */
    class UIImage : public UIElement {
    public:
        UIImage(const std::string& id);
        ~UIImage() override = default;

        void SetImage(const std::string& imagePath) { m_imagePath = imagePath; }
        const std::string& GetImage() const { return m_imagePath; }

        void SetColor(const Vec4& color) { m_tintColor = color; }
        void SetUVRect(const UIRect& uvRect) { m_uvRect = uvRect; }

        void Update(float deltaTime) override;
        void Render() override;

    private:
        std::string m_imagePath;
        Vec4 m_tintColor = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
        UIRect m_uvRect = UIRect(0.0f, 0.0f, 1.0f, 1.0f);
    };

    /**
     * @class UISlider
     * @brief Draggable slider widget
     */
    class UISlider : public UIElement {
    public:
        UISlider(const std::string& id);
        ~UISlider() override = default;

        void SetValue(float value) { m_value = std::max(m_minValue, std::min(m_maxValue, value)); }
        float GetValue() const { return m_value; }

        void SetRange(float minValue, float maxValue) {
            m_minValue = minValue;
            m_maxValue = maxValue;
            SetValue(m_value);
        }

        void SetOnValueChanged(std::function<void(float)> callback) { m_onValueChanged = callback; }

        void Update(float deltaTime) override;
        void Render() override;
        bool HandleInput(const UIEvent& event) override;

    private:
        float m_value = 0.0f;
        float m_minValue = 0.0f;
        float m_maxValue = 1.0f;
        std::function<void(float)> m_onValueChanged;
        bool m_isDragging = false;
    };

    /**
     * @class UICheckbox
     * @brief Toggle checkbox widget
     */
    class UICheckbox : public UIElement {
    public:
        UICheckbox(const std::string& id);
        ~UICheckbox() override = default;

        void SetChecked(bool checked) { m_checked = checked; }
        bool IsChecked() const { return m_checked; }

        void SetLabel(const std::string& label) { m_label = label; }
        const std::string& GetLabel() const { return m_label; }

        void SetOnCheckedChanged(std::function<void(bool)> callback) { m_onCheckedChanged = callback; }

        void Update(float deltaTime) override;
        void Render() override;
        bool HandleInput(const UIEvent& event) override;

    private:
        bool m_checked = false;
        std::string m_label;
        std::function<void(bool)> m_onCheckedChanged;
    };

    /**
     * @class UIProgressBar
     * @brief Progress display widget
     */
    class UIProgressBar : public UIElement {
    public:
        UIProgressBar(const std::string& id);
        ~UIProgressBar() override = default;

        void SetProgress(float progress) { m_progress = std::max(0.0f, std::min(1.0f, progress)); }
        float GetProgress() const { return m_progress; }

        void SetText(const std::string& text) { m_text = text; }
        void SetShowPercentage(bool show) { m_showPercentage = show; }

        void Update(float deltaTime) override;
        void Render() override;

    private:
        float m_progress = 0.0f;
        std::string m_text;
        bool m_showPercentage = true;
    };

    /**
     * @class UIInventorySlot
     * @brief Minecraft-style inventory slot
     */
    class UIInventorySlot : public UIElement {
    public:
        UIInventorySlot(const std::string& id);
        ~UIInventorySlot() override = default;

        void SetItem(const std::string& itemId, int count = 1) {
            m_itemId = itemId;
            m_itemCount = count;
        }

        void ClearItem() {
            m_itemId.clear();
            m_itemCount = 0;
        }

        const std::string& GetItemId() const { return m_itemId; }
        int GetItemCount() const { return m_itemCount; }

        void SetOnItemClick(std::function<void()> callback) { m_onItemClick = callback; }
        void SetOnItemDrop(std::function<void(const std::string&)> callback) { m_onItemDrop = callback; }

        void Update(float deltaTime) override;
        void Render() override;
        bool HandleInput(const UIEvent& event) override;

    private:
        std::string m_itemId;
        int m_itemCount = 0;
        std::function<void()> m_onItemClick;
        std::function<void(const std::string&)> m_onItemDrop;
        bool m_isDragging = false;
    };

    /**
     * @class UIHotbar
     * @brief Minecraft-style hotbar
     */
    class UIHotbar : public UIElement {
    public:
        UIHotbar(const std::string& id);
        ~UIHotbar() override = default;

        void SetSlotCount(int count) { m_slotCount = count; m_slots.resize(count); }
        int GetSlotCount() const { return m_slotCount; }

        void SetSelectedSlot(int slot) { m_selectedSlot = slot; }
        int GetSelectedSlot() const { return m_selectedSlot; }

        void SetSlotItem(int slot, const std::string& itemId, int count = 1);
        void ClearSlotItem(int slot);
        std::pair<std::string, int> GetSlotItem(int slot) const;

        void SetOnSlotSelected(std::function<void(int)> callback) { m_onSlotSelected = callback; }

        void Update(float deltaTime) override;
        void Render() override;
        bool HandleInput(const UIEvent& event) override;

    private:
        int m_slotCount = 9;
        int m_selectedSlot = 0;
        std::vector<std::pair<std::string, int>> m_slots;
        std::function<void(int)> m_onSlotSelected;
    };

    /**
     * @class UICrosshair
     * @brief Game crosshair widget
     */
    class UICrosshair : public UIElement {
    public:
        UICrosshair(const std::string& id);
        ~UICrosshair() override = default;

        void SetStyle(CrosshairStyle style) { m_style = style; }
        CrosshairStyle GetStyle() const { return m_style; }

        void SetColor(const Vec4& color) { m_color = color; }
        void SetSize(float size) { m_size = size; }

        void Update(float deltaTime) override;
        void Render() override;

    private:
        enum class CrosshairStyle {
            CROSS,
            DOT,
            CIRCLE,
            CUSTOM
        };

        CrosshairStyle m_style = CrosshairStyle::CROSS;
        Vec4 m_color = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
        float m_size = 16.0f;
    };

    /**
     * @class UIStatusBar
     * @brief Health, hunger, etc. bars
     */
    class UIStatusBar : public UIElement {
    public:
        UIStatusBar(const std::string& id);
        ~UIStatusBar() override = default;

        void SetValue(float value) { m_value = std::max(0.0f, std::min(m_maxValue, value)); }
        float GetValue() const { return m_value; }

        void SetMaxValue(float maxValue) { m_maxValue = maxValue; }
        float GetMaxValue() const { return m_maxValue; }

        void SetBarType(StatusBarType type) { m_type = type; }
        StatusBarType GetBarType() const { return m_type; }

        void SetIcon(const std::string& iconPath) { m_iconPath = iconPath; }

        void Update(float deltaTime) override;
        void Render() override;

    private:
        enum class StatusBarType {
            HEALTH,
            HUNGER,
            EXPERIENCE,
            ARMOR,
            BREATH,
            CUSTOM
        };

        float m_value = 20.0f;
        float m_maxValue = 20.0f;
        StatusBarType m_type = StatusBarType::HEALTH;
        std::string m_iconPath;
        float m_animationValue = 0.0f;
    };

    /**
     * @class UIMenu
     * @brief Dropdown or context menu
     */
    class UIMenu : public UIElement {
    public:
        UIMenu(const std::string& id);
        ~UIMenu() override = default;

        void AddItem(const std::string& itemId, const std::string& text);
        void RemoveItem(const std::string& itemId);
        void ClearItems();

        void SetOnItemSelected(std::function<void(const std::string&)> callback) {
            m_onItemSelected = callback;
        }

        void Show() { SetVisible(true); }
        void Hide() { SetVisible(false); }
        void Toggle() { SetVisible(!IsVisible()); }

        void Update(float deltaTime) override;
        void Render() override;
        bool HandleInput(const UIEvent& event) override;

    private:
        struct MenuItem {
            std::string id;
            std::string text;
            bool enabled = true;
            std::string iconPath;
        };

        std::vector<MenuItem> m_items;
        int m_hoveredIndex = -1;
        std::function<void(const std::string&)> m_onItemSelected;
    };

    /**
     * @class UIModal
     * @brief Modal dialog widget
     */
    class UIModal : public UIElement {
    public:
        UIModal(const std::string& id);
        ~UIModal() override = default;

        void SetTitle(const std::string& title) { m_title = title; }
        void SetContent(const std::string& content) { m_content = content; }

        void AddButton(const std::string& buttonId, const std::string& text,
                      std::function<void()> callback);

        void SetOnClose(std::function<void()> callback) { m_onClose = callback; }

        void Show() { SetVisible(true); }
        void Hide() { SetVisible(false); }

        void Update(float deltaTime) override;
        void Render() override;
        bool HandleInput(const UIEvent& event) override;

    private:
        struct ModalButton {
            std::string id;
            std::string text;
            std::function<void()> callback;
        };

        std::string m_title;
        std::string m_content;
        std::vector<ModalButton> m_buttons;
        std::function<void()> m_onClose;
    };

    /**
     * @class UIChatWindow
     * @brief Minecraft-style chat window
     */
    class UIChatWindow : public UIElement {
    public:
        UIChatWindow(const std::string& id);
        ~UIChatWindow() override = default;

        void AddMessage(const std::string& message, const std::string& sender = "");
        void ClearMessages();
        void SetMaxMessages(int maxMessages) { m_maxMessages = maxMessages; }

        void SetOnMessageSend(std::function<void(const std::string&)> callback) {
            m_onMessageSend = callback;
        }

        void OpenInput() { m_inputOpen = true; }
        void CloseInput() { m_inputOpen = false; }

        void Update(float deltaTime) override;
        void Render() override;
        bool HandleInput(const UIEvent& event) override;

    private:
        struct ChatMessage {
            std::string message;
            std::string sender;
            float timestamp;
            Vec4 color;
        };

        std::vector<ChatMessage> m_messages;
        std::string m_inputText;
        int m_maxMessages = 100;
        bool m_inputOpen = false;
        float m_fadeTime = 10.0f;
        std::function<void(const std::string&)> m_onMessageSend;
    };

    /**
     * @class UIDebugPanel
     * @brief Debug information panel
     */
    class UIDebugPanel : public UIElement {
    public:
        UIDebugPanel(const std::string& id);
        ~UIDebugPanel() override = default;

        void AddDebugInfo(const std::string& category, const std::string& info);
        void ClearDebugInfo(const std::string& category);
        void SetVisible(bool visible);

        void Update(float deltaTime) override;
        void Render() override;

    private:
        std::unordered_map<std::string, std::vector<std::string>> m_debugInfo;
        float m_updateTimer = 0.0f;
        bool m_autoUpdate = true;
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_UI_UI_WIDGETS_HPP
