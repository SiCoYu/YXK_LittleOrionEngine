#ifndef  __MUSHDOOMSTATEDIE_H__
#define  __MUSHDOOMSTATEDIE_H__

#include "EnemyState.h"

class Mushdoom;

class MushdoomStateDie : public EnemyState<Mushdoom>
{
public:
	MushdoomStateDie(Mushdoom* enemy);
	~MushdoomStateDie() = default;

	void OnStateEnter() override;
	void OnStateUpdate() override;
	void OnStateExit() override;

private:
	bool activate_timer = false;
	float current_time = 0.f;
	int seconds_to_disappear = 1;
};

#endif