// es-app/src/guis/GuiThemeBrowser.h
#pragma once

#include "GuiComponent.h"

#include "components/ComponentList.h"
#include "components/ImageComponent.h"
#include "components/TextComponent.h"
#include "components/NinePatchComponent.h"

#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <functional>
#include <memory>

class BusyComponent;

class GuiThemeBrowser : public GuiComponent
{
public:
	GuiThemeBrowser(Window* window);
	~GuiThemeBrowser() override;

	bool input(InputConfig* config, Input input) override;
	void update(int deltaTime) override;
	void render(const Transform4x4f& parentTrans) override;

	// Help prompts (barra inferior del sistema)
	std::vector<HelpPrompt> getHelpPrompts() override;
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

	// Catálogo (snapshot ZIP)
	bool updateCatalogZipSilent();
	bool hasCommand(const std::string& cmd) const;

	void loadThemes();
	void rebuildList();
	void updatePreview();
	void updateFooter();
	std::string getPreviewPath(const ThemeEntry& e) const;

	bool isInstalled(const ThemeEntry& e) const;

	// Temas: snapshot (sin .git). Update = reinstalar limpio.
	bool downloadOrUpdateSilent(const ThemeEntry& e);
	bool uninstallThemeSilent(const ThemeEntry& e);

	int runCmd(const std::string& cmd);

	// popup helper (Window::setInfoPopup espera InfoPopup*)
	void showPopup(const std::string& msg, int durationMs);

	// Jobs (thread) para no congelar la UI
	void startJob(const std::string& busyMsg, std::function<bool()> fn,
	              const std::string& okMsg, const std::string& failMsg,
	              bool refreshCatalogAfter);
	void finishJobIfDone();

	void setBusyVisible(bool v);

private:
	NinePatchComponent mFrame;

	ComponentList  mList;
	ImageComponent mPreview;
	TextComponent  mRepoLabel;
	TextComponent  mHeader;
	TextComponent  mFooter;

	std::vector<ThemeEntry> mThemes;

	// Busy overlay
	std::unique_ptr<BusyComponent> mBusy;
	bool mBusyVisible;

	// Job thread state
	std::thread mJobThread;
	std::atomic<bool> mJobRunning;
	std::atomic<bool> mJobDone;
	bool mJobOk;
	bool mJobRefreshCatalogAfter;
	std::string mJobDoneMsg;
	std::mutex mJobMutex;

	// Paths
	std::string mCatalogDir;   // ~/.emulationstation/esx/theme-catalog
	std::string mPreviewDir;   // <catalog>/theme-previews
	std::string mThemesDir;    // ~/.emulationstation/themes

	int mLastSelectedIndex;

	// layout cache
	float mInnerX;
	float mInnerY;
	float mInnerW;
	float mInnerH;
};
