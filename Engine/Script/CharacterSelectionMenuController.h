#ifndef  __CHARATERSELECTIONMENUCONTROLLER_H
#define  __CHARATERSELECTIONMENUCONTROLLER_H

#include "Script.h"

#include "Animation/Tween/LOTween.h"
#include "Animation/Tween/Tween.h"
#include "Animation/Tween/TweenSequence.h"


class ComponentAudioSource;
class ComponentButton;
class ComponentTransform2D;

class InputManager;
class LevelSelectionMenuController;
class MainMenuController;
class WorldManager;

class CharacterSelectionMenuController : public Script
{
public:

	enum class CharacterSelectionStatus
	{
		BACK_HOVERED,
		SELECTING_PLAYER,
		PLAYER_SELECTED
	};

	CharacterSelectionMenuController();
	~CharacterSelectionMenuController() = default;

	void Awake() override;
	void Update() override;

	void OnInspector(ImGuiContext * context) override;
	void InitPublicGameObjects() override;

	void SwitchCharacterSelection();
	void SetCharacterSelection(int position);
	void SwitchMultiplayer(bool enabled);

	void UpdateCursorsColors();

	void Open();
	void Close();

private:
	unsigned current = 0;

	CharacterSelectionStatus character_selection_status = CharacterSelectionStatus::SELECTING_PLAYER;
	InputManager* input_manager = nullptr;
	bool player1_choice = false;
	bool multiplayer = false;

	bool enabled = false;
	bool just_opened = false;

	GameObject* character_selection_panel = nullptr;

	GameObject* main_menu_game_object = nullptr;
	MainMenuController* main_menu_controller = nullptr;

	GameObject* level_selection_game_object = nullptr;
	LevelSelectionMenuController* level_selection_controller = nullptr;

	GameObject* male_character_position = nullptr;
	ComponentButton* character_selector_1_button = nullptr;

	GameObject* female_character_position = nullptr;
	ComponentButton* character_selector_2_button = nullptr;

	GameObject* character_selector1 = nullptr;
	GameObject* character_selector2 = nullptr;

	GameObject* back_button_game_object = nullptr;
	ComponentButton* back_button = nullptr;

	GameObject* cursor;
	ComponentTransform2D* cursor_transform;

	GameObject* world_manager_game_object = nullptr;
	WorldManager* world_manager = nullptr;

	GameObject* player2_press_start_text = nullptr;
	GameObject* press_space_continue_text = nullptr;

	GameObject* audio_controller = nullptr;
	ComponentAudioSource* audio_source = nullptr;

	GameObject* character1_selection_light = nullptr;
	GameObject* character2_selection_light = nullptr;

	GameObject* gamepad_icons = nullptr;
	GameObject* pc_icons = nullptr;

	TweenSequence* selection_sequence;
	TweenSequence* selection_sequence2;
	
	
};
extern "C" SCRIPT_API CharacterSelectionMenuController* CharacterSelectionMenuControllerDLL(); //This is how we are going to load the script
#endif

