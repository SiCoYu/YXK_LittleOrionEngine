#include "NecroBurrow.h"

#include "Component/ComponentAudioSource.h"
#include "Component/ComponentBillboard.h"
#include "Component/ComponentParticleSystem.h"
#include "Component/ComponentScript.h"
#include "Component/ComponentTransform.h"

#include "Main/Application.h"
#include "Main/GameObject.h"
#include "Module/ModuleInput.h"
#include "Module/ModuleScene.h"
#include "Module/ModuleTime.h"

#include <imgui.h>


NecroBurrow* NecroBurrowDLL()
{
	NecroBurrow* instance = new NecroBurrow();
	return instance;
}

NecroBurrow::NecroBurrow()
{
}

// Use this for initialization before Start()
void NecroBurrow::Awake()
{
	burrow_source = static_cast<ComponentAudioSource*>(owner->GetComponent(Component::ComponentType::AUDIO_SOURCE));
	burrow_billboard = static_cast<ComponentBillboard*>(owner->children[0]->GetComponent(Component::ComponentType::BILLBOARD));
	dirt_1_particles = static_cast<ComponentParticleSystem*>(owner->children[1]->GetComponent(Component::ComponentType::PARTICLE_SYSTEM));
	dirt_2_particles = static_cast<ComponentParticleSystem*>(owner->children[2]->GetComponent(Component::ComponentType::PARTICLE_SYSTEM));
	dust_1_particles = static_cast<ComponentParticleSystem*>(owner->children[3]->GetComponent(Component::ComponentType::PARTICLE_SYSTEM));
}

// Use this for initialization
void NecroBurrow::Start()
{

}

// Update is called once per frame
void NecroBurrow::Update()
{
	if (showing)
	{
		current_time += App->time->delta_time;
		float current_progress = math::Min(1.f, current_time / transition_time);

		float current_size = math::Lerp(0.f, 2.f, current_progress);
		burrow_billboard->width = current_size;
		burrow_billboard->height = current_size;

		if (current_progress > 0.5f && !sound_already_played)
		{
			burrow_source->PlayEvent("Stop_NecroBurrow");
			sound_already_played = true;
		}

		if (current_progress == 1.f)
		{
			showing = false;
		}
	}
	
	if (hiding)
	{
		current_time += App->time->delta_time;
		float current_progress = math::Min(1.f, current_time / transition_time);

		float current_size = math::Lerp(2.5f, 0.f, current_progress);
		burrow_billboard->width = current_size;
		burrow_billboard->height = current_size;

		if (current_progress > 0.5f && !sound_already_played)
		{
			burrow_source->PlayEvent("Stop_Necroburrow");
			sound_already_played = true;
		}

		if (current_progress == 1.f)
		{
			hiding = false;
		}
	}
}

void NecroBurrow::Show()
{
	showing = true;
	current_time = 0.f;

	owner->SetEnabled(true);
	burrow_billboard->width = 0.f;
	burrow_billboard->height = 0.f;

	dirt_1_particles->Play();
	dirt_2_particles->Play();
	dust_1_particles->Play();

	burrow_source->PlayEvent("Play_Necroburrow");
	sound_already_played = false;
}

void NecroBurrow::Hide()
{
	hiding = true;
	current_time = 0.f;

	owner->SetEnabled(true);
	burrow_billboard->width = 2.5f;
	burrow_billboard->height = 2.5f;

	dirt_1_particles->Play();
	dirt_2_particles->Play();
	dust_1_particles->Play();

	burrow_source->PlayEvent("Play_Necroburrow");
	sound_already_played = false;
}

// Use this for showing variables on inspector
void NecroBurrow::OnInspector(ImGuiContext* context)
{
	//Necessary to be able to write with imgui
	ImGui::SetCurrentContext(context);
	ShowDraggedObjects();

}