#include "views/gamelist/ISimpleGameListView.h"

#include "views/UIModeController.h"
#include "views/ViewController.h"
#include "CollectionSystemManager.h"
#include "Scripting.h"
#include "Settings.h"
#include "Sound.h"
#include "SystemData.h"
#include "LocaleES.h"   // ← NUEVO: soporte de idioma

ISimpleGameListView::ISimpleGameListView(Window* window, FileData* root) 
	: IGameListView(window, root),
	mHeaderText(window), mHeaderImage(window), mBackground(window)
{
	// Localización del texto por defecto del encabezado
	LocaleES& loc = LocaleES::getInstance();
	loc.loadFromSettings();

	mHeaderText.setText(loc.translate("LOGO TEXT")); // ← Traducción desde .ini
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

	// Remove old theme extras
	for (auto extra : mThemeExtras)
	{
		removeChild(extra);
		delete extra;
	}
	mThemeExtras.clear();

	// Add new theme extras
	mThemeExtras = ThemeData::makeExtras(theme, getName(), mWindow);
	for (auto extra : mThemeExtras)
	{
		addChild(extra);
	}

	// Mostrar imagen si está disponible, sino texto
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
	if (!cursor->isPlaceHolder()) {
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
	if (input.value != 0)
	{
		if(config->isMappedTo("a", input))
		{
			FileData* cursor = getCursor();
			if(cursor->getType() == GAME)
			{
				Sound::getFromTheme(getTheme(), getName(), "launch")->play();
				launch(cursor);
			}
			else
			{
				if(cursor->getChildren().size() > 0)
				{
					mCursorStack.push(cursor);
					populateList(cursor->getChildrenListToDisplay());
					setCursor(getCursor());
				}
			}

			return true;
		}
		else if(config->isMappedTo("b", input))
		{
			if(mCursorStack.size())
			{
				populateList(mCursorStack.top()->getParent()->getChildrenListToDisplay());
				setCursor(mCursorStack.top());
				mCursorStack.pop();
				Sound::getFromTheme(getTheme(), getName(), "back")->play();
			}
			else
			{
				onFocusLost();
				SystemData* systemToView = getCursor()->getSystem();
				if (systemToView->isCollection())
				{
					systemToView = CollectionSystemManager::get()->getSystemToView(systemToView);
				}
				ViewController::get()->goToSystemView(systemToView);
			}

			return true;
		}
		else if(config->isMappedLike(getQuickSystemSelectRightButton(), input))
		{
			if(Settings::getInstance()->getBool("QuickSystemSelect"))
			{
				onFocusLost();
				ViewController::get()->goToNextGameList();
				return true;
			}
		}
		else if(config->isMappedLike(getQuickSystemSelectLeftButton(), input))
		{
			if(Settings::getInstance()->getBool("QuickSystemSelect"))
			{
				onFocusLost();
				ViewController::get()->goToPrevGameList();
				return true;
			}
		}
		else if (config->isMappedTo("x", input))
		{
			if (mRoot->getSystem()->isGameSystem())
			{
				FileData* randomGame = getCursor()->getSystem()->getRandomGame();
				if (randomGame)
				{
					setCursor(randomGame);
				}
				return true;
			}
		}
		else if (config->isMappedTo("y", input) && !UIModeController::getInstance()->isUIModeKid())
		{
			if(mRoot->getSystem()->isGameSystem())
			{
				if (CollectionSystemManager::get()->toggleGameInCollection(getCursor()))
				{
					return true;
				}
			}
		}
	}

	FileData* cursor = getCursor();
	SystemData* system = this->mRoot->getSystem();
    if (system != NULL) {
        Scripting::fireEvent("game-select", system->getName(), cursor->getPath(), cursor->getName(), "input");
    }
	else
	{
	    Scripting::fireEvent("game-select", "NULL", "NULL", "NULL", "input");
	}
	return IGameListView::input(config, input);
}
