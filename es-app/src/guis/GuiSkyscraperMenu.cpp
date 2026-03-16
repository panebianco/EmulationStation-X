#include "guis/GuiSkyscraperMenu.h"

#include "components/TextComponent.h"
#include "guis/GuiInfoPopup.h"
#include "guis/GuiMsgBox.h"
#include "views/ViewController.h"
#include "SystemData.h"
#include "Settings.h"
#include "Log.h"
#include "resources/Font.h"
#include "renderers/Renderer.h"
#include "LocaleES.h"

#include <cstdlib>

#define SKY_SCRIPT "$HOME/.emulationstation/scripts/skyscraper-esx.sh"

namespace
{
	inline unsigned int getMenuTextColor()
	{
		return Settings::getInstance()->getBool("MenuDark") ? 0xFFFFFFFF : 0x777777FF;
	}

	inline std::string quote(const std::string& s)
	{
		return std::string("\"") + s + "\"";
	}

	inline std::string tr(const std::string& key)
	{
		return es_translate(key);
	}
}

GuiSkyscraperMenu::GuiSkyscraperMenu(Window* window)
	: GuiComponent(window)
	, mMenu(window, es_translate("SKYSCRAPER").c_str())
{
	std::shared_ptr<Font> font = Font::get(FONT_SIZE_MEDIUM);
	const unsigned int textColor = getMenuTextColor();

	// ---- GATHER ----
	{
		ComponentListRow row;

		auto txt = std::make_shared<TextComponent>(window, tr("GATHER RESOURCES"), font, textColor);
		row.addElement(txt, true);

		row.makeAcceptInputHandler([this] { runGather(); });

		mMenu.addRow(row);
	}

	// ---- GENERATE ----
	{
		ComponentListRow row;

		auto txt = std::make_shared<TextComponent>(window, tr("GENERATE GAMELIST"), font, textColor);
		row.addElement(txt, true);

		row.makeAcceptInputHandler([this] { runGenerate(); });

		mMenu.addRow(row);
	}

	// ---- LOG ----
	{
		ComponentListRow row;

		auto txt = std::make_shared<TextComponent>(window, tr("OPEN LOG"), font, textColor);
		row.addElement(txt, true);

		row.makeAcceptInputHandler([this] { openLog(); });

		mMenu.addRow(row);
	}

	addChild(&mMenu);

	setSize(mMenu.getSize());
	setPosition(
		(Renderer::getScreenWidth() - mSize.x()) / 2,
		(Renderer::getScreenHeight() - mSize.y()) / 2
	);
}

bool GuiSkyscraperMenu::input(InputConfig* config, Input input)
{
	if (GuiComponent::input(config, input))
		return true;

	if (config->isMappedTo("b", input) && input.value)
	{
		delete this;
		return true;
	}

	return false;
}

std::string GuiSkyscraperMenu::getCurrentSystem()
{
	auto vc = ViewController::get();
	if (!vc)
		return "nes";

	auto state = vc->getState();
	SystemData* sys = state.getSystem();

	if (!sys)
		return "nes";

	return sys->getName();
}

std::string GuiSkyscraperMenu::getCurrentLanguage()
{
	std::string lang = Settings::getInstance()->getString("Language");

	if (lang.empty())
		return "en";

	return lang;
}

int GuiSkyscraperMenu::launchScript(const std::string& action)
{
	const std::string system = getCurrentSystem();
	const std::string lang   = getCurrentLanguage();

	const std::string cmd =
		std::string(SKY_SCRIPT) +
		" " + quote(action) +
		" " + quote(system) +
		" " + quote(lang);

	LOG(LogInfo) << "Launching Skyscraper: " << cmd;

	return std::system(cmd.c_str());
}

void GuiSkyscraperMenu::runGather()
{
	const std::string system = getCurrentSystem();

	mWindow->pushGui(new GuiMsgBox(
	mWindow,
	tr("SKYSCRAPER_CONFIRM"),
	tr("CONTINUE"),
		[this, system]
		{
			mWindow->setInfoPopup(new GuiInfoPopup(
				mWindow,
				tr("WORKING... THIS MAY TAKE SEVERAL MINUTES."),
				3000
			));

			const int ret = launchScript("gather");

			if (ret == 0)
			{
				mWindow->setInfoPopup(new GuiInfoPopup(
					mWindow,
					tr("SKYSCRAPER GATHER FINISHED: ") + system,
					5000
				));
			}
			else
			{
				mWindow->setInfoPopup(new GuiInfoPopup(
					mWindow,
					tr("SKYSCRAPER GATHER FAILED: ") + system,
					5000
				));
			}
		},
		tr("CANCEL"),
		nullptr
	));
}

void GuiSkyscraperMenu::runGenerate()
{
	const std::string system = getCurrentSystem();

	mWindow->setInfoPopup(new GuiInfoPopup(
		mWindow,
		tr("GENERATING GAMELIST... PLEASE WAIT A MOMENT."),
		2500
	));

	const int ret = launchScript("generate");

	if (ret == 0)
	{
		mWindow->setInfoPopup(new GuiInfoPopup(
			mWindow,
			tr("SKYSCRAPER GENERATE FINISHED: ") + system,
			5000
		));
	}
	else
	{
		mWindow->setInfoPopup(new GuiInfoPopup(
			mWindow,
			tr("SKYSCRAPER GENERATE FAILED: ") + system,
			5000
		));
	}
}

void GuiSkyscraperMenu::openLog()
{
	mWindow->setInfoPopup(new GuiInfoPopup(
		mWindow,
		tr("LOG: /TMP/ESX-SKYSCRAPER/LOG"),
		3000
	));
}

HelpStyle GuiSkyscraperMenu::getHelpStyle()
{
	HelpStyle style;

	auto vc = ViewController::get();
	if (vc && vc->getState().getSystem())
		style.applyTheme(vc->getState().getSystem()->getTheme(), "system");

	return style;
}

std::vector<HelpPrompt> GuiSkyscraperMenu::getHelpPrompts()
{
	return {
		{ "up/down", tr("CHOOSE") },
		{ "a", tr("SELECT") },
		{ "b", tr("BACK") }
	};
}
