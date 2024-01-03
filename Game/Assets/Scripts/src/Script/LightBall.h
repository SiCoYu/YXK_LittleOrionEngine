#ifndef  __LIGHTBALL_H__
#define  __LIGHTBALL_H__

#include "Script.h"

class ComponentAudioSource;
class ComponentBillboard;
class ComponentCollider;
class ComponentLight;
class ComponentParticleSystem;
class ComponentTrail;

class ComboCounter;
class EnemyManager;
class EnemyController;
class InputManager;
class PlayerController;
class WorldManager;
class UIManager;


class AllyInteractionsScript;
struct Boss;
struct Player;

class LightBall : public Script
{
public:

	enum class LightBallState
	{
		UNNACTIVE,
		ACTIVATED,
		PROJECTILE,
		TRANSITION
	};

	struct TutorialCheckPoints
	{
		enum class CheckPointStatus{
			IN_PROGRESS,
			SUCCESS,
			FAIL
		};
		CheckPointStatus lightball_throw = CheckPointStatus::IN_PROGRESS;
		CheckPointStatus player1_bounce = CheckPointStatus::IN_PROGRESS;
		CheckPointStatus player2_bounce = CheckPointStatus::IN_PROGRESS;
		bool player1_is_owner = true;
		bool parry_enabled = false;
		size_t lightball_bounces = 0;
	};

	LightBall();
	~LightBall() = default;

	void Awake() override;
	void Start() override;
	void Update() override;

	void CheckTutorialBounceStatus(bool success);

	void OnInspector(ImGuiContext*) override;

	void ThrowLightBallToMainPlayer();

	void UpdateMarker() const;
	void DisableMarker() const;

	void InitPublicGameObjects();

	void ActiveReturn();



	//TODO Delete this when OnCollisionEnter implemented
	bool AlreadyHitted(EnemyController* enemy);

public:
	//Ally LightBall Tutorial
	bool ally_has_light_ball = false;

private:
	void SetStartingOwner();
	void BallMovingForward(const float3& target);
	void BallReturning(const float3& target);
	void DetectCollision();
	void CheckPlayerStatus();
	bool HasToBePassed() const;
	bool CanBePassed() const;

	bool CanBeActivated() const;
	void SetReturnAnimation() const;
	void UpdateCooldown();

	void SwapPlayersTarget();
	bool IsLightBallAvailable() const;
	void SetActiveVFX(bool active) const;
	void SetLightBallVFX();
	void SetSizeEffects(const float percentage) const;
	void SetEffectProperties() const;
	void IncreaseSizeEffects(float percentage) const;

	void RestartBall();
	void BallParryBehaviour();

private:
	InputManager* input_manager = nullptr;
	WorldManager* world_manager = nullptr;
	EnemyManager* enemy_manager = nullptr; 
	UIManager* ui_manager = nullptr;

	ComboCounter* combo_counter = nullptr; 

	ComponentAudioSource* audio_source = nullptr;
	ComponentBillboard* interior_texture_LB = nullptr;
	float width_interior_texture_LB = 0.0F, height_interior_texture_LB = 0.0F;
	ComponentBillboard* exterior_texture_LB = nullptr;
	float width_exterior_texture_LB = 0.0F, height_exterior_texture_LB = 0.0F;
	ComponentBillboard* small_texture_LB = nullptr;
	float width_small_texture_LB = 0.0F, height_small_texture_LB = 0.0F;
	ComponentLight* light_LB = nullptr;
	ComponentParticleSystem* blue_sparkle = nullptr;
	ComponentParticleSystem* purple_sparkle = nullptr;
	ComponentTrail* trail_light_ball_VFX = nullptr;

	//Player's references
	Player* player_1 = nullptr;
	Player* player_2 = nullptr;
	GameObject* player_1_hand = nullptr;
	GameObject* player_2_hand = nullptr;
	bool forced_pass = false;

	//Ball reference
	GameObject* ball = nullptr;

	//Boss reference
	Boss* boss = nullptr;
	bool is_boss = false;
	bool boss_hitted = false;

	GameObject* current_player_hand = nullptr;
	GameObject* current_player = nullptr;
	PlayerController* current_player_controller = nullptr;
	GameObject* next_target = nullptr;
	GameObject* next_target_hand = nullptr;
	PlayerController* next_target_controller = nullptr;

	//Behaviour
	LightBallState current_state = LightBallState::UNNACTIVE;
	float default_damage = 25.f;
	float damage = default_damage;
	float3 force_position = float3::zero;

	//Movement
	float distance = 0.f;
	float singleplayer_distance = 9.f;
	float multiplayer_distance = 6.f;
	float cast_distance = 1.2f;
	float slow_distance = 180.f;
	float shield_radius_range = 2.f; // For shield rebbound make ball faster
	float range = 1.0f;
	float default_speed = 0.5f; // For shield rebbound make ball faster
	float speed = default_speed;
	float default_maximum_time_to_return = 4000.f; // For shield rebbound make ball faster
	float maximum_time_to_return = default_maximum_time_to_return;
	float current_time = 0.f;
	float progress = 0.f;
	bool moving = false;
	bool ball_enabled = false;
	float hit_range = 2.f;
	float3 next_position = float3::zero;
	float3 next_direction = float3::zero;
	float3 current_velocity = float3::zero;
	float3 expected_velocity = float3::zero;
	float3 steering = float3::zero;

	//Scale Evo
	float3 cast_scale = float3(0.3f, 0.3f, 0.3f);
	float time_to_rescale = 0.f;
	float current_time_to_rescale = 0.f;
	float3 new_scale = float3::zero;

	//TODO Delete this when OnCollisionEnter implemented
	std::vector<EnemyController*>enemies_hitted;

	//Debug
	float ball_distance_magnitude = 0.f;
	bool casted = false;
	bool throwed = false;
	bool transition = false;

	//Cooldown
	float cooldown = 1000.f;
	float current_cooldown = 0.f;

	//UI Indicators
	GameObject* player_1_indicator = nullptr;
	GameObject* player_2_indicator = nullptr;

	//Delete when the animations and STM work better
	float frame_counter = 0.f;

	//Tutorial
	bool on_tutorial = false;
	TutorialCheckPoints shield_tutorial;
	friend AllyInteractionsScript;
};

extern "C" SCRIPT_API LightBall* LightBallDLL(); //This is how we are going to load the script
#endif