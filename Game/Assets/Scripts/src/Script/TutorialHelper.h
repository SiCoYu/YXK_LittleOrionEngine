#pragma once

#include "DialogueManager.h"
#include "LightBall.h"
class GameObject;
class ComponentText;
class WorldManager;
class InputManager;
class ModuleScene;
class TutorialHelper
{
public:
	TutorialHelper() = default;
	~TutorialHelper() = default;

	void InitDependencies(const ModuleScene &scene);
	void UpdateUI(LightBall::TutorialCheckPoints & shield_tutorial);
	bool CheckShieldTutorialDialogue(LightBall::TutorialCheckPoints & shield_tutorial, const DialogueManager::NextDialogueCallback & success, const DialogueManager::NextDialogueCallback & failure);
	void ResetTutorial(LightBall::TutorialCheckPoints & shield_tutorial);

	private:
	GameObject* lightshield_tutorial_helper_ui = nullptr;
	ComponentText* tutorial_hint_text = nullptr;
	ComponentText* tutorial_hint_text_p2 = nullptr;
	ComponentText* bounce_counter_text = nullptr;
	ComponentText* next_step_text = nullptr;

	DialogueManager* dialogue_manager = nullptr;
	WorldManager* world = nullptr;
	InputManager* input_manager = nullptr;
};

