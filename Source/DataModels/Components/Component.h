#pragma once

#include "../../FileSystem/UniqueID.h"

enum class ComponentType {/*MATERIAL,*/ MESHRENDERER, TRANSFORM, LIGHT, CAMERA, BOUNDINGBOX };

class GameObject;

class Component
{
public:
	Component(const ComponentType type, const bool active, GameObject* owner);
	virtual ~Component();

	bool Init();
	virtual void Update() = 0; // Pure Virtual because each component will perform its own Update

	virtual void Display() = 0; // Pure Virtual because each component will draw itself in the Inspector Window

	virtual void Enable();
	virtual void Disable();

	virtual void Draw();

	bool GetActive();
	ComponentType GetType();

	GameObject* GetOwner();
	const UID& GetUID() const;
private:
	ComponentType type;
	bool active;
	GameObject* owner;
	UID componentUID;
};

inline bool Component::Init()
{
	return true;
}

inline void Component::Enable()
{
	if (type != ComponentType::TRANSFORM)
		active = true;
}

inline void Component::Disable()
{
	if (type != ComponentType::TRANSFORM)
		active = false;
}

inline void Component::Draw()
{
}

inline bool Component::GetActive()
{
	return this->active;
}

inline ComponentType Component::GetType()
{
	return this->type;
}

inline GameObject* Component::GetOwner()
{
	return this->owner;
}

inline const UID& Component::GetUID() const
{
	return componentUID;
}
