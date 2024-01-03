#include "UILives.h"

#include "Component/ComponentCanvasRenderer.h"
#include "Component/ComponentScript.h"
#include "Component/ComponentTransform.h"

#include "Main/Application.h"
#include "Main/GameObject.h"
#include "Module/ModuleInput.h"
#include "Module/ModuleScene.h"

#include <imgui.h>


UILives* UILivesDLL()
{
	UILives* instance = new UILives();
	return instance;
}

UILives::UILives()
{

}

// Use this for initialization before Start()
void UILives::Awake()
{
	for (size_t i = 0; i < owner->children.size(); ++i)
	{
		lives_indicator.emplace_back(static_cast<ComponentCanvasRenderer*>(owner->children[i]->GetComponent(Component::ComponentType::CANVAS_RENDERER)));
	}
}

// Use this for initialization
void UILives::Start()
{
	DeactiveAll();
	lives_index = 0;
	lives_indicator[lives_index]->Enable();
}

// Update is called once per frame
void UILives::Update()
{

}

void UILives::IncreaseLives()
{
	--lives_index;
	if (lives_indicator.size() < lives_index)
	{
		DeactiveAll();
		lives_indicator[lives_index]->Enable();
	}
	else
	{
		++lives_index;
	}
}

void UILives::DecreaseLives()
{
	++lives_index;
	if (lives_indicator.size()>lives_index)
	{
		DeactiveAll();
		lives_indicator[lives_index]->Enable();
	}
	else if (lives_indicator.size() == lives_index)
	{
		DeactiveAll();
	}
	else
	{
		--lives_index;
	}
}

void UILives::DeactiveAll() const
{
	for (size_t i = 0; i < lives_indicator.size(); i++)
	{
		lives_indicator[i]->Disable();
	}
}

// Use this for showing variables on inspector
void UILives::OnInspector(ImGuiContext* context)
{
	//Necessary to be able to write with imgui
	ImGui::SetCurrentContext(context);
	ShowDraggedObjects();
}