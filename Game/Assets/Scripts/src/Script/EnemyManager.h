#ifndef  __ENEMYMANAGER_H__
#define  __ENEMYMANAGER_H__

#include "Script.h"

#include <list>

class EventManager;
class EnemyController;
class MushdoomBall;
class WorldManager;
class Mushdoom;

enum class EnemyType;

class EnemyManager : public Script
{
public:
	EnemyManager();
	~EnemyManager() = default;

	void Awake() override;
	void Start() override;
	void Update() override;

	void AddEnemy(EnemyController* enemy);
	void KillEnemy(EnemyController* enemy);

	void SpawnEnemy(EnemyType type, const float3& spawn_position, bool invincible = false);
	void SpawnWave(std::vector<float3>& spawns, unsigned enemies_per_wave, EnemyType enemy_type);
	void InitEnemies();

	void OnInspector(ImGuiContext*) override;
	void InitPublicGameObjects();
	bool ArePlayersInDanger() const;
	//void Save(Config& config) const override;
	//void Load(const Config& config) override;

	void CancelMeleeAttack(EnemyController* enemy);

	void CancelRangedAttack(EnemyController* enemy);
	void KillAllTheEnemies();

	void NewRequestAttack(EnemyController* enemy);
	bool IsEnemyAttacking(EnemyController* enemy);


private:
	bool CheckSpawnAvailability(float3& spawn_position);
	bool IsEnemyMeleeAttacking(EnemyController* enemy);
	bool IsEnemyRangedAttacking(EnemyController* enemy);


public:
	std::vector<EnemyController*> enemies;
	unsigned current_number_of_enemies_alive = 0;

private:
	//We need a reference to an existing mushdoom to duplicate_him
	GameObject* mushdoom_go = nullptr;
	EventManager* event_manager = nullptr;
	WorldManager* world_manager = nullptr;
	std::vector<std::pair<float3, EnemyType>> enemies_spawning_queue;
	unsigned total_enemies_killed = 0;
	int max_double_z_spawn = 5;

	size_t current_melee_mushdoom_attackers = 0;
	int simultaneous_attackers = 3;

	size_t current_ranged_mushdoom_attackers = 0;
	int simultaneous_ranged_attackers = 4;

	int mushdooms_to_spawn = 0;
	int biters_to_spawn = 0;

	const unsigned MAX_NUMBER_OF_ENEMIES = 20;
	const float3 graveyard_position = float3(100.f, 100.f, 100.f);
};
extern "C" SCRIPT_API EnemyManager* EnemyManagerDLL(); //This is how we are going to load the script
#endif