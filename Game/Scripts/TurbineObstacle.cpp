#include "StdAfx.h"
#include "TurbineObstacle.h"
#include "GameObject/GameObject.h"
#include "Components/Component.h"
#include "Components/ComponentRigidBody.h"

REGISTERCLASS(TurbineObstacle);

TurbineObstacle::TurbineObstacle() : Script(), rigidbody(nullptr)
{

}

void TurbineObstacle::Start()
{
	rigidbody = owner->GetComponent<ComponentRigidBody>();

}

void TurbineObstacle::Update(float deltaTime)
{
	rigidbody->UpdateRigidBody();
}