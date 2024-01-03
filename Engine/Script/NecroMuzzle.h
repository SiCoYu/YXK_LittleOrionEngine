#ifndef  __NECROMUZZLE_H__
#define  __NECROMUZZLE_H__

#include "Script.h"

class ComponentBillboard;
class ComponentParticleSystem;

class NecroMuzzle : public Script
{
public:
	NecroMuzzle();
	~NecroMuzzle() = default;

	void Awake() override;
	void Start() override;
	void Update() override;

	void Shoot();
	void Hide() const;

	void OnInspector(ImGuiContext*) override;

private:
	ComponentBillboard* beam_billboard = nullptr;
	GameObject* muzzle_mesh = nullptr;
	ComponentParticleSystem* shoot_particles = nullptr;

	bool shooting = false;
	float muzzle_time = 200.f;
	float current_time = 0.f;

	const float4 spawn_color_range_left = float4(0.493f, 0.364f, 0.799f, 1.f);
	const float4 spawn_color_range_right = float4(0.366f, 0.132f, 0.926f, 1.f);

};
extern "C" SCRIPT_API NecroMuzzle* NecroMuzzleDLL(); //This is how we are going to load the script
#endif