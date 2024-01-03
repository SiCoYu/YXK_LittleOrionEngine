#ifndef  __ENVIRONMENTSFX_H__
#define  __ENVIRONMENTSFX_H__

#include "Script.h"

class ComponentAudioSource;

class EnvironmentSFX : public Script
{

public:

	EnvironmentSFX();
	~EnvironmentSFX() = default;

	void Awake() override;

	void OnInspector(ImGuiContext*) override;

	void PlayEventSFX();

private:
	void GetSFX();

private:
	std::vector<ComponentAudioSource*> sfx;
};

extern "C" SCRIPT_API EnvironmentSFX* EnvironmentSFXDLL(); //This is how we are going to load the script

#endif