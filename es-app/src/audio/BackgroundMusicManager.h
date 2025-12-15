#pragma once

#include <string>
#include <vector>
#include <deque>

struct _Mix_Music;
typedef struct _Mix_Music Mix_Music;

class BackgroundMusicManager
{
public:
    // =============================
    // Singleton
    // =============================
    static BackgroundMusicManager& getInstance();

    // =============================
    // Ciclo de vida
    // =============================
    void init();
    void shutdown();

    // =============================
    // Enable / Disable (GuiMenu)
    // =============================
    void setEnabled(bool enabled);
    bool isEnabled() const { return mEnabled; }   // ✅ NECESARIO para GuiMenu.cpp

    // =============================
    // Hooks del flujo de juegos
    // =============================
    void onGameLaunched();
    void onGameEnded();

    // =============================
    // Loop principal (delay seguro)
    // =============================
    void update(int deltaTimeMs);

    // =============================
    // Control manual
    // =============================
    void playNext();

    // =============================
    // Popup helpers
    // =============================
    bool songNameChanged();
    void resetSongNameChangedFlag();
    std::string getCurrentSongDisplayName() const;

private:
    BackgroundMusicManager();
    ~BackgroundMusicManager();

    // =============================
    // Mixer open / close
    // =============================
    bool openMixer();
    void closeMixer();

    // =============================
    // Playlist
    // =============================
    void buildPlaylist();
    void buildPlaylistFromPath(const std::string& path);
    bool isValidAudioFile(const std::string& path) const;

    // =============================
    // Shuffle inteligente
    // =============================
    void addLastPlayed(const std::string& song);
    bool wasPlayedRecently(const std::string& song) const;
    int pickNextIndex();

    // =============================
    // Playback interno
    // =============================
    void playCurrent();
    void stopMusicInternal(bool fadeOut);

    // =============================
    // Now playing
    // =============================
    void setNowPlaying(const std::string& fullPath);

    // =============================
    // Callback SDL_mixer
    // =============================
    static void musicFinishedCallbackStatic();
    void musicFinishedCallback();

private:
    // =============================
    // Singleton
    // =============================
    static BackgroundMusicManager* sInstance;

    // =============================
    // Estado
    // =============================
    bool mInitialized;
    bool mEnabled;
    bool mGameRunning;
    bool mMixerOpenedByUs;

    // =============================
    // Playlist
    // =============================
    std::vector<std::string> mPlaylist;
    std::deque<std::string>  mLastPlayed;
    int mCurrentIndex;

    // =============================
    // SDL_mixer
    // =============================
    Mix_Music* mCurrentMusic;

    // =============================
    // Popup
    // =============================
    std::string mNowPlayingText;
    bool mSongNameChanged;

    // =============================
    // ✅ FIX CRÍTICO
    // Reanudar música con delay al volver del juego
    // =============================
    bool mPendingResume;
    int  mResumeDelayMs;   // ej: 600–900 ms
    int  mResumeTimerMs;
    int  mPendingIndex;
};
