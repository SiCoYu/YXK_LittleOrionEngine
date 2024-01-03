#ifndef  __BITERSTATEATTACK_H__
#define  __BITERSTATEATTACK_H__

#include "EnemyState.h"

class Biter;

class BiterStateAttack : public EnemyState<Biter>
{
public:
	BiterStateAttack(Biter* enemy);
	~BiterStateAttack() = default;

	void OnStateEnter() override;
	void OnStateUpdate() override;
	void OnStateExit() override;

private:
	bool attacked = false;
	bool p1_attacked = false;
	bool p2_attacked = false;
	float init_attack_percentage = 0.80F;
	float final_attack_percentage = 0.89F;
};

#endif