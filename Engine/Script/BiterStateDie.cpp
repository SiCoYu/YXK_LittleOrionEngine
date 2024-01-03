#include "BiterStateDie.h"

#include "Component/ComponentAnimation.h"
#include "Component/ComponentAudioSource.h"
#include "Component/ComponentBillboard.h"
#include "Component/ComponentCollider.h"
#include "Component/ComponentParticleSystem.h"

#include "Module/ModuleDebugDraw.h"

#include "Main/Application.h"
#include "Module/ModuleTime.h"

#include "Biter.h"
#include "DebugModeScript.h"

BiterStateDie::BiterStateDie(Biter* enemy) : EnemyState(enemy)
{
	strcpy_s(state_name, "Die");
}

void BiterStateDie::OnStateEnter()
{
	if (!enemy->animation->IsOnState("Die"))
	{
		enemy->animation->ActiveAnimation("die");
	}
	//enemy->audio_source->PlayEvent("play_receive_hit");

	attacked = false;
	p1_attacked = false;
	p2_attacked = false;
}

void BiterStateDie::OnStateUpdate()
{
	enemy->ExplosionStatus();

	if (!attacked && enemy->explosion_current_time >= death_explosion_delay_time)
	{
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
		enemy->explosion_particles->Emit(400);
		enemy->explosion_particles_2->active = true;
		enemy->explosion_particles_2->Emit(300);
		enemy->shockwave_vfx->Play();
	}

	if (!enemy->is_exploding && enemy->explosion_current_time >= death_explosion_time)
	{
		//Explosion has ended and Biter has to disappear
		enemy->passive_particles->Enable();
		enemy->passive_particles->Play();
		enemy->collider->detect_collision = false;
		enemy->collider->active_physics = false;
		enemy->collider->Disable();
		enemy->collider->SetCollisionDetection();
		activate_timer = true;
	}

	if (activate_timer)
	{
		current_time += enemy->App->time->delta_time;

		if (current_time > (seconds_to_disappear/* * 1000*/))
		{
			activate_timer = false;
			current_time = 0;
			OnStateExit();
		}
	}

	if (enemy->debug->show_biter_explosion_attack)
	{
		enemy->App->debug_draw->RenderSphere(enemy->current_position + enemy->offset, enemy->explosion_radius);
	}
}

void BiterStateDie::OnStateExit()
{
	enemy->enemy_manager->KillEnemy(enemy);
	enemy->charge_current_cooldown = enemy->charge_cooldown;
	enemy->explosion_current_cooldown = enemy->explosion_cooldown;
}