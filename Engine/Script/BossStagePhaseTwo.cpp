#include "BossStagePhaseTwo.h"

#include "Component/ComponentTransform.h"
#include "Main/GameObject.h"

#include "BossAttackCrystalsGuided.h"
#include "BossAttackCrystalsCone.h"


BossStagePhaseTwo::BossStagePhaseTwo(BossController* boss_controller) : BossStage(boss_controller)
{
	name = "PhaseTwo";

	which_attack_generator = std::mt19937(random_device());
	which_attack_distribution = std::uniform_int_distribution<>(0, 1);

	attack_crystal_guided = std::make_unique<BossAttackCrystalsGuided>(boss_controller);
	attack_crystal_cone = std::make_unique<BossAttackCrystalsCone>(boss_controller);
}

void BossStagePhaseTwo::Update(float delta_time)
{
	switch (current_state)
	{
		case StatePhaseTwo::TRANSITIONING_FROM_PHASE_ONE:
			TransitioningFromPhaseOneBehaviour();
			break;

		case StatePhaseTwo::IDLE:
			IdleBehaviour(delta_time);
			break;

		case StatePhaseTwo::ATTACKING_GUIDED_PROJECTILES:
			AttackingGuidedProjectiles(delta_time);
			break;

		case StatePhaseTwo::ATTACKING_PATTERN_PROJECTILES:
			AttackingPatternProjectiles(delta_time);
			break;

		default:
			break;
	}

	boss_controller->UpdateCrystals();
}

void BossStagePhaseTwo::ChangeState()
{
	boss_controller->TriggerBossAnimation("transition_three");
	boss_controller->ActiveSound(BossBone::RIGHT_HAND, "Play_DamageHeavyCrystal");
	boss_controller->vulnerable = false;
	boss_controller->UnspawnGuidedEffect();

	boss_controller->SetEnableVulnerableZone(BossBone::RIGHT_FINGER, false, false);
	boss_controller->SetEnableVulnerableZone(BossBone::LEFT_FINGER, false, false);
}

std::string BossStagePhaseTwo::GetCurrentStateName() const
{
	switch (current_state)
	{
		case StatePhaseTwo::TRANSITIONING_FROM_PHASE_ONE:
			return std::string("TRANSITIONING FROM PHASE ONE");

		case StatePhaseTwo::IDLE:
			return std::string("IDLE");
			
		case StatePhaseTwo::ATTACKING_GUIDED_PROJECTILES:
			return std::string("ATTACKING GUIDED PROJECTILES : ") + GetCurrentGuidedProjectilesState();

		case StatePhaseTwo::ATTACKING_PATTERN_PROJECTILES:
			return std::string("ATTACKING PATTERN PROJECTILES");

		case StatePhaseTwo::TRANSITION_BY_ANIMATION:
			return std::string("TRANSITION_BY_ANIMATION");

		default:
			return std::string("UNKNOWN");
	}

	return std::string();
}

std::string BossStagePhaseTwo::GetCurrentGuidedProjectilesState() const
{
	return attack_crystal_guided->GetCurrentGuidedProjectilesState();
}

void BossStagePhaseTwo::TransitioningFromPhaseOneBehaviour()
{
	float3 camera_direction = boss_controller->GetCameraPosition() - boss_controller->owner->transform.GetGlobalTranslation();
	boss_controller->RotateBoss(camera_direction);

	if (boss_controller->IsAnimationOnState("Phase_Two_Transition") && boss_controller->GetAnimationPercentage() > 0.5)
	{
		boss_controller->SetCameraTrauma(0.55);

		if(!scream_done)
		{
			scream_done = true;
			boss_controller->ActiveSound(BossBone::HEAD, "Play_Scream1");
		}
	}

	if(boss_controller->IsAnimationOnState("Idle_Phase_Two"))
	{
		current_state = StatePhaseTwo::IDLE;
		boss_controller->vulnerable = true;

		boss_controller->SetEnableVulnerableZone(BossBone::RIGHT_FINGER, true, true);
		//LEFT
		boss_controller->SetEnableVulnerableZone(BossBone::LEFT_FINGER, true, true);
		scream_done = false;
	}
}

void BossStagePhaseTwo::IdleBehaviour(float delta_time)
{
	if (boss_controller->waiting_for_spawning)
	{
		return;
	}

	current_idle_time += delta_time;

	if(current_idle_time >= max_current_idle_time)
	{
		current_idle_time = 0.f;

		/** Use random distribution for choosing which attack follows */
		if(which_attack_distribution(which_attack_generator) == 0)
		{
			current_state = StatePhaseTwo::ATTACKING_GUIDED_PROJECTILES;
			attack_crystal_guided->ExecuteAttack();
		}
		else
		{
			attack_crystal_cone->ExecuteAttack();
			current_state = StatePhaseTwo::ATTACKING_PATTERN_PROJECTILES;
		}
	}
}

/** Code that define the behaviour of guided projectiles attack */
void BossStagePhaseTwo::AttackingGuidedProjectiles(float delta_time)
{
	attack_crystal_guided->Update(delta_time);
	if(!attack_crystal_guided->IsAttackExecuting())
	{
		current_state = StatePhaseTwo::IDLE;
	}
}

void BossStagePhaseTwo::AttackingPatternProjectiles(float delta_time)
{
	attack_crystal_cone->Update(delta_time);
	if(!attack_crystal_cone->IsAttackExecuting())
	{
		current_state = StatePhaseTwo::IDLE;
	}

}

void BossStagePhaseTwo::TransitionByAnimationBehaviour(float delta_time)
{
	if(boss_controller->IsAnimationOver())
	{
		current_state = next_transition_state;
	}
}