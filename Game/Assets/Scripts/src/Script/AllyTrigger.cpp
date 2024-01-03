#include "AllyTrigger.h"

#include "Component/ComponentCamera.h"
#include "Main/Application.h"
#include "Main/GameObject.h"
#include "Module/ModuleScene.h"

#include "PlayerController.h"
#include "WorldManager.h"
#include "Log/EngineLog.h"

namespace
{
	const std::unordered_map<AllyInteractionsScript::State, std::string> enum_names{
	{AllyInteractionsScript::State::INTRO_TUTORIAL_SHOWED, "FirstTutorial"},
	{AllyInteractionsScript::State::LIGHTBALL_TRANSFORM, "LightBallTransform"},
	{AllyInteractionsScript::State::FINISH_LIGHTBALL_TRANSFORM, "FinishTransform"},
	{AllyInteractionsScript::State::ANY, "Any"},
	{AllyInteractionsScript::State::FINISH, "Finish"} };

	const std::unordered_map<DialogueID, std::string> dialogue_id_names{
		{DialogueID::ALLY_CORRUPTED_WATER, "Corrupted water"},
		{DialogueID::ALLY_LEVEL1_CELEBRATION, "Level 1 Celebration"},
		{DialogueID::INTRO, "Intro"} };
}
AllyTrigger* AllyTriggerDLL()
{
	AllyTrigger *instance = new AllyTrigger();
	return instance;
}

void AllyTrigger::Awake()
{
	world = static_cast<WorldManager*>(App->scene->GetGameObjectByName("World Manager")->GetComponentScript("WorldManager")->script);
	ally = static_cast<AllyInteractionsScript*>(App->scene->GetGameObjectByName("AllyController")->GetComponentScript("AllyInteractionsScript")->script);
	Component* mesh_renderer = owner->GetComponent(Component::ComponentType::MESH_RENDERER);
	if (mesh_renderer)
	{
		owner->GetComponent(Component::ComponentType::MESH_RENDERER)->Disable();
	}
	main_camera = static_cast<ComponentCamera*>(App->scene->GetGameObjectByName("Main Camera")->GetComponent(Component::ComponentType::CAMERA));

}

void AllyTrigger::Start()
{
	player_1 = world->GetPlayer1()->player_go;
	player_2 = world->GetPlayer2()->player_go;
}

void AllyTrigger::Update()
{
	if (ally->GetCurrentState() == required_ally_state || !ally->owner->IsEnabled())
	{
		return;
	}
	const float player_1_distance = player_1->transform.GetGlobalTranslation().Distance(owner->transform.GetGlobalTranslation());
	const float player_2_distance = player_2->transform.GetGlobalTranslation().Distance(owner->transform.GetGlobalTranslation());
	bool player_1_inside = player_1_distance <= range;
	bool player_2_inside = player_2_distance <= range;
	if (player_1_inside || player_2_inside)
	{
		if (complains)
		{
			Complain(player_1_inside, player_2_inside);
		}else
		if (dialogue && ally->ally_is_lightball)
		{
			ally->ActivateLightBallDialogue(dialogue_id);
			owner->SetEnabled(false);
		}
		if(light_shield_tutorial)
		{
			SetPlayersPositions(player_1_inside, player_2_inside);	

			ally->StartLightShieldTutorial(owner->GetChildrenWithName("camera_position"));
			owner->SetEnabled(false);
		}
	}

}

void AllyTrigger::Complain(bool player_1_inside, bool player_2_inside)
{
	if (ally->ally_is_lightball)
	{
		owner->SetEnabled(false);
		return;
	}
	App->engine_log->Log("Hey Listen!");
	SetPlayersPositions(player_1_inside, player_2_inside);
	ally->ComplainDialogue();
}

void AllyTrigger::SetPlayersPositions(bool player_1_inside, bool player_2_inside)
{
	if (player_1_inside || player_2_inside)
	{
		const ComponentTransform& transform = return_position_player1 ? return_position_player1->transform : ally->owner->transform;
		player_1->transform.SetGlobalMatrixTranslation(transform.GetGlobalTranslation());
		player_1->transform.SetGlobalMatrixRotation(transform.GetGlobalRotation());

		if (world->multiplayer)
		{
			const ComponentTransform& transform = return_position_player2 ? return_position_player2->transform : ally->owner->transform;
			player_2->transform.SetGlobalMatrixTranslation(transform.GetGlobalTranslation());
			player_2->transform.SetGlobalMatrixRotation(transform.GetGlobalRotation());
		}
	}
}

void AllyTrigger::OnInspector(ImGuiContext* context)
{
	//Necessary to be able to write with imgui
	ImGui::SetCurrentContext(context);
	if(ImGui::CollapsingHeader("Ally Trigger")) {
		ImGui::Text("Return Positions");
		ImGui::Separator();
		ShowDraggedObjects();
		ImGui::Separator();
		ImGui::Text("General");
		ImGui::Separator();
		ImGui::DragFloat("Range", &range, 0.001f, 0.0f, 10.0f);
		assert(enum_names.find(required_ally_state) != enum_names.end());
		if (ImGui::BeginCombo("Seletable States", enum_names.at(required_ally_state).c_str()))
		{
			for (auto& state : enum_names)
			{
				if (ImGui::Selectable(state.second.c_str()))
				{
					required_ally_state = state.first;
				}
			}
			ImGui::EndCombo();
		}
		ImGui::Text("Behaviour");
		ImGui::Separator();
		ImGui::Checkbox("Don't Ignore", &complains);
		ImGui::Checkbox("Talk", &dialogue);
		ImGui::Checkbox("Light shield tutorial", &light_shield_tutorial);
		if (dialogue &&  ImGui::BeginCombo("Select dialogue", dialogue_id_names.at(dialogue_id).c_str()))
		{
			for (auto& dialogue_name : dialogue_id_names)
			{
				if (ImGui::Selectable(dialogue_name.second.c_str()))
				{
					dialogue_id = dialogue_name.first;
				}
			}
			ImGui::EndCombo();
		}
	}
}
void AllyTrigger::Save(Config & config) const
{
	config.AddFloat(range, "Range");
	config.AddInt(static_cast<int>(required_ally_state), "State");
	config.AddInt(static_cast<int>(dialogue_id), "DialogueId");
	config.AddBool(complains, "Complains");
	config.AddBool(dialogue, "Dialogue");
	config.AddBool(light_shield_tutorial, "LightShieldTutorial");
	Script::Save(config);
}

void AllyTrigger::Load(const Config & config)
{
	range = config.GetFloat("Range", 1.0f);
	required_ally_state = static_cast<AllyInteractionsScript::State>(config.GetInt("State", static_cast<int>(AllyInteractionsScript::State::ANY)));
	dialogue_id = static_cast<DialogueID>(config.GetInt("DialogueId", static_cast<int>(DialogueID::INTRO)));

	complains = config.GetBool("Complains", true);
	dialogue = config.GetBool("Dialogue", false);
	light_shield_tutorial = config.GetBool("LightShieldTutorial", false);
	Script::Load(config);
}

void AllyTrigger::InitPublicGameObjects()
{
	public_gameobjects.push_back(&return_position_player1);
	variable_names.push_back(GET_VARIABLE_NAME(return_position_player1));

	public_gameobjects.push_back(&return_position_player2);
	variable_names.push_back(GET_VARIABLE_NAME(return_position_player2));

	for (unsigned int i = 0; i < public_gameobjects.size(); ++i)
	{
		name_gameobjects.push_back(is_object);
		go_uuids.push_back(0);
	}
}
