#ifndef  _LEVELSELECTIONMENUCONTROLLER_H_
#define  _LEVELSELECTIONMENUCONTROLLER_H_

#include "Script.h"

class ComponentAudioSource;
class ComponentButton;
class ComponentTransform2D;
class CharacterSelectionMenuController;
class InputManager;
class WorldManager;

class LevelSelectionMenuController : public Script
{

public:
	LevelSelectionMenuController();
	~LevelSelectionMenuController() = default;

	void Awake() override;
	void Update() override;

	void OnInspector(ImGuiContext * context) override;
	void InitPublicGameObjects() override;

	void Open();
	void Close();

	void UnlockLevels(size_t num_levels);

private:
	bool selecting_level = false;
	bool enabled = false;
	bool just_opened = false;

	GameObject* level_selection_panel = nullptr;

	GameObject* character_menu_game_object = nullptr;
	CharacterSelectionMenuController* character_menu_controller = nullptr;

	GameObject* level_selection_cursor = nullptr;
	unsigned current = 0;
	std::vector<ComponentTransform2D*> cursor_positions;
	GameObject* level1_cursor_position = nullptr;
	GameObject* level2_cursor_position = nullptr;
	GameObject* level3_cursor_position = nullptr;

	std::vector<GameObject*> level_thumbnails;
	GameObject* level1_thumbnail = nullptr;
	GameObject* level2_thumbnail = nullptr;
	GameObject* level3_thumbnail = nullptr;

	GameObject* back_button_game_object = nullptr;
	ComponentButton* back_button = nullptr;

	GameObject* back_cursor = nullptr;

	GameObject* audio_controller = nullptr;
	ComponentAudioSource* audio_source = nullptr;

	GameObject* world_manager_game_object = nullptr;
	WorldManager* world_manager = nullptr;

	GameObject* gamepad_icons = nullptr;
	GameObject* pc_icons = nullptr;

	const size_t NUM_UNLOCKED_LEVELS = 3;

	InputManager* input_manager = nullptr;

};
extern "C" SCRIPT_API LevelSelectionMenuController* LevelSelectionMenuControllerDLL(); //This is how we are going to load the script
#endif


