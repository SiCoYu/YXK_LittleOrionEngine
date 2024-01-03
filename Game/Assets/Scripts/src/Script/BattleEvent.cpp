#include "BattleEvent.h"

#include "Component/ComponentCollider.h"
#include "Component/ComponentScript.h"
#include "Component/ComponentTransform.h"

#include "Log/EngineLog.h"
#include "Main/Application.h"
#include "Main/GameObject.h"
#include "Module/ModuleInput.h"
#include "Module/ModuleScene.h"

#include <imgui.h>

#include "EnemyManager.h"
#include "EnemyController.h"

BattleEvent* BattleEventDLL()
{
	BattleEvent* instance = new BattleEvent();
	return instance;
}


BattleEvent::BattleEvent()
{
}

// Use this for initialization before Start()
void BattleEvent::Awake()
{
	GameObject* enemy_manager_go = App->scene->GetGameObjectByName("EnemyManager");
	enemy_manager = static_cast<EnemyManager*>(enemy_manager_go->GetComponentScript("EnemyManager")->script);

	InitSpawns();
}

// Use this for initialization
void BattleEvent::Start()
{
	event_finished = false;
}

void BattleEvent::InitSpawns()
{
	GameObject* spawns_go = owner->GetChildrenWithName("Spawns");
	for (size_t i = 0; i < spawns_go->children.size(); ++i)
	{
		spawns.emplace_back(spawns_go->children[i]->transform.GetGlobalTranslation());
	}
	
	GameObject* special_biter_go = owner->GetChildrenWithName("Biter Spawns");
	if (special_biter_go != nullptr)
	{
		for (size_t i = 0; i < special_biter_go->children.size(); ++i)
		{
			biter_spawns.emplace_back(special_biter_go->children[i]->transform.GetGlobalTranslation());
		}
	}

	GameObject* necro_spawns_go = owner->GetChildrenWithName("Necro Spawns");
	if (necro_spawns_go != nullptr)
	{
		for (size_t i = 0; i < necro_spawns_go->children.size(); ++i)
		{
			necro_spawns.emplace_back(necro_spawns_go->children[i]->transform.GetGlobalTranslation());

			Necroplant::Necrospot* necrospot = new Necroplant::Necrospot();
			necrospot->position = necro_spawns_go->children[i]->transform.GetGlobalTranslation();
			necrospot->ocupied = false;
			necro_spots.emplace_back(necrospot);
		}
	}
}

void BattleEvent::ActivateEvent()
{
	event_activated = true;
	current_wave = 0;
	SpawnCurrentWave();
}

bool BattleEvent::CheckEvent() const
{
	return event_finished;
}

bool BattleEvent::IsActivated() const
{
	return event_activated;
}

void BattleEvent::UpdateBattle(int& enemies_killed_on_wave)
{
	BattleEventWave current_battle_event_wave = battle_events_waves[current_wave];
	if (enemies_killed_on_wave >= current_battle_event_wave.num_mushdooms + current_battle_event_wave.num_biters + current_battle_event_wave.num_necroplants)
	{
		enemies_killed_on_wave = 0;
		++current_wave;
		if (current_wave < battle_events_waves.size())
		{
			SpawnCurrentWave();
		}
		else
		{
			event_finished = true;
		}
	}
}

void BattleEvent::EndEvent()
{
	event_activated = false;
	//If something is going to be played, sound or anything common
}

void BattleEvent::RestartEvent()
{
	event_activated = false;
	event_finished = false;
	current_wave = 0;
}

void BattleEvent::SpawnCurrentWave()
{
	enemy_manager->SpawnWave(spawns, battle_events_waves[current_wave].num_mushdooms, EnemyType::MUSHDOOM);
	enemy_manager->SpawnWave(biter_spawns, battle_events_waves[current_wave].num_biters, EnemyType::BITER);
	enemy_manager->SpawnWave(necro_spawns, battle_events_waves[current_wave].num_necroplants, EnemyType::NECROPLANT);
}

// Use this for showing variables on inspector
void BattleEvent::OnInspector(ImGuiContext* context)
{
	//Necessary to be able to write with imgui
	ImGui::SetCurrentContext(context);
	ShowDraggedObjects();

	for (size_t i = 0; i < battle_events_waves.size(); ++i)
	{
		ImGui::PushID(i);
		ImGui::TextColored(ImVec4(1.f, 0.f, 1.f, 1.f), "Wave %u", i);
		ImGui::SameLine();
		if (ImGui::Button("-"))
		{
			battle_events_waves.erase(battle_events_waves.begin() + i);
			break;
		}

		ImGui::DragInt("Number of Mushdooms", &battle_events_waves[i].num_mushdooms, 0.1f, 0, 100);
		ImGui::DragInt("Number of Biters", &battle_events_waves[i].num_biters, 0.1f, 0, 100);
		ImGui::DragInt("Number of Necroplants", &battle_events_waves[i].num_necroplants, 0.1f, 0, 100);
		ImGui::PopID();
	}

	ImGui::Separator();
	ImGui::Spacing();

	if (ImGui::Button("Add Wave"))
	{
		battle_events_waves.push_back(BattleEventWave());
	}
	
}

//Use this for linking GO AND VARIABLES automatically if you need to save variables 
 void BattleEvent::Save(Config& config) const
 {
	 size_t num_waves = battle_events_waves.size();
	 config.AddUInt(num_waves, "Number of Waves");

	 std::vector<int> mushdooms_per_wave(num_waves);
	 std::vector<int> biters_per_wave(num_waves);
	 std::vector<int> necroplants_per_wave(num_waves);
	 for (size_t i = 0; i < num_waves; ++i)
	 {
		 mushdooms_per_wave[i] = battle_events_waves[i].num_mushdooms;
		 biters_per_wave[i] = battle_events_waves[i].num_biters;
		 necroplants_per_wave[i] = battle_events_waves[i].num_necroplants;
	 }

	 config.AddVector(mushdooms_per_wave, "Mushdooms per wave");
	 config.AddVector(biters_per_wave, "Biters per wave");
	 config.AddVector(necroplants_per_wave, "Necroplants per wave");

 	Script::Save(config);
 }

 //Use this for linking GO AND VARIABLES automatically
 void BattleEvent::Load(const Config& config)
 {
	 size_t num_waves = config.GetUInt("Number of Waves", 0);
	 battle_events_waves = std::vector<BattleEventWave>(num_waves);

	 if (num_waves != 0)
	 {
		 std::vector<int> mushdooms_per_wave(num_waves);
		 std::vector<int> biters_per_wave(num_waves);
		 std::vector<int> necroplants_per_wave(num_waves);
		 config.GetVector("Mushdooms per wave", mushdooms_per_wave, std::vector<int>());
		 config.GetVector("Biters per wave", biters_per_wave, std::vector<int>());
		 config.GetVector("Necroplants per wave", necroplants_per_wave, std::vector<int>());

		 for (size_t i = 0; i < num_waves; ++i)
		 {
			 battle_events_waves[i].num_mushdooms = mushdooms_per_wave[i];
			 battle_events_waves[i].num_biters = biters_per_wave[i];
			 battle_events_waves[i].num_necroplants = necroplants_per_wave[i];
		 }
	 }
	
 	Script::Load(config);
 }