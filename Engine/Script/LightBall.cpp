#include "LightBall.h"

#include "Component/ComponentAnimation.h"
#include "Component/ComponentAudioSource.h"
#include "Component/ComponentBillboard.h"
#include "Component/ComponentLight.h"
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
#include <math.h>

#include "BossController.h"
#include "ComboCounter.h"
#include "EnemyController.h"
#include "EnemyManager.h"
#include "InputManager.h"
#include "LightShield.h"
#include "PlayerController.h"
#include "PlayerMovement.h"
#include "UIManager.h"
#include "WorldManager.h"

using TCP = LightBall::TutorialCheckPoints::CheckPointStatus;

LightBall* LightBallDLL()
{
	LightBall* instance = new LightBall();
	return instance;
}

LightBall::LightBall()
{

}

// Use this for initialization before Start()
void LightBall::Awake()
{
	world_manager = static_cast<WorldManager*>(App->scene->GetGameObjectByName("World Manager")->GetComponentScript("WorldManager")->script);
	enemy_manager = static_cast<CEnemyManager*>(App->scene->GetGameObjectByName("EnemyManager")->GetComponentScript("EnemyManager")->script);
	input_manager = static_cast<InputManager*>(App->scene->GetGameObjectByName("Input Manager")->GetComponentScript("InputManager")->script);
	
	audio_source = static_cast<ComponentAudioSource*>(owner->GetComponent(Component::ComponentType::AUDIO_SOURCE));

	GameObject* input_go = App->scene->GetGameObjectByName("Input Manager");
	const ComponentScript* input_component = input_go->GetComponentScript("InputManager");
	input_manager = (InputManager*)input_component->script;

	GameObject* ui = App->scene->GetGameObjectByName("UIManager");
	ComponentScript* component_ui = ui->GetComponentScript("UIManager");
	ui_manager = static_cast<UIManager*>(component_ui->script);

	GameObject* combo_counter_go = App->scene->GetGameObjectByName("Combo Counter Indicator");
	ComponentScript* combo_counter_component = combo_counter_go->GetComponentScript("ComboCounter");
	combo_counter = static_cast<ComboCounter*>(combo_counter_component->script);



	SetLightBallVFX();
}

// Use this for initialization
void LightBall::Start()
{
	player_1 = world_manager->GetPlayer1();
	player_2 = world_manager->GetPlayer2();

	player_1_hand = player_1->player_go->children[0]->children[0]->children[0];
	player_2_hand = player_2->player_go->children[0]->children[0]->children[0];

	ball = owner->children[0];

	if (world_manager->ThereIsBoss())
	{
		is_boss = true;
		boss = world_manager->GetBoss();
	}

	DisableMarker();
	SetStartingOwner();
	UpdateMarker();
	ball->SetEnabled(false);

	if (!world_manager->multiplayer)
	{
		distance = singleplayer_distance;
	}
	else
	{
		distance = multiplayer_distance;
	}


}

void LightBall::SetStartingOwner()
{
	if (ally_has_light_ball)
	{
		current_player = App->scene->GetGameObjectByName("Ally");
		current_player_hand = current_player;
		current_player_controller = player_1->player_controller;

		next_target = player_1->player_go;
		next_target_hand = player_1_hand;
		next_target_controller = player_1->player_controller;
		ui_manager->HideLightballUI(world_manager->multiplayer);
	}
	else
	{
		next_target = current_player = player_1->player_go;
		next_target_hand = current_player_hand = player_1_hand;
		next_target_controller = current_player_controller = player_1->player_controller;

		if (world_manager->multiplayer)
		{

			next_target = player_2->player_go;
			next_target_hand = player_2_hand;
			next_target_controller = player_2->player_controller;
		}
		ui_manager->ShowPlayersLightballUI(world_manager->multiplayer);
	}
}

// Update is called once per frame
void LightBall::Update()
{

	if (world_manager->on_pause && !on_tutorial)
	{
		return;
	}

	if (!ally_has_light_ball && input_manager->GetGameInput("LightBall", static_cast<PlayerID>(current_player_controller->player)))
	{
		if (IsLightBallAvailable())
		{
			current_state = LightBallState::ACTIVATED;
			moving = true;
			shield_tutorial.lightball_throw = TCP::SUCCESS;
		}
	}
	CheckPlayerStatus();
	shield_tutorial.parry_enabled = false;
	switch (current_state)
	{
	case LightBallState::UNNACTIVE:
		if (!ally_has_light_ball) 
		{
			UpdateCooldown();
		}
		owner->transform.SetGlobalMatrixTranslation(current_player->transform.GetGlobalTranslation() + float3::unitY
			+ current_player->transform.GetFrontVector() * cast_distance);
		SetActiveVFX(false);
		if (on_tutorial && (shield_tutorial.player1_bounce != TCP::IN_PROGRESS && shield_tutorial.player2_bounce != TCP::IN_PROGRESS))
		{
			on_tutorial = false;
		}
		break;
	case LightBallState::ACTIVATED:
		if (!casted)
		{
			current_player_controller->animation->SetBool("throwingball", true);
			casted = true;
			ball_enabled = false;
			owner->transform.SetScale(cast_scale);
			SetSizeEffects(0.3f);
		}
		if (casted && current_player_controller->animation->IsOnState("ThrowingBall"))
		{
			ui_manager->EmptyLightballUI(world_manager->multiplayer);
			if (!ball_enabled)
			{
				ball->SetEnabled(true);
				ball->GetComponent(Component::ComponentType::MESH_RENDERER)->active = false;
				SetActiveVFX(true);
				SetEffectProperties();
				ball_enabled = true;
				DisableMarker();
			}
			if (current_player_controller->animation->GetCurrentClipPercentatge() <= 0.6f)
			{
				IncreaseSizeEffects(0.1f);
				owner->transform.SetGlobalMatrixTranslation(current_player_hand->transform.GetGlobalTranslation());
			}
			if (current_player_controller->animation->GetCurrentClipPercentatge() >= 0.6f)
			{
				current_state = LightBallState::PROJECTILE;
				throwed = true;
				current_player_controller->animation->SetBool("throwingball", false);
				owner->transform.SetGlobalMatrixTranslation(current_player_hand->transform.GetGlobalTranslation() + float3::unitY
					+ current_player_hand->transform.GetFrontVector() * cast_distance);
				force_position = (current_player->transform.GetGlobalTranslation() + float3::unitY) + current_player->transform.GetFrontVector() * distance;
				frame_counter = 0.f;
				audio_source->PlayEvent("play_lightball");
			}
		}
		else
		{
			frame_counter += App->time->delta_time;
			if (frame_counter > 500.f)
			{
				current_state = LightBallState::UNNACTIVE;
				current_cooldown = cooldown;
				casted = false;
				moving = false;
				frame_counter = 0.f;
				//TODO Delete this when OnCollisionEnter implemented
				enemies_hitted.clear();
				boss_hitted = false;
			}
		}

		break;

	case LightBallState::PROJECTILE:

		BallMovingForward(force_position);

		if (ball_distance_magnitude < 1)
		{
			current_state = LightBallState::TRANSITION;
			current_time = 0.f;
			//Debug
			throwed = false;
			transition = true;
			casted = false;
			//TODO Delete this when OnCollisionEnter implemented
			enemies_hitted.clear();
			boss_hitted = false;
		}
		break;

	case LightBallState::TRANSITION:
		if (!CanBePassed())
		{
			next_target = current_player;
			next_target_controller = current_player_controller;
			next_target_hand = current_player_hand;
			forced_pass = true;
		}
		current_time += App->time->delta_time;
		next_direction = next_target->transform.GetGlobalTranslation() + float3::unitY;
		BallReturning(next_direction);

		bool parry = (world_manager->multiplayer && next_target_controller->is_alive) ? next_target_controller->light_shield->parry_enable : current_player_controller->light_shield->parry_enable;
		shield_tutorial.parry_enabled = true;
		
		if (ball_distance_magnitude < range)
		{
			RestartBall();
			//Debug
			transition = false;
		}
		else if (parry && ball_distance_magnitude < shield_radius_range)
		{
			BallParryBehaviour();
			//Debug
			transition = false;
		}
		break;
	}
	shield_tutorial.player1_is_owner = current_player == player_1->player_go;

}

void LightBall::CheckTutorialBounceStatus(bool success)
{
	if (!world_manager->multiplayer)
	{
		shield_tutorial.player2_bounce = TCP::SUCCESS;
	}
	if (success)
	{
		++shield_tutorial.lightball_bounces;
	}
	shield_tutorial.player1_bounce = !success ? TCP::FAIL : TCP::SUCCESS;
	shield_tutorial.player2_bounce = !success ? TCP::FAIL : TCP::SUCCESS;
}

// Use this for showing variables on inspector
void LightBall::OnInspector(ImGuiContext* context)
{
	//Necessary to be able to write with imgui
	ImGui::SetCurrentContext(context);
	ImGui::DragFloat("Cast Distance", &cast_distance);
	ImGui::DragFloat("Progress", &progress);
	ImGui::DragFloat3("New Scale", new_scale.ptr());
	ImGui::DragFloat3("Cast Scale", cast_scale.ptr());
	ImGui::DragFloat3("Force Position", force_position.ptr());
	ImGui::DragFloat3("Next Position", next_position.ptr());
	ImGui::DragFloat("Distance", &distance);
	ImGui::DragFloat("Single Player Distance", &singleplayer_distance);
	ImGui::DragFloat("Damage", &damage);
	ImGui::DragFloat("Default Damage", &default_damage);
	ImGui::DragFloat("Speed", &speed);
	ImGui::DragFloat3("Speed", current_velocity.ptr());
	ImGui::DragFloat3("Expected", expected_velocity.ptr());
	ImGui::DragFloat3("Steering", steering.ptr());
	ImGui::DragFloat("Range", &range);
	ImGui::DragFloat("Slow Distance", &slow_distance);
	ImGui::DragFloat("Magnitude Distance", &ball_distance_magnitude);
	ImGui::Separator();
	ImGui::Text("Status when Active:");
	ImGui::Checkbox("Casted", &casted);
	ImGui::Checkbox("Throwed", &throwed);
	ImGui::Checkbox("Transition", &transition);
	ImGui::DragFloat("Cooldown", &cooldown);
	ImGui::DragFloat("Current Cooldown", &current_cooldown);
	ImGui::DragFloat("Current Frame", &frame_counter);
	int aux = static_cast<int>(current_state);
	ImGui::DragInt("Current State", &aux);

	ImGui::DragFloat("Maximum time to return", &maximum_time_to_return);

	ShowDraggedObjects();

}

void LightBall::BallMovingForward(const float3& target)
{
	float3 ball_distance = target - owner->transform.GetGlobalTranslation();
	expected_velocity = ball_distance.ScaledToLength(speed);
	steering = expected_velocity - current_velocity;

	current_velocity += steering;

	ball_distance_magnitude = ball_distance.Length();
	float slow_down_factor = math::Clamp01(ball_distance_magnitude / slow_distance);

	current_velocity *= slow_down_factor;

	next_position = owner->transform.GetGlobalTranslation() + current_velocity * App->time->delta_time;
	owner->transform.SetGlobalMatrixTranslation(next_position);

	DetectCollision();
}

void LightBall::BallReturning(const float3& target)
{
	progress = current_time / maximum_time_to_return;
	float3 new_position = owner->transform.GetGlobalTranslation().Lerp(target, progress);
	owner->transform.SetGlobalMatrixTranslation(new_position);
	float3 ball_distance = target - owner->transform.GetGlobalTranslation();
	ball_distance_magnitude = ball_distance.Length();
	DetectCollision();
}

void LightBall::ThrowLightBallToMainPlayer()
{
	current_state = LightBallState::ACTIVATED;
	moving = true;
	next_target = player_1->player_go;
	next_target_controller = player_1->player_controller;
	next_target_hand = player_1->player_go->children[0]->children[0];
	ui_manager->ShowPlayersLightballUI(world_manager->multiplayer);
}

void LightBall::DetectCollision()
{
	for (auto& enemy : enemy_manager->enemies)
	{
		if (!enemy->is_alive)
		{
			continue;
		}

		//TODO Modify this when OnCollisionEnter implemented
		float enemy_distance = owner->transform.GetGlobalTranslation().Distance(enemy->owner->transform.GetGlobalTranslation());
		if (enemy_distance <= hit_range && !AlreadyHitted(enemy))
		{
			enemy->TakeDamage(damage, owner->transform.GetGlobalTranslation());
			enemy->HitLightBallVFX();
			enemies_hitted.push_back(enemy);
			combo_counter->AddHit(damage);
		}
	}

	if (is_boss)
	{
		float3 current_pos = owner->transform.GetGlobalTranslation();
		float3 position_hitted = float3::zero;
		if (boss->boss_controller->HitByBall(current_pos, position_hitted) && !boss_hitted)
		{
			boss->boss_controller->TakeDamage(damage, current_pos, position_hitted);
			boss_hitted = true;
		}
	}
}

void LightBall::CheckPlayerStatus()
{
	if (ally_has_light_ball)
	{
		return;
	}

	if (forced_pass)
	{
		bool both_alive = player_1->player_controller->is_alive && player_2->player_controller->is_alive;
		if (both_alive)
		{
			if (current_player_controller->player == PlayerController::WhichPlayer::PLAYER1)
			{
				next_target = player_2->player_go;
				next_target_controller = player_2->player_controller;
				next_target_hand = player_2_hand;
			}
			else
			{
				next_target = player_1->player_go;
				next_target_controller = player_1->player_controller;
				next_target_hand = player_1_hand;
			}
			distance = multiplayer_distance;
			forced_pass = false;
		}
	}

	if (HasToBePassed())
	{
		next_direction = next_target->transform.GetGlobalTranslation() + float3::unitY;
		current_player = next_target;
		current_player_controller = next_target_controller;
		current_player_hand = next_target_hand;

		SetActiveVFX(true);
		SetEffectProperties();
		SetSizeEffects(0.9f);
		current_state = LightBallState::TRANSITION;
		ball->SetEnabled(true);
		forced_pass = true;
		distance = singleplayer_distance;
	}

}

bool LightBall::HasToBePassed() const
{
	return !current_player_controller->is_alive && world_manager->multiplayer;
}

bool LightBall::CanBePassed() const
{
	return next_target_controller->is_alive;
}

bool LightBall::CanBeActivated() const
{
	return !(current_player_controller->is_attacking || current_player_controller->is_dashing);
}

//TODO Delete this when OnCollisionEnter implemented
bool LightBall::AlreadyHitted(EnemyController* enemy)
{
	return std::find(enemies_hitted.begin(), enemies_hitted.end(), enemy) != enemies_hitted.end();
}

void LightBall::SwapPlayersTarget()
{
	if (ally_has_light_ball)
	{
		next_target = current_player = player_1->player_go;
		next_target_hand = current_player_hand = player_1_hand;
		next_target_controller = current_player_controller = player_1->player_controller;

		if (world_manager->multiplayer)
		{
			next_target =  player_2->player_go;
			next_target_hand =  player_2_hand;
			next_target_controller =  player_2->player_controller;
		}

		UpdateMarker();
		UpdateCooldown();

		ally_has_light_ball = false;
		return;
	}
	if (world_manager->multiplayer)
	{
		std::swap(current_player, next_target);
		std::swap(current_player_controller, next_target_controller);
		std::swap(current_player_hand, next_target_hand);
	}
}

bool LightBall::IsLightBallAvailable() const
{
	return !moving && CanBeActivated() && current_cooldown >= cooldown && current_state == LightBallState::UNNACTIVE && !current_player_controller->light_shield->active;
}

void LightBall::SetReturnAnimation() const
{
	if (current_player_controller->player_movement->IsGrounded())
	{
		current_player_controller->animation->ActiveAnimation("idle");
	}
	else
	{
		current_player_controller->animation->ActiveAnimation("fall");
	}
}

void LightBall::UpdateCooldown()
{
	if (current_cooldown < cooldown)
	{
		current_cooldown += App->time->delta_time;
		ui_manager->FillLightballUI(static_cast<uint32_t>(current_player_controller->player), current_cooldown / cooldown);
	}
	else
	{
		UpdateMarker();
	}

}

void LightBall::UpdateMarker() const
{
	if (ally_has_light_ball)
	{
		DisableMarker();
		return;
	}
	if (current_player_controller->player == PlayerController::WhichPlayer::PLAYER1)
	{
		player_1_indicator->SetEnabled(true);
		
	}
	else
	{
		player_2_indicator->SetEnabled(true);
	}
}

void LightBall::DisableMarker() const
{
	player_1_indicator->SetEnabled(false);
	player_2_indicator->SetEnabled(false);
}

void LightBall::InitPublicGameObjects()
{
	public_gameobjects.push_back(&player_1_indicator);
	variable_names.push_back(GET_VARIABLE_NAME(player_1_indicator));

	public_gameobjects.push_back(&player_2_indicator);
	variable_names.push_back(GET_VARIABLE_NAME(player_2_indicator));

	//IMPORTANT, public gameobjects, name_gameobjects and go_uuids MUST have same size
	for (int i = 0; i < public_gameobjects.size(); ++i)
	{
		name_gameobjects.push_back(is_object);
		go_uuids.push_back(0);
	}
}

void LightBall::ActiveReturn()
{
	current_state = LightBallState::TRANSITION;
	moving = true;
}

void LightBall::SetActiveVFX(bool active) const
{
	light_LB->active = active;
	interior_texture_LB->active = active;
	exterior_texture_LB->active = active;
	small_texture_LB->active = active;
	blue_sparkle->active = active;
	purple_sparkle->active = active;
	if (active)
	{
		trail_light_ball_VFX->duration = 1000.0F;
	}
	else
	{
		trail_light_ball_VFX->duration = 0.0F;
	}
}

void LightBall::SetLightBallVFX()
{
	for (const auto go : owner->children)
	{
		if (go->name == "interior_texture_LB")
		{
			interior_texture_LB = static_cast<ComponentBillboard*>(go->GetComponent(Component::ComponentType::BILLBOARD));
			width_interior_texture_LB = interior_texture_LB->width; height_interior_texture_LB = interior_texture_LB->height;
		}
		if (go->name == "exterior_texture_LB")
		{
			exterior_texture_LB = static_cast<ComponentBillboard*>(go->GetComponent(Component::ComponentType::BILLBOARD));
			width_exterior_texture_LB = exterior_texture_LB->width; height_exterior_texture_LB = exterior_texture_LB->height;
		}
		if (go->name == "small_texture_LB")
		{
			small_texture_LB = static_cast<ComponentBillboard*>(go->GetComponent(Component::ComponentType::BILLBOARD));
			width_small_texture_LB = small_texture_LB->width; height_small_texture_LB = small_texture_LB->height;
		}
		if (go->name == "light_LB")
		{
			light_LB = static_cast<ComponentLight*>(go->GetComponent(Component::ComponentType::LIGHT));
		}
		if (go->name == "blue_sparkle")
		{
			blue_sparkle = static_cast<ComponentParticleSystem*>(go->GetComponent(Component::ComponentType::PARTICLE_SYSTEM));
		}
		if (go->name == "purple_sparkle")
		{
			purple_sparkle = static_cast<ComponentParticleSystem*>(go->GetComponent(Component::ComponentType::PARTICLE_SYSTEM));
		}
		if (go->name == "trail_light_ball_VFX")
		{
			trail_light_ball_VFX = static_cast<ComponentTrail*>(go->GetComponent(Component::ComponentType::TRAIL));
		}
	}
}

void LightBall::SetSizeEffects(const float percentage) const
{
	interior_texture_LB->width = width_interior_texture_LB; interior_texture_LB->height = height_interior_texture_LB;
	interior_texture_LB->width *= percentage; interior_texture_LB->height *= percentage;

	exterior_texture_LB->width = width_exterior_texture_LB; exterior_texture_LB->height = height_exterior_texture_LB;
	exterior_texture_LB->width *= percentage; exterior_texture_LB->height *= percentage;

	small_texture_LB->width = width_small_texture_LB; small_texture_LB->height = height_small_texture_LB;
	small_texture_LB->width *= percentage; small_texture_LB->height *= percentage;
}

void LightBall::SetEffectProperties() const
{
	interior_texture_LB->loop = true;
	interior_texture_LB->Play();

	exterior_texture_LB->loop = true;
	exterior_texture_LB->Play();

	small_texture_LB->loop = true;
	small_texture_LB->Play();

}

void LightBall::IncreaseSizeEffects(float percentage) const
{
	if (small_texture_LB->width < width_small_texture_LB)
	{
		small_texture_LB->width += percentage;
		small_texture_LB->height += percentage;
	}
	if (exterior_texture_LB->width < width_exterior_texture_LB)
	{
		exterior_texture_LB->width += percentage;
		exterior_texture_LB->height += percentage;
	}
	if (interior_texture_LB->width < width_interior_texture_LB)
	{
		interior_texture_LB->width += percentage;
		interior_texture_LB->height += percentage;
	}
}

void LightBall::RestartBall()
{
	CheckTutorialBounceStatus(false);
	SwapPlayersTarget();
	ball->SetEnabled(false);
	moving = false;
	current_velocity = float3::zero;
	current_state = LightBallState::UNNACTIVE;
	current_cooldown = 0;
	speed = default_speed;
	damage = default_damage;
	maximum_time_to_return = default_maximum_time_to_return;

	//TODO Delete this when OnCollisionEnter implemented
	boss_hitted = false;
	enemies_hitted.clear();

	//Reset lightshield parry time
	current_player_controller->light_shield->ResetParryTime();
	next_target_controller->light_shield->ResetParryTime();
}

void LightBall::BallParryBehaviour()
{
	CheckTutorialBounceStatus(true);
	//Shield activated so ball rebounds
	SwapPlayersTarget();
	current_state = LightBallState::PROJECTILE;
	//First we get the current direction and then we normalize it and scale it inverse
	float3 current_ball_direction = ((current_player->transform.GetGlobalTranslation() + float3::unitY) - owner->transform.GetGlobalTranslation());
	force_position = current_ball_direction.Normalized() * -distance;
	//Then add direction to actual position to get target
	force_position += owner->transform.GetGlobalTranslation();
	force_position.y = math::Clamp(force_position.y, current_player->transform.GetGlobalTranslation().y, current_player->transform.GetGlobalTranslation().y + 2.f);
	//We clamp speed to not buggable values
	speed = math::Clamp(speed * 1.2f, 0.5f, 10.f);
	damage = math::Clamp(damage * 1.2f, 0.f, 100.f);
	maximum_time_to_return = math::Clamp(maximum_time_to_return / 1.2f, 500.f, 10000.f);
	current_player_controller->light_shield->ParryLightBall();
	boss_hitted = false;
	enemies_hitted.clear();
}