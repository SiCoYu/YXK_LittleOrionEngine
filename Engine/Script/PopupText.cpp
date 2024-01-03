#include "PopupText.h"

#include "Component/ComponentLight.h"
#include "Component/ComponentParticleSystem.h"
#include "Component/ComponentScript.h"
#include "Component/ComponentTransform.h"

#include "Log/EngineLog.h"
#include "Main/Application.h"
#include "Main/GameObject.h"
#include "Module/ModuleInput.h"
#include "Module/ModuleScene.h"

#include <imgui.h>

#include "DialogueManager.h"

PopupText* PopupTextDLL()
{
	PopupText* instance = new PopupText();
	return instance;
}

PopupText::PopupText()
{

}

// Use this for initialization before Start()
void PopupText::Awake()
{
	GameObject* dialogue_manager_go = App->scene->GetGameObjectByName("Dialogue Manager");
	dialogue_manager = static_cast<DialogueManager*>(dialogue_manager_go->GetComponentScript("DialogueManager")->script);

	ui_popup_go = App->scene->GetGameObjectByName("Popups UI")->children[index];
	sparkle = static_cast<ComponentParticleSystem*>(owner->children[0]->GetComponent(Component::ComponentType::PARTICLE_SYSTEM));
	light = static_cast<ComponentLight*>(owner->children[1]->GetComponent(Component::ComponentType::LIGHT));
}

// Use this for initialization
void PopupText::Start()
{
	sparkle->Stop();
	light->active = false;
}

// Use this for showing variables on inspector
void PopupText::OnInspector(ImGuiContext* context)
{
	//Necessary to be able to write with imgui
	ImGui::SetCurrentContext(context);

	ImGui::DragInt("Index", &index);

	if (ImGui::BeginCombo("Popup Text", pop_up_name.c_str()))
	{
		if (ImGui::Selectable("Popup Attack"))
		{
			pop_up_name = "Popup Attack";
		}

		if (ImGui::Selectable("Popup LightBall"))
		{
			pop_up_name = "Popup LightBall";
		}

		if (ImGui::Selectable("Popup LightOrbs"))
		{
			pop_up_name = "Popup LightOrbs";
		}

		if (ImGui::Selectable("Popup Jump"))
		{
			pop_up_name = "Popup Jump";
		}

		ImGui::EndCombo();
	}
}

bool PopupText::CheckPlayerDistance(const float3& player_position)
{
	return owner->transform.GetGlobalTranslation().Distance(player_position) <= range_detection;
}

void PopupText::ActivateEvent()
{
	current_pop_up = dialogue_manager->ShowPopup(pop_up_name);
	if(current_pop_up)
	{
		message_activated = true;
	}
}

bool PopupText::CheckEvent() const
{
	return message_activated;
}

void PopupText::EndEvent()
{
	//if(current_pop_up)
	//{
	//	dialogue_manager->DisablePopup(current_pop_up);
	//}
}

void PopupText::EnablePopUp()
{
	ui_popup_go->SetEnabled(true);
	light->active = true;
	sparkle->Play();
}

void PopupText::DisablePopUp()
{
	ui_popup_go->SetEnabled(false);
	sparkle->Stop();
	light->active = false;
}

bool PopupText::IsPopUpEnabled()
{
	return ui_popup_go->IsEnabled();
}

//Use this for linking GO AND VARIABLES automatically if you need to save variables 
 void PopupText::Save(Config& config) const
 {
	config.AddString(pop_up_name, "Popup Name");
	config.AddUInt(index, "IndexPopup");
 	Script::Save(config);
 }

 //Use this for linking GO AND VARIABLES automatically
 void PopupText::Load(const Config& config)
 {
	 config.GetString("Popup Name",pop_up_name,"");
	 index = config.GetUInt("IndexPopup", 0);
 	 Script::Load(config);
 }

 //Use this for linking JUST GO automatically 
 void PopupText::InitPublicGameObjects()
 { 
	 for (int i = 0; i < public_gameobjects.size(); ++i)
	 {
		 name_gameobjects.push_back(is_object);
		 go_uuids.push_back(0);
	 }
 }