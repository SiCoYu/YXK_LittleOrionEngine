#include "Dissolver.h"

#include "Component/ComponentMeshRenderer.h"
#include "Component/ComponentScript.h"
#include "Component/ComponentTransform.h"

#include "Main/Application.h"
#include "Main/GameObject.h"
#include "Module/ModuleInput.h"
#include "Module/ModuleScene.h"
#include "Module/ModuleTime.h"

#include "ResourceManagement/Resources/Material.h"

#include <imgui.h>


Dissolver* DissolverDLL()
{
	Dissolver* instance = new Dissolver();
	return instance;
}

Dissolver::Dissolver()
{

}

// Use this for initialization before Start()
void Dissolver::Awake()
{
	for (auto& child : owner->children)
	{
		ComponentMeshRenderer* mesh_renderer = static_cast<ComponentMeshRenderer*>(child->GetComponent(Component::ComponentType::MESH_RENDERER));
		if (mesh_renderer != nullptr && mesh_renderer->material_to_render != nullptr 
			&& (mesh_renderer->material_to_render->material_type == Material::MaterialType::MATERIAL_DISSOLVING || mesh_renderer->material_to_render->material_type == Material::MaterialType::MATERIAL_LIQUID))
		{
			dissolving_materials.emplace_back(mesh_renderer->material_to_render);
		}
	}
}

// Use this for initialization
void Dissolver::Start()
{
	for (auto& material : dissolving_materials)
	{
		material->SetDissolveProgress(starting_dissolve_progress);
	}
}

// Update is called once per frame
void Dissolver::Update()
{
	if (dissolving)
	{
		current_time += App->time->delta_time;
		float progress = math::Min((float)current_time/dissolving_time * (1 - starting_dissolve_progress) + starting_dissolve_progress, 1.f);
		
		for (auto& material : dissolving_materials)
		{
			material->SetDissolveProgress(progress);
		}

		if (current_time > dissolving_time)
		{
			dissolving = false;
		}
	}
}

// Update is called once per frame
void Dissolver::SetStartingDissolveProgress(float dissolve_progress)
{
	starting_dissolve_progress = dissolve_progress;
}

void Dissolver::ResetDissolve()
{
	for (auto& material : dissolving_materials)
	{
		material->SetDissolveProgress(0.f);
	}
}

bool Dissolver::IsDissolving() const
{
	return dissolving;
}

void Dissolver::SetDissolvingTime(float time)
{
	dissolving_time = time;
}

void Dissolver::Dissolve()
{
	dissolving = true;
	current_time = 0;
}

// Use this for showing variables on inspector
void Dissolver::OnInspector(ImGuiContext* context)
{
	//Necessary to be able to write with imgui
	ImGui::SetCurrentContext(context);
	ShowDraggedObjects();
	ImGui::DragFloat("Starting Disolve Progress", &starting_dissolve_progress);
}

//Use this for linking JUST GO automatically 
void Dissolver::InitPublicGameObjects()
{
	//IMPORTANT, public gameobjects, name_gameobjects and go_uuids MUST have same size

	for (int i = 0; i < public_gameobjects.size(); ++i)
	{
		name_gameobjects.push_back(is_object);
		go_uuids.push_back(0);
	}
}

std::vector<std::shared_ptr<Material>> Dissolver::GetMaterials()
{
	return dissolving_materials;
}

void Dissolver::RestartDisolve()
{
	for (auto& material : dissolving_materials)
	{
		material->SetDissolveProgress(starting_dissolve_progress);
	}
}

//Use this for linking GO AND VARIABLES automatically if you need to save variables 
 void Dissolver::Save(Config& config) const
 {
 	config.AddFloat(starting_dissolve_progress, "StartingDissolveProgress");
 	Script::Save(config);
}

// //Use this for linking GO AND VARIABLES automatically
void Dissolver::Load(const Config& config)
{
	starting_dissolve_progress = config.GetFloat("StartingDissolveProgress", 0.f);
 	Script::Load(config);
}