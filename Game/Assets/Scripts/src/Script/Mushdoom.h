#ifndef  __MUSHDOOM_H__
#define  __MUSHDOOM_H__

#include "EnemyController.h"
#include "EnemyManager.h"
#include "EnemyState.h"

class MushdoomBall;
class ComponentParticleSystem;
enum class NodeID;

class Mushdoom : public EnemyController
{
public:
	Mushdoom();
	~Mushdoom();

	void Awake() override;

	void Start() override;
	void Update() override;

	void OnInspector(ImGuiContext*) override;
	void SetProperties(EnemyController* original_enemy) override;

	void Seek(const float3& velocity) const;
	void Avoid(const float3& velocity) const;
	void Strafe(const float3& target_position, const float direction) const;

	void BattleCircleAI();
	void PursueAI();
	void CancelAttack();
	void LeaveAttackRange();
	void KnockBack() override;

	void TakeDamage(float damage, const float3& damage_source_position, float knockback = 0.f) override;
	void Kill() override;
	void ResetEnemy() override;

	void RotateTowardsPlayer() const;

	void NewThrow();
	void CalculateSparation(float separation);
	void ResetMovement();

	void HitSFX(NodeID attack) override;

private:
	void SpawnJump() const;
	void SpawnMovement(const float3& velocity) const;
	void CreateBattleCircle();

public:
	typedef EnemyState<Mushdoom> MushdoomState;
	MushdoomState* current_state = nullptr;
	MushdoomState* previous_state = nullptr;
	MushdoomState* idle_state = nullptr;
	MushdoomState* scream_state = nullptr;
	MushdoomState* pursue_state = nullptr;
	MushdoomState* attack_state = nullptr;
	MushdoomState* die_state = nullptr;
	MushdoomState* hit_state = nullptr;
	MushdoomState* throw_state = nullptr;

	MushdoomBall* mushdoom_ball = nullptr;
	ComponentParticleSystem* throw_effect = nullptr;
	ComponentParticleSystem* ball_die_effect = nullptr;

	float3 current_hit_pos = float3::zero;
	float3 target_hit_pos = float3::zero;

	int current_time_to_throw = 0;

	bool engage_player = false;
	bool leave_attack_range = false;
	bool is_throwing = false;
	bool is_moving = false;
	bool ranged_attack = false;

	mutable bool is_spawning = false;
	mutable bool has_ended_spawn = false;

private:
	float2 throw_delay_limits = float2::zero;

	float danger_distance = 50.f;
	float strafe_distance = 8.f;
	float separation_distance = 2.25f;
	float attack_ranged_damage = attack_damage * 2;
	float attack_range_offset = 2;
	float throw_delay = 2.f;
	float separation_offset = 0.5f;
	float knockback_divider = 20.f;

	mutable float3 final_spawn_position = float3::zero;
	mutable float3 initial_spawn_position = float3::zero;
	mutable float current_spawning_time = 0.0f;

	const float MAX_SPAWN_TIME = 400.f;
	float smooth_rotation = 0.01f;

	float3 target = float3::zero;
	float3 position = float3::zero;
	float3 desired_velocity = float3::zero;

	float distance = 0.0f;
};
extern "C" SCRIPT_API Mushdoom* MushdoomDLL(); //This is how we are going to load the script
#endif