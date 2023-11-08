#include "StdAfx.h"
#include "BossShieldScript.h"

#include "Auxiliar/Audio/AudioData.h"

#include "Components/ComponentScript.h"
#include "Components/ComponentRigidBody.h"
#include "Components/ComponentMeshRenderer.h"
#include "Components/ComponentAudioSource.h"

REGISTERCLASS(BossShieldScript);

BossShieldScript::BossShieldScript() : Script(), rigidBody(nullptr), parentRigidBody(nullptr), audioSource(nullptr)
{
}

void BossShieldScript::Start()
{
	rigidBody = owner->GetComponent<ComponentRigidBody>();
	audioSource = owner->GetComponent<ComponentAudioSource>();

	// This is the rigidbody of the boss itself
	parentRigidBody = owner->GetParent()->GetComponent<ComponentRigidBody>();
}

void BossShieldScript::Update(float deltaTime)
{
	rigidBody->UpdateRigidBody();
}

void BossShieldScript::OnCollisionEnter(ComponentRigidBody* other)
{
	if (other->GetOwner()->CompareTag("PriorityTarget"))
	{
		hitBySpecialTarget = true;
	}
	else if (other->GetOwner()->CompareTag("Rock"))
	{
		other->GetOwner()->GetComponent<ComponentRigidBody>()->Disable();
		other->GetOwner()->GetComponent<ComponentMeshRenderer>()->Disable();
	}
}

void BossShieldScript::ActivateShield() const
{
	owner->Enable();

	parentRigidBody->SetIsKinematic(true);
	parentRigidBody->SetIsTrigger(true);
	parentRigidBody->SetUpMobility();

	rigidBody->SetIsTrigger(false);

	audioSource->PostEvent(AUDIO::SFX::NPC::FINALBOSS::ENERGYSHIELD);
	// VFX Here: Any effect related to the activation of the shield
}

void BossShieldScript::DeactivateShield() const
{
	rigidBody->SetIsTrigger(true);

	parentRigidBody->SetIsKinematic(false);
	parentRigidBody->SetIsTrigger(false);
	parentRigidBody->SetUpMobility();

	owner->Disable();

	audioSource->PostEvent(AUDIO::SFX::NPC::FINALBOSS::ENERGYSHIELD_STOP);
	// VFX Here: Any effect related to the deactivation of the shield
}

bool BossShieldScript::WasHitBySpecialTarget() const
{
	return hitBySpecialTarget;
}

void BossShieldScript::DisableHitBySpecialTarget()
{
	hitBySpecialTarget = false;
}