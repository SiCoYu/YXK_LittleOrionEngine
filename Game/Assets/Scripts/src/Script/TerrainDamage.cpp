#include "TerrainDamage.h"

#include "Component/ComponentCollider.h"
#include "Component/ComponentScript.h"
#include "Component/ComponentTransform.h"


#include "Main/Application.h"
#include "Main/GameObject.h"
#include "Module/ModuleInput.h"
#include "Module/ModuleScene.h"
#include "Module/ModuleTime.h"

#include "EnemyController.h"
#include "EventManager.h"
#include "PlayerController.h"
#include "WorldManager.h"

#include <imgui.h>



TerrainDamage* TerrainDamageDLL()
{
	TerrainDamage* instance = new TerrainDamage();
	return instance;

}


TerrainDamage::TerrainDamage()
{

}


// Use this for initialization before Start()
void TerrainDamage::Awake()
{
	collider = static_cast<ComponentCollider*>(owner->GetComponent(Component::ComponentType::COLLIDER));
	GameObject* world_manager_go = App->scene->GetGameObjectByName("World Manager");
	world = static_cast<WorldManager*>(world_manager_go->GetComponentScript("WorldManager")->script);
	event_manager = static_cast<EventManager*>(App->scene->GetGameObjectByName("EventManager")->GetComponentScript("EventManager")->script);
}


// Use this for initialization
void TerrainDamage::Start()
{
	player_1 = world->GetPlayer1();
	player_2 = world->GetPlayer2();
}



// Update is called once per frame
void TerrainDamage::Update()
{
	if (world->on_pause)
	{
		return;
	}
	if (event_manager->current_zone != zone)
	{
		return;
	}

	if (player_1->player_controller->is_alive &&
		static_cast<ComponentCollider*>(player_1->player_go->GetComponent(Component::ComponentType::COLLIDER))->DetectCollisionWith(collider).collider)
	{
		ComputeDamage(0);
	}

	if (world->multiplayer && player_2->player_controller->is_alive &&
		static_cast<ComponentCollider*>(player_2->player_go->GetComponent(Component::ComponentType::COLLIDER))->DetectCollisionWith(collider).collider)
	{
		ComputeDamage(1);
	}

}

// Use this for showing variables on inspector
void TerrainDamage::OnInspector(ImGuiContext* context)
{
	//Necessary to be able to write with imgui
	ImGui::SetCurrentContext(context);

	if (ImGui::BeginCombo("Terrain Type", terrain_name.c_str()))
	{
		
		if (ImGui::Selectable("Water"))
		{
			terrain_type = TerrainType::CORRUPTED_WATER;
			terrain_name = "Water";
		}

		if (ImGui::Selectable("Poison"))
		{
			terrain_type = TerrainType::CORRUPTED_POISON;
			terrain_name = "Poison";
		}

		if (ImGui::Selectable("Magma"))
		{
			terrain_type = TerrainType::MAGMA;
			terrain_name = "Magma";
		}
		
		ImGui::EndCombo();
	}
	ImGui::DragInt("Zone", &zone);

	ShowDraggedObjects();
}


//Use this for linking JUST GO automatically 
void TerrainDamage::InitPublicGameObjects()
{
	//IMPORTANT, public gameobjects, name_gameobjects and go_uuids MUST have same size

	for (int i = 0; i < public_gameobjects.size(); ++i)
	{
		name_gameobjects.push_back(is_object);
		go_uuids.push_back(0);
	}

}

void TerrainDamage::Save(Config & config) const
{
	config.AddInt((int) terrain_type, "TerrainType");
	config.AddInt(zone, "TerrainZone");
	Script::Save(config);
}

void TerrainDamage::Load(const Config & config)
{
	terrain_type = static_cast<TerrainType>(config.GetInt("TerrainType", 0));

	if(terrain_type == TerrainType::CORRUPTED_WATER)
	{
		terrain_name = "Water";
	}
	else if(terrain_type == TerrainType::MAGMA)
	{
		terrain_name = "Magma";
	}

	zone = config.GetInt("TerrainZone", 0);
	Script::Load(config);
}

void TerrainDamage::ComputeDamage(unsigned player) const
{
	float damage_dealt = 0.f;
	
	float time_multiplier = App->time->delta_time / 1000.f;

	switch (terrain_type)
	{
		case TerrainType::CORRUPTED_WATER:
			damage_dealt = water_damage;
			break;
		case TerrainType::CORRUPTED_POISON:
			damage_dealt = poison_damage * time_multiplier;
			break;
		case TerrainType::MAGMA:
			damage_dealt = magma_damage * time_multiplier;
			break;
		default:
			break;
	}

	(player == 0) ? player_1->player_controller->TakeDamage(damage_dealt, player_1->player_controller->hit_effect) : player_2->player_controller->TakeDamage(damage_dealt, player_2->player_controller->hit_effect);

}