#ifndef  __BOSSATTACKCRYSTALSGUIDED_H__
#define  __BOSSATTACKCRYSTALSGUIDED_H__

#include "BossAttack.h"

enum class GuidedProjectilesState
{
	NONE,
	COMPUTING_NEW_POSITION,
	CHARGING_PROJECTILE,
	UPDATE_PROJECTILE
};

class BossController;

class BossAttackCrystalsGuided : public BossAttack
{
public:
	BossAttackCrystalsGuided(BossController* boss_controller);
	~BossAttackCrystalsGuided() = default;

	void Update(float delta_time) override;
	void ExecuteAttack() override;

	std::string GetCurrentGuidedProjectilesState() const;

private:
	GuidedProjectilesState guided_projectile_state = GuidedProjectilesState::NONE;
	uint32_t current_number_of_projectiles_thrown = 0;
	const uint32_t number_of_projectiles_thrown = 4;
	float3 current_projectile_position = float3::zero;
	float3 current_projectile_position_multiplayer = float3::zero;
	bool is_charging = false;
	float current_time_charging_from_ground = 0.f;
	const float time_charging_from_ground = 400.f;
	bool player1_hit = false;
	bool player2_hit = false;
	bool end = false;
	
	std::mt19937 guided_projectile_position_generator;
	std::uniform_int_distribution<> attack_guided_projectile_position_distribution;

};
#endif