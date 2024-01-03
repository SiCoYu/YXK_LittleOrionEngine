#include "AllyInteractionsScript.h"

#include "Component/ComponentBillboard.h"
#include "Component/ComponentCamera.h"
#include "Component/ComponentScript.h"
#include "Component/ComponentTransform.h"
#include "Component/ComponentText.h"

#include "Main/Application.h"
#include "Main/GameObject.h"
#include "Module/ModuleInput.h"
#include "Module/ModuleScene.h"
#include "Module/ModuleTime.h"

#include "CinematicCamera.h"
#include "DialogueManager.h"
#include "EnemyManager.h"
#include "EventManager.h"
#include "InputManager.h"
#include "LightBall.h"
#include "PlayerController.h"
#include "PlayerMovement.h"
#include "TutorialHelper.h"
#include "WorldManager.h"
#include "LightShield.h"

#include "Log/EngineLog.h"
#include "imgui.h"

namespace
{
	const float smooth_rotation = 0.01f;
	const float litghBall_movement_amplitude = 0.5f;
	const float lightBall_movement_speed = 0.001f;
	const float animation_time = 2.0f;
	const size_t interaction_range = 6;
}
using TCP = LightBall::TutorialCheckPoints::CheckPointStatus;
AllyInteractionsScript* AllyInteractionsScriptDLL()
{
	AllyInteractionsScript *instance = new AllyInteractionsScript();
	return instance;
}


void AllyInteractionsScript::Awake()
{

	world = static_cast<WorldManager*>(App->scene->GetGameObjectByName("World Manager")->GetComponentScript("WorldManager")->script);

	dialogue_manager = static_cast<DialogueManager*>(App->scene->GetGameObjectByName("Dialogue Manager")->GetComponentScript("DialogueManager")->script);

	camera = static_cast<ComponentCamera*>(App->scene->GetGameObjectByName("AllyCamera")->GetComponent(Component::ComponentType::CAMERA));
	main_camera = static_cast<ComponentCamera*>(App->scene->GetGameObjectByName("Main Camera")->GetComponent(Component::ComponentType::CAMERA));

	input_manager = static_cast<InputManager*>(App->scene->GetGameObjectByName("Input Manager")->GetComponentScript("InputManager")->script);

	controller_helper_ui = App->scene->GetGameObjectByName("ControllerHelper");
	keyboard_helper_ui = App->scene->GetGameObjectByName("KeyBoardHelper");
	arrow_indicator_helper = static_cast<ComponentBillboard*>(owner->parent->GetChildrenWithName("ArrowIndicatorVFX")->GetComponent(Component::ComponentType::BILLBOARD));

	event_manager = static_cast<CEventManager*>(App->scene->GetGameObjectByName("EventManager")->GetComponentScript("EventManager")->script);
	enemy_manager = static_cast<CEnemyManager*>(App->scene->GetGameObjectByName("EnemyManager")->GetComponentScript("EnemyManager")->script);
	light_ball = static_cast<LightBall*>(App->scene->GetGameObjectByName("LightBall")->GetComponentScript("LightBall")->script);
	light_ball->ally_has_light_ball = !ally_is_lightball;
	GameObject* ally_positions = App->scene->GetGameObjectByName("Ally Positions");
	if (ally_positions)
	{
		positions = ally_positions->children;
	}
	current_position = -1;

	cinematic_camera = static_cast<CinematicCamera*>(App->scene->GetGameObjectByName("Cinematic Camera Holder")->GetComponentScript("CinematicCamera")->script);

	//lightball
	for (const auto go : App->scene->GetGameObjectByName("ligthball_ally")->children)
	{
		if (go->name == "ally_lightball_interior")
		{
			interior_texture_LB = static_cast<ComponentBillboard*>(go->GetComponent(Component::ComponentType::BILLBOARD));
		}
		if (go->name == "ally_lightball_exterior")
		{
			exterior_texture_LB = static_cast<ComponentBillboard*>(go->GetComponent(Component::ComponentType::BILLBOARD));
		}
		if (go->name == "ally_lightball_small")
		{
			small_texture_LB = static_cast<ComponentBillboard*>(go->GetComponent(Component::ComponentType::BILLBOARD));
		}
	}

	tutorial_helper = std::make_unique<TutorialHelper>();
	tutorial_helper->InitDependencies(*App->scene);
}

void AllyInteractionsScript::Start()
{
	player_1 = world->GetPlayer1();
	player_2 = world->GetPlayer2();
	player_1_mesh = App->scene->GetGameObjectByName("Player1_Mesh")->GetComponent(Component::ComponentType::MESH_RENDERER);
	player_2_mesh = App->scene->GetGameObjectByName("Player2_Mesh")->GetComponent(Component::ComponentType::MESH_RENDERER);

	interior_texture_LB->Play();
	exterior_texture_LB->Play();
	small_texture_LB->Play();

	if (!ally_is_lightball)
	{
		cinematic_camera->PlayCinematic(0);
	}

}

void AllyInteractionsScript::Update()
{
	CheckShieldTutorial();
	if (cinematic_camera->IsPlayingCinematic())
	{
		return;
	}
	if (ally_is_lightball) //In lightball mode trigger conversation with ally triggers
	{
		AllyLightBallUpdate();
	}
	else
	{
		firs_popup_showed |= event_manager->popup_activated;
		AllyHumanoidUpdate();
	}
}

void AllyInteractionsScript::AllyLightBallUpdate()
{
	if (dialogue_manager->IsOnDialogue())
	{
		light_ball->SetActiveVFX(true);

		const float adjusted_time = App->time->delta_time*0.001f;
		time_passed += adjusted_time;

		float3 position = lightBall_translation_reference;

		float3 focus_vector = light_ball->owner->transform.GetTranslation() - light_ball->current_player->transform.GetTranslation();
		Quat rotation = Quat::RotateY(adjusted_time);

		focus_vector = rotation * focus_vector;
		position = focus_vector + light_ball->current_player->transform.GetTranslation();

		position.y = sin(App->time->time*lightBall_movement_speed) * litghBall_movement_amplitude + lightBall_translation_reference.y;
		light_ball->owner->transform.SetTranslation(position);
	}
}

void AllyInteractionsScript::AllyHumanoidUpdate()
{
	float player_distance = player_1->player_go->transform.GetGlobalTranslation().Distance(owner->transform.GetGlobalTranslation());
	bool player_is_inside_interaction_range = player_distance <= interaction_range;
	bool ready_to_talk = !dialogue_manager->IsOnDialogue() && player_is_inside_interaction_range && can_start_next_dialogue;

	if (player_is_inside_interaction_range)
	{
		RotateTowardsPlayer();
	}
	if (enemy_manager->ArePlayersInDanger() || State::FINISH == current_state)
	{
		return;
	}
	if (!dialogue_manager->IsOnDialogue() && player_is_inside_interaction_range && can_start_next_dialogue)
	{
		player_1->player_controller->player_movement->BlockJump(true);
		bool player_input = input_manager->GetGameInputUp("Jump", static_cast<PlayerID>(player_1->player_controller->player));
		if (player_input)
		{
			EnableAllyOnConversation(true);
			if (event_manager->popup_activated)
			{
				event_manager->DisablePopups();
			}
			ActivateDialogue();
			//App->engine_log->Log("Talking with Ally");
		}
	}
	else
	{
		player_1->player_controller->player_movement->BlockJump(false);
	}
	bool ui_helper_active = player_is_inside_interaction_range && !dialogue_manager->IsOnDialogue();
	ActivateUIHelpers(ui_helper_active || complaining);
	arrow_indicator_helper->active = ui_helper_active;
	if (ui_helper_active && !arrow_indicator_helper->IsPlaying()) {
		arrow_indicator_helper->Play();
	}
	can_start_next_dialogue = true;
}

void AllyInteractionsScript::ActivateDialogue()
{

	if ((current_state == State::INTRO || current_state == State::INTRO_REPEAT) && firs_popup_showed)
	{
		current_state = State::INTRO_TUTORIAL_SHOWED;
		dialogue_manager->StartDialogue(DialogueID::ALLY_INTRO_TUTORIAL_SHOWED, [this](DialogueFeeling feeling) { PlayAnimation(feeling); });
	}else if (current_state == State::INTRO)
	{
		dialogue_manager->StartDialogue(DialogueID::ALLY_INTRO, [this](DialogueFeeling feeling) { PlayAnimation(feeling); });
		current_state = State::INTRO_REPEAT;
	}
	else if(current_state == State::INTRO_REPEAT)
	{
		dialogue_manager->StartDialogue(DialogueID::ALLY_INTRO_REPEAT, [this](DialogueFeeling feeling) { PlayAnimation(feeling); });
	}
	else if (current_state == State::INTRO_TUTORIAL_SHOWED)
	{
		current_state = State::LIGHTBALL_TRANSFORM;
		dialogue_manager->StartDialogue(DialogueID::LIGHTBALL, [this](DialogueFeeling feeling) { PlayAnimation(feeling); });
	}
}


void AllyInteractionsScript::OnInspector(ImGuiContext * context)
{
	ImGui::SetCurrentContext(context);
	ImGui::Checkbox("Can start next dialogue", &can_start_next_dialogue);
	ImGui::Checkbox("Ally Is Lightball", &ally_is_lightball);
	ImGui::InputInt("Current position index:", &current_position);
	ShowDraggedObjects();
}

void AllyInteractionsScript::InitPublicGameObjects()
{
	for (int i = 0; i < public_gameobjects.size(); ++i)
	{
		name_gameobjects.push_back(is_object);
		go_uuids.push_back(0);
	}
}

void AllyInteractionsScript::Save(Config & config) const
{
	config.AddBool(ally_is_lightball, "IsLightball");
	Script::Save(config);
}

void AllyInteractionsScript::Load(const Config & config)
{
	ally_is_lightball = config.GetBool("IsLightball", false);
	Script::Load(config);
}

void AllyInteractionsScript::ComplainDialogue()
{
	complaining = true;
	EnableAllyOnConversation(true);
	if (event_manager->popup_activated)
	{
		event_manager->DisablePopups();
	}
	if(enemy_manager->ArePlayersInDanger())
	{
		dialogue_manager->StartDialogue(DialogueID::DO_NOT_IGNORE2, [this](DialogueFeeling feeling) { PlayAnimation(feeling); });
	}
	else
	{
		dialogue_manager->StartDialogue(DialogueID::DO_NOT_IGNORE, [this](DialogueFeeling feeling) { PlayAnimation(feeling); });
	}
}


void AllyInteractionsScript::ActivateUIHelpers(bool activate)
{
	if (keyboard_helper_ui && controller_helper_ui)
	{
		if (activate)
		{
			if (input_manager->total_game_controllers <= 0)
			{
				controller_helper_ui->SetEnabled(false);
				keyboard_helper_ui->SetEnabled(true);
			}
			else
			{
				controller_helper_ui->SetEnabled(true);
				keyboard_helper_ui->SetEnabled(false);
			}
		}else
		{
			keyboard_helper_ui->SetEnabled(false);
			controller_helper_ui->SetEnabled(false);
		}
	}
}

void AllyInteractionsScript::PlayAnimation(DialogueFeeling feeling)
{
	if (static_cast<int>(feeling) == -1)
	{
		can_start_next_dialogue = false;
		complaining = false;
		event_manager->popup_activated = false;
		EnableAllyOnConversation(false);
		player_1->player_controller->player_movement->BlockJump(false);
		if (current_state == State::LIGHTBALL_TRANSFORM)
		{
			TransformIntoLightBall();
		}
		if (current_state == State::INTRO_TUTORIAL_SHOWED)
		{
			GoToNextPosition();
		}
		if (current_state == State::INTRO_REPEAT )
		{
			cinematic_camera->PlayCinematic(1);
		}
	}

}
void AllyInteractionsScript::TransformIntoLightBall()
{
	if (!ally_is_lightball)
	{
		light_ball->ThrowLightBallToMainPlayer();
		ally_is_lightball = true;
		current_state = State::FINISH_LIGHTBALL_TRANSFORM;
		owner->parent->GetChildrenWithName("tree_character")->SetEnabled(false);
		ActivateUIHelpers(false);
	}
}
void AllyInteractionsScript::LightBallModeCallback(DialogueFeeling feeling)
{
	if (static_cast<int>(feeling) == -1)
	{
		light_ball->ActiveReturn();
	}

	if (feeling == DialogueFeeling::IDLE)
	{
		current_lightball_speed = 0.005f;
		time_passed = 0.0f;
	}
}

//Copied from mushdoom
void AllyInteractionsScript::RotateTowardsPlayer() const
{
	float3 desired_direction = player_1->player_go->transform.GetGlobalTranslation() - owner->parent->transform.GetGlobalTranslation();
	desired_direction.y = 0;
	Quat desired_rotation = Quat::LookAt(float3::unitZ, desired_direction.Normalized(), float3::unitY, float3::unitY);
	Quat new_rotation = Quat::Slerp(owner->parent->transform.GetGlobalRotation(), desired_rotation, App->time->delta_time * smooth_rotation);
	owner->parent->transform.SetGlobalMatrixRotation(new_rotation);
}

void AllyInteractionsScript::EnableAllyOnConversation(bool enable)
{
	EnableCamera(enable);
	ChangePlayersMeshVisibility(!enable);
}

void AllyInteractionsScript::EnableCamera(bool enable)
{
	if (!enable)
	{
		main_camera->depth = 5.f;
		camera->depth = 0.f;
		camera->active = false;
	}
	else
	{
		camera->active = true;
		camera->depth = 5.f;
		main_camera->depth = 0.f;
	}
}

void AllyInteractionsScript::ChangePlayersMeshVisibility(bool enable)
{
	if (player_1_mesh)
	{
		if(enable)
		{
			player_1_mesh->Enable();
		}
		else
		{
			player_1_mesh->Disable();
		}

	}
	if (player_2_mesh)
	{
		if (enable)
		{
			player_2_mesh->Enable();
		}
		else
		{
			player_2_mesh->Disable();
		}
	}
}

void AllyInteractionsScript::GoToNextPosition()
{
	if (positions.empty())
	{
		return;
	}
	current_position = max(min(++current_position, 0), positions.size() - 1);
	//App->engine_log->Log("Go to next position %d",current_position);
	GameObject* current_go = positions[current_position];
	owner->parent->transform.SetGlobalMatrixTranslation(current_go->transform.GetGlobalTranslation());
	owner->parent->transform.SetGlobalMatrixRotation(current_go->transform.GetGlobalRotation());
	current_go->SetEnabled(false);
}


void AllyInteractionsScript::ActivateLightBallDialogue(DialogueID dialogue)
{
	if (!dialogue_manager->IsOnDialogue() )
	{
		lightBall_translation_reference = light_ball->owner->transform.GetTranslation();
		dialogue_manager->StartDialogue(dialogue, [this](DialogueFeeling feeling) { LightBallModeCallback(feeling); });
	}
}

AllyInteractionsScript::State AllyInteractionsScript::GetCurrentState() const
{
	return current_state;
}

void AllyInteractionsScript::StartLightShieldTutorial(GameObject * camera_position)
{
	assert(camera_position);
	tutorial_helper->ResetTutorial(light_ball->shield_tutorial);
	shield_tutorial_started = true;
	camera->owner->transform.SetGlobalModelMatrix(camera_position->transform.GetGlobalModelMatrix());
	EnableCamera(true);
	light_ball->owner->transform.SetGlobalMatrixTranslation(light_ball->current_player_hand->transform.GetGlobalTranslation());
	lightBall_translation_reference = light_ball->owner->transform.GetTranslation();
	if (!player_1->player_controller->is_alive)
	{
		player_1->player_controller->Revive();
	}
	if (world->multiplayer && !player_2->player_controller->is_alive)
	{
		player_2->player_controller->Revive();
	}
	dialogue_manager->StartDialogue(DialogueID::LIGHTSHIELD_TUTORIAL_START, [&](DialogueFeeling feeling) { if (feeling == DialogueFeeling::FINISH) {
		light_ball->on_tutorial = true;
		player_1->player_controller->light_shield->on_tutorial = true;
		player_1->player_controller->light_shield->FillDurability();
		if (world->multiplayer)
		{
			player_2->player_controller->light_shield->on_tutorial = true;
			player_1->player_controller->light_shield->FillDurability();
		}
		world->on_pause = true;
	}});
}

void AllyInteractionsScript::CheckShieldTutorial()
{
	player_1->player_controller->CheckLightShieldStatus();
	if(world->multiplayer)
	{ 
		player_2->player_controller->CheckLightShieldStatus();
	}
	if (dialogue_manager->IsOnDialogue())
	{
		light_ball->on_tutorial = false;
		return;
	}
	if (shield_tutorial_started && !light_ball->on_tutorial)
	{
		lightBall_translation_reference = light_ball->owner->transform.GetTranslation();
		bool dialogue = tutorial_helper->CheckShieldTutorialDialogue(light_ball->shield_tutorial, [this](DialogueFeeling feeling) {
			if (feeling == DialogueFeeling::FINISH)
			{
				EnableCamera(false);
				shield_tutorial_started = false;
				player_1->player_controller->light_shield->on_tutorial = false;
				player_1->player_controller->light_shield->FillDurability();
				if (world->multiplayer)
				{
					player_2->player_controller->light_shield->on_tutorial = false;
					player_1->player_controller->light_shield->FillDurability();
				}
			}}, [this](DialogueFeeling feeling)
			{
				light_ball->on_tutorial = true; 
				world->on_pause = true; 
				player_1->player_controller->light_shield->FillDurability();
				if (world->multiplayer)
				{
					player_1->player_controller->light_shield->FillDurability();
				}
			});
		if (!dialogue)
		{
			light_ball->on_tutorial = true;
		}
	}
	else if (shield_tutorial_started)
	{
		tutorial_helper->UpdateUI(light_ball->shield_tutorial);
	}
}


