#ifndef  __EVENTMANAGER_H__
#define  __EVENTMANAGER_H__

#include "Script.h"

class AudioManager;
class BattleEvent;
class CameraController;
class CameraLine;
class CameraShake;
class CinematicCamera;
class CheckPointManager;
class ComponentAnimation;
class ComponentAudioSource;
class ComponentCollider;
class ComponentParticleSystem;
class Dissolver;
class EnvironmentSFX;
class EnvironmentVFX;
class GameObject;
class CEnemyManager;
class PopupText;
class UIManager;
class WorldManager;

struct Player;

struct Zone
{
	Dissolver* dissolver;
	CameraLine* camera_rail;
	CameraLine* camera_focus;
	CameraLine* level_path;
	EnvironmentVFX* zone_vfx;
	EnvironmentSFX* zone_sfx;
};

class CEventManager : public Script
{
public:
	CEventManager();
	~CEventManager() = default;

	void Awake() override;
	void Start() override;
	void Update() override;

	void OnInspector(ImGuiContext*) override;
	void InitPublicGameObjects();

	void RestartEvents(unsigned int spawning_zone);
	void RestartObstacles();
	void EndEvent();
	void ComputeRootPosition();
	void DisablePopups();

	void GetZones();
	void InitCameraZones(const GameObject* go, Zone* zone);
	bool UpdateZone(int desired_zone);

private:
	void InitBattleEventZones();
	void GetPopupsEventZones();
	bool InitBattleEvent();

	void CheckBattleEvents();
	void CheckPopupTexts();
	void BattleEventStatus();

	bool CheckLose();
	bool CheckWin();

	void MoveRoot();

	void UpdateCamera() const;
	void UpdateBattleZone();

public:
	int current_zone = 0;
	BattleEvent* current_battle = nullptr;
	int enemies_killed_on_wave = 0;
	bool battle_event_triggered = false;
	bool tutorial_activate = false;
	bool popup_activated = false;

	//Boss
	bool boss_killed = false;

private:
	WorldManager* world = nullptr;
	AudioManager* audio_manager = nullptr;
	UIManager* ui_manager = nullptr;
	CEnemyManager* enemy_manager = nullptr;
	CameraController* camera_controller = nullptr;
	CameraShake* camera_shake = nullptr;
	CinematicCamera* cinematic_camera = nullptr;
	CheckPointManager* checkpoint_manager = nullptr;


	//Player's info
	Player* player_1 = nullptr;
	Player* player_2 = nullptr;

	ComponentParticleSystem* dust_roots_VFX = nullptr;
	ComponentAudioSource* dust_roots_SFX = nullptr;

	//Control Battle Wave's
	std::vector<BattleEvent*> battle_triggers;
	int current_event = 0;
	int current_event_beated = 0;

	//Control Win/Lose
	ComponentCollider* level_ending = nullptr;

	bool ending = false;

	//Dialogue
	std::vector<PopupText*> popup_triggers;
	std::vector<Zone*> zone_list;

	//Root events
	std::vector<GameObject*> root_obstacles;
	unsigned num_of_roots = 3;
	float total_time_moving_roots = 3000.f; //ms
	float current_time_moving_roots = 0.f; //ms
	float3 target_root_position = float3::zero;
	float3 current_root_position = float3::zero;
	const float root_distance_multiplier = 15.f;
	bool roots_moving = false;
};
extern "C" SCRIPT_API CEventManager* EventManagerDLL(); //This is how we are going to load the script
#endif