#include "AudioManager.h"

#include "Component/ComponentAudioSource.h"
#include "Component/ComponentScript.h"
#include "Component/ComponentTransform.h"

#include "Main/Application.h"
#include "Main/GameObject.h"
#include "Module/ModuleInput.h"
#include "Module/ModuleScene.h"

#include "EnvironmentSFX.h"

#include <imgui.h>



AudioManager* AudioManagerDLL()
{
	AudioManager* instance = new AudioManager();
	return instance;
}

AudioManager::AudioManager()
{

}

// Use this for initialization before Start()
void AudioManager::Awake()
{
	gos_with_audio = App->scene->GetGameObjectsWithComponent(Component::ComponentType::AUDIO_SOURCE);
	InitializeAudioSources();
}

// Use this for initialization
void AudioManager::Start()
{
	PlayAudioOnAwake();
}

// Update is called once per frame
void AudioManager::Update()
{

}

// Use this for showing variables on inspector
void AudioManager::OnInspector(ImGuiContext* context)
{
	//Necessary to be able to write with imgui
	ImGui::SetCurrentContext(context);
	ShowDraggedObjects();
}

//Use this for linking JUST GO automatically 
void AudioManager::InitPublicGameObjects()
{
	//IMPORTANT, public gameobjects, name_gameobjects and go_uuids MUST have same size
	for (unsigned int i = 0; i < public_gameobjects.size(); ++i)
	{
		name_gameobjects.push_back(is_object);
		go_uuids.push_back(0);
	}
}

void AudioManager::InitializeAudioSources()
{
	GameObject* audio_go = nullptr;
	audio_go = App->scene->GetGameObjectByName("AudioUI");
	if (audio_go != nullptr)
	{
		audio_source_ui = static_cast<ComponentAudioSource*>(audio_go->GetComponent(Component::ComponentType::AUDIO_SOURCE));
		audio_go = nullptr;
	}
	audio_go = App->scene->GetGameObjectByName("AudioAmbient");
	if (audio_go != nullptr)
	{
		audio_source_ambient = static_cast<ComponentAudioSource*>(audio_go->GetComponent(Component::ComponentType::AUDIO_SOURCE));
		audio_go = nullptr;
	}
	audio_go = App->scene->GetGameObjectByName("AudioMusic");
	if (audio_go != nullptr)
	{
		audio_source_music = static_cast<ComponentAudioSource*>(audio_go->GetComponent(Component::ComponentType::AUDIO_SOURCE));
	}
}

void AudioManager::PlayAudioOnAwake()
{
	for (unsigned int i = 0; i < gos_with_audio.size(); ++i)
	{
		ComponentAudioSource* audio_source = static_cast<ComponentAudioSource*>(gos_with_audio[i]->GetComponent(Component::ComponentType::AUDIO_SOURCE));
		if (audio_source->play_on_awake)
		{
			audio_source->PlayAwake();
		}
	}
}