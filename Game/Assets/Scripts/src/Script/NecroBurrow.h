#ifndef  __NECROBURROW_H__
#define  __NECROBURROW_H__

#include "Script.h"

class ComponentAudioSource;
class ComponentBillboard;
class ComponentParticleSystem;

class NecroBurrow : public Script
{
public:
	NecroBurrow();
	~NecroBurrow() = default;

	void Awake() override;
	void Start() override;
	void Update() override;

	void Show();
	void Hide();

	void OnInspector(ImGuiContext*) override;

private:
	ComponentBillboard* burrow_billboard = nullptr;
	ComponentParticleSystem* dirt_1_particles = nullptr;
	ComponentParticleSystem* dirt_2_particles = nullptr;
	ComponentParticleSystem* dust_1_particles = nullptr;

	ComponentAudioSource* burrow_source = nullptr;
	bool sound_already_played = false;

	float current_time = 0.f;
	float transition_time = 500.f;

	bool showing = false;
	bool hiding = false;
};
extern "C" SCRIPT_API NecroBurrow* NecroBurrowDLL(); //This is how we are going to load the script
#endif