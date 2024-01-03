#ifndef  __DEBUGMODESCRIPT_H__
#define  __DEBUGMODESCRIPT_H__

#include "Script.h"

class CameraController;
class ComponentCollider;
class CEnemyManager;
class CEventManager;
class InputManager;
class PlayerController;
class SceneCamerasController;
class TeleportPoint;
class WorldManager;
class AllyInteractionsScript;
struct Player;

class DebugModeScript : public Script
{
public:
	DebugModeScript();
	~DebugModeScript() = default;

	void Awake() override;
	void Start() override;
	void Update() override;
	void OnInspector(ImGuiContext*) override;

	void ShowDebugWindow();
	void DrawDebugInfo() const;
	void DrawDebugFeatures();
	void DrawEnemyOptions();
	void DrawPlayerOptions();

	void TeleportPlayers() const;
	void CheckTeleportStatus();
	void SpawnEnemies();

	void InstantKillPlayer(PlayerController* player_to_die);

public:
	bool debug_enabled = false;
	bool render_wireframe = false;

	//Debug Player
	bool disable_autoaim = false;
	bool is_player_invincible = false;
	bool is_player_god_mode = false;
	bool show_movement = false;

	bool show_biter_basic_attack = false;
	bool show_biter_charge_attack = false;
	bool show_biter_explosion_attack = false;

private:

	WorldManager* world = nullptr;
	InputManager* input_manager = nullptr;
	Player* player_1 = nullptr;
	Player* player_2 = nullptr;
	float instant_damage = 5000.f;

	//Debug Performance
	std::string base_str_tris = "#Tris: ";
	std::string base_str_verts = "#Verts: ";
	std::string base_str_fps = "FPS: ";
	float warp_cooldown = 0.0f;

	//Debug Features
	bool show_aabbtree = false;
	bool show_navmesh = false;
	bool show_quadtree = false;
	bool show_octtree = false;
	bool show_bounding_boxes = false;
	bool show_global_bounding_boxes = false;
	bool show_pathfind_points = false;
	bool show_physics = false;
	bool draw_camera_lines = false;
	bool trigger_roots = false;

	//Debug Teleport
	CEventManager* event_manager = nullptr;
	CameraController* camera_controller = nullptr;
	std::vector<TeleportPoint*> teleport_zones;
	GameObject* selected_teleport_zone = nullptr;
	TeleportPoint* selected_teleport_point = nullptr;
	ComponentCollider* collider_player1 = nullptr;
	ComponentCollider* collider_player2 = nullptr;
	float3 teleport_offset = float3(2.f, 0.f, 2.f);
	bool has_teleported_player_recently = false;
	AllyInteractionsScript* ally = nullptr;

	//Debug enemies
	CEnemyManager* enemy_manager;
	float3 spawn_offset = float3(4.f, 5.f, 0.f);
	bool spawn_enemy = false;
	bool spawn_mushdoom = false;
	bool spawn_biter = false;
	bool spawn_invincible = false;

};
extern "C" SCRIPT_API DebugModeScript* DebugModeScriptDLL(); //This is how we are going to load the script
#endif