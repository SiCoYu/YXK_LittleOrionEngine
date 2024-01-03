#ifndef  __MUSHDOOMSTATEIDLE_H__
#define  __MUSHDOOMSTATEIDLE_H__

#include "EnemyState.h"

class Mushdoom;

class MushdoomStateIdle : public EnemyState<Mushdoom>
{
public:
	MushdoomStateIdle(Mushdoom* enemy);
	~MushdoomStateIdle() = default;

	void OnStateEnter() override;
	void OnStateUpdate() override;
	void OnStateExit() override;

private:

};

#endif