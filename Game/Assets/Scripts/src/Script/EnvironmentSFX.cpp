#include "EnvironmentSFX.h"

#include "Component/ComponentAudioSource.h"
#include "Component/ComponentScript.h"
#include "Component/ComponentTransform.h"

#include "Main/Application.h"
#include "Main/GameObject.h"
#include "Module/ModuleInput.h"
#include "Module/ModuleScene.h"

#include <imgui.h>

EnvironmentSFX* EnvironmentSFXDLL()
{
	EnvironmentSFX* instance = new EnvironmentSFX();
	return instance;
}

EnvironmentSFX::EnvironmentSFX()
{

}

// Use this for initialization before Start()
void EnvironmentSFX::Awake()
{
	GetSFX();
}

// Use this for showing variables on inspector
void EnvironmentSFX::OnInspector(ImGuiContext* context)
{
	//Necessary to be able to write with imgui
	ImGui::SetCurrentContext(context);
	ShowDraggedObjects();
}

void EnvironmentSFX::PlayEventSFX()
{
	for (auto& audio : sfx)
	{
		audio->PlayEvent(audio->GetEventName());
	}
}

void EnvironmentSFX::GetSFX()
{
	for (unsigned i = 0; i < owner->children.size(); ++i)
	{
		sfx.emplace_back(static_cast<ComponentAudioSource*>(owner->children[i]->GetComponent(Component::ComponentType::AUDIO_SOURCE)));
	}
}
