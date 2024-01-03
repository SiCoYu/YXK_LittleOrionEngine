#include "CinematicCamera.h"

#include "Component/ComponentCamera.h"
#include "Component/ComponentScript.h"
#include "Component/ComponentTransform.h"

#include "Log/EngineLog.h"

#include "Main/Application.h"
#include "Main/GameObject.h"
#include "Module/ModuleDebugDraw.h"
#include "Module/ModuleInput.h"
#include "Module/ModuleScene.h"
#include "Module/ModuleTime.h"

#include <imgui.h>

#include "CameraLine.h"
#include "WorldManager.h"


class CinematicAction
{
public:
	bool finished = false;
	float duration = 0.f; //ms
	float current_time = 0.f; //ms
	GameObject* camera = nullptr;

	virtual void Update(float delta_time) = 0;
};


class MoveAction : public CinematicAction
{
public:
	float3 target_position = float3::zero;
	float3 starting_position = float3::zero;


	MoveAction(const float3& target_position, const float3& starting_position, float duration, GameObject* camera)
	{
		this->target_position = target_position;
		this->starting_position = starting_position;
		this->duration = duration;
		this->camera = camera;
	}

	void Update(float delta_time) override
	{
		current_time += delta_time;
		if (current_time > duration)
		{
			finished = true;
			return;
		}

		float3 new_position = math::Lerp(starting_position, target_position, current_time / duration);
		camera->transform.SetGlobalMatrixTranslation(new_position);
	}
};

class RotateAction : public CinematicAction
{
public:

	RotateAction(const float3& rotation_degrees, const Quat& current_rotation, float duration, GameObject* camera)
	{
		this->rotation_degrees = rotation_degrees;
		this->duration = duration;
		this->current_rotation = current_rotation;
		this->camera = camera;

		float3 current_direction = camera->transform.GetFrontVector() + camera->transform.GetGlobalTranslation();
		direction = Quat::FromEulerXYZ(rotation_degrees.x, rotation_degrees.y, rotation_degrees.z) * current_direction;
		new_quat = Quat::LookAt(float3::unitZ, direction.Normalized(), float3::unitY, float3::unitY);
	}

	void Update(float delta_time) override
	{
		current_time += delta_time;
		if (current_time > duration)
		{
			finished = true;
			return;
		}

		Quat rot = current_rotation.Slerp(new_quat, current_time / duration);
		camera->transform.SetRotation(rot);
	}

	float3 rotation_degrees = float3::zero; //euler
	float3 direction = float3::zero;
	Quat current_rotation;
	Quat new_quat;
};

class LookAtAction : public CinematicAction
{
public:
	float3 target_to_look = float3::zero;
	float3 current_position = float3::zero;
	Quat look_quat;

	LookAtAction(const float3& target_to_look, const float3& current_position, float duration, GameObject* camera)
	{
		this->target_to_look = target_to_look;
		this->duration = duration;
		this->current_position = current_position;
		this->camera = camera;

		float3 direction = target_to_look - current_position;
		look_quat = Quat::LookAt(float3::unitZ, direction.Normalized(), float3::unitY, float3::unitY);	}

	void Update(float delta_time) override
	{
		current_time += delta_time;
		if (current_time > duration)
		{
			finished = true;
			return;
		}

		Quat rot = Quat::Slerp(camera->transform.GetRotation(), look_quat, delta_time * 0.004f);
		camera->transform.SetRotation(rot);
	}
};

class MoveWhileRotateAction : public CinematicAction
{
public:
	float3 target_position = float3::zero;
	float3 starting_position = float3::zero;
	float3 direction = float3::zero;
	Quat new_quat;
	Quat current_rotation;
	float3 rotation_degrees = float3::zero; //euler


	MoveWhileRotateAction(const float3& target_position, const float3& starting_position, const float3& rotation_degrees, const Quat& current_rotation, float duration, GameObject* camera)
	{
		this->target_position = target_position;
		this->starting_position = starting_position;
		this->rotation_degrees = rotation_degrees;
		this->duration = duration;
		this->current_rotation = current_rotation;
		this->camera = camera;

		float3 current_direction = camera->transform.GetFrontVector() + camera->transform.GetGlobalTranslation();
		direction = Quat::FromEulerXYZ(rotation_degrees.x, rotation_degrees.y, rotation_degrees.z) * current_direction;
		new_quat = Quat::LookAt(float3::unitZ, direction.Normalized(), float3::unitY, float3::unitY);
	}

	void Update(float delta_time) override
	{
		current_time += delta_time;
		if (current_time > duration)
		{
			finished = true;
			return;
		}


		Quat rot = math::Slerp(camera->transform.GetRotation(), new_quat, current_time / duration);
		camera->transform.SetGlobalMatrixRotation(rot);

		float3 new_position = math::Lerp(starting_position, target_position, current_time / duration);
		camera->transform.SetGlobalMatrixTranslation(new_position);

		float3 new_direction = direction - camera->transform.GetGlobalTranslation();
		new_quat = Quat::LookAt(float3::unitZ, new_direction.Normalized(), float3::unitY, float3::unitY);
	}
};


class MoveWhileLookAtAction : public CinematicAction
{
public:
	float3 target_position = float3::zero;
	float3 starting_position = float3::zero;
	float3 target_to_look = float3::zero;
	Quat look_quat;

	MoveWhileLookAtAction(const float3& target_position, const float3& starting_position, const float3& target_to_look, float duration, GameObject* camera)
	{
		this->target_position = target_position;
		this->starting_position = starting_position;
		this->target_to_look = target_to_look;
		this->duration = duration;
		this->camera = camera;

		float3 direction = target_to_look - starting_position;
		look_quat = Quat::LookAt(float3::unitZ, direction.Normalized(), float3::unitY, float3::unitY);

	}

	void Update(float delta_time) override
	{
		current_time += delta_time;
		if (current_time > duration)
		{
			finished = true;
			return;
		}

		Quat rot = Quat::Slerp(camera->transform.GetRotation(), look_quat, current_time / duration);
		camera->transform.SetRotation(rot);

		float3 new_position = math::Lerp(starting_position, target_position, current_time / duration);
		camera->transform.SetGlobalMatrixTranslation(new_position);

		float3 direction = target_to_look - camera->transform.GetGlobalTranslation();
		look_quat = Quat::LookAt(float3::unitZ, direction.Normalized(), float3::unitY, float3::unitY);
	}
};

class WaitAction : public CinematicAction
{
public:

	WaitAction(float duration, GameObject* camera)
	{
		this->duration = duration;
		this->camera = camera;
	}

	void Update(float delta_time) override
	{
		current_time += delta_time;
		if (current_time > duration)
		{
			finished = true;
			return;
		}
	}

};

class SetupCameraAction : public CinematicAction
{
public:
	float3 starting_position = float3::zero;
	Quat starting_rotation;

	SetupCameraAction(const float3& starting_position, const float3& starting_rotation_target, GameObject* camera)
	{
		this->camera = camera;
		this->starting_position = starting_position;


		float3 direction = starting_rotation_target - starting_position;
		this->starting_rotation = Quat::LookAt(float3::unitZ, direction.Normalized(), float3::unitY, float3::unitY);
	}

	void Update(float delta_time) override
	{

		camera->transform.SetGlobalMatrixTranslation(starting_position);
		camera->transform.SetGlobalMatrixRotation(starting_rotation);
		finished = true;
		return;
		
	}

};

CinematicCamera* CinematicCameraDLL()
{
	CinematicCamera* instance = new CinematicCamera();
	return instance;
}

CinematicCamera::CinematicCamera()
{

}

// Use this for initialization before Start()
void CinematicCamera::Awake()
{
	//Init Cinematic 1
	cinematics_go = App->scene->GetGameObjectByName("Cinematics");
	cinematic_camera = static_cast<ComponentCamera*>(owner->children[0]->GetComponent(Component::ComponentType::CAMERA));
	main_camera = static_cast<ComponentCamera*>(App->scene->GetGameObjectByName("Main Camera")->GetComponent(Component::ComponentType::CAMERA));

	world_manager = static_cast<WorldManager*>(App->scene->GetGameObjectByName("World Manager")->GetComponentScript("WorldManager")->script);
}



// Use this for initialization
void CinematicCamera::Start()
{
	SetMainCamera(false);
	CreateCinematics();
}


// Update is called once per frame
void CinematicCamera::Update()
{
	if(current_action == nullptr && !cinematic_queue.empty())
	{
		//Retrieve next element
		current_action = cinematic_queue.front();
	}

	//If action we call update
	if(current_action)
	{
		current_action->Update(App->time->delta_time);

		if(current_action->finished)
		{
			current_action = nullptr;
			cinematic_queue.erase(cinematic_queue.begin());

			if(cinematic_queue.empty())
			{
				world_manager->on_pause = false;
				SetMainCamera(false);
			}
		}
	}

}

// Use this for showing variables on inspector
void CinematicCamera::OnInspector(ImGuiContext* context)
{
	//Necessary to be able to write with imgui
	ImGui::SetCurrentContext(context);
	ShowDraggedObjects();

	int aux = static_cast<int>(cinematic_queue.size());
	ImGui::DragInt("Current state: %d", &aux);

	ImGui::Separator();
	if(current_action)
	{
		ImGui::Text("Current Time: %.3f", current_action->current_time);
		ImGui::Text("Duration: %.3f", current_action->duration);
	}
}

//Use this for linking JUST GO automatically 
void CinematicCamera::InitPublicGameObjects()
{
	//IMPORTANT, public gameobjects, name_gameobjects and go_uuids MUST have same size

	for (int i = 0; i < public_gameobjects.size(); ++i)
	{
		name_gameobjects.push_back(is_object);
		go_uuids.push_back(0);
	}
}

void CinematicCamera::SetMainCamera(bool cinematic) const
{
	if(!cinematic)
	{
		main_camera->depth = 5.f;
		cinematic_camera->depth = 0.f;
	}
	else
	{
		cinematic_camera->depth = 5.f;
		main_camera->depth = 0.f;
	}
}

//Call this method to trigger a cinematic by index
void CinematicCamera::PlayCinematic(size_t index)
{
	if(index >= cinematics.size())
	{
		App->engine_log->Log("Error, invalid cinematic index.");
		return;
	}
	world_manager->on_pause = true;
	cinematic_queue = cinematics[index];
	SetMainCamera(true);
}

bool CinematicCamera::IsPlayingCinematic() const
{
	return !cinematic_queue.empty();
}

void CinematicCamera::CreateCinematics()
{
	switch (world_manager->current_level)
	{
		case Level::FIRST: 
		{
			//First cinematic
			cinematic_rail = static_cast<CameraLine*>(cinematics_go->children[0]->GetComponentScript("CameraLine")->script);

			std::vector<CinematicAction*> cinematic1;
			cinematic1.push_back(new MoveAction(cinematic_rail->GetPosition(1),
				cinematic_rail->GetPosition(0), 4000.f, owner));
			cinematic1.push_back(new MoveAction(cinematic_rail->GetPosition(2),
				cinematic_rail->GetPosition(1), 2000.f, owner));

			cinematic1.push_back(new WaitAction(2000, owner));

			cinematics.emplace_back(cinematic1);

			////Second Cinematic 
			cinematic_rail = static_cast<CameraLine*>(cinematics_go->children[1]->GetComponentScript("CameraLine")->script);
			std::vector<CinematicAction*> cinematic2;

			cinematic2.push_back(new SetupCameraAction(cinematic_rail->GetPosition(0), cinematic_rail->GetPosition(3), owner));
			cinematic2.push_back(new WaitAction(1000, owner));
			cinematic2.push_back(new MoveWhileLookAtAction(cinematic_rail->GetPosition(1),
				cinematic_rail->GetPosition(0), cinematic_rail->GetPosition(2), 2000.f, owner));

			cinematic2.push_back(new WaitAction(2000, owner));

			cinematics.emplace_back(cinematic2);
			break;
		}

		case Level::SECOND:
			break;

		case Level::BOSS:
			break;

		default:
			break;
	}
}

