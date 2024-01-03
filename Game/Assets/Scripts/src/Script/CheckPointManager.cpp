#include "CheckPointManager.h"

#include "Component/ComponentScript.h"
#include "Component/ComponentTransform.h"

#include "Main/Application.h"
#include "Main/GameObject.h"

#include "Module/ModuleInput.h"
#include "Module/ModuleScene.h"

#include <imgui.h>

#include "CameraController.h"
#include "EventManager.h"
#include "DebugModeScript.h"
#include "Log/EngineLog.h"
#include "PlayerController.h"
#include "PlayerAttack.h"
#include "TeleportPoint.h"
#include "WorldManager.h"



CheckPointManager* CheckPointManagerDLL()
{
	CheckPointManager* instance = new CheckPointManager();
	return instance;
}


CheckPointManager::CheckPointManager()
{

}


// Use this for initialization before Start()
void CheckPointManager::Awake()
{
	GameObject* world_go = App->scene->GetGameObjectByName("World Manager");
	ComponentScript* world_component = world_go->GetComponentScript("WorldManager");
	world = static_cast<WorldManager*>(world_component->script);

	GameObject* event_go = App->scene->GetGameObjectByName("EventManager");
	ComponentScript* event_component = event_go->GetComponentScript("EventManager");
	event_manager = static_cast<EventManager*>(event_component->script);

	GameObject* camera_go = App->scene->GetGameObjectByName("Camera Holder");
	ComponentScript* camera_component = camera_go->GetComponentScript("CameraController");
	camera_controller = static_cast<CameraController*>(camera_component->script);
}



// Use this for initialization

void CheckPointManager::Start()
{
	player_1 = world->GetPlayer1();
	player_2 = world->GetPlayer2();

	for (unsigned int i = 0; i < owner->children.size(); ++i)
	{
		checkpoints.push_back(static_cast<TeleportPoint*>(owner->children[i]->GetComponentScript("TeleportPoint")->script));
	}
	if (checkpoints.size() > 0)
	{
		last_checkpoint = checkpoints[0];
	}
}



void CheckPointManager::RespawnOnLastCheckPoint()
{
	last_checkpoint = GetClosestCheckpoint(event_manager->current_zone);
	if(!world->ThereIsBoss())
	{
		event_manager->RestartEvents(last_checkpoint->zone_to_spawn);

	}

	if (event_manager->UpdateZone(last_checkpoint->zone_to_spawn))
	{
		App->engine_log->Log("Respawn at zone %d", last_checkpoint->zone_to_spawn);
		camera_controller->freeze = false;
		camera_controller->fixed_position = false;
		//respawn the player
		player_1->player_controller->Respawn();
		if (!world->multiplayer)
		{
			player_1->player_go->transform.SetGlobalMatrixTranslation(last_checkpoint->owner->transform.GetGlobalTranslation());
		}
		else
		{
			player_1->player_go->transform.SetGlobalMatrixTranslation(last_checkpoint->owner->transform.GetGlobalTranslation());
			player_2->player_go->transform.SetGlobalMatrixTranslation(last_checkpoint->owner->transform.GetGlobalTranslation() + teleport_offset);
			player_2->player_controller->Respawn();
		}
	}

}


TeleportPoint* CheckPointManager::GetClosestCheckpoint(int current_zone) const
{
	TeleportPoint* closest_checkpoint = checkpoints[0];
	int shortest_distance = 10;
	int new_distance;
	for (unsigned int i = 0; i < checkpoints.size() ; ++i)
	{
		new_distance = std::abs(checkpoints[i]->zone_to_spawn - current_zone);
		if (new_distance < shortest_distance && current_zone >= checkpoints[i]->zone_to_spawn)
		{
			closest_checkpoint = checkpoints[i];
			shortest_distance = new_distance;
		}
	}

	return closest_checkpoint;
}