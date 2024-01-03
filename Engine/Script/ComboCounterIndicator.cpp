#include "ComboCounterIndicator.h"

#include "Component/ComponentImage.h"
#include "Component/ComponentScript.h"
#include "Component/ComponentText.h"
#include "Component/ComponentTransform.h"

#include "Main/Application.h"
#include "Main/GameObject.h"
#include "Module/ModuleInput.h"
#include "Module/ModuleScene.h"
#include "Module/ModuleTime.h"

#include "ProgressBar.h"

#include <imgui.h>


ComboCounterIndicator* ComboCounterIndicatorDLL()
{
	ComboCounterIndicator* instance = new ComboCounterIndicator();
	return instance;
}

ComboCounterIndicator::ComboCounterIndicator()
{
}

// Use this for initialization before Start()
void ComboCounterIndicator::Awake()
{
	combo_counter_progress_bar_border_go = owner->children[1];
	combo_counter_progress_bar_border = static_cast<ComponentImage*>(combo_counter_progress_bar_border_go->GetComponent(Component::ComponentType::UI_IMAGE));

	GameObject* combo_counter_progress_bar_background_go = combo_counter_progress_bar_border_go->children[0]->children[0];
	combo_counter_progress_bar_background = static_cast<ComponentImage*>(combo_counter_progress_bar_background_go->GetComponent(Component::ComponentType::UI_IMAGE));
	combo_counter_progress_bar = static_cast<ProgressBar*>(combo_counter_progress_bar_background_go->GetComponentScript("ProgressBar")->script);

	GameObject* combo_counter_progress_bar_go = combo_counter_progress_bar_background_go->children[0];
	combo_counter_progress_bar_fill = static_cast<ComponentImage*>(combo_counter_progress_bar_go->GetComponent(Component::ComponentType::UI_IMAGE));

	GameObject* level_indicator_blur_go = owner->children[2];
	level_indicator_blur = static_cast<ComponentImage*>(level_indicator_blur_go->GetComponent(Component::ComponentType::UI_IMAGE));

	GameObject* combo_counter_avaluations_root = App->scene->GetGameObjectByName("Combo Counter Avaluations");
	for (auto& combo_counter_avaluation : combo_counter_avaluations_root->children)
	{
		combo_counter_avaluations.emplace_back(combo_counter_avaluation);
	}
}

// Use this for initialization
void ComboCounterIndicator::Start()
{
}

// Update is called once per frame
void ComboCounterIndicator::Update()
{
	switch (indicator_state)
	{
		case ComboCounterIndicatorState::HIDDEN:
			return;

		case ComboCounterIndicatorState::INITIAL_TRANSITION:
		{
			current_time += App->time->delta_time;
			float current_progress = math::Min(current_time / initial_transitioning_time, 1.f);

			SetCurrentLevelInitialAlpha(current_progress);

			float scale = math::Lerp(initial_start_scale, initial_final_scale, current_progress);
			current_level_initial->owner->transform_2d.SetScale(float3(scale));

			if (current_progress == 1.f)
			{
				current_time = 0.f;
				indicator_state = ComboCounterIndicatorState::INITIAL_VISIBLE;
			}

			break;
		}

		case ComboCounterIndicatorState::INITIAL_VISIBLE:
			current_time += App->time->delta_time;
			if (current_time >= initial_visible_time)
			{
				SetCurrentLevelInitialAlpha(0.f);

				float4 blur_color = level_indicator_blur->color;
				blur_color.w = 1.f;
				level_indicator_blur->SetColor(blur_color);

				level_indicator_blur->owner->transform_2d.SetScale(float3::one);

				current_level_word->active = true;

				current_time = 0.f;
				indicator_state = ComboCounterIndicatorState::WORD_TRANSITION;

				if (!combo_counter_bar_shown)
				{
					combo_counter_bar_showing = true;
					combo_counter_bar_shown = true;
				}
				return;
			}
			break;

		case ComboCounterIndicatorState::WORD_TRANSITION:
		{
			current_time += App->time->delta_time;
			float current_progress = math::Min(current_time / word_transitioning_time, 1.f);

			float posisiton = math::Lerp(word_position, 0.f, current_progress);
			current_level_word->owner->transform_2d.SetTranslation(float3(posisiton, 0.f, 0.f));

			float alpha = math::Lerp(1.f, 0.f, current_progress);
			float4 level_indicator_blur_color = level_indicator_blur->color;
			level_indicator_blur_color.w = alpha;
			level_indicator_blur->SetColor(level_indicator_blur_color);

			float scale = math::Lerp(1.f, blur_scale, current_progress);
			level_indicator_blur->owner->transform_2d.SetScale(float3(scale));

			float4 current_level_word_color = current_level_word->color;
			current_level_word_color.w = math::Lerp(0.f, 1.f, current_progress);
			current_level_word->SetColor(current_level_word_color);

			if (combo_counter_bar_showing)
			{
				float3 current_combo_counter_progress_indicator_position = combo_counter_progress_bar_border_go->transform_2d.GetTranslation();
				current_combo_counter_progress_indicator_position.x = math::Lerp(hiding_translation, 0.f, current_progress);
				combo_counter_progress_bar_border_go->transform_2d.SetTranslation(current_combo_counter_progress_indicator_position);

				float combo_counter_bar_alpha = math::Lerp(0.f, 1.f, current_progress);
				SetCurrentLevelBarAlpha(combo_counter_bar_alpha);
			}

			if (current_progress == 1.f)
			{
				current_time = 0.f;
				indicator_state = ComboCounterIndicatorState::VISIBLE;

				if (combo_counter_bar_showing)
				{
					combo_counter_bar_showing = false;
				}
			}

			break;
		}

		case ComboCounterIndicatorState::VISIBLE:
			break;

		case ComboCounterIndicatorState::HIDING:
		{
			current_time += App->time->delta_time;
			float current_progress = math::Min(current_time / word_transitioning_time, 1.f);

			float3 current_combo_counter_progress_indicator_position = combo_counter_progress_bar_border_go->transform_2d.GetTranslation();
			current_combo_counter_progress_indicator_position.x = math::Lerp(0.f, hiding_translation, current_progress);
			combo_counter_progress_bar_border_go->transform_2d.SetTranslation(current_combo_counter_progress_indicator_position);

			float combo_counter_bar_alpha = math::Lerp(1.f, 0.f, current_progress);
			SetCurrentLevelBarAlpha(combo_counter_bar_alpha);

			float3 current_combo_counter_word_position = current_level_word->owner->transform_2d.GetTranslation();
			current_combo_counter_word_position.x = math::Lerp(0.f, hiding_translation, current_progress);
			current_level_word->owner->transform_2d.SetTranslation(current_combo_counter_word_position);

			float4 current_level_word_color = current_level_word->color;
			current_level_word_color.w = math::Lerp(1.f, 0.f, current_progress);
			current_level_word->SetColor(current_level_word_color);

			if (current_progress == 1.f)
			{
				current_level_word->active = false;
				combo_counter_bar_shown = false;

				current_time = 0.f;
				indicator_state = ComboCounterIndicatorState::HIDDEN;
				current_level = ComboCounter::ComboCounterLevel::NONE;
			}

			break;
		}

	}
}

void ComboCounterIndicator::SetComboCounterProgress(float combo_counter_progress)
{
	combo_counter_progress_bar->SetProgress(combo_counter_progress);
}

void ComboCounterIndicator::SetComboCounterLevel(ComboCounter::ComboCounterLevel level)
{
	if (current_level == level || indicator_state == ComboCounterIndicatorState::HIDING)
	{
		return;
	}

	if (current_level != ComboCounter::ComboCounterLevel::NONE)
	{
		current_level_word->active = false;
		SetCurrentLevelInitialAlpha(0.f);
	}

	current_level = level;
	
	int current_level_int = (int)current_level - 1;
	current_level_initial = static_cast<ComponentImage*>(combo_counter_avaluations[current_level_int]->children[0]->GetComponent(Component::ComponentType::UI_IMAGE));
	
	current_level_word = static_cast<ComponentImage*>(combo_counter_avaluations[current_level_int]->children[1]->GetComponent(Component::ComponentType::UI_IMAGE));
	current_level_word->owner->transform.SetTranslation(float3::zero);

	combo_counter_progress_bar_border_go->transform.SetTranslation(float3(0.f, 100.f, 0.f));

	word_position = current_level_word->owner->transform_2d.GetWidth() / 2.f;
	initial_final_scale = current_level_initial->owner->transform_2d.GetScale().x;

	indicator_state = ComboCounterIndicatorState::INITIAL_TRANSITION;
	current_time = 0.f;
}

void ComboCounterIndicator::Hide()
{
	current_time = 0.f;
	indicator_state = ComboCounterIndicatorState::HIDING;
}

// Use this for showing variables on inspector
void ComboCounterIndicator::OnInspector(ImGuiContext* context)
{
	//Necessary to be able to write with imgui
	ImGui::SetCurrentContext(context);
	ShowDraggedObjects();

}

void ComboCounterIndicator::SetCurrentLevelInitialAlpha(float alpha)
{
	float4 initial_color = current_level_initial->color;
	initial_color.w = alpha;
	current_level_initial->SetColor(initial_color);
}

void ComboCounterIndicator::SetCurrentLevelBarAlpha(float alpha)
{
	float4 color = combo_counter_progress_bar_border->color;
	color.w = alpha;
	combo_counter_progress_bar_border->SetColor(color);

	color = combo_counter_progress_bar_background->color;
	color.w = alpha;
	combo_counter_progress_bar_background->SetColor(color);

	color = combo_counter_progress_bar_fill->color;
	color.w = alpha;
	combo_counter_progress_bar_fill->SetColor(color);
}
