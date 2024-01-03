#include "LightShield.h"

#include "Component/ComponentAnimation.h"
#include "Component/ComponentAudioSource.h"
#include "Component/ComponentBillboard.h"
#include "Component/ComponentScript.h"
#include "Component/ComponentTransform.h"

#include "Main/Application.h"
#include "Main/GameObject.h"
#include "Module/ModuleInput.h"
#include "Module/ModuleScene.h"
#include "Module/ModuleTime.h"

#include <imgui.h>

#include "EnemyController.h"
#include "EnemyManager.h"
#include "PlayerController.h"
#include "PlayerMovement.h"
#include "UIManager.h"


namespace
{
	const float max_durability = 100.f;
	const float recover_factor = 25.f;
	const float using_factor = 25.f;
	const float default_parry_time = 750.f;
	const float minimum_parry_time = 200.f;
}


LightShield* LightShieldDLL()
{
	LightShield* instance = new LightShield();
	return instance;
}


// Use this for initialization before Start()
void LightShield::Awake()
{
	enemy_manager = static_cast<CEnemyManager*>(App->scene->GetGameObjectByName("EnemyManager")->GetComponentScript("EnemyManager")->script);
	player_controller = static_cast<PlayerController*>(owner->GetComponentScript("PlayerController")->script);

	GameObject* ui = App->scene->GetGameObjectByName("UIManager");
	ComponentScript* component_ui = ui->GetComponentScript("UIManager");
	ui_manager = static_cast<UIManager*>(component_ui->script);
	audio_source = static_cast<ComponentAudioSource*>(owner->GetComponent(Component::ComponentType::AUDIO_SOURCE));

	InitVFX();
}


// Use this for initialization
void LightShield::Start()
{
	small_texture_DB_VFX->loop = true;
	big_texture_DB_VFX->loop = true;
	smoke_texture_DB_VFX->loop = true;

	//Init parry time
	current_parry_time = default_parry_time;
}


// Update is called once per frame
void LightShield::Update()
{
	ui_manager->UpdateShieldlUI(static_cast<uint32_t>(player_controller->player), durability / max_durability);

	if(broken && durability >= max_durability)
	{
		broken = false;
	}

	if(!active && durability < max_durability)
	{
		float recover = App->time->delta_time * 0.001f * recover_factor;
		recover = (broken) ? recover / 5.f : recover;
		durability += recover;
		durability = math::Clamp(durability, 0.f, max_durability);
		
	}

	if(parry_enable)
	{
		current_time_activated += App->time->delta_time;
		if(current_time_activated > default_parry_time)
		{
			parry_enable = false;
			current_time_activated = 0.f;		
		}
	}

	if(active)
	{
		durability = math::Clamp(durability - App->time->delta_time * 0.001f * using_factor, 0.f, max_durability);

		if (durability <= 0)
		{
			BreakShield();
		}
	}
	player_controller->animation->SetBool("shield", active);
}



// Use this for showing variables on inspector
void LightShield::OnInspector(ImGuiContext* context)
{
	//Necessary to be able to write with imgui
	ImGui::SetCurrentContext(context);
	ShowDraggedObjects();

	ImGui::Checkbox("Active", &active);
	ImGui::Checkbox("Broken", &broken);
	ImGui::DragFloat("Durability: ", &durability);

	ImGui::DragFloat("Current parry time: %.3f", &current_parry_time);


}



//Use this for linking JUST GO automatically 
void LightShield::InitPublicGameObjects()
{
	//IMPORTANT, public gameobjects, name_gameobjects and go_uuids MUST have same size
	for (int i = 0; i < public_gameobjects.size(); ++i)
	{
		name_gameobjects.push_back(is_object);
		go_uuids.push_back(0);
	}
}

void LightShield::CastShield()
{
	if(broken)
	{
		return;
	}

	active = true;
	parry_enable = true;
	audio_source->PlayEvent("play_light_shield_cast");
	audio_source->PlayEvent("play_light_shield_loop");

	//Slow player if shield active
	player_controller->player_movement->SlowPlayer(60.f);

	small_texture_DB_VFX->Play();
	big_texture_DB_VFX->Play();
	smoke_texture_DB_VFX->Play();
}

void LightShield::UncastShield()
{
	active = false;

	parry_enable = false;
	current_time_activated = 0.f;

	audio_source->PlayEvent("stop_light_shield_loop");

	//Restore movement speed to default
	player_controller->player_movement->SlowPlayer(0.f);

	small_texture_DB_VFX->Disable();
	big_texture_DB_VFX->Disable();
	smoke_texture_DB_VFX->Disable();
}

void LightShield::FillDurability()
{
	durability = max_durability;
}

float LightShield::BlockAttack(float damage)
{
	durability -= damage;

	if(durability <= 0)
	{
		BreakShield();
		return abs(durability);
	}

	return 0.f;
}

void LightShield::BreakShield()
{
	//Stun player and knockback enemies
	active = false;
	durability = 0.f;
	broken = true;
	ShockWave();
	audio_source->PlayEvent("play_light_shield_break");
	player_controller->Stun(breaking_shield_stun_time);
	break_shield_texture_DB_VFX->Play();
	small_texture_DB_VFX->Disable();
	big_texture_DB_VFX->Disable();
	smoke_texture_DB_VFX->Disable();
}

void LightShield::ShockWave()
{
	for(auto& enemy : enemy_manager->enemies)
	{
		if(enemy->is_alive && owner->transform.GetGlobalTranslation().Distance(enemy->owner->transform.GetGlobalTranslation()) <= shield_radius)
		{
			enemy->knockback_direction = enemy->owner->transform.GetGlobalTranslation() - owner->transform.GetGlobalTranslation();
			enemy->TakeDamage(0.f, owner->transform.GetGlobalTranslation(), 100.f);
		}
	}
}

bool LightShield::IsActive() const
{
	return active;
}

void LightShield::InitVFX()
{
	//VFX
	for (const auto& go : owner->children)
	{

		if (go->name == "ShieldEffect")
		{
			for (const auto& shield_go : go->children)
			{
				if (shield_go->name == "SmallTexture")
				{
					small_texture_DB_VFX = static_cast<ComponentBillboard*>(shield_go->GetComponent(Component::ComponentType::BILLBOARD));
					small_texture_DB_VFX->Disable();
				}
				if (shield_go->name == "ExteriorTexture")
				{
					big_texture_DB_VFX = static_cast<ComponentBillboard*>(shield_go->GetComponent(Component::ComponentType::BILLBOARD));
					big_texture_DB_VFX->Disable();
				}
				if (shield_go->name == "ShineTexture")
				{
					smoke_texture_DB_VFX = static_cast<ComponentBillboard*>(shield_go->GetComponent(Component::ComponentType::BILLBOARD));
					smoke_texture_DB_VFX->Disable();
				}
				if (shield_go->name == "BreakShieldTexture")
				{
					break_shield_texture_DB_VFX = static_cast<ComponentBillboard*>(shield_go->GetComponent(Component::ComponentType::BILLBOARD));
					break_shield_texture_DB_VFX->Disable();
				}
				
			}
		}

	}

}

void LightShield::ParryLightBall()
{
	float auxiliar_time = current_parry_time / 1.5f;
	current_parry_time = math::Clamp(auxiliar_time, minimum_parry_time, default_parry_time);
}

void LightShield::ResetParryTime()
{
	current_parry_time = default_parry_time;
}
