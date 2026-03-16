#include "guis/GuiSkyscraperMenu.h"

#include "components/TextComponent.h"
#include "guis/GuiInfoPopup.h"
#include "views/ViewController.h"
#include "SystemData.h"
#include "Settings.h"
#include "Log.h"
#include "resources/Font.h"
#include "renderers/Renderer.h"

#include <cstdlib>

#define SKY_SCRIPT "$HOME/.emulationstation/scripts/skyscraper-esx.sh"

namespace
{
	inline unsigned int getMenuTextColor()
	{
		return Settings::getInstance()->getBool("MenuDark") ? 0xFFFFFFFF : 0x777777FF;
	}
}

GuiSkyscraperMenu::GuiSkyscraperMenu(Window* window)
	: GuiComponent(window)
	, mMenu(window, "SKYSCRAPER")
{
	std::shared_ptr<Font> font = Font::get(FONT_SIZE_MEDIUM);
	const unsigned int textColor = getMenuTextColor();

	// ---- GATHER ----
	{
		ComponentListRow row;

		auto txt = std::make_shared<TextComponent>(window, "Gather resources", font, textColor);
		row.addElement(txt, true);

		row.makeAcceptInputHandler([this] { runGather(); });

		mMenu.addRow(row);
	}

	// ---- GENERATE ----
	{
		ComponentListRow row;

		auto txt = std::make_shared<TextComponent>(window, "Generate gamelist", font, textColor);
		row.addElement(txt, true);

		row.makeAcceptInputHandler([this] { runGenerate(); });

		mMenu.addRow(row);
	}

	// ---- LOG ----
	{
		ComponentListRow row;

		auto txt = std::make_shared<TextComponent>(window, "Open log", font, textColor);
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
	auto state = ViewController::get()->getState();
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
	std::string system = getCurrentSystem();
	std::string lang = getCurrentLanguage();

	std::string cmd =
		std::string(SKY_SCRIPT) +
		" " + action +
		" " + system +
		" " + lang;

	LOG(LogInfo) << "Launching Skyscraper: " << cmd;

	return std::system(cmd.c_str());
}

void GuiSkyscraperMenu::runGather()
{
	const std::string system = getCurrentSystem();

	mWindow->setInfoPopup(new GuiInfoPopup(
		mWindow,
		"Running Skyscraper gather: " + system,
		2000
	));

	const int ret = launchScript("gather");

	if (ret == 0)
	{
		mWindow->setInfoPopup(new GuiInfoPopup(
			mWindow,
			"Skyscraper gather finished: " + system,
			5000
		));
	}
	else
	{
		mWindow->setInfoPopup(new GuiInfoPopup(
			mWindow,
			"Skyscraper gather failed: " + system,
			5000
		));
	}
}

void GuiSkyscraperMenu::runGenerate()
{
	const std::string system = getCurrentSystem();

	mWindow->setInfoPopup(new GuiInfoPopup(
		mWindow,
		"Running Skyscraper generate: " + system,
		2000
	));

	const int ret = launchScript("generate");

	if (ret == 0)
	{
		mWindow->setInfoPopup(new GuiInfoPopup(
			mWindow,
			"Skyscraper generate finished: " + system,
			5000
		));
	}
	else
	{
		mWindow->setInfoPopup(new GuiInfoPopup(
			mWindow,
			"Skyscraper generate failed: " + system,
			5000
		));
	}
}

void GuiSkyscraperMenu::openLog()
{
	std::system("nano /tmp/esx-skyscraper/log");
}

HelpStyle GuiSkyscraperMenu::getHelpStyle()
{
	HelpStyle style;
	style.applyTheme(ViewController::get()->getState().getSystem()->getTheme(), "system");
	return style;
}

std::vector<HelpPrompt> GuiSkyscraperMenu::getHelpPrompts()
{
	return {
		{ "up/down", "CHOOSE" },
		{ "a", "SELECT" },
		{ "b", "BACK" }
	};
}