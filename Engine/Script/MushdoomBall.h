#ifndef  __MUSHDOOMBALL_H__
#define  __MUSHDOOMBALL_H__

#include "Script.h"

class Mushdoom;
class ComponentCollider;
struct Player;

class MushdoomBall : public Script
{
public:
	MushdoomBall();
	~MushdoomBall() = default;

	void Awake() override;
	void Start() override;
	void Update() override;

	void OnInspector(ImGuiContext*) override;

	void ResetBall();

private:
	bool PlayerHit() const;
	float3 Parabola(const float3& start, const float3& end, float angle, float height, float t) const;

public:
	Mushdoom* mushdoom = nullptr;

private:
	ComponentCollider* collider = nullptr;
	float3 start_pos = float3::zero;
	float3 end_pos = float3::zero;

	float3 start_pos_offset = float3(0, 0.4f, 0.4f);
	float3 end_pos_offset = float3(0, 1.f, 0);

	float parabola_angle = 4.f;
	float parabola_height = 1.f;
	float parabola_speed = 0.7f;
	float current_time = 0;
	float time_to_enable_collider = 500.f;
};
extern "C" SCRIPT_API MushdoomBall* MushdoomBallDLL(); //This is how we are going to load the script
#endif