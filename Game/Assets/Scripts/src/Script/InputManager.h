#ifndef  __INPUTMANAGER_H__
#define  __INPUTMANAGER_H__

#include "Script.h"
#include "Module/ModuleInput.h"

class WorldManager;

class InputManager : public Script
{
public:
	InputManager();
	~InputManager() = default;

	void Awake() override;
	void Start() override;
	void Update() override;

	void OnInspector(ImGuiContext*) override;

	//InputGetters
	bool GetTriggerAsButton(ControllerAxis type, PlayerID controller_id) const;
	float GetTrigger(ControllerAxis type, PlayerID controller_id) const;

	float GetVertical(PlayerID player_id) const;
	float GetHorizontal(PlayerID player_id) const;
	float GetVerticalRaw(PlayerID player_id) const;
	float GetHorizontalRaw(PlayerID player_id) const;

	bool GetControllerButtonDown(ControllerCode controller_code, ControllerID controller_id);
	bool GetControllerButton(ControllerCode controller_code, ControllerID controller_id);
	
	bool GetKeyDown(KeyCode key_code);
	bool GetKeyUp(KeyCode key_code);

	bool GetGameInput(const char* name, PlayerID player_id) const;
	bool GetGameInputDown(const char* name, PlayerID player_id) const;
	bool GetGameInputUp(const char* name, PlayerID player_id) const;

	bool GetDebugInputDown() const;

public:
	bool singleplayer_input = true;
	int total_game_controllers = 0;

	WorldManager* world_manager = nullptr;

};
extern "C" SCRIPT_API InputManager* InputManagerDLL(); //This is how we are going to load the script
#endif