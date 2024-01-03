#ifndef  __BOSSATTACKAVALANCHE_H__
#define  __BOSSATTACKAVALANCHE_H__

#include "BossAttack.h"

class BossController;

class BossAttackAvalanche : public BossAttack
{
public:
	BossAttackAvalanche(BossController* boss_controller);
	~BossAttackAvalanche() = default;

	void Update(float delta_time) override;
	void ExecuteAttack() override;

private:
	float ExponencialFunction(float x) const;
	void SpawnRock(float exp_value);

private:
	float current_attacking_avalanche_time = 0.f;
	const float attacking_avalanche_time = 10000.f;

	float current_time_for_spawning_rock = 0.f;
	float current_time_for_spawning_above_player = 0.f;
	bool available_for_spawning_above = false;
	const float min_time_for_spawning_rock = 500.f;
	const float max_time_for_spawning_rock = 100.f;

	const float rocks_radius = 2.f;

	//Random
	std::mt19937 angle_generator;
	std::uniform_real_distribution<float> angle_distribution;

	std::mt19937 radius_generator;
	std::uniform_real_distribution<float> radius_distribution;

};
#endif