#include "Mushdoom.h"

#include "Component/ComponentAnimation.h"
#include "Component/ComponentAudioSource.h"
#include "Component/ComponentBillboard.h"
#include "Component/ComponentScript.h"
#include "Component/ComponentTransform.h"
#include "Component/ComponentParticleSystem.h"

#include "Main/Application.h"
#include "Main/GameObject.h"
#include "Module/ModuleAI.h"
#include "Module/ModuleInput.h"
#include "Module/ModuleScene.h"
#include "Module/ModuleTime.h"


#include <imgui.h>
#include <imgui_stdlib.h>
#include <string>

#include "Attacks.h"
#include "LightOrbsManager.h"
#include "MushdoomBall.h"
#include "MushdoomStateAttack.h"
#include "MushdoomStateDie.h"
#include "MushdoomStateHit.h"
#include "MushdoomStateIdle.h"
#include "MushdoomStatePursue.h"
#include "MushdoomStateScream.h"
#include "MushdoomStateThrow.h"
#include "HitBlinker.h"
#include "PlayerController.h"
#include "UIManager.h"
#include "WorldManager.h"

Mushdoom* MushdoomDLL()
{
	Mushdoom* instance = new Mushdoom();
	return instance;
}

Mushdoom::Mushdoom()
{
	current_state = new MushdoomState();
	previous_state = new MushdoomState();
	idle_state = new MushdoomStateIdle(this);
	scream_state = new MushdoomStateScream(this);
	pursue_state = new MushdoomStatePursue(this);
	attack_state = new MushdoomStateAttack(this);
	die_state = new MushdoomStateDie(this);
	hit_state = new MushdoomStateHit(this);
	throw_state = new MushdoomStateThrow(this);

	//throw_delay_limits = float2(1, 1);
	move_speed = 5;
}

Mushdoom::~Mushdoom()
{
	delete current_state;
	delete previous_state;
	delete idle_state;
	delete scream_state;
	delete pursue_state;
	delete attack_state;
	delete die_state;
	delete hit_state;
	delete throw_state;
}

void Mushdoom::Awake()
{
	enemy_type = EnemyType::MUSHDOOM;
	EnemyController::Awake();

	for (const auto& ch : owner->children)
	{
		if (ch->tag == "Projectile")
		{
			mushdoom_ball = static_cast<MushdoomBall*>(ch->GetComponentScript("MushdoomBall")->script);
			break;
		}
	}

	for (const auto& ch : owner->children)
	{
		if (ch->tag == "ThrowEffect")
		{
			throw_effect = static_cast<ComponentParticleSystem*>(ch->GetComponent(Component::ComponentType::PARTICLE_SYSTEM));
			break;
		}
	}

	for (const auto& ch : owner->children)
	{
		if (ch->tag == "DieMushdoomBall")
		{
			ball_die_effect = static_cast<ComponentParticleSystem*>(ch->GetComponent(Component::ComponentType::PARTICLE_SYSTEM));
			break;
		}
	}

	if (mushdoom_ball)
	{
		mushdoom_ball->owner->SetEnabled(false);
	}

}

void Mushdoom::Start()
{
	EnemyController::Start();
	current_state = idle_state;
	current_state->OnStateEnter();
	ball_die_effect->Stop();
	throw_effect->Stop();
	//throw_delay = rand() % (int)throw_delay_limits.x + (int)throw_delay_limits.y;
}

void Mushdoom::Update()
{
	if (world_manager->on_pause)
	{
		return;
	}

	if (is_alive)
	{
		// If some mushdoom get buged we kill them
		if (owner->transform.GetGlobalTranslation().y < -50.f)
		{
			TakeDamage(10000.f, owner->transform.GetGlobalTranslation());
		}

		if (GetClosestTarget() == nullptr && current_state != idle_state && current_state != die_state)
		{
			current_state->Exit(idle_state);
		}

		current_state->OnStateUpdate();
	}
}

void Mushdoom::OnInspector(ImGuiContext* context)
{
	EnemyController::OnInspector(context);

	ImGui::Text("Mushdoom Stats");
	ImGui::InputText("Current State", current_state->state_name, IM_ARRAYSIZE(current_state->state_name), ImGuiInputTextFlags_ReadOnly);
	ImGui::InputFloat("Danger Distance", &danger_distance);
	ImGui::InputFloat("Strafe Distance", &strafe_distance);
	ImGui::InputFloat("Separate Distance", &separation_distance);
	ImGui::InputFloat("Ranged Damage", &attack_ranged_damage);
	ImGui::InputFloat2("Throw Delay", &throw_delay_limits[0]);
	ImGui::Checkbox("Ranged Attack", &ranged_attack);
	ImGui::Checkbox("Engage Player", &engage_player);
}

void Mushdoom::SetProperties(EnemyController* original_enemy)
{
	EnemyController::SetProperties(original_enemy);

	const Mushdoom* original_mushdoom = (Mushdoom*)original_enemy;

	danger_distance = original_mushdoom->danger_distance;
	strafe_distance = original_mushdoom->strafe_distance;
	separation_distance = original_mushdoom->separation_distance;
	attack_ranged_damage = original_mushdoom->attack_ranged_damage;
	attack_range_offset = original_mushdoom->attack_range_offset;
	throw_delay = original_mushdoom->throw_delay;
}

void Mushdoom::Seek(const float3& velocity) const
{
	float3 final_velocity = velocity;

	bool falling = velocity.y < 0.f;

	ComponentCollider* ground_check = collider->DetectCollisionWithGround(ground_ray_percentage).collider;
	bool grounded = ground_check
		&& (ground_check->owner->tag == "EnvironmentCollider"
			|| ground_check->owner->tag == "Respawn" || ground_check->owner->tag == "Groundspawn");

	final_velocity = final_velocity.Normalized().ScaledToLength(move_speed);

	RotateTowardsPlayer();

	if (falling && grounded)
	{
		final_velocity.y = 0.f;
	}
	else if (!grounded && !is_spawning)
	{
		final_velocity.y -= fall_speed;
		float3 new_position = owner->transform.GetGlobalTranslation() + final_velocity * App->time->delta_time * 0.001f;
		owner->transform.SetGlobalMatrixTranslation(new_position);
		collider->UpdateDimensions();
	}

	if (is_spawning)
	{
		SpawnJump();

	}
	else if (ground_check && (ground_check->owner->tag == "Respawn" || ground_check->owner->tag == "Groundspawn"))
	{
		has_ended_spawn = false;
		SpawnMovement(final_velocity);
	}
	else if (ground_check && ground_check->owner->tag == "EnvironmentCollider")
	{
		has_ended_spawn = true;
		float3 position = owner->transform.GetGlobalTranslation() + ((final_velocity.Normalized() + CollisionAvoidance(final_velocity))* move_speed * App->time->delta_time * 0.001f);

		float3 next_position = float3::zero;
		App->artificial_intelligence->FindNextPolyByDirection(position, next_position);

		if (App->artificial_intelligence->IsPointWalkable(position) || grounded)
		{
			owner->transform.SetGlobalMatrixTranslation(position);
			collider->UpdateDimensions();
		}
	}
}

void Mushdoom::Avoid(const float3& velocity) const
{
	float3 avoid_velocity = velocity * -1;
	Seek(avoid_velocity);
}

void Mushdoom::Strafe(const float3& target_position, const float direction) const
{
	float3 perpendicular_vec = float3::unitY.Cross(target_position);
	float3 desired_velocity = perpendicular_vec * direction;
	Seek(desired_velocity);
}

void Mushdoom::TakeDamage(float damage, const float3& damage_source_position, float knockback)
{
	if (!is_alive || animation->IsOnState("Die")) return;

	if(!invincible)
	{
		health -= damage;
		hit_blinker->Blink();
	}

	ui_manager->SpawnDamageIndicator(damage, owner->transform.GetGlobalTranslation(), damage_source_position);

	//Knockback computations
	this->knockback_power = knockback_power;
	float3 current_position = owner->transform.GetGlobalTranslation();
	knockback_target_position = (knockback_direction.Normalized() * this->knockback_power) + current_position;

	if (health <= 0)
	{
		current_state->Exit(die_state);
		is_dying = true;
	}
	else
	{
		current_state->Exit(hit_state);
	}
}

void Mushdoom::Kill()
{
	light_orbs_manager->SpawnOrbs(owner->transform.GetGlobalTranslation(), 2);
	is_alive = false;

	//Reset mushdoom ranged/melee counter
	CancelAttack();
	//Set enable false the mesh of the mushdoom
	owner->SetEnabled(false);
	is_dying = false;
}

void Mushdoom::ResetEnemy()
{
	EnemyController::ResetEnemy();

	engage_player = false;
	leave_attack_range = false;
	detect_player_distance = 30.f;
	animation->Stop();
	current_state = idle_state;
	current_state->OnStateEnter();
	animation->Play();

	throw_effect->Stop();
	is_spawning = false;
	has_ended_spawn = false;
	owner->SetEnabled(true);
	if (mushdoom_ball) 
	{
		mushdoom_ball->owner->SetEnabled(false);
		mushdoom_ball->owner->transform.SetGlobalMatrixTranslation(float3(0, 0, 35));
	}
	collider->detect_collision = true;
	collider->SetCollisionDetection();
	InitVFX();
}

void Mushdoom::BattleCircleAI()
{
	move_speed = init_move_speed;
	is_moving = true;

	ResetMovement();

	CalculateSparation(separation_distance);

	if (leave_attack_range)
	{
		if (distance <= attack_range + attack_range_offset)
		{
			desired_velocity = target - position;
			desired_velocity *= -1;
			Seek(desired_velocity);
		}
		else
		{
			leave_attack_range = false;
		}

		return;
	}

	CreateBattleCircle();
}

void Mushdoom::PursueAI()
{
	ResetMovement();

	// Move until enemy reaches danger distance && is separated of enemies
	if (distance > danger_distance)
	{
		is_moving = true;

		CalculateSparation(separation_distance);

		if (desired_velocity.Length() < 2)
		{
			desired_velocity = target - position;
		}

		Seek(desired_velocity);
	}
	// After reaching danger distance...
	else
	{
		CalculateSparation(separation_distance - separation_offset);

		if (!desired_velocity.Equals(float3::zero))
		{
			Avoid(desired_velocity);
		}
		else
		{
			is_moving = false;
		}
	}
}

void Mushdoom::CancelAttack()
{
	if(engage_player)
	{
		enemy_manager->CancelMeleeAttack(this);
		engage_player = false;
	}
	else if(ranged_attack)
	{
		enemy_manager->CancelRangedAttack(this);
		ranged_attack = false;
	}
}

void Mushdoom::LeaveAttackRange()
{
	leave_attack_range = true;
}

void Mushdoom::RotateTowardsPlayer() const
{
	float3 desired_direction = current_target->owner->transform.GetGlobalTranslation() - owner->transform.GetGlobalTranslation();
	desired_direction.y = 0;
	Quat desired_rotation = Quat::LookAt(float3::unitZ, desired_direction.Normalized(), float3::unitY, float3::unitY);
	Quat new_rotation = Quat::Slerp(owner->transform.GetGlobalRotation(), desired_rotation, App->time->delta_time * smooth_rotation);
	owner->transform.SetGlobalMatrixRotation(new_rotation);
}

void Mushdoom::SpawnJump() const
{
	if (current_spawning_time >= MAX_SPAWN_TIME)
	{
		is_spawning = false;
		current_spawning_time = 0;
		return;
	}
	else
	{
		float3 next_position = initial_spawn_position.Lerp(final_spawn_position, (float)(current_spawning_time / MAX_SPAWN_TIME));
		owner->transform.SetGlobalMatrixTranslation(next_position);
		collider->UpdateDimensions();
		current_spawning_time += App->time->delta_time;
		if (current_spawning_time > MAX_SPAWN_TIME)
		{
			is_spawning = false;
			current_spawning_time = MAX_SPAWN_TIME;
		}
	}
}

void Mushdoom::SpawnMovement(const float3& velocity) const
{
	float3 velocity_aux = velocity.Normalized();
	velocity_aux.y = 0.0f;
	float3 origin = collider->GetOrigin();
	float3 end = origin + velocity_aux.ScaledToLength(frontal_collision_detection_distance);
	ComponentCollider* front_check = collider->RaycastClosestHit(origin, end).collider;

	float3 position = owner->transform.GetGlobalTranslation() + velocity * App->time->delta_time * 0.001f;

	if (front_check && (front_check->owner->tag == "Respawn" || front_check->owner->tag == "Groundspawn"))
	{
		initial_spawn_position = position;
		float3 front_direction = owner->transform.GetFrontVector().Normalized().ScaledToLength(front_distance);
		if (front_check->owner->tag == "Respawn")
		{
			position.x += front_direction.x;
			position.z += front_direction.z;
			position.y += jump_distance;
		}
		else if (front_check->owner->tag == "Groundspawn")
		{
			position.x = owner->transform.GetGlobalTranslation().x;
			position.z = owner->transform.GetGlobalTranslation().z;
			position.y += jump_distance;
			audio_source->PlayEvent("play_ground");
			animation->ActiveAnimation("ground");
		}

		is_spawning = true;
		final_spawn_position = position;
		current_spawning_time = 0.0f;
	}
	else
	{
		owner->transform.SetGlobalMatrixTranslation(position);
		collider->UpdateDimensions();
	}
}

void Mushdoom::KnockBack()
{
	float3 new_position = owner->transform.GetGlobalTranslation().Lerp(knockback_target_position, animation->GetCurrentClipPercentatge());
	new_position = new_position - owner->transform.GetGlobalTranslation();

	float speed = init_move_speed;
	float3 final_velocity = new_position;

	speed = new_position.Length();

	bool falling = new_position.y < 0.f;

	ComponentCollider* ground_check = collider->DetectCollisionWithGround(1.f).collider;
	bool grounded = ground_check != nullptr && ground_check->owner->tag == "EnvironmentCollider";


	if (falling && grounded)
	{
		final_velocity.y = 0.f;
	}

	float3 position = owner->transform.GetGlobalTranslation() + ((final_velocity.Normalized() + CollisionAvoidance(final_velocity))* speed * App->time->delta_time * 0.001f);

	float3 next_position = float3::zero;
	if (App->artificial_intelligence->FindNextPolyByDirection(position, next_position))
	{
		position.y = next_position.y;
	}

	if (App->artificial_intelligence->IsPointWalkable(position) || grounded)
	{
		owner->transform.SetGlobalMatrixTranslation(position);
	}
}

void Mushdoom::NewThrow()
{
	if (ranged_attack)
	{
		current_time_to_throw += App->time->delta_time;
		is_throwing = false;
		// Throw ball every 'throw_delay' seconds
		if (current_time_to_throw > (throw_delay * 1000))
		{
			is_throwing = true;
			current_time_to_throw = 0;
		}
	}
}

void Mushdoom::CalculateSparation(float separation)
{
	for (auto& enemy : enemy_manager->enemies)
	{

		if (enemy->is_alive && enemy->owner->transform.GetGlobalTranslation().Distance(position) <= separation)
		{
			desired_velocity += enemy->owner->transform.GetGlobalTranslation() - position;
		}
	}
}

void Mushdoom::ResetMovement()
{
	current_target = GetClosestTarget();

	target = current_target->owner->transform.GetGlobalTranslation();
	position = owner->transform.GetGlobalTranslation();
	desired_velocity = float3::zero;

	distance = abs(position.DistanceSq(target));

}

void Mushdoom::HitSFX(NodeID attack)
{
	switch (attack)
	{
		case NodeID::LIGHT_ONE:
			audio_source->PlayEvent("play_receive_hit");
			break;
		case NodeID::LIGHT_TWO:
			audio_source->PlayEvent("play_ground");
			break;
		case NodeID::LIGHT_THREE:
			audio_source->PlayEvent("play_receive_hit");
			break;
		case NodeID::HEAVY_ONE:
			audio_source->PlayEvent("play_receive_hit");
			break;
		case NodeID::HEAVY_TWO:
			audio_source->PlayEvent("play_ground");
			break;
		case NodeID::HEAVY_THREE:
			audio_source->PlayEvent("play_receive_hit");
			break;
		case NodeID::DOWN_SMASH:
			audio_source->PlayEvent("play_receive_hit");
			break;
	}
}

void Mushdoom::CreateBattleCircle()
{
	bool avoid = !desired_velocity.Equals(float3::zero) && distance <= danger_distance;
	bool strafe = !avoid && !engage_player && distance <= strafe_distance;
	bool attack = engage_player && distance <= strafe_distance;

	if (avoid)
	{
		Avoid(desired_velocity);
	}
	else if (strafe)
	{
		float direction = rand() % 1;

		if (direction == 0)
		{
			direction = -1;
		}

		Strafe(target, direction);
	}
	else if (attack)
	{
		if (distance > attack_range)
		{
			desired_velocity = target - position;
			Seek(desired_velocity);
		}
		else
		{
			is_attacking = true;
		}
	}
	else
	{
		desired_velocity = target - position;
		Seek(desired_velocity);
	}
}