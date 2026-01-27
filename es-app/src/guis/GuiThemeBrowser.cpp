// es-app/src/guis/GuiThemeBrowser.cpp
#include "guis/GuiThemeBrowser.h"

#include "Log.h"
#include "Window.h"
#include "resources/Font.h"
#include "utils/FileSystemUtil.h"
#include "renderers/Renderer.h"

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
{
	const std::string home = Utils::FileSystem::getHomePath();

	// Catálogo git local (vive en ~/.emulationstation/esx/theme-catalog)
	mCatalogDir = home + "/.emulationstation/esx/theme-catalog";

	// Dentro del repo: theme-previews/themes.ini + previews png/jpg
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

	// ✅ CLAVE: actualizar catálogo al abrir (sin UI extra rara)
	updateCatalog();

	loadThemes();
	rebuildList();

	// este menú “maneja” la lista manualmente
	mList.onFocusGained();

	updatePreview();
	updateFooter();
	updateHelpPrompts();
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

// ============================================================
// Catálogo: git clone / pull al abrir
// ============================================================
bool GuiThemeBrowser::updateCatalog()
{
	// ⚠️ Poné aquí el repo del catálogo (themes.ini + previews)
	const std::string catalogRepo = "https://github.com/Renetrox/esx-theme-catalog";

	// Si no querés catálogo remoto todavía, dejá el repo vacío
	if (catalogRepo.empty())
		return false;

	// Asegurar carpeta padre: ~/.emulationstation/esx
	Utils::FileSystem::createDirectory(Utils::FileSystem::getParent(mCatalogDir));

	const bool hasGit = Utils::FileSystem::exists(mCatalogDir + "/.git");
	const bool exists = Utils::FileSystem::exists(mCatalogDir);

	if (!exists)
	{
		showPopup("Updating theme catalog...", 2500);
		const std::string cmd = "git clone --depth 1 " + quote(catalogRepo) + " " + quote(mCatalogDir);
		const int rc = runCmd(cmd);
		if (rc != 0)
		{
			showPopup("Catalog download failed.", 4000);
			return false;
		}
		return true;
	}

	// Existe pero no es repo: no tocar (estabilidad)
	if (exists && !hasGit)
	{
		LOG(LogWarning) << "GuiThemeBrowser: catalog folder exists but is not a git repo: " << mCatalogDir;
		return false;
	}

	// Pull rápido y seguro
	showPopup("Updating theme catalog...", 2500);
	const std::string pullCmd = "git -C " + quote(mCatalogDir) + " pull --ff-only";
	int rc = runCmd(pullCmd);

	// Fallback ultra estable: si pull falla, reinstala limpio
	if (rc != 0)
	{
		LOG(LogWarning) << "GuiThemeBrowser: catalog pull failed, reinstalling clean.";
		runCmd("rm -rf " + quote(mCatalogDir));

		const std::string cloneCmd = "git clone --depth 1 " + quote(catalogRepo) + " " + quote(mCatalogDir);
		rc = runCmd(cloneCmd);
		if (rc != 0)
		{
			showPopup("Catalog update failed.", 4000);
			return false;
		}
	}

	return true;
}

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

		// Badge ASCII (installed / not installed)
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

bool GuiThemeBrowser::downloadOrUpdate(const ThemeEntry& e)
{
	if (e.repo.empty())
	{
		showPopup("No repo URL for this theme.", 4000);
		return false;
	}

	Utils::FileSystem::createDirectory(mThemesDir);

	const std::string dst = mThemesDir + "/" + e.folder;
	const bool installed = Utils::FileSystem::exists(dst);

	showPopup(installed ? "Updating theme..." : "Downloading theme...", 2000);

	int rc = 0;

	if (!installed)
	{
		const std::string cmd = "git clone --depth 1 " + quote(e.repo) + " " + quote(dst);
		rc = runCmd(cmd);
	}
	else
	{
		const std::string pullCmd = "git -C " + quote(dst) + " pull --ff-only";
		rc = runCmd(pullCmd);

		// Fallback ultra estable: si falla el pull, reinstala limpio
		if (rc != 0)
		{
			LOG(LogWarning) << "Theme update failed, reinstalling: " << e.folder;

			runCmd("rm -rf " + quote(dst));

			const std::string cloneCmd = "git clone --depth 1 " + quote(e.repo) + " " + quote(dst);
			rc = runCmd(cloneCmd);
		}
	}

	if (rc == 0)
	{
		showPopup("Theme ready.", 4000);
		return true;
	}

	showPopup("Download/Update failed.", 5000);
	return false;
}

bool GuiThemeBrowser::uninstallTheme(const ThemeEntry& e)
{
	const std::string dst = mThemesDir + "/" + e.folder;

	if (!Utils::FileSystem::exists(dst))
	{
		showPopup("Theme is not installed.", 3000);
		return false;
	}

	const std::string cmd = "rm -rf " + quote(dst);
	const int rc = runCmd(cmd);

	if (rc == 0 && !Utils::FileSystem::exists(dst))
	{
		showPopup("Theme removed.", 4000);
		return true;
	}

	showPopup("Remove failed.", 5000);
	return false;
}

bool GuiThemeBrowser::input(InputConfig* config, Input input)
{
	if (input.value == 0)
		return false;

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
		const ThemeEntry& cur = mThemes[mLastSelectedIndex];
		downloadOrUpdate(cur);
		rebuildList();
		return true;
	}

	if (config->isMappedTo("x", input) || config->isMappedTo("delete", input))
	{
		const ThemeEntry& cur = mThemes[mLastSelectedIndex];
		if (isInstalled(cur))
		{
			uninstallTheme(cur);
			rebuildList();
		}
		else
		{
			showPopup("Not installed.", 2500);
		}
		return true;
	}

	if (mList.input(config, input))
		return true;

	return GuiComponent::input(config, input);
}

void GuiThemeBrowser::update(int deltaTime)
{
	GuiComponent::update(deltaTime);
	mFrame.update(deltaTime);
	mList.update(deltaTime);
	mPreview.update(deltaTime);
	mRepoLabel.update(deltaTime);
	mHeader.update(deltaTime);
	mFooter.update(deltaTime);
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
