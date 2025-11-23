#pragma once
#ifndef ES_APP_LOCALE_ES_H
#define ES_APP_LOCALE_ES_H

#include <map>
#include <string>

// Pequeño sistema de localización para EmulationStation-X.
// Lee archivos .lang del estilo:
//
//   SCRAPER = Scraper
//   UI_SETTINGS = Ajustes de interfaz
//
// Archivos buscados en:
//   ~/.emulationstation/lang/<codigo>.lang
//   /etc/emulationstation/lang/<codigo>.lang
//
// El idioma actual se toma de Settings("Language").

class LocaleES
{
public:
	// Singleton
	static LocaleES* getInstance();

	// Vuelve a cargar el idioma según Settings("Language").
	// Útil después de cambiar el idioma desde el menú.
	void reloadFromLanguageSetting();

	// Fijar manualmente el código de idioma (ej: "en", "es", "pt_BR").
	// Internamente llama a load() si cambia.
	void setLanguage(const std::string& langCode);

	// Devuelve la traducción para msgid.
	// Si no existe, devuelve msgid tal cual.
	std::string translate(const std::string& msgid) const;

	// Consultar el idioma actual.
	const std::string& getCurrentLanguage() const { return mCurrentLang; }

private:
	// Constructor privado: usar getInstance().
	LocaleES();

	// Carga el archivo <langCode>.lang desde las rutas conocidas.
	void load(const std::string& langCode);

	std::string mCurrentLang;
	std::map<std::string, std::string> mMessages;
};

// Helper para usar en el código:
//
//   ES_TR("SCRAPER")
//   ES_TR("UI SETTINGS")
//
// En los .lang:
//
//   SCRAPER=Scrapear juegos
//   UI SETTINGS=Ajustes de interfaz
//
inline std::string ES_TR(const std::string& key)
{
	return LocaleES::getInstance()->translate(key);
}

#endif // ES_APP_LOCALE_ES_H
