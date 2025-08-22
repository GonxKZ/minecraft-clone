/**
 * @file UIWidget.hpp
 * @brief VoxelCraft UI Widget System - Specialized UI Components
 * @version 1.0.0
 * @author VoxelCraft Team
 *
 * This file defines the UIWidget class and various specialized UI widgets
 * that provide common UI functionality for the VoxelCraft game engine,
 * including buttons, sliders, text boxes, progress bars, and other interactive
 * elements with advanced styling and behavior capabilities.
 */

#ifndef VOXELCRAFT_UI_UI_WIDGET_HPP
#define VOXELCRAFT_UI_UI_WIDGET_HPP

#include <memory>
#include <string>
#include <functional>
#include <vector>
#include <unordered_map>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "UIElement.hpp"
#include "../core/Config.hpp"

namespace VoxelCraft {

    // Forward declarations
    class UIManager;
    class UIStyle;
    struct UIEvent;

    /**
     * @class UIWidget
     * @brief Base class for specialized UI widgets
     *
     * The UIWidget class extends UIElement to provide specialized UI components
     * with advanced functionality and behavior. It serves as the base class for
     * buttons, sliders, text inputs, and other interactive UI elements.
     *
     * Key Features:
     * - Interactive behavior with user input handling
     * - Advanced styling and theming support
     * - Animation and transition effects
     * - Focus management and keyboard navigation
     * - Accessibility features and screen reader support
     * - Event-driven programming model
     * - Data binding and validation
     * - Drag and drop support
     * - Customizable appearance and behavior
     */
    class UIWidget : public UIElement {
    public:
        /**
         * @brief Constructor
         * @param config Widget configuration
         */
        explicit UIWidget(const UIElementConfig& config);

        /**
         * @brief Virtual destructor
         */
        virtual ~UIWidget();

        // Widget-specific functionality

        /**
         * @brief Get widget type
         * @return Widget type as string
         */
        virtual std::string GetWidgetType() const = 0;

        /**
         * @brief Check if widget is interactive
         * @return true if interactive, false otherwise
         */
        virtual bool IsInteractive() const { return true; }

        /**
         * @brief Get widget value
         * @return Widget value as string
         */
        virtual std::string GetValue() const { return ""; }

        /**
         * @brief Set widget value
         * @param value New value
         * @return true if successful, false otherwise
         */
        virtual bool SetValue(const std::string& value) { return false; }

        /**
         * @brief Enable widget
         */
        virtual void Enable();

        /**
         * @brief Disable widget
         */
        virtual void Disable();

        /**
         * @brief Check if widget is enabled
         * @return true if enabled, false otherwise
         */
        bool IsWidgetEnabled() const { return m_enabled; }

        // Focus and navigation

        /**
         * @brief Check if widget can receive focus
         * @return true if focusable, false otherwise
         */
        virtual bool CanReceiveFocus() const { return m_focusable; }

        /**
         * @brief Get next focusable widget
         * @return Next widget ID or 0 if none
         */
        virtual uint32_t GetNextFocusableWidget() const { return 0; }

        /**
         * @brief Get previous focusable widget
         * @return Previous widget ID or 0 if none
         */
        virtual uint32_t GetPreviousFocusableWidget() const { return 0; }

        // Styling

        /**
         * @brief Apply widget-specific style
         * @param style Style to apply
         */
        virtual void ApplyWidgetStyle(const UIStyle& style);

        /**
         * @brief Get widget style properties
         * @return Style properties
         */
        virtual std::unordered_map<std::string, std::any> GetStyleProperties() const;

        // Animation

        /**
         * @brief Play widget-specific animation
         * @param animationName Animation name
         * @param duration Animation duration
         * @return true if successful, false otherwise
         */
        virtual bool PlayAnimation(const std::string& animationName, float duration);

        // Data binding

        /**
         * @brief Bind widget to data source
         * @param dataSource Data source
         * @param propertyName Property name
         * @return true if successful, false otherwise
         */
        virtual bool BindToDataSource(void* dataSource, const std::string& propertyName);

        /**
         * @brief Update widget from bound data
         */
        virtual void UpdateFromBoundData();

        /**
         * @brief Update bound data from widget
         */
        virtual void UpdateBoundData();

        // Validation

        /**
         * @brief Validate widget input
         * @param input Input to validate
         * @return true if valid, false otherwise
         */
        virtual bool ValidateInput(const std::string& input) const { return true; }

        /**
         * @brief Get validation error message
         * @return Error message
         */
        virtual std::string GetValidationError() const { return ""; }

        // Drag and drop

        /**
         * @brief Check if widget accepts drag and drop
         * @return true if accepts, false otherwise
         */
        virtual bool AcceptsDragAndDrop() const { return false; }

        /**
         * @brief Handle drag operation
         * @param dragData Drag data
         * @return true if accepted, false otherwise
         */
        virtual bool HandleDrag(const std::unordered_map<std::string, std::any>& dragData);

        /**
         * @brief Handle drop operation
         * @param dropData Drop data
         * @return true if accepted, false otherwise
         */
        virtual bool HandleDrop(const std::unordered_map<std::string, std::any>& dropData);

        // Accessibility

        /**
         * @brief Get accessibility label
         * @return Accessibility label
         */
        virtual std::string GetAccessibilityLabel() const;

        /**
         * @brief Get accessibility description
         * @return Accessibility description
         */
        virtual std::string GetAccessibilityDescription() const;

        /**
         * @brief Check if widget is accessible
         * @return true if accessible, false otherwise
         */
        virtual bool IsAccessible() const { return true; }

    protected:
        /**
         * @brief Handle widget-specific mouse input
         * @param localPos Local mouse position
         * @param button Mouse button
         * @param action Button action
         * @param mods Modifier keys
         * @return true if handled, false otherwise
         */
        virtual bool HandleWidgetMouseInput(const glm::vec2& localPos, int button, int action, int mods);

        /**
         * @brief Handle widget-specific keyboard input
         * @param key Key code
         * @param scancode Scan code
         * @param action Key action
         * @param mods Modifier keys
         * @return true if handled, false otherwise
         */
        virtual bool HandleWidgetKeyboardInput(int key, int scancode, int action, int mods);

        /**
         * @brief Handle widget-specific text input
         * @param text Input text
         * @return true if handled, false otherwise
         */
        virtual bool HandleWidgetTextInput(const std::string& text);

        /**
         * @brief Render widget-specific content
         */
        virtual void RenderWidgetContent();

        /**
         * @brief Update widget-specific logic
         * @param deltaTime Time elapsed
         */
        virtual void UpdateWidget(double deltaTime);

        /**
         * @brief Handle widget state change
         * @param oldState Previous state
         * @param newState New state
         */
        virtual void OnWidgetStateChanged(UIElementState oldState, UIElementState newState);

        // Widget-specific properties
        bool m_enabled;                           ///< Widget enabled state
        bool m_focusable;                         ///< Widget can receive focus
        std::string m_widgetValue;                ///< Widget value
        std::unordered_map<std::string, std::any> m_widgetProperties; ///< Widget properties

        // Data binding
        void* m_dataSource;                       ///< Bound data source
        std::string m_boundProperty;              ///< Bound property name
        std::function<void()> m_dataChangedCallback; ///< Data changed callback

        // Validation
        std::string m_validationError;            ///< Validation error message
        std::function<bool(const std::string&)> m_validator; ///< Input validator

        // Drag and drop
        bool m_isDragging;                        ///< Widget is being dragged
        std::unordered_map<std::string, std::any> m_dragData; ///< Drag data
    };

    // =============================================================================
    // BUTTON WIDGET
    // =============================================================================

    /**
     * @struct ButtonConfig
     * @brief Button widget configuration
     */
    struct ButtonConfig {
        std::string text;                         ///< Button text
        std::string icon;                         ///< Button icon texture
        std::function<void()> onClick;            ///< Click callback
        std::function<void()> onPress;            ///< Press callback
        std::function<void()> onRelease;          ///< Release callback
        glm::vec4 normalColor;                    ///< Normal state color
        glm::vec4 hoverColor;                     ///< Hover state color
        glm::vec4 pressedColor;                   ///< Pressed state color
        glm::vec4 disabledColor;                  ///< Disabled state color
        bool toggleButton;                        ///< Button is toggle button
        bool isToggled;                           ///< Toggle state
    };

    /**
     * @class UIButton
     * @brief Interactive button widget
     *
     * The UIButton class provides a fully interactive button widget with
     * click handling, toggle functionality, visual feedback, and accessibility
     * support for the VoxelCraft UI system.
     */
    class UIButton : public UIWidget {
    public:
        /**
         * @brief Constructor
         * @param config Button configuration
         */
        explicit UIButton(const UIElementConfig& elementConfig, const ButtonConfig& buttonConfig);

        /**
         * @brief Get widget type
         * @return "Button"
         */
        std::string GetWidgetType() const override { return "Button"; }

        /**
         * @brief Set button text
         * @param text New text
         */
        void SetButtonText(const std::string& text);

        /**
         * @brief Get button text
         * @return Current text
         */
        const std::string& GetButtonText() const { return m_buttonConfig.text; }

        /**
         * @brief Set click callback
         * @param callback Click callback
         */
        void SetClickCallback(std::function<void()> callback);

        /**
         * @brief Click the button programmatically
         */
        void Click();

        /**
         * @brief Check if button is toggle button
         * @return true if toggle button, false otherwise
         */
        bool IsToggleButton() const { return m_buttonConfig.toggleButton; }

        /**
         * @brief Check if button is toggled
         * @return true if toggled, false otherwise
         */
        bool IsToggled() const { return m_buttonConfig.isToggled; }

        /**
         * @brief Set toggle state
         * @param toggled New toggle state
         */
        void SetToggled(bool toggled);

        /**
         * @brief Toggle button state
         */
        void Toggle();

        /**
         * @brief Get button configuration
         * @return Button configuration
         */
        const ButtonConfig& GetButtonConfig() const { return m_buttonConfig; }

        /**
         * @brief Set button configuration
         * @param config New configuration
         */
        void SetButtonConfig(const ButtonConfig& config);

    protected:
        /**
         * @brief Handle widget mouse input
         * @param localPos Local mouse position
         * @param button Mouse button
         * @param action Button action
         * @param mods Modifier keys
         * @return true if handled, false otherwise
         */
        bool HandleWidgetMouseInput(const glm::vec2& localPos, int button, int action, int mods) override;

        /**
         * @brief Handle widget keyboard input
         * @param key Key code
         * @param scancode Scan code
         * @param action Key action
         * @param mods Modifier keys
         * @return true if handled, false otherwise
         */
        bool HandleWidgetKeyboardInput(int key, int scancode, int action, int mods) override;

        /**
         * @brief Render widget content
         */
        void RenderWidgetContent() override;

        /**
         * @brief Update widget
         * @param deltaTime Time elapsed
         */
        void UpdateWidget(double deltaTime) override;

        /**
         * @brief Handle widget state change
         * @param oldState Previous state
         * @param newState New state
         */
        void OnWidgetStateChanged(UIElementState oldState, UIElementState newState) override;

    private:
        /**
         * @brief Handle button click
         */
        void HandleClick();

        /**
         * @brief Handle button press
         */
        void HandlePress();

        /**
         * @brief Handle button release
         */
        void HandleRelease();

        /**
         * @brief Update button appearance based on state
         */
        void UpdateButtonAppearance();

        ButtonConfig m_buttonConfig;              ///< Button configuration
        bool m_isPressed;                         ///< Button is currently pressed
        double m_pressTime;                       ///< Button press time
    };

    // =============================================================================
    // TEXT BOX WIDGET
    // =============================================================================

    /**
     * @struct TextBoxConfig
     * @brief Text box widget configuration
     */
    struct TextBoxConfig {
        std::string placeholderText;              ///< Placeholder text
        std::string defaultText;                  ///< Default text
        int maxLength;                            ///< Maximum text length
        bool multiline;                           ///< Allow multiline text
        bool passwordMode;                        ///< Password input mode
        bool readOnly;                            ///< Read-only mode
        bool autoComplete;                        ///< Enable auto-complete
        std::vector<std::string> autoCompleteList; ///< Auto-complete suggestions
        std::function<bool(const std::string&)> validator; ///< Text validator
        std::function<void(const std::string&)> onTextChanged; ///< Text changed callback
        std::function<void(const std::string&)> onEnterPressed; ///< Enter pressed callback
    };

    /**
     * @class UITextBox
     * @brief Text input widget
     *
     * The UITextBox class provides a text input widget with advanced features
     * including validation, auto-complete, multiline support, and password mode
     * for the VoxelCraft UI system.
     */
    class UITextBox : public UIWidget {
    public:
        /**
         * @brief Constructor
         * @param config Element configuration
         * @param textBoxConfig Text box configuration
         */
        explicit UITextBox(const UIElementConfig& elementConfig, const TextBoxConfig& textBoxConfig);

        /**
         * @brief Get widget type
         * @return "TextBox"
         */
        std::string GetWidgetType() const override { return "TextBox"; }

        /**
         * @brief Get text box text
         * @return Current text
         */
        std::string GetText() const override { return m_text; }

        /**
         * @brief Set text box text
         * @param text New text
         * @return true if successful, false otherwise
         */
        bool SetText(const std::string& text);

        /**
         * @brief Set placeholder text
         * @param placeholder New placeholder text
         */
        void SetPlaceholderText(const std::string& placeholder);

        /**
         * @brief Clear text
         */
        void ClearText();

        /**
         * @brief Select all text
         */
        void SelectAll();

        /**
         * @brief Get selection range
         * @return Selection range (start, end)
         */
        std::pair<int, int> GetSelection() const { return m_selection; }

        /**
         * @brief Set selection range
         * @param start Selection start
         * @param end Selection end
         */
        void SetSelection(int start, int end);

        /**
         * @brief Get cursor position
         * @return Cursor position
         */
        int GetCursorPosition() const { return m_cursorPosition; }

        /**
         * @brief Set cursor position
         * @param position New cursor position
         */
        void SetCursorPosition(int position);

        /**
         * @brief Check if text is valid
         * @return true if valid, false otherwise
         */
        bool IsTextValid() const;

        /**
         * @brief Get validation error
         * @return Validation error message
         */
        std::string GetValidationError() const override { return m_validationError; }

    protected:
        /**
         * @brief Handle widget mouse input
         * @param localPos Local mouse position
         * @param button Mouse button
         * @param action Button action
         * @param mods Modifier keys
         * @return true if handled, false otherwise
         */
        bool HandleWidgetMouseInput(const glm::vec2& localPos, int button, int action, int mods) override;

        /**
         * @brief Handle widget keyboard input
         * @param key Key code
         * @param scancode Scan code
         * @param action Key action
         * @param mods Modifier keys
         * @return true if handled, false otherwise
         */
        bool HandleWidgetKeyboardInput(int key, int scancode, int action, int mods) override;

        /**
         * @brief Handle widget text input
         * @param text Input text
         * @return true if handled, false otherwise
         */
        bool HandleWidgetTextInput(const std::string& text) override;

        /**
         * @brief Render widget content
         */
        void RenderWidgetContent() override;

        /**
         * @brief Update widget
         * @param deltaTime Time elapsed
         */
        void UpdateWidget(double deltaTime) override;

        /**
         * @brief Validate input
         * @param input Input to validate
         * @return true if valid, false otherwise
         */
        bool ValidateInput(const std::string& input) const override;

    private:
        /**
         * @brief Insert text at cursor position
         * @param text Text to insert
         */
        void InsertText(const std::string& text);

        /**
         * @brief Delete character at cursor
         */
        void DeleteCharacter();

        /**
         * @brief Delete character before cursor
         */
        void BackspaceCharacter();

        /**
         * @brief Update cursor blink
         * @param deltaTime Time elapsed
         */
        void UpdateCursorBlink(double deltaTime);

        /**
         * @brief Update auto-complete
         */
        void UpdateAutoComplete();

        /**
         * @brief Get character index at position
         * @param position Screen position
         * @return Character index
         */
        int GetCharacterIndexAtPosition(const glm::vec2& position) const;

        TextBoxConfig m_textBoxConfig;            ///< Text box configuration
        std::string m_text;                       ///< Current text
        std::string m_placeholderText;            ///< Placeholder text
        int m_cursorPosition;                     ///< Cursor position
        std::pair<int, int> m_selection;          ///< Text selection (start, end)
        double m_cursorBlinkTime;                 ///< Cursor blink timer
        bool m_cursorVisible;                     ///< Cursor visibility
        bool m_showPlaceholder;                   ///< Show placeholder text
        std::vector<std::string> m_autoCompleteSuggestions; ///< Auto-complete suggestions
        int m_selectedSuggestion;                 ///< Selected suggestion index
    };

    // =============================================================================
    // SLIDER WIDGET
    // =============================================================================

    /**
     * @struct SliderConfig
     * @brief Slider widget configuration
     */
    struct SliderConfig {
        float minValue;                           ///< Minimum value
        float maxValue;                           ///< Maximum value
        float currentValue;                       ///< Current value
        float stepSize;                           ///< Step size for value changes
        bool vertical;                            ///< Vertical slider
        bool showValue;                           ///< Show current value
        std::string valueFormat;                  ///< Value display format
        std::function<void(float)> onValueChanged; ///< Value changed callback
        glm::vec4 trackColor;                     ///< Track color
        glm::vec4 thumbColor;                     ///< Thumb color
        glm::vec4 valueTextColor;                 ///< Value text color
    };

    /**
     * @class UISlider
     * @brief Slider widget for value selection
     *
     * The UISlider class provides a slider widget for selecting values within
     * a range, with support for horizontal/vertical orientation, step values,
     * and visual feedback for the VoxelCraft UI system.
     */
    class UISlider : public UIWidget {
    public:
        /**
         * @brief Constructor
         * @param config Element configuration
         * @param sliderConfig Slider configuration
         */
        explicit UISlider(const UIElementConfig& elementConfig, const SliderConfig& sliderConfig);

        /**
         * @brief Get widget type
         * @return "Slider"
         */
        std::string GetWidgetType() const override { return "Slider"; }

        /**
         * @brief Get slider value
         * @return Current value
         */
        float GetValue() const { return m_sliderConfig.currentValue; }

        /**
         * @brief Set slider value
         * @param value New value
         * @return true if successful, false otherwise
         */
        bool SetValue(float value);

        /**
         * @brief Set value range
         * @param minValue Minimum value
         * @param maxValue Maximum value
         */
        void SetRange(float minValue, float maxValue);

        /**
         * @brief Get normalized value (0.0 - 1.0)
         * @return Normalized value
         */
        float GetNormalizedValue() const;

        /**
         * @brief Set normalized value (0.0 - 1.0)
         * @param normalizedValue Normalized value
         * @return true if successful, false otherwise
         */
        bool SetNormalizedValue(float normalizedValue);

        /**
         * @brief Get slider configuration
         * @return Slider configuration
         */
        const SliderConfig& GetSliderConfig() const { return m_sliderConfig; }

        /**
         * @brief Set slider configuration
         * @param config New configuration
         */
        void SetSliderConfig(const SliderConfig& config);

    protected:
        /**
         * @brief Handle widget mouse input
         * @param localPos Local mouse position
         * @param button Mouse button
         * @param action Button action
         * @param mods Modifier keys
         * @return true if handled, false otherwise
         */
        bool HandleWidgetMouseInput(const glm::vec2& localPos, int button, int action, int mods) override;

        /**
         * @brief Handle widget keyboard input
         * @param key Key code
         * @param scancode Scan code
         * @param action Key action
         * @param mods Modifier keys
         * @return true if handled, false otherwise
         */
        bool HandleWidgetKeyboardInput(int key, int scancode, int action, int mods) override;

        /**
         * @brief Render widget content
         */
        void RenderWidgetContent() override;

        /**
         * @brief Update widget
         * @param deltaTime Time elapsed
         */
        void UpdateWidget(double deltaTime) override;

    private:
        /**
         * @brief Update thumb position based on value
         */
        void UpdateThumbPosition();

        /**
         * @brief Get value from position
         * @param position Local position
         * @return Value at position
         */
        float GetValueFromPosition(const glm::vec2& position) const;

        /**
         * @brief Handle value change
         * @param newValue New value
         */
        void HandleValueChange(float newValue);

        SliderConfig m_sliderConfig;              ///< Slider configuration
        UIRect m_thumbRect;                       ///< Thumb rectangle
        bool m_isDragging;                        ///< Thumb is being dragged
        glm::vec2 m_dragStartPos;                 ///< Drag start position
        float m_dragStartValue;                   ///< Drag start value
    };

    // =============================================================================
    // PROGRESS BAR WIDGET
    // =============================================================================

    /**
     * @struct ProgressBarConfig
     * @brief Progress bar widget configuration
     */
    struct ProgressBarConfig {
        float minValue;                           ///< Minimum value
        float maxValue;                           ///< Maximum value
        float currentValue;                       ///< Current value
        bool showText;                            ///< Show progress text
        std::string textFormat;                   ///< Text display format
        bool vertical;                            ///< Vertical progress bar
        bool indeterminate;                       ///< Indeterminate progress
        glm::vec4 backgroundColor;                ///< Background color
        glm::vec4 fillColor;                      ///< Fill color
        glm::vec4 textColor;                      ///< Text color
        float animationSpeed;                     ///< Animation speed
    };

    /**
     * @class UIProgressBar
     * @brief Progress bar widget
     *
     * The UIProgressBar class provides a progress bar widget for displaying
     * progress or loading states with smooth animations and customizable
     * appearance for the VoxelCraft UI system.
     */
    class UIProgressBar : public UIWidget {
    public:
        /**
         * @brief Constructor
         * @param config Element configuration
         * @param progressConfig Progress bar configuration
         */
        explicit UIProgressBar(const UIElementConfig& elementConfig, const ProgressBarConfig& progressConfig);

        /**
         * @brief Get widget type
         * @return "ProgressBar"
         */
        std::string GetWidgetType() const override { return "ProgressBar"; }

        /**
         * @brief Get progress value
         * @return Current value
         */
        float GetProgressValue() const { return m_progressConfig.currentValue; }

        /**
         * @brief Set progress value
         * @param value New value
         * @return true if successful, false otherwise
         */
        bool SetProgressValue(float value);

        /**
         * @brief Set progress range
         * @param minValue Minimum value
         * @param maxValue Maximum value
         */
        void SetProgressRange(float minValue, float maxValue);

        /**
         * @brief Get normalized progress (0.0 - 1.0)
         * @return Normalized progress
         */
        float GetNormalizedProgress() const;

        /**
         * @brief Check if progress is complete
         * @return true if complete, false otherwise
         */
        bool IsComplete() const;

        /**
         * @brief Set indeterminate mode
         * @param indeterminate New indeterminate state
         */
        void SetIndeterminate(bool indeterminate);

        /**
         * @brief Get progress bar configuration
         * @return Progress bar configuration
         */
        const ProgressBarConfig& GetProgressConfig() const { return m_progressConfig; }

        /**
         * @brief Set progress bar configuration
         * @param config New configuration
         */
        void SetProgressConfig(const ProgressBarConfig& config);

    protected:
        /**
         * @brief Render widget content
         */
        void RenderWidgetContent() override;

        /**
         * @brief Update widget
         * @param deltaTime Time elapsed
         */
        void UpdateWidget(double deltaTime) override;

    private:
        /**
         * @brief Update fill animation
         * @param deltaTime Time elapsed
         */
        void UpdateFillAnimation(double deltaTime);

        /**
         * @brief Update indeterminate animation
         * @param deltaTime Time elapsed
         */
        void UpdateIndeterminateAnimation(double deltaTime);

        /**
         * @brief Get progress text
         * @return Progress text
         */
        std::string GetProgressText() const;

        ProgressBarConfig m_progressConfig;       ///< Progress bar configuration
        float m_displayValue;                     ///< Currently displayed value
        float m_indeterminatePosition;            ///< Indeterminate animation position
    };

    // =============================================================================
    // CHECKBOX WIDGET
    // =============================================================================

    /**
     * @struct CheckBoxConfig
     * @brief Checkbox widget configuration
     */
    struct CheckBoxConfig {
        std::string label;                        ///< Checkbox label
        bool checked;                             ///< Checked state
        bool tristate;                            ///< Allow indeterminate state
        int checkState;                           ///< Current check state (0=unchecked, 1=checked, 2=indeterminate)
        std::function<void(bool)> onCheckChanged; ///< Check changed callback
        glm::vec4 checkColor;                     ///< Check mark color
        glm::vec4 backgroundColor;                ///< Background color
        glm::vec4 borderColor;                    ///< Border color
        float checkSize;                          ///< Check mark size
    };

    /**
     * @class UICheckBox
     * @brief Checkbox widget
     *
     * The UICheckBox class provides a checkbox widget with support for
     * checked/unchecked states, tri-state checkboxes, and visual feedback
     * for the VoxelCraft UI system.
     */
    class UICheckBox : public UIWidget {
    public:
        /**
         * @brief Constructor
         * @param config Element configuration
         * @param checkBoxConfig Checkbox configuration
         */
        explicit UICheckBox(const UIElementConfig& elementConfig, const CheckBoxConfig& checkBoxConfig);

        /**
         * @brief Get widget type
         * @return "CheckBox"
         */
        std::string GetWidgetType() const override { return "CheckBox"; }

        /**
         * @brief Check if checkbox is checked
         * @return true if checked, false otherwise
         */
        bool IsChecked() const { return m_checkBoxConfig.checkState == 1; }

        /**
         * @brief Set checked state
         * @param checked New checked state
         */
        void SetChecked(bool checked);

        /**
         * @brief Get check state
         * @return Check state (0=unchecked, 1=checked, 2=indeterminate)
         */
        int GetCheckState() const { return m_checkBoxConfig.checkState; }

        /**
         * @brief Set check state
         * @param state New check state
         */
        void SetCheckState(int state);

        /**
         * @brief Toggle checkbox state
         */
        void Toggle();

        /**
         * @brief Get checkbox label
         * @return Checkbox label
         */
        const std::string& GetLabel() const { return m_checkBoxConfig.label; }

        /**
         * @brief Set checkbox label
         * @param label New label
         */
        void SetLabel(const std::string& label);

        /**
         * @brief Check if tri-state checkbox
         * @return true if tri-state, false otherwise
         */
        bool IsTriState() const { return m_checkBoxConfig.tristate; }

        /**
         * @brief Get checkbox configuration
         * @return Checkbox configuration
         */
        const CheckBoxConfig& GetCheckBoxConfig() const { return m_checkBoxConfig; }

        /**
         * @brief Set checkbox configuration
         * @param config New configuration
         */
        void SetCheckBoxConfig(const CheckBoxConfig& config);

    protected:
        /**
         * @brief Handle widget mouse input
         * @param localPos Local mouse position
         * @param button Mouse button
         * @param action Button action
         * @param mods Modifier keys
         * @return true if handled, false otherwise
         */
        bool HandleWidgetMouseInput(const glm::vec2& localPos, int button, int action, int mods) override;

        /**
         * @brief Handle widget keyboard input
         * @param key Key code
         * @param scancode Scan code
         * @param action Key action
         * @param mods Modifier keys
         * @return true if handled, false otherwise
         */
        bool HandleWidgetKeyboardInput(int key, int scancode, int action, int mods) override;

        /**
         * @brief Render widget content
         */
        void RenderWidgetContent() override;

        /**
         * @brief Update widget
         * @param deltaTime Time elapsed
         */
        void UpdateWidget(double deltaTime) override;

    private:
        /**
         * @brief Handle checkbox click
         */
        void HandleClick();

        /**
         * @brief Update checkbox appearance
         */
        void UpdateCheckBoxAppearance();

        CheckBoxConfig m_checkBoxConfig;          ///< Checkbox configuration
        UIRect m_checkRect;                       ///< Check area rectangle
    };

    // =============================================================================
    // LIST BOX WIDGET
    // =============================================================================

    /**
     * @struct ListBoxItem
     * @brief List box item structure
     */
    struct ListBoxItem {
        std::string text;                         ///< Item text
        std::string icon;                         ///< Item icon
        std::any data;                            ///< Item data
        bool enabled;                             ///< Item is enabled
        bool selected;                            ///< Item is selected
        glm::vec4 textColor;                      ///< Text color
        glm::vec4 backgroundColor;                ///< Background color
    };

    /**
     * @struct ListBoxConfig
     * @brief List box widget configuration
     */
    struct ListBoxConfig {
        std::vector<ListBoxItem> items;           ///< List items
        int selectedIndex;                        ///< Selected item index
        bool multiSelect;                         ///< Allow multiple selection
        bool showScrollBar;                       ///< Show scroll bar
        bool showIcons;                           ///< Show item icons
        float itemHeight;                         ///< Height of each item
        int visibleItems;                         ///< Number of visible items
        std::function<void(int)> onSelectionChanged; ///< Selection changed callback
        std::function<void(int)> onItemDoubleClick; ///< Item double-click callback
        glm::vec4 itemColor;                      ///< Normal item color
        glm::vec4 selectedColor;                  ///< Selected item color
        glm::vec4 hoverColor;                     ///< Hover item color
        glm::vec4 disabledColor;                  ///< Disabled item color
    };

    /**
     * @class UIListBox
     * @brief List box widget
     *
     * The UIListBox class provides a list box widget for displaying and
     * selecting items from a list, with support for scrolling, icons,
     * and multiple selection modes for the VoxelCraft UI system.
     */
    class UIListBox : public UIWidget {
    public:
        /**
         * @brief Constructor
         * @param config Element configuration
         * @param listBoxConfig List box configuration
         */
        explicit UIListBox(const UIElementConfig& elementConfig, const ListBoxConfig& listBoxConfig);

        /**
         * @brief Get widget type
         * @return "ListBox"
         */
        std::string GetWidgetType() const override { return "ListBox"; }

        /**
         * @brief Add item to list
         * @param item Item to add
         * @return Item index
         */
        int AddItem(const ListBoxItem& item);

        /**
         * @brief Remove item from list
         * @param index Item index
         * @return true if removed, false otherwise
         */
        bool RemoveItem(int index);

        /**
         * @brief Clear all items
         */
        void ClearItems();

        /**
         * @brief Get item at index
         * @param index Item index
         * @return Item reference
         */
        ListBoxItem& GetItem(int index);

        /**
         * @brief Get selected item index
         * @return Selected item index or -1 if none
         */
        int GetSelectedIndex() const { return m_listBoxConfig.selectedIndex; }

        /**
         * @brief Set selected item index
         * @param index New selected index
         * @return true if successful, false otherwise
         */
        bool SetSelectedIndex(int index);

        /**
         * @brief Get selected item indices (for multi-select)
         * @return Vector of selected indices
         */
        std::vector<int> GetSelectedIndices() const;

        /**
         * @brief Get selected item
         * @return Selected item or nullptr if none
         */
        const ListBoxItem* GetSelectedItem() const;

        /**
         * @brief Get item count
         * @return Number of items
         */
        int GetItemCount() const { return static_cast<int>(m_listBoxConfig.items.size()); }

        /**
         * @brief Scroll to item
         * @param index Item index
         * @return true if successful, false otherwise
         */
        bool ScrollToItem(int index);

        /**
         * @brief Get scroll position
         * @return Scroll position
         */
        float GetScrollPosition() const { return m_scrollPosition; }

        /**
         * @brief Set scroll position
         * @param position New scroll position
         */
        void SetScrollPosition(float position);

        /**
         * @brief Get list box configuration
         * @return List box configuration
         */
        const ListBoxConfig& GetListBoxConfig() const { return m_listBoxConfig; }

        /**
         * @brief Set list box configuration
         * @param config New configuration
         */
        void SetListBoxConfig(const ListBoxConfig& config);

    protected:
        /**
         * @brief Handle widget mouse input
         * @param localPos Local mouse position
         * @param button Mouse button
         * @param action Button action
         * @param mods Modifier keys
         * @return true if handled, false otherwise
         */
        bool HandleWidgetMouseInput(const glm::vec2& localPos, int button, int action, int mods) override;

        /**
         * @brief Handle widget keyboard input
         * @param key Key code
         * @param scancode Scan code
         * @param action Key action
         * @param mods Modifier keys
         * @return true if handled, false otherwise
         */
        bool HandleWidgetKeyboardInput(int key, int scancode, int action, int mods) override;

        /**
         * @brief Render widget content
         */
        void RenderWidgetContent() override;

        /**
         * @brief Update widget
         * @param deltaTime Time elapsed
         */
        void UpdateWidget(double deltaTime) override;

    private:
        /**
         * @brief Get item at position
         * @param localPos Local position
         * @return Item index or -1 if none
         */
        int GetItemAtPosition(const glm::vec2& localPos) const;

        /**
         * @brief Handle item selection
         * @param index Item index
         */
        void HandleItemSelection(int index);

        /**
         * @brief Update scroll bar
         */
        void UpdateScrollBar();

        /**
         * @brief Clamp scroll position
         */
        void ClampScrollPosition();

        ListBoxConfig m_listBoxConfig;            ///< List box configuration
        float m_scrollPosition;                   ///< Current scroll position
        float m_maxScrollPosition;                ///< Maximum scroll position
        UIRect m_scrollBarRect;                   ///< Scroll bar rectangle
        UIRect m_scrollThumbRect;                 ///< Scroll thumb rectangle
        bool m_isScrolling;                       ///< User is scrolling
        double m_lastClickTime;                   ///< Last click time for double-click detection
        int m_lastClickedIndex;                   ///< Last clicked item index
    };

} // namespace VoxelCraft

#endif // VOXELCRAFT_UI_UI_WIDGET_HPP
