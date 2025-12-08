// es-app/src/guis/GuiThemeOptions.cpp

#include "guis/GuiThemeOptions.h"
#include "guis/GuiMsgBox.h"
#include "LocaleES.h"
#include "Settings.h"
#include "utils/FileSystemUtil.h"
#include "resources/Font.h"
#include "views/ViewController.h"
#include "SystemData.h"
#include "Window.h"
#include "renderers/Renderer.h"

#include <fstream>
#include <sstream>

namespace
{
	inline std::string _(const std::string& key)
	{
		return LocaleES::getInstance().translate(key);
	}

	struct ThemeOption
	{
		std::string id;
		std::string type;
		std::string label;
		std::string path;
		std::string applyTo;
		std::vector<std::pair<std::string, std::string>> values;
		std::string defaultValue;
	};

	std::string trim(const std::string& s)
	{
		size_t start = s.find_first_not_of(" \t\r\n");
		if(start == std::string::npos)
			return "";
		size_t end = s.find_last_not_of(" \t\r\n");
		return s.substr(start, end - start + 1);
	}

	void parseValues(const std::string& str, std::vector<std::pair<std::string, std::string>>& out)
	{
		out.clear();
		std::stringstream ss(str);
		std::string item;

		while(std::getline(ss, item, ','))
		{
			item = trim(item);
			if(item.empty())
				continue;

			auto pos = item.find('|');
			if(pos == std::string::npos)
				out.emplace_back(item, item);
			else
			{
				std::string internal = trim(item.substr(0, pos));
				std::string label    = trim(item.substr(pos + 1));
				if(!internal.empty() && !label.empty())
					out.emplace_back(internal, label);
			}
		}
	}

	std::vector<ThemeOption> loadThemeOptionsFromIni(const std::string& themeDir)
	{
		std::vector<ThemeOption> options;
		std::string iniPath = themeDir + "/theme.ini";

		if(!Utils::FileSystem::isRegularFile(iniPath))
			return options;

		std::ifstream file(iniPath.c_str());
		if(!file)
			return options;

		ThemeOption current;
		std::string line;

		auto pushIfValid = [&options](const ThemeOption& opt)
		{
			if(!opt.id.empty() && !opt.type.empty() && !opt.values.empty())
				options.push_back(opt);
		};

		while(std::getline(file, line))
		{
			line = trim(line);
			if(line.empty() || line[0] == ';' || line[0] == '#')
				continue;

			if(line.front() == '[' && line.back() == ']')
			{
				pushIfValid(current);
				current = ThemeOption();
				current.id = trim(line.substr(1, line.size() - 2));
				continue;
			}

			auto pos = line.find('=');
			if(pos != std::string::npos)
			{
				std::string key   = trim(line.substr(0, pos));
				std::string value = trim(line.substr(pos + 1));

				if(key == "label") current.label = value;
				else if(key == "type") current.type = value;
				else if(key == "path") current.path = value;
				else if(key == "apply_to" || key == "applyTo") current.applyTo = value;
				else if(key == "values") parseValues(value, current.values);
				else if(key == "default") current.defaultValue = value;
			}
		}

		pushIfValid(current);
		return options;
	}

	void updateThemeIniValue(const std::string& iniPath, const std::string& key, const std::string& value)
	{
		std::ifstream in(iniPath.c_str());
		if(!in)
			return;

		std::vector<std::string> lines;
		std::string line;
		bool updated = false;
		bool inTop   = true;

		while(std::getline(in, line))
		{
			std::string t = trim(line);

			if(inTop && !t.empty() && t.front() == '[')
				inTop = false;

			if(inTop)
			{
				auto posEq = t.find('=');
				if(posEq != std::string::npos)
				{
					std::string thisKey = trim(t.substr(0, posEq));
					if(thisKey == key)
					{
						line    = key + " = " + value;
						updated = true;
					}
				}
			}
			lines.push_back(line);
		}
		in.close();

		if(!updated)
		{
			size_t insertPos = lines.size();
			for(size_t i = 0; i < lines.size(); ++i)
			{
				std::string t = trim(lines[i]);
				if(!t.empty() && t.front() == '[')
				{
					insertPos = i;
					break;
				}
			}
			lines.insert(lines.begin() + insertPos, key + " = " + value);
		}

		std::ofstream out(iniPath.c_str(), std::ios::trunc);
		if(!out)
			return;
		for(const auto& l : lines)
			out << l << "\n";
	}

	void applyThemeOption(const std::string& themeDir, const ThemeOption& opt, const std::string& value)
	{
		if(opt.id.empty() || value.empty())
			return;

		if(opt.applyTo == "layout" || opt.id == "layout")
		{
			Settings::getInstance()->setString("ThemeLayout", value);
			Settings::getInstance()->saveFile();
		}
		else
		{
			const std::string iniPath = themeDir + "/theme.ini";
			updateThemeIniValue(iniPath, opt.id, value);
		}

		auto vc = ViewController::get();
		if(vc != nullptr)
		{
			vc->reloadAll();
		}
	}

	class GuiThemeOptionSelect : public GuiComponent
	{
	public:
		GuiThemeOptionSelect(Window* window,
		                     const std::string& themeDir,
		                     const ThemeOption& opt,
		                     const std::string& title)
			: GuiComponent(window), mMenu(window, title.c_str()),
			  mThemeDir(themeDir), mOption(opt)
		{
			for(const auto& v : mOption.values)
			{
				ComponentListRow row;

				auto text = std::make_shared<TextComponent>(
					mWindow,
					v.second,
					Font::get(FONT_SIZE_MEDIUM),
					0x777777FF   // texto gris (como el resto de menús)
				);

				text->setColor(0x777777FF);

				row.addElement(text, true);

				row.makeAcceptInputHandler([this, v]()
				{
					applyThemeOption(mThemeDir, mOption, v.first);
					delete this;
				});

				mMenu.addRow(row);
			}

			addChild(&mMenu);

			setSize((float)Renderer::getScreenWidth(), (float)Renderer::getScreenHeight());
			mMenu.setPosition(
				(mSize.x() - mMenu.getSize().x()) / 2.0f,
				Renderer::getScreenHeight() * 0.15f);
		}

		bool input(InputConfig* config, Input input) override
		{
			if(GuiComponent::input(config, input))
				return true;

			if(config->isMappedTo("b", input) && input.value != 0)
			{
				delete this;
				return true;
			}
			return false;
		}

		std::vector<HelpPrompt> getHelpPrompts() override
		{
			std::vector<HelpPrompt> prompts;
			prompts.push_back(HelpPrompt("a", _("SELECT")));
			prompts.push_back(HelpPrompt("b", _("BACK")));
			return prompts;
		}

		HelpStyle getHelpStyle() override
		{
			HelpStyle style;
			auto system = ViewController::get()->getState().getSystem();
			if(system)
				style.applyTheme(system->getTheme(), "system");
			return style;
		}

	private:
		MenuComponent mMenu;
		std::string   mThemeDir;
		ThemeOption   mOption;
	};
}

GuiThemeOptions::GuiThemeOptions(Window* window)
	: GuiComponent(window),
	  mMenu(window, _("THEME OPTIONS").c_str())
{
	std::string themeSet = Settings::getInstance()->getString("ThemeSet");
	std::string themeDir;

	if(!themeSet.empty())
		themeDir = Utils::FileSystem::getHomePath() + "/.emulationstation/themes/" + themeSet;

	auto options = loadThemeOptionsFromIni(themeDir);

	if(options.empty())
	{
		ComponentListRow row;
		row.addElement(
			std::make_shared<TextComponent>(
				mWindow,
				_("NO THEME OPTIONS AVAILABLE"),
				Font::get(FONT_SIZE_MEDIUM),
				0x777777FF),
			true);
		mMenu.addRow(row);
	}
	else
	{
		for(const auto& opt : options)
		{
			std::string entryLabel = !opt.label.empty() ? opt.label : opt.id;

			ComponentListRow row;

			auto text = std::make_shared<TextComponent>(
				mWindow,
				entryLabel,
				Font::get(FONT_SIZE_MEDIUM),
				0x777777FF   // gris
			);
			text->setColor(0x777777FF);

			row.addElement(text, true);

			row.makeAcceptInputHandler([this, themeDir, opt, entryLabel]
			{
				if(opt.type == "select" && !opt.values.empty())
				{
					mWindow->pushGui(
						new GuiThemeOptionSelect(
							mWindow, themeDir, opt, entryLabel));
				}
				else
				{
					std::string msg = entryLabel + "\n\n" + _("(Feature not yet implemented)");
					mWindow->pushGui(new GuiMsgBox(mWindow, msg, _("BACK")));
				}
			});

			mMenu.addRow(row);
		}
	}

	addChild(&mMenu);

	setSize((float)Renderer::getScreenWidth(), (float)Renderer::getScreenHeight());
	mMenu.setPosition(
		(mSize.x() - mMenu.getSize().x()) / 2.0f,
		Renderer::getScreenHeight() * 0.15f);
}

bool GuiThemeOptions::input(InputConfig* config, Input input)
{
	if(GuiComponent::input(config, input))
		return true;

	if(config->isMappedTo("b", input) && input.value != 0)
	{
		delete this;
		return true;
	}
	return false;
}

std::vector<HelpPrompt> GuiThemeOptions::getHelpPrompts()
{
	std::vector<HelpPrompt> prompts;
	prompts.push_back(HelpPrompt("b", _("BACK")));
	return prompts;
}

HelpStyle GuiThemeOptions::getHelpStyle()
{
	HelpStyle style;
	auto system = ViewController::get()->getState().getSystem();
	if(system)
		style.applyTheme(system->getTheme(), "system");
	return style;
}
