#include "BossAttackAvalanche.h"

#include "Component/ComponentTransform.h"
#include "Main/GameObject.h"

#include "BossController.h"


namespace
{
	const float total_time_for_spawning_above_player = 2000.f;
}

BossAttackAvalanche::BossAttackAvalanche(BossController* boss_controller) : BossAttack(boss_controller)
{
	angle_generator = std::mt19937(random_device());
	radius_generator = std::mt19937(random_device());

	angle_distribution = std::uniform_real_distribution<float>(35.f, 170.0f);
	radius_distribution = std::uniform_real_distribution<float>(5.f, 15.0f);
}


// Update is called once per frame
void BossAttackAvalanche::Update(float delta_time)
{
	if (current_attacking_avalanche_time >= attacking_avalanche_time)
	{
		if (boss_controller->IsAvalancheDone())
		{
			current_attacking_avalanche_time = 0.f;
			current_time_for_spawning_rock = 0.f;
			is_attack_executing = false;
		}
	}
	else
	{
		if (boss_controller->IsAnimationOnState("Avalanche_Cast")) 
		{
			current_time_for_spawning_above_player += delta_time;
			current_time_for_spawning_rock += delta_time;
			current_attacking_avalanche_time += delta_time;

			float time_percentatge = current_attacking_avalanche_time / attacking_avalanche_time;
			float exp_value = ExponencialFunction(time_percentatge);

			float time_for_spawning = math::Lerp(min_time_for_spawning_rock, max_time_for_spawning_rock, exp_value);

			if (current_time_for_spawning_rock >= time_for_spawning)
			{
				current_time_for_spawning_rock = 0.f;
				SpawnRock(exp_value);
			}

			if(current_time_for_spawning_above_player >= total_time_for_spawning_above_player && !available_for_spawning_above)
			{
				available_for_spawning_above = true;
			}
		}
	}

	boss_controller->UpdateRocks();
}

void BossAttackAvalanche::ExecuteAttack()
{
	is_attack_executing = true;
}

float BossAttackAvalanche::ExponencialFunction(float x) const
{
	return math::Pow(2.0f, x) - 1.f;
}

void BossAttackAvalanche::SpawnRock(float exp_value)
{
	bool valid = false;

	float3 rock_target_position;
	unsigned attempts = 0;

	bool rock_already_spawn = false;

	if(available_for_spawning_above)
	{
		//First try to spawn a rock above player
		rock_target_position = boss_controller->GetPlayerPosition(0);
		bool valid_above_player_1 = boss_controller->CheckIfRockIsValid(rock_target_position);
		bool valid_above_player_2 = false;

		//Same on multiplayer
		if(boss_controller->IsMultiplayer() && !valid_above_player_1)
		{
			rock_target_position = boss_controller->GetPlayerPosition(1);
			valid_above_player_2 = boss_controller->CheckIfRockIsValid(rock_target_position);
		}

		rock_already_spawn = valid_above_player_1 || valid_above_player_2;

		if(rock_already_spawn)
		{
			available_for_spawning_above = false;
			current_time_for_spawning_above_player = 0.f;
		}
	}

	while (!rock_already_spawn && !valid && attempts < 100)
	{
		rock_target_position = boss_controller->owner->transform.GetGlobalTranslation();

		float random_angle = angle_distribution(angle_generator);
		float random_radius = radius_distribution(radius_generator);

		rock_target_position.x += math::Cos(math::DegToRad(random_angle)) * random_radius;
		rock_target_position.z += math::Sin(math::DegToRad(random_angle)) * random_radius;
		rock_target_position.y = boss_controller->ground_y_position;

		valid = boss_controller->CheckIfRockIsValid(rock_target_position);
		++attempts;
	}

	boss_controller->SpawnRock(rock_target_position, exp_value);

}
