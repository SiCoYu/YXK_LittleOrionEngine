#include "LightOrbsManager.h"

#include "Component/ComponentMeshRenderer.h"
#include "Component/ComponentScript.h"
#include "Component/ComponentTransform.h"

#include "Main/Application.h"
#include "Main/GameObject.h"
#include "Module/ModuleInput.h"
#include "Module/ModuleScene.h"

#include "LightOrb.h"

#include "imgui.h"


LightOrbsManager* LightOrbsManagerDLL()
{
	LightOrbsManager* instance = new LightOrbsManager();
	return instance;
}

LightOrbsManager::LightOrbsManager()
{
	
}

// Use this for initialization before Start()
void LightOrbsManager::Awake()
{
	std::vector<ComponentMeshRenderer*> mesh_collider = GetMeshCollider();
	light_orb_game_object = owner->children[0];
	LightOrb* light_orb = static_cast<LightOrb*>(light_orb_game_object->GetComponentScript("LightOrb")->script);
	light_orb->SetMeshEnvironment(mesh_collider);

	for (unsigned int i = 0; i < light_orbs.size(); ++i)
	{
		GameObject* light_orb_clone = App->scene->DuplicateGameObject(light_orb_game_object, owner);
		light_orbs[i] = static_cast<LightOrb*>(light_orb_clone->GetComponentScript("LightOrb")->script);
		light_orbs[i]->CloneProperties(light_orb);
		light_orb_clone->children[0]->SetEnabled(false);
	}

}

// Use this for initialization
void LightOrbsManager::Start()
{

}

// Update is called once per frame
void LightOrbsManager::Update()
{


}

void LightOrbsManager::SpawnOrbs(const float3& enemy_position, int number_orbs)
{
	unsigned int current_spawned = 0;
	unsigned int i = 0;

	while (i < light_orbs.size())
	{
		if (!light_orbs[i]->IsAlive())
		{
			light_orbs[i]->Spawn(enemy_position);
			++current_spawned;

			if (current_spawned == number_orbs)
			{
				return;
			}
		}

		++i;
	}
}

// Use this for showing variables on inspector
void LightOrbsManager::OnInspector(ImGuiContext* context)
{
	//Necessary to be able to write with imgui
	ImGui::SetCurrentContext(context);
	ShowDraggedObjects();
}

std::vector<ComponentMeshRenderer*>  LightOrbsManager::GetMeshCollider()
{
	std::vector<GameObject*> meshes = App->scene->GetGameObjectsWithTag("EnvironmentCollider");
	std::vector<ComponentMeshRenderer*> mesh_collider;

	for (auto& mesh : meshes) 
	{
		mesh_collider.emplace_back(static_cast<ComponentMeshRenderer*>(mesh->GetComponent(Component::ComponentType::MESH_RENDERER)));
	}

	return mesh_collider;
}