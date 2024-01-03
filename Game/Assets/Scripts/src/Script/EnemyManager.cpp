#include "EnemyManager.h"

#include "Component/ComponentBillboard.h"
#include "Component/ComponentCollider.h"
#include "Component/ComponentScript.h"
#include "Component/ComponentTransform.h"

#include "Log/EngineLog.h"
#include "Main/Application.h"
#include "Main/GameObject.h"
#include "Module/ModuleInput.h"
#include "Module/ModuleScene.h"
#include "Module/ModuleScriptManager.h"


#include "imgui.h"
#include <stdlib.h> 
#include <time.h>

#include "BattleEvent.h"
#include "Biter.h"
#include "EnemyController.h"
#include "EventManager.h"
#include "Mushdoom.h"
#include "Necroplant.h"
#include "MushdoomBall.h"
#include "WorldManager.h"

EnemyManager* EnemyManagerDLL()
{
	EnemyManager* instance = new EnemyManager();
	return instance;
}

EnemyManager::EnemyManager()
{
	
}

// Use this for initialization before Start()
void EnemyManager::Awake()
{
	event_manager = static_cast<EventManager*>(App->scene->GetGameObjectByName("EventManager")->GetComponentScript("EventManager")->script);

	GameObject* world_manager_go = App->scene->GetGameObjectByName("World Manager");
	ComponentScript* world_manager_component = world_manager_go->GetComponentScript("WorldManager");
	world_manager = static_cast<WorldManager*>(world_manager_component->script);

}

// Use this for initialization
void EnemyManager::Start()
{
	InitEnemies();
}

// Update is called once per frame
void EnemyManager::Update()
{
	if (world_manager->on_pause)
	{
		return;
	}

	if(!enemies_spawning_queue.empty() && current_number_of_enemies_alive < MAX_NUMBER_OF_ENEMIES)
	{
		for (auto it = enemies_spawning_queue.begin(); it != enemies_spawning_queue.end(); ++it)
		{
			float3 real_spawn_position(it->first.x, it->first.y, it->first.z);

			if (CheckSpawnAvailability(real_spawn_position) || it->second == EnemyType::NECROPLANT)
			{
				SpawnEnemy(it->second, real_spawn_position);
				enemies_spawning_queue.erase(it);
				break;
			}
		}
	}
}

void EnemyManager::AddEnemy(EnemyController* enemy)
{
	enemies.emplace_back(enemy);
}

void EnemyManager::KillEnemy(EnemyController* enemy)
{
	//This method is called once the enemy animation ended
	enemy->Kill();
	enemy->owner->transform.SetGlobalMatrixTranslation(graveyard_position);

	--current_number_of_enemies_alive;
	++total_enemies_killed;

	if (event_manager->battle_event_triggered && enemy->is_wave_enemy)
	{
		++event_manager->enemies_killed_on_wave;
	}
}

void EnemyManager::SpawnEnemy(EnemyType type, const float3& spawn_position, bool invincible)
{
	for (size_t i = 0; i < enemies.size(); ++i)
	{
		if (!enemies[i]->is_alive && enemies[i]->enemy_type == type)
		{
			EnemyController* enemy = nullptr;
			switch(type)
			{
				case EnemyType::MUSHDOOM:
					enemy = static_cast<Mushdoom*>(enemies[i]);
					break;

				case EnemyType::BITER:
					enemy = static_cast<Biter*>(enemies[i]);
					break;

				case EnemyType::NECROPLANT:
					enemy = static_cast<Necroplant*>(enemies[i]);
					break;
			}

			assert(enemy != nullptr);
			enemy->owner->transform.SetGlobalMatrixTranslation(spawn_position);

			if (enemy->collider)
			{
				enemy->collider->UpdateDimensions();
			}
			enemy->ResetEnemy();
			
			++current_number_of_enemies_alive;
			enemy->invincible = invincible;
			enemy->is_wave_enemy = true;
				
			break;
		}
	}
}

void EnemyManager::SpawnWave(std::vector<float3>& spawns, unsigned enemies_per_wave, EnemyType enemy_type)
{
	for (size_t i = 0; i < enemies_per_wave; ++i)
	{
		int random_spawn = rand() % spawns.size();
		enemies_spawning_queue.emplace_back(std::make_pair(spawns[random_spawn], enemy_type));
	}
}

void EnemyManager::InitEnemies()
{
	GameObject* mushdooms = App->scene->GetGameObjectByName("Mushdooms");
	if (mushdooms != nullptr)
	{
		for (int i = 0; i < mushdooms->children.size(); ++i)
		{
			Mushdoom* mushdoom = static_cast<Mushdoom*>(mushdooms->children[i]->GetComponentScript("Mushdoom")->script);
			if(!mushdoom->is_alive)
			{
				mushdoom->collider->active_physics = false;
				mushdoom->owner->SetEnabled(false);
				mushdoom->owner->transform.SetGlobalMatrixTranslation(graveyard_position);
			}
			else
			{
				++current_number_of_enemies_alive;
			}
			enemies.emplace_back(mushdoom);
		}
	}

	GameObject* biters = App->scene->GetGameObjectByName("Biters");
	if (biters != nullptr)
	{
		for (int i = 0; i < biters->children.size(); ++i)
		{
			Biter* biter = static_cast<Biter*>(biters->children[i]->GetComponentScript("Biter")->script);
			if (!biter->is_alive)
			{
				biter->collider->active_physics = false;
				biter->owner->SetEnabled(false);
				biter->owner->transform.SetGlobalMatrixTranslation(graveyard_position);
			}
			else
			{
				++current_number_of_enemies_alive;
			}
			enemies.emplace_back(biter);
		}
	}

	GameObject* necroplants = App->scene->GetGameObjectByName("Necroplants");
	if (necroplants != nullptr)
	{
		for (int i = 0; i < necroplants->children.size(); ++i)
		{
			Necroplant* necroplant = static_cast<Necroplant*>(necroplants->children[i]->GetComponentScript("Necroplant")->script);
			if (!necroplant->is_alive)
			{
				necroplant->owner->SetEnabled(false);
				necroplant->owner->transform.SetGlobalMatrixTranslation(graveyard_position);
			}
			else
			{
				++current_number_of_enemies_alive;
			}
			enemies.emplace_back(necroplant);
		}
	}
}

// Use this for showing variables on inspector
void EnemyManager::OnInspector(ImGuiContext* context)
{
	//Necessary to be able to write with imgui
	ImGui::SetCurrentContext(context);
	ShowDraggedObjects();

	ImGui::Text("enemies_spawning_queue size:  %d", enemies_spawning_queue.size());
	ImGui::Text("current_number_of_enemies_alive: %d", current_number_of_enemies_alive);
	ImGui::Text("total_enemies_killed: %d", total_enemies_killed);
	ImGui::Text("enemies: %d", enemies.size());
	ImGui::DragInt("MAX_DOUBLE_Z_RANGE:", &max_double_z_spawn);

	ImGui::Separator();
	for (const auto& enemy : enemies)
	{
		std::string enemy_type;
		switch (enemy->enemy_type)
		{
		case EnemyType::MUSHDOOM:
			enemy_type = "Mushdoom";
			break;
		case EnemyType::BITER:
			enemy_type = "Biter";
			break;
		case EnemyType::NECROPLANT:
			enemy_type = "Necroplant";
			break;
		default:
			break;
		}
		ImGui::Text("%s | Is Alive: %d", enemy_type.c_str(), enemy->is_alive);
	}

	int aux = current_ranged_mushdoom_attackers;
	ImGui::DragInt("CURRENT RANGED MUSHDOOMS", &aux);

	aux = current_melee_mushdoom_attackers;
	ImGui::DragInt("CURRENT MELEE MUSHDOOMS", &aux);
}

//Use this for linking JUST GO automatically 
void EnemyManager::InitPublicGameObjects()
{
	public_gameobjects.push_back(&mushdoom_go);
	variable_names.push_back(GET_VARIABLE_NAME(mushdoom_go));

	for (int i = 0; i < public_gameobjects.size(); ++i)
	{
		name_gameobjects.push_back(is_object);
		go_uuids.push_back(0);
	}
}

bool EnemyManager::ArePlayersInDanger() const
{
	for (size_t i = 0; i < current_number_of_enemies_alive; i++)
	{
		if ( enemies[i]->PlayerInSight())
		{
			return true;
		}
	}
	return false;
}

void EnemyManager::CancelMeleeAttack(EnemyController* enemy)
{
	--current_melee_mushdoom_attackers;
}

void EnemyManager::CancelRangedAttack(EnemyController* enemy)
{
	--current_ranged_mushdoom_attackers;
}

bool EnemyManager::CheckSpawnAvailability(float3& spawn_position)
{
	for (const auto& enemy : enemies)
	{
		if (!enemy->is_alive)
		{
			continue;
		}

		if (enemy->owner->transform.GetGlobalTranslation().Distance(spawn_position) <= 6.f)
		{
			return false;
		}
	}

	return true;
}

void EnemyManager::KillAllTheEnemies()
{
	for(auto& enemy : enemies)
	{
		if(enemy->is_alive)
		{
			enemy->invincible = false;
			KillEnemy(enemy);
		}
	}
}


void EnemyManager::NewRequestAttack(EnemyController* enemy)
{
	if (!IsEnemyAttacking(enemy))
	{
		Mushdoom* mushdoom = static_cast<Mushdoom*>(enemy);
		if (current_melee_mushdoom_attackers < simultaneous_attackers)
		{
			++current_melee_mushdoom_attackers;
			mushdoom->engage_player = true;
			
			if(mushdoom->ranged_attack)
			{
				mushdoom->ranged_attack = false;
				--current_ranged_mushdoom_attackers;
			}
		}
		else
		{
			++current_ranged_mushdoom_attackers;
			mushdoom->ranged_attack = true;

			if (mushdoom->engage_player)
			{
				mushdoom->engage_player = false;
				--current_melee_mushdoom_attackers;
			}
		}
	}
}

bool EnemyManager::IsEnemyAttacking(EnemyController* enemy)
{
	return IsEnemyMeleeAttacking(enemy) || IsEnemyRangedAttacking(enemy);
}

bool EnemyManager::IsEnemyMeleeAttacking(EnemyController* enemy)
{
	return static_cast<Mushdoom*>(enemy)->engage_player;
}

bool EnemyManager::IsEnemyRangedAttacking(EnemyController* enemy)
{
	return static_cast<Mushdoom*>(enemy)->ranged_attack;
}