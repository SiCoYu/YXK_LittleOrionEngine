#ifndef  __BOSSSTAGEPHASEONE_H__
#define  __BOSSSTAGEPHASEONE_H__

#include "BossStage.h"
#include <MathGeoLib.h>

class BossAttackSlam;
class BossAttackEarthquake;

enum class StatePhaseOne
{
	IDLE,
	LOOKING_AT_PLAYERS,
	ATTACKING_SLAM,
	ATTACKING_EARTHQUAKE,
	VULNERABLE
};

class BossStagePhaseOne : public BossStage
{
public:
	BossStagePhaseOne(BossController* boss_controller); 
	~BossStagePhaseOne() = default;

	void Update(float delta_time) override;
	void ChangeState() override;

	/** Returns the name of current state */
	std::string GetCurrentStateName() const override;

private:
	/** Functions with the logic of each state (they are all called on the update) */
	void IdleBehaviour(float delta_time);
	void LookingAtPlayersBehaviour(float delta_time);
	void AttackingSlamBehaviour(float delta_time);
	void AttackingEarthquake(float delta_time);
	void VulnerableBehaviour(float delta_time);
	/***/

private:
	StatePhaseOne current_state = StatePhaseOne::IDLE;

	std::unique_ptr<BossAttackSlam> attack_slam;
	std::unique_ptr<BossAttackEarthquake> attack_earthquake;

	//IDLE variables
	float current_time_on_idle = 0.f;
	const float max_time_on_idle = 500.f;

	//LOOKING AT variables
	float current_time_looking_for_players = 0.f;
	const float max_time_looking_for_players = 1000.f;

	uint32_t number_of_slams_done = 0;
	uint32_t number_of_slams_before_earthquake = 2;

	//VULNERABLE variables
	float current_time_vulnerable = 0.f;
	const float max_time_vulnerable = 5000.f;
	bool waiting_animation_to_finish = false;

	bool music_triggered = false;

	PlayerController::WhichPlayer target_player = PlayerController::WhichPlayer::PLAYER1;

	std::random_device random_device;

	std::mt19937 rnd_gen;
	std::uniform_real_distribution<float> distribution;

	std::mt19937 which_attack_generator;
	std::uniform_int_distribution<> which_attack_distribution;

};

#endif __BOSSSTAGEPHASEONE_H__