#ifndef  __PROGRESSBAR_H__
#define  __PROGRESSBAR_H__

#include "Script.h"

class ComponentTransform2D;

class ProgressBar : public Script
{
public:
	ProgressBar();
	~ProgressBar() = default;

	void Awake() override;
	void Start() override;
	void Update() override;

	void OnInspector(ImGuiContext*) override;
	
	void SetProgress(float new_progress);

	void Save(Config& config) const override;
	void Load(const Config& config) override;

	float progress = 1.f;

private:
	bool inverted = false;
	bool vertical = false;

	float3 rotation = float3::zero;

	GameObject* fill_bar_game_object = nullptr;

	ComponentTransform2D* fill_bar_transform = nullptr;
	ComponentTransform2D* progress_bar_transform = nullptr;

};
extern "C" SCRIPT_API ProgressBar* ProgressBarDLL(); //This is how we are going to load the script
#endif