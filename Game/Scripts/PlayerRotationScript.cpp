#include "PlayerRotationScript.h"

#include "Application.h"
#include "ModuleInput.h"

#include "Components/ComponentRigidBody.h"

REGISTERCLASS(PlayerRotationScript);

PlayerRotationScript::PlayerRotationScript() : Script(), rotationSensitivityHorizontal(1.0f),
rotationSensitivityVertical(1.0f)
{
	REGISTER_FIELD(rotationSensitivityVertical, float);
	REGISTER_FIELD(rotationSensitivityHorizontal, float);
}

void PlayerRotationScript::Start()
{
	
}

void PlayerRotationScript::PreUpdate(float deltaTime)
{
	Rotation(deltaTime);
}

void PlayerRotationScript::Rotation(float deltaTime)
{
	ComponentRigidBody* rigidBody = owner->GetComponent<ComponentRigidBody>();
	btRigidBody* btRb = rigidBody->GetRigidBody();

	float horizontalMotion = App->GetModule<ModuleInput>()->GetMouseMotion().x * rotationSensitivityHorizontal;
	btVector3 angularVelocity(0, 0, 0);

	if (horizontalMotion != 0)
	{
		btRb->setAngularFactor(btVector3(0.0f, 1.0f, 0.0f));
		angularVelocity = btVector3(0.0f, -horizontalMotion * deltaTime, 0.0f);
	}

	btRb->setAngularVelocity(angularVelocity);
}
