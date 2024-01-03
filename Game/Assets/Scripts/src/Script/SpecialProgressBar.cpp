#include "SpecialProgressBar.h"

#include "Component/ComponentScript.h"
#include "Component/ComponentTransform.h"

#include "Main/Application.h"
#include "Main/GameObject.h"
#include "Module/ModuleInput.h"
#include "Module/ModuleScene.h"


#include "imgui.h"


SpecialProgressBar* SpecialProgressBarDLL()
{
	SpecialProgressBar* instance = new SpecialProgressBar();
	return instance;
}

SpecialProgressBar::SpecialProgressBar()
{
	
}

// Use this for initialization before Start()
void SpecialProgressBar::Awake()
{
	special_bar_orb_1_transform = &owner->children[0]->transform_2d;
	special_bar_orb_2_transform = &owner->children[1]->transform_2d;
	special_bar_orb_3_transform = &owner->children[2]->transform_2d;
	special_bar_orb_4_transform = &owner->children[3]->transform_2d;

	original_size = special_bar_orb_1_transform->size.x;
}

// Use this for initialization
void SpecialProgressBar::Start()
{

}

// Update is called once per frame
void SpecialProgressBar::Update()
{

}

void SpecialProgressBar::SetProgress(float percentage)
{
	float scaled_size;
	if (percentage < 0.25f)
	{
		scaled_size = percentage / 0.25f * original_size; 
		special_bar_orb_1_transform->SetSize(float2(scaled_size));
		special_bar_orb_2_transform->SetSize(float2(0.f));
		special_bar_orb_3_transform->SetSize(float2(0.f));
		special_bar_orb_4_transform->SetSize(float2(0.f));
	}
	else if (percentage >= 0.25f && percentage < 0.5f)
	{
		scaled_size = (percentage - 0.25f) / 0.25f * original_size;
		special_bar_orb_1_transform->SetSize(float2(original_size));
		special_bar_orb_2_transform->SetSize(float2(scaled_size));
		special_bar_orb_3_transform->SetSize(float2(0.f));
		special_bar_orb_4_transform->SetSize(float2(0.f));
	}
	else if (percentage >= 0.5f && percentage < 0.75f)
	{
		scaled_size = (percentage - 0.5f) / 0.25f * original_size;
		special_bar_orb_1_transform->SetSize(float2(original_size));
		special_bar_orb_2_transform->SetSize(float2(original_size));
		special_bar_orb_3_transform->SetSize(float2(scaled_size));
		special_bar_orb_4_transform->SetSize(float2(0.f));
	}
	else
	{
		scaled_size = (percentage - 0.75f) / 0.25f * original_size;
		special_bar_orb_1_transform->SetSize(float2(original_size));
		special_bar_orb_2_transform->SetSize(float2(original_size));
		special_bar_orb_3_transform->SetSize(float2(original_size));
		special_bar_orb_4_transform->SetSize(float2(scaled_size));
	}
}

// Use this for showing variables on inspector
void SpecialProgressBar::OnInspector(ImGuiContext* context)

{
	//Necessary to be able to write with imgui
	ImGui::SetCurrentContext(context);
	ShowDraggedObjects();
}

//Use this for linking JUST GO automatically 
void SpecialProgressBar::InitPublicGameObjects()
{
	//IMPORTANT, public gameobjects, name_gameobjects and go_uuids MUST have same size
	for (int i = 0; i < public_gameobjects.size(); ++i)
	{
		name_gameobjects.push_back(is_object);
		go_uuids.push_back(0);
	}
}