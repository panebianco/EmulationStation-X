#pragma once

#include "GuiComponent.h"
#include "TextComponent.h"

#include <functional>
#include <memory>
#include <string>

class InputConfig;
struct Input;

class KeyboardKeyComponent : public GuiComponent
{
public:
	KeyboardKeyComponent(
		Window* window,
		const std::string& text,
		const std::function<void()>& callback);

	void setDisplayedText(const std::string& text);

	bool input(InputConfig* config, Input input) override;
	void render(const Transform4x4f& parentTrans) override;
	void onSizeChanged() override;

	void onFocusGained() override;
	void onFocusLost() override;

private:
	std::shared_ptr<TextComponent> mLabel;
	std::function<void()> mCallback;
	bool mFocused;
};