#ifndef  __ALLYINTERACTIONSSCRIPT_H__
#define  __ALLYINTERACTIONSSCRIPT_H__
#include "Script.h"

#include "Dialogues.h"
#include <vector>
#include <unordered_map>
class CinematicCamera;
class Component;
class ComponentBillboard;
class ComponentCamera;
class ComponentText;
class DialogueManager;
class EnemyManager;
class EventManager;
class InputManager;
class LightBall;
class Player;
class WorldManager; 
class TutorialHelper;
class AllyInteractionsScript : public Script
{
public:

	enum class State
	{
		ANY,
		INTRO,
		INTRO_REPEAT,
		INTRO_TUTORIAL_SHOWED,
		LIGHTBALL_TRANSFORM,
		FINISH_LIGHTBALL_TRANSFORM,
		CORRUPTED_WATER_TUTORIAL,
		LEVEL1_FINISH_CELEBRATION,
		FINISH
	};

	AllyInteractionsScript() = default;
	~AllyInteractionsScript() = default;

	void Awake() override;
	void Start() override;
	void Update() override;

	void AllyLightBallUpdate();


	void OnInspector(ImGuiContext*) override;
	void InitPublicGameObjects();
	void Save(Config& config) const override;
	void Load(const Config& config) override;

	//API
	void ComplainDialogue();
	void ActivateLightBallDialogue(DialogueID dialogue);
	AllyInteractionsScript::State GetCurrentState() const;
	void StartLightShieldTutorial(GameObject* camera_position);
	void TransformIntoLightBall();
	bool ally_is_lightball = false;

private:
	void AllyHumanoidUpdate();

	void ActivateDialogue();
	void GoToNextPosition();
	void ChangePlayersMeshVisibility(bool enable);
	void ActivateUIHelpers(bool activate);
	void PlayAnimation(DialogueFeeling feeling);
	void RotateTowardsPlayer() const;
	void EnableAllyOnConversation(bool enable);
	void EnableCamera(bool enable);
	void LightBallModeCallback(DialogueFeeling feeling);

private:
	DialogueManager* dialogue_manager = nullptr;
	Player* player_1 = nullptr;
	Player* player_2 = nullptr;
	WorldManager* world = nullptr;
	InputManager* input_manager = nullptr;
	EventManager* event_manager = nullptr;
	EnemyManager* enemy_manager = nullptr;

	GameObject* keyboard_helper_ui = nullptr;
	GameObject* controller_helper_ui = nullptr;
	ComponentBillboard* arrow_indicator_helper = nullptr;
	LightBall* light_ball = nullptr;

	bool can_start_next_dialogue = true;
	bool firs_popup_showed = false;
	bool complaining = false;
	State current_state = State::INTRO;

	//Translation
	std::vector<GameObject*> positions;
	int current_position = -1;
	float3 lightBall_translation_reference;
	bool current_lightball_speed = 0.005f;
	float time_passed = 0.0f;
	//Camera view
	Component* player_1_mesh = nullptr;
	Component* player_2_mesh = nullptr;
	ComponentCamera* camera = nullptr;
	ComponentCamera* main_camera = nullptr;
	CinematicCamera* cinematic_camera = nullptr;

	//Shield tutorial
	bool shield_tutorial_started = false;
	void CheckShieldTutorial();

	//lightball

	ComponentBillboard* interior_texture_LB = nullptr;
	ComponentBillboard* exterior_texture_LB = nullptr;
	ComponentBillboard* small_texture_LB = nullptr;
	std::unique_ptr<TutorialHelper> tutorial_helper;


};
extern "C" SCRIPT_API AllyInteractionsScript* AllyInteractionsScriptDLL(); //This is how we are going to load the script
#endif