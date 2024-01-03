#include "BiterStateExplosion.h"

#include "Component/ComponentAnimation.h"
#include "Component/ComponentAudioSource.h"
#include "Component/ComponentBillboard.h"
#include "Component/ComponentParticleSystem.h"

#include "Main/Application.h"
#include "Module/ModuleDebugDraw.h"
#include "Module/ModuleTime.h"

#include "Biter.h"
#include "DebugModeScript.h"

BiterStateExplosion::BiterStateExplosion(Biter* enemy) : EnemyState(enemy)
{
	strcpy_s(state_name, "Explosion");
}

void BiterStateExplosion::OnStateEnter()
{
	if (!enemy->animation->IsOnState("Explosion"))
	{
		enemy->animation->ActiveAnimation("explosion");
	}

	attacked = false;
	p1_attacked = false;
	p2_attacked = false;
}

void BiterStateExplosion::OnStateUpdate()
{
	enemy->ExplosionStatus();
	if (!attacked && enemy->explosion_current_time <= enemy->explosion_delay_time && !enemy->charge_explosion_purple_vfx->IsPlaying())
	{
		enemy->passive_particles->Disable();
		enemy->charge_explosion_purple_vfx->loop = true;
		enemy->charge_explosion_purple_vfx->Play();
		enemy->charge_explosion_blue_vfx->loop = true;
		enemy->charge_explosion_blue_vfx->Play();
		enemy->charge_explosion_black_vfx->loop = true;
		enemy->charge_explosion_black_vfx->Play();
		enemy->inner_circle_explosion_vfx->loop = true;
		//enemy->inner_circle_explosion_vfx->Play();
		
	}
	if (!attacked && enemy->explosion_current_time >= enemy->explosion_delay_time)
	{
		enemy->charge_explosion_purple_vfx->Disable();
		enemy->charge_explosion_blue_vfx->Disable();
		enemy->charge_explosion_black_vfx->Disable();
		enemy->inner_circle_explosion_vfx->Disable();
		attacked = true;
		enemy->attack_particles->active = false;
		enemy->attack_particles->Stop();

		if (enemy->PlayerInAttackArea(enemy->current_position + enemy->offset, enemy->explosion_radius))
		{
			enemy->DealDamageToPlayersInArea(enemy->current_position + enemy->offset, enemy->explosion_radius, enemy->explosion_damage, p1_attacked, p2_attacked, true);
		}

		enemy->is_exploding = false;
		enemy->passive_particles->Disable();
		enemy->explosion_particles->active = true;
		enemy->explosion_particles->Emit(450);
		enemy->explosion_particles_2->active = true;
		enemy->explosion_particles_2->Emit(300);
		enemy->shockwave_vfx->Play();
	}
	
	if (!enemy->is_exploding && enemy->explosion_current_time >= enemy->explosion_total_ability_time)
	{
		enemy->passive_particles->Enable();
		enemy->passive_particles->Play();
		enemy->explosion_particles->Stop();
		enemy->explosion_particles->active = false;
		enemy->explosion_particles_2->Stop();
		enemy->explosion_particles_2->active = false;
		enemy->shockwave_vfx->Disable();
		enemy->explosion_current_cooldown = enemy->explosion_cooldown;
		Exit(enemy->pursue_state);
	}

	if (enemy->debug->show_biter_explosion_attack)
	{
		enemy->App->debug_draw->RenderSphere(enemy->current_position + enemy->offset, enemy->explosion_radius);
	}
}

void BiterStateExplosion::OnStateExit()
{
	enemy->explosion_current_time = 0.0F;
	enemy->ApplySpecialAttackDelay();
}