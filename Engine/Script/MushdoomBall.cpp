#include "MushdoomBall.h"

#include "Component/ComponentScript.h"
#include "Component/ComponentTransform.h"
#include "Component/ComponentParticleSystem.h"

#include "Main/Application.h"
#include "Main/GameObject.h"
#include "Module/ModuleScene.h"
#include "Module/ModuleTime.h"


#include "imgui.h"

#include "Mushdoom.h"
#include "PlayerController.h"

MushdoomBall* MushdoomBallDLL()
{
	MushdoomBall* instance = new MushdoomBall();
	return instance;
}

MushdoomBall::MushdoomBall()
{
	
}

void MushdoomBall::Awake()
{
	mushdoom = static_cast<Mushdoom*>(owner->parent->GetComponentScript("Mushdoom")->script);
	collider = static_cast<ComponentCollider*>(owner->children[1]->GetComponent(Component::ComponentType::COLLIDER));
}

void MushdoomBall::Start()
{
	ResetBall();
}

void MushdoomBall::Update()
{
	current_time += App->time->delta_time;

	float3 position = Parabola(start_pos, end_pos, parabola_angle, parabola_height, current_time / (parabola_speed * 1000));

	owner->transform.SetGlobalMatrixTranslation(position);

	if (position.x == end_pos.x && position.y == end_pos.y && position.z == end_pos.z)
	{
		if (mushdoom->ball_die_effect)
		{
			mushdoom->ball_die_effect->owner->transform.SetGlobalMatrixTranslation(owner->transform.GetGlobalTranslation());
		
		}

		owner->SetEnabled(false);
		owner->transform.SetGlobalMatrixTranslation(float3(0, 0, 40));
	}

	if (current_time >= time_to_enable_collider && !collider->owner->IsEnabled())
	{
		collider->owner->SetEnabled(true);
	}

	if (PlayerHit() && collider->owner->IsEnabled())
	{
		if (mushdoom->ball_die_effect)
		{
			mushdoom->ball_die_effect->owner->transform.SetGlobalMatrixTranslation(owner->transform.GetGlobalTranslation());
			mushdoom->ball_die_effect->Emit(20);
		}

		owner->SetEnabled(false);
		owner->transform.SetGlobalMatrixTranslation(float3(0, 0, 40));
		mushdoom->current_target->EmitFromImpactPlayerBallMushdoom();
		mushdoom->current_target->TakeDamage(mushdoom->attack_damage * 2, mushdoom->current_target->hit_effect);
	}
}

void MushdoomBall::OnInspector(ImGuiContext* context)
{
	ImGui::SetCurrentContext(context);
	ShowDraggedObjects();

	ImGui::Text("Positions");
	ImGui::DragFloat3("Start Position", &start_pos[0]);
	ImGui::DragFloat3("End Position", &end_pos[0]);
	ImGui::NewLine();

	ImGui::Text("Position Offsets");
	ImGui::DragFloat3("Start Offset", &start_pos_offset[0]);
	ImGui::DragFloat3("End Offset", &end_pos_offset[0]);
	ImGui::NewLine();

	ImGui::Text("Parabola Parameters");
	ImGui::InputFloat("Angle", &parabola_angle);
	ImGui::InputFloat("Height", &parabola_height);
	ImGui::InputFloat("Speed", &parabola_speed);
}

void MushdoomBall::ResetBall()
{
	start_pos = mushdoom->owner->transform.GetGlobalTranslation() + start_pos_offset;
	if(mushdoom->current_target)
	{
		end_pos = mushdoom->current_target->owner->transform.GetGlobalTranslation() + end_pos_offset;
	}
	else
	{
		end_pos = start_pos;
	}

	owner->transform.SetGlobalMatrixTranslation(start_pos);

	current_time = 0;

	collider->owner->SetEnabled(false);
}

bool MushdoomBall::PlayerHit() const
{
	if (mushdoom->current_target == nullptr)
	{
		return false;
	}
	else
	{
		mushdoom->ball_die_effect->Emit(20);
		return collider->IsCollidingWith(mushdoom->current_target->GetCollider());
	}
}

float3 MushdoomBall::Parabola(const float3& start, const float3& end, float angle, float height, float t) const
{
	float3 mid = float3::Lerp(start, end, t);

	mid.y = -parabola_angle * parabola_height * t * t + parabola_angle * parabola_height * t;

	return (t >= 0.998f) ? end : float3(mid.x, mid.y + math::Lerp(start.y, end.y, t), mid.z);
}
