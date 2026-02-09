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

    // Mixer control (solo música)
    bool openMixer();
    void closeMixer();
    bool reopenMixer();

    // Playlist
    void buildPlaylist();
    void buildPlaylistFromPath(const std::string& path);
    bool isValidAudioFile(const std::string& path) const;

    // Shuffle inteligente
    void addLastPlayed(const std::string& song);
    bool wasPlayedRecently(const std::string& song) const;
    int  pickNextIndex();

    // Playback interno
    void playCurrent();
    void stopMusicInternal(bool fadeOut);

    void setNowPlaying(const std::string& fullPath);

    // Ducking helpers
    int  computeBaseMusicVolume() const;
    void applyMusicVolumeImmediate(int vol);
    void updateDucking(int deltaTimeMs);

    // Callback SDL_mixer
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

    // Popup
    std::string mNowPlayingText;
    bool mSongNameChanged;

    // Resume post-game
    bool mPendingResume;
    int  mResumeDelayMs;
    int  mResumeTimerMs;
    int  mPendingIndex;
    bool mPendingReopenOnResume;

    // audio thread → main thread
    std::atomic<bool> mPendingNextFromCallback;

    // Ducking (main thread only)
    bool  mVideoPlaying;
    int   mBaseVolume;        // 0..128
    float mDuckFactor;        // 0..1 (ej: 0.35)
    int   mDuckTargetVol;     // 0..128
    int   mDuckCurrentVol;    // 0..128
};
