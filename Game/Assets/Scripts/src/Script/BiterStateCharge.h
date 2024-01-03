#ifndef  __BITERSTATECHARGE_H__
#define  __BITERSTATECHARGE_H__

#include "EnemyState.h"

class Biter;

class BiterStateCharge : public EnemyState<Biter>
{
public:
	BiterStateCharge(Biter* enemy);
	~BiterStateCharge() = default;

	void OnStateEnter() override;
	void OnStateUpdate() override;
	void OnStateExit() override;

private:
	bool p1_attacked = false;
	bool p2_attacked = false;
};

#endif