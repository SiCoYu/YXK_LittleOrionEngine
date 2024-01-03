#include "BossAttackCrystalsGuided.h"


#include "BossController.h"

BossAttackCrystalsGuided::BossAttackCrystalsGuided(BossController* boss_controller) : BossAttack(boss_controller)
{
	guided_projectile_position_generator = std::mt19937(random_device());
	attack_guided_projectile_position_distribution = std::uniform_int_distribution<>(-2, 2);
}


// Update is called once per frame
void BossAttackCrystalsGuided::Update(float delta_time)
{
	if(!is_attack_executing)
	{
		return;
	}

	switch (guided_projectile_state)
	{
	case GuidedProjectilesState::NONE:
		if (boss_controller->IsAnimationOnState("Stay_Punch_Right") && boss_controller->GetAnimationPercentage() > 0.5f)
		{
			guided_projectile_state = GuidedProjectilesState::COMPUTING_NEW_POSITION;
			boss_controller->TriggerBossAnimation("idle_two");
			boss_controller->ActiveSound(BossBone::RIGHT_HAND, "Play_Smash2");
		}
		break;

	case GuidedProjectilesState::COMPUTING_NEW_POSITION:
		//Looks for next point to place the projectile
		current_projectile_position = boss_controller->GetPlayerPosition(0);
		current_projectile_position.y = boss_controller->ground_y_position;
		if (!player1_hit)
		{
			//Spawn effect
			boss_controller->SpawnGuidedEffect(current_projectile_position);		
		}

		if(boss_controller->IsMultiplayer() && !player2_hit)
		{
			current_projectile_position_multiplayer = boss_controller->GetPlayerPosition(1);
			current_projectile_position_multiplayer.y = boss_controller->ground_y_position;
			//Spawn effect
			boss_controller->SpawnGuidedEffect(current_projectile_position_multiplayer);
		}

		guided_projectile_state = GuidedProjectilesState::CHARGING_PROJECTILE;

		break;

	case GuidedProjectilesState::CHARGING_PROJECTILE:
		current_time_charging_from_ground += delta_time;
		if (current_time_charging_from_ground >= time_charging_from_ground)
		{
			current_time_charging_from_ground = 0.f;
			if(!player1_hit)
			{
				boss_controller->SpawnCrystal(current_projectile_position);
				boss_controller->ActiveSound(BossBone::CHEST, "Play_Smash2");
			}
			player1_hit = player1_hit || boss_controller->ComputeDangerZone(current_projectile_position, 3.f, 100.f, CrowdControl::KNOCKUP);
			

			if (boss_controller->IsMultiplayer() && !player2_hit)
			{
				player2_hit = player2_hit || boss_controller->ComputeDangerZone(current_projectile_position_multiplayer, 3.f, 100.f, CrowdControl::KNOCKUP, 1);
				boss_controller->SpawnCrystal(current_projectile_position_multiplayer);
				boss_controller->ActiveSound(BossBone::CHEST, "Play_Smash2");
			}


			guided_projectile_state = GuidedProjectilesState::UPDATE_PROJECTILE;
		}
		break;

	case GuidedProjectilesState::UPDATE_PROJECTILE:

		current_time_charging_from_ground += delta_time;
		if (current_time_charging_from_ground >= 1.2f * time_charging_from_ground)
		{
			current_time_charging_from_ground = 0.f;
			++current_number_of_projectiles_thrown;
			guided_projectile_state = GuidedProjectilesState::COMPUTING_NEW_POSITION;

			//Hide effect
			boss_controller->UnspawnGuidedEffect();

			//Check if both players have been hit
			end = player1_hit;
			if(boss_controller->IsMultiplayer())
			{
				end = end && player2_hit;
			}
		}
		break;

	default:
		break;
	}

	//If all projectiles have been thrown
	if (current_number_of_projectiles_thrown >= number_of_projectiles_thrown || end)
	{
		current_number_of_projectiles_thrown = 0;
		is_attack_executing = false;

		player1_hit = false;
		player2_hit = false;
		end = false;
	}
}

void BossAttackCrystalsGuided::ExecuteAttack()
{
	is_attack_executing = true;
	boss_controller->TriggerBossAnimation("right_punch");
	guided_projectile_state = GuidedProjectilesState::NONE;
}

std::string BossAttackCrystalsGuided::GetCurrentGuidedProjectilesState() const
{
	switch (guided_projectile_state)
	{
		case GuidedProjectilesState::NONE:
			return std::string("NONE");

		case GuidedProjectilesState::COMPUTING_NEW_POSITION:
			return std::string("COMPUTING NEW POSITION");

		case GuidedProjectilesState::CHARGING_PROJECTILE:
			return std::string("CHARGING PROJECTILE");

		case GuidedProjectilesState::UPDATE_PROJECTILE:
			return std::string("UPDATE PROJECTILE");

		default:
			return std::string("UNKNOWN");
	}
	return std::string();
}
