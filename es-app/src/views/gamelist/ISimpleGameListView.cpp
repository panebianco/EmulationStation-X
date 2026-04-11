#include "views/gamelist/ISimpleGameListView.h"

#include "views/UIModeController.h"
#include "views/ViewController.h"
#include "CollectionSystemManager.h"
#include "Scripting.h"
#include "Settings.h"
#include "Sound.h"
#include "SystemData.h"
#include "LocaleES.h"
#include "NavigationSounds.h"

ISimpleGameListView::ISimpleGameListView(Window* window, FileData* root)
	: IGameListView(window, root),
	mHeaderText(window), mHeaderImage(window), mBackground(window)
{
	LocaleES& loc = LocaleES::getInstance();
	loc.loadFromSettings();

	mHeaderText.setText(loc.translate("LOGO TEXT"));
	mHeaderText.setSize(mSize.x(), 0);
	mHeaderText.setPosition(0, 0);
	mHeaderText.setHorizontalAlignment(ALIGN_CENTER);
	mHeaderText.setDefaultZIndex(50);

	mHeaderImage.setResize(0, mSize.y() * 0.185f);
	mHeaderImage.setOrigin(0.5f, 0.0f);
	mHeaderImage.setPosition(mSize.x() / 2, 0);
	mHeaderImage.setDefaultZIndex(50);

	mBackground.setResize(mSize.x(), mSize.y());
	mBackground.setDefaultZIndex(0);

	addChild(&mHeaderText);
	addChild(&mBackground);
}

void ISimpleGameListView::onThemeChanged(const std::shared_ptr<ThemeData>& theme)
{
	using namespace ThemeFlags;
	mBackground.applyTheme(theme, getName(), "background", ALL);
	mHeaderImage.applyTheme(theme, getName(), "logo", ALL);
	mHeaderText.applyTheme(theme, getName(), "logoText", ALL);

	for (auto extra : mThemeExtras)
	{
		removeChild(extra);
		delete extra;
	}
	mThemeExtras.clear();

	mThemeExtras = ThemeData::makeExtras(theme, getName(), mWindow);
	for (auto extra : mThemeExtras)
		addChild(extra);

	if (mHeaderImage.hasImage())
	{
		removeChild(&mHeaderText);
		addChild(&mHeaderImage);
	}
	else
	{
		addChild(&mHeaderText);
		removeChild(&mHeaderImage);
	}
}

void ISimpleGameListView::onFileChanged(FileData* /*file*/, FileChangeType /*change*/)
{
	FileData* cursor = getCursor();
	if (!cursor->isPlaceHolder())
	{
		populateList(cursor->getParent()->getChildrenListToDisplay());
		setCursor(cursor);
	}
	else
	{
		populateList(mRoot->getChildrenListToDisplay());
		setCursor(cursor);
	}
}

bool ISimpleGameListView::input(InputConfig* config, Input input)
{
	bool horizontalGameList = false;
	const std::shared_ptr<ThemeData>& activeTheme = getTheme();
	if (activeTheme)
	{
		const ThemeData::ThemeElement* gamelistElem = activeTheme->getElement(getName(), "gamelist", "textlist");
		if (gamelistElem && gamelistElem->has("orientation"))
		{
			const std::string orientation = gamelistElem->get<std::string>("orientation");
			horizontalGameList = (orientation == "horizontal");
		}
	}

	if (input.value != 0)
	{
		if (config->isMappedTo("a", input))
		{
			FileData* cursor = getCursor();
			if (cursor->getType() == GAME)
			{
				std::shared_ptr<Sound> launchSnd;

				SystemData* sys = mRoot ? mRoot->getSystem() : nullptr;
				if (sys != nullptr)
				{
					const std::shared_ptr<ThemeData>& theme = sys->getTheme();
					if (theme)
						launchSnd = NavigationSounds::getFromTheme(theme, "launch");
				}

				if (!launchSnd)
					launchSnd = Sound::getFromTheme(getTheme(), getName(), "launch");

				if (launchSnd)
					launchSnd->play();

				launch(cursor);
			}
			else
			{
				if (cursor->getChildren().size() > 0)
				{
					mCursorStack.push(cursor);
					populateList(cursor->getChildrenListToDisplay());
					setCursor(getCursor());
				}
			}

			return true;
		}
		else if (config->isMappedTo("b", input))
		{
			std::shared_ptr<Sound> backSnd;
			SystemData* sys = mRoot ? mRoot->getSystem() : nullptr;
			if (sys != nullptr)
			{
				const std::shared_ptr<ThemeData>& theme = sys->getTheme();
				if (theme)
					backSnd = NavigationSounds::getFromTheme(theme, "back");
			}

			if (!backSnd)
				backSnd = Sound::getFromTheme(getTheme(), getName(), "back");

			if (backSnd)
				backSnd->play();

			if (mCursorStack.size())
			{
				populateList(mCursorStack.top()->getParent()->getChildrenListToDisplay());
				setCursor(mCursorStack.top());
				mCursorStack.pop();
			}
			else
			{
				onFocusLost();
				SystemData* systemToView = getCursor()->getSystem();
				if (systemToView->isCollection())
					systemToView = CollectionSystemManager::get()->getSystemToView(systemToView);

				ViewController::get()->goToSystemView(systemToView);
			}

			return true;
		}
		else if (config->isMappedTo("x", input))
		{
			if (mRoot->getSystem()->isGameSystem())
			{
				FileData* randomGame = getCursor()->getSystem()->getRandomGame();
				if (randomGame)
					setCursor(randomGame);

				return true;
			}
		}
		else if (config->isMappedTo("y", input) && !UIModeController::getInstance()->isUIModeKid())
		{
			if (mRoot->getSystem()->isGameSystem())
			{
				if (CollectionSystemManager::get()->toggleGameInCollection(getCursor()))
				{
					SystemData* sys = mRoot ? mRoot->getSystem() : nullptr;
					if (sys != nullptr)
					{
						const std::shared_ptr<ThemeData>& theme = sys->getTheme();
						if (theme)
						{
							auto favSnd = NavigationSounds::getFromTheme(theme, "favorite");
							if (!favSnd)
								favSnd = NavigationSounds::getFromTheme(theme, "select");
							if (favSnd)
								favSnd->play();
						}
					}
					return true;
				}
			}
		}
		else
		{
			// Quick system select
			if (Settings::getInstance()->getBool("QuickSystemSelect"))
			{
				bool nextSystem = false;
				bool prevSystem = false;

				if (horizontalGameList)
				{
					// En gamelist horizontal: L/R cambian sistemas
					nextSystem = config->isMappedLike("rightshoulder", input);
					prevSystem = config->isMappedLike("leftshoulder", input);
				}
				else
				{
					// Comportamiento normal
					nextSystem = config->isMappedLike(getQuickSystemSelectRightButton(), input);
					prevSystem = config->isMappedLike(getQuickSystemSelectLeftButton(), input);
				}

				if (nextSystem || prevSystem)
				{
					SystemData* sys = mRoot ? mRoot->getSystem() : nullptr;
					if (sys != nullptr)
					{
						const std::shared_ptr<ThemeData>& theme = sys->getTheme();
						if (theme)
						{
							auto snd = NavigationSounds::getFromTheme(theme, "quicksysselect");
							if (!snd)
								snd = NavigationSounds::getFromTheme(theme, "systembrowse");
							if (snd)
								snd->play();
						}
					}

					onFocusLost();

					if (nextSystem)
						ViewController::get()->goToNextGameList();
					else
						ViewController::get()->goToPrevGameList();

					return true;
				}
			}
		}
	}

	FileData* cursor = getCursor();
	SystemData* currentSystem = this->mRoot->getSystem();
	if (currentSystem != NULL)
	{
		Scripting::fireEvent("game-select", currentSystem->getName(), cursor->getPath(), cursor->getName(), "input");
	}
	else
	{
		Scripting::fireEvent("game-select", "NULL", "NULL", "NULL", "input");
	}

	return IGameListView::input(config, input);
}
