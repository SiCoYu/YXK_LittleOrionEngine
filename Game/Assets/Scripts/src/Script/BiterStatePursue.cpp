#include "BiterStatePursue.h"

#include "Component/ComponentAnimation.h"
#include "Component/ComponentAudioSource.h"

#include "Main/GameObject.h"

#include "Biter.h"

BiterStatePursue::BiterStatePursue(Biter* enemy) : EnemyState(enemy)
{
	strcpy(state_name, "Pursue");
}

void BiterStatePursue::OnStateEnter()
{
	if (!enemy->animation->IsOnState("Pursue"))
	{
		enemy->animation->ActiveAnimation("pursue");
	}
}

void BiterStatePursue::OnStateUpdate()
{
	enemy->AirMovementAI();

	if (enemy->is_attacking)
	{
		Exit(enemy->attack_state);
	}
	else if (enemy->is_charging)
	{
		Exit(enemy->charge_state);
	}
	else if (enemy->is_exploding)
	{
		Exit(enemy->explosion_state);
	}
}

void BiterStatePursue::OnStateExit()
{
}