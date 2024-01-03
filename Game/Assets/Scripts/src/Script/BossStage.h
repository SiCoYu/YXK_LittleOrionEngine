#ifndef  __BOSSSTAGE_H__
#define  __BOSSSTAGE_H__

//class BossController;
#include "BossController.h"
#include <string>
#include <random>

class BossStage
{
public:
	BossStage() = default;
	BossStage(BossController* boss_controller)
	{
		InitBossController(boss_controller);
	}

	virtual ~BossStage() = default;
	virtual void Update(float delta_time) = 0;
	virtual void ChangeState() {};
	void InitBossController(BossController* boss_controller)
	{
		this->boss_controller = boss_controller;
	}
	virtual std::string GetCurrentStateName() const = 0; 

public:
	std::string name;
protected:
	BossController* boss_controller;
};


#endif __BOSSSTAGE_H__