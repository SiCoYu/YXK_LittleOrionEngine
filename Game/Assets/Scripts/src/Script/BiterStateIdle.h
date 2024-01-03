#ifndef  __BITERSTATEIDLE_H__
#define  __BITERSTATEIDLE_H__

#include "EnemyState.h"

class Biter;

class BiterStateIdle : public EnemyState<Biter>
{
public:
	BiterStateIdle(Biter* enemy);
	~BiterStateIdle() = default;

	void OnStateEnter() override;
	void OnStateUpdate() override;
	void OnStateExit() override;
};

#endif