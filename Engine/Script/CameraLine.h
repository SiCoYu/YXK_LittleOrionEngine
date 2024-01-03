#ifndef  __CAMERALINE_H__
#define  __CAMERALINE_H__

#include "Script.h"

class CameraLine : public Script
{
public:
	CameraLine();
	~CameraLine() = default;

	void Awake() override;

	float3 GetPoint(float progress);
	float GetProgress(const float3& position, bool draw_lines = false);
	float3 GetPosition(size_t pos) const;
	int GetClosestPoint(const float3& position);
	LineSegment ProjectOnSegment(const LineSegment& segment, const float3& position);
	void DrawLine();


	void OnInspector(ImGuiContext*) override;

private:
	std::vector<float3> positions;
	std::vector<float> distance_acumulated;
	float total_distance;
	float distance_to_start = 0.f;
	float distance_to_lower_point = 0.f;
	float dot_product = 0.f;

};
extern "C" SCRIPT_API CameraLine* CameraLineDLL(); //This is how we are going to load the script
#endif