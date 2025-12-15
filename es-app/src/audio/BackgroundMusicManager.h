#pragma once

#include <string>
#include <vector>
#include <deque>

struct _Mix_Music;
typedef struct _Mix_Music Mix_Music;

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

    void update(int deltaTimeMs);

    void playNext();

    bool songNameChanged();
    void resetSongNameChangedFlag();
    std::string getCurrentSongDisplayName() const;

private:
    BackgroundMusicManager();
    ~BackgroundMusicManager();

    bool openMixer();
    void closeMixer();

    // ✅ nuevo: reabrir audio “limpio” (estilo antiguo shutdown()+init())
    bool reopenMixer();

    void buildPlaylist();
    void buildPlaylistFromPath(const std::string& path);
    bool isValidAudioFile(const std::string& path) const;

    void addLastPlayed(const std::string& song);
    bool wasPlayedRecently(const std::string& song) const;
    int pickNextIndex();

    void playCurrent();
    void stopMusicInternal(bool fadeOut);

    void setNowPlaying(const std::string& fullPath);

    static void musicFinishedCallbackStatic();
    void musicFinishedCallback();

private:
    static BackgroundMusicManager* sInstance;

    bool mInitialized;
    bool mEnabled;
    bool mGameRunning;
    bool mMixerOpenedByUs;

    std::vector<std::string> mPlaylist;
    std::deque<std::string>  mLastPlayed;
    int mCurrentIndex;

    Mix_Music* mCurrentMusic;

    std::string mNowPlayingText;
    bool mSongNameChanged;

    // Delay al volver del juego
    bool mPendingResume;
    int  mResumeDelayMs;
    int  mResumeTimerMs;
    int  mPendingIndex;

    // ✅ pedir “reopen audio” al volver (como el antiguo)
    bool mPendingReopenOnResume;

    // Next track desde callback (main thread)
    bool mPendingNextFromCallback;
};
