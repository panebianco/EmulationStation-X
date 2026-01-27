// es-app/src/guis/GuiThemeBrowser.h
#pragma once

#include "GuiComponent.h"

#include "components/ComponentList.h"
#include "components/ImageComponent.h"
#include "components/TextComponent.h"
#include "components/NinePatchComponent.h"

#include <string>
#include <vector>

class GuiThemeBrowser : public GuiComponent
{
public:
	GuiThemeBrowser(Window* window);

	bool input(InputConfig* config, Input input) override;
	void update(int deltaTime) override;
	void render(const Transform4x4f& parentTrans) override;

	// Help prompts (barra inferior del sistema)
	std::vector<HelpPrompt> getHelpPrompts();
	void updateHelpPrompts();

private:
	struct ThemeEntry
	{
		std::string id;
		std::string title;
		std::string repo;
		std::string previewHint;
		std::string folder;  // ~/.emulationstation/themes/<folder>
		std::string author;  // NUEVO
	};

	// Catálogo (repo con themes.ini + previews)
	bool updateCatalog();

	void loadThemes();
	void rebuildList();
	void updatePreview();
	void updateFooter();
	std::string getPreviewPath(const ThemeEntry& e) const;

	bool isInstalled(const ThemeEntry& e) const;
	bool downloadOrUpdate(const ThemeEntry& e);
	bool uninstallTheme(const ThemeEntry& e);

	int runCmd(const std::string& cmd);

	// popup helper (Window::setInfoPopup espera InfoPopup*)
	void showPopup(const std::string& msg, int durationMs);

private:
	NinePatchComponent mFrame;

	ComponentList  mList;
	ImageComponent mPreview;
	TextComponent  mRepoLabel;
	TextComponent  mHeader;
	TextComponent  mFooter;

	std::vector<ThemeEntry> mThemes;

	// OJO: ahora el previewDir apunta al repo catálogo clonado
	std::string mCatalogDir;
	std::string mPreviewDir;
	std::string mThemesDir;

	int mLastSelectedIndex;

	// layout cache
	float mInnerX;
	float mInnerY;
	float mInnerW;
	float mInnerH;
};
