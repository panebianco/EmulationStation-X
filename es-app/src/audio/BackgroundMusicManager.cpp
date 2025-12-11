#include "audio/BackgroundMusicManager.h"

#include "Settings.h"
#include "Log.h"
#include "utils/FileSystemUtil.h"
#include "utils/StringUtil.h"

#include <SDL.h>

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
//  Constructor / Destructor
// =============================

BackgroundMusicManager::BackgroundMusicManager()
    : mInitialized(false)
    , mEnabled(true)      // por defecto ON, luego se lee de Settings
    , mGameRunning(false)
    , mWasPlayingBeforeGame(false)
    , mCurrentIndex(-1)
    , mCurrentMusic(nullptr)
{
    // Estado inicial según Settings
    mEnabled = Settings::getInstance()->getBool("BackgroundMusic");
}

BackgroundMusicManager::~BackgroundMusicManager()
{
    shutdown();
}

// =============================
//  Init / Shutdown
// =============================

void BackgroundMusicManager::init()
{
    if (mInitialized)
        return;

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
    {
        LOG(LogError) << "BackgroundMusicManager - Error al abrir audio: "
                      << Mix_GetError();
        return;
    }

    Mix_AllocateChannels(16);
    Mix_HaltMusic();

    // Callback para saber cuándo termina una pista
    Mix_HookMusicFinished(&BackgroundMusicManager::musicFinishedCallbackStatic);

    buildPlaylist();
    mInitialized = true;

    LOG(LogInfo) << "BackgroundMusicManager - SDL_mixer inicializado.";
    LOG(LogInfo) << "BackgroundMusicManager - Playlist size: " << mPlaylist.size();
    LOG(LogInfo) << "BackgroundMusicManager - Estado inicial: "
                 << (mEnabled ? "ON" : "OFF");

    // Si está deshabilitada en Settings, no arrancamos música todavía
    if (!mEnabled)
        return;

    if (!mPlaylist.empty())
        playCurrent();
}

void BackgroundMusicManager::shutdown()
{
    if (!mInitialized)
        return;

    stopMusicInternal(false);

    Mix_HookMusicFinished(nullptr);

    Mix_CloseAudio();
    mInitialized = false;
    mPlaylist.clear();
    mCurrentIndex = -1;

    LOG(LogInfo) << "BackgroundMusicManager - Shutdown";
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

    LOG(LogInfo) << "BackgroundMusicManager - setEnabled("
                 << (enabled ? "true" : "false") << ")";

    // Si todavía no está inicializado, lo hacemos ahora si se habilita
    if (!mInitialized)
    {
        if (mEnabled)
            init();
        return;
    }

    // Si se desactiva → fade out y no recordamos estado previo
    if (!mEnabled)
    {
        mWasPlayingBeforeGame = false;
        stopMusicInternal(true);
        return;
    }

    // Si se activa y hay playlist → reproducir
    if (mPlaylist.empty())
        buildPlaylist();

    if (!mPlaylist.empty())
        playCurrent();
}

// =============================
//  Hooks de juego
// =============================

void BackgroundMusicManager::onGameLaunched()
{
    if (!mInitialized)
        return;

    mGameRunning = true;

    // Recordamos si había música sonando antes del juego
    mWasPlayingBeforeGame = (Mix_PlayingMusic() != 0);

    LOG(LogInfo) << "BackgroundMusicManager - onGameLaunched() → deteniendo música "
                 << (mWasPlayingBeforeGame ? "(estaba sonando)" : "(no había música)");

    // Detenemos con fade
    stopMusicInternal(true);
}

void BackgroundMusicManager::onGameEnded()
{
    if (!mInitialized)
        return;

    mGameRunning = false;

    LOG(LogInfo) << "BackgroundMusicManager - onGameEnded()";

    // Si el usuario apagó la música desde el menú, no hacemos nada
    if (!mEnabled)
        return;

    // Si antes del juego no había música, tampoco reanudamos
    if (!mWasPlayingBeforeGame)
        return;

    // Nos aseguramos de tener playlist
    if (mPlaylist.empty())
        buildPlaylist();

    if (!mPlaylist.empty())
    {
        LOG(LogInfo) << "BackgroundMusicManager - onGameEnded() → reanudando música";
        playCurrent();
    }
}

// =============================
//  Playlist
// =============================

void BackgroundMusicManager::buildPlaylist()
{
    mPlaylist.clear();
    mCurrentIndex = -1;

    const std::string home = Utils::FileSystem::getHomePath();

    // RetroPie clásico
    buildPlaylistFromPath(home + "/RetroPie/music");

    // Carpeta alternativa, como hace Batocera/ES-DE
    buildPlaylistFromPath(home + "/.emulationstation/music");

    if (!mPlaylist.empty())
    {
        mCurrentIndex = 0;
        LOG(LogInfo) << "BackgroundMusicManager - Playlist size: "
                     << mPlaylist.size();
    }
    else
    {
        LOG(LogInfo) << "BackgroundMusicManager - No se encontraron pistas de música.";
    }
}

void BackgroundMusicManager::buildPlaylistFromPath(const std::string& path)
{
    if (!Utils::FileSystem::exists(path) || !Utils::FileSystem::isDirectory(path))
        return;

    auto entries = Utils::FileSystem::getDirContent(path);

    for (const auto& e : entries)
    {
        if (Utils::FileSystem::isDirectory(e))
            continue;

        if (isValidAudioFile(e))
            mPlaylist.push_back(e);
    }
}

bool BackgroundMusicManager::isValidAudioFile(const std::string& path) const
{
    const std::string ext = Utils::String::toLower(
        Utils::FileSystem::getExtension(path));

    if (ext == ".mp3" || ext == ".ogg" || ext == ".flac" ||
        ext == ".wav" || ext == ".mod")
        return true;

    return false;
}

// =============================
//  Reproducción
// =============================

void BackgroundMusicManager::playCurrent()
{
    if (!mInitialized)
        return;

    if (!mEnabled)
        return;

    if (mPlaylist.empty() || mCurrentIndex < 0 ||
        mCurrentIndex >= (int)mPlaylist.size())
        return;

    // Liberar pista anterior si existe
    if (mCurrentMusic)
    {
        Mix_HaltMusic();
        Mix_FreeMusic(mCurrentMusic);
        mCurrentMusic = nullptr;
    }

    const std::string& path = mPlaylist[mCurrentIndex];

    mCurrentMusic = Mix_LoadMUS(path.c_str());
    if (!mCurrentMusic)
    {
        LOG(LogError) << "BackgroundMusicManager - Error al cargar: " << path
                      << " (" << Mix_GetError() << ")";
        // Probar siguiente pista
        playNext();
        return;
    }

    LOG(LogInfo) << "BackgroundMusicManager - Playing: " << path;

    // 0 = reproducir una vez, luego dispara el callback
    if (Mix_PlayMusic(mCurrentMusic, 0) == -1)
    {
        LOG(LogError) << "BackgroundMusicManager - Mix_PlayMusic error: "
                      << Mix_GetError();
    }
}

void BackgroundMusicManager::stopMusicInternal(bool fadeOut)
{
    if (!mInitialized)
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

void BackgroundMusicManager::playNext()
{
    if (!mInitialized)
        return;

    if (mPlaylist.empty())
        return;

    mCurrentIndex++;
    if (mCurrentIndex >= (int)mPlaylist.size())
        mCurrentIndex = 0;

    playCurrent();
}

// =============================
//  Callbacks de fin de pista
// =============================

void BackgroundMusicManager::musicFinishedCallbackStatic()
{
    if (sInstance)
        sInstance->musicFinishedCallback();
}

void BackgroundMusicManager::musicFinishedCallback()
{
    // Si hay juego corriendo, no avanzar playlist
    if (mGameRunning)
        return;

    playNext();
}
