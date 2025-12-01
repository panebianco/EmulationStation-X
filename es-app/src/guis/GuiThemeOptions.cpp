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
		std::string id;          // [id] de la sección, p.ej. "layout", "colorScheme"
		std::string type;        // "select" (por ahora)
		std::string label;       // texto a mostrar en el menú
		std::string path;        // reservado para futuro (avatars, etc.)
		std::string applyTo;     // p.ej. "layout" para opciones que cambian el layout

		// Lista de valores posibles: first = valor interno, second = texto visible
		std::vector<std::pair<std::string, std::string>> values;

		// Valor por defecto (texto interno, ej. "dark")
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
			{
				// Si no hay '|', usamos el mismo texto para valor interno y visible
				out.emplace_back(item, item);
			}
			else
			{
				std::string internal = trim(item.substr(0, pos));
				std::string label    = trim(item.substr(pos + 1));
				if(!internal.empty() && !label.empty())
					out.emplace_back(internal, label);
			}
		}
	}

	// Lee solo las secciones del theme.ini que describen opciones de menú.
	// Las secciones sin "type" (como [layout_smd], [layout_snes], etc.)
	// se ignoran y quedan solo para ThemeData::loadExternalThemeVariables.
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
			// Sólo agregamos secciones que realmente definen una opción:
			// - tienen id
			// - tienen type (select, etc.)
			// - y al menos un valor
			if(!opt.id.empty() && !opt.type.empty() && !opt.values.empty())
				options.push_back(opt);
		};

		while(std::getline(file, line))
		{
			line = trim(line);
			if(line.empty() || line[0] == ';' || line[0] == '#')
				continue;

			// Nueva sección [id]
			if(line.front() == '[' && line.back() == ']')
			{
				// Guardar la sección anterior si era una opción válida
				pushIfValid(current);

				current = ThemeOption();
				current.id = trim(line.substr(1, line.size() - 2));
				continue;
			}

			// clave = valor
			auto pos = line.find('=');
			if(pos != std::string::npos)
			{
				std::string key   = trim(line.substr(0, pos));
				std::string value = trim(line.substr(pos + 1));

				if(key == "label")
					current.label = value;
				else if(key == "type")
					current.type = value;
				else if(key == "path")
					current.path = value;
				else if(key == "apply_to" || key == "applyTo")
					current.applyTo = value;
				else if(key == "values")
					parseValues(value, current.values);
				else if(key == "default")
					current.defaultValue = value;
			}
		}

		// Última sección del archivo
		pushIfValid(current);

		return options;
	}

	// Actualiza/crea una línea "key = value" en la parte superior de theme.ini
	// (antes de la primera sección [ ... ]).
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
			// Insertar nueva línea antes de la primera sección
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

	// Aplica una opción de tema.
	// - Si es applyTo="layout" (o id=="layout"), guarda en Settings::ThemeLayout.
	// - Para el resto, escribe key = value en la parte superior de theme.ini.
	void applyThemeOption(const std::string& themeDir, const ThemeOption& opt, const std::string& value)
	{
		if(opt.id.empty() || value.empty())
			return;

		// Opción especial: cambia el layout del theme
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
			// Recargar todo para que se vuelvan a aplicar themes/variables
			vc->reloadAll();
		}
	}

	// Pequeña GUI para seleccionar entre N valores de una opción de theme.ini
	class GuiThemeOptionSelect : public GuiComponent
	{
	public:
		GuiThemeOptionSelect(Window* window,
		                     const std::string& themeDir,
		                     const ThemeOption& opt,
		                     const std::string& title)
			: GuiComponent(window)
			, mMenu(window, title.c_str())
			, mThemeDir(themeDir)
			, mOption(opt)
		{
			for(const auto& v : mOption.values)
			{
				ComponentListRow row;
				row.addElement(
					std::make_shared<TextComponent>(
						mWindow,
						v.second,
						Font::get(FONT_SIZE_MEDIUM),
						0xCCCCCCFF),
					true);

				// Al elegir un valor se aplica la opción, se recarga el tema y se cierra esta GUI
				row.makeAcceptInputHandler([this, v]()
				{
					applyThemeOption(mThemeDir, mOption, v.first);
					delete this;
				});

				mMenu.addRow(row);
			}

			addChild(&mMenu);
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
} // namespace anónimo

GuiThemeOptions::GuiThemeOptions(Window* window)
	: GuiComponent(window),
	  mMenu(window, _("THEME OPTIONS").c_str())
{
	// ThemeSet actual
	std::string themeSet = Settings::getInstance()->getString("ThemeSet");
	std::string themeDir;

	if(!themeSet.empty())
		themeDir = Utils::FileSystem::getHomePath() + "/.emulationstation/themes/" + themeSet;

	auto options = loadThemeOptionsFromIni(themeDir);

	if(options.empty())
	{
		// Si no hay opciones en theme.ini
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
		// Crear una entrada por cada sección de opciones del theme.ini
		for(const auto& opt : options)
		{
			std::string entryLabel = !opt.label.empty()
				? opt.label
				: opt.id;

			ComponentListRow row;
			row.addElement(
				std::make_shared<TextComponent>(
					mWindow,
					entryLabel,
					Font::get(FONT_SIZE_MEDIUM),
					0xCCCCCCFF),
				true);

			// Handler cuando se selecciona una opción del theme.ini
			row.makeAcceptInputHandler([this, themeDir, opt, entryLabel]
			{
				if(opt.type == "select" && !opt.values.empty())
				{
					// Nuevo submenú con todas las opciones (1, 2, 3, las que haya)
					mWindow->pushGui(
						new GuiThemeOptionSelect(
							mWindow,
							themeDir,
							opt,
							entryLabel));
				}
				else
				{
					// Tipos no soportados (por ahora)
					std::string msg = entryLabel + "\n\n" + _("(Feature not yet implemented)");
					mWindow->pushGui(new GuiMsgBox(mWindow, msg, _("BACK")));
				}
			});

			mMenu.addRow(row);
		}
	}

	addChild(&mMenu);
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

	// Copiado del patrón de otros GUI:
	auto system = ViewController::get()->getState().getSystem();
	if(system)
		style.applyTheme(system->getTheme(), "system");

	return style;
}
