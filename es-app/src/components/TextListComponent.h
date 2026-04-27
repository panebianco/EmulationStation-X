#pragma once
#ifndef ES_APP_COMPONENTS_TEXT_LIST_COMPONENT_H
#define ES_APP_COMPONENTS_TEXT_LIST_COMPONENT_H

#include "components/IList.h"
#include "components/ImageComponent.h"
#include "math/Misc.h"
#include "utils/StringUtil.h"
#include "Log.h"
#include "Settings.h"
#include "Sound.h"

#include <memory>
#include <type_traits>
#include <vector>
#include <cmath>

class TextCache;
class SystemData;

struct TextListData
{
	unsigned int colorId;
	std::shared_ptr<TextCache> textCache;
};

// A graphical list. Supports multiple colors for rows and scrolling.
template <typename T>
class TextListComponent : public IList<TextListData, T>
{
protected:
	using IList<TextListData, T>::mEntries;
	using IList<TextListData, T>::listUpdate;
	using IList<TextListData, T>::listInput;
	using IList<TextListData, T>::listRenderTitleOverlay;
	using IList<TextListData, T>::getTransform;
	using IList<TextListData, T>::mSize;
	using IList<TextListData, T>::mCursor;
	using IList<TextListData, T>::mViewportTop;
	using IList<TextListData, T>::mEntry;

public:
	using IList<TextListData, T>::size;
	using IList<TextListData, T>::isScrolling;
	using IList<TextListData, T>::stopScrolling;

	static constexpr int REFRESH_LIST_CURSOR_POS = -1;

	TextListComponent(Window* window);

	bool input(InputConfig* config, Input input) override;
	void update(int deltaTime) override;
	void render(const Transform4x4f& parentTrans) override;
	void applyTheme(const std::shared_ptr<ThemeData>& theme, const std::string& view, const std::string& element, unsigned int properties) override;

	void add(const std::string& name, const T& obj, unsigned int colorId);

	enum Alignment
	{
		ALIGN_LEFT,
		ALIGN_CENTER,
		ALIGN_RIGHT
	};

	enum Orientation
	{
		ORIENTATION_VERTICAL,
		ORIENTATION_HORIZONTAL
	};

	enum CarouselLogoAlignment
	{
		CAROUSEL_ALIGN_LEFT,
		CAROUSEL_ALIGN_CENTER,
		CAROUSEL_ALIGN_RIGHT,
		CAROUSEL_ALIGN_TOP,
		CAROUSEL_ALIGN_BOTTOM
	};

	inline void setAlignment(Alignment align) { mAlignment = align; }
	inline void setOrientation(Orientation orientation) { mOrientation = orientation; }

	inline void setCursorChangedCallback(const std::function<void(CursorState state)>& func) { mCursorChangedCallback = func; }

	inline void setFont(const std::shared_ptr<Font>& font)
	{
		mFont = font;
		for (auto it = mEntries.begin(); it != mEntries.end(); it++)
			it->data.textCache.reset();
	}

	inline void setUppercase(bool uppercase)
	{
		mUppercase = uppercase;
		for (auto it = mEntries.begin(); it != mEntries.end(); it++)
			it->data.textCache.reset();
	}

	inline void setSelectorHeight(float selectorScale) { mSelectorHeight = selectorScale; }
	inline void setSelectorOffsetY(float selectorOffsetY) { mSelectorOffsetY = selectorOffsetY; }
	inline void setSelectorColor(unsigned int color) { mSelectorColor = color; }
	inline void setSelectorColorEnd(unsigned int color) { mSelectorColorEnd = color; }
	inline void setSelectorColorGradientHorizontal(bool horizontal) { mSelectorColorGradientHorizontal = horizontal; }
	inline void setSelectedColor(unsigned int color) { mSelectedColor = color; }
	inline void setColor(unsigned int id, unsigned int color) { mColors[id] = color; }
	inline void setLineSpacing(float lineSpacing) { mLineSpacing = lineSpacing; }

	inline void setSelectorWidth(float selectorWidth) { mSelectorWidth = selectorWidth; }
	inline void setSelectorHorizontalOffset(float selectorHorizontalOffset) { mSelectorHorizontalOffset = selectorHorizontalOffset; }
	inline void setTopAligned(bool topAligned) { mTopAligned = topAligned; }

	inline void setShowRowNumbers(bool showRowNumbers)
	{
		mShowRowNumbers = showRowNumbers;
		for (auto it = mEntries.begin(); it != mEntries.end(); it++)
			it->data.textCache.reset();
	}

protected:
	virtual void onScroll(int /*amt*/) override
	{
		if (!mScrollSound.empty())
			Sound::get(mScrollSound)->play();
	}

	virtual void onCursorChanged(const CursorState& state) override;

private:
	int mMarqueeOffset;
	int mMarqueeOffset2;
	int mMarqueeTime;

	Alignment mAlignment;
	Orientation mOrientation;
	float mHorizontalMargin;

	int viewportTop();
	void renderHorizontalCarousel(const Transform4x4f& trans);

	std::function<void(CursorState state)> mCursorChangedCallback;

	std::shared_ptr<Font> mFont;
	bool mUppercase;
	float mLineSpacing;
	float mSelectorHeight;
	float mSelectorOffsetY;
	unsigned int mSelectorColor;
	unsigned int mSelectorColorEnd;
	bool mSelectorColorGradientHorizontal = true;
	unsigned int mSelectedColor;
	std::string mScrollSound;
	static const unsigned int COLOR_ID_COUNT = 2;
	unsigned int mColors[COLOR_ID_COUNT];
	int mViewportHeight;
	int mCursorPrev = -1;

	float mSelectorWidth;
	float mSelectorHorizontalOffset;
	bool mTopAligned;
	bool mShowRowNumbers;

	// ES-X: modo carrusel opcional para textlist/gamelist.
	bool mCarouselMode;
	bool mCarouselLoop;
	int mCarouselMaxLogoCount;
	float mCarouselLogoScale;
	float mCarouselMinLogoOpacity;
	float mCarouselLogoSpacingX;
	float mCarouselScaledLogoSpacing;

	Vector2f mCarouselLogoSize;
	CarouselLogoAlignment mCarouselLogoAlignment;

	float mCarouselLogoOffsetX;
	float mCarouselLogoOffsetY;
	float mCarouselSelectedLogoOffsetX;
	float mCarouselSelectedLogoOffsetY;

	// Cámara visual del carrusel.
	// mCursor sigue siendo la selección real; esto solo suaviza el render.
	float mCarouselCamOffset;

	// ES-X: tarjetas y texto controlado del carrusel.
	unsigned int mCarouselItemColor;
	unsigned int mCarouselSelectedItemColor;
	int mCarouselTextMaxLines;

	ImageComponent mSelectorImage;
};

template <typename T>
TextListComponent<T>::TextListComponent(Window* window) :
	IList<TextListData, T>(window), mSelectorImage(window)
{
	mMarqueeOffset = 0;
	mMarqueeOffset2 = 0;
	mMarqueeTime = 0;

	mHorizontalMargin = 0;
	mAlignment = ALIGN_CENTER;
	mOrientation = ORIENTATION_VERTICAL;

	mFont = Font::get(FONT_SIZE_MEDIUM);
	mUppercase = false;
	mLineSpacing = 1.5f;
	mSelectorHeight = mFont->getSize() * mLineSpacing;
	mSelectorOffsetY = 0;
	mSelectorColor = 0x000000FF;
	mSelectorColorEnd = 0x000000FF;
	mSelectorColorGradientHorizontal = true;
	mSelectedColor = 0;
	mColors[0] = 0x0000FFFF;
	mColors[1] = 0x00FF00FF;

	mSelectorWidth = 0.0f;
	mSelectorHorizontalOffset = 0.0f;

	mTopAligned = std::is_same<T, SystemData*>::value;
	mShowRowNumbers = false;

	mCarouselMode = false;
	mCarouselLoop = false;
	mCarouselMaxLogoCount = 5;
	mCarouselLogoScale = 1.20f;
	mCarouselMinLogoOpacity = 0.45f;
	mCarouselLogoSpacingX = 0.0f;
	mCarouselScaledLogoSpacing = 0.0f;

	mCarouselLogoSize = Vector2f::Zero();
	mCarouselLogoAlignment = CAROUSEL_ALIGN_CENTER;

	mCarouselLogoOffsetX = 0.0f;
	mCarouselLogoOffsetY = 0.0f;
	mCarouselSelectedLogoOffsetX = 0.0f;
	mCarouselSelectedLogoOffsetY = 0.0f;

	mCarouselCamOffset = 0.0f;

	mCarouselItemColor = 0x00000088;
	mCarouselSelectedItemColor = 0x101020CC;
	mCarouselTextMaxLines = 2;
}

template <typename T>
void TextListComponent<T>::renderHorizontalCarousel(const Transform4x4f& trans)
{
	std::shared_ptr<Font>& font = mFont;

	if (size() == 0)
		return;

	int visibleCount = mCarouselMaxLogoCount;
	if (visibleCount < 1)
		visibleCount = 1;

	if (!mCarouselLoop && visibleCount > size())
		visibleCount = size();

	if (visibleCount > 1 && (visibleCount % 2) == 0)
		visibleCount--;

	if (visibleCount < 1)
		visibleCount = 1;

	const int sideCount = visibleCount / 2;
	const int buffer = (mCarouselLoop && size() > 1) ? 2 : 1;

	const float baseTextHeight = Math::max(font->getHeight(1.0f), (float)font->getSize());

	float itemWidth = 0.0f;
	float itemHeight = 0.0f;

	if (mCarouselLogoSize.x() > 1.0f)
		itemWidth = mCarouselLogoSize.x();
	else if (mSelectorWidth > 0.0f)
		itemWidth = mSelectorWidth;
	else
		itemWidth = mSize.x() / (float)visibleCount;

	if (mCarouselLogoSize.y() > 1.0f)
		itemHeight = mCarouselLogoSize.y();
	else if (mSelectorHeight > 1.0f && mSelectorHeight <= mSize.y())
		itemHeight = mSelectorHeight;
	else
		itemHeight = baseTextHeight * 4.0f;

	if (itemWidth <= 1.0f)
		itemWidth = mSize.x() / 5.0f;

	if (itemHeight <= 1.0f)
		itemHeight = baseTextHeight * 4.0f;

	float spacing = mCarouselLogoSpacingX;
	if (spacing <= 1.0f)
		spacing = itemWidth + mHorizontalMargin;
	if (spacing <= 1.0f)
		spacing = itemWidth;

	const float centerX = (mSize.x() * 0.5f) + mCarouselLogoOffsetX;
	const float centerY = (mSize.y() * 0.5f) + mSelectorOffsetY + mCarouselLogoOffsetY;

	Vector3f dim(mSize.x(), mSize.y(), 0);
	dim = trans * dim - trans.translation();

	Renderer::pushClipRect(
		Vector2i((int)trans.translation().x(), (int)trans.translation().y()),
		Vector2i((int)dim.x(), (int)dim.y()));

	int visualCenter = (int)std::round(mCarouselCamOffset);

	if (!mCarouselLoop)
	{
		if (visualCenter < 0)
			visualCenter = 0;
		if (visualCenter >= size())
			visualCenter = size() - 1;
	}

	int start = visualCenter - sideCount - buffer;
	int end   = visualCenter + sideCount + buffer;

	if (size() == 1)
	{
		start = 0;
		end = 0;
	}

	auto getRealIndex = [&](int virtualIndex) -> int
	{
		if (size() <= 0)
			return -1;

		if (mCarouselLoop && size() > 1)
		{
			int index = virtualIndex;

			while (index < 0)
				index += size();

			while (index >= size())
				index -= size();

			return index;
		}

		if (virtualIndex < 0 || virtualIndex >= size())
			return -1;

		return virtualIndex;
	};

	auto applyOpacity = [](unsigned int color, float opacity01) -> unsigned int
	{
		if (opacity01 < 0.0f)
			opacity01 = 0.0f;
		if (opacity01 > 1.0f)
			opacity01 = 1.0f;

		unsigned int baseAlpha = color & 0x000000FF;
		unsigned int alpha = (unsigned int)((float)baseAlpha * opacity01);

		if (alpha > 255)
			alpha = 255;

		return (color & 0xFFFFFF00) | alpha;
	};

	auto truncateToWidth = [&](std::string text, float maxWidth) -> std::string
	{
		text = Utils::String::trim(text);

		if (text.empty())
			return "";

		if (font->sizeText(text).x() <= maxWidth)
			return text;

		const std::string dots = "...";

		while (text.size() > 1 && font->sizeText(text + dots).x() > maxWidth)
			text.pop_back();

		return text + dots;
	};

	auto makeLines = [&](const std::string& original, float maxWidth, int maxLines) -> std::vector<std::string>
	{
		std::vector<std::string> lines;

		if (maxLines < 1)
			maxLines = 1;
		if (maxLines > 3)
			maxLines = 3;

		std::string text = Utils::String::trim(original);

		if (text.empty())
		{
			lines.push_back("");
			return lines;
		}

		if (maxLines == 1)
		{
			lines.push_back(truncateToWidth(text, maxWidth));
			return lines;
		}

		if (font->sizeText(text).x() <= maxWidth)
		{
			lines.push_back(text);
			return lines;
		}

		std::vector<std::string> words = Utils::String::delimitedStringToVector(text, " ");
		std::string current;

		for (size_t i = 0; i < words.size(); i++)
		{
			std::string candidate = current.empty() ? words[i] : current + " " + words[i];

			if (font->sizeText(candidate).x() <= maxWidth || current.empty())
			{
				current = candidate;
			}
			else
			{
				lines.push_back(truncateToWidth(current, maxWidth));
				current = words[i];

				if ((int)lines.size() >= maxLines - 1)
				{
					std::string rest = current;
					for (size_t j = i + 1; j < words.size(); j++)
						rest += " " + words[j];

					lines.push_back(truncateToWidth(rest, maxWidth));

					if ((int)lines.size() > maxLines)
						lines.resize((size_t)maxLines);

					return lines;
				}
			}
		}

		if (!current.empty())
			lines.push_back(truncateToWidth(current, maxWidth));

		if (lines.empty())
			lines.push_back(truncateToWidth(text, maxWidth));

		if ((int)lines.size() > maxLines)
			lines.resize((size_t)maxLines);

		return lines;
	};

	int activeVirtualPos = mCursor;
	float bestActiveDistance = 99999.0f;

	for (int virtualPos = start; virtualPos <= end; virtualPos++)
	{
		int realIndex = getRealIndex(virtualPos);
		if (realIndex < 0)
			continue;

		if (realIndex == mCursor)
		{
			float d = (float)virtualPos - mCarouselCamOffset;
			float ad = d < 0.0f ? -d : d;

			if (ad < bestActiveDistance)
			{
				bestActiveDistance = ad;
				activeVirtualPos = virtualPos;
			}
		}
	}

	auto renderEntry = [&](int virtualPos, bool selected)
	{
		int realIndex = getRealIndex(virtualPos);
		if (realIndex < 0)
			return;

		typename IList<TextListData, T>::Entry& entry = mEntries.at((unsigned int)realIndex);

		float distance = (float)virtualPos - mCarouselCamOffset;
		float absDistance = distance < 0.0f ? -distance : distance;

		float influence = Math::max(0.0f, 1.0f - absDistance);

		float scale = 1.0f + ((mCarouselLogoScale - 1.0f) * influence);
		if (scale < 0.1f)
			scale = 1.0f;

		float minOpacity = mCarouselMinLogoOpacity;
		if (minOpacity < 0.0f)
			minOpacity = 0.0f;
		if (minOpacity > 1.0f)
			minOpacity = 1.0f;

		float opacity01 = minOpacity + ((1.0f - minOpacity) * influence);

		float itemCenterX = centerX + (distance * spacing);
		float itemCenterY = centerY;

		// Imitación de scaledLogoSpacing de SystemView.
		// Cuando el central escala, empuja laterales para que no se encimen.
		if (mCarouselScaledLogoSpacing != 0.0f && absDistance > 0.001f)
		{
			float logoDiffX = ((spacing * mCarouselLogoScale) - spacing)
				/ 2.0f * mCarouselScaledLogoSpacing;

			if (distance < 0.0f)
				itemCenterX -= logoDiffX;
			else
				itemCenterX += logoDiffX;
		}

		if (selected)
		{
			itemCenterX += mCarouselSelectedLogoOffsetX;
			itemCenterY += mCarouselSelectedLogoOffsetY;
		}

		const float scaledW = itemWidth * scale;
		const float scaledH = itemHeight * scale;

		const bool visuallyCentered = (std::fabs(distance) < 0.50f);

		unsigned int bgColor = (selected && visuallyCentered) ? mCarouselSelectedItemColor : mCarouselItemColor;
		bgColor = applyOpacity(bgColor, opacity01);

		Renderer::setMatrix(trans);
		Renderer::drawRect(
			itemCenterX - (scaledW * 0.5f),
			itemCenterY - (scaledH * 0.5f),
			scaledW,
			scaledH,
			bgColor,
			bgColor);

		unsigned int textColor;
		if (selected && visuallyCentered && mSelectedColor)
			textColor = mSelectedColor;
		else
			textColor = mColors[entry.data.colorId];

		textColor = applyOpacity(textColor, opacity01);

		std::string displayName = mUppercase ? Utils::String::toUpper(entry.name) : entry.name;

		if (mShowRowNumbers)
			displayName = std::to_string(realIndex + 1) + ". " + displayName;

		const float maxTextWidth = itemWidth * 0.86f;
		std::vector<std::string> lines = makeLines(displayName, maxTextWidth, mCarouselTextMaxLines);

		const float lineGap = baseTextHeight * 0.15f;
		const float totalTextHeight =
			(lines.size() * baseTextHeight) +
			(lines.size() > 1 ? ((lines.size() - 1) * lineGap) : 0.0f);

		float y = -(totalTextHeight * 0.5f);

		if (mCarouselLogoAlignment == CAROUSEL_ALIGN_TOP)
			y = -(itemHeight * 0.5f) + (itemHeight * 0.08f);
		else if (mCarouselLogoAlignment == CAROUSEL_ALIGN_BOTTOM)
			y = (itemHeight * 0.5f) - totalTextHeight - (itemHeight * 0.08f);

		for (size_t line = 0; line < lines.size(); line++)
		{
			TextCache* cache = font->buildTextCache(lines[line], 0, 0, 0x000000FF);
			cache->setColor(textColor);

			float textX = -(cache->metrics.size.x() * 0.5f);

			if (mCarouselLogoAlignment == CAROUSEL_ALIGN_LEFT || mAlignment == ALIGN_LEFT)
				textX = -(itemWidth * 0.5f) + (itemWidth * 0.06f);
			else if (mCarouselLogoAlignment == CAROUSEL_ALIGN_RIGHT || mAlignment == ALIGN_RIGHT)
				textX = (itemWidth * 0.5f) - cache->metrics.size.x() - (itemWidth * 0.06f);

			Transform4x4f drawTrans = trans;
			drawTrans.translate(Vector3f(itemCenterX, itemCenterY, 0.0f));
			drawTrans.scale(Vector3f(scale, scale, 1.0f));
			drawTrans.translate(Vector3f(textX, y, 0.0f));

			Renderer::setMatrix(drawTrans);
			font->renderTextCache(cache);
			delete cache;

			y += baseTextHeight + lineGap;
		}
	};

	for (int virtualPos = start; virtualPos <= end; virtualPos++)
	{
		if (virtualPos == activeVirtualPos)
			continue;

		renderEntry(virtualPos, false);
	}

	// Importante:
	// La tarjeta activa se salta arriba para dibujar los laterales primero,
	// pero luego DEBE renderizarse al final para quedar encima.
	if (getRealIndex(activeVirtualPos) >= 0)
		renderEntry(activeVirtualPos, true);

	Renderer::popClipRect();

	listRenderTitleOverlay(trans);

	GuiComponent::renderChildren(trans);
}

template <typename T>
void TextListComponent<T>::render(const Transform4x4f& parentTrans)
{
	Transform4x4f trans = parentTrans * getTransform();

	std::shared_ptr<Font>& font = mFont;

	if (size() == 0)
		return;

	if (mOrientation == ORIENTATION_HORIZONTAL && mCarouselMode)
	{
		renderHorizontalCarousel(trans);
		return;
	}

	const float entrySize = Math::max(font->getHeight(1.0f), (float)font->getSize()) * mLineSpacing;

	mViewportHeight = (int)(mSize.y() / entrySize);

	if (mViewportTop == REFRESH_LIST_CURSOR_POS)
	{
		mViewportTop = mCursor - mViewportHeight / 2;
		mCursorPrev = -1;
	}

	if (mCursor != mCursorPrev)
	{
		mViewportTop = (size() > mViewportHeight) ? viewportTop() : 0;
		mCursorPrev = mCursor;
	}

	unsigned int listCutoff = mViewportTop + mViewportHeight;
	if (listCutoff > size())
		listCutoff = size();

	float y;
	if (mTopAligned)
		y = 0.0f;
	else
		y = (mSize.y() - (mViewportHeight * entrySize)) * 0.5f;

	const float selectorWidth = (mSelectorWidth > 0.0f) ? mSelectorWidth : mSize.x();

	if (mSelectorImage.hasImage())
	{
		mSelectorImage.setSize(selectorWidth, mSelectorHeight);
		mSelectorImage.setPosition(mSelectorHorizontalOffset,
			y + (mCursor - mViewportTop) * entrySize + mSelectorOffsetY, 0.0f);
		mSelectorImage.render(trans);
	}
	else
	{
		Renderer::setMatrix(trans);
		Renderer::drawRect(
			mSelectorHorizontalOffset,
			y + (mCursor - mViewportTop) * entrySize + mSelectorOffsetY,
			selectorWidth,
			mSelectorHeight,
			mSelectorColor,
			mSelectorColorEnd,
			mSelectorColorGradientHorizontal);
	}

	Vector3f dim(mSize.x(), mSize.y(), 0);
	dim = trans * dim - trans.translation();

	Renderer::pushClipRect(
		Vector2i((int)(trans.translation().x() + mHorizontalMargin), (int)trans.translation().y()),
		Vector2i((int)(dim.x() - mHorizontalMargin * 2), (int)dim.y()));

	for (int i = mViewportTop; i < (int)listCutoff; i++)
	{
		typename IList<TextListData, T>::Entry& entry = mEntries.at((unsigned int)i);

		unsigned int color;
		if (mCursor == i && mSelectedColor)
			color = mSelectedColor;
		else
			color = mColors[entry.data.colorId];

		std::string displayName = mUppercase ? Utils::String::toUpper(entry.name) : entry.name;

		if (mShowRowNumbers)
			displayName = std::to_string(i + 1) + ". " + displayName;

		if (!entry.data.textCache)
			entry.data.textCache = std::unique_ptr<TextCache>(
				font->buildTextCache(displayName, 0, 0, 0x000000FF));

		entry.data.textCache->setColor(color);

		Vector3f offset(0, y, 0);

		switch (mAlignment)
		{
		case ALIGN_LEFT:
			offset[0] = mHorizontalMargin;
			break;
		case ALIGN_CENTER:
			offset[0] = (int)((mSize.x() - entry.data.textCache->metrics.size.x()) / 2);
			if (offset[0] < mHorizontalMargin)
				offset[0] = mHorizontalMargin;
			break;
		case ALIGN_RIGHT:
			offset[0] = (mSize.x() - entry.data.textCache->metrics.size.x());
			offset[0] -= mHorizontalMargin;
			if (offset[0] < mHorizontalMargin)
				offset[0] = mHorizontalMargin;
			break;
		}

		Transform4x4f drawTrans = trans;

		if ((mCursor == i) && (mMarqueeOffset > 0))
			drawTrans.translate(offset - Vector3f((float)mMarqueeOffset, 0, 0));
		else
			drawTrans.translate(offset);

		Renderer::setMatrix(drawTrans);
		font->renderTextCache(entry.data.textCache.get());

		if ((mCursor == i) && (mMarqueeOffset2 < 0))
		{
			drawTrans = trans;
			drawTrans.translate(offset - Vector3f((float)mMarqueeOffset2, 0, 0));
			Renderer::setMatrix(drawTrans);
			font->renderTextCache(entry.data.textCache.get());
		}

		y += entrySize;
	}

	Renderer::popClipRect();

	listRenderTitleOverlay(trans);

	GuiComponent::renderChildren(trans);
}

template <typename T>
int TextListComponent<T>::viewportTop()
{
	int viewportTopMax = size() - mViewportHeight;
	int topNew = mViewportTop;

	if (mCursorPrev == -1)
		mCursorPrev = mCursor;

	int delta = mCursor - mCursorPrev;

	if (Settings::getInstance()->getBool("UseFullscreenPaging"))
	{
		if (delta <= -mViewportHeight || delta >= mViewportHeight
			|| delta < 0 && mCursor - mViewportTop < mViewportHeight / 2
			|| delta > 0 && mCursor - mViewportTop > mViewportHeight / 2)
		{
			topNew += delta;
		}
	}
	else
	{
		topNew = mCursor - mViewportHeight / 2;
	}

	if (mCursor <= mViewportHeight / 2)
		topNew = 0;
	else if (mCursor >= viewportTopMax + mViewportHeight / 2)
		topNew = viewportTopMax;

	return topNew;
}

template <typename T>
bool TextListComponent<T>::input(InputConfig* config, Input input)
{
	bool isSingleStep = false;

	if (mOrientation == ORIENTATION_HORIZONTAL)
		isSingleStep = config->isMappedLike("right", input) || config->isMappedLike("left", input);
	else
		isSingleStep = config->isMappedLike("down", input) || config->isMappedLike("up", input);

	bool isPageStep = config->isMappedLike("rightshoulder", input) || config->isMappedLike("leftshoulder", input);

	if (size() > 0 && (isSingleStep || isPageStep))
	{
		if (input.value != 0)
		{
			int delta;
			mCursorPrev = mCursor;

			if (isSingleStep)
			{
				if (mOrientation == ORIENTATION_HORIZONTAL)
					delta = config->isMappedLike("right", input) ? 1 : -1;
				else
					delta = config->isMappedLike("down", input) ? 1 : -1;
			}
			else
			{
				delta = Settings::getInstance()->getBool("UseFullscreenPaging") ? mViewportHeight : 10;
				if (config->isMappedLike("leftshoulder", input))
					delta = -delta;
			}

			listInput(delta);
			return true;
		}
		else
		{
			stopScrolling();
		}
	}

	return GuiComponent::input(config, input);
}

template <typename T>
void TextListComponent<T>::update(int deltaTime)
{
	listUpdate(deltaTime);

	if (mCarouselMode && mOrientation == ORIENTATION_HORIZONTAL && size() > 0)
	{
		float target = (float)mCursor;

		if (mCarouselLoop && size() > 1)
		{
			float diff = target - mCarouselCamOffset;
			float half = (float)size() * 0.5f;

			if (diff > half)
				target -= (float)size();
			else if (diff < -half)
				target += (float)size();
		}

		float diff = target - mCarouselCamOffset;

		if (std::fabs(diff) < 0.001f)
		{
			mCarouselCamOffset = target;
		}
		else
		{
			float factor = Math::min(1.0f, (float)deltaTime / 110.0f);
			mCarouselCamOffset += diff * factor;
		}

		if (mCarouselLoop && size() > 1)
		{
			while (mCarouselCamOffset < 0.0f)
				mCarouselCamOffset += (float)size();

			while (mCarouselCamOffset >= (float)size())
				mCarouselCamOffset -= (float)size();
		}
	}
	else if (size() > 0)
	{
		mCarouselCamOffset = (float)mCursor;
	}

	if (!isScrolling() && size() > 0)
	{
		mMarqueeOffset = 0;
		mMarqueeOffset2 = 0;

		auto name = mEntries.at((unsigned int)mCursor).name;
		std::string displayName = mUppercase ? Utils::String::toUpper(name) : name;

		if (mShowRowNumbers)
			displayName = std::to_string(mCursor + 1) + ". " + displayName;

		const float textLength = mFont->sizeText(displayName).x();
		const float limit = mSize.x() - mHorizontalMargin * 2;

		if (textLength > limit)
		{
			const float speed = mFont->sizeText("ABCDEFGHIJKLMNOPQRSTUVWXYZ").x() * 0.247f;
			const float delay = 3000;
			const float scrollLength = textLength;
			const float returnLength = speed * 1.5f;
			const float scrollTime = (scrollLength * 1000) / speed;
			const float returnTime = (returnLength * 1000) / speed;
			const int maxTime = (int)(delay + scrollTime + returnTime);

			mMarqueeTime += deltaTime;
			while (mMarqueeTime > maxTime)
				mMarqueeTime -= maxTime;

			mMarqueeOffset = (int)(Math::Scroll::loop(delay, scrollTime + returnTime, (float)mMarqueeTime, scrollLength + returnLength));

			if (mMarqueeOffset > (scrollLength - (limit - returnLength)))
				mMarqueeOffset2 = (int)(mMarqueeOffset - (scrollLength + returnLength));
		}
	}

	GuiComponent::update(deltaTime);
}

template <typename T>
void TextListComponent<T>::add(const std::string& name, const T& obj, unsigned int color)
{
	assert(color < COLOR_ID_COUNT);

	typename IList<TextListData, T>::Entry entry;
	entry.name = name;
	entry.object = obj;
	entry.data.colorId = color;
	static_cast<IList<TextListData, T>*>(this)->add(entry);
}

template <typename T>
void TextListComponent<T>::onCursorChanged(const CursorState& state)
{
	mMarqueeOffset = 0;
	mMarqueeOffset2 = 0;
	mMarqueeTime = 0;

	if (size() > 0)
	{
		if (!mCarouselMode)
			mCarouselCamOffset = (float)mCursor;
		else if (mCarouselCamOffset < 0.0f || mCarouselCamOffset >= (float)size())
			mCarouselCamOffset = (float)mCursor;
	}

	if (mCursorChangedCallback)
		mCursorChangedCallback(state);
}

template <typename T>
void TextListComponent<T>::applyTheme(const std::shared_ptr<ThemeData>& theme, const std::string& view, const std::string& element, unsigned int properties)
{
	if (Settings::getInstance()->getBool("UseFullscreenPaging"))
	{
		mViewportTop = REFRESH_LIST_CURSOR_POS;
	}

	GuiComponent::applyTheme(theme, view, element, properties);

	const ThemeData::ThemeElement* elem = theme->getElement(view, element, "textlist");
	if (!elem)
		return;

	using namespace ThemeFlags;

	if (properties & COLOR)
	{
		if (elem->has("selectorColor"))
		{
			setSelectorColor(elem->get<unsigned int>("selectorColor"));
			setSelectorColorEnd(elem->get<unsigned int>("selectorColor"));
		}
		if (elem->has("selectorColorEnd"))
			setSelectorColorEnd(elem->get<unsigned int>("selectorColorEnd"));
		if (elem->has("selectorGradientType"))
			setSelectorColorGradientHorizontal(!(elem->get<std::string>("selectorGradientType").compare("horizontal")));
		if (elem->has("selectedColor"))
			setSelectedColor(elem->get<unsigned int>("selectedColor"));
		if (elem->has("primaryColor"))
			setColor(0, elem->get<unsigned int>("primaryColor"));
		if (elem->has("secondaryColor"))
			setColor(1, elem->get<unsigned int>("secondaryColor"));
	}

	setFont(Font::getFromTheme(elem, properties, mFont));
	const float selectorHeight = Math::max(mFont->getHeight(1.0f), (float)mFont->getSize()) * mLineSpacing;
	setSelectorHeight(selectorHeight);

	if (properties & SOUND)
	{
		if (elem->has("scrollSound"))
		{
			mScrollSound = elem->get<std::string>("scrollSound");
		}
		else
		{
			const ThemeData::ThemeElement* sndElem = theme->getElement("all", "scroll", "sound");

			if (sndElem && sndElem->has("path"))
				mScrollSound = sndElem->get<std::string>("path");
		}
	}

	if (properties & ALIGNMENT)
	{
		if (elem->has("alignment"))
		{
			const std::string& str = elem->get<std::string>("alignment");
			if (str == "left")
				setAlignment(ALIGN_LEFT);
			else if (str == "center")
				setAlignment(ALIGN_CENTER);
			else if (str == "right")
				setAlignment(ALIGN_RIGHT);
			else
				LOG(LogError) << "Unknown TextListComponent alignment \"" << str << "\"!";
		}

		if (elem->has("horizontalMargin"))
		{
			mHorizontalMargin = elem->get<float>("horizontalMargin") *
				(this->mParent ? this->mParent->getSize().x() : (float)Renderer::getScreenWidth());
		}
	}

	if (elem->has("orientation"))
	{
		const std::string& str = elem->get<std::string>("orientation");
		if (str == "horizontal")
			setOrientation(ORIENTATION_HORIZONTAL);
		else if (str == "vertical")
			setOrientation(ORIENTATION_VERTICAL);
		else
		{
			LOG(LogWarning) << "Unknown TextListComponent orientation \"" << str << "\"! Using vertical.";
			setOrientation(ORIENTATION_VERTICAL);
		}
	}
	else
	{
		setOrientation(ORIENTATION_VERTICAL);
	}

	mCarouselMode = false;
	if (elem->has("carouselMode"))
		mCarouselMode = elem->get<bool>("carouselMode");

	mCarouselLoop = false;
	if (elem->has("carouselLoop"))
		mCarouselLoop = elem->get<bool>("carouselLoop");

	mCarouselMaxLogoCount = 5;
	if (elem->has("maxLogoCount"))
		mCarouselMaxLogoCount = (int)std::round(elem->get<float>("maxLogoCount"));

	mCarouselLogoScale = 1.20f;
	if (elem->has("logoScale"))
		mCarouselLogoScale = elem->get<float>("logoScale");

	mCarouselMinLogoOpacity = 0.45f;
	if (elem->has("minLogoOpacity"))
		mCarouselMinLogoOpacity = elem->get<float>("minLogoOpacity");

	mCarouselScaledLogoSpacing = 0.0f;
	if (elem->has("scaledLogoSpacing"))
		mCarouselScaledLogoSpacing = elem->get<float>("scaledLogoSpacing");

	mCarouselLogoSpacingX = 0.0f;
	if (elem->has("logoSpacingX"))
	{
		float scale = this->mParent ? this->mParent->getSize().x() : (float)Renderer::getScreenWidth();
		mCarouselLogoSpacingX = elem->get<float>("logoSpacingX") * scale;
	}

	mCarouselLogoSize = Vector2f::Zero();
	if (elem->has("logoSize"))
	{
		Vector2f logoSize = elem->get<Vector2f>("logoSize");
		mCarouselLogoSize = Vector2f(
			logoSize.x() * Renderer::getScreenWidth(),
			logoSize.y() * Renderer::getScreenHeight());
	}

	mCarouselLogoAlignment = CAROUSEL_ALIGN_CENTER;
	if (elem->has("logoAlignment"))
	{
		const std::string& str = elem->get<std::string>("logoAlignment");

		if (str == "left")
			mCarouselLogoAlignment = CAROUSEL_ALIGN_LEFT;
		else if (str == "right")
			mCarouselLogoAlignment = CAROUSEL_ALIGN_RIGHT;
		else if (str == "top")
			mCarouselLogoAlignment = CAROUSEL_ALIGN_TOP;
		else if (str == "bottom")
			mCarouselLogoAlignment = CAROUSEL_ALIGN_BOTTOM;
		else
			mCarouselLogoAlignment = CAROUSEL_ALIGN_CENTER;
	}

	mCarouselLogoOffsetX = 0.0f;
	mCarouselLogoOffsetY = 0.0f;

	if (elem->has("logoOffset"))
	{
		Vector2f v = elem->get<Vector2f>("logoOffset");
		mCarouselLogoOffsetX = v.x() * Renderer::getScreenWidth();
		mCarouselLogoOffsetY = v.y() * Renderer::getScreenHeight();
	}

	if (elem->has("logoOffsetX"))
		mCarouselLogoOffsetX = elem->get<float>("logoOffsetX") * Renderer::getScreenWidth();

	if (elem->has("logoOffsetY"))
		mCarouselLogoOffsetY = elem->get<float>("logoOffsetY") * Renderer::getScreenHeight();

	mCarouselSelectedLogoOffsetX = 0.0f;
	mCarouselSelectedLogoOffsetY = 0.0f;

	if (elem->has("selectedLogoOffsetX"))
		mCarouselSelectedLogoOffsetX = elem->get<float>("selectedLogoOffsetX") * Renderer::getScreenWidth();

	if (elem->has("selectedLogoOffsetY"))
		mCarouselSelectedLogoOffsetY = elem->get<float>("selectedLogoOffsetY") * Renderer::getScreenHeight();

	mCarouselItemColor = 0x00000088;
	if (elem->has("carouselItemColor"))
		mCarouselItemColor = elem->get<unsigned int>("carouselItemColor");

	mCarouselSelectedItemColor = 0x101020CC;
	if (elem->has("carouselSelectedItemColor"))
		mCarouselSelectedItemColor = elem->get<unsigned int>("carouselSelectedItemColor");

	mCarouselTextMaxLines = 2;
	if (elem->has("carouselTextMaxLines"))
		mCarouselTextMaxLines = (int)std::round(elem->get<float>("carouselTextMaxLines"));

	if (mCarouselTextMaxLines < 1)
		mCarouselTextMaxLines = 1;
	if (mCarouselTextMaxLines > 3)
		mCarouselTextMaxLines = 3;

	if (properties & FORCE_UPPERCASE && elem->has("forceUppercase"))
		setUppercase(elem->get<bool>("forceUppercase"));

	if (elem->has("showRowNumbers"))
		setShowRowNumbers(elem->get<bool>("showRowNumbers"));

	if (properties & LINE_SPACING)
	{
		if (elem->has("lineSpacing"))
			setLineSpacing(elem->get<float>("lineSpacing"));

		if (elem->has("selectorHeight"))
			setSelectorHeight(elem->get<float>("selectorHeight") * Renderer::getScreenHeight());

		if (elem->has("selectorOffsetY"))
		{
			float scale = this->mParent ? this->mParent->getSize().y() : (float)Renderer::getScreenHeight();
			setSelectorOffsetY(elem->get<float>("selectorOffsetY") * scale);
		}
		else
		{
			setSelectorOffsetY(0.0f);
		}

		if (elem->has("selectorWidth"))
		{
			setSelectorWidth(elem->get<float>("selectorWidth") * Renderer::getScreenWidth());
		}
		else
		{
			setSelectorWidth(0.0f);
		}

		if (elem->has("selectorHorizontalOffset"))
		{
			float scale = this->mParent ? this->mParent->getSize().x() : (float)Renderer::getScreenWidth();
			setSelectorHorizontalOffset(elem->get<float>("selectorHorizontalOffset") * scale);
		}
		else
		{
			setSelectorHorizontalOffset(0.0f);
		}
	}

	if (elem->has("selectorImagePath"))
	{
		std::string path = elem->get<std::string>("selectorImagePath");
		bool tile = elem->has("selectorImageTile") && elem->get<bool>("selectorImageTile");
		mSelectorImage.setImage(path, tile);
		mSelectorImage.setSize((mSelectorWidth > 0.0f) ? mSelectorWidth : mSize.x(), mSelectorHeight);
		mSelectorImage.setColorShift(mSelectorColor);
		mSelectorImage.setColorShiftEnd(mSelectorColorEnd);
	}
	else
	{
		mSelectorImage.setImage("");
	}
}

#endif // ES_APP_COMPONENTS_TEXT_LIST_COMPONENT_H
