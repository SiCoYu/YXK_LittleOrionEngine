#include "ProgressBar.h"

#include "Component/ComponentScript.h"
#include "Component/ComponentTransform.h"
#include "Component/ComponentTransform2D.h"

#include "Main/Application.h"
#include "Main/GameObject.h"
#include "Module/ModuleInput.h"
#include "Module/ModuleScene.h"


#include "imgui.h"


ProgressBar* ProgressBarDLL()
{
	ProgressBar* instance = new ProgressBar();
	return instance;
}

ProgressBar::ProgressBar()
{
	
}

// Use this for initialization before Start()
void ProgressBar::Awake()
{

	fill_bar_game_object = owner->children[0];
	fill_bar_transform = &fill_bar_game_object->transform_2d;
	progress_bar_transform = &owner->transform_2d;

}

// Use this for initialization
void ProgressBar::Start()
{
	SetProgress(progress);
}

// Update is called once per frame
void ProgressBar::Update()
{
}

void ProgressBar::SetProgress(float new_progress)
{
	if (new_progress < 0.f || new_progress > 1.f)
	{
		return;
	}

	progress = new_progress;
	float progress_bar_width = progress_bar_transform->GetWidth();
	float progress_bar_height = progress_bar_transform->GetHeight();
	
	float fill_bar_padding = 0;

	if (!vertical)
	{
		fill_bar_padding = progress_bar_width * (1 - progress);
	}
	else 
	{
		fill_bar_padding = progress_bar_height * (1 - progress);
	}
	float3 translation = fill_bar_transform->GetTranslation();

	float sign_value = (inverted) ? 1.f : -1.f;

	if (vertical)
	{
		fill_bar_transform->SetTranslation(float3(translation.x, sign_value * fill_bar_padding, translation.z));
	}
	else
	{
		fill_bar_transform->SetTranslation(float3(sign_value * fill_bar_padding, translation.y, translation.z));
	}
}

// Use this for showing variables on inspector
void ProgressBar::OnInspector(ImGuiContext* context)
{
	//Necessary to be able to write with imgui
	ImGui::SetCurrentContext(context);
	ShowDraggedObjects();

	ImGui::Checkbox("Inverted", &inverted);
	ImGui::Checkbox("Vertical", &vertical);
}

void ProgressBar::Save(Config& config) const
{
	config.AddBool(inverted, "Inverted");
	config.AddBool(vertical, "Vertical");
	Script::Save(config);
}

void ProgressBar::Load(const Config& config)
{
	inverted = config.GetBool("Inverted", false);
	vertical = config.GetBool("Vertical", false);
	Script::Load(config);
}