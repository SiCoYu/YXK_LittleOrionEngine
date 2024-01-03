#ifndef  __CINEMATICCAMERA_H__
#define  __CINEMATICCAMERA_H__

#include "Script.h"
#include <vector>

class CameraLine;
struct CinematicAction;
class ComponentCamera;
class WorldManager;

class CinematicCamera : public Script
{
public:
	CinematicCamera();
	~CinematicCamera() = default;

	void Awake() override;
	void Start() override;
	void Update() override;

	void OnInspector(ImGuiContext*) override;
	void InitPublicGameObjects();
	void PlayCinematic(size_t index);
	bool IsPlayingCinematic() const;


private:
	void CreateCinematics();
	void SetMainCamera(bool cinematic) const;

private:
	std::vector<CinematicAction*> cinematic_queue;
	std::vector<std::vector<CinematicAction*>> cinematics;
	CinematicAction* current_action = nullptr;

	//Define Cinematics
	CameraLine* cinematic_rail = nullptr;
	GameObject* cinematics_go = nullptr;

	ComponentCamera* main_camera = nullptr;
	ComponentCamera* cinematic_camera = nullptr;
	WorldManager* world_manager = nullptr;

};
extern "C" SCRIPT_API CinematicCamera* CinematicCameraDLL(); //This is how we are going to load the script

#endif