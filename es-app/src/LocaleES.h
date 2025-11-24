#pragma once
#ifndef ES_CORE_LOCALE_ES_H
#define ES_CORE_LOCALE_ES_H

#include <map>
#include <string>

class LocaleES
{
public:
    // Singleton
    static LocaleES& getInstance();

    // Carga el idioma a partir de Settings::Language ("en", "es", etc.)
    void loadFromSettings();

    // Carga directamente un archivo .ini
    void load(const std::string& path);

    // Traduce una clave. Si no existe, devuelve la clave original (inglés).
    std::string translate(const std::string& key) const;

    const std::string& getCurrentLanguage() const { return mCurrentLanguage; }

private:
    LocaleES();
    LocaleES(const LocaleES&) = delete;
    LocaleES& operator=(const LocaleES&) = delete;

    void clear();

    std::map<std::string, std::string> mStrings;
    std::string mCurrentLanguage;
};

#endif // ES_CORE_LOCALE_ES_H
