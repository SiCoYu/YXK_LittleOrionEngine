#include "BossAttackEarthquake.h"

#include "Component/ComponentTransform.h"
#include "Main/GameObject.h"

#include "BossController.h"
#include "PlayerController.h"

BossAttackEarthquake::BossAttackEarthquake(BossController* boss_controller) : BossAttack(boss_controller)
{

}

// Update is called once per frame
void BossAttackEarthquake::Update(float delta_time)
{
	if(!is_attack_executing)
	{
		return;
	}
	if (boss_controller->IsAnimationOnState("Earthquake") && boss_controller->GetAnimationPercentage() > 0.60f)
	{
		if (!attack_started)
		{
			initial_position = boss_controller->GetBonesGameObjectPosition(BossBone::RIGHT_HAND);
			attack_started = true;
			boss_controller->SetCameraTrauma(0.65f);

			//Effects
			boss_controller->SpawnEarthquakeEffect();

			boss_controller->ActiveSound(BossBone::RIGHT_FOREARM, "Play_Earthquake2");
		}
	}
	if (boss_controller->GetAnimationPercentage() > 0.70f) 
	{
		boss_controller->RotateBoss(PlayerController::WhichPlayer::PLAYER1);
	}
	if (attack_started)
	{
		current_time_attacking_earthquake += delta_time;
		current_earthquake_radius += 0.2f;
		boss_controller->UpdateEarthquakeEffect();
		if (!player1_already_hitted)
		{
			player1_already_hitted = boss_controller->ComputeDangerZoneCircle(initial_position,
				current_earthquake_radius,
				earthquake_damage, earthquake_hit_lenght, CrowdControl::STUN);
		}
		if (boss_controller->IsMultiplayer() && !player2_already_hitted)
		{
			player2_already_hitted = boss_controller->ComputeDangerZoneCircle(initial_position,
				current_earthquake_radius,
				earthquake_damage, earthquake_hit_lenght, CrowdControl::STUN);
		}
	}

	if(!swing_sound_triggered && boss_controller->GetAnimationPercentage() > 0.7f)
	{
		swing_sound_triggered = true;
		boss_controller->ActiveSound(BossBone::RIGHT_HAND, "Play_Byun");
	}

	if (current_time_attacking_earthquake >= max_time_attacking_earthquake)
	{
		current_time_attacking_earthquake = 0.f;
		current_earthquake_radius = 1.f;
		player1_already_hitted = false;
		player2_already_hitted = false;
		attack_started = false;
		swing_sound_triggered = false;
		is_attack_executing = false;
	}
}

void BossAttackEarthquake::ExecuteAttack()
{
	is_attack_executing = true;
	boss_controller->TriggerBossAnimation("earthquake");
	boss_controller->ActiveSound(BossBone::RIGHT_HAND, "Play_SwingBeforeEarthquake");
}
