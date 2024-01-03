#include "LightTeleportManager.h"

#include "Component/ComponentAnimation.h"
#include "Component/ComponentScript.h"
#include "Component/ComponentTransform.h"

#include "Main/Application.h"
#include "Main/GameObject.h"
#include "Module/ModuleInput.h"
#include "Module/ModuleScene.h"
#include "Module/ModuleTime.h"


#include "imgui.h"

#include "EnemyController.h"
#include "EnemyManager.h"
#include "WorldManager.h"


LightTeleportManager* LightTeleportManagerDLL()
{
	LightTeleportManager* instance = new LightTeleportManager();
	return instance;
}

LightTeleportManager::LightTeleportManager()
{
	
}

// Use this for initialization before Start()
void LightTeleportManager::Awake()
{
	for(const auto& go : owner->children)
	{
		if(go->name == "LightProjectile")
		{
			projectile = go;
		}
	}
	projectile->transform.SetGlobalMatrixTranslation(owner->transform.GetGlobalTranslation() + float3(0, 1, 0));
	projectile->SetEnabled(false);

	GameObject* enemy_go = App->scene->GetGameObjectByName("EnemyManager");
	ComponentScript* enemy_component = enemy_go->GetComponentScript("EnemyManager");
	enemy_manager = static_cast<CEnemyManager*>(enemy_component->script);


	GameObject* world_go = App->scene->GetGameObjectByName("World Manager");
	ComponentScript* world_component = world_go->GetComponentScript("WorldManager");
	world_manager = static_cast<WorldManager*>(world_component->script);

	projectile_collider = static_cast<ComponentCollider*>(projectile->GetComponent(Component::ComponentType::COLLIDER));

	player_collider = static_cast<ComponentCollider*>(owner->GetComponent(Component::ComponentType::COLLIDER));

	animation = static_cast<ComponentAnimation*>(owner->GetComponent(Component::ComponentType::ANIMATION));
}

// Use this for initialization
void LightTeleportManager::Start()
{


}

// Update is called once per frame
void LightTeleportManager::Update()
{

	if (world_manager->on_pause)
	{
		return;
	}

	switch (tp_state)
	{
		case LightTeleportState::DISABLE:
		{
			if(casting_projectile && animation->GetCurrentClipPercentatge() > 0.30f)
			{
				ShootProjectile();
				casting_projectile = false;
			}

			return;
		}
		case LightTeleportState::SHOOTING_PROJECTILE:
		{
			//Lerp the position of the projectile using delta time for a constant velocity
			float3 new_position = float3::Lerp(projectile_initial_position, projectile_direction, current_projectile_lerp);
			current_projectile_lerp += App->time->delta_time * projectile_speed * 0.001f;
			projectile->transform.SetGlobalMatrixTranslation(new_position);

			if (DetectCollision())
			{
				break;
			}


			if (current_projectile_lerp >= 1.0f)
			{
				projectile->SetEnabled(false);
				projectile->transform.SetGlobalMatrixTranslation(owner->transform.GetGlobalTranslation() + float3(0, 1, 0));
				tp_state = LightTeleportState::COOLDOWN;
			}

			break;
		}
		case LightTeleportState::WAITING_FOR_WARP:
		{
			if(warping)
			{
				if(animation->GetCurrentClipPercentatge() > 0.30f)
				{
					TriggerWarp();
				}
				break;
			}

			current_time_marked += App->time->delta_time *  0.001f;

			if (current_time_marked >= cooldown_enemy_mark)
			{
				current_time_marked = 0.0f;
				marked_enemy = nullptr;
				tp_state = LightTeleportState::COOLDOWN;
			}

			break;
		}
		case LightTeleportState::WARP:
		{
			float3 new_position = float3::Lerp(player_initial_position,
				marked_enemy->owner->transform.GetGlobalTranslation(),
				current_warp_lerp);
			owner->transform.SetGlobalMatrixTranslation(new_position);
			current_warp_lerp += App->time->delta_time * 0.001f * warp_speed;

			if(current_warp_lerp >= 1.0f || marked_enemy->is_dying || !marked_enemy->is_alive)
			{
				//We arrived to desired position particles/damage/anims should be triggered here
				
				
				//Get out of collisions
				std::vector<CollisionInformation> collisions = static_cast<ComponentCollider*>
					(owner->GetComponent(Component::ComponentType::COLLIDER))->GetCollisions();
				for (auto& collision : collisions)
				{
					float3 normalized_collision = collision.normal.Normalized();
					float3 new_position = owner->transform.GetGlobalTranslation()
						+ float3(normalized_collision.x, normalized_collision.y, normalized_collision.z)
						*  collision.distance;
					owner->transform.SetGlobalMatrixTranslation(new_position);
				}
				owner->children[0]->SetEnabled(true);
				animation->ActiveAnimation("punch");
				warping = false;
				current_warp_lerp = 0.0f;
				tp_state = LightTeleportState::COOLDOWN;

			}

			break;
		}
		case LightTeleportState::COOLDOWN:
			current_cooldown_time_teleport += App->time->delta_time;
			if (current_cooldown_time_teleport >= teleport_cooldown)
			{
				current_cooldown_time_teleport = 0.0f;
				tp_state = LightTeleportState::DISABLE;
			}
			return;
		default:
			break;
	}

}

// Use this for showing variables on inspector
void LightTeleportManager::OnInspector(ImGuiContext* context)
{
	//Necessary to be able to write with imgui
	ImGui::SetCurrentContext(context);
	ShowDraggedObjects();

	ImGui::Text("Parameters:");
	ImGui::DragFloat("maximum_projectile_distance", &maximum_projectile_distance);
	ImGui::DragFloat("projectile_speed", &projectile_speed);
	ImGui::Separator();
	ImGui::Text("Debug info");
	ImGui::DragFloat("current_projectile_time", &current_projectile_lerp);
	ImGui::DragFloat3("projectile_initial_position", projectile_initial_position.ptr());
	ImGui::DragFloat3("projectile_direction", projectile_direction.ptr());
	ImGui::Checkbox("Casting projectile", &casting_projectile);
}

bool LightTeleportManager::LightWarp()
{
	//Returns true if we are OnWarp so we cannot

	switch (tp_state)
	{
		case LightTeleportState::DISABLE:
		{
			animation->ActiveAnimation("punch");
			casting_projectile = true;

			break;
		}
		case LightTeleportState::WAITING_FOR_WARP:
		{
			if(marked_enemy && !marked_enemy->is_dying && marked_enemy->is_alive)
			{
				animation->ActiveAnimation("kick");
				warping = true;			
				return true;
			}
			break;

		}
		default:
			break;
	}

	return false;
}

bool LightTeleportManager::IsOnCoolDown()
{
	return tp_state == LightTeleportState::COOLDOWN;
}

bool LightTeleportManager::DetectCollision()
{
	////Check if projectile collides an enemy
	for (auto& enemy : enemy_manager->enemies)
	{
		if (!enemy->is_alive)
		{
			continue;
		}
		//TODO Modify this when OnCollisionEnter implemented
		if (projectile_collider->IsCollidingWith(enemy->collider))
		{
			//Mark enemy
			marked_enemy = enemy;
			projectile->SetEnabled(false);
			projectile->transform.SetGlobalMatrixTranslation(owner->transform.GetGlobalTranslation() + float3(0, 1, 0));
			tp_state = LightTeleportState::WAITING_FOR_WARP;
			return true;
		}
	}

	return false;
}

void LightTeleportManager::ShootProjectile()
{
	//Prepare projectile casting

	tp_state = LightTeleportState::SHOOTING_PROJECTILE;
	//Get Direction of the ball
	projectile->SetEnabled(true);
	projectile_initial_position = projectile->transform.GetGlobalTranslation();
	projectile_direction = owner->transform.GetFrontVector();
	projectile_direction.ScaleToLength(maximum_projectile_distance);
	projectile_direction = projectile_direction + projectile_initial_position;
	current_projectile_lerp = 0.0f;
}

void LightTeleportManager::TriggerWarp()
{
	//Prepare Warp

	tp_state = LightTeleportState::WARP;
	//Disable mesh renderer of character
	player_initial_position = owner->transform.GetGlobalTranslation();
	owner->children[0]->SetEnabled(false);
}
