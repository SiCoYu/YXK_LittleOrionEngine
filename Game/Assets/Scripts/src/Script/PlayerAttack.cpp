#include "PlayerAttack.h"

#include "Component/Component.h"
#include "Component/ComponentAnimation.h"
#include "Component/ComponentAudioSource.h"
#include "Component/ComponentBillboard.h"
#include "Component/ComponentCollider.h"
#include "Component/ComponentParticleSystem.h"
#include "Component/ComponentScript.h"
#include "Component/ComponentTransform.h"
#include "Component/ComponentTrail.h"

#include "Log/EngineLog.h"
#include "Main/Application.h"
#include "Main/GameObject.h"
#include "Module/ModuleDebugDraw.h"
#include "Module/ModuleInput.h"
#include "Module/ModulePhysics.h"
#include "Module/ModuleScene.h"
#include "Module/ModuleTime.h"

#include "imgui.h"

#include "BossController.h"
#include "CameraShake.h"
#include "ComboCounter.h"
#include "DebugModeScript.h"
#include "EnemyController.h"
#include "InputManager.h"
#include "PlayerMovement.h"
#include "UIManager.h"
#include "WorldManager.h"

PlayerAttack* PlayerAttackDLL()
{
	PlayerAttack* instance = new PlayerAttack();

	return instance;
}

PlayerAttack::PlayerAttack()
{

}

// Use this for initialization before Start()
void PlayerAttack::Awake()
{
	GameObject* enemy_manager_go = App->scene->GetGameObjectByName("EnemyManager");
	enemy_manager = static_cast<EnemyManager*>(enemy_manager_go->GetComponentScript("EnemyManager")->script);

	animation = (ComponentAnimation*) owner->GetComponent(Component::ComponentType::ANIMATION);
	audio_source = static_cast<ComponentAudioSource*>(owner->GetComponent(Component::ComponentType::AUDIO_SOURCE));

	GameObject* debug_go = App->scene->GetGameObjectByName("DebugSystem");
	debug = static_cast<DebugModeScript*>(debug_go->GetComponentScript("DebugModeScript")->script);

	GameObject* input_go = App->scene->GetGameObjectByName("Input Manager");
	input_manager = static_cast<InputManager*>(input_go->GetComponentScript("InputManager")->script);

	GameObject* main_camera_go = App->scene->GetGameObjectByName("Main Camera");
	camera_shake = static_cast<CameraShake*>(main_camera_go->GetComponentScript("CameraShake")->script);

	GameObject* world_go = App->scene->GetGameObjectByName("World Manager");
	world_manager = static_cast<WorldManager*>(world_go->GetComponentScript("WorldManager")->script);

	GameObject* ui_manager_go = App->scene->GetGameObjectByName("UIManager");
	ui_manager = static_cast<UIManager*>(ui_manager_go->GetComponentScript("UIManager")->script);

	GameObject* combo_counter_go = App->scene->GetGameObjectByName("Combo Counter Indicator");
	ComponentScript* combo_counter_component = combo_counter_go->GetComponentScript("ComboCounter");
	combo_counter = static_cast<ComboCounter*>(combo_counter_component->script);

	InitVFX();
}

// Use this for initialization
void PlayerAttack::Start()
{
	player_movement = static_cast<PlayerMovement*>(owner->GetComponentScript("PlayerMovement")->script);
	
	if (world_manager->ThereIsBoss()) 
	{
		is_boss = true;
		boss = world_manager->GetBoss();
	}

	GetAttackList();
	InitAttackList();
	current_attack = attack_list[0];

}

// Update is called once per frame
bool PlayerAttack::Attack(uint32_t player)
{
	ComputeRangeVectors();

	this->player = player;
	anim_percentatge = animation->GetCurrentClipPercentatge();

	UpdateAttackInputs();
	CheckAttackInputs(player);

	RotateToAttackDirection();

	current_id = current_attack->id;

	if(input_queue.size() > 0)
	{
		 MakeAttack();
	}

	same_attack = current_id == current_attack->id;
	is_attacking = current_attack->id != NodeID::NO_COMBO;
	animation->SetBool("is_attacking", is_attacking);
	percentatge = anim_percentatge > current_attack->percentatge_of_collision;
	on_state =  animation->IsOnState(current_attack->animation_state);

	if(same_attack && is_attacking && attack_activated && percentatge && on_state)
	{
		//We compute collisions for dealing damage
		ComputeCollisions();
		attack_activated = false;
		//Add attack order for debugging
	}

	if(trigger_audio && anim_percentatge >= current_attack->percentatge_of_sfx && on_state && same_attack && is_attacking)
	{
		audio_source->PlayEvent(current_attack->sfx_attack);
		trigger_audio = false;
	}

	if(is_attacking && anim_percentatge >= back_to_idle_percentatge)
	{
		//If animation is almost finished then we reset to no combat node

		//Reset combo to no combo and clear input queue
		input_queue.clear();
		DisableTrails();
		current_attack = attack_list[0];

		accumulated_distance = 0.f;
		previous_position = owner->transform.GetGlobalTranslation();
		is_attacking = false;
		animation->SetBool("is_attacking", is_attacking);
		animation->SetBool("light_attack", false);
		animation->SetBool("heavy_attack", false);
		attack_activated = false;
		current_id = NodeID::NO_COMBO;
		attack_direction = float3::zero;
	}
	else if(is_attacking && anim_percentatge <= back_to_idle_percentatge)
	{
		if(!current_attack->is_aerial && current_attack->speed > 0.f && animation->IsOnState(current_attack->animation_state))
		{

			float3 reverse_direction = owner->transform.GetGlobalTranslation() - previous_position;
			previous_position = owner->transform.GetGlobalTranslation();

			accumulated_distance += reverse_direction.Length();

			if(accumulated_distance <= current_attack->distance && !EnemyInFront())
			{
				player_movement->MoveIntoDirection(owner->transform.GetFrontVector(), current_attack->speed);
			}
		}
		else if(current_attack->is_aerial && !player_movement->IsGrounded()
			&& on_state && same_attack && anim_percentatge > down_tail_smash_anim_percentatge)
		{
			player_movement->MoveIntoDirection(owner->transform.GetUpVector(), current_attack->speed);
		}
	}

	return is_attacking;
}

void PlayerAttack::ComputeCollisions()
{
	for (auto& enemy : enemy_manager->enemies)
	{
		if (!enemy->is_alive || enemy->animation->IsOnState("Die") || animation->IsOnState("Hit"))
		{
			continue;
		}

		if (EnemyOnRange(enemy))
		{
			enemy->HitVFX(current_attack->id); 
			enemy->HitSFX(current_attack->id);
			Hit(enemy);
		}
	}

	if(is_boss)
	{
		float3 current_pos = owner->transform.GetGlobalTranslation();
		float3 position_hitted = float3::zero;
		if (boss->boss_controller->HitByPlayer(current_pos,
			owner->transform.GetFrontVector(), 
			current_attack->attack_range_distance, position_hitted))
		{
			current_damage_power = current_attack->damage;
			if (debug->is_player_god_mode)
			{
				current_damage_power *= debug_damage;
			}
			boss->boss_controller->TakeDamage(current_damage_power, current_pos, position_hitted);
		}
	}
}

void PlayerAttack::Hit(EnemyController* enemy)
{
	current_damage_power = current_attack->damage;
	camera_shake->trauma = ((current_damage_power / enemy->GetTotalHealth()) * trauma_multiplier);
	enemy->knockback_direction = (enemy->owner->transform.GetGlobalTranslation() - owner->transform.GetGlobalTranslation());

	if (debug->is_player_god_mode)
	{
		current_damage_power *= debug_damage;
	}

	enemy->TakeDamage(current_damage_power, owner->transform.GetGlobalTranslation(), current_attack->knockback_power);
	combo_counter->AddHit(current_damage_power);
}

// Use this for showing variables on inspector
void PlayerAttack::OnInspector(ImGuiContext* context)
{
	//Necessary to be able to write with imgui
	ImGui::SetCurrentContext(context);
	ShowDraggedObjects();
	ImGui::DragFloat("Smooth Auto Aim", &smooth_autoaim_factor, 0.001f, 0.0f, 0.2f);
	ImGui::DragFloat("Range Auto Aim", &player_autoaim_range, 0.01f, 0.0f, 10.f);

	ImGui::DragFloat("Back to idle percentatge", &back_to_idle_percentatge);
	ImGui::Checkbox("Attack Activated", &attack_activated);
	ImGui::Checkbox("Is Attacking", &is_attacking);
	ImGui::Checkbox("Same Attack", &same_attack);
	ImGui::Checkbox("On State", &on_state);
	ImGui::Checkbox("Percentatge", &percentatge);
	ImGui::Checkbox("Debug Collision Draw", &debug_collision_draw);
	ImGui::DragFloat3("Position:", position.ptr());
	ImGui::DragFloat3("Front:", front.ptr());
	ImGui::DragFloat3("Max Collision:", max_collision.ptr());
	ImGui::DragFloat3("Min Collision:", min_collision.ptr());
	ImGui::DragFloat("Distance: ", &COLLISION_DISTANCE);
	ImGui::DragFloat3("Attack Direction:", attack_direction.ptr());

	ImGui::Separator();

	if(current_attack != nullptr)
	{
		int aux = static_cast<int>(current_attack->id);
		ImGui::DragInt("Current Node ID", &aux);
		ImGui::Text("Percentatge_of_collision: %.3f", current_attack->percentatge_of_collision);
	}

	ImGui::Text("Anim percentatge: %.3f", anim_percentatge);
	if(input_manager != nullptr)
	{
		ImGui::Text("Vertical input: %.3f", input_manager->GetVertical(PlayerID::ONE));
	}

	ImGui::DragFloat("Trauma multiplier", &trauma_multiplier, 0.01f, 0.1f, 1.0f);

	//Debug Input Queue
	ImGui::Separator();
	ImGui::Text("INPUT QUEUE");
	for(const auto& input : input_queue)
	{
		ImGui::Text("Button: %d", input->button);
		ImGui::Text("Time: %.3f", input->time_of_combo);
		ImGui::Separator();
	}
	ImGui::Separator();

	ImGui::Text("ATTACK ORDER");
	for(auto att : attack_order)
	{
		ImGui::Text("%d", att);
	}

}

//Use this for linking JUST GO automatically
void PlayerAttack::InitPublicGameObjects()
{
	//IMPORTANT, public gameobjects, name_gameobjects and go_uuids MUST have same size
	for (int i = 0; i < public_gameobjects.size(); ++i)
	{
		name_gameobjects.push_back(is_object);
		go_uuids.push_back(0);
	}
}

void PlayerAttack::Save(Config& config) const
{
	config.AddFloat(COLLISION_DISTANCE, "COLLISION_DISTANCE");

	config.AddFloat(back_to_idle_percentatge, "back_to_idle_percentatge");

	config.AddFloat(debug_damage, "debug_damage");

	Script::Save(config);
}

void PlayerAttack::Load(const Config & config)
{
	COLLISION_DISTANCE = config.GetFloat("COLLISION_DISTANCE", 4.f);

	back_to_idle_percentatge = config.GetFloat("back_to_idle_percentatge", 0.85f);

	debug_damage = config.GetFloat("debug_damage", 50.f);

	Script::Load(config);
}

void PlayerAttack::UpdateAttackInputs()
{
	float delta = App->time->delta_time;

	if(input_queue.empty())
	{
		return;
	}

	for(auto it = input_queue.begin(); it != input_queue.end(); ++it)
	{
		(*it)->time_of_combo -= delta;
	}

	input_queue.erase(std::remove_if(
		input_queue.begin(), input_queue.end(),
		[](InputCombo* input)
		{
			return input->time_of_combo <= 0.f;
		}
	), input_queue.end());

}

void PlayerAttack::CheckAttackInputs(int player)
{
	float total_anim_time = float(animation->GetTotalAnimationTime());
	float time = total_anim_time - (total_anim_time * anim_percentatge) + TIME_COMBO_OFFSET;

	//If punch is pressed then we add 0 to the InputCombo and compute how many time will be valid
	if(input_manager->GetGameInputDown("Punch", static_cast<PlayerID>(player)) && player_movement->IsGrounded())
	{
		InputCombo* input_combo = new InputCombo(Move::LIGHT, time);
		input_queue.emplace_back(input_combo);
	}

	if(input_manager->GetGameInputDown("Kick", static_cast<PlayerID>(player)))
	{
		if(!player_movement->IsGrounded())
		{
			InputCombo* input_combo = new InputCombo(Move::DOWN_KICK, time);
			input_queue.emplace_back(input_combo);
		}
		else if(player_movement->IsGrounded())
		{
			InputCombo* input_combo = new InputCombo(Move::HEAVY, time);
			input_queue.emplace_back(input_combo);
		}
	}

}

void PlayerAttack::ComputeRangeVectors()
{
	position = owner->transform.GetGlobalTranslation();
	front = position + owner->transform.GetFrontVector();

	float angle = math::Acos(current_attack->attack_range_angle);

	max_collision = math::Quat(owner->transform.GetUpVector(), math::Acos(current_attack->attack_range_angle)) * owner->transform.GetFrontVector();
	min_collision = math::Quat(owner->transform.GetUpVector(), -math::Acos(current_attack->attack_range_angle)) * owner->transform.GetFrontVector();

	max_collision.ScaleToLength(COLLISION_DISTANCE);
	min_collision.ScaleToLength(COLLISION_DISTANCE);

	max_collision += position;
	min_collision += position;

	if (debug_collision_draw)
	{
		App->debug_draw->RenderLine(position, front, color_front);
		App->debug_draw->RenderLine(position, max_collision, color_angle);
		App->debug_draw->RenderLine(position, min_collision, color_angle);
	}
}

void PlayerAttack::GetAttackList()
{
	NoCombo* no_combo = new NoCombo();
	attack_list.emplace_back(no_combo);

	LightOne* light_one = new LightOne();
	attack_list.emplace_back(light_one);

	LightTwo* light_two = new LightTwo();
	attack_list.emplace_back(light_two);

	LightThree* light_three = new LightThree();
	attack_list.emplace_back(light_three);

	HeavyOne* heavy_one = new HeavyOne();
	attack_list.emplace_back(heavy_one);

	HeavyTwo* heavy_two = new HeavyTwo();
	attack_list.emplace_back(heavy_two);

	HeavyThree* heavy_three = new HeavyThree();
	attack_list.emplace_back(heavy_three);

	DownSmash* down_smash = new DownSmash();
	attack_list.emplace_back(down_smash);
}

void PlayerAttack::InitAttackList()
{
	for(auto& attack : attack_list)
	{
		attack->Init();
	}
}

void PlayerAttack::MakeAttack()
{
	bool attack = false;
	InputCombo* input = input_queue[0];

	//When no combo actions have to be done instantly
	if(current_attack->id == NodeID::NO_COMBO)
	{
		//Check if Button matches with some transition and modify current attack (strange)
		if(UpdateAttack(input->button))
		{
			CheckDirection();
			animation->ActiveAnimation(current_attack->animation);
			animation->SetBool("is_attacking", true);
			if (!debug->disable_autoaim)
			{
				AutoAimAttack();
			}
			EmitVFX();
			input_queue.clear();
			trigger_audio = true;
			attack_activated = true;
			accumulated_distance = 0.f;
			previous_position = owner->transform.GetGlobalTranslation();
		}
	}
	//When in middle of a combo we want the action be done at the end of an animation
	else
	{
		//Check if current animation is done and check if transitions of given button is correct
		if (anim_percentatge >= current_attack->min_transition_animation_time && anim_percentatge <= current_attack->max_transition_animation_time  && UpdateAttack(input->button))
		{
			CheckDirection();

			if (!debug->disable_autoaim)
			{
				AutoAimAttack();
			}
			animation->ActiveAnimation(current_attack->animation);
			animation->SetBool("is_attacking", true);
			EmitVFX();
			input_queue.clear();
			trigger_audio = true;
			attack_activated = true;
			accumulated_distance = 0.f;
			previous_position = owner->transform.GetGlobalTranslation();

		}
	}

	return;
}

bool PlayerAttack::UpdateAttack(Move input_to_find)
{
	bool attack = false;
	std::map<Move, NodeID>::iterator next_transition;
	next_transition = current_attack->transitions.find(input_to_find);
	if(next_transition != current_attack->transitions.end())
	{
		auto it = std::find_if(attack_list.begin(), attack_list.end(), [&next_transition](Node* node)
		{
			return node->id == next_transition->second;
		});
		if (it != attack_list.end())
		{
			SetBooleanPower(input_to_find);
			DisableTrails();
			current_attack = *it;
			attack = true;
		}
	}

	return attack;
}

bool PlayerAttack::EnemyOnRange(EnemyController* enemy) const
{
	//Position is our local (0,0) so we can work with direction and forward_vector as locals
	float3 position_with_offset = position - owner->transform.GetFrontVector();
	float3 direction = enemy->owner->transform.GetGlobalTranslation() - position_with_offset;

	if(direction.Length() > current_attack->attack_range_distance)
	{
		return false;
	}
	//For downtail smash
	else if(current_attack->is_aerial)
	{
		return true;
	}

	direction.Normalize();
	float3 forward_vector = owner->transform.GetFrontVector();

	if(forward_vector.Dot(direction) > current_attack->attack_range_angle)
	{
		return true;
	}

	return false;
}

bool PlayerAttack::EnemyInFront() const
{
	for (auto& enemy : enemy_manager->enemies)
	{
		if (!enemy->is_alive || enemy->animation->IsOnState("Die") || animation->IsOnState("Hit"))
		{
			continue;
		}

		if (EnemyOnRange(enemy))
		{
			return true;
		}
	}
	return false;
}

void PlayerAttack::InitVFX()
{
	//Here you collect all vfx of the attacks and push it on the correct order as Attacks.h nodes have in their indexes

	for(auto ch : owner->children)
	{
		if (ch->tag == "VFX")
		{
			for (auto effect : ch->children)
			{
				if (effect->tag == "AttackVFX")
				{
					for(auto effect_component : effect->components)
					{
						if(effect_component->type == Component::ComponentType::PARTICLE_SYSTEM)
						{
							ComponentParticleSystem* vfx_part = static_cast<ComponentParticleSystem*>(effect_component);
							vfx_attacks_container.push_back(vfx_part);
							vfx_part->Stop();
							vfx_part->Disable();
						}

						if(effect_component->type == Component::ComponentType::BILLBOARD)
						{
							ComponentBillboard* vfx_bill = static_cast<ComponentBillboard*>(effect_component);
							vfx_attacks_container.push_back(vfx_bill);
							vfx_bill->loop = false;
							vfx_bill->Disable();
						}

						if(effect_component->type == Component::ComponentType::TRAIL)
						{
							ComponentTrail* vfx_trail = static_cast<ComponentTrail*>(effect_component);
							vfx_attacks_container.push_back(vfx_trail);
							vfx_trail->duration = 0.0F;
							vfx_trail->Disable();
						}
					}
				}
			}
		}
	}
}

void PlayerAttack::EmitVFX()
{
	size_t emit_index = 0;

	for(size_t i = 0; i < current_attack->vfx_attack_indexes.size(); ++i)
	{
		if(current_attack->vfx_attack_indexes[i] < vfx_attacks_container.size() && current_attack->vfx_attack_indexes[i] >= 0)
		{
			Component* component_vfx = vfx_attacks_container[current_attack->vfx_attack_indexes[i]];
			if(component_vfx->type == Component::ComponentType::PARTICLE_SYSTEM)
			{
				size_t num_of_emit = current_attack->count_of_emit_vfx[emit_index];
				++emit_index;
				static_cast<ComponentParticleSystem*>(component_vfx)->Enable();
				static_cast<ComponentParticleSystem*>(component_vfx)->Emit(num_of_emit);
			}
			else if(component_vfx->type == Component::ComponentType::BILLBOARD)
			{
				if (!static_cast<ComponentBillboard*>(component_vfx)->loop)
				{
					static_cast<ComponentBillboard*>(component_vfx)->Enable();
					static_cast<ComponentBillboard*>(component_vfx)->Play();
				}
				else if(static_cast<ComponentBillboard*>(component_vfx)->loop)
				{
					static_cast<ComponentBillboard*>(component_vfx)->Enable();
					static_cast<ComponentBillboard*>(component_vfx)->Play();
				}
			}
			else if(component_vfx->type == Component::ComponentType::TRAIL)
			{
				static_cast<ComponentTrail*>(component_vfx)->Enable();
				static_cast<ComponentTrail*>(component_vfx)->duration = 200.0F;
			}

			
		}
	}

}

void PlayerAttack::DisableVFX()
{
	for (auto & component : vfx_attacks_container)
	{
		component->Disable();
	}
}

void PlayerAttack::DisableTrails()
{
	for (int i = 0; i < current_attack->vfx_attack_indexes.size(); ++i)
	{
		Component* comp = vfx_attacks_container[current_attack->vfx_attack_indexes[i]];
		if (comp->type == Component::ComponentType::TRAIL)
		{
			static_cast<ComponentTrail*>(comp)->duration = 0.0F;

		}
	}
}

bool PlayerAttack::GetCloserEnemyPosition(float3& closer_position)
{
	closer_position = float3::zero;
	float distance = player_autoaim_range;
	bool needs_to_rotate = false;
	for (auto& enemy : enemy_manager->enemies)
	{
		if (!enemy->is_alive || enemy->animation->IsOnState("Die"))
		{
			continue;
		}

		if (EnemyOnRange(enemy))
		{
			float3 current_enemy_position = enemy->owner->transform.GetGlobalTranslation();
			float current_enemy_distance = owner->transform.GetGlobalTranslation().Distance(current_enemy_position);
			if(current_enemy_distance < distance)
			{
				distance = current_enemy_distance;
				closer_position = current_enemy_position;
				needs_to_rotate = true;
			}
		}
	}
	return needs_to_rotate;
}

void PlayerAttack::AutoAimAttack()
{
	float3 closer_position = float3::zero;
	bool needs_autoaim = GetCloserEnemyPosition(closer_position);

	if(needs_autoaim)
	{
		float3 direction_to_enemy = closer_position - owner->transform.GetGlobalTranslation();
		player_movement->RotatePlayer(direction_to_enemy, smooth_autoaim_factor);
	}
}

void PlayerAttack::CheckDirection()
{
	attack_direction = float3::zero;
	player_movement->GetInputDirection(player, attack_direction);
}

void PlayerAttack::RotateToAttackDirection()
{
	if (attack_direction.xz().Length() != 0)
	{
		player_movement->RotatePlayer(attack_direction);
	}
}

void PlayerAttack::SetBooleanPower(const Move input)
{
	switch (input)
	{
		case Move::LIGHT:
			animation->SetBool("light_attack", true);
			animation->SetBool("heavy_attack", false);
			break;
		case Move::HEAVY:
			animation->SetBool("light_attack", false);
			animation->SetBool("heavy_attack", true);
			break;
		default:
			animation->SetBool("light_attack", false);
			animation->SetBool("heavy_attack", false);
			break;
	}
}
