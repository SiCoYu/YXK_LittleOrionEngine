#include "MushdoomStateDie.h"

#include "Component/ComponentAnimation.h"
#include "Component/ComponentAudioSource.h"
#include "Component/ComponentCollider.h"

#include "Main/GameObject.h"
#include "Main/Application.h"
#include "Module/ModuleTime.h"

#include "Mushdoom.h"
#include "MushdoomBall.h"
#include "PlayerAttack.h"
#include "PlayerController.h"

MushdoomStateDie::MushdoomStateDie(Mushdoom* enemy) : EnemyState(enemy)
{
	strcpy(state_name, "Die");
}

void MushdoomStateDie::OnStateEnter()
{

	if (!enemy->animation->IsOnState("Die"))
	{
		enemy->animation->ActiveAnimation("die");
	}

	enemy->DeathVFX();

	enemy->collider->detect_collision = false;
	enemy->collider->active_physics = false;

	enemy->collider->Disable();
	enemy->collider->SetCollisionDetection();
}

void MushdoomStateDie::OnStateUpdate()
{
	enemy->KnockBack();

	if (!activate_timer && enemy->animation->IsOnState("Die") && enemy->animation->GetCurrentClipPercentatge() >= animation_end)
	{
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
}

void MushdoomStateDie::OnStateExit()
{
	enemy->enemy_manager->KillEnemy(enemy);
}
