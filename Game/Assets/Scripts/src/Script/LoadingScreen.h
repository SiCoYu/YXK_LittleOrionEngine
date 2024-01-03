#ifndef  __LOADINGSCREEN_H__
#define  __LOADINGSCREEN_H__

#include "Script.h"
#include "Animation/Tween/LOTween.h"
#include "Animation/Tween/Tween.h"
#include "Animation/Tween/TweenSequence.h"

class ComponentText;

class LoadingScreen : public Script
{
public:
	LoadingScreen();
	~LoadingScreen() = default;

	//void Awake() override;
	void Start() override;
	void Update() override;

	void OnInspector(ImGuiContext*) override;
	void InitPublicGameObjects();


private:
	ComponentText* percentatge_text = nullptr;
	ComponentText* hints_text = nullptr;
	TweenSequence* sequence = nullptr;
	std::vector<std::string> hints;
	float hints_buffer = 0.0f;
	const float hints_change_time = 10.0f;
	int current_hint_index = 0;

	void InitHints();
	void UpdateHints();

};
extern "C" SCRIPT_API LoadingScreen* LoadingScreenDLL(); //This is how we are going to load the script
#endif