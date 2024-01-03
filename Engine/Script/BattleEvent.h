#ifndef  __BattleEvent_H__
#define  __BattleEvent_H__

#include "Script.h"

#include "Necroplant.h"

class CEnemyManager;

class BattleEvent : public Script
{
public:

	struct BattleEventWave//Senpai OMAR <3
	{
		int num_mushdooms;
		int num_biters;
		int num_necroplants;
	};

	BattleEvent();
	~BattleEvent() = default;

	void Awake() override;
	void Start() override;

	void InitSpawns();

	void ActivateEvent();
	bool CheckEvent() const;
	bool IsActivated() const;
	void UpdateBattle(int& enemies_already_killed);
	void EndEvent();

	void RestartEvent();
	void SpawnCurrentWave();

	void OnInspector(ImGuiContext*) override;

	void Save(Config& config) const override;
	void Load(const Config& config) override;

public:
	bool event_finished = false;

	std::vector<BattleEventWave> battle_events_waves;

	// Necroplants
	std::vector<Necroplant::Necrospot*> necro_spots;
	

private:
	CEnemyManager* enemy_manager = nullptr;
	bool event_activated = false;
	int current_wave = 0;

	std::vector<float3> spawns;
	std::vector<float3> biter_spawns;
	std::vector<float3> necro_spawns;

};

extern "C" SCRIPT_API BattleEvent* BattleEventDLL(); //This is how we are going to load the script

#endif