#include "HitBlinker.h"

#include "Component/ComponentScript.h"
#include "Component/ComponentTransform.h"

#include "Main/Application.h"
#include "Main/GameObject.h"
#include "Module/ModuleInput.h"
#include "Module/ModuleScene.h"
#include "Module/ModuleTime.h"

#include "ResourceManagement/Resources/Material.h"
#include "Component/ComponentMeshRenderer.h"

#include <imgui.h>


HitBlinker* HitBlinkerDLL()
{
	HitBlinker* instance = new HitBlinker();
	return instance;
}

HitBlinker::HitBlinker()
{
}

// Use this for initialization before Start()
void HitBlinker::Awake()
{
	
}

// Use this for initialization
void HitBlinker::Start()
{
	mesh_renderer = static_cast<ComponentMeshRenderer*>(owner->GetComponent(Component::ComponentType::MESH_RENDERER));
}

// Update is called once per frame
void HitBlinker::Update()
{
	if (material == nullptr)
	{
		if (mesh_renderer->material_to_render != nullptr)
		{
			material = mesh_renderer->material_to_render->GetInstance();
			mesh_renderer->material_to_render = material;
		}
		else
		{
			return;
		}
	}

	if(pending_to_reset)
	{

		material->SetFinalAddedColor(float4::zero);
		pending_to_reset = false;
	}

	if (blinking)
	{
		current_time += App->time->delta_time;
		float current_progress = math::Min(current_time / blink_time, 1.f);

		float current_progress_normalized = math::PingPongMod(2*current_progress, 1.f);
		float4 current_color = float4::Lerp(float4::zero, blink_color, current_progress_normalized);
		material->SetFinalAddedColor(current_color);

		if (current_progress == 1)
		{
			blinking = false;
		}
	}

}

void HitBlinker::Blink()
{
	blinking = true;
	current_time = 0.f;
}

void HitBlinker::Reset()
{
	pending_to_reset = true;
	blinking = false;
}

// Use this for showing variables on inspector
void HitBlinker::OnInspector(ImGuiContext* context)
{
	//Necessary to be able to write with imgui
	ImGui::SetCurrentContext(context);
	ImGui::ColorEdit4("Blink Color", blink_color.ptr());
	ImGui::DragFloat("Blink Time", &blink_time);
}

 void HitBlinker::Save(Config& config) const
 {
 	config.AddColor(blink_color, "BlinkColor");
 	config.AddFloat(blink_time, "BlinkTime");
 	Script::Save(config);
 }

 void HitBlinker::Load(const Config& config)
 {
	config.GetColor("BlinkColor", blink_color, float4(0.5f, 0.5f, 0.5f, 1.f));
	blink_time = config.GetFloat("BlinkTime", 1000.f);
	Script::Load(config);
 }