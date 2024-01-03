#ifndef  __ENEMYCONTROLLER_H__
#define  __ENEMYCONTROLLER_H__

#include "Script.h"
#include <list>
#include <vector>

class ComponentAnimation;
class ComponentAudioSource;
class ComponentBillboard;
class ComponentCollider;
class ComponentParticleSystem;
class DebugModeScript;
class EnemyManager;
class HitBlinker;
class LightOrbsManager;
class PlayerController;
class UIManager;
class WorldManager;

struct Player;
enum class NodeID;

enum class EnemyType
{
	MUSHDOOM,
	BITER,
	NECROPLANT
};

class EnemyController : public Script
{
public:
	EnemyController();
	~EnemyController() = default;

	void Awake() override;
	void InitMembers();
	void Start() override;
	virtual void Update() {}

	void OnInspector(ImGuiContext*) override;
	virtual void InitPublicGameObjects();
	virtual void SetProperties(EnemyController* original_enemy);
	void Save(Config& config) const override;
	void Load(const Config &config) override;

	bool PlayerInSight() const;
	bool PlayerInAttackRange() const;
	bool PlayerInAttackArea(const float3& area, const float& damage_radius) const;
	bool PlayerInRange(float range) const;
	void DealDamageToPlayersInArea(const float3& area, const float& damage_radius, float damage, bool& player_1_hit, bool& player_2_hit, const bool& is_explosion) const;

	PlayerController* GetClosestTarget();
	float3 CollisionAvoidance(const float3& velocity) const;
	ComponentBillboard* GetAttackVFX(const Player* player, const bool& is_explosion) const;

	void Attack();
	void Attack(float damage);
	bool PlayerHit() const;
	void HitVFX(NodeID attack);
	virtual void HitSFX(NodeID attack) {};
	void DeathVFX();
	void HitLightBallVFX() const;

	void InitVFX();
	void HideVFX() const;

	float GetCurrentHealth() const;
	float GetTotalHealth() const;

	virtual void ResetEnemy();
	virtual void TakeDamage(float damage, const float3& damage_source_position, float knockback = 0.f) {};
	virtual void Kill() {};

	virtual void KnockBack() {}

public:
	EnemyType enemy_type;
	std::vector<GameObject*> obstacles;
	std::vector<GameObject*> env_colliders;

	ComponentAnimation* animation = nullptr;
	ComponentAudioSource* audio_source = nullptr;
	ComponentBillboard* hit_effect = nullptr;
	ComponentBillboard* death_effect = nullptr;
	ComponentBillboard* LB_flare_hit_effect = nullptr;
	ComponentBillboard* light_one_hit_VFX = nullptr;
	ComponentBillboard* light_two_hit_VFX = nullptr;
	ComponentBillboard* light_three_hit_VFX = nullptr;
	ComponentBillboard* heavy_one_hit_VFX = nullptr;
	ComponentBillboard* heavy_three_hit_VFX = nullptr;
	ComponentBillboard* heavy_two_hit_VFX = nullptr;

	ComponentCollider* collider = nullptr;
	ComponentParticleSystem* LB_hit_effect_purple = nullptr;
	ComponentParticleSystem* LB_hit_effect_blue = nullptr;
	ComponentParticleSystem* LB_hit_effect_emerald = nullptr;
	ComponentParticleSystem* hit_particles_VFX = nullptr;

	EnemyManager* enemy_manager = nullptr;
	WorldManager* world_manager = nullptr;
	PlayerController* current_target = nullptr;
	UIManager* ui_manager = nullptr;
	DebugModeScript* debug = nullptr;

	float3 knockback_direction = float3::zero;
	float3 knockback_target_position = float3::zero;
	float3 death_position = float3::zero;

	float knockback_power = 0.0f;
	float frontal_collision_detection_distance = 1.5f;

	float move_speed = 6.f;
	float init_move_speed = move_speed;
	float rotate_speed = 1.f;
	float attack_speed = 1.f;
	float attack_damage = 15.f;
	float jump_distance = 3.f;
	float fall_speed = 10.f;
	float ground_ray_percentage = 1.0f;
	float front_distance = 5.f;

	bool is_alive = true;
	bool is_dying = false;
	bool is_attacking = false;
	bool move_with_physics = false;

	bool is_wave_enemy = false;

	//Debug
	bool invincible = false;

protected:
	Player* player_1 = nullptr;
	Player* player_2 = nullptr;
	LightOrbsManager* light_orbs_manager = nullptr;
	HitBlinker* hit_blinker = nullptr;

	float attack_range = 2.5f;
	float max_health = 55.f;
	float health = max_health;
	float detect_player_distance = 30.f;
	float switch_target_distance = 1.f;

	int last_time = 0;
	int current_time = 0;

private:
	const float MAX_SEE_AHEAD = 1.f;
	const float MAX_AVOID_FORCE = 1.f;
	const float MIN_SEPARATE_DISTANCE = 3.f;
};
extern "C" SCRIPT_API EnemyController* EnemyControllerDLL(); //This is how we are going to load the script
#endif