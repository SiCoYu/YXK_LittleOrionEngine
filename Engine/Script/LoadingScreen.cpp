#include "LoadingScreen.h"

#include "Component/ComponentScript.h"
#include "Component/ComponentText.h"
#include "Component/ComponentTransform.h"

#include "Main/Application.h"
#include "Main/GameObject.h"
#include "Module/ModuleAnimation.h"
#include "Module/ModuleInput.h"
#include "Module/ModuleScene.h"
#include "Module/ModuleResourceManager.h"

#include "EditorUI/Panel/InspectorSubpanel/PanelComponent.h"

#include "imgui.h"

#include "ProgressBar.h"

LoadingScreen* LoadingScreenDLL()
{
	LoadingScreen* instance = new LoadingScreen();
	return instance;
}

LoadingScreen::LoadingScreen()
{
	panel = new PanelComponent();
}

// Use this for initialization before Start()
void LoadingScreen::Start()
{
	ComponentTransform2D* bunny_1 = &App->scene->GetGameObjectByName("bunny_1")->transform_2d;
	ComponentTransform2D* bunny_2 = &App->scene->GetGameObjectByName("bunny_2")->transform_2d;
	ComponentTransform2D* bunny_3 = &App->scene->GetGameObjectByName("bunny_3")->transform_2d;
	ComponentTransform2D* bunny_4 = &App->scene->GetGameObjectByName("bunny_4")->transform_2d;
	ComponentTransform2D* bunny_5 = &App->scene->GetGameObjectByName("bunny_5")->transform_2d;
	ComponentTransform2D* bunny_6 = &App->scene->GetGameObjectByName("bunny_6")->transform_2d;
	ComponentTransform2D* bunny_7 = &App->scene->GetGameObjectByName("bunny_7")->transform_2d;
	ComponentTransform2D* bunny_8 = &App->scene->GetGameObjectByName("bunny_8")->transform_2d;
	
	float2 bunny_1_init_pos = bunny_1->anchored_position;
	float2 bunny_2_init_pos = bunny_2->anchored_position;
	float2 bunny_3_init_pos = bunny_3->anchored_position;
	float2 bunny_4_init_pos = bunny_4->anchored_position;
	float2 bunny_5_init_pos = bunny_5->anchored_position;
	float2 bunny_6_init_pos = bunny_6->anchored_position;
	float2 bunny_7_init_pos = bunny_7->anchored_position;
	float2 bunny_8_init_pos = bunny_8->anchored_position;

	percentatge_text = static_cast<ComponentText*>(App->scene->GetGameObjectByName("Percentatge Text")->GetComponent(Component::ComponentType::UI_TEXT));
	hints_text = static_cast<ComponentText*>(App->scene->GetGameObjectByName("Hints")->GetComponent(Component::ComponentType::UI_TEXT));

	InitHints();
	hints_text->text = hints[current_hint_index];

	sequence = App->animations->CreateTweenSequence();
	
	sequence->Append(Tween::LOTranslate(bunny_1, float2(bunny_1_init_pos.x, bunny_1_init_pos.y + 50.f), 0.75f)->SetLoops(-1, Tween::TweenLoop::YOYO)->SetEase(Tween::EaseType::EASE_OUT_SINE));
	sequence->Insert(0.5f, Tween::LOTranslate(bunny_2, float2(bunny_2_init_pos.x, bunny_2_init_pos.y + 50.f), 0.75f)->SetLoops(-1, Tween::TweenLoop::YOYO)->SetEase(Tween::EaseType::EASE_OUT_SINE));
	sequence->Insert(0.75f, Tween::LOTranslate(bunny_3, float2(bunny_3_init_pos.x, bunny_3_init_pos.y + 50.f), 0.75f)->SetLoops(-1, Tween::TweenLoop::YOYO)->SetEase(Tween::EaseType::EASE_OUT_SINE));
	sequence->Insert(1.0f, Tween::LOTranslate(bunny_4, float2(bunny_4_init_pos.x, bunny_4_init_pos.y + 50.f), 0.75f)->SetLoops(-1, Tween::TweenLoop::YOYO)->SetEase(Tween::EaseType::EASE_OUT_SINE));
	sequence->Insert(1.25f, Tween::LOTranslate(bunny_5, float2(bunny_5_init_pos.x, bunny_5_init_pos.y + 50.f), 0.75f)->SetLoops(-1, Tween::TweenLoop::YOYO)->SetEase(Tween::EaseType::EASE_OUT_SINE));
	sequence->Insert(1.5f, Tween::LOTranslate(bunny_6, float2(bunny_6_init_pos.x, bunny_6_init_pos.y + 50.f), 0.75f)->SetLoops(-1, Tween::TweenLoop::YOYO)->SetEase(Tween::EaseType::EASE_OUT_SINE));
	sequence->Insert(1.75f, Tween::LOTranslate(bunny_7, float2(bunny_7_init_pos.x, bunny_7_init_pos.y + 50.f), 0.75f)->SetLoops(-1, Tween::TweenLoop::YOYO)->SetEase(Tween::EaseType::EASE_OUT_SINE));
	sequence->Insert(2.0f, Tween::LOTranslate(bunny_8, float2(bunny_8_init_pos.x, bunny_8_init_pos.y + 50.f), 0.75f)->SetLoops(-1, Tween::TweenLoop::YOYO)->SetEase(Tween::EaseType::EASE_OUT_SINE));

	sequence->Play();
}

// Update is called once per frame
void LoadingScreen::Update()
{
	float current_loaded_resources = App->resources->loading_thread_communication.current_number_of_resources_loaded;
	float total_loaded_resources = App->resources->loading_thread_communication.total_number_of_resources_to_load;
	float percentatge = (current_loaded_resources / total_loaded_resources) * 100.f;

	int percentatge_to_render = static_cast<int>(percentatge);
	percentatge_text->SetText(std::to_string(percentatge_to_render));

	UpdateHints();
}

// Use this for showing variables on inspector
void LoadingScreen::OnInspector(ImGuiContext* context)
{
	//Necessary to be able to write with imgui
	ImGui::SetCurrentContext(context);
	ShowDraggedObjects();

}

//Use this for linking JUST GO automatically 
void LoadingScreen::InitPublicGameObjects()
{
	//IMPORTANT, public gameobjects, name_gameobjects and go_uuids MUST have same size

	for (int i = 0; i < public_gameobjects.size(); ++i)
	{
		name_gameobjects.push_back(is_object);
		go_uuids.push_back(0);
	}
}

void LoadingScreen::InitHints()
{
	hints.push_back("Use attack buttons on the air to make an aerial attack!");
	hints.push_back("Dashing through an attack makes you invulnerable!");
	hints.push_back("There are two different combos a light combo presing (X) and a heavy combo pressing(Y)!");
	hints.push_back("Bring nibery's heart where it belongs! The sacred tree!");
}

void LoadingScreen::UpdateHints()
{
	hints_buffer += App->time->real_time_delta_time / 1000.f;

	if (hints_buffer >= hints_change_time)
	{
		hints_buffer = 0;
		
		current_hint_index += 1;
		if (current_hint_index >= hints.size())
		{
			current_hint_index = 0;
		}

		hints_text->text = hints[current_hint_index];
	}
}
