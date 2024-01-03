#include "PauseMenuController.h"

#include "Component/ComponentAudioSource.h"

#include "Main/Application.h"
#include "Main/GameObject.h"
#include "Module/ModuleAnimation.h"
#include "Module/ModuleInput.h"
#include "Module/ModuleTime.h"
#include "Module/ModuleScene.h"

#include "InputManager.h"
#include "UIMainMenuInputController.h"
#include "WorldManager.h"

#include "imgui.h"

PauseMenuController* PauseMenuControllerDLL()
{
	PauseMenuController* instance = new PauseMenuController();
	return instance;
}

PauseMenuController::PauseMenuController()
{
	
}

void PauseMenuController::Awake()
{
	buttons.push_back(resume_button);
	buttons.push_back(main_menu_button);
	buttons.push_back(level_selection_button);
	buttons.push_back(help_button);

	//audio_source = (ComponentAudioSource*)audio_controller->GetComponent(Component::ComponentType::AUDIO_SOURCE);
	GameObject* audio_ui_go = App->scene->GetGameObjectByName("AudioUI");
	audio_source = static_cast<ComponentAudioSource*>(audio_ui_go->GetComponent(Component::ComponentType::AUDIO_SOURCE));

	current = buttons.size() - 1;
	awaked = true;

	GameObject* world_go = App->scene->GetGameObjectByName("World Manager");
	const ComponentScript* world_component = world_go->GetComponentScript("WorldManager");
	world_manager = (WorldManager*)world_component->script;
	
	GameObject* input_go = App->scene->GetGameObjectByName("Input Manager");
	const ComponentScript* input_component = input_go->GetComponentScript("InputManager");
	input_manager = (InputManager*)input_component->script;

}

void PauseMenuController::Update()
{
	if (!awaked)
	{
		Awake();
	}
	if (!pause_menu_panel->IsEnabled() && !on_help)
	{
		return;
	}

	if(on_help)
	{

		if (UIMainMenuInputController::ConfirmMovedLeft(input_manager))
		{
			help_panel_1->SetEnabled(true);
			help_panel_2->SetEnabled(false);
			audio_source->PlayEvent("menu_hover");
		}

		if (UIMainMenuInputController::ConfirmMovedRight(input_manager))
		{
			help_panel_1->SetEnabled(false);
			help_panel_2->SetEnabled(true);
			audio_source->PlayEvent("menu_hover");
		}

		if (UIMainMenuInputController::ComfirmButtonPressed(input_manager) 
			|| UIMainMenuInputController::CancelButtonPressed(input_manager))
		{
			on_help = false;
			help_panel_1->SetEnabled(false);
			help_panel_2->SetEnabled(false);
			pause_menu_panel->SetEnabled(true);
			audio_source->PlayEvent("menu_select");
		}

		return;
	}

	if (UIMainMenuInputController::ComfirmButtonPressed(input_manager))
	{
		audio_source->PlayEvent("menu_select");
		//Change scene
		switch (current)
		{
			case 1:
				App->time->time_scale = 1.f;
				App->scene->LoadScene(MAIN_MENU_SCENE);
				break;
			case 3:
				//Active help
				help_panel_1->SetEnabled(true);
				pause_menu_panel->SetEnabled(false);
				on_help = true;
				return;
			case 0:
				//Resume game
				Close();
				return;
			case 2:
				//Active level selection
				//load
				App->time->time_scale = 1.f;
				world_manager->level_selection_activated = true;
				App->scene->LoadScene(MAIN_MENU_SCENE);

				break;
			default:
				break;
		}
	}
	if (UIMainMenuInputController::ConfirmMovedDown(input_manager))
	{
		current = (current - 1) == -1 ? buttons.size() -1: current-1;
		pause_cursor_go->transform_2d.SetTranslation(float3(buttons[current]->transform_2d.GetTranslation().x - (buttons[current]->transform_2d.GetWidth()/2) - OFFSET_X, buttons[current]->transform_2d.GetTranslation().y, 0.0f));
		audio_source->PlayEvent("menu_hover");
	}

	else if (UIMainMenuInputController::ConfirmMovedUp(input_manager))
	{
		current += 1;
		current = current % buttons.size();
		pause_cursor_go->transform_2d.SetTranslation(float3(buttons[current]->transform_2d.GetTranslation().x - (buttons[current]->transform_2d.GetWidth() / 2) - OFFSET_X, buttons[current]->transform_2d.GetTranslation().y, 0.0f));
		audio_source->PlayEvent("menu_hover");
	}
}

void PauseMenuController::Open()
{
	current = 0;
	pause_cursor_go->transform_2d.SetTranslation(float3(buttons[current]->transform_2d.GetTranslation().x - (buttons[current]->transform_2d.GetWidth() / 2) - OFFSET_X, buttons[current]->transform_2d.GetTranslation().y, 0.0f));
	pause_menu_panel->SetEnabled(true);
	help_panel_1->SetEnabled(false);
	help_panel_2->SetEnabled(false);
	pause_menu_panel->SetEnabled(true);
	pause_cursor_go->transform_2d.SetTranslation(float3(buttons[0]->transform_2d.GetTranslation().x - (buttons[0]->transform_2d.GetWidth() / 2) - OFFSET_X, buttons[0]->transform_2d.GetTranslation().y, 0.0f));

	patterns->SetEnabled(true);
	SetupPatterns();
}

void PauseMenuController::Close()
{
	world_manager->on_pause = false;
	App->time->time_scale = 1.f;
	pause_menu_panel->SetEnabled(false);
	help_panel_1->SetEnabled(false);
	help_panel_2->SetEnabled(false);
	patterns->SetEnabled(false);
	on_help = false;
}

void PauseMenuController::SetupPatterns()
{
	pattern_b_pos = patterns->children[0]->transform_2d.anchored_position;

	ComponentImage* img_b = (ComponentImage*)patterns->children[0]->GetComponent(Component::ComponentType::UI_IMAGE);
	
	img_b->SetColor(float4(1, 1, 1, 0.5f));
	
	TweenSequence* sequence = App->animations->CreateTweenSequence();
	sequence->Append(Tween::LOColor(img_b, float4::zero, 2.5f)->SetLoops(-1, Tween::TweenLoop::YOYO));
	
	sequence->Play();
}

void PauseMenuController::OnInspector(ImGuiContext* context)
{
	ImGui::SetCurrentContext(context);
	ShowDraggedObjects();

	ImGui::Checkbox("On Help", &on_help);
}

void PauseMenuController::InitPublicGameObjects()
{
	public_gameobjects.push_back(&resume_button);
	variable_names.push_back(GET_VARIABLE_NAME(resume_button));

	public_gameobjects.push_back(&help_button);
	variable_names.push_back(GET_VARIABLE_NAME(help_button));

	public_gameobjects.push_back(&level_selection_button);
	variable_names.push_back(GET_VARIABLE_NAME(level_selection_button));

	public_gameobjects.push_back(&main_menu_button);
	variable_names.push_back(GET_VARIABLE_NAME(main_menu_button));

	public_gameobjects.push_back(&audio_controller);
	variable_names.push_back(GET_VARIABLE_NAME(audio_controller));

	public_gameobjects.push_back(&help_panel_1);
	variable_names.push_back(GET_VARIABLE_NAME(help_panel_1));

	public_gameobjects.push_back(&help_panel_2);
	variable_names.push_back(GET_VARIABLE_NAME(help_panel_2));

	public_gameobjects.push_back(&pause_menu_panel);
	variable_names.push_back(GET_VARIABLE_NAME(pause_menu_panel));

	public_gameobjects.push_back(&pause_cursor_go);
	variable_names.push_back(GET_VARIABLE_NAME(pause_cursor_go));

	public_gameobjects.push_back(&patterns);
	variable_names.push_back(GET_VARIABLE_NAME(patterns));

	for (unsigned int i = 0; i < public_gameobjects.size(); ++i)
	{
		name_gameobjects.push_back(is_object);
		go_uuids.push_back(0);
	}
}


