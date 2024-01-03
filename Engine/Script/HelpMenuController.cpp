#include "HelpMenuController.h"
#include "Component/ComponentAudioSource.h"
#include "Component/ComponentButton.h"

#include "Main/Application.h"
#include "Main/GameObject.h"
#include "Module/ModuleAnimation.h"
#include "Module/ModuleInput.h"
#include "Module/ModuleScene.h"

#include "InputManager.h"
#include "MainMenuController.h"
#include "UIMainMenuInputController.h"

#include "imgui.h"

HelpMenuController* HelpMenuControllerDLL()
{
	HelpMenuController* instance = new HelpMenuController();
	return instance;
}

HelpMenuController::HelpMenuController()
{
	
}

void HelpMenuController::Awake()
{
	audio_source = (ComponentAudioSource*)audio_controller->GetComponent(Component::ComponentType::AUDIO_SOURCE);
	main_menu_controller = static_cast<MainMenuController*>(main_menu_panel->GetComponentScript("MainMenuController")->script);

	help_joycon_button = static_cast<ComponentButton*>(help_joycon_button_game_object->GetComponent(Component::ComponentType::UI_BUTTON));
	help_keyboard_button = static_cast<ComponentButton*>(help_keyboard_button_game_object->GetComponent(Component::ComponentType::UI_BUTTON));
	back_button = static_cast<ComponentButton*>(back_button_game_object->GetComponent(Component::ComponentType::UI_BUTTON));

	GameObject* input_go = App->scene->GetGameObjectByName("Input Manager");
	const ComponentScript* input_component = input_go->GetComponentScript("InputManager");
	input_manager = (InputManager*)input_component->script;
}

void HelpMenuController::Update()
{
	if (!enabled)
	{
		return;
	}
	if (just_opened)
	{
		just_opened = false;
		return;
	}

	if (UIMainMenuInputController::ConfirmMovedLeft(input_manager) || UIMainMenuInputController::ConfirmMovedRight(input_manager))
	{
		help_joycon_background->SetEnabled(!help_joycon_background->IsEnabled());
		help_keyboard_background->SetEnabled(!help_keyboard_background->IsEnabled());
		audio_source->PlayEvent("menu_hover");
	}
	if (help_joycon_button->IsClicked())
	{
		help_joycon_background->SetEnabled(true);
		help_keyboard_background->SetEnabled(false);
	}
	if (help_keyboard_button->IsClicked())
	{
		help_joycon_background->SetEnabled(false);
		help_keyboard_background->SetEnabled(true);
	}

	if (UIMainMenuInputController::ComfirmButtonPressed(input_manager))
	{
		TweenSequence* sequence = App->animations->CreateTweenSequence();
		sequence->Append(Tween::LOScale(&back_button->owner->transform_2d, float3::one * 1.1f, 0.25f)->SetEase(Tween::EaseType::SINE));
		sequence->OnCompleted([this](void) {
			Close();
		});
		sequence->Play();

		return;
	}
	if (back_button->IsClicked())
	{
		TweenSequence* sequence = App->animations->CreateTweenSequence();
		sequence->Append(Tween::LOScale(&back_button->owner->transform_2d, float3::one * 1.1f, 0.25f)->SetEase(Tween::EaseType::SINE));
		sequence->OnCompleted([this](void) {
			Close();
		});
		sequence->Play();

		return;
	}
}

void HelpMenuController::Open()
{
	enabled = true;
	just_opened = true;
	help_menu_panel->SetEnabled(true);
	help_joycon_background->SetEnabled(true);
	help_keyboard_background->SetEnabled(false);
}

void HelpMenuController::Close()
{
	enabled = false;
	help_menu_panel->SetEnabled(false);
	//audio_source->PlayEvent("Click_backward");
	audio_source->PlayEvent("menu_select");
	main_menu_controller->Open();
}

void HelpMenuController::OnInspector(ImGuiContext* context)
{
	ImGui::SetCurrentContext(context);
	ShowDraggedObjects();

}

void HelpMenuController::InitPublicGameObjects()
{
	public_gameobjects.push_back(&main_menu_panel);
	variable_names.push_back(GET_VARIABLE_NAME(main_menu_panel));

	public_gameobjects.push_back(&help_menu_panel);
	variable_names.push_back(GET_VARIABLE_NAME(help_menu_panel));

	public_gameobjects.push_back(&help_joycon_background);
	variable_names.push_back(GET_VARIABLE_NAME(help_joycon));

	public_gameobjects.push_back(&help_keyboard_background);
	variable_names.push_back(GET_VARIABLE_NAME(help_keyboard));

	public_gameobjects.push_back(&help_joycon_button_game_object);
	variable_names.push_back(GET_VARIABLE_NAME(help_joycon_button_game_object));

	public_gameobjects.push_back(&help_keyboard_button_game_object);
	variable_names.push_back(GET_VARIABLE_NAME(help_keyboard_button_game_object));

	public_gameobjects.push_back(&back_button_game_object);
	variable_names.push_back(GET_VARIABLE_NAME(back_button_game_object));

	public_gameobjects.push_back(&audio_controller);
	variable_names.push_back(GET_VARIABLE_NAME(audio_controller));

	for (unsigned int i = 0; i < public_gameobjects.size(); ++i)
	{
		name_gameobjects.push_back(is_object);
		go_uuids.push_back(0);
	}
}


