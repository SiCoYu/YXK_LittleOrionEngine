#include "EventManager.h"

#include "Component/ComponentAnimation.h"
#include "Component/ComponentAudioSource.h"
#include "Component/ComponentCollider.h"
#include "Component/ComponentParticleSystem.h"
#include "Component/ComponentScript.h"
#include "Component/ComponentTransform.h"

#include "Log/EngineLog.h"
#include "Main/Application.h"
#include "Main/GameObject.h"
#include "Module/ModuleInput.h"
#include "Module/ModuleScene.h"
#include "Module/ModuleTime.h"

#include "ResourceManagement/Resources/Material.h"

#include "imgui.h"

#include "AudioManager.h"
#include "BattleEvent.h"
#include "CameraController.h"
#include "CameraLine.h"
#include "CameraShake.h"
#include "CinematicCamera.h"
#include "CheckPointManager.h"
#include "Dissolver.h"
#include "EnemyManager.h"
#include "EnvironmentSFX.h"
#include "EnvironmentVFX.h"
#include "PlayerController.h"
#include "PopupText.h"
#include "TeleportPoint.h"
#include "UIManager.h"
#include "WorldManager.h"

CEventManager* EventManagerDLL()
{
	CEventManager* instance = new CEventManager();
	return instance;
}

CEventManager::CEventManager()
{
	
}

// Use this for initialization before Start()
void CEventManager::Awake()
{
	GameObject* world_manager_go = App->scene->GetGameObjectByName("World Manager");
	ComponentScript* world_manager_component = world_manager_go->GetComponentScript("WorldManager");
	world = static_cast<WorldManager*>(world_manager_component->script);

	GameObject* enemy_manager_go = App->scene->GetGameObjectByName("EnemyManager");
	ComponentScript* enemy_manager_component = enemy_manager_go->GetComponentScript("EnemyManager");
	enemy_manager = static_cast<CEnemyManager*>(enemy_manager_component->script);

	GameObject* audio_manager_go = App->scene->GetGameObjectByName("AudioManager");
	ComponentScript* audio_manager_component = audio_manager_go->GetComponentScript("AudioManager");
	audio_manager = static_cast<AudioManager*>(audio_manager_component->script);
	
	GameObject* camera_controller_go = App->scene->GetGameObjectByName("Camera Holder");
	ComponentScript* camera_controller_component = camera_controller_go->GetComponentScript("CameraController");
	camera_controller = static_cast<CameraController*>(camera_controller_component->script);

	GameObject* ui_manager_go = App->scene->GetGameObjectByName("UIManager");
	ComponentScript* ui_manager_component = ui_manager_go->GetComponentScript("UIManager");
	ui_manager = static_cast<UIManager*>(ui_manager_component->script);

	GameObject* checkpoint_manager_go = App->scene->GetGameObjectByName("CheckPoints");
	ComponentScript* checkpoint_manager_component = checkpoint_manager_go->GetComponentScript("CheckPointManager");
	checkpoint_manager = static_cast<CheckPointManager*>(checkpoint_manager_component->script);

	GameObject* win_go= App->scene->GetGameObjectByName("Level End");
	level_ending = static_cast<ComponentCollider*>(win_go->GetComponent(Component::ComponentType::COLLIDER));

	GameObject* dust_roots = App->scene->GetGameObjectByName("DustRootsVFX");
	dust_roots_VFX = static_cast<ComponentParticleSystem*>(dust_roots->GetComponent(Component::ComponentType::PARTICLE_SYSTEM));
	dust_roots_SFX = static_cast<ComponentAudioSource*>(dust_roots->GetComponent(Component::ComponentType::AUDIO_SOURCE));

	camera_shake = (CameraShake*)App->scene->GetGameObjectByName("Main Camera")->GetComponentScript("CameraShake")->script;

	GameObject* cinematic_go = App->scene->GetGameObjectByName("Cinematic Camera Holder");
	if(cinematic_go)
	{
		cinematic_camera = static_cast<CinematicCamera*>(cinematic_go->GetComponentScript("CinematicCamera")->script);
	}

	//Event Zones
	GetZones();
	GetPopupsEventZones();
	InitBattleEventZones();

	UpdateCamera();
	UpdateZone(current_zone);
}

// Use this for initialization
void CEventManager::Start()
{
	player_1 = world->GetPlayer1();
	player_2 = world->GetPlayer2();
	dust_roots_VFX->Stop();
	dust_roots_SFX->StopSelectedEvent();
	if (!world->ThereIsBoss()) 
	{
		current_battle = battle_triggers[current_zone];
	}
	else 
	{
		current_battle = static_cast<BattleEvent*>(App->scene->GetGameObjectByName("Battle Boss Event")->GetComponentScript("BattleEvent")->script);
	}
	if (zone_list[current_zone]->zone_vfx != nullptr) 
	{
		zone_list[current_zone]->zone_vfx->StartVFX();
	}
}

// Update is called once per frame
void CEventManager::Update()
{
	if (world->on_pause)
	{
		return;
	}

	if (ending)
	{
		return;
	}
	if (!world->ThereIsBoss())
	{
		CheckBattleEvents();
	}
	CheckPopupTexts();
	//For now the only event we have is enemy spawning
	if(battle_event_triggered && !world->ThereIsBoss())
	{
		BattleEventStatus();
	}

	if(roots_moving)
	{
		MoveRoot();
	}

	if(CheckLose())
	{
		player_1->player_controller->owner->SetEnabled(false);
		player_2->player_controller->owner->SetEnabled(false);
		
		world->on_pause = true;
		ui_manager->SetLoseScreen();

		audio_manager->audio_source_ambient->PlayEvent("play_gameover_music");
		ending = true;
	}

	if(CheckWin())
	{
		world->on_pause = true;
		player_1->player_controller->owner->SetEnabled(false);
		player_2->player_controller->owner->SetEnabled(false);
		if (world->current_level == Level::FIRST)
		{
			world->LoadLevel(Level::SECOND);
			ending = true;
		}
		else if(world->current_level == Level::SECOND)
		{
			world->LoadLevel(Level::BOSS);
			ending = true;
		}
		else if(world->current_level == Level::BOSS)
		{
			world->LoadLevel(Level::MAIN_MENU);
			ending = true;
		}
		else
		{
			ui_manager->SetWinScreen();
			audio_manager->audio_source_ui->PlayEvent("play_win_screen");
			ending = true;
		}
	}
}

// Use this for showing variables on inspector
void CEventManager::OnInspector(ImGuiContext* context)
{
	//Necessary to be able to write with imgui
	ImGui::SetCurrentContext(context);
	ShowDraggedObjects();
	ImGui::DragInt("enemies_killed_on_wave:", &enemies_killed_on_wave);
	ImGui::DragInt("Current Battle Event Beated:", &current_event_beated);
	ImGui::DragInt("Current Zone:", &current_zone);
	ImGui::DragInt("Current Event:", &current_event);
	ImGui::Checkbox("Tutorial activate", &tutorial_activate);
}

//Use this for linking JUST GO automatically 
void CEventManager::InitPublicGameObjects()
{
	//IMPORTANT, public gameobjects, name_gameobjects and go_uuids MUST have same size
	for (int i = 0; i < public_gameobjects.size(); ++i)
	{
		name_gameobjects.push_back(is_object);
		go_uuids.push_back(0);
	}
}

void CEventManager::InitBattleEventZones()
{
	if(world->ThereIsBoss())
	{
		return;
	}

	GameObject* trigger_go_dad = App->scene->GetGameObjectByName("Triggers");
	if(trigger_go_dad != nullptr)
	{
		for (size_t i = 0; i < trigger_go_dad->children.size(); ++i)
		{
			battle_triggers.emplace_back(static_cast<BattleEvent*>(trigger_go_dad->children[i]->GetComponentScript("BattleEvent")->script));
		}	
	}
}

void CEventManager::GetPopupsEventZones()
{
	GameObject* popup_go_dad = App->scene->GetGameObjectByName("PopUps");
	if (popup_go_dad) 
	{
		for (size_t i = 0; i < popup_go_dad->children.size(); ++i)
		{
			popup_triggers.emplace_back(static_cast<PopupText*>(popup_go_dad->children[i]->GetComponentScript("PopupText")->script));
		}
	}
}

bool CEventManager::InitBattleEvent()
{
	////Right now only one event at the same time can occour
	if(battle_event_triggered)
	{
		return false;
	}
	current_battle->ActivateEvent();
	battle_event_triggered = true;
	camera_controller->freeze = true;
	return true;
}

void CEventManager::CheckBattleEvents()
{
	if(current_battle && !current_battle->CheckEvent())
	{
		bool activate = camera_controller->progress_level >= 1.f;

		if(activate)
		{
			InitBattleEvent();
		}
	}
}

void CEventManager::CheckPopupTexts()
{
	for (size_t i = 0; i< popup_triggers.size(); ++i)
	{
		tutorial_activate = false;
		if (!world->multiplayer)
		{
			tutorial_activate = popup_triggers[i]->CheckPlayerDistance(player_1->player_go->transform.GetGlobalTranslation());
		}
		else
		{
			tutorial_activate = popup_triggers[i]->CheckPlayerDistance(player_1->player_go->transform.GetGlobalTranslation()) ||
				popup_triggers[i]->CheckPlayerDistance(player_2->player_go->transform.GetGlobalTranslation());
		}

		if (tutorial_activate && !popup_activated)
		{
			popup_activated = true;
			popup_triggers[i]->EnablePopUp();
		}
		else if(!tutorial_activate && popup_activated && popup_triggers[i]->IsPopUpEnabled())
		{
			popup_activated = false;
			popup_triggers[i]->DisablePopUp();
		}
	}
}

void CEventManager::BattleEventStatus()
{
	App->engine_log->Log("CheckEvent: %d %d",current_battle->CheckEvent(), current_battle->IsActivated());
	if(current_battle->CheckEvent() && current_battle->IsActivated())
	{
		App->engine_log->Log("EndEvent reached");
		EndEvent();
		battle_event_triggered = false;
		camera_controller->freeze = false;
		camera_controller->fixed_position = false;
		current_event = current_zone;
		if(current_event < battle_triggers.size())
		{
			current_event_beated = current_event;
			current_battle = battle_triggers[current_event_beated];
		}
		ComputeRootPosition();

	}
	else
	{
		current_battle->UpdateBattle(enemies_killed_on_wave);
	}

}

bool CEventManager::CheckLose()
{
	if (!world->multiplayer)
	{
		if (!player_1->player_controller->is_alive)
		{
			if (world->tries == 1)
			{
				ui_manager->DecreaseLives();
				return true;
			}
			else
			{
				world->tries--;
				ui_manager->DecreaseLives();
				checkpoint_manager->RespawnOnLastCheckPoint();
				return false;
			}
	
		}
	}
	//Multiplayer
	else
	{
		if (!player_1->player_controller->is_alive && !player_2->player_controller->is_alive)
		{
			if (world->tries == 1)
			{
				ui_manager->DecreaseLives();
				return true;
			}
			else
			{
				world->tries--;
				ui_manager->DecreaseLives();
				checkpoint_manager->RespawnOnLastCheckPoint();
				return false;
			}
		}
	}

	return false;
}

bool CEventManager::CheckWin()
{
	if(!world->multiplayer && !world->ThereIsBoss())
	{
		return level_ending->IsCollidingWith((static_cast<ComponentCollider*>(player_1->player_go->GetComponent(Component::ComponentType::COLLIDER))));
	}
	else if(world->multiplayer && !world->ThereIsBoss())
	{
		return level_ending->IsCollidingWith((static_cast<ComponentCollider*>(player_1->player_go->GetComponent(Component::ComponentType::COLLIDER)))) ||
			level_ending->IsCollidingWith((static_cast<ComponentCollider*>(player_2->player_go->GetComponent(Component::ComponentType::COLLIDER))));
	}
	else
	{
		return boss_killed;
	}
}


void CEventManager::MoveRoot()
{
	camera_shake->trauma = 0.6f;
	
	float current_lerp = current_time_moving_roots / total_time_moving_roots;
	float3 new_root_position = float3::Lerp(current_root_position, target_root_position, current_lerp);
	root_obstacles[current_zone]->transform.SetGlobalMatrixTranslation(new_root_position);

	if(current_lerp >= 1.f)
	{
		current_time_moving_roots = 0.0f;
		dust_roots_VFX->Stop();
		dust_roots_SFX->StopSelectedEvent();
		roots_moving = false;
		root_obstacles[current_zone]->SetEnabled(false);
	}

	current_time_moving_roots += App->time->delta_time;
}

void CEventManager::ComputeRootPosition()
{
	if(root_obstacles[current_zone] != nullptr)
	{
		current_root_position = root_obstacles[current_zone]->transform.GetGlobalTranslation();
		target_root_position = current_root_position - (float3::unitY * root_distance_multiplier);
		roots_moving = true;
	}
}

void CEventManager::DisablePopups()
{
	popup_activated = true;
	for (size_t i = 0; i < popup_triggers.size(); ++i)
	{
		popup_triggers[i]->DisablePopUp();
	}
}

void CEventManager::RestartEvents(unsigned int spawning_zone)
{
	battle_event_triggered = false;
	enemy_manager->KillAllTheEnemies();
	for (auto & battle : battle_triggers)
	{
		battle->RestartEvent();
	}

	enemies_killed_on_wave = 0;

	RestartObstacles();

	for (unsigned i = spawning_zone; i < zone_list.size(); ++i)
	{
		if (i > checkpoint_manager->last_checkpoint->zone_to_spawn)
		{
			zone_list[i]->dissolver->RestartDisolve();
			if (root_obstacles[i] != nullptr)
			{
				dust_roots_VFX->owner->transform.SetGlobalMatrixTranslation(root_obstacles[i]->children[0]->transform.GetGlobalTranslation());

			}
			if (zone_list[i]->zone_vfx != nullptr)
			{
				zone_list[i]->zone_vfx->StopVFX();
			}
			//Wwise event to restart sequence

		}

	}
	for (unsigned i = 0; i < current_zone; ++i)
	{
		if (zone_list[i]->zone_sfx != nullptr)
		{
			zone_list[i]->zone_sfx->PlayEventSFX();
		}
	}

}

void CEventManager::RestartObstacles()
{
	TeleportPoint* last_checkpoint = checkpoint_manager->GetClosestCheckpoint(current_zone);
	for (unsigned int i = 0; i< root_obstacles.size(); ++i)
	{
		if (root_obstacles[i] != nullptr)
		{
			if (i > last_checkpoint->zone_to_spawn)
			{
				root_obstacles[i]->SetEnabled(true);
				root_obstacles[i]->children[0]->GetComponent(Component::ComponentType::MESH_RENDERER)->active = false;
				root_obstacles[i]->transform.SetGlobalMatrixTranslation(float3::zero);
			}

		}

	}

}

void CEventManager::EndEvent()
{
	current_battle->EndEvent();

	++current_zone;
	if (root_obstacles[current_zone]!= nullptr)
	{
		dust_roots_VFX->owner->transform.SetGlobalMatrixTranslation(root_obstacles[current_zone]->children[0]->transform.GetGlobalTranslation());
		dust_roots_VFX->Play();
		dust_roots_SFX->PlayEvent(dust_roots_SFX->GetEventName());
	}
	if (zone_list[current_zone]->zone_vfx != nullptr) 
	{
		zone_list[current_zone]->zone_vfx->StopVFX();
	}
	//Roots
	if(current_zone < zone_list.size())
	{
		zone_list[current_zone]->dissolver->Dissolve();
		if (zone_list[current_zone]->zone_vfx != nullptr)
		{
			zone_list[current_zone]->zone_vfx->StartVFX();
		}
		if (zone_list[current_zone]->zone_sfx != nullptr)
		{
			zone_list[current_zone]->zone_sfx->PlayEventSFX();
		}
		camera_controller->UpdateCameraLines(zone_list[current_zone]->camera_rail, zone_list[current_zone]->camera_focus, zone_list[current_zone]->level_path);
	}
}

void CEventManager::GetZones()
{
	GameObject* zones_group = App->scene->GetGameObjectByName("Zones");

	for(unsigned i = 0; i < zones_group->children.size(); ++i)
	{
		Zone* new_zone = new Zone();

		GameObject* camera_group = zones_group->children[i]->GetChildrenWithName("Camera Rails");
		InitCameraZones(camera_group, new_zone);

		GameObject* meshes = zones_group->children[i]->GetChildrenWithName("Meshes");
		new_zone->dissolver = static_cast<Dissolver*>(meshes->GetComponentScript("Dissolver")->script);
		new_zone->dissolver->RestartDisolve();

		GameObject* vfx = zones_group->children[i]->GetChildrenWithName("Zone VFX");
		if (vfx) 
		{
			new_zone->zone_vfx = static_cast<EnvironmentVFX*>(vfx->GetComponentScript("EnvironmentVFX")->script);
		}

		GameObject* sfx = zones_group->children[i]->GetChildrenWithName("Zone SFX");
		if (sfx)
		{
			new_zone->zone_sfx = static_cast<EnvironmentSFX*>(sfx->GetComponentScript("EnvironmentSFX")->script);
		}
		zone_list.emplace_back(new_zone);

	
		GameObject* roots = zones_group->children[i]->GetChildrenWithName("Roots");
		root_obstacles.emplace_back(roots);

		
	}
}

void CEventManager::InitCameraZones(const GameObject* go, Zone* camera_zone)
{
	if(go)
	{
		camera_zone->camera_rail = static_cast<CameraLine*>(go->children[0]->GetComponentScript("CameraLine")->script);
		camera_zone->camera_focus = static_cast<CameraLine*>(go->children[1]->GetComponentScript("CameraLine")->script);
		camera_zone->level_path = static_cast<CameraLine*>(go->children[2]->GetComponentScript("CameraLine")->script);
	}
}

bool CEventManager::UpdateZone(int desired_zone)
{
	App->engine_log->Log("Zone list size = %d", zone_list.size());
	if (desired_zone < zone_list.size())
	{
		int previous_zone = current_zone;
		current_zone = desired_zone;
		if (previous_zone != current_zone)
		{
			UpdateCamera();
		}
		UpdateBattleZone();
		return true;
	}
	return false;
}

void CEventManager::UpdateCamera() const
{
	camera_controller->UpdateCameraLines(zone_list[current_zone]->camera_rail, zone_list[current_zone]->camera_focus, zone_list[current_zone]->level_path);
}

void CEventManager::UpdateBattleZone()
{
	if(world->ThereIsBoss())
	{
		return;
	}

	if (current_zone < battle_triggers.size())
	{
		current_event = current_event_beated = current_zone;
		current_battle = battle_triggers[current_event];
	}
	else
	{
		current_event = current_event_beated = battle_triggers.size() - 1;
		current_battle = battle_triggers[current_event];
	}
}
