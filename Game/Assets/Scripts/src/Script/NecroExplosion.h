#ifndef  __NECROEXPLOSION_H__
#define  __NECROEXPLOSION_H__

#include "Script.h"

class ComponentAudioSource;
class ComponentBillboard;
class ComponentParticleSystem;

class NecroExplosion : public Script
{
public:
	NecroExplosion();
	~NecroExplosion() = default;

	void Awake() override;
	void Start() override;
	void Update() override;

	void Explode(const float3& projectile_direction);
	void Hide() const;

	bool HasExplode() const;
	
	void OnInspector(ImGuiContext*) override;

private:
	ComponentBillboard* beam_billboard = nullptr;
	GameObject* explosion_mesh = nullptr;
	ComponentParticleSystem* explosion_particles = nullptr;
	ComponentAudioSource* explosion_source = nullptr;

	bool exploding = false;
	float explosion_time = 200.f;
	float current_time = 0.f;

	const float4 spawn_color_range_left = float4(0.493f, 0.364f, 0.799f, 1.f);
	const float4 spawn_color_range_right = float4(0.366f, 0.132f, 0.926f, 1.f);

};
extern "C" SCRIPT_API NecroExplosion* NecroExplosionDLL(); //This is how we are going to load the script
#endif