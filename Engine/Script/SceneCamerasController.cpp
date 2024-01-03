#include "SceneCamerasController.h"

#include "Component/ComponentScript.h"
#include "Component/ComponentTransform.h"
#include "Component/ComponentCamera.h"

#include "Main/Application.h"
#include "Main/GameObject.h"
#include "Module/ModuleInput.h"
#include "Module/ModuleScene.h"


#include "DebugModeScript.h"
#include "InputManager.h"

#include "imgui.h"



SceneCamerasController* SceneCamerasControllerDLL()
{
	SceneCamerasController* instance = new SceneCamerasController();
	return instance;
}

SceneCamerasController::SceneCamerasController()
{
	
}

// Use this for initialization before Start()
void SceneCamerasController::Awake()
{
	camera_list.push_back(game_camera);
	camera_list.push_back(god_camera);
	camera_list.push_back(camera_1);
	camera_list.push_back(camera_2);
	camera_list.push_back(camera_3);
	camera_rendering = (ComponentCamera*)game_camera->GetComponent(Component::ComponentType::CAMERA);

	ComponentScript* component_debug = debug->GetComponentScript("DebugModeScript");
	debug_mode = (DebugModeScript*)component_debug->script;

	GameObject* input_manager_go = App->scene->GetGameObjectByName("Input Manager");
	ComponentScript* input_manager_component = input_manager_go->GetComponentScript("InputManager");
	input_manager = static_cast<InputManager*>(input_manager_component->script);
}

// Use this for initialization
void SceneCamerasController::Start()
{

}

// Update is called once per frame
void SceneCamerasController::Update()
{
	if (App->input->GetKeyDown(KeyCode::Alpha3) || input_manager->GetControllerButtonDown(ControllerCode::LeftDpad, ControllerID::ONE) && debug_mode->debug_enabled)
	{
		if (index < camera_list.size() - 1) 
		{
			++index;
			UpdateCameraRendering();
		}	
	}
	if (App->input->GetKeyDown(KeyCode::Alpha2) || input_manager->GetControllerButtonDown(ControllerCode::RightDpad, ControllerID::ONE) && debug_mode->debug_enabled)
	{
		if (index > 0)
		{
			--index;
			UpdateCameraRendering();
		}
	}
}

// Use this for showing variables on inspector
void SceneCamerasController::OnInspector(ImGuiContext* context)
{
	//Necessary to be able to write with imgui
	ImGui::SetCurrentContext(context);
	ShowDraggedObjects();
}

void SceneCamerasController::UpdateCameraRendering()
{
	camera_rendering->Disable();
	camera_rendering = (ComponentCamera*)camera_list[index]->GetComponent(Component::ComponentType::CAMERA);
	camera_rendering->Enable();
}

void SceneCamerasController::SetMainCameraRendering()
{
	camera_rendering->Disable();
	camera_rendering = (ComponentCamera*)camera_list[0]->GetComponent(Component::ComponentType::CAMERA);
	camera_rendering->Enable();
	index = 0;
}


void SceneCamerasController::InitPublicGameObjects()
{
	//IMPORTANT, public gameobjects, name_gameobjects and go_uuids MUST have same size

	public_gameobjects.push_back(&game_camera);
	public_gameobjects.push_back(&god_camera);
	public_gameobjects.push_back(&camera_1);
	public_gameobjects.push_back(&camera_2);
	public_gameobjects.push_back(&camera_3);
	public_gameobjects.push_back(&debug);

	variable_names.push_back(GET_VARIABLE_NAME(game_camera));
	variable_names.push_back(GET_VARIABLE_NAME(god_camera));
	variable_names.push_back(GET_VARIABLE_NAME(camera_1));
	variable_names.push_back(GET_VARIABLE_NAME(camera_2));
	variable_names.push_back(GET_VARIABLE_NAME(camera_3));
	variable_names.push_back(GET_VARIABLE_NAME(debug));

	for (unsigned int i = 0; i < public_gameobjects.size(); ++i)
	{
		name_gameobjects.push_back(is_object);
		go_uuids.push_back(0);
	}
}