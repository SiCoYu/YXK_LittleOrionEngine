#ifndef  __LIGHTTELEPORTMANAGER_H__
#define  __LIGHTTELEPORTMANAGER_H__

#include "Script.h"

class ComponentAnimation;
class ComponentCollider;
class EnemyController;
class CEnemyManager;
class WorldManager;

class LightTeleportManager : public Script
{
	enum class LightTeleportState
	{
		DISABLE,
		SHOOTING_PROJECTILE,
		WAITING_FOR_WARP,
		WARP,
		COOLDOWN
	};

public:
	LightTeleportManager();
	~LightTeleportManager() = default;

	void Awake() override;
	void Start() override;
	void Update() override;

	void OnInspector(ImGuiContext*) override;
	//void Save(Config& config) const override;
	//void Load(const Config& config) override;

	bool LightWarp();
	bool IsOnCoolDown();

private:
	bool DetectCollision();
	void ShootProjectile();
	void TriggerWarp();

public:
	float teleport_cooldown = 0.0f;

private:
	GameObject* projectile = nullptr;
	CEnemyManager* enemy_manager = nullptr;
	WorldManager* world_manager = nullptr;
	EnemyController* marked_enemy = nullptr;
	ComponentAnimation* animation = nullptr;
	ComponentCollider* projectile_collider = nullptr;
	ComponentCollider* player_collider = nullptr;
	LightTeleportState tp_state = LightTeleportState::DISABLE;

	//Cooldown
	float current_cooldown_time_teleport = 0.0f;
	bool casting_projectile = false;
	bool warping = false;

	//Projectile variables
	float3 projectile_direction = float3::zero;
	float3 projectile_initial_position = float3::zero;
	float maximum_projectile_distance = 15.0f;
	float projectile_speed = 3.f;
	float current_projectile_lerp = 0.0f;
	
	//Marked time
	float current_time_marked = 0.0f;
	float cooldown_enemy_mark = 5.0f;

	//Warp variables
	float3 player_initial_position = float3::zero;
	float3 player_warp_target = float3::zero;
	float warp_speed = 4.0f;
	float current_warp_lerp = 0.0f;

};
extern "C" SCRIPT_API LightTeleportManager* LightTeleportManagerDLL(); //This is how we are going to load the script
#endif