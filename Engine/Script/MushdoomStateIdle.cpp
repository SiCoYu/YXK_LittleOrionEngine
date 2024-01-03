#include "MushdoomStateIdle.h"

#include "Component/ComponentAnimation.h"
#include "Component/ComponentAudioSource.h"

#include "Main/Application.h"
#include "Main/GameObject.h"

#include "Mushdoom.h"
#include "MushdoomBall.h"
#include "PlayerController.h"

MushdoomStateIdle::MushdoomStateIdle(Mushdoom* enemy) : EnemyState(enemy)
{
	strcpy_s(state_name, "Idle");
}

void MushdoomStateIdle::OnStateEnter()
{
	if (!enemy->animation->IsOnState("Idle"))
	{
		enemy->animation->ActiveAnimation("idle");
		enemy->audio_source->PlayEvent("play_idle");
	}

}

void MushdoomStateIdle::OnStateUpdate()
{
	if (enemy->PlayerInSight())
	{
		if (enemy->current_target != nullptr && enemy->current_target->is_alive)
		{
			enemy->RotateTowardsPlayer();
			enemy->PursueAI();
			if (enemy->is_moving)
			{
				Exit(enemy->pursue_state);
			}
		}
	}
}

void MushdoomStateIdle::OnStateExit()
{
	
}
