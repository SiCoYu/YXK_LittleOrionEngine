#ifndef  __BOSSCONTROLLER_H__
#define  __BOSSCONTROLLER_H__

#include "Script.h"

#include <stack>

#include "PlayerController.h"

class BattleEvent;
class BossStage;
class CameraController;
class CameraShake;
class ComponentAnimation;
class ComponentBillboard;
class ComponentMeshRenderer;
class ComponentParticleSystem;
class Dissolver;
class HitBlinker;
class CEventManager;
class ProgressBar;
class UIManager;
class WorldManager;

struct Player;

enum class BossBone
{
	HEAD,
	RIGHT_HAND,
	RIGHT_FOREARM,
	RIGHT_FINGER,
	LEFT_HAND,
	LEFT_FOREARM,
	LEFT_FINGER,
	CHEST, 
	NONE
};


enum class CrowdControl
{
	STUN,
	KNOCKUP,
	NONE
};


class BossController : public Script
{
public:
	BossController();
	~BossController() = default;

	struct Projectile
	{
		GameObject* projectile_go = nullptr;
		bool spawned = false;
		bool vfx_played = false;
		float remaining_time_alive = 0.f;
		float total_time_alive = 0.f;

		bool player1_already_hitted = false;
		bool player2_already_hitted = false;

		Projectile() = default;
		Projectile(GameObject* projectile_go)
		{
			this->projectile_go = projectile_go;
		}
	};

	struct VulnerableZone 
	{
		GameObject* zone_go = nullptr;
		BossBone bone = BossBone::NONE;
		bool vulnerable = false;
		bool active = false;

		VulnerableZone() = default;
		VulnerableZone(GameObject* zone)
		{
			this->zone_go = zone;
		}
	};

	void Awake() override;
	void Start() override;
	void Update() override;

	void OnInspector(ImGuiContext*) override;
	void InitPublicGameObjects();

	void RotateBoss(PlayerController::WhichPlayer player) const;
	void RotateBoss(const float3& direction_to_look) const;

	bool HitByBall(const float3& ball_position, float3& hit_position_object) const;
	bool HitByPlayer(const float3& player_position, const float3& player_front_vector, float range, float3& hit_position_object) const;

	void TakeDamage(float damage, const float3& damage_dealer_position, const float3& hitted_position_object);

	void Print(const char* text, ...) const;

	void ComputeIfSmashedPlayer(float damage, uint32_t which_player, CrowdControl cc = CrowdControl::NONE) const;

	//Sphere
	bool ComputeDangerZone(const float3& center, float radius, float damage, CrowdControl cc, uint32_t which_player = 0) const;

	//Box
	void ComputeDangerZone(const float3 points [8], float damage, CrowdControl cc = CrowdControl::NONE) const;

	//Circle
	bool ComputeDangerZoneCircle(const float3 & center, float radius, float damage, float lenght, CrowdControl cc, uint32_t which_player = 0) const;

	//Debug Draws
	void DrawSphere(const float3& center, float radius) const;
	void DrawBox(const float3 points[8]) const;
	void DrawCircle(const float3 & center, float radius) const;

	/** Check if animation have finished */
	bool IsAnimationOver();

	float3 GetPlayerPosition(size_t player);
	bool IsMultiplayer() const;
	void BossKilled();
	float3 GetCameraPosition() const;

	//Crystals and Rocks projectiles 
	void UpdateCrystals();
	void UpdateRocks();

	void SpawnCrystal(const float3& spawn_position);
	void SpawnRock(const float3& spawn_position, float exp_value);

	bool IsAvalancheDone() const;
	void FreezeCamera(bool state) const;
	bool CheckIfRockIsValid(const float3& rock_position) const;

	float GetRandom(float min, float max);
	
	BattleEvent* GetBattleEvent();

	//Trigger Animation
	void TriggerBossAnimation(const std::string& trigger);
	bool IsAnimationOnState(const std::string& state) const;
	float GetAnimationPercentage() const;
	GameObject* GetBoneGameObject(BossBone bone) const;
	float3 GetBonesGameObjectPosition(BossBone bone) const;

	///Effects
	//Slam
	void SpawnSlamEffect(BossBone bone);
	//GuidedEffect
	void SpawnGuidedEffect(const float3& position) const;
	void UnspawnGuidedEffect() const;
	//Earthquake
	void SpawnEarthquakeEffect();
	void UpdateEarthquakeEffect();
	//Avalanche
	void EmitAvalancheSmoke(const float3& position) const;
	//ShadowRock
	void SpawnShadowRock(const float3& position, size_t rock_index) const;
	void UpdateShadowRock(size_t rock_index) const;
	void UnSpawnShadowRock(size_t rock_index) const;

	//Dispersion
	void PlaySmokeDispersionEffect(const float3& position, float width, float height, size_t animation_time) const;
	void PlayRocksDispersionEffect(const float3& position, float width, float height, size_t animation_time) const;
	void PlayGreyEffect(const float3& position, float width, float heigth, float lifetime);

	//Camera shake
	void SetCameraTrauma(float trauma) const;

	//Set Vulnerable Zones up to
	void SetEnableVulnerableZone(const BossBone type, const bool vulnerable, const bool enable) const;

	//Sound
	void ActiveSound(BossBone bone, const std::string& event) const;
	void TriggerHitSound(BossBone bone) const;
	void TriggerMusic() const;
	void StopMusic() const;

	//Collider Wall
	void BlockInitialPath() const;

	//Players Spawning
	void PlayersSpawning();
	void CheckPlayersSpawning();

	bool IsBossObstacle() const;

	void CheckIfBossIsObstacle();

private:
	void InitPlayers();
	void InitStages();
	void InitVulnerableZones();
	VulnerableZone* SetVulnerableZones(const GameObject* bone, const BossBone type) const;
	void InitCrystals();
	void InitRocks();
	void InitEffects();
	float3 GetRotationDirection(unsigned player) const;
	void TransitionToNextStage();
	void InitBonesGameObjects();
	float3 ComputeKnockupDirection(const float3& damage_center, const float3& player_position) const;

	void ApplyCrowdControl(CrowdControl cc, const float3& damage_position, class PlayerController* player_controller) const;

	//Debug vulnerable bones
	std::string GetBossBoneName(BossBone bone) const;

public:
	float3 debug_float3_debug_variable = float3::zero;
	bool vulnerable = false;
	bool stunned = false;
	CEventManager* event_manager = nullptr;
	const float ground_y_position = 0.540f;
	bool waiting_for_spawning = false;

private:
	ComponentAnimation* animation = nullptr;
	//World Info
	WorldManager* world_manager = nullptr;
	ProgressBar* boss_health_bar = nullptr;
	UIManager* ui_manager = nullptr;
	CameraController* camera_controller = nullptr;
	CameraShake* camera_shake = nullptr;
	Player* player_1 = nullptr;
	Player* player_2 = nullptr;
	bool is_multiplayer = false;

	HitBlinker* hit_blinker = nullptr;

	//Boss Info Stages
	BossStage* current_stage = nullptr;
	std::stack<BossStage*> stages;
	
	//Boss Status
	float total_health = 2000.f;
	float current_health = total_health;
	float health_factor = 0.75f;
	float next_health_stage = total_health * health_factor;
	float is_alive = false;
	float die_time = 0.8f;
	float avalanche_trauma = 0.25f;

	//Boss Hit Positions
	std::vector<VulnerableZone*> vulnerable_zones;

	//Crystals
	std::vector<Projectile> crystals;
	std::vector<Projectile> rocks;

	//Slam Effect
	Dissolver* right_slam_dissolver = nullptr;

	//Guided Crystal Effects
	GameObject* guided_crystal_effect = nullptr;
	GameObject* guided_crystal_effect_player2 = nullptr;

	//Earthquake effect
	ComponentAnimation* earthquake_effect_animation = nullptr;
	ComponentMeshRenderer* earthquake_effect_renderer = nullptr;
	ComponentMeshRenderer* crack_effect = nullptr;

	//Rocks Effect Smoke
	std::vector<ComponentParticleSystem*> rock_crash_smoke_vfxs;

	//Rocks Shadow Effect
	std::vector<GameObject*> rocks_shadows_go;

	//Rocks and smoke dispersion
	ComponentBillboard* rocks_dispersion = nullptr;
	ComponentBillboard* smoke_dispersion = nullptr;

	std::vector<ComponentBillboard*> rocks_dispersion_vector;
	std::vector<ComponentBillboard*> smoke_dispersion_vector;

	//Health stage changes
	float change_stages_health[3]{ 2 * (total_health / 3), total_health/ 3, 0.f};
	unsigned int current_stage_index = 0;

	//WallCollider go
	GameObject* wall_collider = nullptr;

	//Camera position
	GameObject* camera_go = nullptr;

	//Bones go
	GameObject* right_hand_go = nullptr;
	GameObject* right_hand_finger = nullptr;
	GameObject* right_forearm_go = nullptr;
	GameObject* left_hand_go = nullptr;
	GameObject* left_hand_finger = nullptr;
	GameObject* left_forearm_go = nullptr;
	GameObject* head = nullptr;
	GameObject* chest = nullptr;

	//Players spawning
	float current_time_waiting_for_spanwing = 0.f;

	mutable bool boss_is_obstacle = false;
	float current_time_not_being_obstacle = 0.f;

};
extern "C" SCRIPT_API BossController* BossControllerDLL(); //This is how we are going to load the script
#endif