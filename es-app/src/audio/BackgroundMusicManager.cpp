// es-core/src/audio/BackgroundMusicManager.cpp
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
#include <vector>

// -----------------------------
//  Helpers (C++11 safe)
// -----------------------------
static inline int clampInt(int v, int lo, int hi)
{
	if (v < lo) return lo;
	if (v > hi) return hi;
	return v;
}

static inline std::string getMusicSourceSetting()
{
	std::string source = Settings::getInstance()->getString("BackgroundMusicSource");
	if (source.empty())
		source = "auto";

	source = Utils::String::toLower(source);

	if (source != "auto" && source != "global" && source != "theme")
		source = "auto";

	return source;
}

static inline bool containsPath(const std::vector<std::string>& list, const std::string& path)
{
	return std::find(list.begin(), list.end(), path) != list.end();
}

static inline void addUniqueDir(std::vector<std::string>& out, const std::string& path)
{
	if (path.empty())
		return;

	if (!containsPath(out, path))
		out.push_back(path);
}

static inline std::vector<std::string> getThemeMusicDirectories()
{
	std::vector<std::string> dirs;

	const std::string themeSet = Settings::getInstance()->getString("ThemeSet");
	if (themeSet.empty())
		return dirs;

	const std::string home = Utils::FileSystem::getHomePath();

	// Rutas típicas de temas en ES / RetroPie / ES-X
	addUniqueDir(dirs, home + "/.emulationstation/themes/" + themeSet + "/music");
	addUniqueDir(dirs, home + "/.emulationstation/themes/" + themeSet + "/_inc/music");

	addUniqueDir(dirs, "/etc/emulationstation/themes/" + themeSet + "/music");
	addUniqueDir(dirs, "/etc/emulationstation/themes/" + themeSet + "/_inc/music");

	addUniqueDir(dirs, "/usr/share/emulationstation/themes/" + themeSet + "/music");
	addUniqueDir(dirs, "/usr/share/emulationstation/themes/" + themeSet + "/_inc/music");

	return dirs;
}

static inline std::vector<std::string> getGlobalMusicDirectories()
{
	std::vector<std::string> dirs;

	const std::string home = Utils::FileSystem::getHomePath();
	addUniqueDir(dirs, home + "/RetroPie/music");
	addUniqueDir(dirs, home + "/.emulationstation/music");

	return dirs;
}

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
	, mPendingResume(false)
	, mResumeDelayMs(600)
	, mResumeTimerMs(0)
	, mPendingIndex(-1)
	, mPendingReopenOnResume(false)
	, mPendingNextFromCallback(false)
	// ducking
	, mVideoPlaying(false)
	, mBaseVolume(96)
	, mDuckFactor(0.35f)
	, mDuckTargetVol(96)
	, mDuckCurrentVol(96)
{
	mEnabled = Settings::getInstance()->getBool("BackgroundMusic");
	mBaseVolume = computeBaseMusicVolume();
	mDuckTargetVol = mBaseVolume;
	mDuckCurrentVol = mBaseVolume;
}

BackgroundMusicManager::~BackgroundMusicManager()
{
	shutdown();
}

// =============================
//  Popup helpers
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
	std::string stem = Utils::FileSystem::getStem(fullPath);
	if (stem.empty())
		stem = fullPath;

	mNowPlayingText = std::string("🎵 Now playing: ") + stem;
	mSongNameChanged = true;
}

// =============================
//  Mixer open/close/reopen
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

	// aplicar volumen actual (con ducking actual)
	applyMusicVolumeImmediate(clampInt(mDuckCurrentVol, 0, 128));

	LOG(LogInfo) << "BGM - Mixer OPEN (owned)";
	return true;
}

void BackgroundMusicManager::closeMixer()
{
	if (!mMixerOpenedByUs)
		return;

	mPendingResume = false;
	mResumeTimerMs = 0;
	mPendingIndex = -1;
	mPendingReopenOnResume = false;
	mPendingNextFromCallback.store(false);

	stopMusicInternal(false);
	Mix_HookMusicFinished(nullptr);
	Mix_HaltMusic();
	Mix_CloseAudio();

	mMixerOpenedByUs = false;
	LOG(LogInfo) << "BGM - Mixer CLOSED";
}

bool BackgroundMusicManager::reopenMixer()
{
	if (mMixerOpenedByUs)
	{
		stopMusicInternal(false);
		Mix_HookMusicFinished(nullptr);
		Mix_HaltMusic();
		Mix_CloseAudio();
		mMixerOpenedByUs = false;
		LOG(LogInfo) << "BGM - Mixer REOPEN (close)";
	}

	return openMixer();
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
		mPendingResume = false;
		mResumeTimerMs = 0;
		mPendingIndex = -1;
		mPendingReopenOnResume = false;
		mPendingNextFromCallback.store(false);

		stopMusicInternal(true);
		return;
	}

	if (!mInitialized)
	{
		init();
		return;
	}

	if (!openMixer())
		return;

	buildPlaylist();

	if (!mPlaylist.empty())
		playCurrent();
}

// =============================
//  ✅ Volumen en tiempo real
// =============================
void BackgroundMusicManager::setVolume(int percent)
{
	percent = clampInt(percent, 0, 100);
	Settings::getInstance()->setInt("BackgroundMusicVolume", percent);

	mBaseVolume = clampInt((percent * 128) / 100, 0, 128);

	const int duck = clampInt((int)std::lround(mBaseVolume * mDuckFactor), 0, 128);
	mDuckTargetVol = mVideoPlaying ? duck : mBaseVolume;

	// aplicar de inmediato
	mDuckCurrentVol = mDuckTargetVol;
	applyMusicVolumeImmediate(mDuckCurrentVol);

	LOG(LogInfo) << "BGM - setVolume(" << percent << "%) -> mixerVol=" << mDuckCurrentVol;
}

// =============================
//  Ducking
// =============================
int BackgroundMusicManager::computeBaseMusicVolume() const
{
	// Si todavía no existe en settings, usar 75 por defecto.
	const std::string raw = Settings::getInstance()->getString("BackgroundMusicVolume");
	const int pct = raw.empty()
		? 75
		: clampInt(Settings::getInstance()->getInt("BackgroundMusicVolume"), 0, 100);

	return clampInt((pct * 128) / 100, 0, 128);
}

void BackgroundMusicManager::applyMusicVolumeImmediate(int vol)
{
	if (!mMixerOpenedByUs)
		return;

	Mix_VolumeMusic(clampInt(vol, 0, 128));
}

void BackgroundMusicManager::setVideoPlaying(bool playing)
{
	mVideoPlaying = playing;

	mBaseVolume = computeBaseMusicVolume();
	const int base = clampInt(mBaseVolume, 0, 128);

	// objetivo: cuando hay video, bajar a base * duckFactor
	const int duck = clampInt((int)std::lround(base * mDuckFactor), 0, 128);

	mDuckTargetVol = mVideoPlaying ? duck : base;
}

void BackgroundMusicManager::updateDucking(int deltaTimeMs)
{
	if (!mMixerOpenedByUs)
		return;

	const int target = clampInt(mDuckTargetVol, 0, 128);
	int cur = clampInt(mDuckCurrentVol, 0, 128);

	if (cur == target)
		return;

	// ~250ms para llegar al target
	const float stepsPerMs = 512.0f / 1000.0f;
	int step = (int)std::lround(stepsPerMs * (float)deltaTimeMs);
	if (step < 1) step = 1;

	if (cur < target) cur = std::min(cur + step, target);
	else              cur = std::max(cur - step, target);

	mDuckCurrentVol = cur;
	Mix_VolumeMusic(cur);
}

// =============================
//  Game hooks
// =============================
void BackgroundMusicManager::onGameLaunched()
{
	if (!mInitialized)
		return;

	mGameRunning = true;

	mPendingResume = false;
	mResumeTimerMs = 0;
	mPendingIndex = -1;
	mPendingReopenOnResume = false;
	mPendingNextFromCallback.store(false);

	LOG(LogInfo) << "BGM - onGameLaunched() -> stop music";
	stopMusicInternal(true);
}

void BackgroundMusicManager::onGameEnded()
{
	mGameRunning = false;

	if (!mInitialized)
		return;

	// ✅ FIX: aunque BackgroundMusic esté OFF, revalidamos el mixer para que
	// los sonidos de UI no queden con "Invalid audio device ID" al volver del juego.
	if (mMixerOpenedByUs)
		reopenMixer();
	else
		openMixer();

	// Si la música está apagada, no hacemos resume.
	if (!mEnabled)
		return;

	buildPlaylist();

	if (!mPlaylist.empty())
		mPendingIndex = pickNextIndex();
	else
		mPendingIndex = -1;

	// ya reabrimos arriba; no hace falta reabrir en resume
	mPendingReopenOnResume = false;

	mPendingResume = true;
	mResumeTimerMs = mResumeDelayMs;

	LOG(LogInfo) << "BGM - onGameEnded(): resume in " << mResumeDelayMs << "ms";
}

// =============================
//  Update (main thread)
// =============================
void BackgroundMusicManager::update(int deltaTimeMs)
{
	if (!mInitialized)
		return;

	// Refrescar volumen base desde settings por si cambió en menú
	mBaseVolume = computeBaseMusicVolume();
	{
		const int base = clampInt(mBaseVolume, 0, 128);
		const int duck = clampInt((int)std::lround(base * mDuckFactor), 0, 128);
		mDuckTargetVol = mVideoPlaying ? duck : base;
	}

	// Ducking siempre (si hay mixer)
	updateDucking(deltaTimeMs);

	// Si la canción terminó, el callback marca el flag.
	if (mPendingNextFromCallback.exchange(false))
	{
		LOG(LogInfo) << "BGM - update: pendingNext -> playNext()";
		if (!mGameRunning && mEnabled)
			playNext();
	}

	// Resume post-game con delay
	if (mPendingResume)
	{
		if (mGameRunning || !mEnabled)
		{
			mPendingResume = false;
			mResumeTimerMs = 0;
			mPendingIndex = -1;
			mPendingReopenOnResume = false;
		}
		else
		{
			mResumeTimerMs -= deltaTimeMs;
			if (mResumeTimerMs <= 0)
			{
				mPendingResume = false;
				mResumeTimerMs = 0;

				buildPlaylist();

				if (!mPlaylist.empty())
				{
					if (mPendingReopenOnResume)
					{
						mPendingReopenOnResume = false;
						if (!reopenMixer())
							return;
					}
					else
					{
						if (!openMixer())
							return;
					}

					if (mPendingIndex >= 0 && mPendingIndex < (int)mPlaylist.size())
						mCurrentIndex = mPendingIndex;
					else
						mCurrentIndex = pickNextIndex();

					mPendingIndex = -1;

					LOG(LogInfo) << "BGM - resume -> playCurrent() index=" << mCurrentIndex;
					playCurrent();
				}
			}
		}
	}

	// Watchdog anti-silencio:
	if (mEnabled && !mGameRunning && mMixerOpenedByUs && !mPendingResume)
	{
		if (!Mix_PlayingMusic() && !Mix_PausedMusic() && !mPlaylist.empty())
		{
			LOG(LogWarning) << "BGM - watchdog: music stopped unexpectedly -> playNext()";
			playNext();
		}
	}
}

// =============================
//  Playlist
// =============================
void BackgroundMusicManager::buildPlaylist()
{
	mPlaylist.clear();
	mLastPlayed.clear();
	mCurrentIndex = -1;

	const std::string source = getMusicSourceSetting();
	bool themeHasTracks = false;

	// THEME
	if (source == "theme" || source == "auto")
	{
		const std::vector<std::string> themeDirs = getThemeMusicDirectories();

		for (std::vector<std::string>::const_iterator it = themeDirs.begin(); it != themeDirs.end(); ++it)
			buildPlaylistFromPath(*it);

		themeHasTracks = !mPlaylist.empty();
	}

	// GLOBAL
	if (source == "global" || (source == "auto" && !themeHasTracks))
	{
		const std::vector<std::string> globalDirs = getGlobalMusicDirectories();

		for (std::vector<std::string>::const_iterator it = globalDirs.begin(); it != globalDirs.end(); ++it)
			buildPlaylistFromPath(*it);
	}

	if (!mPlaylist.empty())
	{
		static std::mt19937 rng{ std::random_device{}() };
		std::uniform_int_distribution<int> dist(0, (int)mPlaylist.size() - 1);
		mCurrentIndex = dist(rng);

		LOG(LogInfo) << "BGM - Playlist ready (" << mPlaylist.size()
					 << " tracks) | source=" << source;
	}
	else
	{
		LOG(LogInfo) << "BGM - Playlist empty | source=" << source;
	}
}

void BackgroundMusicManager::buildPlaylistFromPath(const std::string& path)
{
	if (!Utils::FileSystem::exists(path) || !Utils::FileSystem::isDirectory(path))
		return;

	for (const auto& e : Utils::FileSystem::getDirContent(path))
	{
		if (!Utils::FileSystem::isDirectory(e) && isValidAudioFile(e))
		{
			if (std::find(mPlaylist.begin(), mPlaylist.end(), e) == mPlaylist.end())
				mPlaylist.push_back(e);
		}
	}
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
	if (mPlaylist.empty())
		return -1;

	if (mPlaylist.size() <= 1)
		return 0;

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

	int attempts = 0;
	int maxAttempts = std::min((int)mPlaylist.size(), 10);

	while (attempts < maxAttempts && !mPlaylist.empty())
	{
		if (mCurrentIndex < 0 || mCurrentIndex >= (int)mPlaylist.size())
			mCurrentIndex = pickNextIndex();

		if (mCurrentIndex < 0 || mCurrentIndex >= (int)mPlaylist.size())
			return;

		const std::string song = mPlaylist[mCurrentIndex];

		if (mCurrentMusic)
		{
			Mix_HaltMusic();
			Mix_FreeMusic(mCurrentMusic);
			mCurrentMusic = nullptr;
		}

		mCurrentMusic = Mix_LoadMUS(song.c_str());
		if (!mCurrentMusic)
		{
			LOG(LogError) << "BGM - Load failed, skipping: " << song
						  << " (" << Mix_GetError() << ")";

			mPlaylist.erase(mPlaylist.begin() + mCurrentIndex);
			mCurrentIndex = -1;
			attempts++;
			continue;
		}

		LOG(LogInfo) << "BGM - Playing: " << song;
		setNowPlaying(song);

		// Aplicar volumen actual (ducking) antes de play
		applyMusicVolumeImmediate(clampInt(mDuckCurrentVol, 0, 128));

		if (Mix_PlayMusic(mCurrentMusic, 0) != 0)
		{
			LOG(LogError) << "BGM - Mix_PlayMusic failed: " << Mix_GetError()
						  << " | skipping: " << song;

			Mix_FreeMusic(mCurrentMusic);
			mCurrentMusic = nullptr;

			mPlaylist.erase(mPlaylist.begin() + mCurrentIndex);
			mCurrentIndex = -1;
			attempts++;
			continue;
		}

		addLastPlayed(song);
		return;
	}

	LOG(LogWarning) << "BGM - No valid tracks left, stopping music";
}

// =============================
//  Controls
// =============================
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

	LOG(LogInfo) << "BGM - stopMusicInternal(fadeOut=" << (fadeOut ? "1" : "0") << ")";

	// ✅ FIX seguro: no liberar Mix_Music mientras SDL_mixer podría seguir usando el puntero.
	// Para evitar heisenbugs/crash, hacemos halt + free (fadeOut se ignora aquí).
	(void)fadeOut;
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

	// OJO: esto se ejecuta desde el thread interno de mixer.
	// Solo seteamos un flag atómico.
	LOG(LogInfo) << "BGM - callback: music finished (pendingNext=1)";
	mPendingNextFromCallback.store(true);
}