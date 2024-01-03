#pragma once

#include "AllyInteractionsScript.h"
class WorldManager;
class ComponentCamera;
class AllyTrigger : public Script
{
public:
	AllyTrigger() = default;
	~AllyTrigger() = default;

	void Awake() override;
	void Start() override;
	void Update() override;


	void OnInspector(ImGuiContext * context);

	void Save(Config& config) const override;
	void Load(const Config& config) override;

	void InitPublicGameObjects() override;

private:
	void Complain(bool player_1_inside, bool player_2_inside);

	void SetPlayersPositions(bool player_1_inside, bool player_2_inside);

	AllyInteractionsScript* ally = nullptr;
	WorldManager* world = nullptr;

	GameObject* player_1 = nullptr;
	GameObject* player_2 = nullptr;

	GameObject* return_position_player1 = nullptr;
	GameObject* return_position_player2 = nullptr;

	//Settings
	float range = 6.0f;
	AllyInteractionsScript::State required_ally_state = AllyInteractionsScript::State::ANY;
	bool complains = false;
	bool dialogue = true;
	DialogueID dialogue_id = DialogueID::INTRO;

	//LightShield
	bool light_shield_tutorial = false;
	ComponentCamera* main_camera = nullptr;

};
extern "C" SCRIPT_API AllyTrigger* AllyTriggerDLL();

