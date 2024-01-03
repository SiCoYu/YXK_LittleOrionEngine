#ifndef  __BITERSTATEEXPLOSION_H__
#define  __BITERSTATEEXPLOSION_H__

#include "EnemyState.h"

class Biter;

class BiterStateExplosion : public EnemyState<Biter>
{
public:
	BiterStateExplosion(Biter* enemy);
	~BiterStateExplosion() = default;

	void OnStateEnter() override;
	void OnStateUpdate() override;
	void OnStateExit() override;

private:
	bool attacked = false;
	bool p1_attacked = false;
	bool p2_attacked = false;
};

#endif