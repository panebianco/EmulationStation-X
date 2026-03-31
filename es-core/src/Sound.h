#pragma once
#ifndef ES_CORE_SOUND_H
#define ES_CORE_SOUND_H

#include <map>
#include <memory>
#include <string>

#include <SDL2/SDL_mixer.h>

class ThemeData;

class Sound
{
	std::string mPath;

	// SDL_mixer sample
	Mix_Chunk* mChunk;

	// Canal donde se está reproduciendo (si aplica)
	int mChannel;

public:
	static std::shared_ptr<Sound> get(const std::string& path);
	static std::shared_ptr<Sound> getFromTheme(const std::shared_ptr<ThemeData>& theme,
	                                           const std::string& view,
	                                           const std::string& elem);

	// Limpia caché global de sonidos antes de cerrar audio/SDL
	static void cleanup();

	~Sound();

	void init();
	void deinit();

	void loadFile(const std::string& path);

	void play();
	bool isPlaying() const;
	void stop();

private:
	Sound(const std::string& path = "");
	static std::map<std::string, std::shared_ptr<Sound>> sMap;

	// Abre mixer si aún no está abierto (fallback seguro)
	static void ensureMixerOpen();

	// Comprueba si el mixer sigue realmente disponible
	static bool isMixerAvailable();
};

#endif // ES_CORE_SOUND_H
