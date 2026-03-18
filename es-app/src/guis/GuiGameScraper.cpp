#include "guis/GuiGameScraper.h"

#include "components/ButtonComponent.h"
#include "components/MenuComponent.h"
#include "components/TextComponent.h"
#include "FileData.h"
#include "PowerSaver.h"
#include "SystemData.h"
#include "Settings.h"

namespace
{
	inline unsigned int getPrimaryTextColor()
	{
		return Settings::getInstance()->getBool("MenuDark") ? 0xFFFFFFFF : 0x777777FF;
	}

	inline unsigned int getSecondaryTextColor()
	{
		return Settings::getInstance()->getBool("MenuDark") ? 0xBBBBBBFF : 0x888888FF;
	}

	inline const char* getFramePath()
	{
		return Settings::getInstance()->getBool("MenuDark") ? ":/frame_dark.png" : ":/frame.png";
	}
}

GuiGameScraper::GuiGameScraper(Window* window, ScraperSearchParams params, std::function<void(const ScraperSearchResult&)> doneFunc)
	: GuiComponent(window),
	  mGrid(window, Vector2i(1, 7)),
	  mBox(window, getFramePath()),
	  mSearchParams(params),
	  mClose(false)
{
	PowerSaver::pause();
	addChild(&mBox);
	addChild(&mGrid);

	// row 0 spacer

	mGameName = std::make_shared<TextComponent>(
		mWindow,
		Utils::String::toUpper(Utils::FileSystem::getFileName(mSearchParams.game->getPath())),
		Font::get(FONT_SIZE_MEDIUM),
		getPrimaryTextColor(),
		ALIGN_CENTER
	);
	mGrid.setEntry(mGameName, Vector2i(0, 1), false, true);

	// row 2 spacer

	mSystemName = std::make_shared<TextComponent>(
		mWindow,
		Utils::String::toUpper(mSearchParams.system->getFullName()),
		Font::get(FONT_SIZE_SMALL),
		getSecondaryTextColor(),
		ALIGN_CENTER
	);
	mGrid.setEntry(mSystemName, Vector2i(0, 3), false, true);

	// row 4 spacer

	// ScraperSearchComponent
	mSearch = std::make_shared<ScraperSearchComponent>(window, ScraperSearchComponent::NEVER_AUTO_ACCEPT);
	mGrid.setEntry(mSearch, Vector2i(0, 5), true);

	// buttons
	std::vector<std::shared_ptr<ButtonComponent>> buttons;

	buttons.push_back(std::make_shared<ButtonComponent>(mWindow, "INPUT", "search", [&] {
		mSearch->openInputScreen(mSearchParams);
		mGrid.resetCursor();
	}));

	buttons.push_back(std::make_shared<ButtonComponent>(mWindow, "CANCEL", "cancel", [&] {
		delete this;
	}));

	mButtonGrid = makeButtonGrid(mWindow, buttons);
	mGrid.setEntry(mButtonGrid, Vector2i(0, 6), true, false);

	// callbacks seguros
	mSearch->setAcceptCallback([this, doneFunc](const ScraperSearchResult& result) {
		doneFunc(result);
		close();
	});

	mSearch->setCancelCallback([&] {
		delete this;
	});

	setSize(Renderer::getScreenWidth() * 0.95f, Renderer::getScreenHeight() * 0.747f);
	setPosition(
		(Renderer::getScreenWidth() - mSize.x()) / 2,
		(Renderer::getScreenHeight() - mSize.y()) / 2
	);

	mGrid.resetCursor();
	mSearch->search(params);
}

void GuiGameScraper::onSizeChanged()
{
	mBox.fitTo(mSize, Vector3f::Zero(), Vector2f(-32, -32));

	mGrid.setRowHeightPerc(0, 0.04f, false);
	mGrid.setRowHeightPerc(1, mGameName->getFont()->getLetterHeight() / mSize.y(), false);
	mGrid.setRowHeightPerc(2, 0.04f, false);
	mGrid.setRowHeightPerc(3, mSystemName->getFont()->getLetterHeight() / mSize.y(), false);
	mGrid.setRowHeightPerc(4, 0.04f, false);
	mGrid.setRowHeightPerc(6, mButtonGrid->getSize().y() / mSize.y(), false);

	mGrid.setSize(mSize);
}

bool GuiGameScraper::input(InputConfig* config, Input input)
{
	if (config->isMappedTo("b", input) && input.value)
	{
		PowerSaver::resume();
		delete this;
		return true;
	}

	return GuiComponent::input(config, input);
}

void GuiGameScraper::update(int deltaTime)
{
	GuiComponent::update(deltaTime);

	if (mClose)
		delete this;
}

std::vector<HelpPrompt> GuiGameScraper::getHelpPrompts()
{
	return mGrid.getHelpPrompts();
}

void GuiGameScraper::close()
{
	mClose = true;
}
