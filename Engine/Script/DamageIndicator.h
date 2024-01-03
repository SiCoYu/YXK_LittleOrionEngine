#ifndef  __DAMAGEINDICATOR_H__
#define  __DAMAGEINDICATOR_H__

#include "Script.h"

class ComponentText;

class DamageIndicator : public Script
{
public:
	enum class DamageIndicatorState
	{
		APPEARING,
		MOVING,
		DEAD
	};

	DamageIndicator();
	~DamageIndicator() = default;

	void Awake() override;
	void Start() override;
	void Update() override;

	void OnInspector(ImGuiContext*) override;
	void InitPublicGameObjects();
	void Save(Config& config) const override;
	void Load(const Config& config) override;

	bool IsAlive() const;
	void Spawn(int number, float power, float2 position, const float2& direction);

private:
	DamageIndicatorState state = DamageIndicator::DamageIndicatorState::DEAD;

	ComponentText* damage_indicator_text = nullptr;

	float2 speed;
	
	float4x4 spatial_transformation;

	float initial_scale = 0.25;
	float target_scale = 0.125;
	float variable_scale = 0.f;
	float scale_duration = 100.f;

	float gravity = 1250.f;
	float distance = 50.f;
	float movement_duration = 500.f;

	float current_time = 0.f;

	float offset_y = 40.f;
};
extern "C" SCRIPT_API DamageIndicator* DamageIndicatorDLL(); //This is how we are going to load the script
#endif