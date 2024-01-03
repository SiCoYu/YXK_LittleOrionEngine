#ifndef  __NECROPROJECTILE_H__
#define  __NECROPROJECTILE_H__

#include "Script.h"

class GameObject;
class ComponentBillboard;
class ComponentTransform;
class NecroExplosion;
class PlayerController;
class WorldManager;

class NecroProjectile : public Script
{
public:
	NecroProjectile();
	~NecroProjectile() = default;

	void Awake() override;
	void Start() override;
	void Update() override;

	void OnInspector(ImGuiContext*) override;

	void Save(Config& config) const override;
	void Load(const Config& config) override;

	bool IsDead() const;

	void Spawn(const float3& spawn_position, const float3& direction);
	void Hide() const;

	void HitPlayer(const float3& projectile_direction);

private:
	bool is_alive = false;
	float alive_time = 0.f;
	float projectile_time_duration = 2500.f;

	float3 projectile_direction = float3::zero;
	float radius = 2.5f;
	float speed = 20.f;

	float rotation_speed = 15.f;
	float rotation_angle = 0.f;

	float beam_alpha_oscillation_speed = 15.f;

	float projectile_damage = 20.f;

	NecroExplosion* necro_explosion = nullptr;
	bool exploding = false;

	GameObject* projectile_mesh_go = nullptr;
	GameObject* projectile_beam_go = nullptr;
	ComponentBillboard* beam_billboard = nullptr;

	ComponentTransform* player_1_transform = nullptr;
	PlayerController* player_1_controller = nullptr;

	ComponentTransform* player_2_transform = nullptr;
	PlayerController* player_2_controller = nullptr;
	WorldManager* world_manager = nullptr;

};
extern "C" SCRIPT_API NecroProjectile* NecroProjectileDLL(); //This is how we are going to load the script
#endif