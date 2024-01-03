#ifndef  __BITERSTATEHIT_H__
#define  __BITERSTATEHIT_H__

#include "EnemyState.h"

class Biter;

class BiterStateHit : public EnemyState<Biter>
{
public:
	BiterStateHit(Biter* enemy);
	~BiterStateHit() = default;

	void OnStateEnter() override;
	void OnStateUpdate() override;
	void OnStateExit() override;

private:
	float total_time_hitted = 0.7f;
};

#endif