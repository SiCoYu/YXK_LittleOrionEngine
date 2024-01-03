#include "NecroMuzzle.h"

#include "Component/ComponentBillboard.h"
#include "Component/ComponentMeshRenderer.h"
#include "Component/ComponentParticleSystem.h"
#include "Component/ComponentScript.h"
#include "Component/ComponentTransform.h"

#include "Main/Application.h"
#include "Main/GameObject.h"
#include "Module/ModuleInput.h"
#include "Module/ModuleScene.h"
#include "Module/ModuleTime.h"

#include "ResourceManagement/Resources/Material.h"

#include <imgui.h>


NecroMuzzle* NecroMuzzleDLL()
{
	NecroMuzzle* instance = new NecroMuzzle();
	return instance;
}

NecroMuzzle::NecroMuzzle()
{
}

// Use this for initialization before Start()
void NecroMuzzle::Awake()
{
	GameObject* beam_billboard_go = owner->children[0];
	beam_billboard = static_cast<ComponentBillboard*>(beam_billboard_go->GetComponent(Component::ComponentType::BILLBOARD));

	muzzle_mesh = owner->children[1];

	GameObject* shoot_particles_go = owner->children[2];
	shoot_particles = static_cast<ComponentParticleSystem*>(shoot_particles_go->GetComponent(Component::ComponentType::PARTICLE_SYSTEM));
}

// Use this for initialization
void NecroMuzzle::Start()
{

}

// Update is called once per frame
void NecroMuzzle::Update()
{
	if (!shooting)
	{
		return;
	}

	current_time += App->time->delta_time;
	float progress = math::Min(1.f, current_time / muzzle_time);

	float beam_size = math::Lerp(2.5f, 0.f, progress);
	beam_billboard->width = beam_size;
	beam_billboard->height = beam_size;
	beam_billboard->color.w = 1 - progress;

	float muzzle_size = math::Lerp(100.f, 0.1f, progress);
	muzzle_mesh->transform.SetScale(float3(muzzle_size));

	if (progress == 1)
	{
		shooting = false;
		Hide();
	}
}

void NecroMuzzle::Shoot()
{
	shooting = true;
	current_time = 0.f;
	muzzle_mesh->SetEnabled(true);

	float random_num = (rand() % 100) / 100.f;
	beam_billboard->color = float4::Lerp(spawn_color_range_left, spawn_color_range_right, random_num);

	random_num = (rand() % 100) / 100.f;
	float4 muzzle_color = float4::Lerp(spawn_color_range_left, spawn_color_range_right, random_num);
	ComponentMeshRenderer* muzzle_mesh_renderer = static_cast<ComponentMeshRenderer*>(muzzle_mesh->GetComponent(Component::ComponentType::MESH_RENDERER));
	muzzle_mesh_renderer->material_to_render->diffuse_color[0] = muzzle_color.x;
	muzzle_mesh_renderer->material_to_render->diffuse_color[1] = muzzle_color.y;
	muzzle_mesh_renderer->material_to_render->diffuse_color[2] = muzzle_color.z;

	shoot_particles->Emit(10);
}

void NecroMuzzle::Hide() const
{
	muzzle_mesh->SetEnabled(false);
}

// Use this for showing variables on inspector
void NecroMuzzle::OnInspector(ImGuiContext* context)
{
	//Necessary to be able to write with imgui
	ImGui::SetCurrentContext(context);
	ShowDraggedObjects();

}