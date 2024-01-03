#include "BiterStateCharge.h"

#include "Component/ComponentAnimation.h"
#include "Component/ComponentAudioSource.h"
#include "Component/ComponentBillboard.h"
#include "Component/ComponentParticleSystem.h"

#include "Main/Application.h"
#include "Module/ModuleDebugDraw.h"

#include "Biter.h"
#include "DebugModeScript.h"

BiterStateCharge::BiterStateCharge(Biter* enemy) : EnemyState(enemy)
{
	strcpy_s(state_name, "Charge");
}

void BiterStateCharge::OnStateEnter()
{
	if (!enemy->animation->IsOnState("Charge"))
	{
		enemy->animation->ActiveAnimation("charge");
		//audio_source->PlayEvent("play_charge_biter");
	}

	p1_attacked = p2_attacked = false;
}

void BiterStateCharge::OnStateUpdate()
{
	enemy->ChargeStatus();

	float3 attack_center = enemy->current_position + enemy->offset;

	if ((!p1_attacked || !p2_attacked))
	{
		enemy->DealDamageToPlayersInArea(enemy->current_position, enemy->charge_radius, enemy->charge_damage, p1_attacked, p2_attacked, false);
	}

	if (!enemy->is_charging)
	{
		enemy->attack_particles->active = false;
		enemy->attack_particles->Stop();
		enemy->charge_current_cooldown = enemy->charge_cooldown;
		Exit(enemy->pursue_state);
	}

	if (enemy->debug->show_biter_charge_attack)
	{
		enemy->App->debug_draw->RenderSphere(attack_center, enemy->charge_radius);
	}
}

void BiterStateCharge::OnStateExit()
{
	enemy->ApplySpecialAttackDelay();
}