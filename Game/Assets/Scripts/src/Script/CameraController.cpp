#include "CameraController.h"

#include "Component/ComponentScript.h"
#include "Component/ComponentTransform.h"
#include "Component/ComponentCamera.h"

#include "Main/Application.h"
#include "Main/GameObject.h"
#include "Module/ModuleCamera.h"
#include "Module/ModuleDebugDraw.h"
#include "Module/ModuleInput.h"
#include "Module/ModuleScene.h"
#include "Module/ModuleTime.h"

#include "imgui.h"
#include <math.h>

#include "CameraLine.h"
#include "DebugModeScript.h"
#include "PlayerController.h"
#include "WorldManager.h"

namespace
{
	const float MAX_PLAYER_DISTANCE = 10.0f;
}
CameraController* CameraControllerDLL()
{
	CameraController* instance = new CameraController();
	return instance;
}

CameraController::CameraController()
{
}

// Use this for initialization before Start()
void CameraController::Awake()
{
	GameObject* world_manager_go = App->scene->GetGameObjectByName("World Manager");
	ComponentScript* world_manager_component = world_manager_go->GetComponentScript("WorldManager");
	world_manager = static_cast<WorldManager*>(world_manager_component->script);
}

// Use this for initialization
void CameraController::Start()
{
	player_1 = world_manager->GetPlayer1();
	player_2 = world_manager->GetPlayer2();
	if(world_manager->multiplayer)
	{
		center_time_position = center_time_multiplayer;
	}
	else
	{
		center_time_position = center_time_singleplayer;
	}
	owner->transform.SetGlobalMatrixTranslation(camera_rail->GetPoint(0));
	owner->transform.LookAt(camera_focus->GetPoint(0));

}

// Update is called once per frame
void CameraController::Update()
{
	if(world_manager->on_pause)
	{
		return;
	}

	if(debug_options)
	{
		level_path->DrawLine();
		camera_focus->DrawLine();
		camera_rail->DrawLine();
	}

	if(!freeze)
	{
		progress_level = level_path->GetProgress(GetPositionToProgress(), debug_options);
		current_look_at = camera_focus->GetPoint(previous_progress);
		current_position = camera_rail->GetPoint(previous_progress);
		if(progress_level > previous_progress)
		{
			is_focusing = true;
			current_time = 0;
			previous_progress = progress_level;
			previous_look_at = current_look_at;
			previous_position = current_position;
		}
		else if (progress_level < previous_progress)
		{
			is_focusing = true;
			current_time = 0;
			previous_progress = progress_level;
			previous_look_at = current_look_at;
			previous_position = current_position;
		}
		if(is_focusing)
		{
			current_time += App->time->delta_time;
			Focus(current_position);
		}
		RotateFocus(current_look_at);
	}
}

// Use this for showing variables on inspector
void CameraController::OnInspector(ImGuiContext* context)
{
	//Necessary to be able to write with imgui
	ImGui::SetCurrentContext(context);
	ImGui::DragFloat("Progress", &progress_level, 0.01f, 0.f, 1.f);
	ImGui::DragFloat("Previous Progress", &previous_progress, 0.01f, 0.f, 1.f);
	ImGui::DragFloat("Smoothing time", &smoothing_time, 0.001f, 0.f, 1.f);

	ImGui::Text("Camera Controller Inspector");
	//Example to Drag and drop and link GOs in the Editor, Unity-like (WIP)
	ImGui::Text("Variables: ");
	ShowDraggedObjects();
	ImGui::Checkbox("Is Focusing", &is_focusing);

	ImGui::DragFloat("Max Distance Players", &max_distance_between_players, 0.5f, 0.f, 100.f);
	ImGui::Checkbox("Freeze", &freeze);

	ImGui::DragFloat("Current time", &current_time);
	ImGui::DragFloat("Focus Progress", &focus_progress);
}

void CameraController::ActivePlayer()
{
	if (god_mode) 
	{
		player_1->player_component->Disable();
		player_2->player_component->Disable();
		
	}
	else 
	{
		player_1->player_component->Enable();
		player_2->player_component->Enable();
	}
}

void CameraController::Focus(const float3& position_to_go)
{
	float distance_to_closest_player = GetClosestPlayer()->transform.GetGlobalTranslation().Distance(owner->transform.GetGlobalTranslation());
	focus_progress = distance_to_closest_player > MAX_PLAYER_DISTANCE ? current_time / center_time_singleplayer  :current_time / center_time_position;
	float3 new_camera_position = Quat::SlerpVector(previous_position, position_to_go, focus_progress);
	owner->transform.SetGlobalMatrixTranslation(new_camera_position);
	is_focusing = focus_progress < 1;
}

void CameraController::RotateFocus(const float3& position_to_look_at) const
{
	float3 desired_direction = position_to_look_at - owner->transform.GetGlobalTranslation();
	Quat desired_rotation = Quat::LookAt(float3::unitZ, desired_direction.Normalized(), float3::unitY, float3::unitY);
	Quat new_rotation = Quat::Slerp(owner->transform.GetRotation(), desired_rotation, App->time->delta_time * smoothing_time);
	owner->transform.SetRotation(new_rotation);
}

float3 CameraController::GetPositionToProgress() const
{
	if(!world_manager->multiplayer)
	{
		return player_1->player_go->transform.GetGlobalTranslation();
	}
	else
	{
		return GetClosestPlayer()->transform.GetGlobalTranslation();
	}

}

GameObject* CameraController::GetClosestPlayer() const
{
	float progress_level_player1 = level_path->GetProgress(player_1->player_go->transform.GetGlobalTranslation());
	float progress_level_player2 = level_path->GetProgress(player_2->player_go->transform.GetGlobalTranslation());

	if(progress_level_player1 > progress_level_player2)
	{
		return player_2->player_go;
	}
	else
	{
		return player_1->player_go;
	}
}

void CameraController::UpdateCameraLines(CameraLine* next_camera_rail, CameraLine* next_camera_focus, CameraLine* next_level_path)
{
	camera_rail = next_camera_rail;
	camera_focus = next_camera_focus;
	level_path = next_level_path;

	previous_progress = 0.f;
	progress_level = 0.f;
}
