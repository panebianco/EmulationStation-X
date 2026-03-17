#include "KeyboardKeyComponent.h"

#include "InputConfig.h"
#include "Window.h"
#include "Settings.h"
#include "resources/Font.h"
#include "renderers/Renderer.h"

namespace
{
	inline bool isMenuDark()
	{
		return Settings::getInstance()->getBool("MenuDark");
	}
}

KeyboardKeyComponent::KeyboardKeyComponent(
	Window* window,
	const std::string& text,
	const std::function<void()>& callback)
	: GuiComponent(window)
	, mLabel(std::make_shared<TextComponent>(
		window,
		text,
		Font::get(FONT_SIZE_MEDIUM),
		0xCFCFCFFF,
		ALIGN_CENTER))
	, mCallback(callback)
	, mFocused(false)
{
	addChild(mLabel.get());
}

void KeyboardKeyComponent::setDisplayedText(const std::string& text)
{
	mLabel->setText(text);
}

bool KeyboardKeyComponent::input(InputConfig* config, Input input)
{
	if (input.value && config->isMappedTo("a", input))
	{
		if (mCallback)
			mCallback();
		return true;
	}

	return GuiComponent::input(config, input);
}

void KeyboardKeyComponent::onSizeChanged()
{
	mLabel->setSize(mSize.x(), mSize.y());
	mLabel->setPosition(0.0f, 0.0f, 0.0f);
}

void KeyboardKeyComponent::onFocusGained()
{
	mFocused = true;
	GuiComponent::onFocusGained();
}

void KeyboardKeyComponent::onFocusLost()
{
	mFocused = false;
	GuiComponent::onFocusLost();
}

void KeyboardKeyComponent::render(const Transform4x4f& parentTrans)
{
	Transform4x4f trans = parentTrans * getTransform();
	Renderer::setMatrix(trans);

	const bool dark = isMenuDark();

	const unsigned int fillColor =
		mFocused ? 0x2D6CDFE0 : (dark ? 0x181818F0 : 0xF0F0F0F0);

	const unsigned int borderColor =
		mFocused ? 0xFFFFFFFF : (dark ? 0x909090FF : 0x707070FF);

	const unsigned int textColor =
		mFocused ? 0xFFFFFFFF : (dark ? 0xD0D0D0FF : 0x222222FF);

	mLabel->setColor(textColor);

	Renderer::drawRect(0.0f, 0.0f, mSize.x(), mSize.y(), fillColor, fillColor);
	Renderer::drawRect(0.0f, 0.0f, mSize.x(), 2.0f, borderColor, borderColor);
	Renderer::drawRect(0.0f, mSize.y() - 2.0f, mSize.x(), 2.0f, borderColor, borderColor);
	Renderer::drawRect(0.0f, 0.0f, 2.0f, mSize.y(), borderColor, borderColor);
	Renderer::drawRect(mSize.x() - 2.0f, 0.0f, 2.0f, mSize.y(), borderColor, borderColor);

	renderChildren(trans);
}
