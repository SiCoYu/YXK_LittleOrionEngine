#include "DamageIndicatorSpawner.h"

#include "Component/ComponentCamera.h"
#include "Component/ComponentCanvas.h"
#include "Component/ComponentScript.h"
#include "Component/ComponentTransform.h"

#include "Log/EngineLog.h"

#include "Main/Application.h"
#include "Main/GameObject.h"
#include "Module/ModuleCamera.h"
#include "Module/ModuleInput.h"
#include "Module/ModuleScene.h"
#include "Module/ModuleUI.h"


#include "DamageIndicator.h"
#include "PlayerAttack.h"

#include "imgui.h"

DamageIndicatorSpawner* DamageIndicatorSpawnerDLL()
{
	DamageIndicatorSpawner* instance = new DamageIndicatorSpawner();
	return instance;
}

DamageIndicatorSpawner::DamageIndicatorSpawner()
{
	
}

// Use this for initialization before Start()
void DamageIndicatorSpawner::Awake()
{

}

// Use this for initialization
void DamageIndicatorSpawner::Start()
{
	GameObject* damage_indicator_game_object = owner->children[0];
	for (unsigned int i = 0; i < damage_indicators.size(); ++i)
	{
		GameObject* damage_indicator_clone = App->scene->DuplicateGameObject(damage_indicator_game_object, owner);
		damage_indicators[i] = static_cast<DamageIndicator*>(damage_indicator_clone->GetComponentScript("DamageIndicator")->script);
	}
}

// Update is called once per frame
void DamageIndicatorSpawner::Update()
{
}

void DamageIndicatorSpawner::SpawnDamageIndicator(int damage, const float3& receiver_position, const float3& agressor_position)
{
	float4 position_float4 = float4(receiver_position, 1.f);
	float4 clip_coordinates = App->cameras->main_camera->GetClipMatrix() * position_float4;
	float3 device_coordinates = clip_coordinates.xyz() / clip_coordinates.w;
	float2 receiver_canvas_position = float2
	(
		device_coordinates.x * App->ui->main_canvas->GetCanvasScreenSize().x / 2.f,
		device_coordinates.y * App->ui->main_canvas->GetCanvasScreenSize().y / 2.f
	);

	position_float4 = float4(agressor_position, 1.f);
	clip_coordinates = App->cameras->main_camera->GetClipMatrix() * position_float4;
	device_coordinates = clip_coordinates.xyz() / clip_coordinates.w;
	float2 agressor_canvas_position = float2
	(
		device_coordinates.x * App->ui->main_canvas->GetCanvasScreenSize().x / 2.f,
		device_coordinates.y * App->ui->main_canvas->GetCanvasScreenSize().y / 2.f
	);

	float power = math::Min((damage - MIN_DAMAGE) / (MAX_DAMAGE - MIN_DAMAGE), 1.5f);

	DamageIndicator* damage_indicator = GetAvailableDamageIndicator();
	if (damage_indicator != nullptr)
	{
		damage_indicator->Spawn(damage, power, receiver_canvas_position, (receiver_canvas_position - agressor_canvas_position).Normalized());
	}
}

DamageIndicator* DamageIndicatorSpawner::GetAvailableDamageIndicator()
{
	for (auto& damage_indicator : damage_indicators)
	{
		if (!damage_indicator->IsAlive())
		{
			return damage_indicator;
		}
	}

	return nullptr;
}

// Use this for showing variables on inspector
void DamageIndicatorSpawner::OnInspector(ImGuiContext* context)
{
	//Necessary to be able to write with imgui
	ImGui::SetCurrentContext(context);
	ShowDraggedObjects();
}

//Use this for linking JUST GO automatically 
void DamageIndicatorSpawner::InitPublicGameObjects()
{
	//IMPORTANT, public gameobjects, name_gameobjects and go_uuids MUST have same size
	for (int i = 0; i < public_gameobjects.size(); ++i)
	{
		name_gameobjects.push_back(is_object);
		go_uuids.push_back(0);
	}
}

//Use this for linking GO AND VARIABLES automatically if you need to save variables 
// void DamageIndicatorSpawner::Save(Config& config) const
// {
// 	config.AddUInt(example->UUID, "ExampleNameforSave");
// 	Script::Save(config);
// }

// //Use this for linking GO AND VARIABLES automatically
// void DamageIndicatorSpawner::Load(const Config& config)
// {
// 	exampleUUID = config.GetUInt("ExampleNameforSave", 0);
// 	Script::Load(config);
// }