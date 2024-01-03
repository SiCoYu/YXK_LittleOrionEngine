#include "MushdoomStatePursue.h"

#include "Component/ComponentAnimation.h"
#include "Component/ComponentAudioSource.h"

#include "Main/GameObject.h"

#include "Mushdoom.h"

MushdoomStatePursue::MushdoomStatePursue(Mushdoom* enemy) : EnemyState(enemy)
{
	strcpy(state_name, "Pursue");
}

void MushdoomStatePursue::OnStateEnter()
{
	if (!enemy->animation->IsOnState("Pursue"))
	{
		enemy->animation->ActiveAnimation("pursue");
	}
}

void MushdoomStatePursue::OnStateUpdate()
{
	float current_percentage = enemy->animation->GetCurrentClipPercentatge();

	// Animation has 60 frames, steps each 15 frames (1, 15, 30, 45)
	if ((int)(current_percentage * 100) % 25 == 0 && current_percentage != 1)
	{
		enemy->audio_source->PlayEvent("play_footstep");
	}

	enemy->PursueAI();

	if (enemy->has_ended_spawn && enemy->ranged_attack)
	{
		enemy->NewThrow();
	}
	else if (!enemy->is_moving)
	{
		if(enemy->has_ended_spawn)
		{
			enemy->enemy_manager->NewRequestAttack(enemy);
		}
		enemy->BattleCircleAI();
	}
	if (enemy->is_attacking)
	{
		Exit(enemy->attack_state);
	}
	else if (enemy->is_throwing)
	{
		Exit(enemy->scream_state);
	}
}

void MushdoomStatePursue::OnStateExit()
{
	enemy->is_moving = false;
}