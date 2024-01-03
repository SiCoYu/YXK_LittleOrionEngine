#include "TutorialHelper.h"

#include "Module/ModuleScene.h"
#include "Main/GameObject.h"
#include "Component/ComponentText.h"

#include "WorldManager.h"
#include "InputManager.h"
using TCP = LightBall::TutorialCheckPoints::CheckPointStatus;
namespace
{
	const size_t MAX_BOUNCES_REQUIRED = 3;
}
void TutorialHelper::InitDependencies(const ModuleScene & scene)
{
	input_manager = static_cast<InputManager*>(scene.GetGameObjectByName("Input Manager")->GetComponentScript("InputManager")->script);
	world = static_cast<WorldManager*>(scene.GetGameObjectByName("World Manager")->GetComponentScript("WorldManager")->script);
	dialogue_manager = static_cast<DialogueManager*>(scene.GetGameObjectByName("Dialogue Manager")->GetComponentScript("DialogueManager")->script);

	lightshield_tutorial_helper_ui = scene.GetGameObjectByName("Tutorial UI");
	if (lightshield_tutorial_helper_ui)
	{
		tutorial_hint_text = static_cast<ComponentText*>(lightshield_tutorial_helper_ui->children[0]->GetComponent(Component::ComponentType::UI_TEXT));
		tutorial_hint_text_p2 = static_cast<ComponentText*>(lightshield_tutorial_helper_ui->children[1]->GetComponent(Component::ComponentType::UI_TEXT));
		next_step_text = static_cast<ComponentText*>(lightshield_tutorial_helper_ui->children[2]->GetComponent(Component::ComponentType::UI_TEXT));
		bounce_counter_text = static_cast<ComponentText*>(lightshield_tutorial_helper_ui->children[3]->GetComponent(Component::ComponentType::UI_TEXT));
	}

}

bool TutorialHelper::CheckShieldTutorialDialogue(LightBall::TutorialCheckPoints& shield_tutorial, const DialogueManager::NextDialogueCallback& success, const DialogueManager::NextDialogueCallback& failure)
{
	bool dialogue = false;
	lightshield_tutorial_helper_ui->SetEnabled(false);
	//Check
	bool all_checkpoints_complete = true;
	all_checkpoints_complete &= shield_tutorial.lightball_throw == TCP::SUCCESS;
	all_checkpoints_complete &= shield_tutorial.lightball_bounces >= MAX_BOUNCES_REQUIRED;

	if (all_checkpoints_complete)
	{
		ResetTutorial(shield_tutorial);
		dialogue_manager->StartDialogue(DialogueID::LIGHTSHIELD_TUTORIAL_END, success);
		dialogue = true;
	}
	else 
	{
		ResetTutorial(shield_tutorial);
		dialogue_manager->StartDialogue(DialogueID::LIGHTSHIELD_TUTORIAL_REPEAT, failure);
		dialogue = true;
	}
	return dialogue;
}

void TutorialHelper::UpdateUI(LightBall::TutorialCheckPoints& shield_tutorial)
{
		lightshield_tutorial_helper_ui->SetEnabled(true);
		const bool lightball_in_progress = shield_tutorial.lightball_throw == TCP::IN_PROGRESS;
		const bool hint_is_for_player_1 = (shield_tutorial.player1_is_owner && lightball_in_progress) || !world->multiplayer || (!shield_tutorial.player1_is_owner && !lightball_in_progress);
		const bool show_key_hint = lightball_in_progress || shield_tutorial.parry_enabled;

		const std::string hint_p2 = lightball_in_progress ? "LB" : "Y";
		const std::string bounce_counter = "Bounces: "+ std::to_string(min(shield_tutorial.lightball_bounces, MAX_BOUNCES_REQUIRED))+ "/"+ std::to_string(MAX_BOUNCES_REQUIRED);
		std::string hint_p1;
		std::string next_step = lightball_in_progress ? "Throw the light ball" : "Get ready";
		if (!lightball_in_progress && shield_tutorial.parry_enabled)
		{
			next_step = "Use your shield";
		}
		if (input_manager->total_game_controllers <= 0 || (input_manager->total_game_controllers == 1 && world->multiplayer))
		{
			hint_p1 = lightball_in_progress ? "I" : "K";
		}
		else
		{
			hint_p1 = lightball_in_progress ? "LB" : "Y";
			
		}
		if (world->multiplayer)
		{
			next_step = hint_is_for_player_1 ? "Player 1 " + next_step : "Player 2 " + next_step;
		}
		tutorial_hint_text->SetText(hint_p1.c_str());
		tutorial_hint_text_p2->SetText(hint_p2.c_str());
		bounce_counter_text->SetText(bounce_counter.c_str());
		tutorial_hint_text->owner->SetEnabled(hint_is_for_player_1 && show_key_hint);
		tutorial_hint_text_p2->owner->SetEnabled(show_key_hint && world->multiplayer && !hint_is_for_player_1);
		bounce_counter_text->owner->SetEnabled(true);
		next_step_text->SetText(next_step.c_str());
}
void TutorialHelper::ResetTutorial(LightBall::TutorialCheckPoints& shield_tutorial)
{
	shield_tutorial.lightball_throw = TCP::IN_PROGRESS;
	shield_tutorial.player1_bounce = TCP::IN_PROGRESS;
	shield_tutorial.player2_bounce = TCP::IN_PROGRESS;
	shield_tutorial.parry_enabled = false;
	shield_tutorial.lightball_bounces = 0;
}
