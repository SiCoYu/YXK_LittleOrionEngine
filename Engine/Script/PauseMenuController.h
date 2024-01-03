#ifndef  __PAUSEMENUCONTROLLER_H__
#define  __PAUSEMENUCONTROLLER_H__

#include "Script.h"

class ComponentAudioSource;
class WorldManager;
class InputManager;

class PauseMenuController : public Script
{

public:
	PauseMenuController();
	~PauseMenuController() = default;

	void Awake() override;
	void Update() override;

	void Open();
	void Close();
	void SetupPatterns();

	void OnInspector(ImGuiContext* context) override;
	void InitPublicGameObjects() override;

private:
	bool game_paused = false;
	float time_scale = 0.0f;
	float OFFSET_X = 20.f;
	bool awaked = false;
	bool on_help = false;

	GameObject* pause_menu_panel = nullptr;
	GameObject* pause_cursor_go = nullptr;

	unsigned current = 0;
	std::vector<GameObject*> buttons;
	GameObject* resume_button = nullptr;
	GameObject* help_button = nullptr;
	GameObject* level_selection_button = nullptr;
	GameObject* main_menu_button = nullptr;

	GameObject* help_panel_1 = nullptr;
	GameObject* help_panel_2 = nullptr;
	
	GameObject* patterns = nullptr;
	float2 pattern_a_pos = float2::zero;
	float2 pattern_b_pos = float2::zero;

	GameObject* audio_controller = nullptr;
	ComponentAudioSource* audio_source = nullptr;
	WorldManager* world_manager = nullptr;

	const size_t MAIN_MENU_SCENE = 0;

	InputManager* input_manager = nullptr;
};
extern "C" SCRIPT_API PauseMenuController* PauseMenuControllerDLL(); //This is how we are going to load the script
#endif