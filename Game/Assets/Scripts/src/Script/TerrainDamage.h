#ifndef  __TERRAINDAMAGE_H__
#define  __TERRAINDAMAGE_H__

#include "Script.h"

class ComponentCollider;
class EventManager;
class WorldManager;

struct Player;

enum class TerrainType
{
	CORRUPTED_WATER = 0,
	CORRUPTED_POISON = 1,
	MAGMA = 2
};

class TerrainDamage : public Script
{
public:
	TerrainDamage();
	~TerrainDamage() = default;

	void Awake() override;
	void Start() override;
	void Update() override;

	void OnInspector(ImGuiContext*) override;
	void InitPublicGameObjects();
	void Save(Config& config) const override;
	void Load(const Config& config) override;

	void ComputeDamage(unsigned player) const;

private:
	ComponentCollider* collider = nullptr;
	
	WorldManager* world = nullptr;
	EventManager* event_manager = nullptr;

	//Player's info
	Player* player_1 = nullptr;
	Player* player_2 = nullptr;

	TerrainType terrain_type = TerrainType::CORRUPTED_POISON;
	float poison_damage = 20.f; // each second
	float magma_damage = 50.f; // each second
	float water_damage = 2000.f;

	std::string terrain_name = "Poison";

	int zone = 0;

};
extern "C" SCRIPT_API TerrainDamage* TerrainDamageDLL(); //This is how we are going to load the script

#endif 