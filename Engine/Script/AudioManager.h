#ifndef  __AUDIOMANAGER_H__
#define  __AUDIOMANAGER_H__

#include "Script.h"

class ComponentAudioSource;
class EnvironmentSFX;

class AudioManager : public Script
{
public:
	AudioManager();
	~AudioManager() = default;

	void Awake() override;
	void Start() override;
	void Update() override;

	void OnInspector(ImGuiContext*) override;
	void InitPublicGameObjects();

	void InitializeAudioSources();

private:
	void PlayAudioOnAwake();

public:
	ComponentAudioSource* audio_source_ui = nullptr;
	ComponentAudioSource* audio_source_ambient = nullptr;
	ComponentAudioSource* audio_source_music = nullptr;

private:
	std::vector <GameObject*> gos_with_audio;
};

extern "C" SCRIPT_API AudioManager* AudioManagerDLL(); //This is how we are going to load the script

#endif