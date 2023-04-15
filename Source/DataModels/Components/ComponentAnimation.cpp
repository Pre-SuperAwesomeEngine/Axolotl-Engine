#include "Application.h"
#include "ComponentAnimation.h"
#include "ComponentTransform.h"
#include "Globals.h"

#include "Animation/AnimationController.h"

#include "FileSystem/Json.h"
#include "FileSystem/ModuleResources.h"

#include "GameObject/GameObject.h"

#include "debugdraw.h"

ComponentAnimation::ComponentAnimation(const bool active, GameObject* owner)
	: Component(ComponentType::ANIMATION, active, owner, false) 
{
	controller = new AnimationController();
	animationIx = 0;
}

ComponentAnimation::~ComponentAnimation()
{
}

AnimationController* ComponentAnimation::GetController()
{
	return controller;
}

void ComponentAnimation::SetAnimations(std::vector<std::shared_ptr<ResourceAnimation>> animations)
{
	this->animations = animations;
	animationIx = 0;

	controller->Play(animations[animationIx], true);
}

void ComponentAnimation::Update()
{
	controller->Update();

	if (animations.size() > 0)
	{
		std::list<GameObject*> children = owner->GetGameObjectsInside();

		for (auto child : children)
		{
			float3 pos;
			Quat rot;

			if (controller->GetTransform(&child->GetName()[0], pos, rot))
			{
				ComponentTransform* transform =
					static_cast<ComponentTransform*>(child->GetComponent(ComponentType::TRANSFORM));
				transform->SetPosition(pos);
				transform->SetRotation(float4x4(rot));
			}
		}
		static_cast<ComponentTransform*>(owner->GetComponent(ComponentType::TRANSFORM))->UpdateTransformMatrices();
	}
}

void ComponentAnimation::Draw()
{
	std::shared_ptr<ResourceModel> resource = owner->GetModel();

	DrawBones(resource->GetNodes());
}

void ComponentAnimation::DrawBones(GameObject* parent, GameObject* lastNonExtraNode) const
{
	ComponentTransform* parentTransform = 
		static_cast<ComponentTransform*>(parent->GetComponent(ComponentType::TRANSFORM));

	ComponentTransform* lastTransform =
		static_cast<ComponentTransform*>(lastNonExtraNode->GetComponent(ComponentType::TRANSFORM));

	std::vector<GameObject*> children = parent->GetChildren();
	for (GameObject* child : children)
	{
		if (child->GetName().find("$AssimpFbx$") == std::string::npos)
		{
			ComponentTransform* childTransform =
				static_cast<ComponentTransform*>(child->GetComponent(ComponentType::TRANSFORM));

			dd::line(childTransform->GetGlobalPosition(), lastTransform->GetGlobalPosition(), dd::colors::Blue);
			dd::axisTriad(childTransform->GetGlobalMatrix(), 0.1f, 2.0f);

			DrawBones(child, child);
		}
		else
		{
			DrawBones(child, lastNonExtraNode);
		}
		
	}
}

void ComponentAnimation::DrawBones(std::vector<ResourceModel::Node*> nodes) const
{
	std::map<std::string, ComponentTransform*> childrenTransforms;
	std::list<GameObject*> children = owner->GetGameObjectsInside();

	for (GameObject* child : children)
	{
		childrenTransforms[child->GetName()] = 
			static_cast<ComponentTransform*>(child->GetComponent(ComponentType::TRANSFORM));
	}
	
	for (ResourceModel::Node* node : nodes)
	{
		dd::axisTriad(childrenTransforms[node->name]->GetGlobalMatrix(), 0.1f, 2.0f);

		if (node->parent != -1)
		{
			ComponentTransform* parentTransform = childrenTransforms[nodes[node->parent]->name];

			dd::line(childrenTransforms[node->name]->GetGlobalPosition(), parentTransform->GetGlobalPosition(), 
				dd::colors::Blue);
		}
	}
}

void ComponentAnimation::SaveOptions(Json& meta)
{
	// Do not delete these
	meta["type"] = GetNameByType(type).c_str();
	meta["active"] = (bool)active;
	meta["removed"] = (bool)canBeRemoved;
}

void ComponentAnimation::LoadOptions(Json& meta)
{
	// Do not delete these
	type = GetTypeByName(meta["type"]);
	active = (bool)meta["active"];
	canBeRemoved = (bool)meta["removed"];
}