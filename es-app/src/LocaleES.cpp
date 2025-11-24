#include "LocaleES.h"

#include "Settings.h"
#include "utils/FileSystemUtil.h"
#include "utils/StringUtil.h"
#include "Log.h"

#include <fstream>

using namespace Utils;

LocaleES& LocaleES::getInstance()
{
    static LocaleES instance;
    return instance;
}

LocaleES::LocaleES()
    : mCurrentLanguage("en")
{
}

void LocaleES::clear()
{
    mStrings.clear();
}

void LocaleES::loadFromSettings()
{
    std::string lang = Settings::getInstance()->getString("Language");
    if (lang.empty())
        lang = "en";

    std::string langDir = Utils::FileSystem::getHomePath() + "/.emulationstation/lang";
    std::string path = langDir + "/" + lang + ".ini";

    if (!FileSystem::exists(path))
    {
        LOG(LogInfo) << "LocaleES: language file not found: " << path
                     << " - using built-in English strings";
        clear();
        mCurrentLanguage = "en";
        return;
    }

    load(path);
}

static std::string stripUtf8Bom(const std::string& line)
{
    if (line.size() >= 3 &&
        (unsigned char)line[0] == 0xEF &&
        (unsigned char)line[1] == 0xBB &&
        (unsigned char)line[2] == 0xBF)
        return line.substr(3);

    return line;
}

void LocaleES::load(const std::string& path)
{
    clear();

    std::ifstream file(path.c_str());
    if (!file.is_open())
    {
        LOG(LogError) << "LocaleES: could not open language file: " << path;
        mCurrentLanguage = "en";
        return;
    }

    std::string line;
    bool firstLine = true;

    while (std::getline(file, line))
    {
        if (firstLine)
        {
            line = stripUtf8Bom(line);
            firstLine = false;
        }

        // quitar espacios
        line = String::trim(line);

        if (line.empty())
            continue;

        // comentarios
        if (line[0] == '#' || line[0] == ';')
            continue;

        // secciones [META], [MAIN], etc.
        if (line.front() == '[' && line.back() == ']')
            continue;

        std::string::size_type pos = line.find('=');
        if (pos == std::string::npos)
            continue;

        std::string key   = String::trim(line.substr(0, pos));
        std::string value = String::trim(line.substr(pos + 1));

        if (key.empty())
            continue;

        mStrings[key] = value;
    }

    file.close();

    mCurrentLanguage = Settings::getInstance()->getString("Language");
    if (mCurrentLanguage.empty())
        mCurrentLanguage = "en";

    LOG(LogInfo) << "LocaleES: loaded " << mStrings.size()
                 << " strings from " << path
                 << " (lang=" << mCurrentLanguage << ")";
}

std::string LocaleES::translate(const std::string& key) const
{
    if (key.empty())
        return key;

    auto it = mStrings.find(key);
    if (it != mStrings.cend())
        return it->second;

    // Si no se encuentra, devolvemos la clave original (inglés)
    return key;
}
