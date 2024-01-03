#ifndef  __MUSHDOOMSTATESCREAM_H__
#define  __MUSHDOOMSTATESCREAM_H__

#include "EnemyState.h"

class Mushdoom;

class MushdoomStateScream : public EnemyState<Mushdoom>
{
public:
	MushdoomStateScream(Mushdoom* enemy);
	~MushdoomStateScream() = default;

	void OnStateEnter() override;
	void OnStateUpdate() override;
	void OnStateExit() override;
};

#endif