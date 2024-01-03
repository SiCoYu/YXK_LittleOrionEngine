#include "CameraLine.h"

#include "Component/ComponentScript.h"
#include "Component/ComponentTransform.h"

#include "Main/Application.h"
#include "Main/GameObject.h"
#include "Module/ModuleDebugDraw.h"
#include "Module/ModuleInput.h"
#include "Module/ModuleScene.h"

#include "imgui.h"

CameraLine* CameraLineDLL()
{
	CameraLine* instance = new CameraLine();
	return instance;
}

CameraLine::CameraLine()
{
	
}

// Use this for initialization before Start()
void CameraLine::Awake()
{
	for (auto& go : owner->children)
	{
		positions.emplace_back(go->transform.GetGlobalTranslation());
		go->SetEnabled(false);
	}

	distance_acumulated.emplace_back(0.f);
	for (size_t i = 1; i < positions.size(); ++i)
	{
		distance_acumulated.emplace_back(positions[i].Distance(positions[i - 1]) + distance_acumulated[i - 1]);
	}
	total_distance = distance_acumulated[distance_acumulated.size() - 1];
}

float3 CameraLine::GetPoint(float progress)
{
	float current_progress = total_distance * progress;
	bool found = false;
	int i = 0;
	if (progress <= 0.f)
	{
		return positions[0];
	}
	if (progress >= 1.f)
	{
		return positions[positions.size() - 1];
	}

	while (!found && i < distance_acumulated.size())
	{
		if (distance_acumulated[i] < current_progress)
		{
			++i;
		}
		else
		{
			found = true;
		}
	}

	float position_percentage = (current_progress - distance_acumulated[i - 1]) / (distance_acumulated[i] - distance_acumulated[i - 1]);

	return positions[i - 1].Lerp(positions[i], position_percentage);
}

void CameraLine::DrawLine()
{
	App->debug_draw->RenderPoint(positions[0], 10.f, float3(0.f, 1.f, 0.f));
	for (size_t i = 1; i < positions.size(); ++i)
	{
		App->debug_draw->RenderPoint(positions[i], 10.f, float3(0.f, 1.f, 0.f));
		App->debug_draw->RenderLine(positions[i - 1], positions[i], float3(0.f, 1.f, 0.f));
	}
}

float CameraLine::GetProgress(const float3& position, bool draw_lines)
{
	int closest_point_index = GetClosestPoint(position);
	LineSegment perpendicular_to_current_segment;

	LineSegment current_line_segment;
	int lower_limit_index = -1;

	if (closest_point_index == 0)
	{
		lower_limit_index = 0;
		current_line_segment = LineSegment(positions[0], positions[1]);
		perpendicular_to_current_segment = ProjectOnSegment(current_line_segment, position);

	}
	else if (closest_point_index == positions.size() - 1)
	{
		lower_limit_index = positions.size() - 2;
		current_line_segment = LineSegment(positions[positions.size() - 2], positions[positions.size() - 1]);
		perpendicular_to_current_segment = ProjectOnSegment(current_line_segment, position);

	}
	else
	{
		LineSegment left_segment = LineSegment(positions[closest_point_index - 1], positions[closest_point_index]);
		LineSegment perpendicular_to_left_segment = ProjectOnSegment(left_segment, position);
		bool is_in_left_segment = left_segment.Contains(perpendicular_to_left_segment.b);

		LineSegment right_segment = LineSegment(positions[closest_point_index], positions[closest_point_index + 1]);
		LineSegment perpendicular_to_right_segment = ProjectOnSegment(right_segment, position);
		bool is_in_right_segment = right_segment.Contains(perpendicular_to_right_segment.b);

		//TODO: This should never happen;
		if (!is_in_left_segment && !is_in_right_segment)
		{
			lower_limit_index = closest_point_index;
			current_line_segment = right_segment;
			perpendicular_to_current_segment = LineSegment(position, positions[closest_point_index]);
		}
		if (!is_in_left_segment && is_in_right_segment)
		{
			lower_limit_index = closest_point_index;
			current_line_segment = right_segment;
			perpendicular_to_current_segment = perpendicular_to_right_segment;
		}
		else if (is_in_left_segment && !is_in_right_segment)
		{
			lower_limit_index = closest_point_index - 1;
			current_line_segment = left_segment;
			perpendicular_to_current_segment = perpendicular_to_left_segment;
		}
		else
		{
			if (perpendicular_to_left_segment.Length() <= perpendicular_to_right_segment.Length())
			{
				lower_limit_index = closest_point_index - 1;
				current_line_segment = left_segment;
				perpendicular_to_current_segment = perpendicular_to_left_segment;
			}
			else
			{
				lower_limit_index = closest_point_index;
				current_line_segment = right_segment;
				perpendicular_to_current_segment = perpendicular_to_right_segment;
			}
		}
	}

	float3 position_projection_in_line = perpendicular_to_current_segment.b;

	distance_to_lower_point = position_projection_in_line.Distance(current_line_segment.a);
	distance_to_start = distance_acumulated[lower_limit_index] + distance_to_lower_point;

	if(draw_lines)
	{
		App->debug_draw->RenderPoint(positions[lower_limit_index], 25.f, float3(0.f, 0.f, 1.f));
		App->debug_draw->RenderLine(current_line_segment.a, current_line_segment.b, float3(1.f, 1.f, 1.f));

		App->debug_draw->RenderPoint(perpendicular_to_current_segment.b, 25.f, float3(1.f,0.f,1.f));
		App->debug_draw->RenderLine(perpendicular_to_current_segment.a, perpendicular_to_current_segment.b, float3(1.f, 1.f, 1.f));

		App->debug_draw->RenderPoint(positions[closest_point_index], 25.f, float3(1.f,0.f,0.f));
		App->debug_draw->RenderLine(perpendicular_to_current_segment.a, positions[closest_point_index], float3(1.f, 0.f, 0.f));
	}

	float3 line_direction = positions[positions.size() - 1] - positions[0];
	float3 current_position_direction = perpendicular_to_current_segment.b - positions[0];
	dot_product = line_direction.Dot(current_position_direction);

	if(dot_product < 0)
	{
		distance_to_start = 0.f;
	}

	return distance_to_start / total_distance;
}

float3 CameraLine::GetPosition(size_t pos) const
{
	return positions[pos];
}

int CameraLine::GetClosestPoint(const float3& position)
{
	int closest_point_index = -1;
	float shortest_distance = 10000;
	for (int i = 0; i < positions.size(); ++i)
	{
		float current_distance = positions[i].Distance(position);
		if (current_distance < shortest_distance) {
			shortest_distance = current_distance;
			closest_point_index = i;
		}
	}
	return closest_point_index;
}

LineSegment CameraLine::ProjectOnSegment(const LineSegment& segment, const float3& position)
{
	float3 AP = position - segment.a;
	float3 AB = segment.b - segment.a;
	float3 projection = segment.a + AP.Dot(AB) / AB.Dot(AB) * AB;

	return LineSegment(position, projection);
}

// Use this for showing variables on inspector
void CameraLine::OnInspector(ImGuiContext* context)
{
	//Necessary to be able to write with imgui
	ImGui::SetCurrentContext(context);
	ShowDraggedObjects();
	ImGui::DragFloat("Distance To Start", &distance_to_start);
	ImGui::DragFloat("Distance To Lower Point", &distance_to_lower_point);
	ImGui::DragFloat("Dot Product", &dot_product);
	//ImGui::DragFloat3("New Scale", new_scale.ptr());
}