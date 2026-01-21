#pragma once
#ifndef ES_CORE_COMPONENTS_SCROLLABLE_TEXT_COMPONENT_H
#define ES_CORE_COMPONENTS_SCROLLABLE_TEXT_COMPONENT_H

#include "GuiComponent.h"
#include "components/ScrollableContainer.h"
#include "components/TextComponent.h"

class ThemeData;

class ScrollableTextComponent : public GuiComponent
{
public:
	explicit ScrollableTextComponent(Window* window);

	void setAutoScroll(bool enabled);
	void setAutoScrollDelay(float secondsOrMs);

	bool getAutoScroll() const { return mAutoScroll; }
	int  getAutoScrollDelayMs() const { return mDelayMs; }

	std::string getValue() const override;
	void setValue(const std::string& value) override;

	void update(int deltaTime) override;
	void render(const Transform4x4f& parentTrans) override;

	// IMPORTANTE: este applyTheme ahora es “robusto”:
	// el WRAPPER toma pos/size/origin/zIndex del ThemeElement,
	// y el TextComponent interno solo toma estilo/texto.
	void applyTheme(const std::shared_ptr<ThemeData>& theme,
	                const std::string& view,
	                const std::string& element,
	                unsigned int properties) override;

protected:
	void onSizeChanged() override;

private:
	void syncLayout();

	ScrollableContainer mScroll;
	TextComponent       mText;

	bool mAutoScroll;
	int  mDelayMs; // ms
};

#endif // ES_CORE_COMPONENTS_SCROLLABLE_TEXT_COMPONENT_H
