#include "Necroplant.h"

#include "PlayerController.h"
#include "Module/ModuleTime.h"
#include "NecroProjectile.h"

#include "Component/ComponentAnimation.h"
#include "Component/ComponentAudioSource.h"
#include "Component/ComponentScript.h"
#include "Component/ComponentTransform.h"

#include "EditorUI/Panel/InspectorSubpanel/PanelComponent.h"

#include "Main/Application.h"
#include "Main/GameObject.h"
#include "Log/EngineLog.h"
#include "Module/ModuleDebugDraw.h"
#include "Module/ModuleInput.h"
#include "Module/ModuleScene.h"

#include <imgui.h>

#include "Attacks.h"
#include "BattleEvent.h"
#include "EnemyManager.h"
#include "EventManager.h"
#include "LightOrbsManager.h"
#include "HitBlinker.h"
#include "NecroBurrow.h"
#include "NecroMuzzle.h"
#include "UIManager.h"
#include "WorldManager.h"

#define SLERP_VALUE 0.1f

Necroplant* NecroplantDLL()
{
	Necroplant* instance = new Necroplant();
	return instance;
}

Necroplant::Necroplant()
{
	panel = new PanelComponent();
}

void Necroplant::Awake()
{
	enemy_type = EnemyType::NECROPLANT;
	EnemyController::Awake();
	event_manager = static_cast<CEventManager*>(App->scene->GetGameObjectByName("EventManager")->GetComponentScript("EventManager")->script);

	max_health = 100.f;
	health = max_health;

	necroplant_upper_body = owner->GetChildrenWithTag("NecroBody");
	necroplant_roots_animation = static_cast<ComponentAnimation*>(owner->GetChildrenWithTag("NecroRoots")->GetComponent(Component::ComponentType::ANIMATION));

	GameObject* necro_muzzle_go = owner->GetChildrenWithName("NecroMuzzle");
	necro_muzzle = static_cast<NecroMuzzle*>(necro_muzzle_go->GetComponentScript("NecroMuzzle")->script);

	muzzle_transform = &necro_muzzle_go->transform;

	GameObject* necroprojectiles_parent = owner->GetChildrenWithName("NecroProjectiles");
	for (size_t i = 0; i < necroprojectiles_parent->children.size(); ++i)
	{
		GameObject* necroprojectile = necroprojectiles_parent->children[i];
		necroprojectiles[i] = static_cast<NecroProjectile*>(necroprojectile->GetComponentScript("NecroProjectile")->script);
	}

	necro_source = static_cast<ComponentAudioSource*>(owner->GetComponent(Component::ComponentType::AUDIO_SOURCE));

	necro_burrow = static_cast<NecroBurrow*>(owner->GetChildrenWithName("NecroBurrow")->GetComponentScript("NecroBurrow")->script);
}

void Necroplant::Start()
{
	EnemyController::Start();
}

// Update is called once per frame
void Necroplant::Update()
{
	if (world_manager->on_pause || !is_alive)
	{
		return;
	}

	if (show_debug_draws)
	{
		App->debug_draw->RenderCircle(owner->transform.GetGlobalTranslation(), detect_player_distance);
		App->debug_draw->RenderCircle(owner->transform.GetGlobalTranslation(), danger_radius);
	}

	GetClosestTarget();
	switch (current_state)
	{
	case NecroplantState::IDLE:
		if (PlayerInRange(detect_player_distance))
		{
			current_state = NecroplantState::AIMING;
			time_since_last_attack = 0;
			break;
		}
		break;

	case NecroplantState::AIMING:
	{
		AimingState();
		break;
	}
	case NecroplantState::NORMAL_ATTACKING:
		NormalAttackState();
		break;

	case NecroplantState::BARRAGE_ATTACKING:
		BarrageAttackState();
		break;

	case NecroplantState::CONE_ATTACKING:
		ConeAttackState();
		break;
	
	case NecroplantState::EMERGING:
		EmergingState();
		break;

	case NecroplantState::BURROWING:
		BurrowingState();
		break;

	case NecroplantState::DYING:
		if (animation->GetCurrentClipPercentatge() >= 0.9f)
		{
			enemy_manager->KillEnemy(this);
		}

		break;
	}

}

void Necroplant::AimingState()
{
	if (current_target == nullptr || !PlayerInRange(detect_player_distance))
	{
		current_state = NecroplantState::IDLE;
		state_name = "Idle";
		return;
	}

	if (!burrowed_recently && PlayerInRange(danger_radius) && necro_spot != nullptr)
	{
		burrowed_recently = true;

		time_since_last_attack = 0;
		current_burrowing_time = 0.f;

		necro_source->PlayEvent("Play_Necrotaunt");
		necro_burrow->Hide();
		necroplant_roots_animation->ActiveAnimation("Burrow");

		current_state = NecroplantState::BURROWING;
		state_name = "Burrowing";
		return;
	}

	time_since_last_attack += App->time->delta_time;

	if (time_since_last_attack > attack_cooldown)
	{
		burrowed_recently = false;
		if (num_attacks == 1)
		{
			float random_value = (rand() % 100) * 0.01f;
			if (random_value < 0.5f)
			{
				current_state = NecroplantState::CONE_ATTACKING;
				state_name = "Cone Attacking";
				time_since_last_attack = 0;
				animation->ActiveAnimation("FastAttack");
				num_attacks = 0;
			}
			else
			{
				current_state = NecroplantState::BARRAGE_ATTACKING;
				state_name = "Barrage Attacking";
				time_since_last_attack = 0;
				animation->ActiveAnimation("FastAttack");
				num_attacks = 0;
			}
		}
		else
		{
			current_state = NecroplantState::NORMAL_ATTACKING;
			state_name = "Normal Attacking";
			animation->ActiveAnimation("Attack");
		}
		return;
	}
	
	LookAtCurrentTarget();
}

void Necroplant::NormalAttackState()
{
	if (current_target == nullptr)
	{
		current_state = NecroplantState::IDLE;
		state_name = "Idle";
		animation->ActiveAnimation("Idle");
		return;
	}

	if (animation->IsOnState("Attack") && animation->GetCurrentClipPercentatge() > 0.7)
	{
		float3 front_direction = necroplant_upper_body->transform.GetFrontVector();
		float3 projectile_direction = float3(front_direction.x, 0.f, front_direction.z);

		SpawnProjectile(projectile_direction);
		current_state = NecroplantState::AIMING;
		state_name = "Aiming";
		animation->ActiveAnimation("Idle");
		time_since_last_attack = 0;
	}
	else
	{
		LookAtCurrentTarget();
	}
}

void Necroplant::BarrageAttackState()
{
	if (current_target == nullptr)
	{
		current_state = NecroplantState::IDLE;
		state_name = "Idle";
		animation->ActiveAnimation("Idle");
		return;
	}

	if (already_attacked_in_iteration && animation->GetCurrentClipPercentatge() < 0.1f)
	{
		already_attacked_in_iteration = false;
	}

	if (!already_attacked_in_iteration && animation->IsOnState("Fast Attack") && animation->GetCurrentClipPercentatge() > 0.7f)
	{
		float3 front_direction = necroplant_upper_body->transform.GetFrontVector();
		float3 projectile_direction = float3(front_direction.x, 0.f, front_direction.z);

		SpawnProjectile(projectile_direction);
		already_attacked_in_iteration = true;
		if (num_attacks == 3)
		{
			current_state = NecroplantState::AIMING;
			state_name = "Aiming";
			animation->ActiveAnimation("Idle");
			time_since_last_attack = 0;
			num_attacks = 0;
		}
	}
	else
	{
		LookAtCurrentTarget();
	}
}

void Necroplant::ConeAttackState()
{
	if (animation->IsOnState("Fast Attack") && animation->GetCurrentClipPercentatge() > 0.7f)
	{
		float3 front_direction = necroplant_upper_body->transform.GetFrontVector();
		float3 cone_starting_aiming_direction = float3(front_direction.x, 0.f, front_direction.z);

		float3 projectile_direction = Quat::RotateAxisAngle(float3::unitY, math::DegToRad(angle_cone_attack)) * cone_starting_aiming_direction;
		Quat desired_rotation = Quat::LookAt(float3::unitZ, projectile_direction.Normalized(), float3::unitY, float3::unitY);
		necroplant_upper_body->transform.SetRotation(desired_rotation);
		SpawnProjectile(projectile_direction.Normalized());

		projectile_direction = cone_starting_aiming_direction;
		desired_rotation = Quat::LookAt(float3::unitZ, projectile_direction.Normalized(), float3::unitY, float3::unitY);
		necroplant_upper_body->transform.SetRotation(desired_rotation);
		SpawnProjectile(projectile_direction.Normalized());

		projectile_direction = Quat::RotateAxisAngle(float3::unitY, math::DegToRad(-angle_cone_attack)) * cone_starting_aiming_direction;
		desired_rotation = Quat::LookAt(float3::unitZ, projectile_direction.Normalized(), float3::unitY, float3::unitY);
		necroplant_upper_body->transform.SetRotation(desired_rotation);
		SpawnProjectile(projectile_direction.Normalized());

		if (num_attacks == 3)
		{
			current_state = NecroplantState::AIMING;
			state_name = "Aiming";
			animation->ActiveAnimation("Idle");
			time_since_last_attack = 0;
			num_attacks = 0;
		}
	}
}

void Necroplant::EmergingState()
{
	current_emerging_time += App->time->delta_time;
	float current_progress = math::Min(1.f, current_emerging_time / emerging_time);

	if (current_target != nullptr)
	{
		LookAtCurrentTarget();
	}
	necroplant_upper_body->transform.SetGlobalMatrixScale(float3(math::Lerp(0.1f, 1.f, current_progress)));

	if (current_progress == 1.f)
	{
		necro_burrow->owner->SetEnabled(false);
		current_state = NecroplantState::IDLE;
		state_name = "Idle";
	}
}

void Necroplant::BurrowingState()
{
	current_burrowing_time += App->time->delta_time;
	float current_progress = math::Min(1.f, current_burrowing_time / burrowing_time);

	necroplant_upper_body->transform.SetGlobalMatrixScale(float3(math::Lerp(1.f, 0.1f, current_progress)));

	if (current_progress == 1.f)
	{
		Necroplant::Necrospot* last_necrospot = necro_spot;
		necro_spot = GetAvailableNecrospot();
		last_necrospot->ocupied = false;
		owner->transform.SetGlobalMatrixTranslation(necro_spot->position);
		necro_spot->ocupied = true;

		current_emerging_time = 0.f;

		necro_source->PlayEvent("Play_Necrotaunt");
		necro_burrow->Show();
		necroplant_roots_animation->ActiveAnimation("Emerge");

		current_state = NecroplantState::EMERGING;
		state_name = "Emerging";
	}
}

Necroplant::Necrospot* Necroplant::GetAvailableNecrospot() const
{
	const std::vector<Necroplant::Necrospot*> necro_spots = event_manager->current_battle->necro_spots;
	std::vector<int> available_necro_spots;
	for (size_t i = 0; i < necro_spots.size(); ++i)
	{
		if (!necro_spots[i]->ocupied)
		{
			available_necro_spots.push_back(i);
		}
	}

	if (available_necro_spots.size() == 0)
	{
		return nullptr;
	}

	int necro_spot_index = rand() % available_necro_spots.size();
	return necro_spots[available_necro_spots[necro_spot_index]];
}

void Necroplant::TakeDamage(float damage, const float3& damage_source_position, float knockback)
{
	if (!is_alive || current_state == NecroplantState::DYING)
	{
		return;
	}

	if (current_state == NecroplantState::EMERGING || current_state == NecroplantState::BURROWING)
	{
		damage = 0;
	}

	health -= damage;
	ui_manager->SpawnDamageIndicator(damage, owner->transform.GetGlobalTranslation(), damage_source_position);
	hit_blinker->Blink();

	if (health <= 0)
	{
		state_name = "Dying";
		animation->ActiveAnimation("Kill");
		DeathVFX();
		necro_source->PlayEvent("Play_Necrodeath");
		current_state = NecroplantState::DYING;
	}
}

//ATTENTION!!! Number of Necrospots should be at least number of Necroplants + 1.
void Necroplant::ResetEnemy()
{
	necro_spot = GetAvailableNecrospot();
	owner->transform.SetGlobalMatrixTranslation(necro_spot->position);
	necro_spot->ocupied = true;

	animation->Stop();
	animation->ActiveAnimation("Idle");
	animation->Play();

	owner->SetEnabled(true);
	necro_source->StopAll();
	for (auto& necroprojectile : necroprojectiles)
	{
		necroprojectile->Hide();
	}
	necro_muzzle->Hide();

	is_alive = true;
	health = max_health;
	num_attacks = 0;

	current_state = NecroplantState::EMERGING;
	state_name = "Emerging";
	current_emerging_time = 0.f;

	necroplant_upper_body->transform.SetGlobalMatrixScale(float3(0.1f));
	necro_source->PlayEvent("Play_Necrotaunt");
	necro_burrow->Show();
	necroplant_roots_animation->Play();
	InitVFX();
}

void Necroplant::Kill()
{
	is_alive = false;
	light_orbs_manager->SpawnOrbs(owner->transform.GetGlobalTranslation(), 2);
	owner->SetEnabled(false);
	if (necro_spot != nullptr)
	{
		necro_spot->ocupied = false;
		necro_spot = nullptr;
	}
}

void Necroplant::LookAtCurrentTarget()
{
	float3 current_target_position = current_target->owner->transform.GetGlobalTranslation();
	float3 look_direction = current_target_position - owner->transform.GetGlobalTranslation();
	float3 desired_direction = float3(look_direction.x, 0, look_direction.z);
	Quat desired_rotation = Quat::LookAt(float3::unitZ, desired_direction.Normalized(), float3::unitY, float3::unitY);
	Quat new_rotation = Quat::Slerp(necroplant_upper_body->transform.GetRotation(), desired_rotation, SLERP_VALUE);
	necroplant_upper_body->transform.SetRotation(new_rotation);
}

void Necroplant::SpawnProjectile(const float3& direction)
{
	++num_attacks;
	unsigned int i = 0; 
	while (i < necroprojectiles.size())
	{
		if (necroprojectiles[i]->IsDead())
		{
			necroprojectiles[i]->Spawn(muzzle_transform->GetGlobalTranslation(), direction);
			necro_muzzle->Shoot();
			necro_source->PlayEvent("Play_Necroprojectile_Shoot");
			return;
		}
		else
		{
			++i;
		}
	}
}

// Use this for showing variables on inspector
void Necroplant::OnInspector(ImGuiContext* context)
{
	EnemyController::OnInspector(context);
	//Necessary to be able to write with imgui
	ImGui::Checkbox("Show debug draws", &show_debug_draws);
	ImGui::TextColored(ImVec4(1.f, 1.f, 0.f, 1.f), state_name.c_str());
	ImGui::DragInt("Current number of attacks", &num_attacks);
}

void Necroplant::HitSFX(NodeID attack)
{
	switch (attack)
	{
		case NodeID::LIGHT_ONE:
			audio_source->PlayEvent("play_receive_hit");
			break;
		case NodeID::LIGHT_TWO:
			audio_source->PlayEvent("Play_Necrotaunt");
			break;
		case NodeID::LIGHT_THREE:
			audio_source->PlayEvent("play_receive_hit");
			break;
		case NodeID::HEAVY_ONE:
			audio_source->PlayEvent("play_receive_hit");
			break;
		case NodeID::HEAVY_TWO:
			audio_source->PlayEvent("play_receive_hit");
			break;
		case NodeID::HEAVY_THREE:
			audio_source->PlayEvent("play_receive_hit");
			break;
		case NodeID::DOWN_SMASH:
			audio_source->PlayEvent("play_receive_hit");
			break;
	}
}
