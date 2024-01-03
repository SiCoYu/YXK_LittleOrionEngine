#ifndef  __BOSSSTAGEPHASETHREE_H__
#define  __BOSSSTAGEPHASETHREE_H__

#include "BossStage.h"
#include <MathGeoLib.h>

class BattleEvent;
class BossAttackAvalanche;
class BossController;

enum class StatePhaseThree
{
	TRANSITIONING_FROM_PHASE_TWO,
	IDLE,
	MOBS_SPAWNING,
	ATTACKING_AVALANCHE,
	VULNERABLE
};

class BossStagePhaseThree  : public BossStage
{
public:
	BossStagePhaseThree(BossController* boss_controller);
	~BossStagePhaseThree() = default;

	void Update(float delta_time) override;

	/** Returns the name of current state */
	std::string GetCurrentStateName() const override;

private:
	/** Functions with the logic of each state (they are all called on the update) */
	void TransitionFromPhaseTwoBehaviour();
	void IdleBehaviour(float delta_time);
	void AttackingAvalanche(float delta_time);
	void VulnerableBehaviour(float delta_time);

	void GetBattleEvent();
	void MobsSpawn();

private:
	StatePhaseThree current_state = StatePhaseThree::TRANSITIONING_FROM_PHASE_TWO;
	std::unique_ptr<BossAttackAvalanche> attack_avalanche;

	//TRANSITION FROM PHASE TWO BEHAVIOUR
	float current_time_transitioning = 0;
	const float max_time_transitioning = 2000.f;

	//MOBS ATTACK
	BattleEvent* battle_event = nullptr;

	//ATTACKING AVALANCHE

	//VULNERABLE
	float current_vulnerable_time = 0.f;
	const float attacking_vulnerable_time = 5000.f;
	bool audio_triggered = false;
	bool scream_done = false;
};

#endif __BOSSSTAGEPHASETHREE_H__
