#ifndef  __MUSHDOOMSTATEPURSUE_H__
#define  __MUSHDOOMSTATEPURSUE_H__

#include "EnemyState.h"

class Mushdoom;

class MushdoomStatePursue : public EnemyState<Mushdoom>
{
public:
	MushdoomStatePursue(Mushdoom* enemy);
	~MushdoomStatePursue() = default;

	void OnStateEnter() override;
	void OnStateUpdate() override;
	void OnStateExit() override;
};

#endif