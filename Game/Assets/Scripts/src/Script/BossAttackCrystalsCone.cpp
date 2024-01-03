#include "BossAttackCrystalsCone.h"

#include "Component/ComponentTransform.h"
#include "Main/GameObject.h"

#include "BossController.h"

BossAttackCrystalsCone::BossAttackCrystalsCone(BossController* boss_controller) : BossAttack(boss_controller)
{

}


// Update is called once per frame
void BossAttackCrystalsCone::Update(float delta_time)
{
	if(!is_attack_executing)
	{
		return;
	}

	switch (pattern_projectile_state)
	{
	case PatternProjectilesState::NONE:
		if (boss_controller->IsAnimationOnState("Stay_Punch_Left") && boss_controller->GetAnimationPercentage() > 0.7f)
		{
			pattern_projectile_state = PatternProjectilesState::COMPUTING_POSITIONS;
			boss_controller->TriggerBossAnimation("idle_two");
			boss_controller->ActiveSound(BossBone::LEFT_HAND, "Play_Smash2");
			boss_controller->ActiveSound(BossBone::LEFT_HAND, "Play_ConeSound");

		}
		break;

	case PatternProjectilesState::COMPUTING_POSITIONS:
		ComputeConePosition();
		pattern_projectile_state = PatternProjectilesState::CHARGING_PROJECTILE;
		break;

	case PatternProjectilesState::CHARGING_PROJECTILE:
		if (current_rows_of_projectiles_thrown == 0)
		{
			GetNextConePositions(starting_distance);
		}
		else if (current_rows_of_projectiles_thrown == 1)
		{
			GetNextConePositions(second_row_distance);
		}
		else if (current_rows_of_projectiles_thrown == 2)
		{
			GetNextConePositions(third_row_distance);

		}
		else if (current_rows_of_projectiles_thrown == 3)
		{
			GetNextConePositions(range_cone);
		}
		for (size_t i = 0; i < maximum_projectiles; ++i)
		{
			boss_controller->SpawnCrystal(next_row_positions[i]);
			next_row_positions[i].y = platform_height;
			boss_controller->ComputeDangerZone(next_row_positions[i], danger_zone_radius, damage, CrowdControl::KNOCKUP);
		}
		++current_rows_of_projectiles_thrown;
		current_time_between_rounds = 0.f;
		pattern_projectile_state = PatternProjectilesState::UPDATE_PROJECTILE;
		break;

	case PatternProjectilesState::UPDATE_PROJECTILE:
		boss_controller->UpdateCrystals();
		current_time_between_rounds += delta_time;
		if (current_time_between_rounds >= timer_between_rounds)
		{
			if (current_rows_of_projectiles_thrown < maximum_rows_to_spawn)
			{
				pattern_projectile_state = PatternProjectilesState::CHARGING_PROJECTILE;
			}
			else
			{
				current_rows_of_projectiles_thrown = 0;
				is_attack_executing = false;
			}
		}
		break;

	default:
		break;
	}
}

void BossAttackCrystalsCone::ExecuteAttack()
{
	is_attack_executing = true;
	boss_controller->TriggerBossAnimation("left_punch");
	pattern_projectile_state = PatternProjectilesState::NONE;
}

void BossAttackCrystalsCone::ComputeConePosition()
{
	current_position = boss_controller->owner->transform.GetGlobalTranslation();
	left = !left;
	angle = boss_controller->GetRandom(minimum_angle, maximum_angle);

	if (left) //Left
	{
		right_path = math::Quat(boss_controller->owner->transform.GetUpVector(), -math::Acos(angle) + math::Acos(angle_cone) * 2) * boss_controller->owner->transform.GetFrontVector();
		central_path = math::Quat(boss_controller->owner->transform.GetUpVector(), -math::Acos(angle) + math::Acos(angle_cone)) * boss_controller->owner->transform.GetFrontVector();
		left_path = math::Quat(boss_controller->owner->transform.GetUpVector(), -math::Acos(angle)) * boss_controller->owner->transform.GetFrontVector();
	}
	else //Right
	{
		right_path = math::Quat(boss_controller->owner->transform.GetUpVector(), math::Acos(angle)) * boss_controller->owner->transform.GetFrontVector();
		central_path = math::Quat(boss_controller->owner->transform.GetUpVector(), math::Acos(angle) - math::Acos(angle_cone)) * boss_controller->owner->transform.GetFrontVector();
		left_path = math::Quat(boss_controller->owner->transform.GetUpVector(), math::Acos(angle) - math::Acos(angle_cone) * 2) * boss_controller->owner->transform.GetFrontVector();
	}

}

void BossAttackCrystalsCone::GetNextConePositions(float position)
{
	float3 right = right_path;
	float3 central = central_path;
	float3 left = left_path;
	next_row_positions.clear();

	right.ScaleToLength(position);
	central.ScaleToLength(position);
	left.ScaleToLength(position);

	right += current_position;
	central += current_position;
	left += current_position;

	next_row_positions.emplace_back(right);
	next_row_positions.emplace_back(central);
	next_row_positions.emplace_back(left);
}
