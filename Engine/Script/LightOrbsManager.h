#ifndef  __LIGHTORBSMANAGER_H__
#define  __LIGHTORBSMANAGER_H__

#include "Script.h"

#include <array>

class LightOrb;
class ComponentMeshRenderer;

class LightOrbsManager : public Script
{
public:
	LightOrbsManager();
	~LightOrbsManager() = default;

	void Awake() override;
	void Start() override;
	void Update() override;

	void OnInspector(ImGuiContext*) override;

	void SpawnOrbs(const float3& enemy_position, int number_orbs);

private:
	std::vector<ComponentMeshRenderer*> GetMeshCollider();
	std::array<LightOrb*, 30> light_orbs;

	GameObject* light_orb_game_object = nullptr;
};
extern "C" SCRIPT_API LightOrbsManager* LightOrbsManagerDLL(); //This is how we are going to load the script
#endif