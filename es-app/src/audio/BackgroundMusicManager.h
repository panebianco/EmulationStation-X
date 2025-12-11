#pragma once

#include <string>
#include <vector>
#include <SDL_mixer.h>

class BackgroundMusicManager
{
public:
    // Singleton
    static BackgroundMusicManager& getInstance();

    // Inicializa SDL_mixer y la playlist (si está habilitado en Settings)
    void init();

    // Apaga la música y cierra SDL_mixer
    void shutdown();

    // Habilita o deshabilita la música de fondo (se guarda en Settings)
    void setEnabled(bool enabled);
    bool isEnabled() const { return mEnabled; }

    // Llamar cuando se lanza un juego (runcommand)
    void onGameLaunched();

    // Llamar cuando se vuelve del juego a ES
    void onGameEnded();

    // Pasar a la siguiente pista manualmente (para futuro botón "Next")
    void playNext();

private:
    BackgroundMusicManager();
    ~BackgroundMusicManager();

    void buildPlaylist();
    void buildPlaylistFromPath(const std::string& path);
    bool isValidAudioFile(const std::string& path) const;

    void playCurrent();
    void stopMusicInternal(bool fadeOut);

    static void musicFinishedCallbackStatic();
    void musicFinishedCallback();

    static BackgroundMusicManager* sInstance;

    bool mInitialized;
    bool mEnabled;
    bool mGameRunning;

    // NUEVO: recordamos si había música sonando antes de lanzar el juego
    bool mWasPlayingBeforeGame;

    std::vector<std::string> mPlaylist;
    int mCurrentIndex;

    Mix_Music* mCurrentMusic;
};
