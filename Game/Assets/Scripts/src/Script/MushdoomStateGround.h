#ifndef  __MUSHDOOMSTATEGROUND_H__
#define  __MUSHDOOMSTATEGROUND_H__

#include "EnemyState.h"

class Mushdoom;

class MushdoomStateGround : public EnemyState<Mushdoom>
{
public:
	MushdoomStateGround(Mushdoom* enemy);
	~MushdoomStateGround() = default;

	void OnStateEnter() override;
	void OnStateUpdate() override;
	void OnStateExit() override;
};

#endif