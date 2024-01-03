#ifndef __COMBOCOUNTER_H__
#define __COMBOCOUNTER_H__

#include "Script.h"

#include <string>

class UIManager;

class ComboCounter : public Script
{
public:
	enum class ComboCounterLevel
	{
		NONE,
		DOPE,
		COOL,
		BRUTAL,
		AWESOME,
		SAVAGE
	};

	void Awake() override;
	void Update() override;
	
	void AddHit(float damage);

private:
	float GetCurrentLevelDuration() const;
	float GetCurrentLevelDifficultyDecrement() const;

private:
	UIManager* ui_manager = nullptr;

	float combo_level_progress = 0.f;
	ComboCounterLevel combo_level = ComboCounterLevel::NONE;

	const float COMBO_LEVEL_HIT_INCREMENT = 0.2f;
	const float COMBO_LEVEL_MAX_DAMAGE_INCREMENT = 0.2f;

};
extern "C" SCRIPT_API ComboCounter* ComboCounterDLL(); //This is how we are going to load the script
#endif