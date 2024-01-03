#ifndef  __LIGHTORB_H__
#define  __LIGHTORB_H__

#include "Script.h"

class ComponentAudioSource;
class ComponentBillboard;
class ComponentMeshRenderer;
class ComponentTrail;
class WorldManager;


struct Player;

class LightOrb : public Script
{
public:
	enum class LightOrbLifeState
	{
		SPAWNING,
		WAITING,
		TRAVELLING,
		DEAD
	};

	LightOrb();
	~LightOrb() = default;

	void Awake() override;
	void Start() override;
	void Update() override;

	void OnInspector(ImGuiContext*) override;
	void Save(Config& config) const override;
	void Load(const Config& config) override;

	void SetMeshEnvironment(const std::vector<ComponentMeshRenderer*>& mesh_environment);
	std::vector<ComponentMeshRenderer*> GetMeshEnvironment() const;

	void Spawn(const float3& spawn_position);
	bool IsAlive() const;

	void CloneProperties(LightOrb* original_orb);

private:
	Player* GetClosestAlivePlayer() const;
	float3 GetClosestTerrainPosition(const float3& spawn_position) const;

private:
	WorldManager* world = nullptr;
	Player* player_to_deliver = nullptr;
	Player* player_1 = nullptr;
	Player* player_2 = nullptr;

	ComponentAudioSource* audio_source = nullptr;
	ComponentBillboard* small_texture_light_orb = nullptr;
	ComponentBillboard* shine_texture_light_orb = nullptr;
	ComponentBillboard* exterior_texture_light_orb = nullptr;
	ComponentTrail* trail_light_orb_VFX = nullptr;
	std::vector<ComponentMeshRenderer*> mesh_environment;

	float current_time = 0.f;
	LightOrbLifeState life_state = LightOrbLifeState::DEAD;

	float spawning_time = 500.f;
	const float speed = 500.f;
	const float GRAVITY = 0.00002f;
	float spread = 2.5f;
	float3 spawning_position = float3::zero;
	float3 initial_speed = float3::zero;

	float stil_time = 7500.f;
	float3 waiting_position = float3::zero;
	float oscillation_speed = 0.05f;
	float max_oscillation_height = 0.3f;

	float travel_time = 500.f;
	float attracting_radio = 3.5f;

	float healing_value = 7.f;
	float special_value = 0.5f;

	float ray_lenght = 50.f;
	float vertical_offset = 0.4f;
};
extern "C" SCRIPT_API LightOrb* LightOrbDLL(); //This is how we are going to load the script
#endif