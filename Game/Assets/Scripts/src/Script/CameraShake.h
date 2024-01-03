#ifndef  __CAMERASHAKE_H__
#define  __CAMERASHAKE_H__

#include "Script.h"

class CameraShake : public Script
{
public:
	CameraShake();
	~CameraShake() = default;

	void Awake() override;
	void Start() override;
	void Update() override;

	void OnInspector(ImGuiContext*) override;
	void InitPublicGameObjects();
	void Save(Config& config) const override;
	void Load(const Config& config) override;

	float GetFloat(float seed) const;
	float3 GenerateFloat3();

public:
	bool shaking = true;
	float trauma = 0.f;

private:
	float trauma_multiplier = 16.f; //the power of the shake
	float trauma_magnitude = 0.8f;//the range of movement
	float trauma_rotation_magnitude = 0.f; //the rotational power
	float trauma_depth_magnitud = 0.6f; //the depth multiplier
	float trauma_decay = 1.3f; //how quickly the shake falls off

	float time_counter = 0.f; //counter stored for smooth transition


};
extern "C" SCRIPT_API CameraShake* CameraShakeDLL(); //This is how we are going to load the script
#endif