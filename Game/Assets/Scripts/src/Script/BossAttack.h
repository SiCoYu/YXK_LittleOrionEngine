#ifndef  __BOSSATTACK_H__
#define  __BOSSATTACK_H__

#include "BossController.h"
#include <string>
#include <random>

class BossAttack
{
public:
	BossAttack() = default;
	BossAttack(BossController* boss_controller)
	{
		InitBossController(boss_controller);
	}

	virtual ~BossAttack() = default;
	virtual void Update(float delta_time) = 0;
	virtual void ExecuteAttack() = 0;
	bool IsAttackExecuting() const
	{
		return is_attack_executing;
	}
	void InitBossController(BossController* boss_controller)
	{
		this->boss_controller = boss_controller;
	}

protected:
	BossController* boss_controller = nullptr;
	bool is_attack_executing = false;

	std::random_device random_device;
};


#endif __BOSSATTACK_H__
