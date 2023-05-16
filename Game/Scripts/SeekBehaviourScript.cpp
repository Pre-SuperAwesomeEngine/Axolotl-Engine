#include "SeekBehaviourScript.h"

#include "Modules/ModuleScene.h"
#include "Modules/ModuleInput.h"
#include "Scene/Scene.h"

#include "Components/ComponentTransform.h"
#include "Components/ComponentRigidBody.h"

REGISTERCLASS(SeekBehaviourScript);

SeekBehaviourScript::SeekBehaviourScript() : Script(), target(nullptr)
{
	REGISTER_FIELD_WITH_ACCESSORS(Target, GameObject*);
}

void SeekBehaviourScript::Start()
{
	if (target)
	{
		targetTransform = static_cast<ComponentTransform*>(target->GetComponent(ComponentType::TRANSFORM));
	}
	ownerRigidBody = static_cast<ComponentRigidBody*>(owner->GetComponent(ComponentType::RIGIDBODY));
}

void SeekBehaviourScript::Update(float deltaTime)
{
	ENGINE_LOG("%s", "Now seeking...");
	ComponentTransform* ownerTransform = static_cast<ComponentTransform*>(owner->GetComponent(ComponentType::TRANSFORM));
	// When this behaviour is triggered, the enemy will go towards its target
	float3 targetPosition = targetTransform->GetPosition();
	ownerRigidBody->SetPositionTarget(targetPosition);
	float3 targetDirection = (targetTransform->GetPosition() - ownerTransform->GetPosition()).Normalized();
	ownerRigidBody->SetRotationTarget(
		Quat::LookAt(ownerTransform->GetLocalForward().Normalized(), targetDirection, ownerTransform->GetGlobalUp(), float3::unitY));
}

GameObject* SeekBehaviourScript::GetTarget() const
{
	return target;
}

void SeekBehaviourScript::SetTarget(GameObject* target)
{
	this->target = target;
}