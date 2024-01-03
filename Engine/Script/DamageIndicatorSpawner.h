#ifndef  __DAMAGEINDICATORSPAWNER_H__
#define  __DAMAGEINDICATORSPAWNER_H__

#include "Script.h"

#include <array>

class DamageIndicator;

class DamageIndicatorSpawner : public Script
{
public:
	DamageIndicatorSpawner();
	~DamageIndicatorSpawner() = default;

	void Awake() override;
	void Start() override;
	void Update() override;

	void OnInspector(ImGuiContext*) override;
	void InitPublicGameObjects();
	//void Save(Config& config) const override;
	//void Load(const Config& config) override;

	void SpawnDamageIndicator(int damage, const float3& receiver_position, const float3& agressor_position);
	DamageIndicator* GetAvailableDamageIndicator();

private:
	std::array<DamageIndicator*, 20> damage_indicators;
};
extern "C" SCRIPT_API DamageIndicatorSpawner* DamageIndicatorSpawnerDLL(); //This is how we are going to load the script
#endif