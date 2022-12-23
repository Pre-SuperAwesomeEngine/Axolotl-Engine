#pragma once

enum class ComponentType {TRANSFORM, MESH, MATERIAL};

class GameObject;

class Component
{
public:
	Component(const ComponentType type, const bool active, GameObject* owner);
	~Component();

	virtual void Enable();

	virtual void Update() = 0; // Pure Virtual because each component will perform a its own Update

	virtual void Disable();

private:
	ComponentType type;
	bool active;
	GameObject* owner;
};

inline void Component::Enable()
{
	active = true;
}

inline void Component::Disable()
{
	active = false;
}
