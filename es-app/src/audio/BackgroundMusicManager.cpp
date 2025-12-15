#include "audio/BackgroundMusicManager.h"

#include "Settings.h"
#include "Log.h"
#include "utils/FileSystemUtil.h"
#include "utils/StringUtil.h"

#include <SDL.h>
#include <SDL_mixer.h>

#include <algorithm>
#include <random>
#include <cmath>

// =============================
//  Singleton
// =============================

BackgroundMusicManager* BackgroundMusicManager::sInstance = nullptr;

BackgroundMusicManager& BackgroundMusicManager::getInstance()
{
    if (!sInstance)
        sInstance = new BackgroundMusicManager();
    return *sInstance;
}

// =============================
//  Ctor / Dtor
// =============================

BackgroundMusicManager::BackgroundMusicManager()
    : mInitialized(false)
    , mEnabled(true)
    , mGameRunning(false)
    , mMixerOpenedByUs(false)
    , mCurrentIndex(-1)
    , mCurrentMusic(nullptr)
    , mNowPlayingText("")
    , mSongNameChanged(false)
{
    mEnabled = Settings::getInstance()->getBool("BackgroundMusic");
}

BackgroundMusicManager::~BackgroundMusicManager()
{
    shutdown();
}

// =============================
//  Public popup helpers
// =============================

bool BackgroundMusicManager::songNameChanged()
{
    return mSongNameChanged;
}

void BackgroundMusicManager::resetSongNameChangedFlag()
{
    mSongNameChanged = false;
}

std::string BackgroundMusicManager::getCurrentSongDisplayName() const
{
    return mNowPlayingText;
}

void BackgroundMusicManager::setNowPlaying(const std::string& fullPath)
{
    // Mostramos nombre amigable: stem del archivo (sin extensión)
    std::string stem = Utils::FileSystem::getStem(fullPath);
    if (stem.empty())
        stem = fullPath;

    // Texto final tipo Batocera
    mNowPlayingText = std::string("🎵 Now playing: ") + stem;
    mSongNameChanged = true;
}

// =============================
//  Mixer open/close
// =============================

bool BackgroundMusicManager::openMixer()
{
    if (mMixerOpenedByUs)
        return true;

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
    {
        LOG(LogError) << "BGM - Mix_OpenAudio failed: " << Mix_GetError();
        return false;
    }

    Mix_AllocateChannels(16);
    Mix_HaltMusic();
    Mix_HookMusicFinished(&BackgroundMusicManager::musicFinishedCallbackStatic);

    mMixerOpenedByUs = true;
    LOG(LogInfo) << "BGM - Mixer OPEN";
    return true;
}

void BackgroundMusicManager::closeMixer()
{
    if (!mMixerOpenedByUs)
        return;

    stopMusicInternal(false);
    Mix_HookMusicFinished(nullptr);
    Mix_HaltMusic();
    Mix_CloseAudio();

    mMixerOpenedByUs = false;
    LOG(LogInfo) << "BGM - Mixer CLOSED";
}

// =============================
//  Init / Shutdown
// =============================

void BackgroundMusicManager::init()
{
    if (mInitialized)
        return;

    if (!openMixer())
        return;

    buildPlaylist();
    mInitialized = true;

    LOG(LogInfo) << "BGM - Initialized | playlist=" << mPlaylist.size()
                 << " | enabled=" << (mEnabled ? "1" : "0");

    if (mEnabled && !mPlaylist.empty())
        playCurrent();
}

void BackgroundMusicManager::shutdown()
{
    closeMixer();

    mInitialized = false;
    mPlaylist.clear();
    mLastPlayed.clear();
    mCurrentIndex = -1;

    LOG(LogInfo) << "BGM - Shutdown";
}

// =============================
//  Enable / Disable
// =============================

void BackgroundMusicManager::setEnabled(bool enabled)
{
    if (enabled == mEnabled)
        return;

    mEnabled = enabled;
    Settings::getInstance()->setBool("BackgroundMusic", enabled);

    LOG(LogInfo) << "BGM - setEnabled(" << (enabled ? "true" : "false") << ")";

    if (!mEnabled)
    {
        stopMusicInternal(true);
        closeMixer();
        return;
    }

    if (!mInitialized)
    {
        init();
        return;
    }

    if (!openMixer())
        return;

    if (mPlaylist.empty())
        buildPlaylist();

    if (!mPlaylist.empty())
        playCurrent();
}

// =============================
//  Game hooks
// =============================

void BackgroundMusicManager::onGameLaunched()
{
    if (!mInitialized)
        return;

    mGameRunning = true;
    closeMixer();
}

void BackgroundMusicManager::onGameEnded()
{
    mGameRunning = false;

    if (!mEnabled || !mInitialized)
        return;

    if (!openMixer())
        return;

    // al volver: shuffle a otra pista
    if (!mPlaylist.empty())
    {
        mCurrentIndex = pickNextIndex();
        LOG(LogInfo) << "BGM - onGameEnded(): next (shuffle) index=" << mCurrentIndex;
    }

    playCurrent();
}

// =============================
//  Playlist
// =============================

void BackgroundMusicManager::buildPlaylist()
{
    mPlaylist.clear();
    mLastPlayed.clear();
    mCurrentIndex = -1;

    const std::string home = Utils::FileSystem::getHomePath();
    buildPlaylistFromPath(home + "/RetroPie/music");
    buildPlaylistFromPath(home + "/.emulationstation/music");

    if (!mPlaylist.empty())
    {
        static std::mt19937 rng{ std::random_device{}() };
        std::uniform_int_distribution<int> dist(0, (int)mPlaylist.size() - 1);
        mCurrentIndex = dist(rng);

        LOG(LogInfo) << "BGM - Playlist ready (" << mPlaylist.size() << " tracks)";
    }
}

void BackgroundMusicManager::buildPlaylistFromPath(const std::string& path)
{
    if (!Utils::FileSystem::exists(path) || !Utils::FileSystem::isDirectory(path))
        return;

    for (const auto& e : Utils::FileSystem::getDirContent(path))
        if (!Utils::FileSystem::isDirectory(e) && isValidAudioFile(e))
            mPlaylist.push_back(e);
}

bool BackgroundMusicManager::isValidAudioFile(const std::string& path) const
{
    const std::string ext = Utils::String::toLower(Utils::FileSystem::getExtension(path));
    return (ext == ".mp3" || ext == ".ogg" || ext == ".flac" || ext == ".wav" || ext == ".mod");
}

// =============================
//  Shuffle inteligente
// =============================

void BackgroundMusicManager::addLastPlayed(const std::string& song)
{
    int maxHistory = std::max(1, (int)std::floor(mPlaylist.size() * 0.4f));

    mLastPlayed.push_front(song);
    while ((int)mLastPlayed.size() > maxHistory)
        mLastPlayed.pop_back();
}

bool BackgroundMusicManager::wasPlayedRecently(const std::string& song) const
{
    return std::find(mLastPlayed.begin(), mLastPlayed.end(), song) != mLastPlayed.end();
}

int BackgroundMusicManager::pickNextIndex()
{
    if (mPlaylist.size() <= 1)
        return mCurrentIndex;

    static std::mt19937 rng{ std::random_device{}() };
    std::uniform_int_distribution<int> dist(0, (int)mPlaylist.size() - 1);

    int tries = 0;
    int idx = mCurrentIndex;

    do {
        idx = dist(rng);
        tries++;
    } while (tries < 20 && wasPlayedRecently(mPlaylist[idx]));

    return idx;
}

// =============================
//  Playback
// =============================

void BackgroundMusicManager::playCurrent()
{
    if (!mInitialized || !mEnabled || !mMixerOpenedByUs || mPlaylist.empty())
        return;

    if (Mix_PlayingMusic() && !Mix_PausedMusic())
        return;

    if (mCurrentIndex < 0 || mCurrentIndex >= (int)mPlaylist.size())
        mCurrentIndex = pickNextIndex();

    if (mCurrentMusic)
    {
        Mix_HaltMusic();
        Mix_FreeMusic(mCurrentMusic);
        mCurrentMusic = nullptr;
    }

    const std::string& song = mPlaylist[mCurrentIndex];
    mCurrentMusic = Mix_LoadMUS(song.c_str());

    if (!mCurrentMusic)
    {
        LOG(LogError) << "BGM - Load failed: " << song << " (" << Mix_GetError() << ")";
        mCurrentIndex = pickNextIndex();
        playCurrent();
        return;
    }

    LOG(LogInfo) << "BGM - Playing: " << song;

    // 👇 acá “anunciamos” la canción para el popup
    setNowPlaying(song);

    Mix_PlayMusic(mCurrentMusic, 0);
    addLastPlayed(song);
}

void BackgroundMusicManager::playNext()
{
    if (!mInitialized || mPlaylist.empty())
        return;

    mCurrentIndex = pickNextIndex();
    playCurrent();
}

void BackgroundMusicManager::stopMusicInternal(bool fadeOut)
{
    if (!mMixerOpenedByUs)
        return;

    if (fadeOut)
        Mix_FadeOutMusic(500);
    else
        Mix_HaltMusic();

    if (mCurrentMusic)
    {
        Mix_FreeMusic(mCurrentMusic);
        mCurrentMusic = nullptr;
    }
}

// =============================
//  Callback
// =============================

void BackgroundMusicManager::musicFinishedCallbackStatic()
{
    if (sInstance)
        sInstance->musicFinishedCallback();
}

void BackgroundMusicManager::musicFinishedCallback()
{
    if (mGameRunning || !mEnabled)
        return;

    playNext();
}
