#include "LightOrb.h"

#include "Component/ComponentAudioSource.h"
#include "Component/ComponentBillboard.h"
#include "Component/ComponentMeshRenderer.h"
#include "Component/ComponentScript.h"
#include "Component/ComponentTransform.h"
#include "Component/ComponentTrail.h"

#include "Main/Application.h"
#include "Main/GameObject.h"
#include "Module/ModuleInput.h"
#include "Module/ModuleScene.h"
#include "Module/ModuleTime.h"

#include "imgui.h"

#include "PlayerController.h"
#include "WorldManager.h"

LightOrb* LightOrbDLL()
{
	LightOrb* instance = new LightOrb();
	return instance;
}

LightOrb::LightOrb()
{
	
}

// Use this for initialization before Start()
void LightOrb::Awake()
{
	GameObject* world_manager_game_object = App->scene->GetGameObjectByName("World Manager");
	world = static_cast<WorldManager*>(world_manager_game_object->GetComponentScript("WorldManager")->script);
	audio_source = static_cast<ComponentAudioSource*>(owner->GetComponent(Component::ComponentType::AUDIO_SOURCE));
	
	for (const auto ch : owner->children)
	{
		if (ch->tag == "small_texture_light_orb_VFX")
		{
			small_texture_light_orb = static_cast<ComponentBillboard*>(ch->GetComponent(Component::ComponentType::BILLBOARD));
			small_texture_light_orb->loop = true;
		}
		if (ch->tag == "shine_texture_light_orb_VFX")
		{
			shine_texture_light_orb = static_cast<ComponentBillboard*>(ch->GetComponent(Component::ComponentType::BILLBOARD));
			shine_texture_light_orb->loop = true;
		}	
		if (ch->tag == "exterior_texture_light_orb_VFX")
		{
			exterior_texture_light_orb= static_cast<ComponentBillboard*>(ch->GetComponent(Component::ComponentType::BILLBOARD));
			exterior_texture_light_orb->loop = true;
		}
		
		if (ch->tag == "trail_light_orb_VFX")
		{
			trail_light_orb_VFX = static_cast<ComponentTrail*>(ch->GetComponent(Component::ComponentType::TRAIL));
			trail_light_orb_VFX->Disable();
		}
	}
}

// Use this for initialization
void LightOrb::Start()
{
	player_1 = world->GetPlayer1();
	player_2 = world->GetPlayer2();
}

// Update is called once per frame
void LightOrb::Update()
{
	if (world->on_pause)
	{
		return;
	}

	if (life_state == LightOrbLifeState::DEAD)
	{
		small_texture_light_orb->Disable();
		shine_texture_light_orb->Disable();
		exterior_texture_light_orb->Disable();
		return;
	}

	float delta_time = App->time->delta_time;
	current_time += delta_time;
	
	switch (life_state)
	{
	case LightOrbLifeState::SPAWNING:	
	{
		float3 next_position = spawning_position + initial_speed * current_time;
		next_position.y = Clamp(spawning_position.y + initial_speed.y * current_time - GRAVITY * 0.5f * current_time * current_time, waiting_position.y, 100.f);
		owner->transform.SetGlobalMatrixTranslation(next_position);
		if (next_position.y <= waiting_position.y && current_time > spawning_time)
		{
			current_time = 0;
			life_state = LightOrbLifeState::WAITING;
		}

		break;
	}
	case LightOrbLifeState::WAITING:
	{
		
		player_to_deliver = GetClosestAlivePlayer();
		if (player_to_deliver != nullptr)
		{
			current_time = 0;
			life_state = LightOrbLifeState::TRAVELLING;
			waiting_position = owner->transform.GetGlobalTranslation();
			break;
		}
		
		float new_height = (math::Sin(current_time * oscillation_speed) + 1) * 0.5f * max_oscillation_height;
		float3 oscillation_position = owner->transform.GetGlobalTranslation();
		oscillation_position.y = waiting_position.y + new_height;
		owner->transform.SetGlobalMatrixTranslation(oscillation_position);

		if (current_time >= stil_time)
		{
			current_time = 0;
			life_state = LightOrbLifeState::DEAD;
			owner->children[0]->SetEnabled(false);
		}
		break;
	}
	case LightOrbLifeState::TRAVELLING:
		if (!player_to_deliver->player_controller->is_alive)
		{
			player_to_deliver = GetClosestAlivePlayer();
			current_time = 0;
			waiting_position = owner->transform.GetGlobalTranslation();

			if (player_to_deliver != nullptr)
			{
				life_state = LightOrbLifeState::TRAVELLING;
			}
			else
			{
				life_state = LightOrbLifeState::WAITING;
			}
			break;
		}

		if (current_time < travel_time)
		{
			//trail_light_orb_VFX->duration = 1000.0f;
			float progress = current_time / travel_time;
			float3 new_position = float3::Lerp(waiting_position, player_to_deliver->player_go->transform.GetGlobalTranslation() + float3::unitY, progress);
			owner->transform.SetGlobalMatrixTranslation(new_position);
		}
		else
		{
			//trail_light_orb_VFX->duration = 0.0f;
			current_time = 0;
			life_state = LightOrbLifeState::DEAD;
			owner->children[0]->SetEnabled(false);
			PlayerController* player_to_deliver_controller = static_cast<PlayerController*>(player_to_deliver->player_go->GetComponentScript("PlayerController")->script);
			player_to_deliver_controller->Heal(healing_value);
			audio_source->PlayEvent("play_orbs_pickup");
			player_to_deliver_controller->EmitHealVFX();
		}
		break;
	}
}

void LightOrb::Spawn(const float3& spawn_position)
{
	life_state = LightOrbLifeState::SPAWNING;
	owner->children[0]->SetEnabled(true);
	current_time = 0.f;

	spawning_position = spawn_position;
	float3 offset = float3(rand() % 100 - 50, 5, rand() % 100 - 50);
	offset.ScaleToLength(spread);
	waiting_position = GetClosestTerrainPosition(spawning_position + offset);

	owner->transform.SetGlobalMatrixTranslation(spawning_position);

	initial_speed = offset / speed;
	initial_speed.y = GRAVITY * 0.5f * speed;

	small_texture_light_orb->Play();
	shine_texture_light_orb->Play();
	exterior_texture_light_orb->Play();
}

Player* LightOrb::GetClosestAlivePlayer() const
{
	float distance_closest_player = 1000.0F;
	Player* closest_player = nullptr;

	if (player_1->player_controller->is_alive)
	{
		distance_closest_player = player_1->player_go->transform.GetGlobalTranslation().Distance(owner->transform.GetGlobalTranslation());
		closest_player = player_1;
	}

	if (world->multiplayer && player_2->player_controller->is_alive)
	{
		float distance_player_2 = player_2->player_go->transform.GetGlobalTranslation().Distance(owner->transform.GetGlobalTranslation());
		if (distance_player_2 < distance_closest_player)
		{
			closest_player = player_2;
			distance_closest_player = distance_player_2;
		}
	}

	if (distance_closest_player > attracting_radio)
	{
		return nullptr;
	}
	else
	{
		return closest_player;
	}
}

bool LightOrb::IsAlive() const
{
	return life_state != LightOrbLifeState::DEAD;
}

void LightOrb::CloneProperties(LightOrb* original_orb)
{
	spread = original_orb->spread;
	spawning_time = original_orb->spawning_time;
	stil_time = original_orb->stil_time;
	travel_time = original_orb->travel_time;
	waiting_position = original_orb->waiting_position;
	spawning_position = original_orb->spawning_position;
	oscillation_speed = original_orb->oscillation_speed;
	max_oscillation_height = original_orb->max_oscillation_height;
	attracting_radio = original_orb->attracting_radio;
	healing_value = original_orb->healing_value;
	special_value = original_orb->special_value;
	mesh_environment = original_orb->GetMeshEnvironment();
}

// Use this for showing variables on inspector
void LightOrb::OnInspector(ImGuiContext* context)
{
	//Necessary to be able to write with imgui
	ImGui::SetCurrentContext(context);
	ShowDraggedObjects();

	ImGui::TextColored(ImVec4(0.f, 1.f, 1.f, 1.f), "Spawning");
	ImGui::Spacing();
	ImGui::DragFloat("Spread", &spread);
	ImGui::DragFloat("SpawningTime", &spawning_time);

	ImGui::Separator();

	ImGui::TextColored(ImVec4(0.f, 1.f, 1.f, 1.f), "Waiting");
	ImGui::Spacing();
	ImGui::DragFloat("StillTime", &stil_time);
	ImGui::DragFloat("OscillationSpeed", &oscillation_speed);
	ImGui::DragFloat("OscillationHeight", &max_oscillation_height);

	ImGui::Separator();

	ImGui::TextColored(ImVec4(0.f, 1.f, 1.f, 1.f), "Travelling");
	ImGui::Spacing();
	ImGui::DragFloat("TravelTime", &travel_time);
	ImGui::DragFloat("Attracting Radio", &attracting_radio);

	ImGui::Separator();

	ImGui::TextColored(ImVec4(0.f, 1.f, 1.f, 1.f), "Healing");
	ImGui::Spacing();
	ImGui::DragFloat("Healing Value", &healing_value);
	ImGui::DragFloat("Special Value", &special_value);
}

//Use this for linking GO AND VARIABLES automatically if you need to save variables 
void LightOrb::Save(Config& config) const
{
	config.AddFloat(spread, "Spread");
	config.AddFloat(stil_time, "StillTime");
	config.AddFloat(oscillation_speed, "OscillationSpeed");
	config.AddFloat(max_oscillation_height, "OscillationHeight");
	config.AddFloat(travel_time, "TravelTime");
	config.AddFloat(healing_value, "HealingValue");
	config.AddFloat(special_value, "SpecialValue");
}

// //Use this for linking GO AND VARIABLES automatically
void LightOrb::Load(const Config& config)
{
	spread = config.GetFloat("Spread", 3.f);
	stil_time = config.GetFloat("StillTime", 5000.f);
	oscillation_speed = config.GetFloat("OscillationSpeed", 0.05f);
	max_oscillation_height = config.GetFloat("OscillationHeight", 0.5f);
	travel_time = config.GetFloat("TravelTime", 500.f);
	healing_value = config.GetFloat("HealingValue", 1.f);
	special_value = config.GetFloat("SpecialValue", 0.1f);
}

float3 LightOrb::GetClosestTerrainPosition (const float3& current_next_position) const
{
	LineSegment ground_line(current_next_position , current_next_position - float3::unitY * ray_lenght);

	float min_distance = INFINITY;
	float3 intersection_point = float3::zero;

	for (const auto& mesh : mesh_environment)
	{
		LineSegment transformed_ray = ground_line;
		transformed_ray.Transform(mesh->owner->transform.GetGlobalModelMatrix().Inverted());
		std::vector<Mesh::Vertex> &vertices = mesh->mesh_to_render->vertices;
		std::vector<uint32_t> &indices = mesh->mesh_to_render->indices;
		for (size_t i = 0; i < indices.size(); i += 3)
		{
			float3 first_point = vertices[indices[i]].position;
			float3 second_point = vertices[indices[i + 1]].position;
			float3 third_point = vertices[indices[i + 2]].position;
			Triangle triangle(first_point, second_point, third_point);

			float distance;
			float3 intersected_point;
			bool intersected = triangle.Intersects(transformed_ray, &distance, &intersected_point);
			if (intersected && distance < min_distance)
			{
				min_distance = distance;
				intersection_point = intersected_point;
			}
		}
	}

	intersection_point.x = current_next_position.x;
	intersection_point.y = ((intersection_point.z + vertical_offset) > current_next_position.y) ? current_next_position.y : intersection_point.z + vertical_offset;
	intersection_point.z = current_next_position.z;

	return intersection_point;

}

void LightOrb::SetMeshEnvironment(const std::vector<ComponentMeshRenderer*>& mesh_environment)
{
	this->mesh_environment = mesh_environment;
}

std::vector<ComponentMeshRenderer*> LightOrb::GetMeshEnvironment() const
{
	return mesh_environment;
}


