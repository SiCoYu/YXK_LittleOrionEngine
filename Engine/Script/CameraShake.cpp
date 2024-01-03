#include "CameraShake.h"

#include "Component/ComponentScript.h"
#include "Component/ComponentTransform.h"

#include "Main/Application.h"
#include "Main/GameObject.h"
#include "Module/ModuleInput.h"
#include "Module/ModuleScene.h"
#include "Module/ModuleTime.h"


#include "imgui.h"

#include <Helper/PerlinNoise.h>


CameraShake* CameraShakeDLL()
{
	CameraShake* instance = new CameraShake();
	return instance;
}

CameraShake::CameraShake()
{
	
}

// Use this for initialization before Start()
void CameraShake::Awake()
{

}

// Use this for initialization
void CameraShake::Start()
{

}

// Update is called once per frame
void CameraShake::Update()
{
	if(shaking && trauma > 0.f)
	{
		//increase the time counter (how fast the position changes) based on the trauma_mult and some root of the Trauma
		time_counter += App->time->delta_time / 1000.f * math::Pow(trauma, 0.3f) * trauma_multiplier;
		
		//Bind the movement to the desired range
		float3 new_position = GenerateFloat3() * trauma_magnitude * trauma;
		owner->transform.SetTranslation(new_position);
		//rotation modifier applied here
		float3 rot = new_position * trauma_rotation_magnitude;
		owner->transform.SetRotation(math::Quat().FromEulerYXZ(rot.y, rot.x, rot.z));

		//decay faster at higher values
		trauma -= App->time->delta_time / 1000.f * trauma_decay * (math::Clamp01(trauma + 0.3f));
		trauma = math::Clamp01(trauma);
	}
	else
	{
		//lerp back towards default position and rotation once shake is done
		float3 new_position = float3::Lerp(owner->transform.GetTranslation(), float3::zero, App->time->delta_time / 1000.f);
		owner->transform.SetTranslation(new_position);
		float3 rot = new_position * trauma_rotation_magnitude;
		owner->transform.SetRotation(math::Quat().FromEulerYXZ(rot.y, rot.x, rot.z));
	}

}

// Use this for showing variables on inspector
void CameraShake::OnInspector(ImGuiContext* context)
{
	//Necessary to be able to write with imgui
	ImGui::SetCurrentContext(context);
	ShowDraggedObjects();

	ImGui::Text("VARIABLES:");
	ImGui::Separator();
	ImGui::Checkbox("Shake activated", &shaking);
	ImGui::DragFloat("trauma", &trauma, 0.01f, 0.0f, 1.0f);
	ImGui::DragFloat("trauma_multiplier", &trauma_multiplier);
	ImGui::DragFloat("trauma_magnitude", &trauma_magnitude);
	ImGui::DragFloat("trauma_rotation_magnitude", &trauma_rotation_magnitude);
	ImGui::DragFloat("trauma_depth_magnitud", &trauma_depth_magnitud);
	ImGui::DragFloat("trauma_decay", &trauma_decay);

}

//Use this for linking JUST GO automatically 
void CameraShake::InitPublicGameObjects()
{
	//IMPORTANT, public gameobjects, name_gameobjects and go_uuids MUST have same size

	for (int i = 0; i < public_gameobjects.size(); ++i)
	{
		name_gameobjects.push_back(is_object);
		go_uuids.push_back(0);
	}
}
//Use this for linking GO AND VARIABLES automatically if you need to save variables 
 void CameraShake::Save(Config& config) const
 {
	config.AddFloat(trauma_multiplier, "trauma_multiplier");
	config.AddFloat(trauma_magnitude, "trauma_magnitude");
	config.AddFloat(trauma_rotation_magnitude, "trauma_rotation_magnitude");
	config.AddFloat(trauma_depth_magnitud, "trauma_depth_magnitud");
	config.AddFloat(trauma_decay, "trauma_decay");

 	Script::Save(config);
 }

// //Use this for linking GO AND VARIABLES automatically
 void CameraShake::Load(const Config& config)
 {
	trauma_multiplier = config.GetFloat("trauma_multiplier", 16.f);
	trauma_magnitude = config.GetFloat("trauma_magnitude", 0.8f);
	trauma_rotation_magnitude = config.GetFloat("trauma_rotation_magnitude", 0.f);
	trauma_depth_magnitud = config.GetFloat("trauma_depth_magnitud", 0.6f);
	trauma_decay = config.GetFloat("trauma_decay", 1.3f);
 	Script::Load(config);
 }

 //Get Perlin float between -1 & 1, based on the time counter
 //Noise get a value between 0 & 1 and we mapped to -1, 1
 float CameraShake::GetFloat(float seed) const
 {
	 float vec[2]{ seed, time_counter };
	 
	 return (noise2(vec) - 0.5f) *2.f;
 }

 //Generate a random float3 using PerlinNoise, we use different seeds to ensure different numbers
 float3 CameraShake::GenerateFloat3()
 {
	 return float3(GetFloat(1.f),
				   GetFloat(10.f),
					GetFloat(100.f) * trauma_depth_magnitud);
 }
