#pragma once
#ifndef ES_APP_GUIS_GUI_SKYSCRAPER_MENU_H
#define ES_APP_GUIS_GUI_SKYSCRAPER_MENU_H

#include "GuiComponent.h"
#include "components/MenuComponent.h"
#include "components/TextComponent.h"
#include "components/OptionListComponent.h"
#include "HelpStyle.h"
#include "HelpPrompt.h"

#include <memory>
#include <string>
#include <vector>

class GuiSkyscraperMenu : public GuiComponent
{
public:
	GuiSkyscraperMenu(Window* window);
	~GuiSkyscraperMenu() {}

	bool input(InputConfig* config, Input input) override;
	std::vector<HelpPrompt> getHelpPrompts() override;
	HelpStyle getHelpStyle() override;

private:
	MenuComponent mMenu;

	std::shared_ptr<OptionListComponent<std::string>> mArtworkType;
	std::shared_ptr<OptionListComponent<std::string>> mDownloadVideos;

	void runGather();
	void runGenerate();
	void openLog();

	std::string getCurrentSystem();
	std::string getCurrentLanguage();
	std::string getSelectedArtworkType();
	std::string getSelectedVideos();

	int launchScript(const std::string& action);
};

#endif
