#include "AudioManager.h"

#include "Log.h"
#include "Settings.h"
#include "Sound.h"

// ✅ Mantener símbolos estáticos por compatibilidad
std::vector<std::shared_ptr<Sound>> AudioManager::sSoundVector;
SDL_AudioSpec AudioManager::sAudioFormat;
std::shared_ptr<AudioManager> AudioManager::sInstance;

// ✅ Ya no usamos el mixer crudo de SDL, pero dejamos la función
// para que el header y cualquier referencia vieja no rompan.
void AudioManager::mixAudio(void* /*unused*/, Uint8* /*stream*/, int /*len*/)
{
	// NO-OP: mezcla de samples ahora no aplica (usamos SDL_mixer para SFX/BGM).
}

AudioManager::AudioManager()
{
	// NO hacemos init de SDL audio acá.
	// SDL_mixer (BGM / SFX) es quien debe poseer el dispositivo.
}

AudioManager::~AudioManager()
{
	// No cerramos SDL audio acá.
	// Si alguien llama deinit(), igual lo tratamos como NO-OP seguro.
}

std::shared_ptr<AudioManager>& AudioManager::getInstance()
{
	if (sInstance == nullptr && Settings::getInstance()->getBool("EnableSounds"))
		sInstance = std::shared_ptr<AudioManager>(new AudioManager);

	return sInstance;
}

void AudioManager::init()
{
	// NO-OP
	// Antes: SDL_InitSubSystem(SDL_INIT_AUDIO) + SDL_OpenAudio(...)
	// Ahora: evitamos conflicto con SDL_mixer y con audio de video (VLC/ALSA/Pulse).
}

void AudioManager::deinit()
{
	// NO-OP: no cerramos SDL audio crudo
	sInstance = nullptr;
}

void AudioManager::registerSound(std::shared_ptr<Sound>& sound)
{
	getInstance();
	sSoundVector.push_back(sound);
}

void AudioManager::unregisterSound(std::shared_ptr<Sound>& sound)
{
	getInstance();
	for (unsigned int i = 0; i < sSoundVector.size(); i++)
	{
		if (sSoundVector.at(i) == sound)
		{
			sSoundVector.erase(sSoundVector.cbegin() + i);
			return;
		}
	}

	LOG(LogWarning) << "AudioManager: tried to unregister a sound that wasn't registered.";
}

void AudioManager::play()
{
	// NO-OP:
	// Antes despertaba SDL_OpenAudio() con SDL_PauseAudio(0).
	// Con SDL_mixer, Sound::play() se encarga.
	getInstance();
}

void AudioManager::stop()
{
	// Opcional: pedimos stop() a los Sound registrados
	// (si tu Sound nuevo usa SDL_mixer, esto puede frenar canales activos).
	for (auto& s : sSoundVector)
	{
		if (s)
			s->stop();
	}
}
