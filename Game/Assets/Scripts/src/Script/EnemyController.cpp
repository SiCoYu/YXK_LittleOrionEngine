#include "EnemyController.h"

#include "Component/ComponentAnimation.h"
#include "Component/ComponentAudioSource.h"
#include "Component/ComponentBillboard.h"
#include "Component/ComponentCollider.h"
#include "Component/ComponentScript.h"
#include "Component/ComponentParticleSystem.h"
#include "Component/ComponentTransform.h"

#include "Main/Application.h"
#include "Main/GameObject.h"
#include "Module/ModuleAI.h"
#include "Module/ModuleInput.h"
#include "Module/ModuleScene.h"
#include "Module/ModuleTime.h"

#include "imgui.h"

#include "Attacks.h"
#include "DebugModeScript.h"
#include "EnemyManager.h"
#include "HitBlinker.h"
#include "LightOrbsManager.h"
#include "PlayerController.h"
#include "UIManager.h"
#include "WorldManager.h"

EnemyController* EnemyControllerDLL()
{
	EnemyController* instance = new EnemyController();
	return instance;
}

EnemyController::EnemyController()
{
	
}

void EnemyController::Awake()
{
	InitMembers();
	//enemy_manager->AddEnemy(this);
	init_move_speed = move_speed;
}

void EnemyController::InitMembers()
{
	animation = static_cast<ComponentAnimation*>(owner->GetComponent(Component::ComponentType::ANIMATION));
	audio_source = static_cast<ComponentAudioSource*>(owner->GetComponent(Component::ComponentType::AUDIO_SOURCE));
	collider = static_cast<ComponentCollider*>(owner->GetComponent(Component::ComponentType::COLLIDER));

	enemy_manager = static_cast<EnemyManager*>(App->scene->GetGameObjectByName("EnemyManager")->GetComponentScript("EnemyManager")->script);
	ui_manager = static_cast<UIManager*>(App->scene->GetGameObjectByName("UIManager")->GetComponentScript("UIManager")->script);
	light_orbs_manager = static_cast<LightOrbsManager*>(App->scene->GetGameObjectByName("Light Orbs")->GetComponentScript("LightOrbsManager")->script);
	world_manager = static_cast<WorldManager*>(App->scene->GetGameObjectByName("World Manager")->GetComponentScript("WorldManager")->script);

	obstacles = App->scene->GetGameObjectsWithTag("Obstacle");
	InitVFX();

	env_colliders = App->scene->GetGameObjectsWithTag("EnvironmentCollider");	
	debug = static_cast<DebugModeScript*>(App->scene->GetGameObjectByName("DebugSystem")->GetComponentScript("DebugModeScript")->script);

	if(enemy_type == EnemyType::NECROPLANT)
	{
		hit_blinker = static_cast<HitBlinker*>(owner->children[0]->children[0]->GetComponentScript("HitBlinker")->script);
	}
	else if(enemy_type == EnemyType::BITER)
	{
		hit_blinker = static_cast<HitBlinker*>(owner->children[0]->GetComponentScript("HitBlinker")->script);
	}
	else
	{
		hit_blinker = static_cast<HitBlinker*>(owner->children[0]->children[0]->GetComponentScript("HitBlinker")->script);
	}
	
}

void EnemyController::Start()
{
	player_1 = world_manager->GetPlayer1();
	player_2 = world_manager->GetPlayer2();
	current_target = player_1->player_controller;
}

void EnemyController::OnInspector(ImGuiContext* context)
{
	ImGui::SetCurrentContext(context);
	ShowDraggedObjects();

	ImGui::Text("Enemy Stats");
	ImGui::InputFloat("Move Speed", &move_speed);
	ImGui::InputFloat("Rotate Speed", &rotate_speed);
	ImGui::InputFloat("Attack Speed", &attack_speed);
	ImGui::InputFloat("Attack Damage", &attack_damage);
	ImGui::InputFloat("Attack Range", &attack_range);
	ImGui::InputFloat("Max Health", &max_health);
	ImGui::InputFloat("Health", &health);
	ImGui::InputFloat("Detect Distance", &detect_player_distance);
	ImGui::InputFloat("Target Distance", &switch_target_distance);
	ImGui::Checkbox("Alive", &is_alive);
	ImGui::SameLine();
	ImGui::Checkbox("Invincible", &invincible);
	ImGui::NewLine();
	ImGui::DragFloat("Frontal Collision Detection Distance", &frontal_collision_detection_distance, 0.1f, 0.1f, 10.f);
	ImGui::DragFloat("Jump Distance", &jump_distance, 0.1f, 0.1f);
	ImGui::DragFloat("Fall Speed", &fall_speed, 0.1f, 0.1f);
	ImGui::DragFloat("Ground Ray Percentage", &ground_ray_percentage, 0.1f, 0.1f);
	ImGui::DragFloat("Front Distance", &front_distance, 0.1f, 0.1f);
	ImGui::NewLine();
}

void EnemyController::InitPublicGameObjects()
{
	for (int i = 0; i < public_gameobjects.size(); ++i)
	{
		name_gameobjects.push_back(is_object);
		go_uuids.push_back(0);
	}
}

void EnemyController::SetProperties(EnemyController* original_enemy)
{
	move_speed = original_enemy->move_speed;
	init_move_speed = move_speed;
	rotate_speed = original_enemy->rotate_speed;
	attack_speed = original_enemy->attack_speed;
	attack_damage = original_enemy->attack_damage;
	attack_range = original_enemy->attack_range;
	max_health = original_enemy->max_health;
	health = max_health;
	detect_player_distance = original_enemy->detect_player_distance;
	switch_target_distance = original_enemy->switch_target_distance;

	move_with_physics = original_enemy->move_with_physics;
	collider->active_physics = original_enemy->collider->active_physics;
}

void EnemyController::Save(Config& config) const
{
	config.AddFloat(detect_player_distance, "PlayerDistance");
	config.AddFloat(jump_distance, "JumpDistance");
	config.AddFloat(fall_speed, "FallSpeed");
	config.AddFloat(ground_ray_percentage, "GroundRay");
	config.AddFloat(front_distance, "FrontDistance");
	config.AddBool(is_alive, "IsAlive");
	Script::Save(config);
}

void EnemyController::Load(const Config &config)
{
	detect_player_distance = config.GetFloat("PlayerDistance", 55.0f);
	fall_speed = config.GetFloat("FallSpeed", 10.0f);
	jump_distance = config.GetFloat("JumpDistance", 4.0f);
	ground_ray_percentage = config.GetFloat("GroundRay", 1.0f);
	front_distance = config.GetFloat("FrontDistance", 5.0f);
	is_alive = config.GetBool("IsAlive", false);
	Script::Load(config);
}

float3 EnemyController::CollisionAvoidance(const float3& velocity) const
{
	float3 ahead = owner->transform.GetGlobalTranslation() + velocity.Normalized() * MAX_SEE_AHEAD;
	float3 ahead2 = owner->transform.GetGlobalTranslation() + velocity.Normalized() * MAX_SEE_AHEAD * 0.5;
	float3 avoidance_force = float3::zero;

	for (auto& obstacle : obstacles)
	{
		if (owner->transform.GetGlobalTranslation().Distance(obstacle->transform.GetGlobalTranslation()) <= MIN_SEPARATE_DISTANCE)
		{
			float3 force = ahead - obstacle->transform.GetGlobalTranslation();
			avoidance_force += force.Normalized() * MAX_AVOID_FORCE;
		}
	}

	for (auto& env_collider : env_colliders)	
	{	
		if (owner->transform.GetGlobalTranslation().Distance(env_collider->transform.GetGlobalTranslation()) <= MIN_SEPARATE_DISTANCE)	
		{	
			float3 force = ahead - env_collider->transform.GetGlobalTranslation();	
			avoidance_force += force.Normalized() * MAX_AVOID_FORCE;	
		}	
	}

	return avoidance_force;
}

ComponentBillboard* EnemyController::GetAttackVFX(const Player* player, const bool& is_explosion) const
{
	ComponentBillboard* vfx = nullptr;
	if (enemy_type != EnemyType::BITER)
	{
		vfx = player->player_controller->hit_effect;
	}
	else
	{
		if (!is_explosion)
		{
			vfx = player->player_controller->hit_effect_biter;
		}
		else
		{
			vfx = nullptr;	//it is already nullptr, but added just in case we implement a hit VFX effect for the Biter explosion
		}
	}
	return vfx;
}

bool EnemyController::PlayerInSight() const
{
	if (current_target == nullptr) return false;

	float3 target = current_target->owner->transform.GetGlobalTranslation();
	return target.Distance(owner->transform.GetGlobalTranslation()) < detect_player_distance;
}

bool EnemyController::PlayerInAttackRange() const
{
	if (!current_target)
	{
		return false;
	}
	float3 target = current_target->owner->transform.GetGlobalTranslation();
	return target.Distance(owner->transform.GetGlobalTranslation()) <= attack_range;
}

bool EnemyController::PlayerInAttackArea(const float3& area, const float& damage_radius) const	
{	
	float3 target = current_target->owner->transform.GetGlobalTranslation();	
	return area.Distance(target) <= damage_radius;	
}

bool EnemyController::PlayerInRange(float range) const
{
	if (current_target == nullptr)
	{
		return false;
	}

	float3 target = current_target->owner->transform.GetGlobalTranslation();
	return target.Distance(owner->transform.GetGlobalTranslation()) <= range;
}

void EnemyController::DealDamageToPlayersInArea(const float3& area, const float& damage_radius, float damage, bool& player_1_hit, bool& player_2_hit, const bool& is_explosion) const	
{	
	float3 target_player;	
	if (!player_1_hit && player_1 && player_1->player_controller->is_alive)	
	{	
		target_player = player_1->player_go->transform.GetGlobalTranslation();	
		if (target_player.Distance(area) <= damage_radius)	
		{	
			player_1->player_controller->TakeDamage(damage, GetAttackVFX(player_1, is_explosion));
			player_1_hit = true;	
		}	
	}	
	if (!player_2_hit && player_2 && player_2->player_controller->is_alive)	
	{	
		target_player = player_2->player_go->transform.GetGlobalTranslation();	
		if (target_player.Distance(area) <= damage_radius)	
		{	
			player_2->player_controller->TakeDamage(damage, GetAttackVFX(player_2, is_explosion));
			player_2_hit = true;
		}	
	}	
}

PlayerController* EnemyController::GetClosestTarget()
{
	if (player_1->player_controller->is_alive && player_2->player_controller->is_alive)
	{
		float3 position = owner->transform.GetGlobalTranslation();

		float distance_player1 = player_1->player_go->transform.GetGlobalTranslation().Distance(position);
		float distance_player2 = player_2->player_go->transform.GetGlobalTranslation().Distance(position);

		if (abs(distance_player1 - distance_player2) >= switch_target_distance)
		{
			if (distance_player1 < distance_player2)
			{
				current_target = player_1->player_controller;
			}
			else
			{
				current_target = player_2->player_controller;
			}
		}
	}
	else if (!player_1->player_controller->is_alive && player_2->player_controller->is_alive)
	{
		current_target = player_2->player_controller;
	}
	else if (player_1->player_controller->is_alive && !player_2->player_controller->is_alive)
	{
		current_target = player_1->player_controller;
	}
	else if (!player_1->player_controller->is_alive && !player_2->player_controller->is_alive)
	{
		current_target = nullptr;
	}

	return current_target;
}

void EnemyController::Attack()
{
	current_target->TakeDamage(attack_damage, current_target->hit_effect);
}

void EnemyController::Attack(float damage)	
{	
	current_target->TakeDamage(damage, current_target->hit_effect);
}

bool EnemyController::PlayerHit() const
{
	return collider->IsCollidingWith(current_target->GetCollider());
}

void EnemyController::HitVFX(NodeID attack)
{
	switch (attack)
	{
		case NodeID::LIGHT_ONE:
			light_two_hit_VFX->loop = false;
			light_two_hit_VFX->Play();
			hit_particles_VFX->Emit(100);
		break;
		case NodeID::LIGHT_TWO:
			light_two_hit_VFX->loop = false;
			light_two_hit_VFX->Play();
			hit_particles_VFX->Emit(100);
		break;
		case NodeID::LIGHT_THREE:
			light_one_hit_VFX->loop = false;
			light_one_hit_VFX->Play();
			hit_particles_VFX->Emit(100);
		break;
		case NodeID::HEAVY_ONE:
			heavy_one_hit_VFX->loop = false;
			heavy_one_hit_VFX->Play();
			hit_particles_VFX->Emit(100);
		break;
		case NodeID::HEAVY_TWO:
			heavy_two_hit_VFX->loop = false;
			heavy_two_hit_VFX->Play();
			hit_particles_VFX->Emit(100);
		break;
		case NodeID::HEAVY_THREE:
			heavy_three_hit_VFX->loop = false;
			heavy_three_hit_VFX->Play();
			hit_particles_VFX->Emit(100);
		break;
		case NodeID::DOWN_SMASH:
		
		break;
	}
	
}

void EnemyController::DeathVFX()
{
	if (death_effect)
	{
		death_effect->loop = false;
		death_effect->Play();
	}
}

void EnemyController::ResetEnemy()
{
	health = max_health;
	is_alive = true;
	is_attacking = false;
	invincible = false;
	last_time = 0;
	current_time = 0;

	if (move_with_physics)
	{
		collider->detect_collision = true;
		collider->active_physics = true;

		collider->Enable();
		collider->SetCollisionDetection();
	}
	else
	{
		collider->active_physics = false;
	}


	hit_blinker->Reset();
	
}

void EnemyController::HitLightBallVFX() const 
{
	if (LB_hit_effect_purple != nullptr)
	{
		LB_hit_effect_purple->Emit(2);
	}

	if (LB_hit_effect_emerald != nullptr)
	{
		LB_hit_effect_emerald->Emit(1);
	}

	if (LB_hit_effect_blue != nullptr)
	{
		LB_hit_effect_blue->Emit(4);
	}

	if (LB_flare_hit_effect != nullptr)
	{
		LB_flare_hit_effect->loop = false;
		LB_flare_hit_effect->Play();
	}
}

void EnemyController::InitVFX()
{
	//Get Components
	for (const auto go : owner->children)
	{
		if (go->tag == "LB_hit_effect_blue")
		{
			LB_hit_effect_blue = static_cast<ComponentParticleSystem*>(go->GetComponent(Component::ComponentType::PARTICLE_SYSTEM));
			LB_hit_effect_blue->Stop();
		}
		if (go->tag == "LB_hit_effect_purple")
		{
			LB_hit_effect_purple = static_cast<ComponentParticleSystem*>(go->GetComponent(Component::ComponentType::PARTICLE_SYSTEM));
			LB_hit_effect_purple->Stop();
		}
		if (go->tag == "LB_hit_effect_emerald")
		{
			LB_hit_effect_emerald = static_cast<ComponentParticleSystem*>(go->GetComponent(Component::ComponentType::PARTICLE_SYSTEM));
			LB_hit_effect_emerald->Stop();
		}
		if (go->tag == "LB_flare_hit")
		{
			LB_flare_hit_effect = static_cast<ComponentBillboard*>(go->GetComponent(Component::ComponentType::BILLBOARD));
			LB_flare_hit_effect->Disable();
		}
		if (go->tag == "VFX")
		{
			for (const auto vfx : go->children)
			{
				if (vfx->tag == "left_punch_hit_VFX")
				{
					light_two_hit_VFX = static_cast<ComponentBillboard*>(vfx->GetComponent(Component::ComponentType::BILLBOARD));
					light_two_hit_VFX->Disable();
				}
				if (vfx->tag == "right_punch_hit_VFX")
				{
					light_one_hit_VFX = static_cast<ComponentBillboard*>(vfx->GetComponent(Component::ComponentType::BILLBOARD));
					light_one_hit_VFX->Disable();
				}
				if (vfx->tag == "front_flip_hit_VFX")
				{
					light_three_hit_VFX = static_cast<ComponentBillboard*>(vfx->GetComponent(Component::ComponentType::BILLBOARD));
					light_three_hit_VFX->Disable();
				}
				if (vfx->tag == "kick_hit_VFX")
				{
					heavy_one_hit_VFX = static_cast<ComponentBillboard*>(vfx->GetComponent(Component::ComponentType::BILLBOARD));
					heavy_one_hit_VFX->Disable();
				}
				if (vfx->tag == "super_kick_VFX")
				{
					heavy_two_hit_VFX = static_cast<ComponentBillboard*>(vfx->GetComponent(Component::ComponentType::BILLBOARD));
					heavy_two_hit_VFX->Disable();
				}
				if (vfx->tag == "super_punch_VFX")
				{
					heavy_three_hit_VFX = static_cast<ComponentBillboard*>(vfx->GetComponent(Component::ComponentType::BILLBOARD));
					heavy_three_hit_VFX->Disable();
				}
				if (vfx->tag == "hit_particles_VFX")
				{
					hit_particles_VFX = static_cast<ComponentParticleSystem*>(vfx->GetComponent(Component::ComponentType::PARTICLE_SYSTEM));
					hit_particles_VFX->Stop();
				}
			}
		}		
		if (go->tag == "DeathEffect")
		{
			death_effect = static_cast<ComponentBillboard*>(go->GetComponent(Component::ComponentType::BILLBOARD));
			death_effect->Disable();
		}
	}
}

void EnemyController::HideVFX() const
{
	if (LB_hit_effect_blue != nullptr)
	{
		LB_hit_effect_blue->Stop();
	}

	if (LB_hit_effect_purple != nullptr)
	{
		LB_hit_effect_purple->Stop();
	}

	if (LB_hit_effect_emerald != nullptr)
	{
		LB_hit_effect_emerald->Stop();
	}

	if (LB_flare_hit_effect != nullptr)
	{
		LB_flare_hit_effect->Disable();
	}

	if (light_two_hit_VFX != nullptr)
	{
		light_two_hit_VFX->Disable();
	}

	if (light_one_hit_VFX != nullptr)
	{
		light_one_hit_VFX->Disable();
	}

	if (light_three_hit_VFX != nullptr)
	{
		light_three_hit_VFX->Disable();
	}

	if (heavy_one_hit_VFX != nullptr)
	{
		heavy_one_hit_VFX->Disable();
	}

	if (heavy_three_hit_VFX != nullptr)
	{
		heavy_three_hit_VFX->Disable();
	}

	if (heavy_two_hit_VFX != nullptr)
	{
		heavy_two_hit_VFX->Disable();
	}

	if (hit_particles_VFX != nullptr)
	{
		hit_particles_VFX->Stop();
	}

	if (death_effect != nullptr)
	{
		death_effect->Disable();
	}
}

float EnemyController::GetCurrentHealth() const
{
	return health;
}

float EnemyController::GetTotalHealth() const
{
	return max_health;
}
