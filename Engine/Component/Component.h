#ifndef _COMPONENT_H_
#define _COMPONENT_H_

#include "Helper/Config.h"
//UGLY but needed for the moment
#include "ResourceManagement/Resources/Resource.h"
#include <pcg_basic.h>

class GameObject;

class Component
{
public:
	enum class ComponentType
	{
		AABB = 0,
		ANIMATION = 8,
		AUDIO_LISTENER = 20,
		AUDIO_SOURCE = 14,
		BILLBOARD = 16,
		CAMERA = 1,
		CANVAS = 5,
		COLLIDER = 9,
		EVENT_SYSTEM = 15,
		LIGHT = 4,
		MESH_RENDERER = 2,
		TRANSFORM = 3,
		TRANSFORM2D = 7,
		SCRIPT = 6,
		CANVAS_RENDERER = 10,
		UI_BUTTON = 11,
		UI_IMAGE = 12,
		UI_SPRITE_MASK = 18,
		UI_TEXT = 13,
		PARTICLE_SYSTEM = 17,
		TRAIL = 19,
		VIDEO_PLAYER = 21
	};

	Component() = default;
	Component(ComponentType componentType) : owner(owner), type(componentType), UUID(pcg32_random()) {};
	Component(GameObject * owner, ComponentType componentType) : owner(owner), type(componentType), UUID(pcg32_random()) {};
	virtual ~Component() = default;

	//Copy and move
	Component(const Component& component_to_copy) = default;
	Component(Component&& component_to_move) = default;

	virtual Component& operator=(const Component& component_to_copy)
	{
		this->active = component_to_copy.active;
		this->UUID = component_to_copy.UUID;
		return *this;
	};
	virtual Component& operator=(Component&& component_to_move)
	{

		this->active = component_to_move.active;
		this->UUID = component_to_move.UUID;
		component_to_move.UUID = 0;

		this->owner = component_to_move.owner;
		component_to_move.owner = nullptr;
		this->type = component_to_move.type;
		return *this;
	}
	virtual void Init() {};
	virtual void Enable() { active = true;};
	virtual void Disable() { active = false;};
	virtual bool IsEnabled() const { return active; };

	virtual void PreUpdate() {};
	virtual void Update() {};
	virtual void PostUpdate() {};

	virtual void Delete() = 0;
	virtual Component* Clone(GameObject* owner, bool original_prefab) = 0;
	void CloneBase(Component* component) const;
	virtual void CopyTo(Component* component_to_copy) const = 0;

	void Save(Config& config) const;
	void Load(const Config &config);

	virtual void SpecializedSave(Config& config) const = 0;
	virtual void SpecializedLoad(const Config &config) = 0;

	virtual void LoadResource(uint32_t uuid, ResourceType resource) {}
	virtual void LoadResource(uint32_t uuid, ResourceType resource, unsigned type) {}
	virtual void InitResource(uint32_t uuid, ResourceType resource) {}
	virtual void InitResource(uint32_t uuid, ResourceType resource, unsigned type) {}

	virtual void ReassignResource() {}

	virtual ComponentType GetType() const { return type; };
	bool Is2DComponent() const;


	static ComponentType GetComponentType(unsigned int component_type_uint)
	{
		return ComponentType(component_type_uint);
	}

	static std::string GetComponentTypeName(ComponentType component_type);

public:
	uint64_t UUID = 0;

	GameObject *owner = nullptr;
	ComponentType type;

	bool active = true;

	bool modified_by_user = false; //This is only for prefab and UI
	bool added_by_user = false; //This is only for prefab and UI
};

#endif //_COMPONENT_H_
