// es-app/src/guis/GuiInfoTextViewer.h
#pragma once
#ifndef ES_APP_GUIS_GUI_INFO_TEXT_VIEWER_H
#define ES_APP_GUIS_GUI_INFO_TEXT_VIEWER_H

#include "GuiComponent.h"
#include "Window.h"

#include "components/TextComponent.h"
#include "resources/Font.h"
#include "renderers/Renderer.h"
#include "utils/StringUtil.h"

#include <string>
#include <algorithm>

class GuiInfoTextViewer : public GuiComponent
{
public:
	GuiInfoTextViewer(Window* window, const std::string& title, const std::string& text)
		: GuiComponent(window)
		, mTitle(window)
		, mBody(window)
	{
		mTitleStr = Utils::String::trim(title);
		mBodyStr  = text;

		mBgColor     = 0x000000CC;
		mPanelColor  = 0x202020F0;
		mBorderColor = 0xFFFFFFFF;

		mTitle.setFont(Font::get((int)(0.045f * Renderer::getScreenHeight()), Font::getDefaultPath()));
		mTitle.setColor(0xFFFFFFFF);
		mTitle.setHorizontalAlignment(ALIGN_CENTER);
		mTitle.setVerticalAlignment(ALIGN_CENTER);
		mTitle.setText(mTitleStr.empty() ? "INFO" : mTitleStr);

		mBody.setFont(Font::get((int)(0.030f * Renderer::getScreenHeight()), Font::getDefaultPath()));
		mBody.setColor(0xFFFFFFFF);
		mBody.setHorizontalAlignment(ALIGN_LEFT);
		mBody.setVerticalAlignment(ALIGN_TOP);
		mBody.setText(mBodyStr);

		addChild(&mTitle);
		addChild(&mBody);

		setSize((float)Renderer::getScreenWidth(), (float)Renderer::getScreenHeight());
		recalcLayout();
	}

	void onSizeChanged() override
	{
		recalcLayout();
	}

	bool input(InputConfig* config, Input input) override
	{
		if (input.value == 0)
			return false;

		if (config->isMappedLike("up", input))
		{
			scrollBy(-mLineStep);
			return true;
		}
		if (config->isMappedLike("down", input))
		{
			scrollBy(mLineStep);
			return true;
		}
		if (config->isMappedLike("left", input))
		{
			scrollBy(-mPageStep);
			return true;
		}
		if (config->isMappedLike("right", input))
		{
			scrollBy(mPageStep);
			return true;
		}

		// ✅ Salir SOLO con back/b (músculo de consola)
		if (config->isMappedTo("b", input) || config->isMappedTo("back", input))
		{
			delete this;
			return true;
		}

		// ✅ A no cierra (confirmar debe seguir siendo confirmar)
		return GuiComponent::input(config, input);
	}

	std::vector<HelpPrompt> getHelpPrompts() override
	{
		std::vector<HelpPrompt> p;
		p.push_back(HelpPrompt("up/down", "SCROLL"));
		p.push_back(HelpPrompt("b", "BACK"));
		return p;
	}

	void render(const Transform4x4f& parentTrans) override
	{
		Transform4x4f trans = parentTrans * getTransform();
		Renderer::setMatrix(trans);

		Renderer::drawRect(0.0f, 0.0f, mSize.x(), mSize.y(), mBgColor, mBgColor);
		Renderer::drawRect(mPanelPos.x(), mPanelPos.y(), mPanelSize.x(), mPanelSize.y(), mPanelColor, mPanelColor);

		Renderer::drawRect(mPanelPos.x(), mPanelPos.y(), mPanelSize.x(), 1.0f, mBorderColor, mBorderColor);
		Renderer::drawRect(mPanelPos.x(), mPanelPos.y() + mPanelSize.y() - 1.0f, mPanelSize.x(), 1.0f, mBorderColor, mBorderColor);
		Renderer::drawRect(mPanelPos.x(), mPanelPos.y(), 1.0f, mPanelSize.y(), mBorderColor, mBorderColor);
		Renderer::drawRect(mPanelPos.x() + mPanelSize.x() - 1.0f, mPanelPos.y(), 1.0f, mPanelSize.y(), mBorderColor, mBorderColor);

		mTitle.render(trans);

		Renderer::pushClipRect(
			Vector2i((int)mBodyClipPos.x(), (int)mBodyClipPos.y()),
			Vector2i((int)mBodyClipSize.x(), (int)mBodyClipSize.y()));

		Transform4x4f bodyTrans = trans;
		bodyTrans.translate(Vector3f(0.0f, -mScrollY, 0.0f));
		mBody.render(bodyTrans);

		Renderer::popClipRect();
	}

private:
	void recalcLayout()
	{
		const float w = mSize.x();
		const float h = mSize.y();

		const float marginX = std::max(20.0f, w * 0.06f);
		const float marginY = std::max(20.0f, h * 0.06f);

		mPanelPos  = Vector2f(marginX, marginY);
		mPanelSize = Vector2f(w - marginX * 2.0f, h - marginY * 2.0f);

		const float titleH = std::max(34.0f, h * 0.08f);
		mTitle.setPosition(mPanelPos.x(), mPanelPos.y());
		mTitle.setSize(mPanelSize.x(), titleH);

		const float pad = std::max(14.0f, h * 0.02f);

		const Vector2f bodyPos(mPanelPos.x() + pad, mPanelPos.y() + titleH + pad * 0.6f);
		const Vector2f bodySize(mPanelSize.x() - pad * 2.0f, mPanelSize.y() - titleH - pad * 1.6f);

		mBody.setPosition(bodyPos.x(), bodyPos.y());
		mBody.setSize(bodySize.x(), bodySize.y());

		mBodyClipPos  = bodyPos;
		mBodyClipSize = bodySize;

		mLineStep = std::max(18.0f, h * 0.035f);
		mPageStep = bodySize.y() * 0.85f;

		if (mScrollY < 0.0f) mScrollY = 0.0f;
	}

	void scrollBy(float delta)
	{
		mScrollY += delta;
		if (mScrollY < 0.0f) mScrollY = 0.0f;

		const float maxTop = 100000.0f;
		if (mScrollY > maxTop) mScrollY = maxTop;
	}

private:
	TextComponent mTitle;
	TextComponent mBody;

	std::string mTitleStr;
	std::string mBodyStr;

	unsigned int mBgColor;
	unsigned int mPanelColor;
	unsigned int mBorderColor;

	Vector2f mPanelPos;
	Vector2f mPanelSize;

	Vector2f mBodyClipPos;
	Vector2f mBodyClipSize;

	float mScrollY  = 0.0f;
	float mLineStep = 24.0f;
	float mPageStep = 240.0f;
};

#endif // ES_APP_GUIS_GUI_INFO_TEXT_VIEWER_H
