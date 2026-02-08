#pragma once

#include <string>
#include <vector>
#include <deque>
#include <atomic>

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

    // Llamar desde el loop principal (Window::update / main loop)
    void update(int deltaTimeMs);

    void playNext();

    // Popup "Now playing"
    bool songNameChanged();
    void resetSongNameChangedFlag();
    std::string getCurrentSongDisplayName() const;

    // Ducking (cuando hay video reproduciéndose)
    void setVideoPlaying(bool playing);

private:
    BackgroundMusicManager();
    ~BackgroundMusicManager();

    bool openMixer();
    void closeMixer();
    bool reopenMixer();

    void buildPlaylist();
    void buildPlaylistFromPath(const std::string& path);
    bool isValidAudioFile(const std::string& path) const;

    void addLastPlayed(const std::string& song);
    bool wasPlayedRecently(const std::string& song) const;
    int  pickNextIndex();

    void playCurrent();
    void stopMusicInternal(bool fadeOut);

    void setNowPlaying(const std::string& fullPath);

    // Ducking helpers
    int  computeBaseMusicVolume() const;
    void applyMusicVolumeImmediate(int vol);
    void updateDucking(int deltaTimeMs);

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

    bool mPendingResume;
    int  mResumeDelayMs;
    int  mResumeTimerMs;
    int  mPendingIndex;

    bool mPendingReopenOnResume;

    // audio thread -> main thread
    std::atomic<bool> mPendingNextFromCallback;

    // Ducking state (main thread only)
    bool  mVideoPlaying;
    int   mBaseVolume;        // 0..128
    float mDuckFactor;        // 0..1 (ej 0.35)
    int   mDuckTargetVol;     // 0..128
    int   mDuckCurrentVol;    // 0..128
};
