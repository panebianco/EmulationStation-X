#pragma once

#include "GuiComponent.h"

#include "components/ComponentList.h"
#include "components/ImageComponent.h"
#include "components/TextComponent.h"
#include "components/NinePatchComponent.h"
#include "components/HelpComponent.h"

#include <string>
#include <vector>
#include <memory>

class GuiThemeBrowser : public GuiComponent
{
public:
	explicit GuiThemeBrowser(Window* window);

	bool input(InputConfig* config, Input input) override;
	void update(int deltaTime) override;
	void render(const Transform4x4f& parentTrans) override;

private:
	struct ThemeEntry
	{
		std::string id;
		std::string title;
		std::string repo;
		std::string previewHint;
		std::string folder; // ~/.emulationstation/themes/<folder>
	};

	void loadThemes();
	void rebuildList();
	void updatePreview();
	std::string getPreviewPath(const ThemeEntry& e) const;

	bool isInstalled(const ThemeEntry& e) const;
	bool downloadOrUpdate(const ThemeEntry& e);
	bool uninstallTheme(const ThemeEntry& e);

	int runCmd(const std::string& cmd);

	// popup helper (Window::setInfoPopup espera InfoPopup*)
	void showPopup(const std::string& msg, int durationMs);

private:
	// Frame “ventana”
	NinePatchComponent mFrame;

	// UI
	ComponentList  mList;
	ImageComponent mPreview;
	TextComponent  mHeader;

	// Barra de ayuda (A/X/B)
	std::shared_ptr<HelpComponent> mHelp;

	// Data
	std::vector<ThemeEntry> mThemes;

	std::string mPreviewDir;
	std::string mThemesDir;
	int mLastSelectedIndex;
};
