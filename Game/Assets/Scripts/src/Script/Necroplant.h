#ifndef  __NECROPLANT_H__
#define  __NECROPLANT_H__

#include "EnemyController.h"

#include <array>

class ComponentAudioSource;
class ComponentTransform;
class EventManager;
class NecroBurrow;
class NecroMuzzle;
class NecroProjectile;


class Necroplant : public EnemyController
{
public:
	enum class NecroplantState
	{
		IDLE,
		AIMING,
		NORMAL_ATTACKING,
		BARRAGE_ATTACKING,
		CONE_ATTACKING,
		DYING,
		EMERGING,
		BURROWING
	};

	struct Necrospot
	{
		float3 position;
		bool ocupied = false;
	};

	Necroplant();
	~Necroplant() = default;

	void Awake() override;
	void Start() override;
	void Update() override;

	void TakeDamage(float damage, const float3& damage_source_position, float knockback = 0.f) override;
	void ResetEnemy() override;
	void Kill() override;

	void LookAtCurrentTarget();
	void SpawnProjectile(const float3& direction);

	void OnInspector(ImGuiContext* context) override;

	void HitSFX(NodeID attack) override;

private:
	void AimingState();
	void NormalAttackState();
	void BarrageAttackState();
	void ConeAttackState();
	void EmergingState();
	void BurrowingState();

	Necrospot* GetAvailableNecrospot() const;

private:
	EventManager* event_manager = nullptr;

	bool show_debug_draws = false;
	GameObject* necroplant_upper_body = nullptr;
	ComponentAnimation* necroplant_roots_animation = nullptr;

	int time_since_last_attack = 0;
	int attack_cooldown = 1500;

	int num_attacks = 0;
	bool already_attacked_in_iteration = false;
	
	float angle_cone_attack = 20.f;

	ComponentTransform* muzzle_transform = nullptr;
	NecroMuzzle* necro_muzzle = nullptr;
	std::array<NecroProjectile*, 4> necroprojectiles;

	Necrospot* necro_spot = nullptr;

	ComponentAudioSource* necro_source = nullptr;

	NecroBurrow* necro_burrow = nullptr;
	float current_burrowing_time = 0.f;
	float burrowing_time = 750.f;
	float current_emerging_time = 0.f;
	float emerging_time = 750.f;
	float danger_radius = 4.f;
	bool burrowed_recently = false;

	NecroplantState current_state;
	std::string state_name = "Idle";
};
extern "C" SCRIPT_API Necroplant* NecroplantDLL(); //This is how we are going to load the script
#endif