#include "guis/GuiThemeBrowser.h"

#include "Log.h"
#include "Window.h"
#include "resources/Font.h"
#include "utils/FileSystemUtil.h"
#include "renderers/Renderer.h"
#include "guis/GuiInfoPopup.h"

#include <fstream>
#include <sstream>
#include <cstdlib>

// ------------------------------------------------------------
// helpers
// ------------------------------------------------------------
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

// ------------------------------------------------------------
// ctor
// ------------------------------------------------------------
GuiThemeBrowser::GuiThemeBrowser(Window* window)
	: GuiComponent(window)
	, mFrame(window)
	, mList(window)
	, mPreview(window)
	, mHeader(window, "Theme Downloader", Font::get(FONT_SIZE_LARGE), 0xFFFFFFFF, ALIGN_CENTER)
	, mPreviewDir("")
	, mThemesDir("")
	, mLastSelectedIndex(0)
{
	const std::string home = Utils::FileSystem::getHomePath();
	mPreviewDir = home + "/.emulationstation/esx/theme-previews";
	mThemesDir  = home + "/.emulationstation/themes";

	const float sw = (float)Renderer::getScreenWidth();
	const float sh = (float)Renderer::getScreenHeight();

	// ---------------- frame ----------------
	mFrame.setImagePath(":/frame_dark.png");

	const float marginX = sw * 0.05f;
	const float marginY = sh * 0.06f;

	mFrame.setPosition(Vector3f(marginX, marginY, 0.0f));
	mFrame.setSize(Vector2f(sw - marginX * 2.0f, sh - marginY * 2.0f));
	mFrame.setCornerSize(Vector2f(16.0f, 16.0f));
	mFrame.setOpacity(0.95f);

	// ---------------- layout ----------------
	const float innerX = marginX + sw * 0.02f;
	const float innerY = marginY + sh * 0.02f;
	const float innerW = (sw - marginX * 2.0f) - sw * 0.04f;
	const float innerH = (sh - marginY * 2.0f) - sh * 0.04f;

	mHeader.setPosition(innerX, innerY);
	mHeader.setSize(innerW, 40.0f);

	const float topPad = 55.0f;

	mList.setPosition(innerX, innerY + topPad);
	mList.setSize(innerW * 0.46f, innerH - topPad);

	mPreview.setPosition(innerX + innerW * 0.50f, innerY + topPad + 10.0f);
	mPreview.setResize(innerW * 0.46f, innerH * 0.68f);
	mPreview.setOrigin(0.0f, 0.0f);

	loadThemes();
	rebuildList();
	updatePreview();
}

// ------------------------------------------------------------
// popup
// ------------------------------------------------------------
void GuiThemeBrowser::showPopup(const std::string& msg, int durationMs)
{
	mWindow->setInfoPopup(new GuiInfoPopup(mWindow, msg, durationMs));
}

// ------------------------------------------------------------
// load themes.ini
// ------------------------------------------------------------
void GuiThemeBrowser::loadThemes()
{
	mThemes.clear();

	const std::string iniPath = mPreviewDir + "/themes.ini";
	if (!fileExists(iniPath))
	{
		LOG(LogWarning) << "GuiThemeBrowser: themes.ini not found: " << iniPath;
		return;
	}

	std::ifstream f(iniPath);
	if (!f.is_open())
	{
		LOG(LogError) << "GuiThemeBrowser: failed to open: " << iniPath;
		return;
	}

	ThemeEntry current;
	bool inSection = false;

	auto flushSection = [&]()
	{
		if (!inSection || current.id.empty())
			return;

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
		if (line.empty() || line[0] == ';' || line[0] == '#')
			continue;

		if (line.front() == '[' && line.back() == ']')
		{
			flushSection();
			inSection = true;
			current.id = trim(line.substr(1, line.size() - 2));
			continue;
		}

		const size_t eq = line.find('=');
		if (eq == std::string::npos || !inSection)
			continue;

		const std::string key = trim(line.substr(0, eq));
		const std::string val = trim(line.substr(eq + 1));

		if      (key == "title")        current.title = val;
		else if (key == "repo")         current.repo = val;
		else if (key == "preview_hint") current.previewHint = val;
		else if (key == "folder")       current.folder = val;
	}

	flushSection();

	if (mLastSelectedIndex < 0 || mLastSelectedIndex >= (int)mThemes.size())
		mLastSelectedIndex = 0;
}

// ------------------------------------------------------------
bool GuiThemeBrowser::isInstalled(const ThemeEntry& e) const
{
	return Utils::FileSystem::exists(mThemesDir + "/" + e.folder);
}

// ------------------------------------------------------------
void GuiThemeBrowser::rebuildList()
{
	mList.clear();

	if (mThemes.empty())
	{
		ComponentListRow row;
		row.addElement(std::make_shared<TextComponent>(
			mWindow,
			"No themes found (themes.ini vacío?)",
			Font::get(FONT_SIZE_MEDIUM),
			0xFFFFFFFF,
			ALIGN_LEFT), true);

		mList.addRow(row, true);
		return;
	}

	for (int i = 0; i < (int)mThemes.size(); i++)
	{
		const ThemeEntry& e = mThemes[i];

		ComponentListRow row;

		std::string label = e.title;
		if (isInstalled(e))
			label += "  [INSTALLED]";

		row.addElement(std::make_shared<TextComponent>(
			mWindow,
			label,
			Font::get(FONT_SIZE_MEDIUM),
			0xFFFFFFFF,
			ALIGN_LEFT), true);

		if (!e.repo.empty())
		{
			row.addElement(std::make_shared<TextComponent>(
				mWindow,
				e.repo,
				Font::get(FONT_SIZE_SMALL),
				0xAAAAAAFF,
				ALIGN_LEFT), false);
		}

		mList.addRow(row, i == mLastSelectedIndex);
	}

	updatePreview();
}

// ------------------------------------------------------------
std::string GuiThemeBrowser::getPreviewPath(const ThemeEntry& e) const
{
	const std::string base = e.previewHint.empty() ? e.id : e.previewHint;

	const std::string p1 = mPreviewDir + "/" + base + ".png";
	const std::string p2 = mPreviewDir + "/" + base + ".jpg";
	const std::string p3 = mPreviewDir + "/" + base + ".jpeg";

	if (fileExists(p1)) return p1;
	if (fileExists(p2)) return p2;
	if (fileExists(p3)) return p3;

	return "";
}

// ------------------------------------------------------------
void GuiThemeBrowser::updatePreview()
{
	if (mThemes.empty())
	{
		mHeader.setText("Theme Downloader");
		mPreview.setImage("");
		return;
	}

	const ThemeEntry& e = mThemes[mLastSelectedIndex];
	mHeader.setText(e.title);

	const std::string p = getPreviewPath(e);
	mPreview.setImage(p.empty() ? "" : p);
}

// ------------------------------------------------------------
int GuiThemeBrowser::runCmd(const std::string& cmd)
{
	LOG(LogInfo) << "GuiThemeBrowser cmd: " << cmd;
	return std::system(cmd.c_str());
}

// ------------------------------------------------------------
bool GuiThemeBrowser::downloadOrUpdate(const ThemeEntry& e)
{
	if (e.repo.empty())
	{
		showPopup("No repo URL for this theme.", 4000);
		return false;
	}

	Utils::FileSystem::createDirectory(mThemesDir);

	const std::string dst = mThemesDir + "/" + e.folder;
	const bool installed = isInstalled(e);

	showPopup(installed ? "Updating theme..." : "Downloading theme...", 2000);

	int rc = installed
		? runCmd("git -C " + quote(dst) + " pull --ff-only")
		: runCmd("git clone --depth 1 " + quote(e.repo) + " " + quote(dst));

	if (rc == 0)
	{
		showPopup("Theme ready.", 4000);
		return true;
	}

	showPopup("Download/Update failed.", 5000);
	return false;
}

// ------------------------------------------------------------
bool GuiThemeBrowser::uninstallTheme(const ThemeEntry& e)
{
	const std::string dst = mThemesDir + "/" + e.folder;

	if (!isInstalled(e))
	{
		showPopup("Theme is not installed.", 3000);
		return false;
	}

	const int rc = runCmd("rm -rf " + quote(dst));

	if (rc == 0 && !Utils::FileSystem::exists(dst))
	{
		showPopup("Theme removed.", 4000);
		return true;
	}

	showPopup("Remove failed.", 5000);
	return false;
}

// ------------------------------------------------------------
bool GuiThemeBrowser::input(InputConfig* config, Input input)
{
	if (input.value == 0)
		return false;

	// Back
	if (config->isMappedTo("b", input) || config->isMappedTo("back", input))
	{
		mWindow->removeGui(this);
		return true;
	}

	if (config->isMappedTo("up", input))
	{
		mLastSelectedIndex = (mLastSelectedIndex - 1 + mThemes.size()) % mThemes.size();
		rebuildList();
		return true;
	}

	if (config->isMappedTo("down", input))
	{
		mLastSelectedIndex = (mLastSelectedIndex + 1) % mThemes.size();
		rebuildList();
		return true;
	}

	// A = download/update
	if (config->isMappedTo("a", input) || config->isMappedTo("start", input))
	{
		downloadOrUpdate(mThemes[mLastSelectedIndex]);
		rebuildList();
		return true;
	}

	// X = uninstall
	if (config->isMappedTo("x", input) || config->isMappedTo("delete", input))
	{
		uninstallTheme(mThemes[mLastSelectedIndex]);
		rebuildList();
		return true;
	}

	return false;
}

// ------------------------------------------------------------
void GuiThemeBrowser::update(int deltaTime)
{
	GuiComponent::update(deltaTime);
	mFrame.update(deltaTime);
	mList.update(deltaTime);
	mPreview.update(deltaTime);
	mHeader.update(deltaTime);
}

// ------------------------------------------------------------
void GuiThemeBrowser::render(const Transform4x4f& parentTrans)
{
	Transform4x4f trans = parentTrans * getTransform();
	Renderer::setMatrix(trans);

	mFrame.render(trans);
	mHeader.render(trans);
	mList.render(trans);
	mPreview.render(trans);
}
