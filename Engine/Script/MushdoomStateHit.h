#ifndef  __MUSHDOOMSTATEHIT_H__
#define  __MUSHDOOMSTATEHIT_H__

#include "EnemyState.h"

class Mushdoom;

class MushdoomStateHit : public EnemyState<Mushdoom>
{
public:
	MushdoomStateHit(Mushdoom* enemy);
	~MushdoomStateHit() = default;

	void OnStateEnter() override;
	void OnStateUpdate() override;
	void OnStateExit() override;

private:
	float total_percentatge_hitted = 0.95f;
};

#endif