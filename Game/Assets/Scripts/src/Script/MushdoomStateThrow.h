#ifndef  __MUSHDOOMSTATETHROW_H__
#define  __MUSHDOOMSTATETHROW_H__

#include "EnemyState.h"

class Mushdoom;

class MushdoomStateThrow : public EnemyState<Mushdoom>
{
public:
	MushdoomStateThrow(Mushdoom* enemy);
	~MushdoomStateThrow() = default;

	void OnStateEnter() override;
	void OnStateUpdate() override;
	void OnStateExit() override;

private:
	float throw_delay = 0.75f;
	float finish_animation = 0.85f;

	bool throw_ball = false;
};

#endif