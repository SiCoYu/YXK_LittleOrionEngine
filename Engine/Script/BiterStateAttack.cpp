#include "BiterStateAttack.h"

#include "Component/ComponentAnimation.h"
#include "Component/ComponentAudioSource.h"
#include "Component/ComponentBillboard.h"
#include "Component/ComponentParticleSystem.h"

#include "Main/Application.h"
#include "Module/ModuleDebugDraw.h"

#include "Biter.h"
#include "DebugModeScript.h"

BiterStateAttack::BiterStateAttack(Biter* enemy) : EnemyState(enemy)
{
	strcpy_s(state_name, "Attack");
}

void BiterStateAttack::OnStateEnter()
{
	if (!enemy->animation->IsOnState("Attack"))
	{
		enemy->animation->ActiveAnimation("attack");
	}

	enemy->is_attacking = true;
	attacked = p1_attacked = p2_attacked = false;
}

void BiterStateAttack::OnStateUpdate()
{
	enemy->AttackPosition();

	float current_percentage = enemy->animation->GetCurrentClipPercentatge();
	float3 attack_center = enemy->current_position + (enemy->current_velocity.Normalized() * enemy->bite_distance) + enemy->offset;
	
	if (current_percentage >= init_attack_percentage && !enemy->warning_basic_attack_vfx->IsPlaying())
	{
		//enemy->warning_basic_attack_vfx->Play();
	}
	if (current_percentage >= init_attack_percentage && current_percentage <= final_attack_percentage && !attacked)
	{
		//enemy->warning_basic_attack_vfx->Disable();
		if (enemy->PlayerInAttackArea(attack_center, enemy->bite_radius))
		{
			enemy->DealDamageToPlayersInArea(attack_center, enemy->bite_radius, enemy->attack_damage, p1_attacked, p2_attacked, false);
			attacked = true;
		}	
	}
	else if (current_percentage >= animation_end)
	{
		Exit(enemy->pursue_state);
	}

	if (enemy->debug->show_biter_basic_attack)
	{
		enemy->App->debug_draw->RenderSphere(attack_center, enemy->bite_radius);
	}
	
}

void BiterStateAttack::OnStateExit()
{
	enemy->is_attacking = false;
	enemy->attack_particles->active = false;
	enemy->attack_particles->Stop();
}