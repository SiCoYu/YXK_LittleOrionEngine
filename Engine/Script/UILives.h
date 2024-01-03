#ifndef  __UILIVES_H__
#define  __UILIVES_H__


#include "Script.h"

class ComponentCanvasRenderer;

class UILives : public Script
{
public:

	UILives();
	~UILives() = default;

	void Awake() override;
	void Start() override;
	void Update() override;

	void IncreaseLives();

	void DecreaseLives();
	void DeactiveAll() const;


	void OnInspector(ImGuiContext*) override;

private:
	std::vector<ComponentCanvasRenderer*> lives_indicator;

	int lives_index = 0;

};

extern "C" SCRIPT_API UILives* UILivesDLL(); //This is how we are going to load the script

#endif