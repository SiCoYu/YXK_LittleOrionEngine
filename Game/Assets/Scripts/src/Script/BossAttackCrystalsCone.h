#ifndef  __BOSSATTACKCRYSTALSCONE_H__
#define  __BOSSATTACKCRYSTALSCONE_H__

#include "BossAttack.h"

enum class PatternProjectilesState
{
	NONE,
	COMPUTING_POSITIONS,
	CHARGING_PROJECTILE,
	UPDATE_PROJECTILE
};

class BossController;

class BossAttackCrystalsCone : public BossAttack
{
public:
	BossAttackCrystalsCone(BossController* boss_controller);
	~BossAttackCrystalsCone() = default;

	void Update(float delta_time) override;
	void ExecuteAttack() override;

private:
	void ComputeConePosition();
	void GetNextConePositions(float position);

private:
	PatternProjectilesState pattern_projectile_state = PatternProjectilesState::NONE;
	float platform_height = 0.540f;

	//ATTACK MANAGEMENT
	float timer_between_rounds = 250.f;
	float current_time_between_rounds = 0.f;
	float danger_zone_radius = 2.5f;
	float current_time_charging_from_ground = 0.f;
	const float time_charging_from_ground = 400.f;
	float damage = 50.f;

	//PROPERTIES OF THE CONE
	uint32_t current_rows_of_projectiles_thrown = 0;
	uint32_t maximum_rows_to_spawn = 4;
	uint32_t maximum_projectiles = 3;
	float range_cone = 14.f;
	float starting_distance = 5.f;

	//ANGLE MANAGEMENT
	float angle_cone = 0.95f;
	float minimum_angle = 0.650f;
	float maximum_angle = 1.f;
	float angle = 0.95f;
	bool left = false;

	//POSITIONS OF THE CONE
	float second_row_distance = (range_cone - starting_distance) * 0.33 + starting_distance;
	float third_row_distance = (range_cone - starting_distance) * 0.66 + starting_distance;
	float3 current_position = float3::zero;
	float3 central_path = float3::zero;
	float3 right_path = float3::zero;
	float3 left_path = float3::zero;
	std::vector<float3> next_row_positions;

};
#endif