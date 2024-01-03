#include "BossStagePhaseThree.h"

#include "Component/ComponentTransform.h"
#include "Main/GameObject.h"

#include "BattleEvent.h"
#include "BossAttackAvalanche.h"
#include "EventManager.h"

BossStagePhaseThree::BossStagePhaseThree(BossController* boss_controller) : BossStage(boss_controller)
{
	name = "PhaseThree";
	attack_avalanche = std::make_unique<BossAttackAvalanche>(boss_controller);
}

void BossStagePhaseThree::Update(float delta_time)
{
	switch (current_state)
	{
		case StatePhaseThree::TRANSITIONING_FROM_PHASE_TWO:
			TransitionFromPhaseTwoBehaviour();
			break;

		case StatePhaseThree::IDLE:
			IdleBehaviour(delta_time);
			break;

		case StatePhaseThree::MOBS_SPAWNING:
			MobsSpawn();
			break;

		case StatePhaseThree::ATTACKING_AVALANCHE:
			AttackingAvalanche(delta_time);
			break;

		case StatePhaseThree::VULNERABLE:
			VulnerableBehaviour(delta_time);
			break;

		default:
			break;
	}
}

std::string BossStagePhaseThree::GetCurrentStateName() const
{
	switch (current_state)
	{
		case StatePhaseThree::TRANSITIONING_FROM_PHASE_TWO:
			return std::string("TRANSITIONING FROM PHASE TWO");

		case StatePhaseThree::IDLE:
			return std::string("IDLE");

		case StatePhaseThree::MOBS_SPAWNING:
			return std::string("MOBS SPAWNING");

		case StatePhaseThree::ATTACKING_AVALANCHE:
			return std::string("ATTACKING AVALANCHE");

		case StatePhaseThree::VULNERABLE:
			return std::string("VULNERABLE");

		default:
			return std::string("UNKNOWN");
	}

	return std::string();
}

void BossStagePhaseThree::TransitionFromPhaseTwoBehaviour()
{
	boss_controller->UpdateCrystals();
	if (boss_controller->IsAnimationOnState("Transition_Three") && boss_controller->GetAnimationPercentage() > 0.3 && boss_controller->GetAnimationPercentage() < 0.7)
	{
		boss_controller->SetCameraTrauma(0.55);

		if(!scream_done)
		{
			scream_done = true;
			boss_controller->ActiveSound(BossBone::HEAD, "Play_ScreamKingKong");
			boss_controller->ActiveSound(BossBone::HEAD, "Play_BossKingKong");
		}
	}

	if (boss_controller->IsAnimationOnState("Spawn_Mobs"))
	{
		GetBattleEvent();
		scream_done = false;
		current_state = StatePhaseThree::MOBS_SPAWNING;
	}
}

void BossStagePhaseThree::IdleBehaviour(float delta_time)
{
	
}

void BossStagePhaseThree::AttackingAvalanche(float delta_time)
{
	attack_avalanche->Update(delta_time);
	if(!attack_avalanche->IsAttackExecuting())
	{
		current_state = StatePhaseThree::VULNERABLE;
		boss_controller->vulnerable = true;
		boss_controller->TriggerBossAnimation("stunned");

		boss_controller->SetEnableVulnerableZone(BossBone::CHEST, false, true);
		boss_controller->SetEnableVulnerableZone(BossBone::HEAD, true, true);

		boss_controller->SetEnableVulnerableZone(BossBone::RIGHT_FOREARM, false, true);
		boss_controller->SetEnableVulnerableZone(BossBone::RIGHT_FINGER, false, true);
		//LEFT
		//boss_controller->SetEnableVulnerableZone(BossBone::LEFT_FOREARM, false, true);
		//boss_controller->SetEnableVulnerableZone(BossBone::LEFT_FINGER, false, true);
		
		boss_controller->stunned = true;
	}
}

void BossStagePhaseThree::VulnerableBehaviour(float delta_time)
{
	if(current_vulnerable_time >= attacking_vulnerable_time)
	{
		current_vulnerable_time = 0.f;
		current_state = StatePhaseThree::ATTACKING_AVALANCHE;
		boss_controller->TriggerBossAnimation("standup");
		boss_controller->ActiveSound(BossBone::CHEST, "Play_BossStandup");
		boss_controller->vulnerable = false;
		attack_avalanche->ExecuteAttack();

		boss_controller->SetEnableVulnerableZone(BossBone::CHEST, false, false);
		boss_controller->SetEnableVulnerableZone(BossBone::HEAD, false, false);

		boss_controller->SetEnableVulnerableZone(BossBone::RIGHT_FOREARM, false, false);
		boss_controller->SetEnableVulnerableZone(BossBone::RIGHT_FINGER, false, false);
		//LEFT
		//boss_controller->SetEnableVulnerableZone(BossBone::LEFT_FOREARM, false, false);
		//boss_controller->SetEnableVulnerableZone(BossBone::LEFT_FINGER, false, false);
		boss_controller->stunned = false;
		audio_triggered = false;
	}
	if (boss_controller->IsAnimationOnState("Boss_Stunned") && boss_controller->GetAnimationPercentage() > 0.5f) 
	{
		boss_controller->ComputeIfSmashedPlayer(5000.f, 0, CrowdControl::STUN);
		if (boss_controller->IsMultiplayer())
		{
			boss_controller->ComputeIfSmashedPlayer(5000.f, 1, CrowdControl::STUN);
		}


		
	}

	if (!audio_triggered && boss_controller->IsAnimationOnState("Boss_Stunned") && boss_controller->GetAnimationPercentage() > 0.8f)
	{
		boss_controller->ActiveSound(BossBone::RIGHT_HAND, "Play_BossDown");
		audio_triggered = true;
	}
	current_vulnerable_time += delta_time;
}


void BossStagePhaseThree::GetBattleEvent()
{
	battle_event = boss_controller->GetBattleEvent();
}


void BossStagePhaseThree::MobsSpawn()
{
	if (!battle_event->IsActivated())
	{
		battle_event->ActivateEvent();
		boss_controller->event_manager->battle_event_triggered = true;
	}
	else 
	{
		if(battle_event->CheckEvent())
		{
			boss_controller->TriggerBossAnimation("avalanche");
			current_state = StatePhaseThree::ATTACKING_AVALANCHE;
			attack_avalanche->ExecuteAttack();
		}
		else
		{
			battle_event->UpdateBattle(boss_controller->event_manager->enemies_killed_on_wave);
		}
	}

}
