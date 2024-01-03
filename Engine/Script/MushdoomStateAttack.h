#ifndef  __MUSHDOOMSTATEATTACK_H__
#define  __MUSHDOOMSTATEATTACK_H__

#include "EnemyState.h"

class Mushdoom;

class MushdoomStateAttack : public EnemyState<Mushdoom>
{
public:
	MushdoomStateAttack(Mushdoom* enemy);
	~MushdoomStateAttack() = default;

	void OnStateEnter() override;
	void OnStateUpdate() override;
	void OnStateExit() override;

private:
	float attack_damage_start = 0.4f;
	float attack_damage_end = 0.6f;

	bool attacked = false;
};

#endif