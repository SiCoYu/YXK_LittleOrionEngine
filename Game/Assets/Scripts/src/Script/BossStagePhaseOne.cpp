#include "BossStagePhaseOne.h"
#include "BossStagePhaseOne.h"

#include "Component/ComponentTransform.h"
#include "Main/GameObject.h"
#include "Log/EngineLog.h"

#include "BossAttackSlam.h"
#include "BossAttackEarthquake.h"
#include "BossController.h"
#include "PlayerController.h"
#include "WorldManager.h"

BossStagePhaseOne::BossStagePhaseOne(BossController* boss_controller) : BossStage(boss_controller)
{
	rnd_gen = std::mt19937(random_device());
	distribution = std::uniform_real_distribution<float>(-10.0f, 10.0f);

	which_attack_generator = std::mt19937(random_device());
	which_attack_distribution = std::uniform_int_distribution<>(2, 3);

	attack_slam = std::make_unique<BossAttackSlam>(boss_controller);
	attack_earthquake = std::make_unique<BossAttackEarthquake>(boss_controller);

	name = "PhaseOne";
}

void BossStagePhaseOne::Update(float delta_time)
{
	switch (current_state)
	{
		case StatePhaseOne::IDLE:
			IdleBehaviour(delta_time);
			break;

		case StatePhaseOne::LOOKING_AT_PLAYERS:
			LookingAtPlayersBehaviour(delta_time);
			break;

		case StatePhaseOne::ATTACKING_SLAM:
			AttackingSlamBehaviour(delta_time);
			break;

		case StatePhaseOne::ATTACKING_EARTHQUAKE:
			AttackingEarthquake(delta_time);
			break;

		case StatePhaseOne::VULNERABLE:
			VulnerableBehaviour(delta_time);
			break;

		default:
			break;
	}
}

void BossStagePhaseOne::ChangeState()
{
	boss_controller->TriggerBossAnimation("transition_two");
	boss_controller->ActiveSound(BossBone::RIGHT_HAND, "Play_DamageHeavyCrystal");
	boss_controller->vulnerable = false;

	boss_controller->SetEnableVulnerableZone(BossBone::RIGHT_FOREARM, false, false);
	boss_controller->SetEnableVulnerableZone(BossBone::RIGHT_FINGER, false, false);
	//LEFT
	boss_controller->SetEnableVulnerableZone(BossBone::LEFT_FOREARM, false, false);
	boss_controller->SetEnableVulnerableZone(BossBone::LEFT_FINGER, false, false);
}

std::string BossStagePhaseOne::GetCurrentStateName() const
{
	switch (current_state)
	{
		case StatePhaseOne::IDLE:
			return std::string("IDLE");

		case StatePhaseOne::LOOKING_AT_PLAYERS:
			return std::string("LOOKING AT PLAYERS");

		case StatePhaseOne::ATTACKING_SLAM:
			return std::string("ATTACKING SLAM");

		case StatePhaseOne::ATTACKING_EARTHQUAKE:
			return std::string("ATTACKING EARTHQUAKE");

		case StatePhaseOne::VULNERABLE:
			return std::string("VULNERABLE");

		default:
			return std::string("UNKNOWN");
	}
}

void BossStagePhaseOne::IdleBehaviour(float delta_time)
{

	if(boss_controller->waiting_for_spawning)
	{
		return;
	}

	//Executing some cool animation instead of this counter
	boss_controller->RotateBoss(target_player);
	current_time_on_idle += delta_time;
	if (current_time_on_idle >= max_time_on_idle)
	{
		current_time_on_idle = 0.f;
		
		if (boss_controller->owner->transform.GetGlobalTranslation().Distance(boss_controller->GetCameraPosition()) < 25.f) 
		{
			if(!music_triggered)
			{
				music_triggered = true;
				boss_controller->TriggerMusic();
				boss_controller->BlockInitialPath();
				boss_controller->FreezeCamera(true);
			}
			if(boss_controller->IsMultiplayer())
			{
				target_player = static_cast<PlayerController::WhichPlayer>(std::rand() % 2 == 0);
			}
			current_state = StatePhaseOne::LOOKING_AT_PLAYERS;
		}
		//Transitioning from looking to attacking slam
	}
}

void BossStagePhaseOne::LookingAtPlayersBehaviour(float delta_time)
{
	boss_controller->RotateBoss(target_player);
	current_time_looking_for_players += delta_time;
	if(current_time_looking_for_players >= max_time_looking_for_players)
	{
		current_time_looking_for_players = 0.f;
		if(number_of_slams_done == number_of_slams_before_earthquake)
		{
			number_of_slams_done = 0;
			number_of_slams_before_earthquake = which_attack_distribution(which_attack_generator);
			current_state = StatePhaseOne::ATTACKING_EARTHQUAKE;
			attack_earthquake->ExecuteAttack();
		}
		else
		{
			++number_of_slams_done;
			current_state = StatePhaseOne::ATTACKING_SLAM;
			
			//Transitioning from looking to attacking slam		
			attack_slam->ExecuteAttack();
		}
	}
}

void BossStagePhaseOne::AttackingSlamBehaviour(float delta_time)
{
	attack_slam->Update(delta_time);
	
	if (boss_controller->IsAnimationOnState("Slam_Right") && boss_controller->GetAnimationPercentage() > 0.5f) 
	{
		boss_controller->RotateBoss(target_player);
	}
	if(!attack_slam->IsAttackExecuting())
	{
		current_state = StatePhaseOne::VULNERABLE;
		boss_controller->vulnerable = true;
		//Change animation from slam to vulnerable
		boss_controller->TriggerBossAnimation("waiting");
	}
}

/** Just a timer for testing the attacks transition */
void BossStagePhaseOne::AttackingEarthquake(float delta_time)
{
	attack_earthquake->Update(delta_time);

	if(!attack_earthquake->IsAttackExecuting())
	{
		current_state = StatePhaseOne::IDLE;
	}
}

void BossStagePhaseOne::VulnerableBehaviour(float delta_time)
{
	if(waiting_animation_to_finish)
	{
		boss_controller->RotateBoss(target_player);
		if(boss_controller->IsAnimationOnState("Slam_Right_Return") && boss_controller->IsAnimationOver())
		{
			waiting_animation_to_finish = false;
			current_state = StatePhaseOne::IDLE;

			boss_controller->SetEnableVulnerableZone(BossBone::RIGHT_FOREARM, false, false);
			boss_controller->SetEnableVulnerableZone(BossBone::RIGHT_FINGER, false, false);
			//LEFT
			//boss_controller->SetEnableVulnerableZone(BossBone::LEFT_FOREARM, false, true);
			//boss_controller->SetEnableVulnerableZone(BossBone::LEFT_FINGER, false, false);
		}

		return;
	}

	current_time_vulnerable += delta_time;
	if(current_time_vulnerable >= max_time_vulnerable)
	{
		//Change animation from vulnerable to idle
		boss_controller->TriggerBossAnimation("return");
		current_time_vulnerable = 0.f;
		boss_controller->vulnerable = false;
		waiting_animation_to_finish = true;
	}

}

