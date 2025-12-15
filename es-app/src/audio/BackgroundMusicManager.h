#pragma once

#include <string>
#include <vector>
#include <deque>
#include <SDL_mixer.h>

class BackgroundMusicManager
{
public:
    static BackgroundMusicManager& getInstance();

    void init();
    void shutdown();

    void setEnabled(bool enabled);
    bool isEnabled() const { return mEnabled; }

    void onGameLaunched();
    void onGameEnded();

    void playNext();

    // ===== Popup support (UI polling) =====
    bool songNameChanged();
    void resetSongNameChangedFlag();
    std::string getCurrentSongDisplayName() const;

private:
    BackgroundMusicManager();
    ~BackgroundMusicManager();

    void buildPlaylist();
    void buildPlaylistFromPath(const std::string& path);
    bool isValidAudioFile(const std::string& path) const;

    bool openMixer();
    void closeMixer();

    void playCurrent();
    void stopMusicInternal(bool fadeOut);

    static void musicFinishedCallbackStatic();
    void musicFinishedCallback();

    // Shuffle inteligente
    void addLastPlayed(const std::string& song);
    bool wasPlayedRecently(const std::string& song) const;
    int pickNextIndex();

    // “Now Playing”
    void setNowPlaying(const std::string& fullPath);

    static BackgroundMusicManager* sInstance;

    bool mInitialized;
    bool mEnabled;
    bool mGameRunning;
    bool mMixerOpenedByUs;

    std::vector<std::string> mPlaylist;
    std::deque<std::string> mLastPlayed;
    int mCurrentIndex;

    Mix_Music* mCurrentMusic;

    // popup state
    std::string mNowPlayingText;   // ya listo para mostrar
    bool mSongNameChanged;
};
