#include "DebugModeScript.h"

#include "Component/ComponentCamera.h"
#include "Component/ComponentCollider.h"
#include "Component/ComponentScript.h"
#include "Component/ComponentTransform.h"

#include "Main/Application.h"
#include "Main/GameObject.h"
#include "Module/ModuleCamera.h"
#include "Module/ModuleDebug.h"
#include "Module/ModuleDebugDraw.h"
#include "Module/ModuleEditor.h"
#include "Module/ModuleInput.h"
#include "Module/ModulePhysics.h"
#include "Module/ModuleRender.h"
#include "Module/ModuleScene.h"
#include "Module/ModuleSpacePartitioning.h"
#include "Module/ModuleTime.h"

#include "Log/EngineLog.h"

#include "FontAwesome5/IconsFontAwesome5.h"

#include "imgui.h"
#include <imgui_internal.h>
#include <imgui_stdlib.h>
#include <iomanip>
#include <sstream>

#include "AllyInteractionsScript.h"
#include "CameraController.h"
#include "EnemyController.h"
#include "EnemyManager.h"
#include "EventManager.h"
#include "InputManager.h"
#include "PlayerController.h"
#include "PlayerAttack.h"
#include "SceneCamerasController.h"
#include "TeleportPoint.h"
#include "WorldManager.h"

DebugModeScript* DebugModeScriptDLL()
{
	DebugModeScript* instance = new DebugModeScript();
	return instance;
}

DebugModeScript::DebugModeScript()
{
	
}

// Use this for initialization before Start()
void DebugModeScript::Awake()
{
	GameObject* enemy_go = App->scene->GetGameObjectByName("EnemyManager");
	const ComponentScript* component = enemy_go->GetComponentScript("EnemyManager");
	enemy_manager = (CEnemyManager*)component->script;

	GameObject* world_go = App->scene->GetGameObjectByName("World Manager");
	ComponentScript* world_component = world_go->GetComponentScript("WorldManager");
	world = static_cast<WorldManager*>(world_component->script);

	GameObject* event_go = App->scene->GetGameObjectByName("EventManager");
	ComponentScript* event_component = event_go->GetComponentScript("EventManager");
	event_manager = static_cast<CEventManager*>(event_component->script);

	GameObject* camera_go = App->scene->GetGameObjectByName("Camera Holder");
	ComponentScript* camera_component = camera_go->GetComponentScript("CameraController");
	camera_controller = static_cast<CameraController*>(camera_component->script);

	GameObject* input_go = App->scene->GetGameObjectByName("Input Manager");
	const ComponentScript* input_component = input_go->GetComponentScript("InputManager");
	input_manager = (InputManager*)input_component->script;
	GameObject* ally_go = App->scene->GetGameObjectByName("AllyController");
	if (ally_go)
	{
		ally = static_cast<AllyInteractionsScript*>(ally_go->GetComponentScript("AllyInteractionsScript")->script);
	}
}

// Use this for initialization
void DebugModeScript::Start()
{
	player_1 = world->GetPlayer1();
	player_2 = world->GetPlayer2();

	for (size_t i = 0; i < owner->children.size(); ++i)
	{
		teleport_zones.emplace_back(static_cast<TeleportPoint*>(owner->children[i]->GetComponentScript("TeleportPoint")->script));
	}

	collider_player1 = static_cast<ComponentCollider*>(player_1->player_go->GetComponent(Component::ComponentType::COLLIDER));
	collider_player2 = static_cast<ComponentCollider*>(player_2->player_go->GetComponent(Component::ComponentType::COLLIDER));
}

// Update is called once per frame
void DebugModeScript::Update()
{

	if (input_manager->GetDebugInputDown())
	{
		debug_enabled = !debug_enabled;
	}
	if (debug_enabled)
	{
		ShowDebugWindow();
	}

	if (has_teleported_player_recently)
	{
		CheckTeleportStatus();
	}
	if (spawn_enemy)
	{
		SpawnEnemies();
	}
	if (show_aabbtree)
	{
		App->debug_draw->RenderAABBTree();
	}
	if (show_quadtree)
	{
		App->debug_draw->RenderQuadTree();
	}
	if (show_octtree)
	{
		App->debug_draw->RenderOcTree();
	}
	if (show_bounding_boxes)
	{
		App->debug_draw->RenderBoundingBoxes();
	}
	if (show_global_bounding_boxes)
	{
		App->debug_draw->RenderGlobalBoundingBoxes();
	}
	if (show_physics)
	{
		App->debug_draw->RenderPhysics();
	}

	if(trigger_roots)
	{
		event_manager->ComputeRootPosition();
		trigger_roots = false;
	}
}

// Use this for showing variables on inspector
void DebugModeScript::OnInspector(ImGuiContext* context)
{
	//Necessary to be able to write with imgui
	ImGui::SetCurrentContext(context);
	ShowDraggedObjects();

}

void DebugModeScript::ShowDebugWindow()
{
	ImGui::SetCurrentContext(App->editor->GetImGuiContext());

	if (ImGui::Begin("Debug Window"))
	{
		DrawDebugInfo();

		ImGui::Separator();

		DrawDebugFeatures();
		ImGui::Separator();

		DrawEnemyOptions();
		ImGui::Separator();

		DrawPlayerOptions();
		
		ImGui::End();
	}
}

void DebugModeScript::DrawDebugInfo() const
{
	if (ImGui::CollapsingHeader(ICON_FA_CHART_BAR " Performance"))
	{
		std::stringstream stream_fps;
		std::stringstream stream_ms;
		stream_fps << std::fixed << std::setprecision(2) << App->time->GetFPS();
		stream_ms << std::fixed << std::setprecision(2) << App->time->delta_time;

		ImGui::LabelText((stream_fps.str() + std::string(" (") + stream_ms.str() + std::string("ms)")).c_str(), base_str_fps.c_str());
		ImGui::LabelText(std::to_string(App->renderer->GetRenderedTris()).c_str(), base_str_tris.c_str());
		ImGui::LabelText(std::to_string(App->renderer->GetRenderedVerts()).c_str(), base_str_verts.c_str());
	}
}

void DebugModeScript::DrawDebugFeatures()
{
	if (ImGui::CollapsingHeader(ICON_FA_COMPASS " Features")) 
	{
		if (ImGui::Checkbox("Draw Wireframe", &render_wireframe))
		{
			App->renderer->SetDrawMode(render_wireframe ? ModuleRender::DrawMode::WIREFRAME : ModuleRender::DrawMode::SHADED);
		}

		ImGui::Checkbox("Draw AABBTree", &show_aabbtree);
		ImGui::Checkbox("Draw QuadTree", &show_quadtree);
		ImGui::Checkbox("Draw OctTree", &show_octtree);
		ImGui::Checkbox("Draw Bounding boxes", &show_bounding_boxes);
		ImGui::Checkbox("Draw Global bounding boxes", &show_global_bounding_boxes);
		ImGui::Checkbox("Draw Physics", &show_physics);
		ImGui::Checkbox("Draw Movement Indicators", &show_movement);
		ImGui::Checkbox("Draw Player1 Combat Indicators", &player_1->player_controller->player_attack->debug_collision_draw);
		
		if(player_2->player_controller->player_attack)
		{
			ImGui::Checkbox("Draw Player2 Combat Indicators", &player_2->player_controller->player_attack->debug_collision_draw);
		}
		
		if(ImGui::Checkbox("Draw Camera Lines", &draw_camera_lines))
		{
			camera_controller->debug_options = draw_camera_lines;
		}

		ImGui::Checkbox("Trigger roots", &trigger_roots);
	}
}

void DebugModeScript::DrawEnemyOptions()
{
	if (ImGui::CollapsingHeader(ICON_FA_OTTER " Enemies"))
	{
		if (ImGui::Button("Kill All the Enemies"))
		{
			enemy_manager->KillAllTheEnemies();
		}

		if (ImGui::Button("Spawn Mushdoom"))
		{
			spawn_enemy = true;
			spawn_mushdoom = true;
		}

		ImGui::SameLine();

		if (ImGui::Button("Spawn invincible Mushdoom"))
		{
			spawn_enemy = true;
			spawn_mushdoom = true;
			spawn_invincible = true;
		}

		if (ImGui::Button("Spawn Biter"))
		{
			spawn_biter = true;
			spawn_enemy = true;
		}

		ImGui::Checkbox("Biter - Show Basic Attack Area", &show_biter_basic_attack);
		ImGui::Checkbox("Biter - Show Charge Attack Area", &show_biter_charge_attack);
		ImGui::Checkbox("Biter - Show Explosion Attack Area", &show_biter_explosion_attack);
	}
}

void DebugModeScript::DrawPlayerOptions()
{
	if (ImGui::CollapsingHeader(ICON_FA_USER_INJURED " Player Options"))
	{
		ImGui::Checkbox("Invincible mode", &is_player_invincible);
		ImGui::Separator();		
		
		ImGui::Checkbox("Saitama mode", &is_player_god_mode);
		ImGui::Separator();

		ImGui::Checkbox("Disable Auto Aim", &disable_autoaim);
		ImGui::Separator();

		if (ImGui::Button("Instant Kill Player 1"))
		{
			if (player_1->player_controller->is_alive)
			{
				InstantKillPlayer(player_1->player_controller);
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Instant Kill Player 2"))
		{
			if (player_2->player_controller->is_alive)
			{
				InstantKillPlayer(player_2->player_controller);
			}
		}
		ImGui::Separator();
		std::string zone_name = "None";
		if (selected_teleport_zone != nullptr)
		{
			zone_name = selected_teleport_zone->name;
		}

		if (ImGui::BeginCombo("Select zone to teleport", zone_name.c_str()))
		{
			for (auto& zone : teleport_zones) {
				if (ImGui::Selectable(zone->owner->name.c_str()))
				{
					selected_teleport_zone = zone->owner;
					selected_teleport_point = zone;
				}
			}
			ImGui::EndCombo();
		}

		if (ImGui::Button("Teleport to zone"))
		{
			if (!has_teleported_player_recently && selected_teleport_zone != nullptr)
			{
				has_teleported_player_recently = true;
				TeleportPlayers();
			}
		}
	}
}

void DebugModeScript::TeleportPlayers() const 
{
	if(event_manager->UpdateZone(selected_teleport_point->zone_to_spawn))
	{	
		App->engine_log->Log("Teleporting to %d", selected_teleport_point->zone_to_spawn);
		event_manager->RestartEvents(selected_teleport_point->zone_to_spawn);
		camera_controller->freeze = false;
		camera_controller->fixed_position = false;
		if (!world->multiplayer)
		{
			player_1->player_go->transform.SetGlobalMatrixTranslation(selected_teleport_zone->transform.GetGlobalTranslation());
		
		}
		else
		{
			player_1->player_go->transform.SetGlobalMatrixTranslation(selected_teleport_zone->transform.GetGlobalTranslation());
			player_2->player_go->transform.SetGlobalMatrixTranslation(selected_teleport_zone->transform.GetGlobalTranslation() + teleport_offset);
		}
		if (ally)
		{
			ally->TransformIntoLightBall();
		}
	}
}

void DebugModeScript::CheckTeleportStatus()
{
	warp_cooldown += (App->time->delta_time / 1000.f);
	if (warp_cooldown >= 1.5f)//Half a second cooldown
	{
		warp_cooldown = 0.0f;
		has_teleported_player_recently = false;
	}
}

void DebugModeScript::SpawnEnemies()
{
	if(spawn_mushdoom)
	{
		enemy_manager->SpawnEnemy(EnemyType::MUSHDOOM, player_1->player_go->transform.GetGlobalTranslation() + spawn_offset, spawn_invincible);
		spawn_mushdoom = false;
		spawn_enemy = false;
		spawn_invincible = false;
	}

	if (spawn_biter)
	{
		enemy_manager->SpawnEnemy(EnemyType::BITER, player_1->player_go->transform.GetGlobalTranslation() + spawn_offset);
		spawn_biter = false;
		spawn_enemy = false;
	}
}

void DebugModeScript::InstantKillPlayer(PlayerController* player_to_die)
{
	player_to_die->TakeDamage(instant_damage, player_to_die->hit_effect);
}

