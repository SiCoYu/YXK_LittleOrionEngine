#ifndef  __HITBLINKER_H__
#define  __HITBLINKER_H__

#include "Script.h"

class ComponentMeshRenderer;
class Material;

class HitBlinker : public Script
{
public:
	HitBlinker();
	~HitBlinker() = default;

	void Awake() override;
	void Start() override;
	void Update() override;

	void Blink();
	void Reset();

	void OnInspector(ImGuiContext*) override;
	void Save(Config& config) const override;
	void Load(const Config& config) override;

public:
	float blink_time = 1000.f;
	float4 blink_color = float4(0.5f, 0.5f, 0.5f, 1.f);

private:
	ComponentMeshRenderer* mesh_renderer = nullptr;
	std::shared_ptr<Material> material = nullptr;

	bool blinking = false;
	float current_time = 0.f;
	bool pending_to_reset = false;
};
extern "C" SCRIPT_API HitBlinker* HitBlinkerDLL(); //This is how we are going to load the script
#endif