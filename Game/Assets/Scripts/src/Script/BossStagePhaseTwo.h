#ifndef  __BOSSSTAGEPHASETWO_H__
#define  __BOSSSTAGEPHASETWO_H__

#include "BossStage.h"
#include <MathGeoLib.h>

class BossAttackCrystalsGuided;
class BossAttackCrystalsCone;

enum class StatePhaseTwo
{
	TRANSITIONING_FROM_PHASE_ONE,
	IDLE,
	ATTACKING_GUIDED_PROJECTILES,
	ATTACKING_PATTERN_PROJECTILES,
	TRANSITION_BY_ANIMATION
};


class BossStagePhaseTwo : public BossStage
{
public:
	BossStagePhaseTwo(BossController* boss_controller);
	~BossStagePhaseTwo() = default;

	void Update(float delta_time) override;
	void ChangeState() override;

	/** Returns the name of current state */
	std::string GetCurrentStateName() const override;
	std::string GetCurrentGuidedProjectilesState() const;

private:
	/** Functions with the logic of each state (they are all called on the update) */
	void TransitioningFromPhaseOneBehaviour();
	void IdleBehaviour(float delta_time);
	void AttackingGuidedProjectiles(float delta_time);
	void AttackingPatternProjectiles(float delta_time);
	void TransitionByAnimationBehaviour(float delta_time);

private:
	StatePhaseTwo current_state = StatePhaseTwo::TRANSITIONING_FROM_PHASE_ONE;

	std::unique_ptr<BossAttackCrystalsGuided> attack_crystal_guided;
	std::unique_ptr<BossAttackCrystalsCone> attack_crystal_cone;

	//TRANSITIONING_FROM_PHASE_ONE
	float current_time_transitioning = 0.f;
	const float max_time_transitioning = 2000.f;

	//IDLE DEBUGING WITHOUT ANIMS
	float current_idle_time = 0.f;
	const float max_current_idle_time = 3500.f;

	//TRANSITION BY ANIMATION
	StatePhaseTwo next_transition_state = StatePhaseTwo::IDLE;

	bool scream_done = false;

	std::random_device random_device;
	std::mt19937 which_attack_generator;
	std::uniform_int_distribution<> which_attack_distribution;

};

#endif __BOSSSTAGEPHASETWO_H__