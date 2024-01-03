#ifndef  __BITERSTATEDIE_H__
#define  __BITERSTATEDIE_H__

#include "EnemyState.h"

class Biter;

class BiterStateDie : public EnemyState<Biter>
{
public:
	BiterStateDie(Biter* enemy);
	~BiterStateDie() = default;

	void OnStateEnter() override;
	void OnStateUpdate() override;
	void OnStateExit() override;

private:
	bool attacked = false;
	bool p1_attacked = false;
	bool p2_attacked = false;
	float death_explosion_time = 2000.0F;
	float death_explosion_delay_time = 1000.0F;
	bool activate_timer = false;
	float current_time = 0.f;
	int seconds_to_disappear = 1;
};

#endif