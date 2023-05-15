#include "ComponentBreakable.h"

#include "GameObject/GameObject.h"

#include "Components/ComponentRigidBody.h"

#include "Globals.h"
#include "FileSystem/Json.h"

ComponentBreakable::ComponentBreakable(bool active, GameObject* owner)
	: Component(ComponentType::BREAKABLE, active, owner, true)
{
	SubscribeToOnCollisionEnter();
}

ComponentBreakable::~ComponentBreakable()
{
}

void ComponentBreakable::Update()
{
	SubscribeToOnCollisionEnter();
}

void ComponentBreakable::SaveOptions(Json& meta)
{
	// Do not delete these
	meta["type"] = GetNameByType(type).c_str();
	meta["active"] = (bool)active;
	meta["removed"] = (bool)canBeRemoved;
	meta["subscribed"] = (bool)subscribed;
}

void ComponentBreakable::LoadOptions(Json& meta)
{
	// Do not delete these
	type = GetTypeByName(meta["type"]);
	active = (bool)meta["active"];
	canBeRemoved = (bool)meta["removed"];
	subscribed = (bool)meta["subscribed"];
}

void ComponentBreakable::SubscribeToOnCollisionEnter()
{
	if (subscribed)
	{
		ENGINE_LOG("Already suscribed");
		return;
	}
	if (auto rb = owner->GetComponent(ComponentType::RIGIDBODY))
	{
		//rb->AddCollisionEnterDelegate(&ComponentBreakable::OnCollisionEnter, this);
		subscribed = true;
	}
}

void ComponentBreakable::UnsubscribeToOnCollisionEnter()
{
	if (!subscribed)
	{
		ENGINE_LOG("Already unsuscribed");
		return;
	}
	if (auto rb = owner->GetComponent(ComponentType::RIGIDBODY))
	{
		static_cast<ComponentRigidBody*>(rb)->ClearCollisionEnterDelegate();
		subscribed = true;
	}
}

void ComponentBreakable::OnCollisionEnter(ComponentRigidBody* rigidbody)
{
	/*
Execercise:

- Only break if the other object is a projectile or if the other object is moving fast enough
- Apply a radial force to the pieces centered at the collision point
*/

	ENGINE_LOG("Breakable: Collision between %s and %s", owner->GetName(), rigidbody->GetOwner()->GetName());

	if (auto rb = owner->GetComponent(ComponentType::RIGIDBODY))
	{
		//rb->RemoveRigidBodyFromSimulation();
	}

	for (auto child : owner->GetChildren())
	{
		if (child->GetComponent(ComponentType::RIGIDBODY))
		{
			continue;
		}
		if (auto rb = child->CreateComponent(ComponentType::RIGIDBODY))
		{
			//rb->SetCollisionShape(ComponentRigidBody::SHAPE::BOX);
			//rb->UpdateNonSimulatedTransform();
		}
	}
}

