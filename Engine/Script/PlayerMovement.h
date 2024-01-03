#ifndef  __PLAYERMOVEMENT_H__
#define  __PLAYERMOVEMENT_H__

#include "Script.h"

#include "MathGeoLib.h"

class ComponentAnimation;
class ComponentAudioSource;
class ComponentCamera;
class ComponentCollider;
class ComponentMeshRenderer;
class ComponentParticleSystem;

class CameraController;
class DebugModeScript;
class CEnemyManager;
class InputManager;
class WorldManager;

struct RayHit 
{
	GameObject* game_object = nullptr;
	float hit_distance = 0.0f;
	float3 hit_point = float3::zero;
};

class PlayerMovement : public Script
{
public:
	PlayerMovement();
	~PlayerMovement() = default;

	void Awake() override;
	void Start() override;

	void OnInspector(ImGuiContext*) override;

	bool IsGrounded() const;

	void Move(int player, bool is_stuned = false);
	void MoveIntoDirection(float3 direction, float speed); // Direction should be different from zero!
	void GetInputDirection(int player_id, float3& my_direction);
	void RotatePlayer(const float3& direction_to_rotate);
	void RotatePlayer(const float3& direction_to_rotate, const float& smooth_rotation_factor);
	void SetHorizontalSpeed(float speed);
	void SlowPlayer(float percentatge);
	void BlockJump(bool block);
	void ResetKnockup();
	void ChangeDashStatus(bool is_dashing);

private:
	void ComputeHorizontalVelocity(float3 direction, float speed);
	float ComputeSpeedWithAcceleration(float final_speed, float current_speed, float dtime);
	float ComputeSpeedWithDeAcceleration(float final_speed, float current_speed, float dtime);
	void ComputeVerticalVelocity(float3 direction, float speed, bool apply_gravity);

	void SetNewPosition(const float3 & new_position) const;
	void ApplyVelocity();

	bool IsInside(const float3& transform) const;

	void InitPublicGameObjects();
	void Save(Config& config) const override;
	void Load(const Config& config) override;

	bool IsPlayerColliding(RayHit* collision, const LineSegment& ray, bool only_mesh = false);
	bool DetectCollisions(int collision_direction);
	bool EnemyInFront(const float3& enemy_position);
	

public:
	bool is_knocked = false;

private:
	bool jump_blocked = false;
	bool grounded = false;
	float gravity = 30.f;
	bool is_jumping = false;
	float jump_speed = 11.f;
	bool is_falling = false;

	float smooth_rotation = 0.012f;
	const float default_speed_horizontal = 10.f;
	float speed_horizontal = default_speed_horizontal;
	
	DebugModeScript* debug = nullptr;
	ComponentAudioSource* audio_source = nullptr;
	ComponentAnimation* animation = nullptr;

	GameObject* camera = nullptr;
	CameraController* camera_controller = nullptr;
	ComponentCamera* game_camera = nullptr;

	GameObject* other_player = nullptr;
	std::vector<GameObject*> mesh_collider;
	std::vector<ComponentMeshRenderer*> boss_zones;
	WorldManager* world = nullptr;
	CEnemyManager* enemy_manager = nullptr;
	InputManager* input_manager = nullptr;

	ComponentParticleSystem* player_footsteps = nullptr;

	float3 direction;
	float3 velocity;
	float acceleration = 0.06f;
	float deceleration = 0.06f;
	bool acceleration_once = false;
	bool is_dashing = false;
	
	//Debug
	bool is_inside = true;
	bool visualize_future_aabb = false;

	mutable float3 device_coordinates = float3::zero;

	//New Collision check
	//Horizontal
	float horizontal_threshold = 0.3f;
	RayHit* horizontal_ray_detector_middle = nullptr;
	float horizontal_ray_middle_position = 0.5f;
	float horizontal_ray_middle_length = 1.5f;

	RayHit* horizontal_ray_detector_knee = nullptr;
	float horizontal_ray_knee_position = 0.3f;
	float horizontal_ray_knee_length = 0.5f;

	//Vertical
	float vertical_offset_start = 0.3f;
	float vertical_offset_end = 0.f;
	float vertical_offset_minimum = 4.f;
	float vertical_offset_maximum = 100.f;

	RayHit* main_vertical_ray_detector = nullptr;
	float vertical_ray_main_position = 0.3f;
	bool main_vertical_ray = false;

	RayHit* border_vertical_ray_detector = nullptr;
	float vertical_ray_border_position = 0.3f;
	bool border_vertical_ray = false;
	float border_ray_offset = 0.4f;

	float minimum_y = -10.f;
	float vertical_threshold = 0.08f;

	//Rotation
	float rotation_angle = 0.3f;
	float rotation_threshold = 0.2f;
	float rotation_ray_length = 0.2f;

	//Diagonal
	RayHit* main_diagonal_ray_detector = nullptr;
	bool main_diagonal_ray = false;

	//Extra
	float range_collision = 5.f;

};
extern "C" SCRIPT_API PlayerMovement* PlayerMovementDLL(); //This is how we are going to load the script
#endif
