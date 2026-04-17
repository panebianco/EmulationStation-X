#pragma once

#include "GuiComponent.h"
#include "components/ComponentGrid.h"
#include "components/ImageComponent.h"
#include "components/TextComponent.h"
#include "components/TextEditComponent.h"
#include "components/KeyboardKeyComponent.h"

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

	bool input(InputConfig* config, Input input) override;
	void textInput(const char* text) override;
	void update(int deltaTime) override;
	void render(const Transform4x4f& parentTrans) override;
	std::vector<HelpPrompt> getHelpPrompts() override;

private:
	struct KeyboardButton
	{
		std::shared_ptr<KeyboardKeyComponent> button;
		std::string key;
		std::string shiftedKey;
	};

	void onSizeChanged() override;
	void updateCaseIndicator();
	void shiftKeys();

	void acceptAndClose();
	void closeWithoutSaving();
	void deleteLastChar();

	std::shared_ptr<KeyboardKeyComponent> makeButton(
		const std::string& key,
		const std::string& shiftedKey);

	ImageComponent mBackground;
	ComponentGrid mGrid;
	std::shared_ptr<ComponentGrid> mKeyboardGrid;

	std::shared_ptr<TextComponent> mTitle;
	std::shared_ptr<TextEditComponent> mText;
	std::shared_ptr<TextComponent> mCaseIndicator;
	std::shared_ptr<KeyboardKeyComponent> mShiftButton;

	std::function<void(const std::string&)> mOkCallback;

	bool mMultiLine;
	bool mShift;
	bool mDeleteRepeat;
	bool mTextEditActive;

	int mDeleteRepeatTimer;
	int mNavigationRepeatTimer;
	int mNavigationRepeatDirX;
	int mNavigationRepeatDirY;
	int mHorizontalKeyCount;

	std::string mInitValue;
	std::vector<KeyboardButton> mKeyboardButtons;
};
