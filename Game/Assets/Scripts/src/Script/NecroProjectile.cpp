#include "NecroProjectile.h"

#include "Component/ComponentBillboard.h"
#include "Component/ComponentMeshRenderer.h"
#include "Component/ComponentScript.h"

#include "Main/Application.h"
#include "Main/GameObject.h"
#include "Module/ModuleDebugDraw.h"
#include "Module/ModuleInput.h"
#include "Module/ModuleScene.h"
#include "Module/ModuleTime.h"

#include "EditorUI/Panel/InspectorSubpanel/PanelComponent.h"

#include <imgui.h>

#include "EnemyController.h"
#include "NecroExplosion.h"
#include "PlayerController.h"
#include "WorldManager.h"

NecroProjectile* NecroProjectileDLL()
{
	NecroProjectile* instance = new NecroProjectile();
	return instance;
}

NecroProjectile::NecroProjectile()
{
	panel = new PanelComponent();
}

// Use this for initialization before Start()
void NecroProjectile::Awake()
{
	projectile_mesh_go = owner->children[0];
	projectile_mesh_go->SetEnabled(false);

	projectile_beam_go = owner->children[1];
	beam_billboard = static_cast<ComponentBillboard*>(projectile_beam_go->GetComponent(Component::ComponentType::BILLBOARD));
	projectile_beam_go->SetEnabled(false);

	GameObject* necroexplosion_go = owner->children[2];
	necro_explosion = static_cast<NecroExplosion*>(necroexplosion_go->GetComponentScript("NecroExplosion")->script);
}

// Use this for initialization
void NecroProjectile::Start()
{
	world_manager = static_cast<WorldManager*>(App->scene->GetGameObjectByName("World Manager")->GetComponentScript("WorldManager")->script);
	player_1_transform = &world_manager->GetPlayer1()->player_go->transform;
	player_1_controller = world_manager->GetPlayer1()->player_controller;

	if (world_manager->multiplayer)
	{
		player_2_transform = &world_manager->GetPlayer2()->player_go->transform;
		player_2_controller = world_manager->GetPlayer2()->player_controller;
	}
}

// Update is called once per frame
void NecroProjectile::Update()
{
	if (world_manager->on_pause)
	{
		return;
	}

	if (!is_alive)
	{
		return;
	}

	if (exploding)
	{
		if (necro_explosion->HasExplode())
		{
			exploding = false;
			is_alive = false;
			return;
		}
		return;
	}
	
	alive_time += App->time->delta_time;

	if (alive_time > projectile_time_duration)
	{
		is_alive = false;
		Hide();
	}

	float3 current_position = owner->transform.GetGlobalTranslation();
	float3 next_position = current_position + App->time->delta_time * 0.001f * speed * projectile_direction;
	owner->transform.SetGlobalMatrixTranslation(next_position);

	rotation_angle += App->time->delta_time * 0.001f * rotation_speed;
	owner->transform.SetGlobalMatrixRotation(Quat::RotateFromTo(float3::unitX, projectile_direction) * Quat::RotateAxisAngle(float3::unitX, rotation_angle));

	float beam_alpha = (math::Sin(beam_alpha_oscillation_speed * App->time->time * 0.001f) * 0.6f + 0.6f) * 0.5f;
	beam_billboard->color.w = beam_alpha;


	HitPlayer(projectile_direction); // TODO: Change this with colliders, as it is right now super unacurate
}

bool NecroProjectile::IsDead() const
{
	return !is_alive;
}

void NecroProjectile::Spawn(const float3& spawn_position, const float3& direction)
{
	owner->transform.SetGlobalMatrixTranslation(spawn_position);
	owner->transform.SetGlobalMatrixRotation(Quat::RotateFromTo(float3::unitX, direction));
	projectile_direction = direction;

	is_alive = true;
	alive_time = 0.f;

	projectile_mesh_go->SetEnabled(true);
	projectile_beam_go->SetEnabled(true);
}

void NecroProjectile::Hide() const
{
	projectile_mesh_go->SetEnabled(false);
	projectile_beam_go->SetEnabled(false);
	necro_explosion->Hide();
}

void NecroProjectile::HitPlayer(const float3& projectile_direction)
{
	bool player_1_hit = owner->transform.GetGlobalTranslation().Distance(player_1_transform->GetGlobalTranslation()) <= radius;
	if (player_1_hit)
	{
		player_1_controller->TakeDamage(projectile_damage, nullptr);
	}

	bool player_2_hit = false;
	if (world_manager->multiplayer)
	{
		player_2_hit = owner->transform.GetGlobalTranslation().Distance(player_2_transform->GetGlobalTranslation()) <= radius;
	}
	if (player_2_hit)
	{
		player_2_controller->TakeDamage(projectile_damage, nullptr);
	}

	if (player_1_hit || player_2_hit)
	{
		Hide();
		necro_explosion->Explode(projectile_direction);
		exploding = true;
	}
}

 void NecroProjectile::Save(Config& config) const
 {
 	config.AddFloat(speed, "Speed");
 	Script::Save(config);
 }

 void NecroProjectile::Load(const Config& config)
 {
	speed = config.GetFloat("Speed", 10.f);
 	Script::Load(config);
 }

 // Use this for showing variables on inspector
 void NecroProjectile::OnInspector(ImGuiContext* context)
 {
	 //Necessary to be able to write with imgui
	 ImGui::SetCurrentContext(context);
	 ShowDraggedObjects();
	 ImGui::DragFloat("Speed", &speed, 0.1f);
 }
