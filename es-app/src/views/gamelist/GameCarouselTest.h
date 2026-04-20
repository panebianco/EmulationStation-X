#include "FileData.h"
#include "GuiComponent.h"
#include "components/ImageComponent.h"
#include "components/TextComponent.h"
#include "resources/Font.h"
#include "renderers/Renderer.h"
#include "math/Misc.h"

#include <vector>
#include <memory>
#include <cmath>

// Prototipo simple de carrusel visual para gamelist.
// No reemplaza la lista real: solo dibuja tarjetas vecinas usando el cursor actual.
// Pensado como base de prueba antes de integrarlo a VideoGameListView o DetailedGameListView.

class GameCarouselTest
{
public:
	struct Item
	{
		FileData* game = nullptr;
		std::shared_ptr<GuiComponent> visual;
	};

	GameCarouselTest(Window* window)
		: mWindow(window)
		, mPosition(0.25f * Renderer::getScreenWidth(), 0.18f * Renderer::getScreenHeight())
		, mSize(0.50f * Renderer::getScreenWidth(), 0.32f * Renderer::getScreenHeight())
		, mItemSize(0.18f * Renderer::getScreenWidth(), 0.22f * Renderer::getScreenHeight())
		, mVisibleItems(5)
		, mSelectedScale(1.22f)
		, mSideOpacity(0.45f)
		, mSpacing(0.22f * Renderer::getScreenWidth())
		, mCursor(0)
	{
	}

	void setEntries(const std::vector<FileData*>& entries)
	{
		mEntries.clear();
		for (auto* game : entries)
		{
			Item item;
			item.game = game;
			item.visual = createVisual(game);
			mEntries.emplace_back(item);
		}

		if (mCursor >= static_cast<int>(mEntries.size()))
			mCursor = 0;
	}

	void setCursor(int cursor)
	{
		if (mEntries.empty())
		{
			mCursor = 0;
			return;
		}

		while (cursor < 0)
			cursor += static_cast<int>(mEntries.size());
		while (cursor >= static_cast<int>(mEntries.size()))
			cursor -= static_cast<int>(mEntries.size());

		mCursor = cursor;
	}

	int getCursor() const
	{
		return mCursor;
	}

	void render(const Transform4x4f& parentTrans)
	{
		if (mEntries.empty())
			return;

		Transform4x4f trans = parentTrans;
		trans.translate(Vector3f(mPosition.x(), mPosition.y(), 0.0f));

		Renderer::pushClipRect(
			Vector2i((int)mPosition.x(), (int)mPosition.y()),
			Vector2i((int)mSize.x(), (int)mSize.y()));

		Renderer::setMatrix(trans);
		Renderer::drawRect(0.0f, 0.0f, mSize.x(), mSize.y(), 0x00000000, 0x00000000);

		std::vector<int> drawLast;
		const int half = mVisibleItems / 2;

		for (int offset = -half; offset <= half; ++offset)
		{
			int index = wrapIndex(mCursor + offset);
			Item& item = mEntries[index];
			if (!item.visual)
				continue;

			if (offset == 0)
				drawLast.push_back(index);
			else
				renderItem(trans, item, offset);
		}

		for (int index : drawLast)
			renderItem(trans, mEntries[index], 0);

		Renderer::popClipRect();
	}

private:
	std::shared_ptr<GuiComponent> createVisual(FileData* game)
	{
		if (!game)
			return nullptr;

		const std::string imagePath =
			!game->getImagePath().empty() ? game->getImagePath() :
			!game->getThumbnailPath().empty() ? game->getThumbnailPath() :
			!game->getMarqueePath().empty() ? game->getMarqueePath() : "";

		if (!imagePath.empty())
		{
			auto image = std::make_shared<ImageComponent>(mWindow, false, true);
			image->setOrigin(0.5f, 0.5f);
			image->setResize(mItemSize.x(), mItemSize.y());
			image->setImage(imagePath);
			image->setRotateByTargetSize(true);
			return image;
		}

		auto text = std::make_shared<TextComponent>(
			mWindow,
			game->getName(),
			Font::get(FONT_SIZE_SMALL),
			0xFFFFFFFF,
			ALIGN_CENTER);

		text->setOrigin(0.5f, 0.5f);
		text->setSize(mItemSize.x(), mItemSize.y());
		text->setRenderBackground(true);
		text->setBackgroundColor(0x202020DD);
		return text;
	}

	void renderItem(const Transform4x4f& baseTrans, Item& item, int offset)
	{
		if (!item.visual)
			return;

		const float distance = (float)std::abs(offset);
		const float centerX = mSize.x() * 0.5f;
		const float centerY = mSize.y() * 0.5f;
		const float x = centerX + (offset * mSpacing);
		const float y = centerY;

		float scale = 1.0f;
		if (offset == 0)
			scale = mSelectedScale;
		else if (distance == 1.0f)
			scale = 0.86f;
		else
			scale = 0.72f;

		float opacity = 1.0f;
		if (offset != 0)
			opacity = (distance == 1.0f) ? 0.70f : mSideOpacity;

		Transform4x4f itemTrans = baseTrans;
		itemTrans.translate(Vector3f(x, y, 0.0f));

		item.visual->setPosition(0.0f, 0.0f, 0.0f);
		item.visual->setScale(scale);
		item.visual->setOpacity((unsigned char)(opacity * 255.0f));
		item.visual->render(itemTrans);
	}

	int wrapIndex(int index) const
	{
		if (mEntries.empty())
			return 0;

		while (index < 0)
			index += static_cast<int>(mEntries.size());
		while (index >= static_cast<int>(mEntries.size()))
			index -= static_cast<int>(mEntries.size());
		return index;
	}

private:
	Window* mWindow;
	std::vector<Item> mEntries;
	Vector2f mPosition;
	Vector2f mSize;
	Vector2f mItemSize;
	int mVisibleItems;
	float mSelectedScale;
	float mSideOpacity;
	float mSpacing;
	int mCursor;
};
