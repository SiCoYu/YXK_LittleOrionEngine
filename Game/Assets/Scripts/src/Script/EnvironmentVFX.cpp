#include "EnvironmentVFX.h"

#include "Component/ComponentParticleSystem.h"
#include "Component/ComponentScript.h"
#include "Component/ComponentTransform.h"
#include "Main/Application.h"
#include "Main/GameObject.h"
#include "Module/ModuleInput.h"
#include "Module/ModuleScene.h"

#include <imgui.h>

EnvironmentVFX* EnvironmentVFXDLL()
{
	EnvironmentVFX* instance = new EnvironmentVFX();
	return instance;
}

EnvironmentVFX::EnvironmentVFX()
{

}

// Use this for initialization before Start()
void EnvironmentVFX::Awake()
{
	GetVFX();
	StopVFX();
}

// Use this for showing variables on inspector
void EnvironmentVFX::OnInspector(ImGuiContext* context)
{
	//Necessary to be able to write with imgui
	ImGui::SetCurrentContext(context);
	ShowDraggedObjects();
}

void EnvironmentVFX::StartVFX()
{
	for (auto& particle : vfx)
	{
		particle->Play();
	}
}

void EnvironmentVFX::StopVFX()
{
	for(auto& particle : vfx)
	{
		particle->Stop();
	}
}

void EnvironmentVFX::GetVFX()
{
	for (unsigned i = 0; i < owner->children.size(); ++i)
	{
		vfx.emplace_back(static_cast<ComponentParticleSystem*>(owner->children[i]->GetComponent(Component::ComponentType::PARTICLE_SYSTEM)));
	}
}
