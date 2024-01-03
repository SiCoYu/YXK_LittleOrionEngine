
#include "PlayerMovement.h"

#include "Component/ComponentAudioSource.h"
#include "Component/ComponentAnimation.h"
#include "Component/ComponentCamera.h"
#include "Component/ComponentCollider.h"
#include "Component/ComponentMeshRenderer.h"
#include "Component/ComponentParticleSystem.h"
#include "Component/ComponentScript.h"
#include "Component/ComponentTransform.h"

#include "Log/EngineLog.h"
#include "Main/Application.h"
#include "Main/GameObject.h"
#include "Module/ModuleAI.h"
#include "Module/ModuleCamera.h"
#include "Module/ModuleDebugDraw.h"
#include "Module/ModuleInput.h"
#include "Module/ModulePhysics.h"
#include "Module/ModuleScene.h"
#include "Module/ModuleTime.h"

#include "imgui.h"

#include "BossController.h"
#include "CameraController.h"
#include "DebugModeScript.h"
#include "EnemyController.h"
#include "EnemyManager.h"
#include "InputManager.h"
#include "PlayerController.h"
#include "WorldManager.h"

#include <algorithm>

namespace
{
	const float max_vertical_velocity = 15.f;
}


PlayerMovement* PlayerMovementDLL()
{
	PlayerMovement* instance = new PlayerMovement();
	return instance;
}

PlayerMovement::PlayerMovement()
{
	
}

// Use this for initialization before Start()
void PlayerMovement::Awake()
{
	game_camera = (ComponentCamera*)camera->children[0]->GetComponent(Component::ComponentType::CAMERA);
	camera_controller = (CameraController*)camera->GetComponentScript("CameraController")->script;

	audio_source = static_cast<ComponentAudioSource*>(owner->GetComponent(Component::ComponentType::AUDIO_SOURCE));
	animation = static_cast<ComponentAnimation*>(owner->GetComponent(Component::ComponentType::ANIMATION));

	GameObject* input_go = App->scene->GetGameObjectByName("Input Manager");
	const ComponentScript* input_component = input_go->GetComponentScript("InputManager");
	input_manager = (InputManager*)input_component->script;

	GameObject* debug_system = App->scene->GetGameObjectByName("DebugSystem");
	const ComponentScript* component_debug = debug_system->GetComponentScript("DebugModeScript");
	debug = (DebugModeScript*)component_debug->script;

	GameObject* enemy_go = App->scene->GetGameObjectByName("EnemyManager");
	enemy_manager = static_cast<CEnemyManager*>(enemy_go->GetComponentScript("EnemyManager")->script);

	mesh_collider = App->scene->GetGameObjectsWithTag("EnvironmentCollider");

	for (auto& child : owner->children)
	{
		if (child->name == "PS_footsteps")
		{
			player_footsteps = static_cast<ComponentParticleSystem*>(child->GetComponent(Component::ComponentType::PARTICLE_SYSTEM));
			player_footsteps->Stop();
		}
	}
	
	horizontal_ray_detector_middle = new RayHit();
	horizontal_ray_detector_knee = new RayHit();

	main_vertical_ray_detector = new RayHit();
	border_vertical_ray_detector = new RayHit();
	main_diagonal_ray_detector = new RayHit();
}

// Use this for initialization
void PlayerMovement::Start()
{
	GameObject* world_go = App->scene->GetGameObjectByName("World Manager");
	ComponentScript* world_component = world_go->GetComponentScript("WorldManager");
	world = static_cast<WorldManager*>(world_component->script);

	if (world->ThereIsBoss())
	{
		std::vector<GameObject*> obstacles = App->scene->GetGameObjectsWithTag("Obstacle");
		for (auto& obstacle : obstacles) 
		{
			boss_zones.emplace_back(static_cast<ComponentMeshRenderer*>(obstacle->GetComponent(Component::ComponentType::MESH_RENDERER)));
		}
	}
}

// Use this for showing variables on inspector
void PlayerMovement::OnInspector(ImGuiContext* context)
{
	//Necessary to be able to write with imgui
	ImGui::SetCurrentContext(context);


	ImGui::TextColored(ImVec4(1.f, 1.f, 0.f, 1.f), "Variables: ");
	ShowDraggedObjects();

	ImGui::TextColored(ImVec4(0.f, 1.f, 1.f, 1.f), "Camera Variables");
	ImGui::Checkbox("Is Inside Frustum", &is_inside);
	ImGui::Checkbox("Future AABB", &visualize_future_aabb);
	ImGui::DragFloat3("Device Coordinates", device_coordinates.ptr());

	ImGui::Separator();

	ImGui::TextColored(ImVec4(0.f, 1.f, 1.f, 1.f), "Movement");
	ImGui::DragFloat3("Velocity", velocity.ptr());
	ImGui::DragFloat("Acceleration", &acceleration, 0.001F, 0.001F, 0.1F);
	ImGui::DragFloat("Deceleration", &deceleration, 0.001F, 0.001F, 0.1F);
	ImGui::DragFloat("Smooth Rotation", &smooth_rotation, 0.001f, 0.001f, 1.f);
	ImGui::DragFloat3("Direction", direction.ptr());
	ImGui::Checkbox("Grounded", &grounded);
	ImGui::DragFloat("Vertical Threshold", &vertical_threshold, 0.001f, 0.f, 1.f);
	ImGui::DragFloat("Minimum Y", &minimum_y, 1.f, -10.f, 50.f);

	ImGui::Checkbox("Main Vertical Ray", &main_vertical_ray);
	ImGui::Checkbox("Border Vertical Ray", &border_vertical_ray);

	ImGui::Separator();

	ImGui::TextColored(ImVec4(0.f, 1.f, 1.f, 1.f), "Vertical Movement");
	ImGui::Spacing();
	ImGui::DragFloat("Gravity", &gravity);
	ImGui::DragFloat("Jump Speed", &jump_speed);


	ImGui::Separator();

	ImGui::TextColored(ImVec4(0.f, 1.f, 1.f, 1.f), "Horizontal Movement");
	ImGui::Spacing();
	ImGui::DragFloat("Horizontal Speed", &speed_horizontal);

	ImGui::Separator();
	ImGui::Checkbox("Is Falling", &is_falling);

	ImGui::Checkbox("Is Knocked", &is_knocked);
}

bool PlayerMovement::IsGrounded() const
{
	return grounded;
}

void PlayerMovement::Move(int player, bool is_stunned)
{
	direction = float3::zero;

	PlayerID player_id = static_cast<PlayerID>(player);
	GetInputDirection(player, direction);
	if (is_stunned || is_knocked) 
	{
		direction = float3::zero;
	}
	animation->SetFloat("horizontal_velocity", direction.xz().Length());

	if (direction.xz().Length() != 0)
	{
		RotatePlayer(direction);
	}

	ComputeHorizontalVelocity(direction, speed_horizontal);

	if (!is_stunned && input_manager->GetGameInputDown("Jump", player_id) && !input_manager->GetGameInputDown("Dash", player_id))
	{
		if (IsGrounded() && !jump_blocked)
		{
			is_jumping = true;
			velocity.y = math::Clamp(velocity.y + jump_speed, -1000.f, max_vertical_velocity);
			animation->SetBool("is_jumping", is_jumping);
		}
	}

	ComputeVerticalVelocity(float3::zero, 0, true);

	animation->SetFloat("vertical_velocity", velocity.y);

	if(is_falling)
	{
		is_jumping = false;
		animation->SetBool("is_jumping", is_jumping);
	}
	if (!animation->IsOnState("Idle") && !animation->IsOnState("Dead") && !animation->IsOnState("Stunned") && !animation->IsOnState("Falling") && !animation->IsOnState("Jump"))
	{
		if (!player_footsteps->IsPlaying())
		{
			player_footsteps->Enable();
			player_footsteps->Play();
		}
	}
	else
	{
		player_footsteps->Stop();
	}
	//Sound
	if (direction.xz().Length() > 0 && grounded)
	{
		// Animation has 75 frames, steps at 15, 35, 50 and 70 frames
		float current_percentage = animation->GetCurrentClipPercentatge();
		if ((int)(current_percentage * 100) % 15 == 0 && current_percentage <= 0.94)
		{
			audio_source->PlayEvent("play_footstep_player");
			
		}
	}

	ApplyVelocity();
}

void PlayerMovement::MoveIntoDirection(float3 direction, float speed)
{
	float3 normalized_direction = direction.Normalized();
	ComputeHorizontalVelocity(float3(normalized_direction.x, 0.f, normalized_direction.z), speed);
	ComputeVerticalVelocity(float3(0.f, normalized_direction.y, 0.f), speed, true);

	ApplyVelocity();
}

void PlayerMovement::GetInputDirection(int player_id, float3& my_direction)
{
	my_direction.x = input_manager->GetHorizontal(static_cast<PlayerID>(player_id));
	my_direction.z = input_manager->GetVertical(static_cast<PlayerID>(player_id));

	Plane xz_plane(float3::zero, float3::unitY);

	float3 main_camera_front = App->cameras->main_camera->GetFrustum().front;
	float3 projected_front = xz_plane.Project(main_camera_front);
	float3 movement_foward = projected_front.ScaledToLength(-my_direction.z);

	float3 main_camera_right = App->cameras->main_camera->GetFrustum().WorldRight();
	float3 projected_right = xz_plane.Project(main_camera_right);
	float3 movement_right = projected_right.ScaledToLength(my_direction.x);

	my_direction = movement_right + movement_foward;
}

void PlayerMovement::RotatePlayer(const float3& direction_to_rotate)
{
	float3 desired_direction = float3(direction_to_rotate.x, 0, direction_to_rotate.z);
	Quat desired_rotation = Quat::LookAt(float3::unitZ, desired_direction.Normalized(), float3::unitY, float3::unitY);
	Quat new_rotation = Quat::Slerp(owner->transform.GetRotation(), desired_rotation, App->time->delta_time * smooth_rotation);
	owner->transform.SetRotation(new_rotation);
}

void PlayerMovement::RotatePlayer(const float3& direction_to_rotate, const float& smooth_rotation_factor)
{
	float3 desired_direction = float3(direction_to_rotate.x, 0, direction_to_rotate.z);
	Quat desired_rotation = Quat::LookAt(float3::unitZ, desired_direction.Normalized(), float3::unitY, float3::unitY);
	Quat new_rotation = Quat::Slerp(owner->transform.GetRotation(), desired_rotation, App->time->delta_time * smooth_rotation_factor);
	owner->transform.SetRotation(new_rotation);
}

void PlayerMovement::SetHorizontalSpeed(float speed)
{
	speed_horizontal = speed;
}

void PlayerMovement::SlowPlayer(float percentatge)
{
	percentatge = math::Clamp(percentatge, 0.f, 100.f);
	speed_horizontal = default_speed_horizontal - default_speed_horizontal * percentatge * 0.01f;
}

void PlayerMovement::BlockJump(bool block)
{
	jump_blocked = block;
}

void PlayerMovement::ResetKnockup()
{
	velocity.y = 0.f;
	is_knocked = false;
}

void PlayerMovement::ApplyVelocity()
{
	float3 next_position = owner->transform.GetGlobalTranslation() + velocity * App->time->delta_time * 0.001f;

	bool can_move = true;

	can_move = IsInside(next_position);

	if (world->multiplayer)
	{
		float3 player_1_position = next_position;
		float3 player_2_position = other_player->transform.GetGlobalTranslation();
		float distance = (player_2_position - player_1_position).Length();

		can_move = can_move && distance < camera_controller->max_distance_between_players;
	}
	if (!can_move)
	{
		velocity.x = 0;
		velocity.z = 0;
		if (grounded)
		{
			velocity.y = 0;
			is_jumping = false;
			animation->SetBool("is_jumping", is_jumping);
		}
		next_position = owner->transform.GetGlobalTranslation() + velocity * App->time->delta_time * 0.001f;
	}
	animation->SetFloat("horizontal_velocity", velocity.xz().Length());
	animation->SetFloat("vertical_velocity", velocity.y);
	SetNewPosition(next_position);

}

void PlayerMovement::ComputeHorizontalVelocity(float3 direction, float speed)
{
	bool there_is_frontal_collision = DetectCollisions(0) || DetectCollisions(2);

	if (direction.Length() > 0 && !there_is_frontal_collision)
	{
		float3 velocity_tmp = direction.ScaledToLength(speed);
		if (velocity != velocity_tmp && acceleration_once && !is_dashing && !is_knocked)
		{
			velocity.x = ComputeSpeedWithAcceleration(velocity_tmp.x, velocity.x, App->time->delta_time * acceleration);
			velocity.z = ComputeSpeedWithAcceleration(velocity_tmp.z, velocity.z, App->time->delta_time * acceleration);
		}
		else
		{
			acceleration_once = false;
			velocity.x = velocity_tmp.x;
			velocity.z = velocity_tmp.z;
		}
		
	}
	else if (direction.Length() <= 0 && !there_is_frontal_collision && !is_dashing && !is_knocked)
	{
		velocity.x = ComputeSpeedWithDeAcceleration(0, velocity.x, App->time->delta_time * deceleration);
		velocity.z = ComputeSpeedWithDeAcceleration(0, velocity.z, App->time->delta_time * deceleration);
		acceleration_once = true;
	}
	else
	{
		velocity.x = 0.0f;
		velocity.z = 0.0f;
	}
}

float PlayerMovement::ComputeSpeedWithAcceleration(float final_speed, float current_speed, float dtime)
{
	float difference = final_speed - current_speed;
	if (difference > dtime)
	{
		return current_speed + dtime;
	}
	if (difference < -dtime)
	{
		return current_speed - dtime;
	}
	return final_speed;
}

float PlayerMovement::ComputeSpeedWithDeAcceleration(float final_speed, float current_speed, float dtime)
{
	float difference = current_speed - final_speed;
	if (difference > dtime)
	{
		return current_speed - dtime;
	}
	if (difference < -dtime)
	{
		return current_speed + dtime;
	}
	return final_speed;
}

void PlayerMovement::ComputeVerticalVelocity(float3 direction, float speed, bool apply_gravity)
{

	grounded = DetectCollisions(1);
	animation->SetBool("grounded", grounded);

	is_falling = velocity.y < 0.f;
	animation->SetBool("is_falling", is_falling);

	if (direction.Length() > 0.f) 
	{
		velocity.y = math::Clamp(velocity.y + speed, -1000.f, max_vertical_velocity);
		is_jumping = false;
		animation->SetBool("is_jumping", is_jumping);
	}

	if (grounded && is_falling)
	{
		velocity.y = 0;
		minimum_y = -10.f;
	}
	else if (!grounded)
	{
		velocity.y += -gravity * App->time->delta_time * 0.001f;
	}

}

void PlayerMovement::SetNewPosition(const float3& new_position) const
{
	float3 next_position = float3(new_position);
	if (grounded && !is_jumping && !is_knocked)
	{
		if (main_vertical_ray)
		{
			next_position.y = main_vertical_ray_detector->hit_point.z;
		}
		else
		{
			next_position.y = border_vertical_ray_detector->hit_point.z;
		}
	}

	next_position.y = math::Clamp(next_position.y, minimum_y, 2000.f);
	owner->transform.SetGlobalMatrixTranslation(next_position);
	
}

bool PlayerMovement::IsInside(const float3& future_transform) const
{
	bool inside = false;
	float3 distance = future_transform - owner->transform.GetGlobalTranslation();
	float4 position_float4 = float4(future_transform, 1.f);
	float4 clip_coordinates = game_camera->GetClipMatrix() * position_float4;
	device_coordinates = clip_coordinates.xyz() / clip_coordinates.w;

	if(device_coordinates.x < 0.95 && device_coordinates.x > -0.95 && device_coordinates.y < 0.95)
	{
		inside = true;
	}
	if(other_player != nullptr && world->multiplayer)
	{
		float4 position_float4_other_player = float4(other_player->transform.GetGlobalTranslation() - distance, 1.f);
		float4 clip_coordinates_other_player = game_camera->GetClipMatrix() * position_float4_other_player;
		float3 device_coordinates_other_player = clip_coordinates_other_player.xyz() / clip_coordinates_other_player.w;

		if (device_coordinates_other_player.x > 0.95 || device_coordinates_other_player.x < -0.95
			|| device_coordinates_other_player.y > 0.95)
		{
			inside = true;
		}
	}
	return inside;
}

void PlayerMovement::InitPublicGameObjects()
{
	//IMPORTANT, public gameobjects, name_gameobjects and go_uuids MUST have same size

	public_gameobjects.push_back(&camera);
	public_gameobjects.push_back(&other_player);

	variable_names.push_back(GET_VARIABLE_NAME(camera));
	variable_names.push_back(GET_VARIABLE_NAME(other_player));

	for (unsigned int i = 0; i < public_gameobjects.size(); ++i)
	{
		name_gameobjects.push_back(is_object);
		go_uuids.push_back(0);
	}
}

//Use this for linking GO AND VARIABLES automatically if you need to save variables 
void PlayerMovement::Save(Config& config) const
{
	config.AddFloat(gravity, "Gravity");
	config.AddFloat(jump_speed, "FirstJumpSpeed");
	config.AddFloat(speed_horizontal, "HorizontalSpeed");
	config.AddFloat(acceleration, "Acceleration");
	config.AddFloat(deceleration, "Deceleration");

	Script::Save(config);
}

//Use this for linking GO AND VARIABLES automatically
void PlayerMovement::Load(const Config& config)
{
	gravity = config.GetFloat("Gravity", 25.f);
	jump_speed = config.GetFloat("FirstJumpSpeed", 10.f);
	speed_horizontal = config.GetFloat("HorizontalSpeed", 10.f);
	acceleration = config.GetFloat("Acceleration", 0.045f);
	deceleration = config.GetFloat("Deceleration", 0.045f);

	Script::Load(config);
}

bool PlayerMovement::IsPlayerColliding(RayHit* collision, const LineSegment& ray, bool only_mesh)
{
	bool collided = false;

	std::vector<ComponentMeshRenderer*> intersected_meshes;
	for (const auto& go : mesh_collider) 
	{
		if(go->IsEnabled())
		{
			intersected_meshes.push_back(static_cast<ComponentMeshRenderer*>(go->GetComponent(Component::ComponentType::MESH_RENDERER)));
		}
	}
	if(!only_mesh)
	{
		if (world->ThereIsBoss() && !world->GetBoss()->boss_controller->IsBossObstacle()) 
		{
			for (const auto& zone : boss_zones)
			{
				if (zone->owner->IsEnabled())
				{
					intersected_meshes.push_back(zone);
				}
			}
		}
		for (const auto& enemy : enemy_manager->enemies)
		{
			if((enemy->is_alive && !enemy->animation->IsOnState("Die")) 
				&& EnemyInFront(enemy->owner->transform.GetGlobalTranslation()))
			{
				bool hitted = false;
				//This is done because global_bounding_boxes don't work as expected and biter have different structure than the others
				if(enemy->enemy_type != EnemyType::BITER)
				{
					hitted = enemy->owner->children[0]->children[0]->aabb.bounding_box.Intersects(ray);
				}
				else
				{
					hitted = enemy->owner->children[0]->aabb.bounding_box.Intersects(ray);
				}
				if (hitted) 
				{
					collision->game_object = enemy->owner;
					collision->hit_distance = 1.f;
					return true;
				}
			}
		}
		if(world->multiplayer)
		{
			if (other_player->aabb.bounding_box.Intersects(ray))
			{
				intersected_meshes.push_back(static_cast<ComponentMeshRenderer*>(other_player->GetComponent(Component::ComponentType::MESH_RENDERER)));
			}
		}
	}

	GameObject* selected = nullptr;
	float min_distance = INFINITY;

	for (const auto& mesh : intersected_meshes)
	{
		LineSegment transformed_ray = ray;
		transformed_ray.Transform(mesh->owner->transform.GetGlobalModelMatrix().Inverted());
		std::vector<Mesh::Vertex> &vertices = mesh->mesh_to_render->vertices;
		std::vector<uint32_t> &indices = mesh->mesh_to_render->indices;
		for (size_t i = 0; i < indices.size(); i += 3)
		{
			float3 first_point = vertices[indices[i]].position;
			float3 second_point = vertices[indices[i + 1]].position;
			float3 third_point = vertices[indices[i + 2]].position;
			Triangle triangle(first_point, second_point, third_point);

			float distance;
			float3 intersected_point;
			bool intersected = triangle.Intersects(transformed_ray, &distance, &intersected_point);
			if (intersected && distance < min_distance)
			{
				min_distance = distance;

				collision->game_object = mesh->owner;
				collision->hit_distance = distance;
				collision->hit_point = intersected_point;
				collided = true;
			}
		}
	}
	return collided;
}

bool PlayerMovement::DetectCollisions(int collision_direction)
{
	float3 player_current_position = owner->transform.GetGlobalTranslation();
	bool collision = false;
	if (collision_direction == 1) //Vertical
	{ 
		vertical_offset_end = math::Clamp(abs(velocity.y), vertical_offset_minimum, vertical_offset_maximum);
		LineSegment ground_line(player_current_position + float3::unitY * vertical_offset_start, player_current_position - float3::unitY * vertical_offset_end);

		LineSegment border_line_detector(player_current_position + float3::unitY * vertical_offset_start - (owner->transform.GetFrontVector() * border_ray_offset),
			player_current_position - float3::unitY * vertical_offset_end - (owner->transform.GetFrontVector() * border_ray_offset));

		LineSegment diagonal_line_detector(player_current_position + float3::unitY,
			player_current_position - float3::unitY * 2 + (owner->transform.GetFrontVector() * 2.f));

		if(IsPlayerColliding(main_vertical_ray_detector, ground_line, true) 
			&& main_vertical_ray_detector->game_object != nullptr 
			&& main_vertical_ray_detector->hit_distance < vertical_threshold)
		{
			collision = true;
			main_vertical_ray = true;
			border_vertical_ray = false;
		}
		else if(IsPlayerColliding(border_vertical_ray_detector, border_line_detector, true) 
			&& border_vertical_ray_detector->game_object != nullptr
			&& border_vertical_ray_detector->hit_distance < vertical_threshold)
		{
			collision = true;
			border_vertical_ray = true;
			main_vertical_ray = false;
		}
		else if (IsPlayerColliding(main_diagonal_ray_detector, diagonal_line_detector, true)
			&& main_diagonal_ray_detector->game_object != nullptr
			&& main_diagonal_ray_detector->hit_distance < vertical_threshold)
		{
			collision = true;
		}
		else
		{
			border_vertical_ray = false;
			main_vertical_ray = false;

			if (main_vertical_ray_detector->hit_distance > vertical_threshold) 
			{
				minimum_y = main_vertical_ray_detector->hit_point.z;
			}
		}

		if (debug->show_movement)
		{
			App->debug_draw->RenderLine(ground_line.a, ground_line.b, float3(0.f, 1.f, 1.f));
			App->debug_draw->RenderLine(border_line_detector.a, border_line_detector.b, float3(1.f, 0.f, 1.f));
			App->debug_draw->RenderLine(diagonal_line_detector.a, diagonal_line_detector.b, float3(1.f, 1.f, 0.f));
		}

	}
	else if (collision_direction == 2) //Rotation
	{
		float3 front = player_current_position + owner->transform.GetFrontVector();

		float angle = math::Acos(rotation_angle);

		float3 max_collision = math::Quat(owner->transform.GetUpVector(), angle) * owner->transform.GetFrontVector();
		float3 min_collision = math::Quat(owner->transform.GetUpVector(), - angle) * owner->transform.GetFrontVector();

		max_collision.ScaleToLength(rotation_ray_length);
		min_collision.ScaleToLength(rotation_ray_length);

		max_collision += player_current_position;
		min_collision += player_current_position;
		
		LineSegment rotation_ray_max_line(player_current_position + float3::unitY * 0.2f, max_collision + float3::unitY * 0.2f);
		LineSegment rotation_ray_min_line(player_current_position + float3::unitY * 0.2f, min_collision + float3::unitY * 0.2f);

		RayHit* rotation_ray = new RayHit();

		if (IsPlayerColliding(rotation_ray, rotation_ray_max_line, true)
			&& rotation_ray->hit_distance > rotation_threshold)
		{
			collision = true;
		}
		else if (IsPlayerColliding(rotation_ray, rotation_ray_min_line, true)
			&& rotation_ray->hit_distance > rotation_threshold)
		{
			collision = true;
		}
		if (collision) 
		{
			App->engine_log->Log("Hit Distance: %.3f", rotation_ray->hit_distance);
		}
		if (debug->show_movement)
		{
			App->debug_draw->RenderLine(rotation_ray_max_line.a, rotation_ray_max_line.b);
			App->debug_draw->RenderLine(rotation_ray_min_line.a, rotation_ray_min_line.b);
		}
	}
	else //Horizontal
	{ 

		float3 cast_ray_position_middle = player_current_position + float3::unitY * horizontal_ray_middle_position - (owner->transform.GetFrontVector() * 0.5f);
		LineSegment ray_detector_middle(cast_ray_position_middle, cast_ray_position_middle + owner->transform.GetFrontVector() * horizontal_ray_middle_length);

		float3 cast_ray_position_knee = player_current_position + float3::unitY * horizontal_ray_knee_position;
		LineSegment ray_detector_knee(cast_ray_position_knee, cast_ray_position_knee + owner->transform.GetFrontVector() * horizontal_ray_knee_length);

		if (IsPlayerColliding(horizontal_ray_detector_middle, ray_detector_middle) 
			&& horizontal_ray_detector_middle->hit_distance > horizontal_threshold)
		{
			collision = true;
		}
		else if (IsPlayerColliding(horizontal_ray_detector_knee, ray_detector_knee, true) 
			&& horizontal_ray_detector_knee->hit_distance > 0.0f)
		{
			collision = true;
		}

		if (debug->show_movement)
		{
			App->debug_draw->RenderLine(ray_detector_middle.a, ray_detector_middle.b);
			App->debug_draw->RenderLine(ray_detector_knee.a, ray_detector_knee.b);
		}
	}
	return collision;
}

bool PlayerMovement::EnemyInFront(const float3& enemy_position)
{
	float3 direction = enemy_position - owner->transform.GetGlobalTranslation();
	float dot_product = math::Dot(direction.Normalized(), owner->transform.GetFrontVector());
	if(dot_product > 0 && direction.Length() < range_collision)
	{
		return true;
	}

	return false;
}

void PlayerMovement::ChangeDashStatus(bool is_dashing)
{
	this->is_dashing = is_dashing;
}