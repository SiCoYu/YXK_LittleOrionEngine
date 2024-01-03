#include "Script.h"

#include "EditorUI/Panel/InspectorSubpanel/PanelComponent.h"+
#include "Main/Application.h"
#include "Main/GameObject.h"
#include "Module/ModuleInput.h"
#include "Module/ModuleScene.h"

Script* ScriptDLL()
{
	return new Script();
}

Script::Script()
{
	panel = new PanelComponent();
}

void Script::AddReferences(GameObject *owner, Application *App)
{
	this->App = App;
	this->owner = owner;
}

void Script::Save(Config& config) const
{
	for (unsigned int i = 0; i < public_gameobjects.size(); ++i)
	{
		GameObject* go = (*public_gameobjects[i]);

		if (go != nullptr)
		{
			std::string aux_name = "Name";
			aux_name.append(std::to_string(i));
			config.AddString(name_gameobjects[i], aux_name);
			config.AddUInt(go->UUID, name_gameobjects[i]);
		}
	}
}

void Script::Load(const Config& config)
{
	for (unsigned int i = 0; i < public_gameobjects.size(); ++i)
	{
		std::string aux_name = "Name";
		aux_name.append(std::to_string(i));

		std::string name_go;
		config.GetString(aux_name, name_go, "Null");

		if (name_go != "Null")
		{
			go_uuids[i] = (config.GetUInt(name_go, 0));
		}

	}
}

void Script::Link()
{
	for (unsigned int i = 0; i < public_gameobjects.size(); ++i)
	{
		if (go_uuids[i] != 0)
		{
			(*public_gameobjects[i]) = App->scene->GetGameObject(go_uuids[i]);
			if ((*public_gameobjects[i]) == nullptr)
			{
				name_gameobjects[i] = "Missing GameObject";
			}
			else
			{
				name_gameobjects[i] = (*public_gameobjects[i])->name;
			}
		}
	}
}

void Script::ShowDraggedObjects()
{

	for (unsigned int i = 0; i < public_gameobjects.size(); ++i)
	{
		ImGui::Text(variable_names[i].c_str());
		ImGui::SameLine();
		ImGui::Button(name_gameobjects[i].c_str());
		panel->DropGOTarget(*(public_gameobjects[i]));
		if (*public_gameobjects[i])
			name_gameobjects[i] = (*public_gameobjects[i])->name;
	}
}

Script::~Script()
{
	//delete panel;
}

