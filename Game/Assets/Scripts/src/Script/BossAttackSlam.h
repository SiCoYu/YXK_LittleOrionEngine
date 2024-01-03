#ifndef  __BOSSATTACKSLAM_H__
#define  __BOSSATTACKSLAM_H__

#include "BossAttack.h"

class BossController;

class BossAttackSlam : public BossAttack
{
public:
	BossAttackSlam(BossController* boss_controller);
	~BossAttackSlam() = default;

	void Update(float delta_time) override;
	void ExecuteAttack() override;

private:
	const float fist_radius = 3.f;

	float slam_damage = 90.f;
	bool swing_sound_triggered = false;

	bool player1_already_hitted = false;
	bool player2_already_hitted = false;

};
#endif