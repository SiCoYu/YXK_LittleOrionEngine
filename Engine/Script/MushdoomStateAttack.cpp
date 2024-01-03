#include "MushdoomStateAttack.h"

#include "Component/ComponentAnimation.h"
#include "Component/ComponentAudioSource.h"

#include "Mushdoom.h"

MushdoomStateAttack::MushdoomStateAttack(Mushdoom* enemy) : EnemyState(enemy)
{
	strcpy_s(state_name, "Attack");
}

void MushdoomStateAttack::OnStateEnter()
{
	if (!enemy->animation->IsOnState("Attack"))
	{
		enemy->animation->ActiveAnimation("attack");
		enemy->audio_source->PlayEvent("play_make_attack");
	}

	enemy->is_attacking = true;
	attacked = true;
}

void MushdoomStateAttack::OnStateUpdate()
{
	float current_percentage = enemy->animation->GetCurrentClipPercentatge();

	if (current_percentage >= attack_damage_start && current_percentage <= attack_damage_end && enemy->PlayerInAttackRange() && attacked)
	{
		enemy->Attack();
		attacked = false;
		enemy->audio_source->PlayEvent("play_receive_hit");
	}
	else if (current_percentage >= animation_end)
	{
		Exit(enemy->pursue_state);
	}
}

void MushdoomStateAttack::OnStateExit()
{
	enemy->is_attacking = false;
	enemy->CancelAttack();
	enemy->LeaveAttackRange();
}
