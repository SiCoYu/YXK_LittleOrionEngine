#include "DamageIndicator.h"

#include "Component/ComponentCanvasRenderer.h"
#include "Component/ComponentScript.h"
#include "Component/ComponentText.h"
#include "Component/ComponentTransform.h"

#include "Main/Application.h"
#include "Main/GameObject.h"
#include "Log/EngineLog.h"
#include "Module/ModuleInput.h"
#include "Module/ModuleScene.h"
#include "Module/ModuleTime.h"


#include "imgui.h"


DamageIndicator* DamageIndicatorDLL()
{
	DamageIndicator* instance = new DamageIndicator();
	return instance;
}

DamageIndicator::DamageIndicator()
{
	
}

// Use this for initialization before Start()
void DamageIndicator::Awake()
{
	damage_indicator_text = static_cast<ComponentText*>(owner->GetComponent(Component::ComponentType::UI_TEXT));
	state = DamageIndicator::DamageIndicatorState::DEAD;
}

// Use this for initialization
void DamageIndicator::Start()
{

}

// Update is called once per frame
void DamageIndicator::Update()
{
	if (state == DamageIndicator::DamageIndicatorState::DEAD)
	{
		return;
	}

	float elapsed_time = App->time->delta_time;
	current_time += elapsed_time;

	switch (state)
	{
	case DamageIndicator::DamageIndicatorState::APPEARING:
	{
		if (current_time > scale_duration)
		{
			state = DamageIndicator::DamageIndicatorState::MOVING;
			owner->transform_2d.SetScale(float3(target_scale + variable_scale));
			current_time = 0.f;
			return;
		}

		float progress = current_time / scale_duration;
		float current_scale = math::Lerp(initial_scale, target_scale, progress);
		owner->transform_2d.SetScale(float3(current_scale + variable_scale));
		break;
	}
	case DamageIndicator::DamageIndicatorState::MOVING:
	{
		if (current_time > movement_duration)
		{
			static_cast<ComponentText*>(owner->GetComponent(Component::ComponentType::CANVAS_RENDERER))->Disable();
			state = DamageIndicator::DamageIndicatorState::DEAD;
			return;
		}

		float alpha = 1 - current_time / movement_duration;
		float4 font_color = damage_indicator_text->GetFontColor();
		damage_indicator_text->SetFontColor(float4(font_color.xyz(), alpha));

		float3 position = float3::zero;
		position.x = speed.x * current_time * 0.001f;
		position.y = speed.y * current_time * 0.001f - 0.5f * gravity * current_time * 0.001f * current_time * 0.001f;

		owner->transform_2d.SetGlobalMatrixTranslation((spatial_transformation *  float4(position, 1.f)).xyz());

		break;
	}
	default:
		break;
	} 
}

bool DamageIndicator::IsAlive() const
{
	return state != DamageIndicatorState::DEAD;
}

void DamageIndicator::Spawn(int number, float power, float2 position, const float2& direction)
{
	state = DamageIndicatorState::APPEARING;
	
	spatial_transformation = float4x4::Translate(float3(position, 0.f) + float3::unitY * offset_y);
	speed.x = direction.x * distance / (movement_duration * 0.001f);
	speed.y = math::Abs(direction.y) * 0.5 * gravity * movement_duration * 0.001f;
	position.y += offset_y;
	owner->transform_2d.SetGlobalMatrixTranslation(float3(position,0.f));

	variable_scale = power * 0.1f;
	owner->transform_2d.SetScale(float3(initial_scale + variable_scale));

	float4 min_font_color(0.96f, 0.84f, 0.32f, 1.f);
	float4 max_font_color(0.8f, 0.18f, 0.12f, 1.f);
	float4 color = float4::Lerp(min_font_color, max_font_color, power);
	App->engine_log->Log("Power is %f", power);
	damage_indicator_text->SetFontColor(color);

	damage_indicator_text->SetText(std::to_string(number));
	static_cast<ComponentText*>(owner->GetComponent(Component::ComponentType::CANVAS_RENDERER))->Enable();

	current_time = 0.f;
}

// Use this for showing variables on inspector
void DamageIndicator::OnInspector(ImGuiContext* context)
{
	//Necessary to be able to write with imgui
	ImGui::SetCurrentContext(context);
	ShowDraggedObjects();

	ImGui::DragFloat2("Speed", speed.ptr());
	ImGui::InputFloat("Movement Duration", &movement_duration);
	ImGui::InputFloat("Scale Duration", &scale_duration);
	ImGui::InputFloat("Offset Y", &offset_y);
}

//Use this for linking JUST GO automatically 
void DamageIndicator::InitPublicGameObjects()
{
	//IMPORTANT, public gameobjects, name_gameobjects and go_uuids MUST have same size

	//public_gameobjects.push_back(&example);
	//variable_names.push_back(GET_VARIABLE_NAME(example));

	for (int i = 0; i < public_gameobjects.size(); ++i)
	{
		name_gameobjects.push_back(is_object);
		go_uuids.push_back(0);
	}
}

//Use this for linking GO AND VARIABLES automatically if you need to save variables 
void DamageIndicator::Save(Config& config) const
{
	config.AddFloat(distance, "Distance");
	config.AddFloat(movement_duration, "Movement Duration");
	config.AddFloat(offset_y, "Offset Y");
	Script::Save(config);
}

// //Use this for linking GO AND VARIABLES automatically
void DamageIndicator::Load(const Config& config)
{
	distance = config.GetFloat("Distance", 0.05f);
	movement_duration = config.GetFloat("Movement Duration", 500.f);
	offset_y = config.GetFloat("Offset Y", 40.f);
	Script::Load(config);
}