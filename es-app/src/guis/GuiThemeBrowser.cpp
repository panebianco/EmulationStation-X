// es-app/src/guis/GuiThemeBrowser.cpp
#include "guis/GuiThemeBrowser.h"

#include "Log.h"
#include "Window.h"
#include "resources/Font.h"
#include "utils/FileSystemUtil.h"
#include "renderers/Renderer.h"

// Busy overlay (ya existe y ya anima busy_0..3.svg)
#include "components/BusyComponent.h"

// Popup concreto (implementa Window::InfoPopup)
#include "guis/GuiInfoPopup.h"

#include <fstream>
#include <sstream>
#include <cstdlib>

static inline std::string trim(const std::string& s)
{
	size_t b = s.find_first_not_of(" \t\r\n");
	if (b == std::string::npos) return "";
	size_t e = s.find_last_not_of(" \t\r\n");
	return s.substr(b, (e - b) + 1);
}

static inline bool fileExists(const std::string& path)
{
	return Utils::FileSystem::exists(path);
}

static inline std::string quote(const std::string& s)
{
	return std::string("\"") + s + "\"";
}

static inline std::string ellipsize(const std::string& s, size_t maxLen)
{
	if (s.size() <= maxLen) return s;
	if (maxLen <= 3) return s.substr(0, maxLen);
	return s.substr(0, maxLen - 3) + "...";
}

GuiThemeBrowser::GuiThemeBrowser(Window* window)
	: GuiComponent(window)
	, mFrame(window)
	, mList(window)
	, mPreview(window)
	, mRepoLabel(window, "", Font::get(FONT_SIZE_SMALL), 0x777777CC, ALIGN_LEFT)
	, mHeader(window, "Theme Downloader", Font::get(FONT_SIZE_LARGE), 0xFFFFFFFF, ALIGN_CENTER)
	, mFooter(window, "", Font::get(FONT_SIZE_SMALL), 0xFFFFFFFF, ALIGN_CENTER)
	, mCatalogDir("")
	, mPreviewDir("")
	, mThemesDir("")
	, mLastSelectedIndex(0)
	, mInnerX(0.0f)
	, mInnerY(0.0f)
	, mInnerW(0.0f)
	, mInnerH(0.0f)
	, mBusy(nullptr)
	, mBusyVisible(false)
	, mJobRunning(false)
	, mJobDone(false)
	, mJobOk(false)
	, mJobRefreshCatalogAfter(false)
	, mJobDoneMsg("")
{
	const std::string home = Utils::FileSystem::getHomePath();

	// Catálogo local (snapshot ZIP) en ~/.emulationstation/esx/theme-catalog
	mCatalogDir = home + "/.emulationstation/esx/theme-catalog";

	// Dentro del catálogo: theme-previews/themes.ini + previews
	mPreviewDir = mCatalogDir + "/theme-previews";

	// Temas instalados
	mThemesDir  = home + "/.emulationstation/themes";

	const float sw = (float)Renderer::getScreenWidth();
	const float sh = (float)Renderer::getScreenHeight();

	// ============================================================
	// Frame general
	// ============================================================
	mFrame.setImagePath(":/frame_dark.png");

	const float marginX = sw * 0.05f;
	const float marginY = sh * 0.06f;

	mFrame.setPosition(Vector3f(marginX, marginY, 0.0f));
	mFrame.setSize(Vector2f(sw - (marginX * 2.0f), sh - (marginY * 2.0f)));
	mFrame.setCornerSize(Vector2f(16.0f, 16.0f));
	mFrame.setOpacity(0.95f);

	// ============================================================
	// Layout interno
	// ============================================================
	mInnerX = marginX + (sw * 0.02f);
	mInnerY = marginY + (sh * 0.02f);
	mInnerW = (sw - (marginX * 2.0f)) - (sw * 0.04f);
	mInnerH = (sh - (marginY * 2.0f)) - (sh * 0.04f);

	// Header
	mHeader.setPosition(mInnerX, mInnerY);
	mHeader.setSize(mInnerW, 40.0f);

	// Footer
	mFooter.setPosition(mInnerX, mInnerY + mInnerH - 32.0f);
	mFooter.setSize(mInnerW, 28.0f);

	const float topPad = 55.0f;
	const float bottomPad = 40.0f;

	// Lista izquierda
	mList.setPosition(mInnerX, mInnerY + topPad);
	mList.setSize(mInnerW * 0.46f, (mInnerH - topPad - bottomPad));

	// Panel derecho
	const float rightX = mInnerX + (mInnerW * 0.50f);
	const float rightY = mInnerY + topPad + 10.0f;
	const float rightW = (mInnerW * 0.46f);
	const float rightH = (mInnerH - topPad - bottomPad);

	// Preview (arriba)
	mPreview.setPosition(rightX, rightY);
	mPreview.setResize(rightW, rightH * 0.82f);
	mPreview.setOrigin(0.0f, 0.0f);

	// Repo debajo de la imagen
	const float repoY = rightY + (rightH * 0.82f) + 8.0f;
	mRepoLabel.setPosition(rightX, repoY);
	mRepoLabel.setSize(rightW, rightH * 0.16f);

	// ============================================================
	// Busy overlay (como CHILD para que no “duerma” sin input)
	// ============================================================
	mBusy.reset(new BusyComponent(mWindow));
	mBusy->setPosition(Vector3f(0.0f, 0.0f, 0.0f));
	mBusy->setSize(Vector2f(sw, sh));
	addChild(mBusy.get());         // ✅ clave: update/render como parte del árbol
	setBusyVisible(false);

	// ============================================================
	// Cargar lo que haya ya en disco (sin bloquear)
	// ============================================================
	loadThemes();
	rebuildList();

	// Este menú “maneja” la lista manualmente
	mList.onFocusGained();

	updatePreview();
	updateFooter();
	updateHelpPrompts();

	// ============================================================
	// Actualizar catálogo en background (sin congelar UI)
	// ============================================================
	startJob("Updating theme catalog...",
		[this]() { return updateCatalogZipSilent(); },
		"Catalog ready.",
		"Catalog update failed.",
		true /*refreshCatalogAfter*/);
}

GuiThemeBrowser::~GuiThemeBrowser()
{
	if (mJobThread.joinable())
		mJobThread.join();
}

void GuiThemeBrowser::setBusyVisible(bool v)
{
	mBusyVisible = v;
	if (mBusy)
		mBusy->setVisible(v);   // ✅ nativo, ya existe en tu GuiComponent
}

void GuiThemeBrowser::showPopup(const std::string& msg, int durationMs)
{
	mWindow->setInfoPopup(new GuiInfoPopup(mWindow, msg, durationMs));
}

int GuiThemeBrowser::runCmd(const std::string& cmd)
{
	LOG(LogInfo) << "GuiThemeBrowser cmd: " << cmd;
	return std::system(cmd.c_str());
}

bool GuiThemeBrowser::hasCommand(const std::string& cmd) const
{
	const std::string c = "sh -c \"command -v " + cmd + " >/dev/null 2>&1\"";
	return (std::system(c.c_str()) == 0);
}

// ============================================================
// Catálogo ZIP (SILENT)
// ============================================================
bool GuiThemeBrowser::updateCatalogZipSilent()
{
	const std::string zipUrl = "https://github.com/Renetrox/esx-theme-catalog/archive/refs/heads/main.zip";

	const bool hasUnzip = hasCommand("unzip");
	const bool hasCurl  = hasCommand("curl");
	const bool hasWget  = hasCommand("wget");

	if (!hasUnzip || (!hasCurl && !hasWget))
	{
		LOG(LogWarning) << "GuiThemeBrowser: missing deps for catalog zip update. unzip=" << hasUnzip
		                << " curl=" << hasCurl << " wget=" << hasWget;
		return false;
	}

	const std::string esxDir = Utils::FileSystem::getParent(mCatalogDir);
	Utils::FileSystem::createDirectory(esxDir);

	const std::string zipPath = esxDir + "/theme-catalog.zip";
	const std::string extracted = esxDir + "/esx-theme-catalog-main";

	std::string dlCmd;
	if (hasCurl)
		dlCmd = "curl -L -o " + quote(zipPath) + " " + quote(zipUrl);
	else
		dlCmd = "wget -O " + quote(zipPath) + " " + quote(zipUrl);

	int rc = runCmd(dlCmd);
	if (rc != 0 || !Utils::FileSystem::exists(zipPath))
		return false;

	runCmd("rm -rf " + quote(extracted));
	runCmd("rm -rf " + quote(mCatalogDir));

	rc = runCmd("unzip -q -o " + quote(zipPath) + " -d " + quote(esxDir));
	runCmd("rm -f " + quote(zipPath));

	if (rc != 0 || !Utils::FileSystem::exists(extracted))
		return false;

	rc = runCmd("mv " + quote(extracted) + " " + quote(mCatalogDir));
	if (rc != 0 || !Utils::FileSystem::exists(mCatalogDir))
		return false;

	return true;
}

void GuiThemeBrowser::loadThemes()
{
	mThemes.clear();

	const std::string iniPath = mPreviewDir + "/themes.ini";
	if (!fileExists(iniPath))
		return;

	std::ifstream f(iniPath);
	if (!f.is_open())
		return;

	ThemeEntry current;
	bool inSection = false;

	auto flushSection = [&]()
	{
		if (!inSection) return;
		if (current.id.empty()) return;

		if (current.title.empty())  current.title  = current.id;
		if (current.folder.empty()) current.folder = current.id;

		mThemes.push_back(current);

		current = ThemeEntry{};
		inSection = false;
	};

	std::string line;
	while (std::getline(f, line))
	{
		line = trim(line);
		if (line.empty()) continue;
		if (line[0] == ';' || line[0] == '#') continue;

		if (line.front() == '[' && line.back() == ']')
		{
			flushSection();
			inSection = true;
			current.id = trim(line.substr(1, line.size() - 2));
			continue;
		}

		const size_t eq = line.find('=');
		if (eq == std::string::npos) continue;
		if (!inSection) continue;

		const std::string key = trim(line.substr(0, eq));
		const std::string val = trim(line.substr(eq + 1));

		if (key == "title") current.title = val;
		else if (key == "repo") current.repo = val;
		else if (key == "preview_hint") current.previewHint = val;
		else if (key == "folder") current.folder = val;
		else if (key == "author") current.author = val;
	}

	flushSection();

	if (mLastSelectedIndex < 0) mLastSelectedIndex = 0;
	if (mLastSelectedIndex >= (int)mThemes.size()) mLastSelectedIndex = 0;
}

bool GuiThemeBrowser::isInstalled(const ThemeEntry& e) const
{
	const std::string dst = mThemesDir + "/" + e.folder;
	return Utils::FileSystem::exists(dst);
}

void GuiThemeBrowser::rebuildList()
{
	mList.clear();

	if (mThemes.empty())
	{
		ComponentListRow row;
		auto txt = std::make_shared<TextComponent>(
			mWindow, "No themes found (themes.ini empty?)",
			Font::get(FONT_SIZE_MEDIUM), 0xFFFFFFFF, ALIGN_LEFT
		);

		row.addElement(txt, false);
		mList.addRow(row, true);
		mLastSelectedIndex = 0;
		return;
	}

	for (int i = 0; i < (int)mThemes.size(); i++)
	{
		const ThemeEntry& e = mThemes[i];
		const bool installed = isInstalled(e);

		ComponentListRow row;

		std::string label = installed ? "☑  " : "☐  ";
		label += (e.title.empty() ? e.id : e.title);

		auto title = std::make_shared<TextComponent>(
			mWindow, label,
			Font::get(FONT_SIZE_MEDIUM), 0xFFFFFFFF, ALIGN_LEFT
		);

		row.addElement(title, false);

		row.makeAcceptInputHandler([this, i]()
		{
			mLastSelectedIndex = i;
			updatePreview();
			updateFooter();
			updateHelpPrompts();
		});

		mList.addRow(row, (i == mLastSelectedIndex));
	}

	updatePreview();
	updateFooter();
	updateHelpPrompts();
}

std::string GuiThemeBrowser::getPreviewPath(const ThemeEntry& e) const
{
	std::string base = e.previewHint.empty() ? e.id : e.previewHint;

	const std::string p1 = mPreviewDir + "/" + base + ".png";
	if (fileExists(p1)) return p1;

	const std::string p2 = mPreviewDir + "/" + base + ".jpg";
	if (fileExists(p2)) return p2;

	const std::string p3 = mPreviewDir + "/" + base + ".jpeg";
	if (fileExists(p3)) return p3;

	return "";
}

void GuiThemeBrowser::updatePreview()
{
	if (mThemes.empty())
	{
		mHeader.setText("Theme Downloader");
		mPreview.setImage("");
		mRepoLabel.setText("");
		return;
	}

	if (mLastSelectedIndex < 0) mLastSelectedIndex = 0;
	if (mLastSelectedIndex >= (int)mThemes.size()) mLastSelectedIndex = 0;

	const ThemeEntry& e = mThemes[mLastSelectedIndex];

	mHeader.setText(e.title.empty() ? e.id : e.title);

	const std::string preview = getPreviewPath(e);
	mPreview.setImage(preview.empty() ? "" : preview);

	std::string repoLine = e.repo.empty() ? "" : ellipsize(e.repo, 70);
	std::string authorLine = e.author.empty() ? "" : ("by " + e.author);

	if (!repoLine.empty() && !authorLine.empty())
		mRepoLabel.setText(repoLine + "\n" + authorLine);
	else if (!repoLine.empty())
		mRepoLabel.setText(repoLine);
	else
		mRepoLabel.setText(authorLine);
}

void GuiThemeBrowser::updateFooter()
{
	if (mThemes.empty())
	{
		mFooter.setText("B Back");
		return;
	}

	const ThemeEntry& e = mThemes[mLastSelectedIndex];
	const bool installed = isInstalled(e);

	std::string s = "A ";
	s += installed ? "Update" : "Download";
	if (installed) s += "    X Uninstall";
	s += "    B Back";

	mFooter.setText(s);
}

bool GuiThemeBrowser::downloadOrUpdateSilent(const ThemeEntry& e)
{
	if (e.repo.empty())
		return false;

	Utils::FileSystem::createDirectory(mThemesDir);

	const std::string dst = mThemesDir + "/" + e.folder;
	const bool installed = Utils::FileSystem::exists(dst);

	if (installed)
		runCmd("rm -rf " + quote(dst));

	const std::string cmd = "git clone --depth 1 " + quote(e.repo) + " " + quote(dst);
	int rc = runCmd(cmd);

	if (rc == 0)
	{
		runCmd("rm -rf " + quote(dst + "/.git"));
		return true;
	}

	return false;
}

bool GuiThemeBrowser::uninstallThemeSilent(const ThemeEntry& e)
{
	const std::string dst = mThemesDir + "/" + e.folder;

	if (!Utils::FileSystem::exists(dst))
		return false;

	const std::string cmd = "rm -rf " + quote(dst);
	const int rc = runCmd(cmd);

	return (rc == 0 && !Utils::FileSystem::exists(dst));
}

void GuiThemeBrowser::startJob(const std::string& busyMsg, std::function<bool()> fn,
                              const std::string& okMsg, const std::string& failMsg,
                              bool refreshCatalogAfter)
{
	if (mJobRunning)
		return;

	mJobRunning = true;
	mJobDone = false;
	mJobOk = false;
	mJobRefreshCatalogAfter = refreshCatalogAfter;

	setBusyVisible(true);
	showPopup(busyMsg, 1200);

	if (mJobThread.joinable())
		mJobThread.join();

	mJobThread = std::thread([this, fn, okMsg, failMsg]() {
		const bool ok = fn();

		{
			std::lock_guard<std::mutex> lk(mJobMutex);
			mJobOk = ok;
			mJobDoneMsg = ok ? okMsg : failMsg;
		}

		mJobDone = true;
		mJobRunning = false;
	});
}

void GuiThemeBrowser::finishJobIfDone()
{
	if (!mJobDone)
		return;

	if (mJobThread.joinable())
		mJobThread.join();

	setBusyVisible(false);

	std::string msg;
	bool ok = false;
	bool refreshCatalog = false;

	{
		std::lock_guard<std::mutex> lk(mJobMutex);
		msg = mJobDoneMsg;
		ok = mJobOk;
		refreshCatalog = mJobRefreshCatalogAfter;
	}

	showPopup(msg, ok ? 2500 : 4500);

	if (refreshCatalog)
	{
		loadThemes();
		rebuildList();
	}
	else
	{
		rebuildList();
	}

	mJobDone = false;
}

bool GuiThemeBrowser::input(InputConfig* config, Input input)
{
	if (input.value == 0)
		return false;

	if (mJobRunning)
		return true;

	if (config->isMappedTo("b", input) || config->isMappedTo("back", input))
	{
		mWindow->removeGui(this);
		return true;
	}

	if (mThemes.empty())
		return GuiComponent::input(config, input);

	if (config->isMappedTo("up", input))
	{
		mLastSelectedIndex--;
		if (mLastSelectedIndex < 0) mLastSelectedIndex = (int)mThemes.size() - 1;
		rebuildList();
		return true;
	}

	if (config->isMappedTo("down", input))
	{
		mLastSelectedIndex++;
		if (mLastSelectedIndex >= (int)mThemes.size()) mLastSelectedIndex = 0;
		rebuildList();
		return true;
	}

	if (config->isMappedTo("a", input) || config->isMappedTo("start", input))
	{
		const ThemeEntry cur = mThemes[mLastSelectedIndex];
		const bool installed = isInstalled(cur);

		startJob(installed ? "Updating theme..." : "Downloading theme...",
			[this, cur]() { return downloadOrUpdateSilent(cur); },
			"Theme ready.",
			"Download/Update failed.",
			false);

		return true;
	}

	if (config->isMappedTo("x", input) || config->isMappedTo("delete", input))
	{
		const ThemeEntry cur = mThemes[mLastSelectedIndex];

		if (!isInstalled(cur))
		{
			showPopup("Not installed.", 2500);
			return true;
		}

		startJob("Removing theme...",
			[this, cur]() { return uninstallThemeSilent(cur); },
			"Theme removed.",
			"Remove failed.",
			false);

		return true;
	}

	if (mList.input(config, input))
		return true;

	return GuiComponent::input(config, input);
}

void GuiThemeBrowser::update(int deltaTime)
{
	// ✅ Actualiza children (incluyendo Busy) por estar en el árbol
	GuiComponent::update(deltaTime);

	mFrame.update(deltaTime);
	mList.update(deltaTime);
	mPreview.update(deltaTime);
	mRepoLabel.update(deltaTime);
	mHeader.update(deltaTime);
	mFooter.update(deltaTime);

	finishJobIfDone();
}

void GuiThemeBrowser::render(const Transform4x4f& parentTrans)
{
	Transform4x4f trans = parentTrans * getTransform();
	Renderer::setMatrix(trans);

	mFrame.render(trans);
	mHeader.render(trans);
	mList.render(trans);
	mPreview.render(trans);
	mRepoLabel.render(trans);
	mFooter.render(trans);

	// ✅ Renderizar children (Busy incluido, y respeta setVisible)
	renderChildren(trans);
}

// ============================================================
// Help prompts
// ============================================================
std::vector<HelpPrompt> GuiThemeBrowser::getHelpPrompts()
{
	std::vector<HelpPrompt> p;

	p.push_back(HelpPrompt("up/down", "Move"));

	const bool hasThemes = !mThemes.empty();
	const bool installed = (hasThemes ? isInstalled(mThemes[mLastSelectedIndex]) : false);

	p.push_back(HelpPrompt("a", installed ? "Update" : "Download"));

	if (installed)
		p.push_back(HelpPrompt("x", "Uninstall"));

	p.push_back(HelpPrompt("b", "Back"));

	return p;
}

void GuiThemeBrowser::updateHelpPrompts()
{
	HelpStyle style;
	mWindow->setHelpPrompts(getHelpPrompts(), style);
}
