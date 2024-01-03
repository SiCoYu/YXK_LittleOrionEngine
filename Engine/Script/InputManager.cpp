#include "InputManager.h"

#include "Component/ComponentScript.h"

#include "Main/Application.h"
#include "Module/ModuleScene.h"


#include "imgui.h"

#include "WorldManager.h"


InputManager* InputManagerDLL()
{
	InputManager* instance = new InputManager();
	return instance;
}

InputManager::InputManager()
{
	
}

// Use this for initialization before Start()
void InputManager::Awake()
{
	GameObject* world_manager_go = App->scene->GetGameObjectByName("World Manager");
	ComponentScript* world_manager_component = world_manager_go->GetComponentScript("WorldManager");
	world_manager = static_cast<WorldManager*>(world_manager_component->script);
}

// Use this for initialization
void InputManager::Start()
{

}

// Update is called once per frame
void InputManager::Update()
{
	total_game_controllers = App->input->total_game_controllers;
	singleplayer_input = !world_manager->multiplayer;
}

// Use this for showing variables on inspector
void InputManager::OnInspector(ImGuiContext* context)
{
	//Necessary to be able to write with imgui
	ImGui::SetCurrentContext(context);
	ShowDraggedObjects();

}

bool InputManager::GetTriggerAsButton(ControllerAxis type, PlayerID controller_id) const
{
	return GetTrigger(type, controller_id) >= 0.95f;
}

float InputManager::GetTrigger(ControllerAxis type, PlayerID player_id) const
{

	// 1 game controller connected && singleplayer
	// Player 1 -> Keyboard || Game Controller
	if (total_game_controllers == 1 && singleplayer_input)
	{
		if (player_id == PlayerID::ONE)
		{
			return App->input->GetTriggerController(type, ControllerID::ONE);
		}
		else
		{
			return 0.f;
		}
	}
	// 1 game controller connected && multiplayer
	// Player 1 -> Keyboard
	// Player 2 -> Game Controller
	else if (total_game_controllers == 1 && !singleplayer_input)
	{
		if (player_id == PlayerID::ONE)
		{
			return 0.f;
		}
		else
		{
			return App->input->GetTriggerController(type, ControllerID::ONE);
		}
	}
	// 2 game controllers connected
	// Player 1 -> Game Controller 1
	// Player 2 -> Game Controller 2
	else if (total_game_controllers == 2)
	{
		return App->input->GetTriggerController(type, static_cast<ControllerID>(player_id));
	}

	return 0.f;
}

float InputManager::GetVertical(PlayerID player_id) const
{
	// No game controllers connected
	// Player 1 -> Keyboard
	if (total_game_controllers == 0)
	{
		if (player_id == PlayerID::ONE)
		{
			if (App->input->GetKey(KeyCode::W))
			{
				return -App->input->MAX_RAW_RANGE;
			}
			else if (App->input->GetKey(KeyCode::S))
			{
				return App->input->MAX_RAW_RANGE;
			}
		}
		else if (player_id == PlayerID::TWO)
		{
			return 0.f;
		}
	}

	// 1 game controller connected && singleplayer
	// Player 1 -> Keyboard || Game Controller
	else if (total_game_controllers == 1 && singleplayer_input)
	{
		if (player_id == PlayerID::ONE)
		{
			if (App->input->GetKey(KeyCode::W))
			{
				return -App->input->MAX_RAW_RANGE;
			}
			else if (App->input->GetKey(KeyCode::S))
			{
				return App->input->MAX_RAW_RANGE;
			}

			return App->input->GetAxisController(ControllerAxis::LEFT_JOYSTICK, ControllerID::ONE).y;
		}
		else if (player_id == PlayerID::TWO)
		{
			return 0.f;
		}
	}

	// 1 game controller connected && multiplayer
	// Player 1 -> Keyboard
	// Player 2 -> Game Controller
	else if (total_game_controllers == 1 && !singleplayer_input)
	{
		if (player_id == PlayerID::ONE)
		{
			if (App->input->GetKey(KeyCode::W))
			{
				return -App->input->MAX_RAW_RANGE;
			}
			else if (App->input->GetKey(KeyCode::S))
			{
				return App->input->MAX_RAW_RANGE;
			}
		}
		else if (player_id == PlayerID::TWO)
		{
			return App->input->GetAxisController(ControllerAxis::LEFT_JOYSTICK, ControllerID::ONE).y;
		}
	}

	// 2 game controllers connected
	// Player 1 -> Game Controller 1
	// Player 2 -> Game Controller 2
	else if (total_game_controllers == 2)
	{
		if (player_id == PlayerID::ONE)
		{
			return App->input->GetAxisController(ControllerAxis::LEFT_JOYSTICK, ControllerID::ONE).y;
		}
		else if (player_id == PlayerID::TWO)
		{
			return App->input->GetAxisController(ControllerAxis::LEFT_JOYSTICK, ControllerID::TWO).y;
		}
	}

	return 0.f;
}

float InputManager::GetHorizontal(PlayerID player_id) const
{
	// No game controllers connected
	// Player 1 -> Keyboard
	if (total_game_controllers == 0)
	{
		if (player_id == PlayerID::ONE)
		{
			if (App->input->GetKey(KeyCode::A))
			{
				return -App->input->MAX_RAW_RANGE;
			}
			else if (App->input->GetKey(KeyCode::D))
			{
				return App->input->MAX_RAW_RANGE;
			}
		}
		else if (player_id == PlayerID::TWO)
		{
			return 0.f;
		}
	}
	// 1 game controller connected && singleplayer
	// Player 1 -> Keyboard || Game Controller
	else if (total_game_controllers == 1 && singleplayer_input)
	{
		if (player_id == PlayerID::ONE)
		{
			if (App->input->GetKey(KeyCode::A))
			{
				return -App->input->MAX_RAW_RANGE;
			}
			else if (App->input->GetKey(KeyCode::D))
			{
				return App->input->MAX_RAW_RANGE;
			}

			return App->input->GetAxisController(ControllerAxis::LEFT_JOYSTICK, ControllerID::ONE).x;
		}
		else if (player_id == PlayerID::TWO)
		{
			return 0.f;
		}
	}

	// 1 game controller connected && multiplayer
	// Player 1 -> Keyboard
	// Player 2 -> Game Controller
	else if (total_game_controllers == 1 && !singleplayer_input)
	{
		if (player_id == PlayerID::ONE)
		{
			if (App->input->GetKey(KeyCode::A))
			{
				return -App->input->MAX_RAW_RANGE;
			}
			else if (App->input->GetKey(KeyCode::D))
			{
				return App->input->MAX_RAW_RANGE;
			}
		}
		else if (player_id == PlayerID::TWO)
		{
			return App->input->GetAxisController(ControllerAxis::LEFT_JOYSTICK, ControllerID::ONE).x;
		}
	}

	// 2 game controllers connected
	// Player 1 -> Game Controller 1
	// Player 2 -> Game Controller 2
	else if (total_game_controllers == 2)
	{
		if (player_id == PlayerID::ONE)
		{
			return App->input->GetAxisController(ControllerAxis::LEFT_JOYSTICK, ControllerID::ONE).x;
		}
		else if (player_id == PlayerID::TWO)
		{
			return App->input->GetAxisController(ControllerAxis::LEFT_JOYSTICK, ControllerID::TWO).x;
		}
	}

	return 0.f;
}

float InputManager::GetVerticalRaw(PlayerID player_id) const
{
	// No game controllers connected
	// Player 1 -> Keyboard
	if (total_game_controllers == 0)
	{
		if (player_id == PlayerID::ONE)
		{
			if (App->input->GetKey(KeyCode::W))
			{
				return -App->input->MAX_RAW_RANGE;
			}
			else if (App->input->GetKey(KeyCode::S))
			{
				return App->input->MAX_RAW_RANGE;
			}
		}
		else if (player_id == PlayerID::TWO)
		{
			return 0.f;
		}
	}

	// 1 game controller connected && singleplayer
	// Player 1 -> Keyboard || Game Controller
	else if (total_game_controllers == 1 && singleplayer_input)
	{
		if (player_id == PlayerID::ONE)
		{
			if (App->input->GetKey(KeyCode::W))
			{
				return -App->input->MAX_RAW_RANGE;
			}
			else if (App->input->GetKey(KeyCode::S))
			{
				return App->input->MAX_RAW_RANGE;
			}

			return App->input->GetAxisControllerRaw(ControllerAxis::LEFT_JOYSTICK, ControllerID::ONE).y;
		}
		else if (player_id == PlayerID::TWO)
		{
			return 0.f;
		}
	}

	// 1 game controller connected && multiplayer
	// Player 1 -> Keyboard
	// Player 2 -> Game Controller
	else if (total_game_controllers == 1 && !singleplayer_input)
	{
		if (player_id == PlayerID::ONE)
		{
			if (App->input->GetKey(KeyCode::W))
			{
				return -App->input->MAX_RAW_RANGE;
			}
			else if (App->input->GetKey(KeyCode::S))
			{
				return App->input->MAX_RAW_RANGE;
			}
		}
		else if (player_id == PlayerID::TWO)
		{
			return App->input->GetAxisControllerRaw(ControllerAxis::LEFT_JOYSTICK, ControllerID::ONE).y;
		}
	}

	// 2 game controllers connected
	// Player 1 -> Game Controller 1
	// Player 2 -> Game Controller 2
	else if (total_game_controllers == 2)
	{
		if (player_id == PlayerID::ONE)
		{
			return App->input->GetAxisControllerRaw(ControllerAxis::LEFT_JOYSTICK, ControllerID::ONE).y;
		}
		else if (player_id == PlayerID::TWO)
		{
			return App->input->GetAxisControllerRaw(ControllerAxis::LEFT_JOYSTICK, ControllerID::TWO).y;
		}
	}

	return 0.f;
}

float InputManager::GetHorizontalRaw(PlayerID player_id) const
{
	// No game controllers connected
	// Player 1 -> Keyboard
	if (total_game_controllers == 0)
	{
		if (player_id == PlayerID::ONE)
		{
			if (App->input->GetKey(KeyCode::A))
			{
				return -App->input->MAX_RAW_RANGE;
			}
			else if (App->input->GetKey(KeyCode::D))
			{
				return App->input->MAX_RAW_RANGE;
			}
		}
		else if (player_id == PlayerID::TWO)
		{
			return 0.f;
		}
	}
	// 1 game controller connected && singleplayer
	// Player 1 -> Keyboard || Game Controller
	else if (total_game_controllers == 1 && singleplayer_input)
	{
		if (player_id == PlayerID::ONE)
		{
			if (App->input->GetKey(KeyCode::A))
			{
				return -App->input->MAX_RAW_RANGE;
			}
			else if (App->input->GetKey(KeyCode::D))
			{
				return App->input->MAX_RAW_RANGE;
			}

			return App->input->GetAxisControllerRaw(ControllerAxis::LEFT_JOYSTICK, ControllerID::ONE).x;
		}
		else if (player_id == PlayerID::TWO)
		{
			return 0.f;
		}
	}

	// 1 game controller connected && multiplayer
	// Player 1 -> Keyboard
	// Player 2 -> Game Controller
	else if (total_game_controllers == 1 && !singleplayer_input)
	{
		if (player_id == PlayerID::ONE)
		{
			if (App->input->GetKey(KeyCode::A))
			{
				return -App->input->MAX_RAW_RANGE;
			}
			else if (App->input->GetKey(KeyCode::D))
			{
				return App->input->MAX_RAW_RANGE;
			}
		}
		else if (player_id == PlayerID::TWO)
		{
			return App->input->GetAxisControllerRaw(ControllerAxis::LEFT_JOYSTICK, ControllerID::ONE).x;
		}
	}

	// 2 game controllers connected
	// Player 1 -> Game Controller 1
	// Player 2 -> Game Controller 2
	else if (total_game_controllers == 2)
	{
		if (player_id == PlayerID::ONE)
		{
			return App->input->GetAxisControllerRaw(ControllerAxis::LEFT_JOYSTICK, ControllerID::ONE).x;
		}
		else if (player_id == PlayerID::TWO)
		{
			return App->input->GetAxisControllerRaw(ControllerAxis::LEFT_JOYSTICK, ControllerID::TWO).x;
		}
	}

	return 0.f;
}

bool InputManager::GetGameInput(const char * name, PlayerID player_id) const
{
	GameInput button = App->input->game_inputs[name];

	// No game controllers connected
	// Player 1 -> Keyboard
	if (total_game_controllers == 0)
	{
		if (player_id == PlayerID::ONE)
		{
			if (App->input->DetectedKeyboardInput(button, KeyState::REPEAT))
			{
				return true;
			}
		}
		else if (player_id == PlayerID::TWO)
		{
			return false;
		}
	}

	// 1 game controller connected && singleplayer
	// Player 1 -> Keyboard || Game Controller
	else if (total_game_controllers == 1 && singleplayer_input)
	{
		if (player_id == PlayerID::ONE)
		{
			if (App->input->DetectedKeyboardInput(button, KeyState::REPEAT))
			{
				return true;
			}

			if (App->input->DetectedGameControllerInput(button, KeyState::REPEAT, ControllerID::ONE))
			{
				return true;
			}
		}
		else if (player_id == PlayerID::TWO)
		{
			return false;
		}
	}

	// 1 game controller connected && multiplayer
	// Player 1 -> Keyboard
	// Player 2 -> Game Controller
	else if (total_game_controllers == 1 && !singleplayer_input)
	{
		if (player_id == PlayerID::ONE)
		{
			if (App->input->DetectedKeyboardInput(button, KeyState::REPEAT))
			{
				return true;
			}
		}
		else if (player_id == PlayerID::TWO)
		{
			if (App->input->DetectedGameControllerInput(button, KeyState::REPEAT, ControllerID::ONE))
			{
				return true;
			}
		}
	}

	// 2 game controllers connected
	// Player 1 -> Game Controller 1
	// Player 2 -> Game Controller 2
	else if (total_game_controllers == 2)
	{
		if (player_id == PlayerID::ONE)
		{
			if (App->input->DetectedGameControllerInput(button, KeyState::REPEAT, ControllerID::ONE))
			{
				return true;
			}
		}
		else if (player_id == PlayerID::TWO)
		{
			if (App->input->DetectedGameControllerInput(button, KeyState::REPEAT, ControllerID::TWO))
			{
				return true;
			}
		}
	}

	return false;
}

bool InputManager::GetGameInputDown(const char * name, PlayerID player_id) const
{
	GameInput button = App->input->game_inputs[name];

	// No game controllers connected
	// Player 1 -> Keyboard
	if (total_game_controllers == 0)
	{
		if (player_id == PlayerID::ONE)
		{
			if (App->input->DetectedKeyboardInput(button, KeyState::DOWN))
			{
				return true;
			}
		}
		else if (player_id == PlayerID::TWO)
		{
			return false;
		}
	}

	// 1 game controller connected && singleplayer
	// Player 1 -> Keyboard || Game Controller
	else if (total_game_controllers == 1 && singleplayer_input)
	{
		if (player_id == PlayerID::ONE)
		{
			if (App->input->DetectedKeyboardInput(button, KeyState::DOWN))
			{
				return true;
			}

			if (App->input->DetectedGameControllerInput(button, KeyState::DOWN, ControllerID::ONE))
			{
				return true;
			}
		}
		else if (player_id == PlayerID::TWO)
		{
			return false;
		}
	}

	// 1 game controller connected && multiplayer
	// Player 1 -> Keyboard
	// Player 2 -> Game Controller
	else if (total_game_controllers == 1 && !singleplayer_input)
	{
		if (player_id == PlayerID::ONE)
		{
			if (App->input->DetectedKeyboardInput(button, KeyState::DOWN))
			{
				return true;
			}
		}
		else if (player_id == PlayerID::TWO)
		{
			if (App->input->DetectedGameControllerInput(button, KeyState::DOWN, ControllerID::ONE))
			{
				return true;
			}
		}
	}

	// 2 game controllers connected
	// Player 1 -> Game Controller 1
	// Player 2 -> Game Controller 2
	else if (total_game_controllers == 2)
	{
		if (player_id == PlayerID::ONE)
		{
			if (App->input->DetectedGameControllerInput(button, KeyState::DOWN, ControllerID::ONE))
			{
				return true;
			}
		}
		else if (player_id == PlayerID::TWO)
		{
			if (App->input->DetectedGameControllerInput(button, KeyState::DOWN, ControllerID::TWO))
			{
				return true;
			}
		}
	}

	return false;
}

bool InputManager::GetGameInputUp(const char * name, PlayerID player_id) const
{
	GameInput button = App->input->game_inputs[name];

	// No game controllers connected
	// Player 1 -> Keyboard
	if (total_game_controllers == 0)
	{
		if (player_id == PlayerID::ONE)
		{
			if (App->input->DetectedKeyboardInput(button, KeyState::UP))
			{
				return true;
			}
		}
		else if (player_id == PlayerID::TWO)
		{
			return false;
		}
	}

	// 1 game controller connected && singleplayer
	// Player 1 -> Keyboard || Game Controller
	else if (total_game_controllers == 1 && singleplayer_input)
	{
		if (player_id == PlayerID::ONE)
		{
			if (App->input->DetectedKeyboardInput(button, KeyState::UP))
			{
				return true;
			}

			if (App->input->DetectedGameControllerInput(button, KeyState::UP, ControllerID::ONE))
			{
				return true;
			}
		}
		else if (player_id == PlayerID::TWO)
		{
			return false;
		}
	}

	// 1 game controller connected && multiplayer
	// Player 1 -> Keyboard
	// Player 2 -> Game Controller
	else if (total_game_controllers == 1 && !singleplayer_input)
	{
		if (player_id == PlayerID::ONE)
		{
			if (App->input->DetectedKeyboardInput(button, KeyState::UP))
			{
				return true;
			}
		}
		else if (player_id == PlayerID::TWO)
		{
			if (App->input->DetectedGameControllerInput(button, KeyState::UP, ControllerID::ONE))
			{
				return true;
			}
		}
	}

	// 2 game controllers connected
	// Player 1 -> Game Controller 1
	// Player 2 -> Game Controller 2
	else if (total_game_controllers == 2)
	{
		if (player_id == PlayerID::ONE)
		{
			if (App->input->DetectedGameControllerInput(button, KeyState::UP, ControllerID::ONE))
			{
				return true;
			}
		}
		else if (player_id == PlayerID::TWO)
		{
			if (App->input->DetectedGameControllerInput(button, KeyState::UP, ControllerID::TWO))
			{
				return true;
			}
		}
	}

	return false;
}

bool InputManager::GetDebugInputDown() const
{
	bool debug = App->input->GetKeyDown(KeyCode::F1);
	if (total_game_controllers > 0)
	{
		debug |= App->input->GetControllerButtonDown(ControllerCode::Back, ControllerID::ONE);
	}

	return debug;
}

bool InputManager::GetControllerButtonDown(ControllerCode controller_code, ControllerID controller_id)
{

	if(total_game_controllers > 1)
	{
		return App->input->GetControllerButtonDown(controller_code, controller_id);
	}
	else if (total_game_controllers > 0 && controller_id == ControllerID::ONE)
	{
		return App->input->GetControllerButtonDown(controller_code, controller_id);
	}

	return false;
}

bool InputManager::GetControllerButton(ControllerCode controller_code, ControllerID controller_id)
{

	if (total_game_controllers > 1)
	{
		return App->input->GetControllerButton(controller_code, controller_id);
	}
	else if (total_game_controllers > 0 && controller_id == ControllerID::ONE)
	{
		return App->input->GetControllerButton(controller_code, controller_id);
	}

	return false;
}

bool InputManager::GetKeyDown(KeyCode key_code)
{
	return App->input->GetKeyDown(key_code);
}

bool InputManager::GetKeyUp(KeyCode key_code)
{
	return App->input->GetKeyUp(key_code);
}

