#ifndef  __CHECKPOINTMANAGER_H__
#define  __CHECKPOINTMANAGER_H__


#include "Script.h"

class CameraController;
class ComponentCollider;
class CEnemyManager;
class CEventManager;
class PlayerController;
class SceneCamerasController;
class TeleportPoint;
class WorldManager;

struct Player;


class CheckPointManager : public Script
{
public:

	CheckPointManager();
	~CheckPointManager() = default;

	void Awake() override;
	void Start() override;

	void RespawnOnLastCheckPoint();

	TeleportPoint* GetClosestCheckpoint(int current_zone) const;

public:
	int last_checkpoint_index = 0; 

	TeleportPoint* last_checkpoint;

private:
	CameraController* camera_controller = nullptr;
	CEventManager* event_manager = nullptr;
	Player* player_1 = nullptr;
	Player* player_2 = nullptr;
	WorldManager* world = nullptr;

	float3 teleport_offset = float3(2.f, 0.f, 2.f);
	std::vector<TeleportPoint*> checkpoints;

};

extern "C" SCRIPT_API CheckPointManager* CheckPointManagerDLL(); //This is how we are going to load the script
#endif