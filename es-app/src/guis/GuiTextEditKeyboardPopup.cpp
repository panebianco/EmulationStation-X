#include "guis/GuiTextEditKeyboardPopup.h"

#include "Window.h"
#include "guis/GuiMsgBox.h"
#include "renderers/Renderer.h"
#include "resources/Font.h"

#include <SDL.h>

#define KEYBOARD_HEIGHT (Renderer::getScreenHeight() * 0.52f)
#define KEYBOARD_PADDINGX (Renderer::getScreenWidth() * 0.02f)
#define KEYBOARD_PADDINGY (Renderer::getScreenWidth() * 0.01f)

GuiTextEditKeyboardPopup::GuiTextEditKeyboardPopup(
    Window* window,
    const std::string& title,
    const std::string& initValue,
    const std::function<void(const std::string&)>& okCallback,
    bool multiLine)
    : GuiComponent(window)
    , mBackground(window, ":/frame_dark.png")
    , mGrid(window, Vector2i(1, 3))
    , mKeyboardGrid(nullptr)
    , mOkCallback(okCallback)
    , mMultiLine(multiLine)
    , mShift(false)
    , mDeleteRepeat(false)
    , mTextEditActive(false)
    , mDeleteRepeatTimer(0)
    , mNavigationRepeatTimer(0)
    , mNavigationRepeatDirX(0)
    , mNavigationRepeatDirY(0)
    , mHorizontalKeyCount(0)
    , mInitValue(initValue)
{
    addChild(&mBackground);
    addChild(&mGrid);

    mTitle = std::make_shared<TextComponent>(
        window,
        title,
        Font::get(FONT_SIZE_MEDIUM),
        0xFFFFFFFF,
        ALIGN_CENTER);

    mText = std::make_shared<TextEditComponent>(window);
    mText->setValue(initValue);
    mText->setSize(Renderer::getScreenWidth() * 0.60f,
                   Font::get(FONT_SIZE_MEDIUM)->getLetterHeight() * 1.8f);

    mCaseIndicator = std::make_shared<TextComponent>(
        window,
        "abc",
        Font::get(FONT_SIZE_SMALL),
        0xB0B0B0FF,
        ALIGN_RIGHT);

    updateCaseIndicator();

    mGrid.setEntry(mTitle, Vector2i(0, 0), false, true);
    mGrid.setEntry(mText, Vector2i(0, 1), true, false);

    addChild(mCaseIndicator.get());

    std::vector<std::vector<std::string>> kbBase {
        {"1","2","3","4","5","6","7","8","9","0","⌫"},
        {"!","@","#","$","%","&","/","(",")","=","⌫"},

        {"q","w","e","r","t","y","u","i","o","p","✓"},
        {"Q","W","E","R","T","Y","U","I","O","P","✓"},

        {"a","s","d","f","g","h","j","k","l","Ñ","-rowspan-"},
        {"A","S","D","F","G","H","J","K","L","Ñ","-rowspan-"},

        {"z","x","c","v","b","n","m",",",".","-","-rowspan-"},
        {"Z","X","C","V","B","N","M",";",";", "_","-rowspan-"}
    };

    mHorizontalKeyCount = 11;
    mKeyboardGrid = std::make_shared<ComponentGrid>(window, Vector2i(mHorizontalKeyCount, 5));

    for (int row = 0; row < 4; ++row)
    {
        for (int col = 0; col < (int)kbBase[row * 2].size(); ++col)
        {
            std::string lower = kbBase[row * 2][col];
            if (lower.empty() || lower == "-rowspan-" || lower == "-colspan-")
                continue;

            std::string upper = kbBase[row * 2 + 1][col];
            std::shared_ptr<ButtonComponent> button = makeButton(lower, upper);
            mKeyboardGrid->setEntry(button, Vector2i(col, row), true, true, Vector2i(1, 1));
        }
    }

    mShiftButton = std::make_shared<ButtonComponent>(
        window, "⇧", "",
        [this] { shiftKeys(); });

    auto spaceButton  = makeButton("␣", "␣");
    auto clearButton  = makeButton("clr", "clr");
    auto cancelButton = makeButton("✖", "✖");

    mKeyboardGrid->setEntry(mShiftButton, Vector2i(0, 4), true, true, Vector2i(2, 1));
    mKeyboardGrid->setEntry(spaceButton,  Vector2i(2, 4), true, true, Vector2i(4, 1));
    mKeyboardGrid->setEntry(clearButton,  Vector2i(6, 4), true, true, Vector2i(2, 1));
    mKeyboardGrid->setEntry(cancelButton, Vector2i(8, 4), true, true, Vector2i(3, 1));

    mGrid.setEntry(mKeyboardGrid, Vector2i(0, 2), true, true);

    float width = Renderer::getScreenWidth() * 0.78f;
    setSize(width, KEYBOARD_HEIGHT);
    setPosition((Renderer::getScreenWidth() - mSize.x()) / 2.0f,
                (Renderer::getScreenHeight() - mSize.y()) / 2.0f,
                0.0f);

    mText->setCursor((int)initValue.size());
}

void GuiTextEditKeyboardPopup::onSizeChanged()
{
    mBackground.fitTo(mSize);
    mGrid.setSize(mSize);

    mGrid.setRowHeightPerc(0, 0.10f);
    mGrid.setRowHeightPerc(1, 0.16f);
    mGrid.setRowHeightPerc(2, 0.74f);

    const float keyboardWidth = mSize.x() - KEYBOARD_PADDINGX * 2.0f;
    const float textHeight = Font::get(FONT_SIZE_MEDIUM)->getLetterHeight() * 1.8f;

    const float textWidth = mSize.x() * 0.70f;
    mText->setSize(textWidth, textHeight);

    Vector3f kbPos = mKeyboardGrid->getPosition();
    Vector2f kbSize = mKeyboardGrid->getSize();

    mKeyboardGrid->setSize(keyboardWidth, kbSize.y() - KEYBOARD_PADDINGY);
    mKeyboardGrid->setPosition(KEYBOARD_PADDINGX, kbPos.y(), 0.0f);

    const float indicatorWidth = Renderer::getScreenWidth() * 0.05f;
    const float indicatorHeight = Font::get(FONT_SIZE_SMALL)->getLetterHeight();
    const float indicatorPadding = Renderer::getScreenWidth() * 0.012f;

    Vector3f textPos = mText->getPosition();
    Vector2f textSize = mText->getSize();

    mCaseIndicator->setSize(indicatorWidth, indicatorHeight);
    mCaseIndicator->setPosition(
        textPos.x() + textSize.x() - indicatorWidth - indicatorPadding,
        textPos.y() + ((textSize.y() - indicatorHeight) * 0.5f),
        0.0f);
}

bool GuiTextEditKeyboardPopup::input(InputConfig* config, Input input)
{
    const bool fromKeyboard = (config->getDeviceId() == DEVICE_KEYBOARD);

    const bool keyboardBack =
        fromKeyboard && mTextEditActive && config->isMappedLike("b", input);

    if (fromKeyboard && input.value)
    {
        if (mTextEditActive && input.id == SDLK_ESCAPE)
        {
            mTextEditActive = false;
            mDeleteRepeatTimer = 0;
            SDL_StopTextInput();
            return true;
        }

        if (input.id == SDLK_RETURN || input.id == SDLK_KP_ENTER)
        {
            if (mTextEditActive)
            {
                mTextEditActive = false;
                mDeleteRepeatTimer = 0;
                SDL_StopTextInput();
                return true;
            }

            if (mGrid.getSelectedComponent() == mText)
            {
                mTextEditActive = true;
                mDeleteRepeatTimer = 0;
                SDL_StartTextInput();
                mText->setCursor((int)mText->getValue().size());
                return true;
            }
        }

        // Borrar con debounce para evitar doble evento
        if (mTextEditActive && (input.id == SDLK_BACKSPACE || input.id == SDLK_DELETE))
        {
            if (mDeleteRepeatTimer > 0)
                return true;

            std::string value = mText->getValue();
            if (!value.empty())
            {
                value.erase(value.size() - 1, 1);
                mText->setValue(value);
                mText->setCursor((int)value.size());
            }

            mDeleteRepeatTimer = 140;
            return true;
        }

        // Mientras se edita con teclado físico, no dejar pasar mappings del frontend
        if (mTextEditActive)
            return true;
    }

    if ((fromKeyboard && input.value && input.id == SDLK_ESCAPE && !mTextEditActive) ||
        (!keyboardBack && input.value && config->isMappedTo("b", input)))
    {
        if (mText->getValue() != mInitValue)
        {
            mWindow->pushGui(new GuiMsgBox(
                mWindow,
                "SAVE CHANGES?",
                "YES",
                [this]
                {
                    if (mTextEditActive)
                    {
                        mTextEditActive = false;
                        mDeleteRepeatTimer = 0;
                        SDL_StopTextInput();
                    }
                    mOkCallback(mText->getValue());
                    delete this;
                    return true;
                },
                "NO",
                [this]
                {
                    if (mTextEditActive)
                    {
                        mTextEditActive = false;
                        mDeleteRepeatTimer = 0;
                        SDL_StopTextInput();
                    }
                    delete this;
                    return true;
                }));
        }
        else
        {
            if (mTextEditActive)
            {
                mTextEditActive = false;
                mDeleteRepeatTimer = 0;
                SDL_StopTextInput();
            }
            delete this;
        }
        return true;
    }

    if (GuiComponent::input(config, input))
        return true;

    return false;
}

void GuiTextEditKeyboardPopup::textInput(const char* text)
{
    if (!mTextEditActive || text == nullptr || text[0] == '\0')
        return;

    mText->textInput(text);
}

void GuiTextEditKeyboardPopup::update(int deltaTime)
{
    GuiComponent::update(deltaTime);

    if (mDeleteRepeatTimer > 0)
    {
        mDeleteRepeatTimer -= deltaTime;
        if (mDeleteRepeatTimer < 0)
            mDeleteRepeatTimer = 0;
    }
}

std::vector<HelpPrompt> GuiTextEditKeyboardPopup::getHelpPrompts()
{
    return {};
}

void GuiTextEditKeyboardPopup::updateCaseIndicator()
{
    if (mCaseIndicator)
        mCaseIndicator->setText(mShift ? "ABC" : "abc");
}

void GuiTextEditKeyboardPopup::shiftKeys()
{
    mShift = !mShift;
    updateCaseIndicator();

    for (auto& kb : mKeyboardButtons)
    {
        Vector2f sz = kb.button->getSize();
        std::string display;

        if (kb.key == "✓" || kb.key == "⌫" || kb.key == "␣" || kb.key == "clr" || kb.key == "✖")
            display = kb.key;
        else
            display = mShift ? kb.shiftedKey : kb.key;

        kb.button->setText(display + " ", "");
        kb.button->setSize(sz.x(), sz.y());
    }
}

std::shared_ptr<ButtonComponent> GuiTextEditKeyboardPopup::makeButton(
    const std::string& key,
    const std::string& shiftedKey)
{
    std::shared_ptr<ButtonComponent> button = std::make_shared<ButtonComponent>(
        mWindow,
        key,
        "",
        [this, key, shiftedKey]
        {
            std::string value = mText->getValue();

            if (key == "✓")
            {
                if (mTextEditActive)
                {
                    mTextEditActive = false;
                    mDeleteRepeatTimer = 0;
                    SDL_StopTextInput();
                }

                mOkCallback(mText->getValue());
                delete this;
                return;
            }
            else if (key == "⌫")
            {
                if (!value.empty())
                {
                    value.erase(value.size() - 1, 1);
                    mText->setValue(value);
                    mText->setCursor((int)value.size());
                }
                return;
            }
            else if (key == "␣")
            {
                value += " ";
                mText->setValue(value);
                mText->setCursor((int)value.size());
                return;
            }
            else if (key == "clr")
            {
                mText->setValue("");
                mText->setCursor(0);
                return;
            }
            else if (key == "✖")
            {
                if (mTextEditActive)
                {
                    mTextEditActive = false;
                    mDeleteRepeatTimer = 0;
                    SDL_StopTextInput();
                }

                delete this;
                return;
            }

            value += (mShift ? shiftedKey : key);
            mText->setValue(value);
            mText->setCursor((int)value.size());
        });

    KeyboardButton kb { button, key, shiftedKey };
    mKeyboardButtons.push_back(kb);
    return button;
}
