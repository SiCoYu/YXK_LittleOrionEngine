#ifndef  __BOSSATTACKEARTHQUAKE_H__
#define  __BOSSATTACKEARTHQUAKE_H__

#include "BossAttack.h"

class BossController;

class BossAttackEarthquake : public BossAttack
{
public:
	BossAttackEarthquake(BossController* boss_controller);
	~BossAttackEarthquake() = default;

	void Update(float delta_time) override;
	void ExecuteAttack() override;

private:
	float current_time_attacking_earthquake = 0.f;
	float current_earthquake_radius = 1.f;
	const float max_time_attacking_earthquake = 3000.f;
	float earthquake_damage = 150.f;
	float earthquake_hit_lenght = 0.5f;
	bool player1_already_hitted = false;
	bool player2_already_hitted = false;

	float3 initial_position = float3::zero;
	bool attack_started = false;
	bool swing_sound_triggered = false;

};
#endif