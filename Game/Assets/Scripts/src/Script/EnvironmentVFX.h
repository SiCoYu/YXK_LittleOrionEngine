#ifndef  __ENVIRONMENTVFX_H__
#define  __ENVIRONMENTVFX_H__

#include "Script.h"

class ComponentParticleSystem;

class EnvironmentVFX : public Script
{

public:

	EnvironmentVFX();
	~EnvironmentVFX() = default;

	void Awake() override;

	void OnInspector(ImGuiContext*) override;

	void StartVFX();
	void StopVFX();

private:
	void GetVFX();

private:
	std::vector<ComponentParticleSystem*> vfx;
};

extern "C" SCRIPT_API EnvironmentVFX* EnvironmentVFXDLL(); //This is how we are going to load the script
#endif