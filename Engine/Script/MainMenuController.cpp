#include "MainMenuController.h"

#include "Component/ComponentAudioSource.h"
#include "Component/ComponentButton.h"
#include "Component/ComponentScript.h"
#include "Component/ComponentTransform.h"
#include "Component/ComponentTransform2D.h"
#include "Filesystem/PathAtlas.h"

#include "Main/Application.h"
#include "Main/GameObject.h"
#include "Module/ModuleAnimation.h"
#include "Module/ModuleEditor.h"
#include "Module/ModuleInput.h"
#include "Module/ModuleScene.h"
#include "Module/ModuleUI.h"


#include "CharacterSelectionMenuController.h"
#include "CreditsMenuController.h"
#include "HelpMenuController.h"
#include "InputManager.h"
#include "UIMainMenuInputController.h"

#include "imgui.h"



MainMenuController* MainMenuControllerDLL()
{
	MainMenuController* instance = new MainMenuController();
	return instance;
}

MainMenuController::MainMenuController()
{
	
}

// Use this for initialization before Start()
void MainMenuController::Awake()
{
	buttons_transforms.push_back(&play_button->transform_2d);
	buttons_transforms.push_back(&help_button->transform_2d);
	buttons_transforms.push_back(&credits_button->transform_2d);
	buttons_transforms.push_back(&exit_button->transform_2d);

	buttons_components.push_back(static_cast<ComponentButton*>(play_button->GetComponent(Component::ComponentType::UI_BUTTON)));
	buttons_components.push_back(static_cast<ComponentButton*>(help_button->GetComponent(Component::ComponentType::UI_BUTTON)));
	buttons_components.push_back(static_cast<ComponentButton*>(credits_button->GetComponent(Component::ComponentType::UI_BUTTON)));
	buttons_components.push_back(static_cast<ComponentButton*>(exit_button->GetComponent(Component::ComponentType::UI_BUTTON)));

	character_selection_controller = static_cast<CharacterSelectionMenuController*>(character_selection_game_object->GetComponentScript("CharacterSelectionMenuController")->script);
	credits_controller = static_cast<CreditsMenuController*>(credits_game_object->GetComponentScript("CreditsMenuController")->script);
	help_controller = static_cast<HelpMenuController*>(help_game_object->GetComponentScript("HelpMenuController")->script);


	audio_source = (ComponentAudioSource*) audio_controller->GetComponent(Component::ComponentType::AUDIO_SOURCE);
	audio_source->PlayEvent("play_music_menu");

	cursor->SetEnabled(false);

	GameObject* input_go = App->scene->GetGameObjectByName("Input Manager");
	const ComponentScript* input_component = input_go->GetComponentScript("InputManager");
	input_manager = (InputManager*)input_component->script;

	for (auto particle : forest_particles->children)
	{
		ComponentImage* img = (ComponentImage*)particle->GetComponent(Component::ComponentType::UI_IMAGE);
		img->SetColor(float4::zero);
	}

	InitIntroSequence();
}


// Update is called once per frame
void MainMenuController::Update()
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

	if (!idle_anim_started)
	{
		InitIdleSequence();
	}

	for (unsigned int i = 0; i < buttons_components.size(); ++i)
	{
		if (buttons_components[i]->IsClicked())
		{
			TweenSequence* sequence = App->animations->CreateTweenSequence();
			sequence->Append(Tween::LOScale(buttons_transforms[i], float3::one * 1.1f, 0.25f)->SetEase(Tween::EaseType::SINE));
			sequence->OnCompleted([this, i](void) {
				OpenSubMenu(i);
			});
			sequence->Play();
		}
	}

	if (UIMainMenuInputController::ComfirmButtonPressed(input_manager))
	{
		TweenSequence* sequence = App->animations->CreateTweenSequence();
		sequence->Append(Tween::LOScale(buttons_transforms[current_highlighted_button], float3::one * 1.1f, 0.25f)->SetEase(Tween::EaseType::SINE));
		sequence->OnCompleted([this](void) {
			OpenSubMenu(current_highlighted_button);
		});
		sequence->Play();
	}

	if(UIMainMenuInputController::ConfirmMovedUp(input_manager))
	{
		if (!cursor_initialized)
		{
			cursor_initialized = true;
			cursor->SetEnabled(true);
		}

		audio_source->PlayEvent("menu_hover");

		current_highlighted_button -= 1;
		current_highlighted_button = current_highlighted_button % 4;
		MoveCursor();
	}
	else if(UIMainMenuInputController::ConfirmMovedDown(input_manager))
	{
		if (!cursor_initialized)
		{
			cursor_initialized = true;
			cursor->SetEnabled(true);
		}

		audio_source->PlayEvent("menu_hover");

		current_highlighted_button += 1;
		current_highlighted_button = current_highlighted_button % 4;
		MoveCursor();
	}
}

void MainMenuController::OpenSubMenu(int menu_index)
{
	//audio_source->PlayEvent("click_forward");
	audio_source->PlayEvent("menu_select");
	//Change scene
	switch (menu_index)
	{
	case 0:
		character_selection_controller->Open();
		break;
	case 1:
		//Active help
		help_controller->Open();
		break;
	case 2:
		//Active credits
		credits_controller->Open();
		break;
	case 3:
		//Close game
		SDL_Event quit_event;
		quit_event.type = SDL_QUIT;
		SDL_PushEvent(&quit_event);
		break;
	default:
		break;
	}

	Close();
}

void MainMenuController::Open()
{
	enabled = true;
	just_opened = true;
	main_menu_panel->SetEnabled(true);
	forest_particles->SetEnabled(true);
}

void MainMenuController::Close()
{
	enabled = false;
	main_menu_panel->SetEnabled(false);
	forest_particles->SetEnabled(false);
}

void MainMenuController::MoveCursor()
{
	cursor->transform_2d.SetTranslation(float3(buttons_transforms[current_highlighted_button]->GetGlobalTranslation().x - buttons_transforms[current_highlighted_button]->GetWidth() / 2 - 10.f,
		buttons_transforms[current_highlighted_button]->GetGlobalTranslation().y, 0.0f));
}

void MainMenuController::InitIntroSequence()
{
	TweenSequence* sequence = App->animations->CreateTweenSequence();

	sequence->Append(Tween::LOColor((ComponentImage*)background->GetComponent(Component::ComponentType::UI_IMAGE), float4::one, 1.5f));

	for (size_t i = 0; i < buttons_transforms.size(); i++)
	{
		sequence->Insert(0.25f * i, Tween::LOTranslate(buttons_transforms[i], float2(0, 111.0f - (74.0f * i)), 0.75f)->SetEase(Tween::EaseType::SMOOTH_STEP));
	}

	ComponentImage* img = (ComponentImage*) title->GetComponent(Component::ComponentType::UI_IMAGE);
	img->SetColor(float4::zero);

	sequence->Insert(0.5f, Tween::LOColor(img, float4::one, 1.0f)->SetEase(Tween::EaseType::SMOOTH_STEP));
	sequence->OnCompleted([this](void)
	{
		intro_anim_finished = true;
	});
	
	sequence->Play();
}

void MainMenuController::InitIdleSequence()
{
	if (!intro_anim_finished) return;

	idle_anim_started = true;
	
	TweenSequence* sequence = App->animations->CreateTweenSequence();

	for(auto particle : forest_particles->children)
	{
		ComponentImage* img = (ComponentImage*)particle->GetComponent(Component::ComponentType::UI_IMAGE);
		sequence->Append(Tween::LOColor(img, float4::one, 2.0f)->SetEase(Tween::EaseType::EASE_IN_BACK)->SetLoops(-1, Tween::TweenLoop::YOYO));
	}

	sequence->Play();
}

// Use this for showing variables on inspector
void MainMenuController::OnInspector(ImGuiContext* context)
{
	//Necessary to be able to write with imgui
	ImGui::SetCurrentContext(context);
	ShowDraggedObjects();

}

//Use this for linking JUST GO automatically
void MainMenuController::InitPublicGameObjects()
{
	//IMPORTANT, public gameobjects, name_gameobjects and go_uuids MUST have same size

	public_gameobjects.push_back(&play_button);
	variable_names.push_back(GET_VARIABLE_NAME(play_button));

	public_gameobjects.push_back(&help_button);
	variable_names.push_back(GET_VARIABLE_NAME(help_button));

	public_gameobjects.push_back(&credits_button);
	variable_names.push_back(GET_VARIABLE_NAME(credits_button));

	public_gameobjects.push_back(&exit_button);
	variable_names.push_back(GET_VARIABLE_NAME(exit_button));

	public_gameobjects.push_back(&credits_game_object);
	variable_names.push_back(GET_VARIABLE_NAME(credits_game_object));

	public_gameobjects.push_back(&help_game_object);
	variable_names.push_back(GET_VARIABLE_NAME(help_game_object));

	public_gameobjects.push_back(&character_selection_game_object);
	variable_names.push_back(GET_VARIABLE_NAME(character_selection_game_object));
	
	public_gameobjects.push_back(&main_menu_panel);
	variable_names.push_back(GET_VARIABLE_NAME(main_menu_panel));

	public_gameobjects.push_back(&cursor);
	variable_names.push_back(GET_VARIABLE_NAME(cursor));
	
	public_gameobjects.push_back(&background);
	variable_names.push_back(GET_VARIABLE_NAME(background));

	public_gameobjects.push_back(&title);
	variable_names.push_back(GET_VARIABLE_NAME(title));

	public_gameobjects.push_back(&forest_particles);
	variable_names.push_back(GET_VARIABLE_NAME(forest_particles));

	public_gameobjects.push_back(&audio_controller);
	variable_names.push_back(GET_VARIABLE_NAME(audio_controller));

	for (unsigned int i = 0; i < public_gameobjects.size(); ++i)
	{
		name_gameobjects.push_back(is_object);
		go_uuids.push_back(0);
	}
}



