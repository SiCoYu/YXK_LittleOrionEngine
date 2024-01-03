#include "UIManager.h"

#include "Component/ComponentImage.h"
#include "Component/ComponentScript.h"
#include "Component/ComponentTransform.h"



#include "Main/Application.h"
#include "Main/GameObject.h"
#include "Module/ModuleAnimation.h"
#include "Module/ModuleCamera.h"
#include "Module/ModuleInput.h"
#include "Module/ModuleScene.h"
#include "Module/ModuleTime.h"
#include "Module/ModuleUI.h"


#include "ComboCounterIndicator.h"
#include "DamageIndicatorSpawner.h"
#include "InputManager.h"
#include "PauseMenuController.h"
#include "ProgressBar.h"
#include "SpecialProgressBar.h"
#include "UILives.h"

#include "imgui.h"

UIManager* UIManagerDLL()
{
	UIManager* instance = new UIManager();
	return instance;
}

UIManager::UIManager()
{

}

// Use this for initialization before Start()
void UIManager::Awake()
{
	player1_health_progress_bar = static_cast<ProgressBar*>(player1_health_progress_bar_game_object->GetComponentScript("ProgressBar")->script);
	player2_health_progress_bar = static_cast<ProgressBar*>(player2_health_progress_bar_game_object->GetComponentScript("ProgressBar")->script);

	player1_light_ball_ui = static_cast<ProgressBar*>(player1_light_ball_ui_game_object->GetComponentScript("ProgressBar")->script);
	player1_shield_ui = static_cast<ProgressBar*>(player1_shield_ui_game_object->GetComponentScript("ProgressBar")->script);

	player2_light_ball_ui = static_cast<ProgressBar*>(player2_light_ball_ui_game_object->GetComponentScript("ProgressBar")->script);
	player2_shield_ui = static_cast<ProgressBar*>(player2_shield_ui_game_object->GetComponentScript("ProgressBar")->script);

	damage_indicator_spawner = static_cast<DamageIndicatorSpawner*>(damage_indicator_spawner_game_object->GetComponentScript("DamageIndicatorSpawner")->script);


	GameObject* combo_counter_indicator_go = App->scene->GetGameObjectByName("Combo Counter Indicator");
	combo_counter_indicator = static_cast<ComboCounterIndicator*>(combo_counter_indicator_go->GetComponentScript("ComboCounterIndicator")->script);

	pause_menu_controller = static_cast<PauseMenuController*>(pause_menu_controller_go->GetComponentScript("PauseMenuController")->script);

	player1_hud_position = player1_hud->transform_2d.anchored_position;
	player2_hud_position = player2_hud->transform_2d.anchored_position;

	GameObject* ui_lives_go = App->scene->GetGameObjectByName("UILives");
	lives_ui = static_cast<UILives*>(ui_lives_go->GetComponentScript("UILives")->script);

}

// Use this for initialization
void UIManager::Start()
{
	GameObject* input_go = App->scene->GetGameObjectByName("Input Manager");
	const ComponentScript* input_component = input_go->GetComponentScript("InputManager");
	input_manager = (InputManager*)input_component->script;
}

// Update is called once per frame
void UIManager::Update()
{
	if (transitioning && App->input->GetAnyKeyPressedDown())
	{
		App->scene->LoadScene(0);
	}
}

void UIManager::FillLightballUI(unsigned int player, float progress) const
{
	progress = math::Clamp01(progress);
	if (player == 0) 
	{
		player1_light_ball_ui->SetProgress(progress);
	}
	else 
	{
		player2_light_ball_ui->SetProgress(progress);
	}
}

void UIManager::EmptyLightballUI(bool multiplayer) const
{
	player1_light_ball_ui->SetProgress(0.f);
	if (multiplayer) 
	{
		player2_light_ball_ui->SetProgress(0.f);
	}
}

void UIManager::ShowPlayersLightballUI(bool multiplayer) const
{
	player1_light_ball_ui_game_object->SetEnabled(true);
	if (multiplayer) 
	{
		player2_light_ball_ui_game_object->SetEnabled(true);
	}

}

void UIManager::HideLightballUI(bool multiplayer) const
{
	player1_light_ball_ui_game_object->SetEnabled(false);
	if (multiplayer)
	{
		player2_light_ball_ui_game_object->SetEnabled(false);
	}
}

void UIManager::UpdateShieldlUI(unsigned int player, float durability) const
{
	if (player == 0)
	{
		player1_shield_ui->SetProgress(durability);
	}
	else
	{
		player2_shield_ui->SetProgress(durability);
	}
}

void UIManager::SetPlayer1Health(float percentage)
{
	if (percentage < player1_health_progress_bar->progress)
	{
		ShakePlayerHud(player1_hud, player1_hud_position);
		ShowDamageOverlay(percentage);
	}

	player1_health_progress_bar->SetProgress(percentage);
}

void UIManager::SetPlayer2Health(float percentage)
{
	if (percentage < player2_health_progress_bar->progress)
	{
		ShakePlayerHud(player2_hud, player2_hud_position);
		ShowDamageOverlay(percentage);
	}

	player2_health_progress_bar->SetProgress(percentage);
}

void UIManager::SetPlayer1SpecialPoints(float percentage)
{
	player1_special_progress_bar->SetProgress(percentage);
}

void UIManager::SetPlayer2SpecialPoints(float percentage)
{
	player2_special_progress_bar->SetProgress(percentage);
}

void UIManager::SetComboCounterProgress(float combo_counter_progress)
{
	combo_counter_indicator->SetComboCounterProgress(combo_counter_progress);
}

void UIManager::SetComboCounterLevel(ComboCounter::ComboCounterLevel combo_counter_level)
{
	combo_counter_indicator->SetComboCounterLevel(combo_counter_level);
}

void UIManager::HideComboCounter()
{
	combo_counter_indicator->Hide();
}

void UIManager::SpawnDamageIndicator(int damage, const float3& receiver_position, const float3& agressor_position)
{
	damage_indicator_spawner->SpawnDamageIndicator(damage, receiver_position, agressor_position);
}

void UIManager::ShowReviveButton(const float3& position)
{
	selected_revive_button = SelectReviveButton();
	selected_revive_button->SetEnabled(true);
	float4 position_float4 = float4(position, 1.f);
	float4 clip_coordinates = App->cameras->main_camera->GetClipMatrix() * position_float4;
	float3 device_coordinates = clip_coordinates.xyz() / clip_coordinates.w;

	float2 canvas_position = float2
	(
		device_coordinates.x * App->ui->main_canvas->GetCanvasScreenSize().x / 2.f,
		device_coordinates.y * App->ui->main_canvas->GetCanvasScreenSize().y / 2.f
	);
	selected_revive_button->transform_2d.SetGlobalMatrixTranslation(float3(canvas_position, 0.f));
}

GameObject* UIManager::SelectReviveButton()
{
	if (input_manager->total_game_controllers == 0) 
	{
		return revive_button_keyboard;
	}
	else if (input_manager->total_game_controllers == 1)
	{
		if (player1_health_progress_bar->progress > 0.f)
		{
			return revive_button_keyboard;
		}
		else
		{
			return revive_button_controller;
		}
	}
	else 
	{
		return revive_button_controller;
	}
}

void UIManager::IncreaseLives()
{
	lives_ui->IncreaseLives();
}

void UIManager::DecreaseLives()
{
	lives_ui->DecreaseLives();
}

void UIManager::HideReviveButton()
{
	if (selected_revive_button != nullptr)
	{
		selected_revive_button->SetEnabled(false);
	}
}

void UIManager::SetSecondPlayerUI()
{
	second_player_hud->SetEnabled(true);
}

void UIManager::SetWinScreen()
{
	ShowMenu(win_screen);
	transitioning = true;
}

void UIManager::SetLoseScreen()
{
	ShowMenu(lose_screen);
	transitioning = true;
}

void UIManager::SetPauseScreen(bool open)
{
	if (open)
	{
		pause_menu_controller->Open();
	}
	else
	{
		pause_menu_controller->Close();
	}
}

void UIManager::ShowMenu(GameObject * menu)
{
	menu->SetEnabled(true);

	ComponentImage* img = (ComponentImage*)menu->GetComponent(Component::ComponentType::UI_IMAGE);
	img->SetColor(float4::zero);

	TweenSequence* sequence = App->animations->CreateTweenSequence();
	sequence->Append(Tween::LOColor(img, float4::one, 1.0f)->SetEase(Tween::EaseType::SMOOTH_STEP));
	sequence->Play();
}

void UIManager::ShakePlayerHud(GameObject* hud, const float2 starting_position)
{
	TweenSequence* sequence = App->animations->CreateTweenSequence();

	for (size_t i = 0; i < 5; i++)
	{
		float rand_x = RandomRange(-10.0f, 10.0f);
		float rand_y = RandomRange(-10.0f, 10.0f);
		float rand_rotation = RandomRange(-0.2f, 0.2f);

		sequence->Append(Tween::LOTranslate(&hud->transform_2d, float2(starting_position.x + rand_x, starting_position.y + rand_y), 0.1f));
		sequence->Insert(0.0f, Tween::LORotate(&hud->transform_2d, rand_rotation, 0.1f));
	}

	sequence->OnCompleted([hud, starting_position](void)
	{
		hud->transform_2d.SetTranslation(float3(starting_position, 0.0f));
		hud->transform_2d.SetRotation(float3::zero);
	});
	sequence->Play();
}

void UIManager::ShowDamageOverlay(float percentage)
{
	if (percentage > 0.25f) return;

	TweenSequence* sequence = App->animations->CreateTweenSequence();
	ComponentImage* img = (ComponentImage*)damage_overlay->GetComponent(Component::ComponentType::UI_IMAGE);
	img->Enable();
	img->SetColor(float4::zero);

	sequence->Append(Tween::LOColor(img, float4(0.75f, 0.0f, 0.0f, 0.2f), 0.45f)->SetEase(Tween::EaseType::SINE));


	sequence->OnCompleted([img](void)
	{
		img->SetColor(float4::zero);
		img->Disable();
	});

	sequence->Play();
}

float UIManager::RandomRange(float a, float b)
{
	float random = ((float)rand()) / (float)RAND_MAX;
	float diff = b - a;
	float r = random * diff;
	return a + r;
}

// Use this for showing variables on inspector
void UIManager::OnInspector(ImGuiContext* context)
{
	//Necessary to be able to write with imgui
	ImGui::SetCurrentContext(context);
	ShowDraggedObjects();

	ImGui::Checkbox("Transitioning", &transitioning);
}

//Use this for linking JUST GO automatically
void UIManager::InitPublicGameObjects()
{
	//IMPORTANT, public gameobjects, name_gameobjects and go_uuids MUST have same size

	public_gameobjects.push_back(&player1_health_progress_bar_game_object);
	variable_names.push_back(GET_VARIABLE_NAME(player1_health_progress_bar_game_object));

	public_gameobjects.push_back(&player2_health_progress_bar_game_object);
	variable_names.push_back(GET_VARIABLE_NAME(player2_health_progress_bar_game_object));

	public_gameobjects.push_back(&damage_indicator_spawner_game_object);
	variable_names.push_back(GET_VARIABLE_NAME(damage_indicator_spawner_game_object));

	public_gameobjects.push_back(&second_player_hud);
	variable_names.push_back(GET_VARIABLE_NAME(second_player_hud));

	public_gameobjects.push_back(&win_screen);
	variable_names.push_back(GET_VARIABLE_NAME(win_screen));

	public_gameobjects.push_back(&lose_screen);
	variable_names.push_back(GET_VARIABLE_NAME(lose_screen));

	public_gameobjects.push_back(&pause_menu_controller_go);
	variable_names.push_back(GET_VARIABLE_NAME(pause_menu_controller_go));

	public_gameobjects.push_back(&revive_button_keyboard);
	variable_names.push_back(GET_VARIABLE_NAME(revive_button_keyboard));

	public_gameobjects.push_back(&revive_button_controller);
	variable_names.push_back(GET_VARIABLE_NAME(revive_button_controller));

	public_gameobjects.push_back(&player1_hud);
	variable_names.push_back(GET_VARIABLE_NAME(player1_hud));

	public_gameobjects.push_back(&player2_hud);
	variable_names.push_back(GET_VARIABLE_NAME(player2_hud));

	public_gameobjects.push_back(&damage_overlay);
	variable_names.push_back(GET_VARIABLE_NAME(damage_overlay));

	public_gameobjects.push_back(&player1_light_ball_ui_game_object);
	variable_names.push_back(GET_VARIABLE_NAME(player1_light_ball_ui_game_object));

	public_gameobjects.push_back(&player1_shield_ui_game_object);
	variable_names.push_back(GET_VARIABLE_NAME(player1_shield_ui_game_object));

	public_gameobjects.push_back(&player2_light_ball_ui_game_object);
	variable_names.push_back(GET_VARIABLE_NAME(player2_light_ball_ui_game_object));

	public_gameobjects.push_back(&player2_shield_ui_game_object);
	variable_names.push_back(GET_VARIABLE_NAME(player2_shield_ui_game_object));

	for (int i = 0; i < public_gameobjects.size(); ++i)
	{
		name_gameobjects.push_back(is_object);
		go_uuids.push_back(0);
	}
}
