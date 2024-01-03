#include "BossController.h"

#include "Component/ComponentAnimation.h"
#include "Component/ComponentAudioSource.h"
#include "Component/ComponentBillboard.h"
#include "Component/ComponentMeshRenderer.h"
#include "Component/ComponentScript.h"
#include "Component/ComponentParticleSystem.h"
#include "Component/ComponentTransform.h"

#include "Log/EngineLog.h"

#include "Main/Application.h"
#include "Main/GameObject.h"
#include "Module/ModuleDebugDraw.h"
#include "Module/ModuleInput.h"
#include "Module/ModuleRender.h"
#include "Module/ModuleScene.h"
#include "Module/ModuleTime.h"

#include <imgui.h>

#include "BattleEvent.h"
#include "BossStagePhaseOne.h"
#include "BossStagePhaseTwo.h"
#include "BossStagePhaseThree.h"
#include "CameraController.h"
#include "CameraShake.h"
#include "Dissolver.h"
#include "EventManager.h"
#include "HitBlinker.h"
#include "PlayerController.h"
#include "PlayerMovement.h"
#include "ProgressBar.h"
#include "UIManager.h"
#include "WorldManager.h"

namespace
{
	const float min_shadow_scale = 0.050f;
	const float max_shadow_scale = 0.150f;
	const float remaining_crystal_time_alive = 1000.f;
	const float max_remaining_rock_time_alive = 1000.f;
	const float min_remaining_rock_time_alive = 200.f;
	const float time_stunned = 1500.f;
	const float time_waiting_for_spawning = 4000.f;
	const float max_time_not_being_obstacle = 1000.f;
}


BossController* BossControllerDLL()
{
	BossController* instance = new BossController();
	return instance;
}

BossController::BossController()
{

}

// Use this for initialization before Start()
void BossController::Awake()
{
	world_manager = static_cast<WorldManager*>(App->scene->GetGameObjectByName("World Manager")->GetComponentScript("WorldManager")->script);
	event_manager = static_cast<CEventManager*>(App->scene->GetGameObjectByName("EventManager")->GetComponentScript("EventManager")->script);
	ui_manager = static_cast<UIManager*>(App->scene->GetGameObjectByName("UIManager")->GetComponentScript("UIManager")->script);
	camera_shake = static_cast<CameraShake*>(App->scene->GetGameObjectByName("Main Camera")->GetComponentScript("CameraShake")->script);

	boss_health_bar = static_cast<ProgressBar*>(App->scene->GetGameObjectByName("Boss Bar")->GetComponentScript("ProgressBar")->script);

	camera_go = camera_shake->owner->parent;
	camera_controller = static_cast<CameraController*>(camera_shake->owner->parent->GetComponentScript("CameraController")->script);

	animation = static_cast<ComponentAnimation*>(owner->GetComponent(Component::ComponentType::ANIMATION));

	wall_collider = App->scene->GetGameObjectByName("ColliderWall");

	hit_blinker = static_cast<HitBlinker*>(owner->children[0]->GetComponentScript("HitBlinker")->script);

	InitBonesGameObjects();

}

// Use this for initialization
void BossController::Start()
{
	InitPlayers();
	is_multiplayer = world_manager->multiplayer;
	InitStages();
	current_stage = stages.top();
	stages.pop();
	is_alive = true;
	InitVulnerableZones();
	InitCrystals();
	InitRocks();
	InitEffects();

	std::srand(time(NULL));
}

// Update is called once per frame
void BossController::Update()
{
	if (is_alive)
	{
		current_stage->Update(App->time->delta_time);

		//Update boss health bar
	}
	//else
	//{
	//	if(animation->IsOnState("Die") && animation->GetCurrentClipPercentatge() > die_time)
	//	{
	//		BossKilled();
	//	}
	//}

	//if(is_alive && current_health <= change_stages_health[current_stage_index])
	//{
	//	TransitionToNextStage();
	//}

	CheckPlayersSpawning();
	CheckIfBossIsObstacle();
}

// Use this for showing variables on inspector
void BossController::OnInspector(ImGuiContext* context)
{
	//Necessary to be able to write with imgui
	ImGui::SetCurrentContext(context);
	ShowDraggedObjects();

	ImGui::DragFloat3("Sphere pos: ", debug_float3_debug_variable.ptr());

	if(current_stage)
	{
		ImGui::Text("Current Stage: %s", current_stage->name.c_str());
		ImGui::Text("Current state: %s", current_stage->GetCurrentStateName().c_str());	
	}

	ImGui::DragFloat("Health", &current_health);
	
	//Zones INFO
	for(const auto& zone : vulnerable_zones)
	{
		ImGui::Text("%s : ", GetBossBoneName(zone->bone).c_str());
		ImGui::SameLine();
		ImGui::Checkbox("Vulnerable", &zone->vulnerable);
		ImGui::SameLine();
		ImGui::Checkbox("Active", &zone->active);
	}

	ImGui::Checkbox("Vulnerable", &vulnerable);
	ImGui::DragFloat("avalanche_trauma", &avalanche_trauma);
}

//Use this for linking JUST GO automatically 
void BossController::InitPublicGameObjects()
{
	//IMPORTANT, public gameobjects, name_gameobjects and go_uuids MUST have same size
	for (int i = 0; i < public_gameobjects.size(); ++i)
	{
		name_gameobjects.push_back(is_object);
		go_uuids.push_back(0);
	}
}

void BossController::InitPlayers()
{
	player_1 = world_manager->GetPlayer1();
	player_2 = world_manager->GetPlayer2();
}

void BossController::InitStages()
{
	stages.push(new BossStagePhaseThree(this));
	stages.push(new BossStagePhaseTwo(this));
	stages.push(new BossStagePhaseOne(this));
}

void BossController::InitVulnerableZones()
{
	vulnerable_zones.emplace_back(SetVulnerableZones(right_forearm_go, BossBone::RIGHT_FOREARM));
	SetEnableVulnerableZone(BossBone::RIGHT_FOREARM, false, false);

	vulnerable_zones.emplace_back(SetVulnerableZones(right_forearm_go, BossBone::RIGHT_FINGER));
	SetEnableVulnerableZone(BossBone::RIGHT_FINGER, false, false);

	vulnerable_zones.emplace_back(SetVulnerableZones(left_forearm_go, BossBone::LEFT_FOREARM));
	SetEnableVulnerableZone(BossBone::LEFT_FOREARM, false, false);

	vulnerable_zones.emplace_back(SetVulnerableZones(left_hand_finger, BossBone::LEFT_FINGER));
	SetEnableVulnerableZone(BossBone::LEFT_FINGER, false, false);

	vulnerable_zones.emplace_back(SetVulnerableZones(chest, BossBone::CHEST));
	SetEnableVulnerableZone(BossBone::CHEST, false, false);

	vulnerable_zones.emplace_back(SetVulnerableZones(head, BossBone::HEAD));
	SetEnableVulnerableZone(BossBone::HEAD, false, false);
}

BossController::VulnerableZone* BossController::SetVulnerableZones(const GameObject* bone, const BossBone type) const
{
	VulnerableZone* zone = new VulnerableZone(bone->children[0]);
	zone->bone = type;
	return zone;
}

void BossController::SetEnableVulnerableZone(const BossBone type, const bool vulnerable, const bool enable) const
{
	auto it = std::find_if(vulnerable_zones.begin(), vulnerable_zones.end(), [&type](VulnerableZone* zone) {return zone->bone == type; });
	if (it != vulnerable_zones.end())
	{
		VulnerableZone* zone = *it;
		zone->active = enable;
		zone->vulnerable = vulnerable;
		zone->zone_go->SetEnabled(enable);
		zone->zone_go->GetComponent(Component::ComponentType::MESH_RENDERER)->active = false;
	}
}

void BossController::ActiveSound(BossBone bone, const std::string& event) const
{
	GameObject* bone_go = GetBoneGameObject(bone);
	static_cast<ComponentAudioSource*>(bone_go->GetComponent(Component::ComponentType::AUDIO_SOURCE))->PlayEvent(event.c_str());
}

void BossController::TriggerHitSound(BossBone bone) const
{
	switch (bone)
	{
		case BossBone::HEAD:
			ActiveSound(bone, "Play_BossHit");
			break;

		case BossBone::RIGHT_HAND:
			ActiveSound(bone, "Play_BossRockHit");
			break;

		case BossBone::RIGHT_FOREARM:
			ActiveSound(bone, "Play_BossRockHit");
			break;

		case BossBone::RIGHT_FINGER:
			ActiveSound(bone, "Play_BossHit");
			break;

		case BossBone::LEFT_HAND:
			ActiveSound(bone, "Play_BossRockHit");
			break;

		case BossBone::LEFT_FOREARM:
			ActiveSound(bone, "Play_BossRockHit");
			break;

		case BossBone::LEFT_FINGER:
			ActiveSound(bone, "Play_BossHit");
			break;

		case BossBone::CHEST:
			ActiveSound(bone, "Play_BossRockHit");
			break;

		case BossBone::NONE:
			break;

		default:
			ActiveSound(bone, "Play_BossRockHit");
			break;
	}
}

void BossController::TriggerMusic() const
{
	static_cast<ComponentAudioSource*>(App->scene->GetGameObjectByName("Camera Holder")->GetComponent(Component::ComponentType::AUDIO_SOURCE))->PlayEvent("Play_BossMusic");
}

void BossController::StopMusic() const
{
	static_cast<ComponentAudioSource*>(App->scene->GetGameObjectByName("Camera Holder")->GetComponent(Component::ComponentType::AUDIO_SOURCE))->PlayEvent("Stop_BossMusic");

}

void BossController::BlockInitialPath() const
{
	float3 new_position = wall_collider->transform.GetGlobalTranslation();
	new_position.y = ground_y_position;
	wall_collider->transform.SetGlobalMatrixTranslation(new_position);
}

void BossController::PlayersSpawning()
{
	waiting_for_spawning = true;
}

void BossController::CheckPlayersSpawning()
{
	if (waiting_for_spawning)
	{
		if (current_time_waiting_for_spanwing >= time_waiting_for_spawning)
		{
			waiting_for_spawning = false;
			current_time_waiting_for_spanwing = 0.f;
		}
		current_time_waiting_for_spanwing += App->time->delta_time;
	}
}

bool BossController::IsBossObstacle() const
{
	return boss_is_obstacle;
}

void BossController::CheckIfBossIsObstacle()
{
	if(boss_is_obstacle)
	{
		if(current_time_not_being_obstacle >= max_time_not_being_obstacle)
		{
			current_time_not_being_obstacle = 0.f;
			boss_is_obstacle = false;
		}
		current_time_not_being_obstacle += App->time->delta_time;
	}
}

void BossController::InitCrystals()
{
	for(auto go : owner->GetChildrenWithName("Crystals")->children)
	{
		go->SetEnabled(false);
		crystals.emplace_back(Projectile(go));
	}
}

void BossController::InitRocks()
{
	GameObject* rocks_go = owner->GetChildrenWithName("Rocks");
	rocks_go->children[0]->SetEnabled(false);
	rocks.emplace_back(Projectile(rocks_go->children[0]));
	for(size_t i = 0; i < 20; ++i)
	{
		GameObject* copy = App->scene->DuplicateGameObject(rocks_go->children[0], rocks_go);
		rocks.emplace_back(Projectile(copy));
	}
}

void BossController::InitEffects()
{
	//Slam dissolver
	right_slam_dissolver = static_cast<Dissolver*>(App->scene->GetGameObjectByName("SlamImpactEffect")->GetComponentScript("Dissolver")->script);
	right_slam_dissolver->SetDissolvingTime(1000.f);
	right_slam_dissolver->owner->SetEnabled(false);

	//Earthquake effect
	earthquake_effect_animation = static_cast<ComponentAnimation*>(App->scene->GetGameObjectByName("Shockwave_Mesh")->GetComponent(Component::ComponentType::ANIMATION));
	earthquake_effect_renderer = static_cast<ComponentMeshRenderer*>(App->scene->GetGameObjectByName("Shockwave_Mesh")->children[0]->GetComponent(Component::ComponentType::MESH_RENDERER));
	earthquake_effect_animation->owner->SetEnabled(false);

	//Crystal Guided Effect
	guided_crystal_effect = App->scene->GetGameObjectByName("BobbyBurrow");
	guided_crystal_effect->SetEnabled(false);
	guided_crystal_effect_player2 = App->scene->DuplicateGameObject(guided_crystal_effect, guided_crystal_effect->parent);

	//Crack
	crack_effect = static_cast<ComponentMeshRenderer*>(App->scene->GetGameObjectByName("Crack Mesh")->GetComponent(Component::ComponentType::MESH_RENDERER));
	crack_effect->owner->SetEnabled(false);

	//Smoke and shadow Avalanche
	GameObject* smoke_avalanche_go = App->scene->GetGameObjectByName("SmokeAvalanche");
	GameObject* shadow_avalanche_go = App->scene->GetGameObjectByName("RocksShadow");
	rock_crash_smoke_vfxs.emplace_back(static_cast<ComponentParticleSystem*>(smoke_avalanche_go->GetComponent(Component::ComponentType::PARTICLE_SYSTEM)));
	rock_crash_smoke_vfxs[0]->Stop();
	shadow_avalanche_go->SetEnabled(false);
	for(size_t i = 1; i < 20; ++i)
	{
		//Smoke
		GameObject* duplicated_smoke = App->scene->DuplicateGameObject(smoke_avalanche_go, smoke_avalanche_go->parent);
		rock_crash_smoke_vfxs.emplace_back(static_cast<ComponentParticleSystem*>(duplicated_smoke->GetComponent(Component::ComponentType::PARTICLE_SYSTEM)));
		rock_crash_smoke_vfxs[i]->Stop();
	
		//Shadow
		GameObject* duplicated_shadow = App->scene->DuplicateGameObject(shadow_avalanche_go, shadow_avalanche_go->parent);
		rocks_shadows_go.emplace_back(duplicated_shadow);
		duplicated_shadow->SetEnabled(false);
	}

	//Smoke and rocks dispersion
	smoke_dispersion = static_cast<ComponentBillboard*>(App->scene->GetGameObjectByName("SmokeDispersion")->GetComponent(Component::ComponentType::BILLBOARD));
	smoke_dispersion->Disable();
	rocks_dispersion = static_cast<ComponentBillboard*>(App->scene->GetGameObjectByName("RocksDispersion")->GetComponent(Component::ComponentType::BILLBOARD));
	rocks_dispersion->Disable();

	GameObject* smoke_dispersion_go = App->scene->GetGameObjectByName("SmokeDispersion");
	GameObject* rock_dispersion_go = App->scene->GetGameObjectByName("RocksDispersion");
	rocks_dispersion_vector.emplace_back(static_cast<ComponentBillboard*>(rock_dispersion_go->GetComponent(Component::ComponentType::BILLBOARD)));
	rock_crash_smoke_vfxs[0]->Disable();
	smoke_dispersion_vector.emplace_back(static_cast<ComponentBillboard*>(smoke_dispersion_go->GetComponent(Component::ComponentType::BILLBOARD)));
	smoke_dispersion_vector[0]->Disable();

	for (size_t i = 1; i < 20; ++i)
	{
		//Smoke
		GameObject* duplicated_smoke = App->scene->DuplicateGameObject(smoke_dispersion_go, smoke_dispersion_go->parent);
		smoke_dispersion_vector.emplace_back(static_cast<ComponentBillboard*>(duplicated_smoke->GetComponent(Component::ComponentType::BILLBOARD)));
		smoke_dispersion_vector[i]->Disable();

		//Rocks
		GameObject* duplicated_rocks = App->scene->DuplicateGameObject(rock_dispersion_go, rock_dispersion_go->parent);
		rocks_dispersion_vector.emplace_back(static_cast<ComponentBillboard*>(duplicated_rocks->GetComponent(Component::ComponentType::BILLBOARD)));
		rocks_dispersion_vector[i]->Disable();
	}
}

void BossController::RotateBoss(PlayerController::WhichPlayer player) const
{
	float3 direction_to_look = GetRotationDirection(static_cast<uint32_t>(player));
	//Rotate Boss
	float3 desired_direction = float3(direction_to_look.x, 0, direction_to_look.z);
	Quat desired_rotation = Quat::LookAt(float3::unitZ, desired_direction.Normalized(), float3::unitY, float3::unitY);
	Quat new_rotation = Quat::Slerp(owner->transform.GetRotation(), desired_rotation, App->time->delta_time * 0.0012f);
	owner->transform.SetRotation(new_rotation);
}

void BossController::RotateBoss(const float3& direction_to_look) const
{
	//Rotate Boss
	float3 desired_direction = float3(direction_to_look.x, 0, direction_to_look.z);
	Quat desired_rotation = Quat::LookAt(float3::unitZ, desired_direction.Normalized(), float3::unitY, float3::unitY);
	Quat new_rotation = Quat::Slerp(owner->transform.GetRotation(), desired_rotation, App->time->delta_time * 0.0012f);
	owner->transform.SetRotation(new_rotation);
}

bool BossController::HitByBall(const float3& ball_position, float3& hit_position_object) const
{
	if (!vulnerable)
	{
		return false;
	}

	for (auto& zone : vulnerable_zones)
	{
		if (zone->active) 
		{
			if (zone->zone_go->aabb.bounding_box.Contains(ball_position))
			{
				hit_position_object = zone->zone_go->transform.GetGlobalTranslation();
				return true;
			}
		}
	}

	return false;
}

bool BossController::HitByPlayer(const float3& player_position, const float3& player_front_vector, const float range, float3& hit_position_object) const
{
	if (!vulnerable)
	{
		return false;
	}

	float3 collision_detector_position = player_position + float3::unitY * 0.75f;
	LineSegment ray_line_collision_detector(collision_detector_position, collision_detector_position + player_front_vector * range / 1.5f);
	
	std::vector<ComponentMeshRenderer*> vulnerable_meshes;

	for (auto& zone : vulnerable_zones)
	{
		if (zone->active) 
		{
			vulnerable_meshes.emplace_back(static_cast<ComponentMeshRenderer*>(zone->zone_go->GetComponent(Component::ComponentType::MESH_RENDERER)));
		}
	}

	int index;
	bool intersects = App->renderer->MeshesIntersectsWithRay(ray_line_collision_detector, vulnerable_meshes, index);
	if (intersects)
	{
		hit_position_object = vulnerable_meshes[index]->owner->transform.GetGlobalTranslation();

		//Trigger the hit sound on the bone
		TriggerHitSound(vulnerable_zones[index]->bone);

		return true;
	}

	return false;
}

void BossController::TakeDamage(float damage, const float3& damage_dealer_position, const float3& hitted_position_object)
{
	if (!vulnerable) 
	{
		return;
	}

	hit_blinker->Blink();

	float true_damage = damage;

	if(stunned)
	{
		true_damage *= 1.5f;
	}

	current_health -= true_damage;
	ui_manager->SpawnDamageIndicator(true_damage, hitted_position_object + 2 * float3::unitY, damage_dealer_position);
	//Update Health Bar here
	if (current_health <= 0.f) 
	{
		is_alive = false;
		current_health = 0.f;
		BossKilled();
		//Trigger Die animation
	}
	else if(current_health < next_health_stage && !stages.empty())
	{
		health_factor -= 0.33f;
		next_health_stage = total_health * health_factor;
		current_stage->ChangeState();
		delete current_stage;
		current_stage = stages.top();
		stages.pop();
	}
	boss_health_bar->SetProgress(current_health / total_health);
}

float3 BossController::GetRotationDirection(unsigned player) const
{
	float3 direction_to_look = float3::zero;

	if (is_multiplayer)
	{
		if(player == 0)
		{
			direction_to_look = player_1->player_go->transform.GetGlobalTranslation();
		}
		else
		{
			direction_to_look = player_2->player_go->transform.GetGlobalTranslation();
		}
	}
	else
	{
		direction_to_look = player_1->player_go->transform.GetGlobalTranslation();
	}

	return direction_to_look - owner->transform.GetGlobalTranslation();
}

/** Depending of which stage we are we allocate next stage and trigger the correct animation */
void BossController::TransitionToNextStage()
{
	if(current_stage_index == 0)
	{
		delete current_stage;
		current_stage = new BossStagePhaseTwo(this);
		vulnerable = true;
		++current_stage_index;
	}
	else if(current_stage_index == 1)
	{

		delete current_stage;
		current_stage = new BossStagePhaseThree(this);
		vulnerable = false;
		++current_stage_index;
	}
	else if(current_stage_index == 2)
	{
		delete current_stage;
		is_alive = false;
		owner->SetEnabled(false);
		boss_health_bar->owner->SetEnabled(false);
	}
}

void BossController::InitBonesGameObjects()
{
	right_hand_go = App->scene->GetGameObjectByName("Finger_2_1_Right");
	right_forearm_go = App->scene->GetGameObjectByName("Hand_Right");
	right_hand_finger = App->scene->GetGameObjectByName("Finger_1_1__Right");

	left_hand_go = App->scene->GetGameObjectByName("Finger_2_1_Left");
	left_forearm_go = App->scene->GetGameObjectByName("Hand_Left");
	left_hand_finger = App->scene->GetGameObjectByName("Finger_1_1_Left");

	chest = App->scene->GetGameObjectByName("Chest");
	head = App->scene->GetGameObjectByName("Head");


}

float3 BossController::ComputeKnockupDirection(const float3& damage_center, const float3& player_position) const
{
	float3 solution;

	//Direction from damage center and player
	float3 direction = player_position - damage_center;
	direction = direction.Normalized();

	//Direction from boss to player
	float3 boss_direction = player_position - owner->transform.GetGlobalTranslation();
	boss_direction = boss_direction.Normalized();

	(boss_direction.Dot(direction) > 0.f) ? solution = direction : solution = boss_direction;
	
	if(damage_center.Equals(player_position))
	{
		solution = boss_direction;
	}

	return boss_direction;
}

void BossController::ApplyCrowdControl(CrowdControl cc, const float3& damage_position, PlayerController* player_controller) const
{
	switch (cc)
	{
		case CrowdControl::STUN:
		{
			player_controller->Stun(time_stunned);
			break;		
		}

		case CrowdControl::KNOCKUP:
		{
			boss_is_obstacle = true;
			float3 knockback_direction = ComputeKnockupDirection(damage_position, player_controller->owner->transform.GetGlobalTranslation());
			player_controller->Knockup(1000.f, knockback_direction);
			break;
		}

		case CrowdControl::NONE:
		{
			break;
		}

		default:
			break;
	}
}

std::string BossController::GetBossBoneName(BossBone bone) const
{
	switch (bone)
	{
		case BossBone::HEAD:
			return std::string("HEAD");

		case BossBone::RIGHT_HAND:
			return std::string("RIGHT_HAND");

		case BossBone::RIGHT_FOREARM:
			return std::string("RIGHT_FOREARM");

		case BossBone::RIGHT_FINGER:
			return std::string("RIGHT_FINGER");

		case BossBone::LEFT_HAND:
			return std::string("LEFT_HAND");

		case BossBone::LEFT_FOREARM:
			return std::string("LEFT_FOREARM");

		case BossBone::LEFT_FINGER:
			return std::string("LEFT_FINGER");

		case BossBone::CHEST:
			return std::string("CHEST");

		case BossBone::NONE:
			return std::string("NONE");

		default:
			break;
	}

	return std::string("INVALID BONE");
}

void BossController::Print(const char* text, ...) const
{
	va_list ap;
	va_start(ap, text);
	App->engine_log->Log(text, ap);
	va_end(ap);
}

void BossController::ComputeIfSmashedPlayer(float damage, uint32_t which_player, CrowdControl cc) const
{
	//Collides so deals dmg
	for (auto& zone : vulnerable_zones)
	{
		if (zone->active) 
		{
			if (which_player == 0)
			{
				if (player_1->player_controller->is_alive && zone->zone_go->aabb.bounding_box.Intersects(player_1->player_go->aabb.bounding_box))
				{
					//Deal damage and stun if needed
					App->engine_log->Log("Player1 got catched by danger zone.");
					ApplyCrowdControl(cc, float3::zero, player_1->player_controller);
					player_1->player_controller->TakeDamage(damage);
					SetCameraTrauma(0.3f);
				}
			}
			else if (is_multiplayer) 
			{
				if (player_2->player_controller->is_alive && zone->zone_go->aabb.bounding_box.Intersects(player_2->player_go->aabb.bounding_box))
				{
					//Deal damage and stun if needed
					App->engine_log->Log("Player2 got catched by danger zone.");
					ApplyCrowdControl(cc, float3::zero, player_2->player_controller);
					player_2->player_controller->TakeDamage(damage);
					SetCameraTrauma(0.3f);
				}
			}
		}
	}

}

/** Compute Danger Zone as a Sphere: if which player is 0 we compute damage for player 1 otherwise for player 2
	Returns true if player is hitted */
bool BossController::ComputeDangerZone(const float3& center, float radius, float damage, CrowdControl cc, uint32_t which_player) const
{
	//Compute damage for player one
	if(which_player == 0)
	{
		//Collides so deals dmg
		if (player_1->player_go->transform.GetGlobalTranslation().Distance(center) <= radius)
		{
			//Deal damage and stun if needed
			App->engine_log->Log("Player1 got catched by danger zone.");
			ApplyCrowdControl(cc, center, player_1->player_controller);
			player_1->player_controller->TakeDamage(damage);
			SetCameraTrauma(0.3f);

			return true;
		}
	}
	else if(is_multiplayer)
	{
		//Collides so deals dmg
		if (player_2->player_go->transform.GetGlobalTranslation().Distance(center) <= radius)
		{
			//Deal damage and stun if needed
			App->engine_log->Log("Player2 got catched by danger zone.");
			ApplyCrowdControl(cc, center, player_2->player_controller);
			player_2->player_controller->TakeDamage(damage);
			SetCameraTrauma(0.3f);
			return true;
		}
	}

	return false;
}

/** Points in order: FloorBottomLeft,FloorBottomRight, FloorTopLeft, FloorTopRight, Same but Upper*/
void BossController::ComputeDangerZone(const float3 points[8], float damage, CrowdControl cc) const
{
	DrawBox(points);
}

bool BossController::ComputeDangerZoneCircle(const float3& center, float radius, float damage, float lenght, CrowdControl cc, uint32_t which_player) const
{
	//Compute damage for player one
	if (which_player == 0)
	{
		//Collides so deals dmg
		float distance_player_1 = player_1->player_go->transform.GetGlobalTranslation().Distance(center);
		if (distance_player_1  >= (radius - lenght)  && distance_player_1 <= (radius + lenght) && player_1->player_controller->player_movement->IsGrounded())
		{
			//Deal damage and stun if needed
			App->engine_log->Log("Player1 got catched by danger zone.");
			ApplyCrowdControl(cc, center, player_1->player_controller);
			player_1->player_controller->TakeDamage(damage);
			SetCameraTrauma(0.3f);

			return true;
		}
	}
	else if (is_multiplayer)
	{
		//Collides so deals dmg
		float distance_player_2 = player_2->player_go->transform.GetGlobalTranslation().Distance(center);
		if (distance_player_2 >= (radius - lenght) && distance_player_2 <= (radius + lenght) && player_2->player_controller->player_movement->IsGrounded())
		{
			//Deal damage and stun if needed
			App->engine_log->Log("Player2 got catched by danger zone.");
			ApplyCrowdControl(cc, center, player_2->player_controller);
			player_2->player_controller->TakeDamage(damage);
			SetCameraTrauma(0.3f);
			return true;
		}
	}

	return false;
}

void BossController::DrawSphere(const float3& center, float radius) const
{
	App->debug_draw->RenderSphere(center, radius, float3(1.f, 0.f, 0.f));
}

void BossController::DrawBox(const float3 points[8]) const
{
	App->debug_draw->RenderBox(points, float3(1.f, 0.f, 0.f));
}

void BossController::DrawCircle(const float3& center, float radius) const
{
	App->debug_draw->RenderCircle(center, radius, float3::unitY, float3(1.f,0.f,1.f));
}

bool BossController::IsAnimationOver()
{
	return animation->GetCurrentClipPercentatge() >= 0.95f;
}

float3 BossController::GetPlayerPosition(size_t player)
{
	return (player == 0) ? player_1->player_go->transform.GetGlobalTranslation() : player_2->player_go->transform.GetGlobalTranslation();
}

bool BossController::IsMultiplayer() const
{
	return is_multiplayer;
}


void BossController::BossKilled()
{
	//Update Event Manager to Spawn whatever would appear after killing the boss
	event_manager->boss_killed = true;
	FreezeCamera(false);
	StopMusic();
}

float3 BossController::GetCameraPosition() const
{
	return camera_go->transform.GetGlobalTranslation();
}

void BossController::UpdateCrystals()
{
	for(auto& crystal : crystals)
	{
		if(crystal.spawned)
		{
			//Update y position of crystal
			float3 target_position = crystal.projectile_go->transform.GetGlobalTranslation();
			target_position.y = 1.5f;

			float3 new_crystal_position = float3::Lerp(crystal.projectile_go->transform.GetGlobalTranslation(), target_position, 1.f - crystal.remaining_time_alive / remaining_crystal_time_alive);
			crystal.projectile_go->transform.SetGlobalMatrixTranslation(new_crystal_position);

			//Erase time
			crystal.remaining_time_alive -= App->time->delta_time;

			if (crystal.remaining_time_alive <= 750.f && !crystal.vfx_played)
			{
				//EmitAvalancheSmoke(crystal.projectile_go->transform.GetGlobalTranslation());
				PlayGreyEffect(target_position, 2.5f, 2.5, 1);
				PlaySmokeDispersionEffect(target_position + float3::unitY, 7, 6, 700);
				PlayRocksDispersionEffect(target_position + float3::unitY, 5, 4, 1250);
				crystal.vfx_played = true;
			}

			if(crystal.remaining_time_alive <= 0.f)
			{
				crystal.remaining_time_alive = 0.f;
				crystal.spawned = false;
				crystal.vfx_played = false;
				crystal.projectile_go->transform.SetGlobalMatrixTranslation(float3::zero);
				crystal.projectile_go->SetEnabled(false);
			}

		}
	}
}

void BossController::UpdateRocks()
{
	size_t index = 0;
	for(auto& rock : rocks)
	{
		if(rock.spawned)
		{
			float3 current_rock_position = rock.projectile_go->transform.GetGlobalTranslation();
			float3 new_rock_position = float3::Lerp(
				float3(current_rock_position.x, 30.f, current_rock_position.z),
				float3(current_rock_position.x, ground_y_position, current_rock_position.z),
				1.f - rock.remaining_time_alive / rock.total_time_alive
			);

			rock.projectile_go->transform.SetGlobalMatrixTranslation(new_rock_position);

			if(!rock.player1_already_hitted)
			{
				rock.player1_already_hitted = ComputeDangerZone(new_rock_position, 2.f, 60.f, CrowdControl::STUN);
			}

			if (!rock.player2_already_hitted)
			{
				rock.player2_already_hitted = ComputeDangerZone(new_rock_position, 2.f, 60.f, CrowdControl::STUN, 1);
			}

			rock.remaining_time_alive -= App->time->delta_time;

			//DrawSphere(float3(current_rock_position.x, ground_y_position, current_rock_position.z), 2.f);
			UpdateShadowRock(index);

			if(rock.remaining_time_alive <= 0.f)
			{	
				//Rock explodes
				rock.remaining_time_alive = 0.f;
				rock.total_time_alive = 0.f;
				rock.spawned = false;
				rock.player1_already_hitted = false;
				rock.player2_already_hitted = false;
				rock.projectile_go->SetEnabled(false);
				//Call VFX methods here for crystal destruction

				SetCameraTrauma(avalanche_trauma);
				PlayRocksDispersionEffect(rock.projectile_go->transform.GetGlobalTranslation(),5 , 2.5f , 900);
				PlaySmokeDispersionEffect(rock.projectile_go->transform.GetGlobalTranslation(),6 , 3, 1100);
				//EmitAvalancheSmoke(rock.projectile_go->transform.GetGlobalTranslation());
				PlayGreyEffect(rock.projectile_go->transform.GetGlobalTranslation(), 2.f, 2.f, 0.6f);
				//Shadow Rock
				UnSpawnShadowRock(index);

				//SFX
				ActiveSound(BossBone::HEAD, "Play_RockExploding");

				rock.projectile_go->transform.SetGlobalMatrixTranslation(float3::zero);
			}
		}
		++index;
	}
}

void BossController::SpawnCrystal(const float3& spawn_position)
{
	for(auto& crystal : crystals)
	{
		if(!crystal.spawned)
		{
			crystal.projectile_go->SetEnabled(true);
			crystal.projectile_go->transform.SetGlobalMatrixTranslation(float3(spawn_position.x, -2.5f, spawn_position.z));
			crystal.spawned = true;
			crystal.remaining_time_alive = remaining_crystal_time_alive;

			return;
		}
	}
}

void BossController::SpawnRock(const float3& spawn_position, float exp_value)
{
	size_t index = 0;
	for(auto& rock : rocks)
	{
		if(!rock.spawned)
		{
			rock.projectile_go->transform.SetGlobalMatrixTranslation(float3(spawn_position.x, 30.f, spawn_position.z));
			rock.spawned = true;
			rock.projectile_go->SetEnabled(true);
			float time_alive = math::Lerp(min_remaining_rock_time_alive, max_remaining_rock_time_alive, exp_value);
			rock.remaining_time_alive = max_remaining_rock_time_alive;
			rock.total_time_alive = max_remaining_rock_time_alive;

			//Spawn shadow
			SpawnShadowRock(float3(spawn_position.x, ground_y_position + 0.3f, spawn_position.z), index);

			return;		
		}
		++index;
	}
}

bool BossController::IsAvalancheDone() const
{
	for(auto& rock : rocks)
	{
		if(rock.spawned)
		{
			return false;
		}
	}

	return true;
}

void BossController::FreezeCamera(bool state) const
{
	camera_controller->freeze = state;
}

bool BossController::CheckIfRockIsValid(const float3& rock_position) const
{
	//To test whether two spheres overlap you just have to check whether the distance between their center is smaller than the sum of their radius
	for (auto& rock : rocks)
	{
		if (rock.spawned)
		{
			//Check point from ground level
			float3 ground_position = rock.projectile_go->transform.GetGlobalTranslation();
			ground_position.y = ground_y_position;
			
			if(ground_position.Distance(rock_position) < 2.f * 2)
			{
				return false;
			}
		}
	}

	return true;
}

float BossController::GetRandom(float min, float max) {

	return ((float)rand() / RAND_MAX) * (max - min) + min;

}

BattleEvent* BossController::GetBattleEvent()
{
	GameObject* battle = App->scene->GetGameObjectByName("Battle Boss Event");
	if (battle != nullptr)
	{

			return static_cast<BattleEvent*>(battle->GetComponentScript("BattleEvent")->script);

	}
	return nullptr;
}

void BossController::TriggerBossAnimation(const std::string& trigger)
{
	animation->ActiveAnimation(trigger);
}

bool BossController::IsAnimationOnState(const std::string& state) const
{
	return animation->IsOnState(state);
}

float BossController::GetAnimationPercentage() const
{
	return animation->GetCurrentClipPercentatge();
}

GameObject* BossController::GetBoneGameObject(BossBone bone) const
{
	switch (bone)
	{
		case BossBone::HEAD:
			return head;

		case BossBone::RIGHT_HAND:
			return right_hand_go;

		case BossBone::RIGHT_FOREARM:
			return right_forearm_go;

		case BossBone::RIGHT_FINGER:
			return right_hand_finger;

		case BossBone::LEFT_HAND:
			return left_hand_go;

		case BossBone::LEFT_FOREARM:
			return left_forearm_go;

		case BossBone::LEFT_FINGER:
			return left_hand_finger;

		case BossBone::CHEST:
			return chest;

		default:
			break;
	}

	App->engine_log->Log("Invalid Bone Selected");

	return nullptr;
}

float3 BossController::GetBonesGameObjectPosition(BossBone bone) const
{
	return GetBoneGameObject(bone)->transform.GetGlobalTranslation();
}

void BossController::SpawnSlamEffect(BossBone bone)
{
	float3 dissolve_position = GetBonesGameObjectPosition(bone);
	right_slam_dissolver->owner->transform.SetGlobalMatrixTranslation(dissolve_position);
	right_slam_dissolver->owner->SetEnabled(true);
	right_slam_dissolver->ResetDissolve();
	right_slam_dissolver->Dissolve();
}

void BossController::SpawnGuidedEffect(const float3& position) const
{

	GameObject* auxiliar_effect = nullptr;
	if(!guided_crystal_effect->IsEnabled())
	{
		auxiliar_effect = guided_crystal_effect;
	}
	else
	{
		auxiliar_effect = guided_crystal_effect_player2;
	}

	auxiliar_effect->transform.SetGlobalMatrixTranslation(float3(position.x, position.y + 0.5f, position.z));
	auxiliar_effect->SetEnabled(true);
	for (auto& child : auxiliar_effect->children)
	{
		Component* particles = child->GetComponent(Component::ComponentType::PARTICLE_SYSTEM);
		if (particles)
		{
			static_cast<ComponentParticleSystem*>(particles)->Emit(10);
		}
	}
}

void BossController::UnspawnGuidedEffect() const
{
	guided_crystal_effect->SetEnabled(false);
	guided_crystal_effect_player2->SetEnabled(false);
}

void BossController::SpawnEarthquakeEffect()
{
	earthquake_effect_renderer->material_to_render->diffuse_color[3] = 1.f;
	earthquake_effect_animation->owner->SetEnabled(true);
	earthquake_effect_animation->Play();
	crack_effect->owner->transform.SetGlobalMatrixRotation(owner->transform.GetGlobalRotation());
	crack_effect->owner->SetEnabled(true);
	crack_effect->material_to_render->transparency = 1.f;
}

void BossController::UpdateEarthquakeEffect()
{
	float anim_percentage = earthquake_effect_animation->GetCurrentClipPercentatge();
	
	earthquake_effect_renderer->material_to_render->transparency = 1.f - anim_percentage;
	crack_effect->material_to_render->transparency = 1.f - anim_percentage;

	if(anim_percentage >= 0.95f)
	{
		earthquake_effect_animation->owner->SetEnabled(false);
		crack_effect->owner->SetEnabled(false);
	}
}

void BossController::EmitAvalancheSmoke(const float3& position) const
{
	for(auto& vfx : rock_crash_smoke_vfxs)
	{
		if(!vfx->IsEmitting())
		{
			vfx->owner->transform.SetGlobalMatrixTranslation(position);
			vfx->Play();
			vfx->Emit(8);
			return;
		}
	}
}

void BossController::SpawnShadowRock(const float3& position, size_t rock_index) const
{
	rocks_shadows_go[rock_index]->transform.SetGlobalMatrixTranslation(position);
	rocks_shadows_go[rock_index]->SetEnabled(true);
}

void BossController::UpdateShadowRock(size_t rock_index) const
{
	float3 new_shadow_scale = float3::Lerp(float3(min_shadow_scale), float3(max_shadow_scale), 1.f - rocks[rock_index].remaining_time_alive / rocks[rock_index].total_time_alive);
	rocks_shadows_go[rock_index]->transform.SetGlobalMatrixScale(new_shadow_scale);
}

void BossController::UnSpawnShadowRock(size_t rock_index) const
{
	rocks_shadows_go[rock_index]->SetEnabled(false);
	rocks_shadows_go[rock_index]->transform.SetGlobalMatrixScale(float3(min_shadow_scale));
}

void BossController::PlaySmokeDispersionEffect(const float3& position, float width, float height, size_t animation_time) const
{
	for (auto& vfx : smoke_dispersion_vector)
	{
		if (!vfx->IsPlaying())
		{
			vfx->width = width;
			vfx->height = height;
			vfx->SetAnimationTime(animation_time);
			vfx->owner->transform.SetGlobalMatrixTranslation(position);
			vfx->Play();
			return;
		}
	}
}

void BossController::PlayRocksDispersionEffect(const float3& position, float width, float height, size_t animation_time) const
{
	for (auto& vfx : rocks_dispersion_vector)
	{
		if (!vfx->IsPlaying())
		{
			vfx->width = width;
			vfx->height = height;
			vfx->SetAnimationTime(animation_time);
			vfx->owner->transform.SetGlobalMatrixTranslation(position);
			vfx->Play();
			return;
		}
	}
}

void BossController::PlayGreyEffect(const float3& position, float width, float heigth, float lifetime)
{
	for (auto& vfx : rock_crash_smoke_vfxs)
	{
		if (!vfx->IsEmitting())
		{
			vfx->owner->transform.SetGlobalMatrixTranslation(position);
			vfx->particles_life_time = lifetime;
			vfx->particles_size = float2(width, heigth);
			vfx->Play();
			vfx->Emit(8);
			return;
		}
	}
}


void BossController::SetCameraTrauma(float trauma) const
{
	camera_shake->trauma = trauma;
}
