#include "BiterStateIdle.h"

#include "Component/ComponentAnimation.h"
#include "Component/ComponentAudioSource.h"

#include "Biter.h"
#include "PlayerController.h"

BiterStateIdle::BiterStateIdle(Biter* enemy) : EnemyState(enemy)
{
	strcpy_s(state_name, "Idle");
}

void BiterStateIdle::OnStateEnter()
{
	if (!enemy->animation->IsOnState("Idle"))
	{
		enemy->animation->ActiveAnimation("idle");
	}
}

void BiterStateIdle::OnStateUpdate()
{
	if (enemy->PlayerInSight())
	{
		if (enemy->current_target != nullptr && enemy->current_target->is_alive)
		{
			Exit(enemy->pursue_state);
		}
	}
}

void BiterStateIdle::OnStateExit()
{
}