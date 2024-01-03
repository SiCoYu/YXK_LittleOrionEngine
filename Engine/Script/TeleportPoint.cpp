#include "TeleportPoint.h"

#include "Component/ComponentScript.h"
#include "Component/ComponentTransform.h"

#include "Main/Application.h"
#include "Main/GameObject.h"
#include "Module/ModuleInput.h"
#include "Module/ModuleScene.h"

#include <imgui.h>

TeleportPoint* TeleportPointDLL()
{
	TeleportPoint* instance = new TeleportPoint();
	return instance;
}

TeleportPoint::TeleportPoint()
{

}


// Use this for showing variables on inspector
void TeleportPoint::OnInspector(ImGuiContext* context)
{
	//Necessary to be able to write with imgui
	ImGui::SetCurrentContext(context);
	ShowDraggedObjects();
	ImGui::DragInt("Spawn Zone", &zone_to_spawn);
}


//Use this for linking GO AND VARIABLES automatically if you need to save variables 
 void TeleportPoint::Save(Config& config) const
 {
 	config.AddInt(zone_to_spawn, "SpawnZone");
 	Script::Save(config);
 }

 //Use this for linking GO AND VARIABLES automatically
 void TeleportPoint::Load(const Config& config)
 {
 	zone_to_spawn = config.GetInt("SpawnZone", 0);
 	Script::Load(config);
 }