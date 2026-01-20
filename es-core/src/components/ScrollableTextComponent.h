#pragma once

#include "GuiComponent.h"
#include "components/ScrollableContainer.h"
#include "components/TextComponent.h"

class ScrollableTextComponent : public GuiComponent
{
public:
	explicit ScrollableTextComponent(Window* window);

	// Interface útil
	void setText(const std::string& text);
	const std::string& getText() const;

	void setAutoScroll(bool enabled, int delayMs = 1000);

	// Theme
	void applyTheme(const std::shared_ptr<ThemeData>& theme,
	                const std::string& view,
	                const std::string& element,
	                unsigned int properties) override;

	// GuiComponent overrides
	void setSize(const Vector2f& size) override;
	void setPosition(float x, float y, float z = 0) override;
	void setPosition(const Vector3f& offset) override;

private:
	void syncChildren();

private:
	ScrollableContainer mScroll;
	TextComponent       mText;

	bool mAutoScroll;
	int  mAutoScrollDelay;
};
