#include "UIMainMenuInputController.h"

#include "Main/Application.h"
#include "Module/ModuleInput.h"

#include "InputManager.h"


bool UIMainMenuInputController::ConfirmMovedUp(InputManager* input)
{
	if(input->total_game_controllers == 0)
	{
		return input->GetKeyDown(KeyCode::UpArrow) || input->GetKeyDown(KeyCode::W);
	}

	return (input->GetKeyDown(KeyCode::UpArrow) || input->GetKeyDown(KeyCode::W) || input->GetControllerButtonDown(ControllerCode::UpDpad, ControllerID::ONE));
}

bool UIMainMenuInputController::ConfirmMovedDown(InputManager* input)
{
	if (input->total_game_controllers == 0)
	{
		return input->GetKeyDown(KeyCode::DownArrow) || input->GetKeyDown(KeyCode::S);
	}
	return (input->GetKeyDown(KeyCode::DownArrow) || input->GetKeyDown(KeyCode::S) || input->GetControllerButtonDown(ControllerCode::DownDpad, ControllerID::ONE));
}

bool UIMainMenuInputController::ConfirmMovedRight(InputManager* input)
{
	if (input->total_game_controllers == 0)
	{
		return input->GetKeyDown(KeyCode::RightArrow) || input->GetKeyDown(KeyCode::D);
	}
	return (input->GetKeyDown(KeyCode::RightArrow) || input->GetKeyDown(KeyCode::D) || input->GetControllerButtonDown(ControllerCode::RightDpad, ControllerID::ONE));
}

bool UIMainMenuInputController::ConfirmMovedLeft(InputManager* input)
{
	if (input->total_game_controllers == 0)
	{
		return input->GetKeyDown(KeyCode::LeftArrow) || input->GetKeyDown(KeyCode::A);
	}
	return (input->GetKeyDown(KeyCode::LeftArrow) || input->GetKeyDown(KeyCode::A) || input->GetControllerButtonDown(ControllerCode::LeftDpad, ControllerID::ONE));
}

bool UIMainMenuInputController::ComfirmButtonPressed(InputManager* input)
{
	if (input->total_game_controllers == 0)
	{
		return input->GetKeyDown(KeyCode::Space);
	}
	return (input->GetKeyDown(KeyCode::Space) || input->GetControllerButtonDown(ControllerCode::A, ControllerID::ONE));
}

bool UIMainMenuInputController::CancelButtonPressed(InputManager* input)
{
	if (input->total_game_controllers == 0)
	{
		return input->GetKeyDown(KeyCode::Escape);
	}
	return (input->GetKeyDown(KeyCode::Escape) || input->GetControllerButtonDown(ControllerCode::B, ControllerID::ONE));
}

bool UIMainMenuInputController::StartButtonPressed(InputManager* input)
{
	if (input->total_game_controllers == 0)
	{
		return input->GetKeyDown(KeyCode::Escape);
	}
	return (input->GetKeyDown(KeyCode::Escape) || input->GetControllerButtonDown(ControllerCode::Start, ControllerID::ONE));
}

