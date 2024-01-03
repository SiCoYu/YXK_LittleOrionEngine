#include "DialogueManager.h"

#include "Component/ComponentAnimation.h"
#include "Component/ComponentCanvas.h"
#include "Component/ComponentCanvasRenderer.h"
#include "Component/ComponentImage.h"
#include "Component/ComponentScript.h"
#include "Component/ComponentText.h"
#include "Component/ComponentTransform.h"

#include "Log/EngineLog.h"
#include "Main/Application.h"
#include "Main/GameObject.h"
#include "Module/ModuleCamera.h"
#include "Module/ModuleInput.h"
#include "Module/ModuleScene.h"
#include "Module/ModuleTime.h"
#include "Module/ModuleUI.h"

#include <imgui.h>

#include "Dialogues.h"
#include "InputManager.h"
#include "PlayerController.h"
#include "PlayerMovement.h"
#include "WorldManager.h"

DialogueManager* DialogueManagerDLL()
{
	DialogueManager* instance = new DialogueManager();
	return instance;
}

DialogueManager::~DialogueManager()
{
	for (auto& dialogue : dialogue_list)
	{
		delete dialogue;
	}
}

// Use this for initialization before Start()
void DialogueManager::Awake()
{
	dialogue_text = static_cast<ComponentText*>(owner->children[0]->GetComponent(Component::ComponentType::UI_TEXT));

	GameObject* world_go = App->scene->GetGameObjectByName("World Manager");
	world_manager = static_cast<WorldManager*>(world_go->GetComponentScript("WorldManager")->script);

	GameObject* input_go = App->scene->GetGameObjectByName("Input Manager");
	input_manager = static_cast<InputManager*>(input_go->GetComponentScript("InputManager")->script);
	
	GetDialogueList();
	InitPopupList();
}

// Use this for initialization
void DialogueManager::Start()
{
	player_1 = world_manager->GetPlayer1();
	player_2 = world_manager->GetPlayer2();
	InitDialogueList();
	static_cast<ComponentText*>(owner->children[0]->GetComponent(Component::ComponentType::CANVAS_RENDERER))->Disable();
}

// Update is called once per frame
void DialogueManager::Update()
{
	if (on_dialogue)
	{
		EnsurePlayersTouchingTheGround();
		bool next = false;
		if (world_manager->multiplayer)
		{
			next = (input_manager->GetGameInputUp("Jump", static_cast<PlayerID>(player_1->player_controller->player)) ||
				input_manager->GetGameInputUp("Jump", static_cast<PlayerID>(player_2->player_controller->player)));
		}
		else
		{
			next = input_manager->GetGameInputUp("Jump", static_cast<PlayerID>(player_1->player_controller->player));
		}
		if (next && end_of_sentence)
		{
			DisplayNextSentence();
		}
		else if (next && !end_of_sentence)
		{
			dialogue_text->text = sentence_to_show.text;
			end_of_sentence = true;
		}
		else
		{
			if (!end_of_sentence)
			{
				spawn_time -= App->time->delta_time;
				if (spawn_time < 0.f)
				{
					UpdateText();
					spawn_time = cooldown_spawn;
				}

			}
		}
	}
}

// Use this for showing variables on inspector
void DialogueManager::OnInspector(ImGuiContext* context)
{
	//Necessary to be able to write with imgui
	ImGui::SetCurrentContext(context);
	ImGui::Checkbox("On Dialogue", &on_dialogue);
	ImGui::Checkbox("Showing popup", &showing_popup);
	ImGui::DragFloat("Cooldown Spawn Letters", &cooldown_spawn);
}

bool DialogueManager::IsDialogue(DialogueID dialogue_id)
{
	bool found = false;

	auto it = std::find_if(dialogue_list.begin(), dialogue_list.end(), [&dialogue_id](Dialogue* node)
	{
		return node->id == dialogue_id;
	});
	if (it != dialogue_list.end())
	{
		current_dialogue = *it;
		found = true;
	}

	return found;
}

void DialogueManager::StartDialogue(DialogueID dialogue_id,const NextDialogueCallback& callback)
{
	if (!IsDialogue(dialogue_id))
	{
		return;
	}
	if (showing_popup)
	{
		DisablePopup(selected_pop_up);
	}
	on_dialogue = true;
	static_cast<ComponentText*>(owner->children[0]->GetComponent(Component::ComponentType::CANVAS_RENDERER))->Enable();
	while(!sentences.empty())
	{
		sentences.pop();
	}
	next_dialogue_callback = callback;
	for(auto& sentence : current_dialogue->sentences)
	{
		sentences.push(sentence);
	}
	DisplayNextSentence();
}

bool DialogueManager::IsOnDialogue() const
{
	return on_dialogue;
}

void DialogueManager::DisplayNextSentence()
{
	if (sentences.empty())
	{
		static_cast<ComponentText*>(owner->children[0]->GetComponent(Component::ComponentType::CANVAS_RENDERER))->Disable();
		on_dialogue = false;
		world_manager->on_pause = false;
		//App->engine_log->Log("Finish Dialogue");
		if (next_dialogue_callback)
		{
			next_dialogue_callback(DialogueFeeling::FINISH);
		}
		player_1->player_controller->can_move = true;
		if (world_manager->multiplayer)
		{
			player_2->player_controller->can_move = true;
		}
		return;
	}
	//App->engine_log->Log("Start next Dialogue");
	sentence_to_show = sentences.front();
	sentences.pop();
	App->engine_log->Log("Start next Dialogue %d", sentences.size());
	end_of_sentence = false;
	string_position = 0;
	dialogue_text->text.clear();
	//App->engine_log->Log(sentence_to_show.text.c_str());
	if (next_dialogue_callback)
	{
		next_dialogue_callback(sentence_to_show.feeling);
	}
}

void DialogueManager::UpdateText()
{
	if(string_position < sentence_to_show.text.size())
	{
		char next_character = sentence_to_show.text.at(string_position);
		dialogue_text->text.append(1, next_character);
		++string_position;
	}
	else
	{
		end_of_sentence = true;
	}
}

void DialogueManager::GetDialogueList()
{
	Intro* intro = new Intro();
	dialogue_list.emplace_back(intro);

	AllyIntro* ally_intro = new AllyIntro();
	dialogue_list.emplace_back(ally_intro);

	AllyIntroRepeat* ally_intro_repeat = new AllyIntroRepeat();
	dialogue_list.emplace_back(ally_intro_repeat);

	AllyPopupShowed* ally_popup_showed= new AllyPopupShowed();
	dialogue_list.emplace_back(ally_popup_showed);

	AllyLightBallTransform* ally_lightball_tutorial = new AllyLightBallTransform();
	dialogue_list.emplace_back(ally_lightball_tutorial);

	AllyDoNotIgnoreMe* ally_do_not_ignore_me = new AllyDoNotIgnoreMe();
	dialogue_list.emplace_back(ally_do_not_ignore_me);
	AllyDoNotIgnoreMe2* ally_do_not_ignore_me2 = new AllyDoNotIgnoreMe2();
	dialogue_list.emplace_back(ally_do_not_ignore_me2);

	AllyCorruptedWater* ally_do_corrupted_water = new AllyCorruptedWater();
	dialogue_list.emplace_back(ally_do_corrupted_water);
	AllyLevel1Celebration* ally_level1_celebration = new AllyLevel1Celebration();
	dialogue_list.emplace_back(ally_level1_celebration);

	AllyLightShieldTutorial* ally_lightshield_tutorial_start = new AllyLightShieldTutorial();
	dialogue_list.emplace_back(ally_lightshield_tutorial_start);

	AllyLightShieldTutorialEnd* ally_lightshield_tutorial_end = new AllyLightShieldTutorialEnd();
	dialogue_list.emplace_back(ally_lightshield_tutorial_end);

	AllyLightShieldTutorialRepeat* ally_lightshield_tutorial_repeat = new AllyLightShieldTutorialRepeat();
	dialogue_list.emplace_back(ally_lightshield_tutorial_repeat);
}

void DialogueManager::InitDialogueList()
{
	for (auto& dialogue : dialogue_list)
	{
		dialogue->Init();
	}
}

void DialogueManager::EnsurePlayersTouchingTheGround() const
{

	App->engine_log->Log("Ground");
	player_1->player_controller->player_movement->MoveIntoDirection(owner->transform.GetUpVector(), -10.f);
	player_1->player_controller->can_move = false;
	player_1->player_controller->Update();
	player_1->player_controller->InterruptDash();
	if (world_manager->multiplayer)
	{
		player_2->player_controller->player_movement->MoveIntoDirection(owner->transform.GetUpVector(), -10.f);
		player_2->player_controller->can_move = false;
		player_2->player_controller->Update();
		player_2->player_controller->InterruptDash();
	}
	world_manager->on_pause = true;
}

void DialogueManager::InitPopupList()
{
	GameObject* popups_go = App->scene->GetGameObjectByName("Popups UI");
	for (size_t i = 0; i < popups_go->children.size(); ++i)
	{
		pop_up_list.emplace_back(popups_go->children[i]);
		static_cast<ComponentImage*>(popups_go->children[i]->GetComponent(Component::ComponentType::CANVAS_RENDERER))->Disable();
	}
}

GameObject* DialogueManager::ShowPopup(const std::string popup_to_show)
{
	if(ValidPopup(popup_to_show) && IsPopupAvailable())
	{
		static_cast<ComponentImage*>(selected_pop_up->GetComponent(Component::ComponentType::CANVAS_RENDERER))->Enable();
		showing_popup = true;
		return selected_pop_up;
	}

	return nullptr;
}

void DialogueManager::DisablePopup(GameObject* popup_to_disable)
{
	static_cast<ComponentImage*>(popup_to_disable->GetComponent(Component::ComponentType::CANVAS_RENDERER))->Disable();
	showing_popup = false;
}

bool DialogueManager::IsPopupAvailable()
{
	return showing_popup;
}

bool DialogueManager::ValidPopup(const std::string popup_to_show)
{
	bool found = false;

	auto it = std::find_if(pop_up_list.begin(), pop_up_list.end(), [&popup_to_show](GameObject* go)
	{
		return go->name == popup_to_show;
	});
	if (it != pop_up_list.end())
	{
		selected_pop_up = *it;
		found = true;
	}

	return found;
}

float2 DialogueManager::GetCameraCoordinates(const float3& position)
{
	float4 position_float4 = float4(position, 1.f);
	float4 clip_coordinates = App->cameras->main_camera->GetClipMatrix() * position_float4;
	float3 device_coordinates = clip_coordinates.xyz() / clip_coordinates.w;

	float2 canvas_position = float2
	(
		device_coordinates.x * App->ui->main_canvas->GetCanvasScreenSize().x / 2.f,
		device_coordinates.y * App->ui->main_canvas->GetCanvasScreenSize().y / 2.f
	);

	return canvas_position;
}