#include "NecroExplosion.h"

#include "Component/ComponentAudioSource.h"
#include "Component/ComponentBillboard.h"
#include "Component/ComponentMeshRenderer.h"
#include "Component/ComponentParticleSystem.h"
#include "Component/ComponentScript.h"
#include "Component/ComponentTransform.h"

#include "Main/Application.h"
#include "Main/GameObject.h"
#include "Module/ModuleCamera.h"
#include "Module/ModuleInput.h"
#include "Module/ModuleScene.h"
#include "Module/ModuleTime.h"

#include "ResourceManagement/Resources/Material.h"

#include <imgui.h>


NecroExplosion* NecroExplosionDLL()
{
	NecroExplosion* instance = new NecroExplosion();
	return instance;
}

NecroExplosion::NecroExplosion()
{
}

// Use this for initialization before Start()
void NecroExplosion::Awake()
{
	GameObject* beam_billboard_go = owner->children[0];
	beam_billboard = static_cast<ComponentBillboard*>(beam_billboard_go->GetComponent(Component::ComponentType::BILLBOARD));

	explosion_mesh = owner->children[1];

	GameObject* shoot_particles_go = owner->children[2];
	explosion_particles = static_cast<ComponentParticleSystem*>(shoot_particles_go->GetComponent(Component::ComponentType::PARTICLE_SYSTEM));

	explosion_source = static_cast<ComponentAudioSource*>(owner->GetComponent(Component::ComponentType::AUDIO_SOURCE));
}

// Use this for initialization
void NecroExplosion::Start()
{

}

// Update is called once per frame
void NecroExplosion::Update()
{
	if (!exploding)
	{
		return;
	}

	current_time += App->time->delta_time;
	float progress = math::Min(1.f, current_time / explosion_time);

	float beam_size = math::Lerp(2.5f, 0.f, progress);
	beam_billboard->width = beam_size;
	beam_billboard->height = beam_size;
	beam_billboard->color.w = 1 - progress;

	float explosion_size = math::Lerp(150.f, 0.1f, progress);
	explosion_mesh->transform.SetScale(float3(explosion_size));

	if (progress == 1)
	{
		explosion_mesh->SetEnabled(false);
		exploding = false;
	}
}

void NecroExplosion::Explode(const float3& projectile_direction)
{
	exploding = true;
	current_time = 0.f;

	explosion_mesh->SetEnabled(true);
	float3 direction_to_camera = (App->cameras->main_camera->owner->transform.GetGlobalTranslation() - owner->transform.GetGlobalTranslation()).Normalized();

	explosion_mesh->transform.SetGlobalMatrixRotation(Quat::LookAt(-float3::unitY, direction_to_camera, float3::unitX, float3::unitY));

	float random_num = (rand() % 100) / 100.f;
	beam_billboard->color = float4::Lerp(spawn_color_range_left, spawn_color_range_right, random_num);

	random_num = (rand() % 100) / 100.f;
	float4 muzzle_color = float4::Lerp(spawn_color_range_left, spawn_color_range_right, random_num);
	ComponentMeshRenderer* muzzle_mesh_renderer = static_cast<ComponentMeshRenderer*>(explosion_mesh->GetComponent(Component::ComponentType::MESH_RENDERER));
	muzzle_mesh_renderer->material_to_render->diffuse_color[0] = muzzle_color.x;
	muzzle_mesh_renderer->material_to_render->diffuse_color[1] = muzzle_color.y;
	muzzle_mesh_renderer->material_to_render->diffuse_color[2] = muzzle_color.z;

	explosion_particles->Emit(25);

	explosion_source->PlayEvent("Play_Necroprojectile_Hit");
}

void NecroExplosion::Hide() const
{
	explosion_mesh->SetEnabled(false);
	explosion_particles->Stop();
	explosion_source->StopAll();
}

bool NecroExplosion::HasExplode() const
{
	return !exploding;
}

// Use this for showing variables on inspector
void NecroExplosion::OnInspector(ImGuiContext* context)
{
	//Necessary to be able to write with imgui
	ImGui::SetCurrentContext(context);
	ShowDraggedObjects();

}