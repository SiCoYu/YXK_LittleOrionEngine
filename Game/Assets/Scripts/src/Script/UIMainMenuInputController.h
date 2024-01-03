#ifndef  __UIMAINMENUINPUTCONTROLLER_H__
#define  __UIMAINMENUINPUTCONTROLLER_H__

class InputManager;

class UIMainMenuInputController
{
public:
	UIMainMenuInputController() = default;
	~UIMainMenuInputController() = default;

	static bool ConfirmMovedUp(InputManager* input);
	static bool ConfirmMovedDown(InputManager* input);
	static bool ConfirmMovedRight(InputManager* input);
	static bool ConfirmMovedLeft(InputManager* input);
	static bool ComfirmButtonPressed(InputManager* input);
	static bool CancelButtonPressed(InputManager* input);
	static bool StartButtonPressed(InputManager* input);

}; 
#endif
