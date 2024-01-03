#ifndef  __PLAYERATTACK_H__
#define  __PLAYERATTACK_H__

#include "Script.h"
#include "EnemyManager.h"
#include "Attacks.h"


class CameraShake;
class ComboCounter;
class Component;
class ComponentAnimation;
class ComponentAudioSource;
class ComponentCollider;
class ComponentParticleSystem;
class DebugModeScript;
class InputManager;
class PlayerMovement;
class UIManager;
class WorldManager;

struct Boss;

const float TIME_COMBO_OFFSET = 100.f;
const float DELTA_TIME_CONVERSION = 0.001f;

const float MAX_DAMAGE = 60.f;
const float MIN_DAMAGE = 20.f;

struct InputCombo
{
	//For now let's assume punch is 0 and kick is 1 and 2 if DOWN_KICK
	Move button;

	//We compute this window depending on the animation and an offset
	float time_of_combo;

	InputCombo() = default;
	InputCombo(Move p_button, float p_time_of_combo)
	{
		button = p_button;
		time_of_combo = p_time_of_combo;
	}
};


class PlayerAttack : public Script
{
public:
	PlayerAttack();
	~PlayerAttack() = default;

	void Awake() override;
	void Start() override;

	bool Attack(uint32_t player);
	void ComputeCollisions();
	void Hit(EnemyController * enemy);

	void OnInspector(ImGuiContext*) override;
	void InitPublicGameObjects();
	void Save(Config& config) const override;
	void Load(const Config& config) override;

	void UpdateAttackInputs();
	void CheckAttackInputs(int player);

	void ComputeRangeVectors();

	void GetAttackList();
	void InitAttackList();

	void MakeAttack();
	bool UpdateAttack(const Move input_to_find);

	bool EnemyOnRange(EnemyController* enemy) const;
	bool EnemyInFront() const;

	void InitVFX();
	void EmitVFX();
	void DisableVFX();

	void DisableTrails();
	bool GetCloserEnemyPosition(float3& closer_position);
	void AutoAimAttack();

	void CheckDirection();
	void RotateToAttackDirection();

	void SetBooleanPower(const Move input);

public:
	ComponentAudioSource* audio_source = nullptr;

	unsigned current_damage_power = 0;
	float trauma_multiplier = 0.4f;
	bool debug_collision_draw = false;

private:
	WorldManager* world_manager = nullptr;
	PlayerMovement* player_movement = nullptr;
	DebugModeScript* debug = nullptr;

	CameraShake* camera_shake = nullptr;
	ComboCounter* combo_counter = nullptr;
	CEnemyManager* enemy_manager = nullptr;
	ComponentAnimation* animation = nullptr;
	InputManager* input_manager = nullptr;
	UIManager* ui_manager = nullptr;

	Boss* boss = nullptr;
	bool is_boss = false;

	std::vector<InputCombo*> input_queue;

	bool attack_activated = false;
	float debug_damage = 50.f;
	float back_to_idle_percentatge = 0.85f;
	float down_tail_smash_anim_percentatge = 0.2f;

	bool on_state = false;
	bool percentatge = false;
	bool is_attacking = false;
	bool same_attack = false;
	bool trigger_audio = false;

	float anim_percentatge = 0.0f;
	NodeID current_id = NodeID::NO_COMBO;
	std::vector<float> attack_order;

	std::vector<Node*> attack_list;
	Node* current_attack = nullptr;
	float3 previous_position = float3::zero;
	float accumulated_distance = 0.f;
	float magnitude = 1.f;

	//Debug front collision
	float3 position = float3::zero;
	float3 front = float3::zero;
	float3 max_collision = float3::zero;
	float3 min_collision = float3::zero;

	float3 color_front = float3::unitZ;
	float3 color_angle = float3::unitY;
	float COLLISION_DISTANCE = 4.f;
	float maximum_dot_product = 0.35f;

	int player = 0;

	//Smooth direction when attacking
	float3 attack_direction = float3::zero;
	float smooth_autoaim_factor = 0.065f;
	float player_autoaim_range = 6.f;

	//VFX
	std::vector<Component*> vfx_attacks_container;
};
extern "C" SCRIPT_API PlayerAttack* PlayerAttackDLL(); //This is how we are going to load the script
#endif
