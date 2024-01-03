#ifndef  __BITERSTATEPURSUE_H__
#define  __BITERSTATEPURSUE_H__

#include "EnemyState.h"

class Biter;

class BiterStatePursue : public EnemyState<Biter>
{
public:
	BiterStatePursue(Biter* enemy);
	~BiterStatePursue() = default;

	void OnStateEnter() override;
	void OnStateUpdate() override;
	void OnStateExit() override;
};

#endif