#include "ComboCounter.h"

#include "Main/Application.h"
#include "Module/ModuleTime.h"
#include "Module/ModuleScene.h"

#include "PlayerAttack.h"
#include "UIManager.h"

#include <MathGeoLib.h>

ComboCounter* ComboCounterDLL()
{
	ComboCounter* instance = new ComboCounter();

	return instance;
}

void ComboCounter::Awake()
{
	GameObject* ui = App->scene->GetGameObjectByName("UIManager");
	ComponentScript* component_ui = ui->GetComponentScript("UIManager");
	ui_manager = static_cast<UIManager*>(component_ui->script);
}

void ComboCounter::Update()
{
	float elapsed_combo_level_duration = App->time->delta_time / GetCurrentLevelDuration();
	combo_level_progress = math::Max(combo_level_progress - elapsed_combo_level_duration, 0.f);

	if (combo_level != ComboCounterLevel::NONE)
	{
		ui_manager->SetComboCounterProgress(combo_level_progress);
		ui_manager->SetComboCounterLevel(combo_level);

		if (combo_level_progress == 0.f)
		{
			ui_manager->HideComboCounter();
			combo_level = ComboCounterLevel::NONE;
		}
	}
}

void ComboCounter::AddHit(float damage)
{
	float damage_power_bonus_increment = COMBO_LEVEL_MAX_DAMAGE_INCREMENT * (damage - MIN_DAMAGE) / (MAX_DAMAGE - MIN_DAMAGE);
	float level_difficulty_decrement = GetCurrentLevelDifficultyDecrement();
	float combo_level_increment = COMBO_LEVEL_HIT_INCREMENT + damage_power_bonus_increment - level_difficulty_decrement;
	combo_level_progress = math::Min(combo_level_progress + combo_level_increment, 1.f);

	if (combo_level_progress == 1.f && combo_level != ComboCounterLevel::SAVAGE)
	{
		combo_level = static_cast<ComboCounterLevel>((int)combo_level + 1);
		combo_level_progress = 0.5f;
	}
}

float ComboCounter::GetCurrentLevelDuration() const
{
	switch (combo_level)
	{
		case ComboCounter::ComboCounterLevel::NONE:
			return 12000.f;

		case ComboCounter::ComboCounterLevel::DOPE:
			return 10000.f;

		case ComboCounter::ComboCounterLevel::COOL:
			return 8000.f;

		case ComboCounter::ComboCounterLevel::BRUTAL:
			return 6000.f;

		case ComboCounter::ComboCounterLevel::AWESOME:
			return 4000;

		case ComboCounter::ComboCounterLevel::SAVAGE:
			return 3000.f;

		default:
			return 0.f;
	}
}

float ComboCounter::GetCurrentLevelDifficultyDecrement() const
{
	switch (combo_level)
	{
	case ComboCounter::ComboCounterLevel::NONE:
		return -0.5f;

	case ComboCounter::ComboCounterLevel::DOPE:
		return 0.f;

	case ComboCounter::ComboCounterLevel::COOL:
		return 0.05f;

	case ComboCounter::ComboCounterLevel::BRUTAL:
		return 0.1f;

	case ComboCounter::ComboCounterLevel::AWESOME:
		return 0.2f;

	case ComboCounter::ComboCounterLevel::SAVAGE:
		return 0.25f;

	default:
		return 0.f;
	}
}