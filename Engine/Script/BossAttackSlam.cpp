#include "BossAttackSlam.h"

#include "Component/ComponentTransform.h"
#include "Main/GameObject.h"

#include "BossController.h"

BossAttackSlam::BossAttackSlam(BossController* boss_controller) : BossAttack(boss_controller)
{

}


// Update is called once per frame
void BossAttackSlam::Update(float delta_time)
{
	if (!is_attack_executing)
	{
		return;
	}

	if (!player1_already_hitted)
	{
		bool hit_by_hand = boss_controller->ComputeDangerZone(boss_controller->GetBonesGameObjectPosition(BossBone::RIGHT_HAND), fist_radius, slam_damage, CrowdControl::KNOCKUP);
		bool hit_by_forearm = boss_controller->ComputeDangerZone(boss_controller->GetBonesGameObjectPosition(BossBone::RIGHT_FOREARM), fist_radius, slam_damage, CrowdControl::KNOCKUP);
	
		player1_already_hitted = hit_by_hand || hit_by_forearm;
	}

	if (boss_controller->IsMultiplayer() && !player2_already_hitted)
	{
		bool hit_by_hand = boss_controller->ComputeDangerZone(boss_controller->GetBonesGameObjectPosition(BossBone::RIGHT_HAND), fist_radius, slam_damage, CrowdControl::KNOCKUP, 1);
		bool hit_by_forearm = boss_controller->ComputeDangerZone(boss_controller->GetBonesGameObjectPosition(BossBone::RIGHT_FOREARM), fist_radius, slam_damage, CrowdControl::KNOCKUP, 1);
	
		player2_already_hitted = hit_by_hand || hit_by_forearm;
	}

	if (!swing_sound_triggered && boss_controller->IsAnimationOnState("Slam_Right") && boss_controller->GetAnimationPercentage() > 0.65f)
	{
		swing_sound_triggered = true;
		boss_controller->ActiveSound(BossBone::RIGHT_HAND, "Play_BossSwing");
	}

	if (boss_controller->IsAnimationOnState("Slam_Right") && boss_controller->IsAnimationOver())
	{
		//Trigger slam effect
		boss_controller->SpawnSlamEffect(BossBone::RIGHT_HAND);
		boss_controller->SetCameraTrauma(0.55f);
		boss_controller->ActiveSound(BossBone::RIGHT_FOREARM, "Play_BossSlam");
		player1_already_hitted = false;
		player2_already_hitted = false;
		swing_sound_triggered = false;
		is_attack_executing = false;
	}

}

void BossAttackSlam::ExecuteAttack()
{
	is_attack_executing = true;
	//Active animation
	boss_controller->TriggerBossAnimation("slam_right");
	boss_controller->ActiveSound(BossBone::RIGHT_HAND, "Play_SwingBeforeSlam");
	boss_controller->SetEnableVulnerableZone(BossBone::RIGHT_FOREARM, true, true);
	boss_controller->SetEnableVulnerableZone(BossBone::RIGHT_FINGER, true, true);
}
