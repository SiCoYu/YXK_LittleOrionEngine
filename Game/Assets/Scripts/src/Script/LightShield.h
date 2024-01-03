#ifndef  __LIGHTSHIELD_H__
#define  __LIGHTSHIELD_H__

#include "Script.h"

class LightShield : public Script
{
public:
	LightShield() = default;
	~LightShield() = default;

	void Awake() override;
	void Start() override;
	void Update() override;

	void OnInspector(ImGuiContext*) override;
	void InitPublicGameObjects();
	void CastShield();
	void UncastShield();
	void FillDurability();

	float BlockAttack(float damage);
	void BreakShield();
	void ShockWave();

	bool IsActive() const;
	void InitVFX();
	void ParryLightBall();
	void ResetParryTime();

public:
	bool active = false;
	bool broken = false;
	bool parry_enable = false;
	bool on_tutorial = false;

private:
	class UIManager* ui_manager = nullptr;
	class ComponentAnimation* animation = nullptr;
	class EnemyManager* enemy_manager = nullptr;
	class ComponentBillboard* break_shield_texture_DB_VFX = nullptr;
	class ComponentBillboard* small_texture_DB_VFX = nullptr;
	class ComponentBillboard* big_texture_DB_VFX = nullptr;
	class ComponentBillboard* smoke_texture_DB_VFX = nullptr;
	class PlayerController* player_controller = nullptr;
	class ComponentAudioSource* audio_source = nullptr;
	float durability = 100.f;
	float current_time_activated = 0.f;

	float current_parry_time = 500.f;
	float shield_radius = 10.f;
	const float breaking_shield_stun_time = 2000.f;
};
extern "C" SCRIPT_API LightShield* LightShieldDLL(); //This is how we are going to load the script
#endif