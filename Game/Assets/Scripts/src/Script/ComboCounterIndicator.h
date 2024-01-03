#ifndef  __COMBOCOUNTERINDICATOR_H__
#define  __COMBOCOUNTERINDICATOR_H__

#include "Script.h"

#include "ComboCounter.h"

class ComponentImage;
class ComponentText;
class ProgressBar;

class ComboCounterIndicator : public Script
{
public:
	enum class ComboCounterIndicatorState
	{
		HIDDEN,
		INITIAL_TRANSITION,
		INITIAL_VISIBLE,
		WORD_TRANSITION,
		VISIBLE,
		HIDING
	};

	ComboCounterIndicator();
	~ComboCounterIndicator() = default;

	void Awake() override;
	void Start() override;
	void Update() override;

	void SetComboCounterProgress(float combo_counter_progress);
	void SetComboCounterLevel(ComboCounter::ComboCounterLevel level);

	void Hide();

	void OnInspector(ImGuiContext*) override;

private:
	void SetCurrentLevelInitialAlpha(float alpha);
	void SetCurrentLevelBarAlpha(float alpha);

private:
	ProgressBar* combo_counter_progress_bar = nullptr;
	GameObject* combo_counter_progress_bar_border_go = nullptr;
	ComponentImage* combo_counter_progress_bar_border = nullptr;
	ComponentImage* combo_counter_progress_bar_background = nullptr;
	ComponentImage* combo_counter_progress_bar_fill = nullptr;

	std::vector<GameObject*> combo_counter_avaluations;
	ComponentImage* level_indicator_blur = nullptr;

	ComboCounter::ComboCounterLevel current_level = ComboCounter::ComboCounterLevel::NONE;
	ComponentImage* current_level_initial = nullptr;
	ComponentImage* current_level_word = nullptr;

	ComboCounterIndicatorState indicator_state = ComboCounterIndicatorState::HIDDEN;
	bool combo_counter_bar_showing = false;
	bool combo_counter_bar_shown = false;

	float initial_transitioning_time = 200.f;
	float initial_start_scale = 3.f;
	float initial_final_scale = 0.f;

	float initial_visible_time = 500.f;

	float word_transitioning_time = 75.f;
	float blur_scale = 1.25f;
	float word_position = 200.f;

	float hidding_time = 250.f;
	float hiding_translation = 200.f;

	float current_time = 0.f;
};
extern "C" SCRIPT_API ComboCounterIndicator* ComboCounterIndicatorDLL(); //This is how we are going to load the script
#endif