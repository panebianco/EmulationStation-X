#include "guis/GuiTextEditKeyboardPopup.h"

#include "Window.h"
#include "Settings.h"
#include "guis/GuiMsgBox.h"
#include "renderers/Renderer.h"
#include "resources/Font.h"

#include <SDL.h>

#define KEYBOARD_HEIGHT   (Renderer::getScreenHeight() * 0.52f)
#define KEYBOARD_PADDINGX (Renderer::getScreenWidth() * 0.02f)
#define KEYBOARD_PADDINGY (Renderer::getScreenWidth() * 0.01f)

#define DELETE_REPEAT_START_DELAY 600
#define DELETE_REPEAT_SPEED 90

namespace
{
	inline bool isMenuDark()
	{
		return Settings::getInstance()->getBool("MenuDark");
	}

	inline unsigned int getPanelColor()
	{
		return isMenuDark() ? 0x101010F2 : 0xF4F4F4F2;
	}

	inline unsigned int getPanelBorderColor()
	{
		return isMenuDark() ? 0x909090FF : 0x707070FF;
	}

	inline unsigned int getTitleColor()
	{
		return isMenuDark() ? 0xFFFFFFFF : 0x222222FF;
	}

	inline unsigned int getCaseIndicatorColor()
	{
		return isMenuDark() ? 0xB0B0B0FF : 0x555555FF;
	}

	inline bool isPrintableKey(SDL_Keycode key)
	{
		// Letras
		if (key >= SDLK_a && key <= SDLK_z)
			return true;

		// Números superiores
		if (key >= SDLK_0 && key <= SDLK_9)
			return true;

		// Numpad
		if (key >= SDLK_KP_0 && key <= SDLK_KP_9)
			return true;

		switch (key)
		{
			case SDLK_SPACE:
			case SDLK_COMMA:
			case SDLK_PERIOD:
			case SDLK_SEMICOLON:
			case SDLK_COLON:
			case SDLK_MINUS:
			case SDLK_UNDERSCORE:
			case SDLK_PLUS:
			case SDLK_EQUALS:
			case SDLK_EXCLAIM:
			case SDLK_QUOTEDBL:
			case SDLK_HASH:
			case SDLK_DOLLAR:
			case SDLK_PERCENT:
			case SDLK_AMPERSAND:
			case SDLK_QUOTE:
			case SDLK_LEFTPAREN:
			case SDLK_RIGHTPAREN:
			case SDLK_ASTERISK:
			case SDLK_SLASH:
			case SDLK_BACKSLASH:
			case SDLK_QUESTION:
			case SDLK_AT:
			case SDLK_LEFTBRACKET:
			case SDLK_RIGHTBRACKET:
			case SDLK_BACKQUOTE:
			case SDLK_CARET:
			case SDLK_LESS:
			case SDLK_GREATER:
				return true;
			default:
				break;
		}

		return false;
	}
}

GuiTextEditKeyboardPopup::GuiTextEditKeyboardPopup(
	Window* window,
	const std::string& title,
	const std::string& initValue,
	const std::function<void(const std::string&)>& okCallback,
	bool multiLine)
	: GuiComponent(window)
	, mBackground(window, "")
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
	addChild(&mGrid);

	mTitle = std::make_shared<TextComponent>(
		window,
		title,
		Font::get(FONT_SIZE_MEDIUM),
		getTitleColor(),
		ALIGN_CENTER);

	mText = std::make_shared<TextEditComponent>(window);
	mText->setValue(initValue);
	mText->setSize(
		Renderer::getScreenWidth() * 0.60f,
		Font::get(FONT_SIZE_MEDIUM)->getLetterHeight() * 1.8f);

	mCaseIndicator = std::make_shared<TextComponent>(
		window,
		"abc",
		Font::get(FONT_SIZE_SMALL),
		getCaseIndicatorColor(),
		ALIGN_RIGHT);

	updateCaseIndicator();

	mGrid.setEntry(mTitle, Vector2i(0, 0), false, true);
	mGrid.setEntry(mText, Vector2i(0, 1), false, false);

	addChild(mCaseIndicator.get());

	std::vector<std::vector<std::string>> kbBase{
		{"1","2","3","4","5","6","7","8","9","0","⌫"},
		{"!","@","#","$","%","&","/","(",")","=","⌫"},

		{"q","w","e","r","t","y","u","i","o","p","✓"},
		{"Q","W","E","R","T","Y","U","I","O","P","✓"},

		{"a","s","d","f","g","h","j","k","l","ñ","-rowspan-"},
		{"A","S","D","F","G","H","J","K","L","Ñ","-rowspan-"},

		{"z","x","c","v","b","n","m",",",".","-","-rowspan-"},
		{"Z","X","C","V","B","N","M",";",":","_","-rowspan-"}
	};

	mHorizontalKeyCount = 11;
	mKeyboardGrid = std::make_shared<ComponentGrid>(window, Vector2i(mHorizontalKeyCount, 5));

	for (int row = 0; row < 4; ++row)
	{
		for (int col = 0; col < (int)kbBase[row * 2].size(); ++col)
		{
			const std::string lower = kbBase[row * 2][col];
			if (lower.empty() || lower == "-rowspan-" || lower == "-colspan-")
				continue;

			const std::string upper = kbBase[row * 2 + 1][col];
			std::shared_ptr<KeyboardKeyComponent> button = makeButton(lower, upper);
			mKeyboardGrid->setEntry(button, Vector2i(col, row), true, true, Vector2i(1, 1));
		}
	}

	mShiftButton = std::make_shared<KeyboardKeyComponent>(
		window,
		"⇧",
		[this] { shiftKeys(); });

	auto spaceButton  = makeButton("␣", "␣");
	auto clearButton  = makeButton("clr", "clr");
	auto cancelButton = makeButton("✖", "✖");

	mKeyboardGrid->setEntry(mShiftButton, Vector2i(0, 4), true, true, Vector2i(2, 1));
	mKeyboardGrid->setEntry(spaceButton,  Vector2i(2, 4), true, true, Vector2i(4, 1));
	mKeyboardGrid->setEntry(clearButton,  Vector2i(6, 4), true, true, Vector2i(2, 1));
	mKeyboardGrid->setEntry(cancelButton, Vector2i(8, 4), true, true, Vector2i(3, 1));

	mGrid.setEntry(mKeyboardGrid, Vector2i(0, 2), true, true);

	const float width = Renderer::getScreenWidth() * 0.78f;
	setSize(width, KEYBOARD_HEIGHT);
	setPosition(
		(Renderer::getScreenWidth() - mSize.x()) / 2.0f,
		(Renderer::getScreenHeight() - mSize.y()) / 2.0f,
		0.0f);

	mText->setCursor((int)initValue.size());
	mText->startEditing();
	mTextEditActive = true;

	mGrid.setCursorTo(mKeyboardGrid);
}

void GuiTextEditKeyboardPopup::acceptAndClose()
{
	if (mTextEditActive)
	{
		mText->stopEditing();
		mTextEditActive = false;
	}

	mDeleteRepeat = false;
	mDeleteRepeatTimer = 0;
	mOkCallback(mText->getValue());
	delete this;
}

void GuiTextEditKeyboardPopup::closeWithoutSaving()
{
	if (mTextEditActive)
	{
		mText->stopEditing();
		mTextEditActive = false;
	}

	mDeleteRepeat = false;
	mDeleteRepeatTimer = 0;
	delete this;
}

void GuiTextEditKeyboardPopup::deleteLastChar()
{
	mText->textInput("\b");
}

void GuiTextEditKeyboardPopup::onSizeChanged()
{
	mGrid.setSize(mSize);
	mGrid.setRowHeightPerc(0, 0.10f);
	mGrid.setRowHeightPerc(1, 0.16f);
	mGrid.setRowHeightPerc(2, 0.74f);

	const float keyboardWidth = mSize.x() - (KEYBOARD_PADDINGX * 2.0f);
	const float textHeight = Font::get(FONT_SIZE_MEDIUM)->getLetterHeight() * 1.8f;
	const float textWidth = mSize.x() * 0.70f;

	mText->setSize(textWidth, textHeight);

	const Vector3f kbPos = mKeyboardGrid->getPosition();
	const Vector2f kbSize = mKeyboardGrid->getSize();

	mKeyboardGrid->setSize(keyboardWidth, kbSize.y() - KEYBOARD_PADDINGY);
	mKeyboardGrid->setPosition(KEYBOARD_PADDINGX, kbPos.y(), 0.0f);

	const float indicatorWidth = Renderer::getScreenWidth() * 0.05f;
	const float indicatorHeight = Font::get(FONT_SIZE_SMALL)->getLetterHeight();
	const float indicatorPadding = Renderer::getScreenWidth() * 0.012f;

	const Vector3f textPos = mText->getPosition();
	const Vector2f textSize = mText->getSize();

	mCaseIndicator->setSize(indicatorWidth, indicatorHeight);
	mCaseIndicator->setPosition(
		textPos.x() + textSize.x() - indicatorWidth - indicatorPadding,
		textPos.y() + ((textSize.y() - indicatorHeight) * 0.5f),
		0.0f);
}

void GuiTextEditKeyboardPopup::render(const Transform4x4f& parentTrans)
{
	Transform4x4f trans = parentTrans * getTransform();
	Renderer::setMatrix(trans);

	const unsigned int panelColor = getPanelColor();
	const unsigned int borderColor = getPanelBorderColor();

	Renderer::drawRect(0.0f, 0.0f, mSize.x(), mSize.y(), panelColor, panelColor);
	Renderer::drawRect(0.0f, 0.0f, mSize.x(), 3.0f, borderColor, borderColor);
	Renderer::drawRect(0.0f, mSize.y() - 3.0f, mSize.x(), 3.0f, borderColor, borderColor);
	Renderer::drawRect(0.0f, 0.0f, 3.0f, mSize.y(), borderColor, borderColor);
	Renderer::drawRect(mSize.x() - 3.0f, 0.0f, 3.0f, mSize.y(), borderColor, borderColor);

	renderChildren(trans);
}

bool GuiTextEditKeyboardPopup::input(InputConfig* config, Input input)
{
	const bool fromKeyboard = (config->getDeviceId() == DEVICE_KEYBOARD);

	if (fromKeyboard && mTextEditActive)
	{
		if (input.value && (input.id == SDLK_RETURN || input.id == SDLK_KP_ENTER))
		{
			if (!mMultiLine)
			{
				acceptAndClose();
				return true;
			}
			else
			{
				mText->textInput("\n");
				return true;
			}
		}

		if (input.value && input.id == SDLK_ESCAPE)
		{
			mText->stopEditing();
			mTextEditActive = false;
			mDeleteRepeat = false;
			mDeleteRepeatTimer = 0;
			return true;
		}

		if (input.id == SDLK_BACKSPACE || input.id == SDLK_DELETE)
		{
			if (input.value)
			{
				mDeleteRepeat = true;
				mDeleteRepeatTimer = -(DELETE_REPEAT_START_DELAY - DELETE_REPEAT_SPEED);
				deleteLastChar();
			}
			else
			{
				mDeleteRepeat = false;
				mDeleteRepeatTimer = 0;
			}
			return true;
		}

		if (isPrintableKey((SDL_Keycode)input.id))
			return true;

		switch ((SDL_Keycode)input.id)
		{
			case SDLK_LSHIFT:
			case SDLK_RSHIFT:
			case SDLK_LCTRL:
			case SDLK_RCTRL:
			case SDLK_LALT:
			case SDLK_RALT:
			case SDLK_LGUI:
			case SDLK_RGUI:
			case SDLK_CAPSLOCK:
				return true;
			default:
				break;
		}
	}

	if (!fromKeyboard)
	{
		if (mGrid.getSelectedComponent() != mKeyboardGrid)
			mGrid.setCursorTo(mKeyboardGrid);

		if (config->isMappedTo("start", input) && input.value)
		{
			acceptAndClose();
			return true;
		}

		if (config->isMappedTo("x", input))
		{
			if (input.value)
			{
				mDeleteRepeat = true;
				mDeleteRepeatTimer = -(DELETE_REPEAT_START_DELAY - DELETE_REPEAT_SPEED);
				deleteLastChar();
			}
			else
			{
				mDeleteRepeat = false;
				mDeleteRepeatTimer = 0;
			}
			return true;
		}

		if (config->isMappedTo("y", input) && input.value)
		{
			mText->textInput(" ");
			return true;
		}
	}

	const bool keyboardBack = fromKeyboard && mTextEditActive && config->isMappedLike("b", input);

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
					acceptAndClose();
					return true;
				},
				"NO",
				[this]
				{
					closeWithoutSaving();
					return true;
				}));
		}
		else
		{
			closeWithoutSaving();
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

	if (mDeleteRepeat)
	{
		mDeleteRepeatTimer += deltaTime;

		while (mDeleteRepeatTimer >= DELETE_REPEAT_SPEED)
		{
			deleteLastChar();
			mDeleteRepeatTimer -= DELETE_REPEAT_SPEED;
		}
	}
	else if (mDeleteRepeatTimer > 0)
	{
		mDeleteRepeatTimer -= deltaTime;
		if (mDeleteRepeatTimer < 0)
			mDeleteRepeatTimer = 0;
	}
}

std::vector<HelpPrompt> GuiTextEditKeyboardPopup::getHelpPrompts()
{
	std::vector<HelpPrompt> prompts;
	prompts.push_back(HelpPrompt("a", "ELEGIR"));
	prompts.push_back(HelpPrompt("b", "ATRAS"));
	prompts.push_back(HelpPrompt("x", "BORRAR"));
	prompts.push_back(HelpPrompt("y", "ESPACIO"));
	prompts.push_back(HelpPrompt("start", "APLICAR"));
	return prompts;
}

void GuiTextEditKeyboardPopup::updateCaseIndicator()
{
	if (mCaseIndicator)
	{
		mCaseIndicator->setColor(getCaseIndicatorColor());
		mCaseIndicator->setText(mShift ? "ABC" : "abc");
	}
}

void GuiTextEditKeyboardPopup::shiftKeys()
{
	mShift = !mShift;
	updateCaseIndicator();

	for (auto& kb : mKeyboardButtons)
	{
		std::string display;

		if (kb.key == "✓" || kb.key == "⌫" || kb.key == "␣" || kb.key == "clr" || kb.key == "✖")
			display = kb.key;
		else
			display = mShift ? kb.shiftedKey : kb.key;

		kb.button->setDisplayedText(display);
	}
}

std::shared_ptr<KeyboardKeyComponent> GuiTextEditKeyboardPopup::makeButton(
	const std::string& key,
	const std::string& shiftedKey)
{
	std::shared_ptr<KeyboardKeyComponent> button =
		std::make_shared<KeyboardKeyComponent>(
			mWindow,
			key,
			[this, key, shiftedKey]
			{
				if (key == "✓")
				{
					acceptAndClose();
					return;
				}
				else if (key == "⌫")
				{
					deleteLastChar();
					return;
				}
				else if (key == "␣")
				{
					mText->textInput(" ");
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
					closeWithoutSaving();
					return;
				}

				mText->textInput((mShift ? shiftedKey : key).c_str());
			});

	KeyboardButton kb{ button, key, shiftedKey };
	mKeyboardButtons.push_back(kb);
	return button;
}
