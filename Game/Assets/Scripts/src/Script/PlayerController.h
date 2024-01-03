#ifndef  __PLAYERCONTROLLER_H__
#define  __PLAYERCONTROLLER_H__

#include "Script.h"

class Boss;
class ComponentAnimation;
class ComponentAudioSource;
class ComponentBillboard;
class ComponentCollider;
class ComponentMeshRenderer;
class ComponentParticleSystem;
class ComponentTrail;

class DebugModeScript;
class HitBlinker;
class InputManager;
class LightShield;
class PlayerAttack;
class PlayerMovement;
class ProgressBar;
class UIManager;
class WorldManager;

struct Player;
enum class EnemyType;

class PlayerController : public Script
{
public:
	enum class PlayerState {
		ALIVE,
		DYING,
		DEAD,
		REVIVING
	};

	enum class WhichPlayer
	{
		PLAYER1,
		PLAYER2
	};

	PlayerController();
	~PlayerController() = default;

	void Awake() override;
	void Start() override;
	void Update() override;

	void OnInspector(ImGuiContext*) override;

	void Save(Config& config) const override;
	void Load(const Config& config) override;

	void TakeDamage(const float damage, ComponentBillboard* vfx);
	void TakeDamage(const float damage);

	void Heal(const float healed_value);
	void ResetStun();
	void ResetKnockup();
	void EmitFromImpactPlayerBallMushdoom() const ;
	void EmitHealVFX();
	void InitVFX();
	ComponentCollider* GetCollider();
	void Stun(float time_stunned);
	void Knockup(float time_knocked, const float3& direction);

	void Respawn();
	bool IsShieldActive() const;

	void InterruptDash();

	void CheckLightShieldStatus() const;
	void Revive();

private:
	void Die();
	float CheckDistance() const;

	void UpdateHealthBar();

	bool CheckDashStatus();
	void Dash();


	//Update readability
	bool CheckStunStatus();
	bool CheckKnockupStatus();
	bool CheckDashBehaviour();
	void MoveAndAttack();
	void CheckAliveVFX();
	void CheckRespawning();
	void CheckDyingStatus();
	void CheckDeadBehaviour();
	void CheckRevivingBehaviour();


public:
	PlayerMovement* player_movement = nullptr;
	PlayerAttack* player_attack = nullptr;
	LightShield* light_shield = nullptr;
	ComponentAnimation* animation = nullptr;
	ComponentAudioSource* audio_source = nullptr;
	ComponentBillboard* hit_effect = nullptr;
	ComponentBillboard* hit_effect_biter = nullptr;
	ComponentBillboard* small_texture_DB_VFX = nullptr;
	ComponentBillboard* smoke_texture_DB_VFX = nullptr;
	ComponentBillboard* big_texture_DB_VFX = nullptr;
	ComponentBillboard* revive_spiral_VFX = nullptr;
	ComponentBillboard* stun_VFX = nullptr;
	ComponentMeshRenderer* sphere_mesh_render = nullptr;
	ComponentParticleSystem* splash_musdhoom_range_VFX = nullptr;
	ComponentParticleSystem* heal_VFX = nullptr;
	ComponentParticleSystem* sparkle_dead_ball_VFX = nullptr;
	ComponentParticleSystem* revive_VFX = nullptr;
	ComponentTrail* dash_trail_VFX = nullptr;
	ComponentTrail* dead_ball_trail_VFX = nullptr;

	float health_points;
	WhichPlayer player = WhichPlayer::PLAYER1;

	bool is_alive = true;
	bool dying = false;
	
	//BOSS
	bool is_boss = false;
	Boss* boss = nullptr;

	//CONTROL
	bool is_attacking = false;
	bool can_move = true;
	bool is_dashing = false;
	bool is_using_teleport = false;
	bool is_reviving = false;

	//CC
	bool is_stunned = false;
	bool is_knocked_up = false;
	float total_time_stunned = 0.f;
	float current_time_stunned = 0.f;

	float current_time_knocked = 0.f;
	float knockup_speed = 8.f;
	float knockback_speed = 2.f;
	float3 knockback_direction = float3::zero;

private:
	//THIS
	PlayerState current_state = PlayerState::ALIVE;
	ComponentCollider* collider = nullptr;

	Player* other_player = nullptr;
	unsigned int other_player_id = 1.f;
	WorldManager* world = nullptr;
	InputManager* input_manager = nullptr;
	HitBlinker* hit_blinker = nullptr;

	UIManager* ui_manager = nullptr;
	DebugModeScript* debug = nullptr;

	GameObject* dead_indicator = nullptr;

	//Dash
	float dash_time = 200.f;
	float current_dash_time = 0.0f;
	float dash_cooldown = 1000.f;
	float current_dash_cooldown = 0.0f;
	float dash_speed = 30.f;
	float dash_slide = 3.f;
	float3 dash_direction = float3::zero;

	//Teleport
	float teleport_cooldown = 0.0f;

	//Health
	float total_health = 400.f;
	float death_timer = 0.0f;
	const float death_time_before_death = 3000.0f;

	//Heal
	float current_time_healing = 0.0F;
	float heal_time = 1000.0F;
	bool heal_vfx_active = false;

	//Special Points
	float special_points = 0.f;
	float total_special_points = 20.f;

	//While Dead
	float current_distance = 0.0f;
	float dead_follow_range = 2.f;
	float dead_speed = 1.f;
	float3 dead_velocity = float3::zero;
	float slow_distance = 2500.f;
	bool button_showed = false;

	//Revive
	float revive_range = 2.f;
	float revive_time = 2200.f;
	float current_time = 0.f;
	GameObject* revive_bar_go = nullptr;
	ProgressBar* revive_bar = nullptr;
	int counter = 0;

	float left_trigger = 0.0f;
	float right_trigger = 0.0f;

	float2 left_axis = float2::zero;

	//Invincible when spawning
	bool spawning = false;
	const float time_spawning = 3000.f;
	float current_time_spawning = 0.f;

};
extern "C" SCRIPT_API PlayerController* PlayerControllerDLL(); //This is how we are going to load the script
#endif