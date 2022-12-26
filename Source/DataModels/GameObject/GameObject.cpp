#include "GameObject.h"
#include "Components/Component.h"
#include "ComponentTransform.h"
#include "ComponentMesh.h"
#include "ComponentMaterial.h"

#include <assert.h>

GameObject::GameObject(const char* name, GameObject* parent) : name(name), parent(parent)
{
	this->parent->children.push_back(this);

	Component* transform = new ComponentTransform(true, this);
	components.push_back(transform);
}

GameObject::~GameObject()
{
	components.clear();
	std::vector<Component*>().swap(components);

	children.clear();
	std::vector<GameObject*>().swap(children);
}

void GameObject::Update()
{
	if (active)
	{
		for (int i = 0; i < components.size(); i++)
		{
			components[i]->Update();
		}
	}
}

Component* GameObject::CreateComponent(ComponentType type)
{
	Component* newComponent = nullptr;

	switch (type)
	{
		case ComponentType::TRANSFORM:
		{
			newComponent = new ComponentTransform(true, this);
			break;
		}

		case ComponentType::MESH:
		{
			newComponent = new ComponentTransform(true, this);
			break;
		}

		case ComponentType::MATERIAL:
		{
			newComponent = new ComponentMaterial(true, this);
			break;
			
		}

		default:
			assert(false && "Unknown Component Type");
	}

	if (newComponent != nullptr)
		components.push_back(newComponent);

	return newComponent;
}