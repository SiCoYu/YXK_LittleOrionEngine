#include "PlayerController.h"

#include "Component/ComponentAnimation.h"
#include "Component/ComponentAudioSource.h"
#include "Component/ComponentBillboard.h"
#include "Component/ComponentCollider.h"
#include "Component/ComponentMeshRenderer.h"
#include "Component/ComponentParticleSystem.h"
#include "Component/ComponentScript.h"
#include "Component/ComponentTransform.h"
#include "Component/ComponentTrail.h"

#include "Log/EngineLog.h"
#include "Main/Application.h"
#include "Main/GameObject.h"
#include "Module/ModuleInput.h"
#include "Module/ModuleScene.h"
#include "Module/ModuleTime.h"

#include "imgui.h"

#include "BossController.h"
#include "DebugModeScript.h"
#include "EnemyController.h"
#include "InputManager.h"
#include "HitBlinker.h"
#include "LightShield.h"
#include "LightTeleportManager.h"
#include "PlayerAttack.h"
#include "PlayerMovement.h"
#include "ProgressBar.h"
#include "UIManager.h"
#include "WorldManager.h"

PlayerController* PlayerControllerDLL()
{
	PlayerController* instance = new PlayerController();
	return instance;
}

PlayerController::PlayerController()
{
	
}

// Use this for initialization before Start()
void PlayerController::Awake()
{
	const ComponentScript* component = owner->GetComponentScript("PlayerMovement");
	player_movement = static_cast<PlayerMovement*>(component->script);

	const ComponentScript* component_attack = owner->GetComponentScript("PlayerAttack");
	player_attack = static_cast<PlayerAttack*>(component_attack->script);

	GameObject* ui = App->scene->GetGameObjectByName("UIManager");
	ComponentScript* component_ui = ui->GetComponentScript("UIManager");
	ui_manager = static_cast<UIManager*>(component_ui->script);

	animation = static_cast<ComponentAnimation*>(owner->GetComponent(Component::ComponentType::ANIMATION));
	audio_source = static_cast<ComponentAudioSource*>(owner->GetComponent(Component::ComponentType::AUDIO_SOURCE));
	collider = static_cast<ComponentCollider*>(owner->GetComponent(Component::ComponentType::COLLIDER));

	GameObject* debug_system = App->scene->GetGameObjectByName("DebugSystem");
	const ComponentScript* component_debug = debug_system->GetComponentScript("DebugModeScript");
	debug = (DebugModeScript*)component_debug->script;

	GameObject* world_go = App->scene->GetGameObjectByName("World Manager");
	const ComponentScript* world_component = world_go->GetComponentScript("WorldManager");
	world = (WorldManager*)world_component->script;

	GameObject* input_go = App->scene->GetGameObjectByName("Input Manager");
	const ComponentScript* input_component = input_go->GetComponentScript("InputManager");
	input_manager = (InputManager*)input_component->script;

	revive_bar_go = App->scene->GetGameObjectByName("Revive Bar Border");
	GameObject* revive_go = App->scene->GetGameObjectByName("Revive Bar");
	const ComponentScript* revive_component = revive_go->GetComponentScript("ProgressBar");
	revive_bar = (ProgressBar*)revive_component->script;

	ComponentScript* component_shield = owner->GetComponentScript("LightShield");
	light_shield = static_cast<LightShield*>(component_shield->script);

	hit_blinker = static_cast<HitBlinker*>(owner->children[0]->GetComponentScript("HitBlinker")->script);

	InitVFX();

}

// Use this for initialization
void PlayerController::Start()
{
	animation->SetBool("is_alive", true);
	animation->SetBool("is_dashing", false);
	revive_bar_go->SetEnabled(false);
	dead_indicator->SetEnabled(false);
	dead_ball_trail_VFX->duration = 0.0F;
	health_points = total_health;
	if(player == WhichPlayer::PLAYER1)
	{
		other_player = world->GetPlayer2();
		other_player_id = 2;
		ui_manager->SetPlayer1Health(health_points / total_health);
	}
	else
	{
		other_player = world->GetPlayer1();
		ui_manager->SetPlayer2Health(health_points / total_health);
	}
	splash_musdhoom_range_VFX->Stop();
	small_texture_DB_VFX->Disable();
	small_texture_DB_VFX->loop = true;
	big_texture_DB_VFX->loop = true;
	smoke_texture_DB_VFX->loop = true;
	revive_spiral_VFX->loop = false;
	sphere_mesh_render->active = false;
	revive_VFX->Stop();
	heal_VFX->Stop();
	sparkle_dead_ball_VFX->Stop();
	dash_trail_VFX->duration = 0.0;

	if (world->ThereIsBoss())
	{
		is_boss = true;
		boss = world->GetBoss();
	}
}

// Update is called once per frame
void PlayerController::Update()
{
	if (world->on_pause)
	{
		return;
	}

	switch(current_state)
	{
		case PlayerState::ALIVE:
			if(!is_reviving)
			{
				CheckKnockupStatus();
				CheckStunStatus();
				if (!is_stunned && !is_knocked_up && CheckDashStatus() && !is_dashing && 
					((input_manager->GetKeyDown(KeyCode::LeftShift) && player == WhichPlayer::PLAYER1) ||
					(input_manager->GetGameInputDown("Dash", static_cast<PlayerID>(player)))))
				{
					Dash();
				}

				CheckLightShieldStatus();

				if(CheckDashBehaviour())
				{
					return;
				}
				
				CheckAliveVFX();
				CheckRespawning();
			}

			break;

		case PlayerState::DYING:
			CheckDyingStatus();
			break;

		case PlayerState::DEAD:
			CheckDeadBehaviour();
			break;

		case PlayerState::REVIVING:
			CheckRevivingBehaviour();
			break;
	}
}

// Use this for showing variables on inspector
void PlayerController::OnInspector(ImGuiContext* context)
{
	//Necessary to be able to write with imgui
	ImGui::SetCurrentContext(context);
	ImGui::Text("Player Controller Script Inspector");
	ShowDraggedObjects();
	std::string selected = std::to_string(static_cast<unsigned int>(player));
	if (ImGui::BeginCombo("Player", selected.c_str()))
	{
		
		if (ImGui::Selectable("1"))
		{
			player = WhichPlayer::PLAYER1;
		}
		if (ImGui::Selectable("2"))
		{
			player = WhichPlayer::PLAYER1;
		}

		ImGui::EndCombo();
	}
	ImGui::Separator();
	ImGui::Text("Status");
	ImGui::DragFloat("Health", &health_points);
	ImGui::DragFloat("Total Health", &total_health);

	ImGui::Spacing();

	ImGui::DragFloat("Heal Time", &heal_time);

	ImGui::Spacing();

	ImGui::DragFloat("Special Points", &special_points);
	ImGui::DragFloat("Total Special Points", &total_special_points);

	ImGui::Checkbox("Is Alive", &is_alive);
	ImGui::Checkbox("Is Reviving", &is_reviving);
	ImGui::Checkbox("Is Dashing", &is_dashing);

	ImGui::Separator();

	ImGui::DragFloat("Current Time", &current_time);
	ImGui::DragInt("Counter", &counter);
	ImGui::DragFloat("Delta Time", &App->time->delta_time);
	ImGui::DragFloat("Current Distance", &current_distance);
	ImGui::DragFloat("Slow Distance", &slow_distance);

	ImGui::Checkbox("Can move", &can_move);

	ImGui::Separator();
	ImGui::Text("Dash");
	ImGui::DragFloat("Dash Speed", &dash_speed); 
	ImGui::DragFloat("Dash Slide", &dash_slide);
	ImGui::DragFloat("Dash Cooldown", &dash_time);

	ImGui::Text("Knockup");
	ImGui::Checkbox("IsKnockedUp", &is_knocked_up);
	ImGui::DragFloat("Current time knocked", &current_time_knocked);
	ImGui::DragFloat("Knockback Speed", &knockback_speed);
	ImGui::DragFloat("Knockup Speed", &knockup_speed);
	ImGui::Checkbox("Spawning invulnerability", &spawning);
	ImGui::DragFloat3("Knockback Direction", knockback_direction.ptr());
}

//Use this for linking GO automatically
void PlayerController::Save(Config& config) const
{
	config.AddUInt(static_cast<unsigned int>(player), "Player");
	Script::Save(config);
}

//Use this for linking GO automatically
void PlayerController::Load(const Config& config)
{
	player = static_cast<WhichPlayer>(config.GetUInt("Player", 0));
	Script::Load(config);
}

void PlayerController::TakeDamage(const float damage, ComponentBillboard* vfx)
{ 
	TakeDamage(damage);

	if (light_shield->IsActive())
	{
		return;
	}

	if (vfx != nullptr)
	{
		vfx->loop = false;
		vfx->Play();
	}
}

void PlayerController::TakeDamage(const float damage)
{
	if (debug->is_player_invincible || debug->is_player_god_mode || is_dashing || spawning)
	{
		return;
	}

	if (light_shield->IsActive())
	{
		float broken_shield = light_shield->BlockAttack(damage);
		health_points -= broken_shield;
		if(broken_shield > 0.f)
		{
			hit_blinker->Blink();
		}
	}
	else
	{
		health_points -= damage;
		hit_blinker->Blink();
	}

	if (health_points <= 0)
	{
		health_points = 0;
		ResetStun();
		ResetKnockup();
		current_state = PlayerState::DYING;
		animation->SetBool("is_alive", false);
	}
	UpdateHealthBar();
}

void PlayerController::Heal(const float healed_value)
{
	health_points = math::Min(health_points + healed_value, total_health);
	UpdateHealthBar();
}

void PlayerController::ResetStun()
{
	//Reset stun when dead
	current_time_stunned = 0.f;
	is_stunned = false;
	animation->SetBool("stunned", is_stunned);
	stun_VFX->Disable();
	player_movement->SlowPlayer(0.f);
}

void PlayerController::ResetKnockup()
{
	is_knocked_up = false;
	player_movement->ResetKnockup();
	animation->SetBool("knocked", is_knocked_up);

}


ComponentCollider* PlayerController::GetCollider()
{
	return static_cast<ComponentCollider*>(owner->GetComponent(Component::ComponentType::COLLIDER));
}

void PlayerController::Die()
{
	//Here make all related to death after die animation
	is_alive = false;
	owner->children[0]->SetEnabled(false);
	collider->active_physics = false;
	current_state = PlayerState::DEAD;
	dead_indicator->SetEnabled(true);
	dead_ball_trail_VFX->duration = 1000.0F;
	sparkle_dead_ball_VFX->Play();
	
	button_showed = false;
}

float PlayerController::CheckDistance() const
{ 
	float3 distance = other_player->player_go->transform.GetGlobalTranslation() - owner->transform.GetGlobalTranslation();
	return distance.Length();
}

void PlayerController::Revive()
{
	revive_spiral_VFX->Play();
	is_alive = true;
	owner->transform.SetGlobalMatrixTranslation(owner->transform.GetGlobalTranslation() + float3::unitY);
	owner->children[0]->SetEnabled(true);
	current_state = PlayerState::ALIVE;
	dead_indicator->SetEnabled(false);
	animation->SetBool("is_alive", true);
	health_points = total_health/4;
	revive_bar_go->SetEnabled(false);
	player_attack->DisableVFX();
	ui_manager->HideReviveButton();
	other_player->player_controller->health_points *= 0.75f;

	if (player == WhichPlayer::PLAYER1)
	{
		ui_manager->SetPlayer1Health(health_points / total_health);
		ui_manager->SetPlayer2Health(other_player->player_controller->health_points / total_health);
	}
	else
	{
		ui_manager->SetPlayer2Health(health_points / total_health);
		ui_manager->SetPlayer1Health(other_player->player_controller->health_points / total_health);
	}

}

void PlayerController::UpdateHealthBar()
{
	if (player == WhichPlayer::PLAYER1)
	{
		ui_manager->SetPlayer1Health(health_points / total_health);
	}
	else
	{
		ui_manager->SetPlayer2Health(health_points / total_health);
	}
}

bool PlayerController::CheckDashStatus()
{
	if(current_dash_cooldown >= App->time->delta_time)
	{
		current_dash_cooldown -= App->time->delta_time;
		return false;
	}
	return true;
}

void PlayerController::Dash()
{
	current_dash_time = dash_time;
	player_movement->GetInputDirection(static_cast<uint32_t>(player), dash_direction);
	if (dash_direction.xz().Length() == 0) 
	{
		dash_direction = owner->transform.GetFrontVector();
	}
	player_movement->MoveIntoDirection(dash_direction, dash_speed);
	player_movement->RotatePlayer(dash_direction, 0.02f);
	current_dash_time -= App->time->delta_time;
	player_movement->ChangeDashStatus(true);
	is_dashing = true;
	animation->SetBool("is_dashing", is_dashing);
	audio_source->PlayEvent("play_dash_player");
}

void PlayerController::Stun(float time_stunned)
{
	if(debug->is_player_invincible || debug->is_player_god_mode || IsShieldActive() || light_shield->on_tutorial)
	{
		return;
	}

	is_stunned = true;
	animation->SetBool("stunned", is_stunned);
	current_time_stunned = 0.f;
	total_time_stunned = time_stunned;
	stun_VFX->Play();
}

void PlayerController::Knockup(float time_knocked, const float3& direction)
{
	if(is_alive && !IsShieldActive() && !is_dashing)
	{
		is_knocked_up = true;
		player_movement->is_knocked = true;
		animation->SetBool("knocked", is_knocked_up);
		knockback_direction = float3(direction.x, 0.f, direction.z);
		player_movement->MoveIntoDirection(owner->transform.GetUpVector(), knockup_speed);	
	}
}

void PlayerController::Respawn()
{
	revive_spiral_VFX->Play();
	is_alive = true;
	owner->transform.SetGlobalMatrixTranslation(owner->transform.GetGlobalTranslation() + float3::unitY);
	owner->children[0]->SetEnabled(true);
	player_attack->DisableVFX();
	current_state = PlayerState::ALIVE;
	dead_indicator->SetEnabled(false);
	animation->SetBool("is_alive", true);
	health_points = total_health;
	revive_bar_go->SetEnabled(false);
	ui_manager->HideReviveButton();

	UpdateHealthBar();

	//Make player invulnerable
	spawning = true;

	//BOSS
	if(is_boss)
	{
		//Reset boss behaviour until player revives
		boss->boss_controller->PlayersSpawning();
	}

	if(hit_blinker != nullptr)
	{
		hit_blinker->Reset();
	}
}

bool PlayerController::CheckStunStatus()
{
	if (is_stunned)
	{
		current_time_stunned += App->time->delta_time;
		if (current_time_stunned >= total_time_stunned)
		{
			ResetStun();
		}

		return true;
	}

	return false;
}

bool PlayerController::CheckKnockupStatus()
{
	if(is_knocked_up)
	{
		player_movement->MoveIntoDirection(knockback_direction, knockback_speed);

		if(animation->IsOnState("Knocked") && animation->GetCurrentClipPercentatge() >= 0.95f)
		{
			is_knocked_up = false;
			player_movement->is_knocked = false;

			animation->SetBool("knocked", is_knocked_up);
		}

		return true;
	}

	return false;
}

void PlayerController::CheckLightShieldStatus() const
{
	if (is_alive && !is_reviving && !is_stunned && !is_knocked_up && !is_dashing && input_manager->GetGameInputDown("Channel Force", static_cast<PlayerID>(player)) && !light_shield->IsActive())
	{
		light_shield->CastShield();
	}
	else if (!is_dashing && input_manager->GetGameInputUp("Channel Force", static_cast<PlayerID>(player)))
	{
		light_shield->UncastShield();
	}
}

void PlayerController::MoveAndAttack()
{
	can_move = !is_stunned && !is_knocked_up;

	if(!can_move)
	{
		animation->SetFloat("horizontal_velocity", 0.0f);
	}
	
	dash_trail_VFX->duration = 0.0F;
	if (!is_attacking)
	{
		player_movement->Move(static_cast<uint32_t>(player), is_stunned || is_knocked_up);
	}
	if (!light_shield->active && can_move)
	{
		is_attacking = player_attack->Attack(static_cast<uint32_t>(player));
	}
	left_trigger = input_manager->GetTrigger(ControllerAxis::LEFT_TRIGGER, static_cast<PlayerID>(player));
}

void PlayerController::CheckAliveVFX()
{
	if (heal_vfx_active)
	{
		current_time_healing += App->time->delta_time;
		if (current_time_healing > heal_time)
		{
			heal_VFX->Stop();
			current_time_healing = 0.0F;
			heal_vfx_active = false;
		}
	}
	sparkle_dead_ball_VFX->Stop();
	revive_VFX->Stop();
}

void PlayerController::CheckRespawning()
{
	if(spawning)
	{
		if(current_time_spawning >= time_spawning)
		{
			spawning = false;
		}
		current_time_spawning += App->time->delta_time;
	}
}

void PlayerController::CheckDyingStatus()
{
	if (animation->GetCurrentClipPercentatge() > 0.99f)
	{
		if (death_timer > death_time_before_death)
		{
			Die();
			death_timer = 0.0f;
			return;
		}

		death_timer += App->time->delta_time;

		if (!player_movement->IsGrounded()) 
		{
			player_movement->MoveIntoDirection(owner->transform.GetUpVector(), -35.f);
		}
	}
}

void PlayerController::CheckDeadBehaviour()
{
	current_distance = CheckDistance();

	sparkle_dead_ball_VFX->Play();

	if (!small_texture_DB_VFX->IsPlaying())
	{
		small_texture_DB_VFX->Play();
		big_texture_DB_VFX->Play();
		smoke_texture_DB_VFX->Play();
	}
	sphere_mesh_render->active = false;
	player_movement->Move(static_cast<uint32_t>(player));
	if (abs(current_distance) <= revive_range)
	{
		if (!button_showed)
		{
			button_showed = true;
		}
		ui_manager->ShowReviveButton(owner->transform.GetGlobalTranslation());

		if (input_manager->GetGameInputDown("Channel Force", static_cast<PlayerID>(other_player_id - 1)))
		{
			//UncastShield if reviving
			other_player->player_controller->light_shield->UncastShield();

			current_state = PlayerState::REVIVING;
			current_time = App->time->delta_time;
			revive_bar->SetProgress(current_time / revive_time);
			revive_bar_go->SetEnabled(true);
			other_player->player_controller->is_reviving = true;
			other_player->player_controller->animation->SetBool("is_reviving", other_player->player_controller->is_reviving);
			audio_source->PlayEvent("play_revive_cast");
		}
	}
	else
	{
		if (button_showed)
		{
			button_showed = false;
			ui_manager->HideReviveButton();
		}
	}
}

void PlayerController::CheckRevivingBehaviour()
{
	current_distance = CheckDistance();
	if (abs(current_distance) <= revive_range &&
		input_manager->GetGameInput("Channel Force", static_cast<PlayerID>(other_player_id - 1)))
	{
		revive_VFX->Play();
		current_time += App->time->delta_time;
		revive_bar->SetProgress(current_time / revive_time);
		if (current_time > revive_time)
		{
			Revive();
			other_player->player_controller->is_reviving = false;
			other_player->player_controller->animation->SetBool("is_reviving", other_player->player_controller->is_reviving);
			audio_source->PlayEvent("play_revive_success");
		}
	}
	else
	{
		revive_VFX->Stop();
		revive_bar_go->SetEnabled(false);
		button_showed = false;
		ui_manager->HideReviveButton();
		current_state = PlayerState::DEAD;
		other_player->player_controller->is_reviving = false;
		other_player->player_controller->animation->SetBool("is_reviving", other_player->player_controller->is_reviving);
		audio_source->StopEvent("play_revive_cast");
	}
}

bool PlayerController::IsShieldActive() const
{
	if(!light_shield)
	{
		return false;
	}
	return light_shield->active;
}

bool PlayerController::CheckDashBehaviour()
{
	if (is_dashing)
	{
		if (current_dash_time >= App->time->delta_time)
		{
			dash_trail_VFX->duration = 500.0f;
			player_movement->MoveIntoDirection(dash_direction, dash_speed);
			player_movement->RotatePlayer(dash_direction);
			current_dash_time -= App->time->delta_time;
		}
		else
		{
			is_dashing = false;
			animation->SetBool("is_dashing", is_dashing);
			current_dash_cooldown = dash_cooldown;
		}

		return true;

	}
	else if (!is_dashing && animation->IsOnState("Dash"))
	{
		player_movement->MoveIntoDirection(owner->transform.GetFrontVector(), dash_slide);
	}
	else if (!is_dashing && !animation->IsOnState("Dash"))
	{
		player_movement->ChangeDashStatus(false);
		MoveAndAttack();
	}

	return false;
}
void PlayerController::InterruptDash() 
{
	is_dashing = false;
	player_movement->ChangeDashStatus(false);
	animation->SetBool("is_dashing", is_dashing);
	current_dash_cooldown = dash_cooldown;
	current_dash_time = 0;
}
void PlayerController::EmitFromImpactPlayerBallMushdoom() const
{
	splash_musdhoom_range_VFX->Emit(6);
}

void PlayerController::EmitHealVFX()
{
	heal_VFX->Play();	
	heal_vfx_active = true;
}

void PlayerController::InitVFX()
{
	//VFX
	for (const auto& go : owner->children)
	{
		if (go->tag == "HitEffect")
		{
			hit_effect = static_cast<ComponentBillboard*>(go->GetComponent(Component::ComponentType::BILLBOARD));
		}
		if (go->tag == "HitEffectBiter")
		{
			hit_effect_biter = static_cast<ComponentBillboard*>(go->GetComponent(Component::ComponentType::BILLBOARD));
		}
		if (go->tag == "DeadIndicator")
		{
			dead_indicator = go;
			for (const auto& sphere : go->children)
			{
				if (sphere->name == "sphere")
				{
					for (const auto& ball : sphere->children)
					{
						if (ball->name == "Sphere")
						{
							sphere_mesh_render = static_cast<ComponentMeshRenderer*>(ball->GetComponent(Component::ComponentType::MESH_RENDERER));
							sphere_mesh_render->Disable();
						}
						if (ball->name == "dead_ball_trail_VFX")
						{
							dead_ball_trail_VFX = static_cast<ComponentTrail*>(ball->GetComponent(Component::ComponentType::TRAIL));
							dead_ball_trail_VFX->Disable();
						}
						if (ball->name == "sparkle_dead_ball_VFX")
						{
							sparkle_dead_ball_VFX = static_cast<ComponentParticleSystem*>(ball->GetComponent(Component::ComponentType::PARTICLE_SYSTEM));
							sparkle_dead_ball_VFX->Stop();
						}
						if (ball->name == "small_texture_DB_VFX")
						{
							small_texture_DB_VFX = static_cast<ComponentBillboard*>(ball->GetComponent(Component::ComponentType::BILLBOARD));
							small_texture_DB_VFX->Disable();
						}
						if (ball->name == "big_texture_DB_VFX")
						{
							big_texture_DB_VFX = static_cast<ComponentBillboard*>(ball->GetComponent(Component::ComponentType::BILLBOARD));
							big_texture_DB_VFX->Disable();
						}
						if (ball->name == "smoke_texture_DB_VFX")
						{
							smoke_texture_DB_VFX = static_cast<ComponentBillboard*>(ball->GetComponent(Component::ComponentType::BILLBOARD));
							smoke_texture_DB_VFX->Disable();
						}
						if (ball->name == "revive_VFX")
						{
							revive_VFX = static_cast<ComponentParticleSystem*>(ball->GetComponent(Component::ComponentType::PARTICLE_SYSTEM));
							revive_VFX->active = false;
						}
					}
				}
			}
		}
		if (go->name == "SplashMusdhoomRangeVFX")
		{
			splash_musdhoom_range_VFX = static_cast<ComponentParticleSystem*>(go->GetComponent(Component::ComponentType::PARTICLE_SYSTEM));
			splash_musdhoom_range_VFX->Stop();
		}
		if (go->name == "HealVFX")
		{
			heal_VFX = static_cast<ComponentParticleSystem*>(go->GetComponent(Component::ComponentType::PARTICLE_SYSTEM));
			heal_VFX->Stop();
		}
		if (go->name == "ReviveSpiralVFX")
		{
			revive_spiral_VFX = static_cast<ComponentBillboard*>(go->GetComponent(Component::ComponentType::BILLBOARD));
			revive_spiral_VFX->Disable();
		}
		if (go->name == "DashTrailVFX")
		{
			dash_trail_VFX = static_cast<ComponentTrail*>(go->GetComponent(Component::ComponentType::TRAIL));
		}
		if(go->name == "StunEffect")
		{
			stun_VFX = static_cast<ComponentBillboard*>(go->GetComponent(Component::ComponentType::BILLBOARD));
			stun_VFX->Disable();
			stun_VFX->loop = true;
		}
	}
}