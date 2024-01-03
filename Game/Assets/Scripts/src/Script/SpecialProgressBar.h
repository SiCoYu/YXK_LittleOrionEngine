#ifndef  __SPECIALPROGRESSBAR_H__
#define  __SPECIALPROGRESSBAR_H__

#include "Script.h"

class ComponentTransform2D;

class SpecialProgressBar : public Script
{
public:
	SpecialProgressBar();
	~SpecialProgressBar() = default;

	void Awake() override;
	void Start() override;
	void Update() override;

	void SetProgress(float percentage);

	void OnInspector(ImGuiContext*) override;
	void InitPublicGameObjects();
	//void Save(Config& config) const override;
	//void Load(const Config& config) override;

private:
	ComponentTransform2D* special_bar_orb_1_transform = nullptr;
	ComponentTransform2D* special_bar_orb_2_transform = nullptr;
	ComponentTransform2D* special_bar_orb_3_transform = nullptr;
	ComponentTransform2D* special_bar_orb_4_transform = nullptr;

	float original_size = 0.f;
};
extern "C" SCRIPT_API SpecialProgressBar* SpecialProgressBarDLL(); //This is how we are going to load the script
#endif