#include "LevelSelectionMenuController.h"

#include "Component/ComponentAudioSource.h"
#include "Component/ComponentButton.h"
#include "Component/ComponentImage.h"
#include "Component/ComponentScript.h"
#include "Component/ComponentTransform2D.h"

#include "Main/Application.h"
#include "Main/GameObject.h"
#include "Module/ModuleEditor.h"
#include "Module/ModuleInput.h"
#include "Module/ModuleScene.h"
#include "Module/ModuleTime.h"
#include "UIMainMenuInputController.h"


#include "CharacterSelectionMenuController.h"
#include "InputManager.h"
#include "WorldManager.h"

#include "imgui.h"

LevelSelectionMenuController* LevelSelectionMenuControllerDLL()
{
	LevelSelectionMenuController* instance = new LevelSelectionMenuController();
	return instance;
}

LevelSelectionMenuController::LevelSelectionMenuController()
{
	
}

// Use this for initialization before Start()
void LevelSelectionMenuController::Awake()
{
	cursor_positions.push_back(&level1_cursor_position->transform_2d);
	cursor_positions.push_back(&level2_cursor_position->transform_2d);
	cursor_positions.push_back(&level3_cursor_position->transform_2d);

	character_menu_controller = static_cast<CharacterSelectionMenuController*>(character_menu_game_object->GetComponentScript("CharacterSelectionMenuController")->script);
	back_button = static_cast<ComponentButton*>(back_button_game_object->GetComponent(Component::ComponentType::UI_BUTTON));
	audio_source = (ComponentAudioSource*)audio_controller->GetComponent(Component::ComponentType::AUDIO_SOURCE);

	GameObject* input_go = App->scene->GetGameObjectByName("Input Manager");
	const ComponentScript* input_component = input_go->GetComponentScript("InputManager");
	input_manager = (InputManager*)input_component->script;
	
	world_manager_game_object = App->scene->GetGameObjectByName("World Manager");
	world_manager = static_cast<WorldManager*>(world_manager_game_object->GetComponentScript("WorldManager")->script);

	level_thumbnails.push_back(level1_thumbnail);
	level_thumbnails.push_back(level2_thumbnail);
	level_thumbnails.push_back(level3_thumbnail);
}

void LevelSelectionMenuController::Update()
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

	//Back
	if (!selecting_level && UIMainMenuInputController::ComfirmButtonPressed(input_manager))
	{
		Close();
		return;
	}
	if (back_button->IsClicked())
	{
		Close();
		return;
	}

	if (selecting_level && UIMainMenuInputController::ComfirmButtonPressed(input_manager))
	{
		//audio_source->PlayEvent("click_forward");
		audio_source->PlayEvent("menu_select");
		//Change scene
		switch (current)
		{
		case 0:
			//level1
			//audio_source->PlayEvent("start_game");
			world_manager->LoadLevel(Level::FIRST);
			return;
		case 1:
			//level2
			world_manager->LoadLevel(Level::SECOND);
			return;
		case 2:
			//boss
			world_manager->LoadLevel(Level::BOSS);
			break;
		default:
			break;
		}
	}

	if(UIMainMenuInputController::ConfirmMovedUp(input_manager) || UIMainMenuInputController::ConfirmMovedDown(input_manager))
	{
		selecting_level = !selecting_level;
		back_cursor->SetEnabled(!back_cursor->IsEnabled());
		audio_source->PlayEvent("menu_hover");
	}

	float4 color = float4::one;
	if (selecting_level)
	{
		color.x = (math::Sin(App->time->time * 0.01f) + 1) * 0.5f;
	}
	static_cast<ComponentImage*>(level_selection_cursor->GetComponent(Component::ComponentType::UI_IMAGE))->SetColor(color);


	// This code is not needed atm as we have just one level
	if (selecting_level && UIMainMenuInputController::ConfirmMovedLeft(input_manager))
	{
		current = (current - 1) == -1 ? NUM_UNLOCKED_LEVELS - 1 : current - 1;
		current = current % NUM_UNLOCKED_LEVELS;
		level_selection_cursor->transform_2d.SetGlobalMatrixTranslation(cursor_positions[current]->GetGlobalTranslation());
	}
	else if (selecting_level && UIMainMenuInputController::ConfirmMovedRight(input_manager))
	{
		current += 1;
		current = current % NUM_UNLOCKED_LEVELS;
		level_selection_cursor->transform_2d.SetGlobalMatrixTranslation(cursor_positions[current]->GetGlobalTranslation());
	}

	if (input_manager->total_game_controllers <= 0)
	{
		gamepad_icons->SetEnabled(false);
		pc_icons->SetEnabled(true);
	}
	else
	{
		gamepad_icons->SetEnabled(true);
		pc_icons->SetEnabled(false);
	}
	
}

void LevelSelectionMenuController::Open()
{
	enabled = true;
	just_opened = true;
	selecting_level = true;
	level_selection_panel->SetEnabled(true);
	back_cursor->SetEnabled(false);
	gamepad_icons->SetEnabled(false);
	pc_icons->SetEnabled(false);
	UnlockLevels(NUM_UNLOCKED_LEVELS);
}

void LevelSelectionMenuController::Close()
{
	enabled = false;
	level_selection_panel->SetEnabled(false);
	character_menu_controller->Open();
	//audio_source->PlayEvent("Click_backward");
	audio_source->PlayEvent("menu_select");
}

void LevelSelectionMenuController::UnlockLevels(size_t num_levels)
{
	for (size_t i = 0; i < level_thumbnails.size(); ++i)
	{
		level_thumbnails[i]->SetEnabled(i >= num_levels);
	}
}

void LevelSelectionMenuController::OnInspector(ImGuiContext* context)
{
	ImGui::SetCurrentContext(context);
	ShowDraggedObjects();

}

void LevelSelectionMenuController::InitPublicGameObjects()
{

	public_gameobjects.push_back(&level1_cursor_position);
	variable_names.push_back(GET_VARIABLE_NAME(level1_cursor_position));

	public_gameobjects.push_back(&level2_cursor_position);
	variable_names.push_back(GET_VARIABLE_NAME(level2_cursor_position));

	public_gameobjects.push_back(&level3_cursor_position);
	variable_names.push_back(GET_VARIABLE_NAME(level3_cursor_position));

	public_gameobjects.push_back(&back_button_game_object);
	variable_names.push_back(GET_VARIABLE_NAME(back));
	
	public_gameobjects.push_back(&character_menu_game_object);
	variable_names.push_back(GET_VARIABLE_NAME(character_menu_game_object));

	public_gameobjects.push_back(&level_selection_panel);
	variable_names.push_back(GET_VARIABLE_NAME(level_selection_panel));

	public_gameobjects.push_back(&audio_controller);
	variable_names.push_back(GET_VARIABLE_NAME(audio_controller));

	public_gameobjects.push_back(&back_cursor);
	variable_names.push_back(GET_VARIABLE_NAME(back_cursor));	
	
	public_gameobjects.push_back(&level_selection_cursor);
	variable_names.push_back(GET_VARIABLE_NAME(level_selection_cursor));

	public_gameobjects.push_back(&level1_thumbnail);
	variable_names.push_back(GET_VARIABLE_NAME(level1_thumbnail));

	public_gameobjects.push_back(&level2_thumbnail);
	variable_names.push_back(GET_VARIABLE_NAME(level2_thumbnail));

	public_gameobjects.push_back(&level3_thumbnail);
	variable_names.push_back(GET_VARIABLE_NAME(level3_thumbnail));

	public_gameobjects.push_back(&gamepad_icons);
	variable_names.push_back(GET_VARIABLE_NAME(gamepad_icons));

	public_gameobjects.push_back(&pc_icons);
	variable_names.push_back(GET_VARIABLE_NAME(pc_icons));

	for (unsigned int i = 0; i < public_gameobjects.size(); ++i)
	{
		name_gameobjects.push_back(is_object);
		go_uuids.push_back(0);
	}
}



