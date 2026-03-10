#pragma once

#include "GuiComponent.h"
#include "components/ButtonComponent.h"
#include "components/ComponentGrid.h"
#include "components/NinePatchComponent.h"
#include "components/TextComponent.h"
#include "components/TextEditComponent.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

class GuiTextEditKeyboardPopup : public GuiComponent
{
public:
    GuiTextEditKeyboardPopup(
        Window* window,
        const std::string& title,
        const std::string& initValue,
        const std::function<void(const std::string&)>& okCallback,
        bool multiLine = false);

    void onSizeChanged() override;
    bool input(InputConfig* config, Input input) override;
    void update(int deltaTime) override;
    std::vector<HelpPrompt> getHelpPrompts() override;

private:
    struct KeyboardButton
    {
        std::shared_ptr<ButtonComponent> button;
        std::string key;
        std::string shiftedKey;
    };

    void shiftKeys();
    void updateCaseIndicator();

    std::shared_ptr<ButtonComponent> makeButton(
        const std::string& key,
        const std::string& shiftedKey);

    NinePatchComponent mBackground;
    ComponentGrid mGrid;
    std::shared_ptr<ComponentGrid> mKeyboardGrid;

    std::shared_ptr<TextComponent> mTitle;
    std::shared_ptr<TextEditComponent> mText;
    std::shared_ptr<TextComponent> mCaseIndicator;
    std::shared_ptr<ButtonComponent> mShiftButton;

    std::function<void(const std::string&)> mOkCallback;

    bool mMultiLine;
    bool mShift;
    bool mDeleteRepeat;
    int mDeleteRepeatTimer;
    int mNavigationRepeatTimer;
    int mNavigationRepeatDirX;
    int mNavigationRepeatDirY;
    int mHorizontalKeyCount;

    std::string mInitValue;
    std::vector<KeyboardButton> mKeyboardButtons;
};
