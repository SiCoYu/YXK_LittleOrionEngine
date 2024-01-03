#ifndef  __CAMERACONTROLLER_H__
#define  __CAMERACONTROLLER_H__

#include "Script.h"

#include "MathGeoLib.h"

class CameraLine;
class ComponentCamera;
class DebugModeScript;
class PlayerController;
class WorldManager;

struct Player;


class CameraController : public Script
{
public:
	CameraController();
	~CameraController() = default;

	void Awake() override;
	void Start() override;
	void Update() override;

	void OnInspector(ImGuiContext*) override;

	void ActivePlayer();

	void Focus(const float3& position_to_focus);
	void RotateFocus(const float3& position_to_rotate) const;

	float3 GetPositionToProgress() const;
	GameObject* GetClosestPlayer() const;

	void UpdateCameraLines(CameraLine* next_camera_rail, CameraLine* next_camera_focus, CameraLine* next_level_path);

public:

	bool god_mode = false;
	bool freeze = false;
	bool out_platforms = false;
	bool fixed_position = false;
	float center_time_multiplayer = 10000.f;
	float center_time_singleplayer = 50.f;
	float center_time_position = 50.f;

	float max_distance_between_players = 25.f;
	bool debug_options = false;

	float progress_level = 0.f;

private:
	Player* player_1 = nullptr;
	Player* player_2 = nullptr;

	WorldManager* world_manager = nullptr;

	bool is_focusing = false;
	float current_time = 0.f;
	float focus_progress = 0.f;
	float focus_progress_rotation = 0.f;

	float smoothing_time = 0.004f;

	float previous_progress = 0.f;
	float3 previous_look_at = float3::zero;
	float3 current_look_at = float3::zero;
	float3 previous_position = float3::zero;
	float3 current_position = float3::zero;

	bool multiplayer = false;

	float3 freezed_position = float3::zero;

	CameraLine* camera_rail = nullptr;
	CameraLine* camera_focus = nullptr;
	CameraLine* level_path = nullptr;

};
extern "C" SCRIPT_API CameraController* CameraControllerDLL(); //This is how we are going to load the script
#endif