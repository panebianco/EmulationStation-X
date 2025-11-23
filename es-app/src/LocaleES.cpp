#include "LocaleES.h"

#include "Settings.h"
#include "utils/FileSystemUtil.h"
#include "Log.h"

#include <fstream>
#include <sstream>
#include <cctype>

// ------------------------------------------------------------
//  Singleton
// ------------------------------------------------------------
LocaleES* LocaleES::getInstance()
{
    static LocaleES instance;
    return &instance;
}

// ------------------------------------------------------------
//  Constructor: inicializa idioma usando Settings
// ------------------------------------------------------------
LocaleES::LocaleES()
    : mCurrentLang("en")
{
    reloadFromLanguageSetting();
}

// ------------------------------------------------------------
//  Leer Setting("Language") y cargar idioma
// ------------------------------------------------------------
void LocaleES::reloadFromLanguageSetting()
{
    std::string lang = Settings::getInstance()->getString("Language");
    if (lang.empty())
        lang = "en";

    setLanguage(lang);
}

// ------------------------------------------------------------
//  Cambiar idioma (si es distinto al actual)
// ------------------------------------------------------------
void LocaleES::setLanguage(const std::string& langCode)
{
    if (langCode == mCurrentLang)
        return;

    mCurrentLang = langCode;
    load(langCode);
}

// ------------------------------------------------------------
//  Trim utilitario
// ------------------------------------------------------------
static inline std::string trim(const std::string& s)
{
    size_t start = 0;
    while (start < s.size() && std::isspace((unsigned char)s[start]))
        start++;

    if (start == s.size())
        return "";

    size_t end = s.size() - 1;
    while (end > start && std::isspace((unsigned char)s[end]))
        end--;

    return s.substr(start, end - start + 1);
}

// ------------------------------------------------------------
//  Cargar archivo <langCode>.lang
//  desde:
//    ~/.emulationstation/lang/<lang>.lang
//    /etc/emulationstation/lang/<lang>.lang
// ------------------------------------------------------------
void LocaleES::load(const std::string& langCode)
{
    mMessages.clear();

    std::string home = Utils::FileSystem::getHomePath();
    std::string pathUser   = home + "/.emulationstation/lang/" + langCode + ".lang";
    std::string pathSystem = "/etc/emulationstation/lang/" + langCode + ".lang";

    std::string pathToUse;

    if (Utils::FileSystem::exists(pathUser))
        pathToUse = pathUser;
    else if (Utils::FileSystem::exists(pathSystem))
        pathToUse = pathSystem;

    if (pathToUse.empty())
    {
        LOG(LogWarning) << "LocaleES: no lang file found for language \"" << langCode << "\".";
        return;
    }

    std::ifstream in(pathToUse.c_str());
    if (!in)
    {
        LOG(LogWarning) << "LocaleES: failed to open lang file \"" << pathToUse << "\".";
        return;
    }

    LOG(LogInfo) << "LocaleES: loading language \"" << langCode << "\" from \"" << pathToUse << "\"";

    std::string line;
    while (std::getline(in,
