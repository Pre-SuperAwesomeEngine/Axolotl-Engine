#include "StdAfx.h"
#include "BossChargeRockScript.h"

#include "Application.h"
#include "Modules/ModuleScene.h"
#include "Scene/Scene.h"

#include "Components/ComponentScript.h"
#include "Components/ComponentRigidbody.h"

#include "../Scripts/HealthSystem.h"

REGISTERCLASS(BossChargeRockScript);

BossChargeRockScript::BossChargeRockScript() : Script(), rockState(RockStates::SKY), fallingRockDamage(10.0f),
	despawnTimer(0.0f), despawnMaxTimer(30.0f), triggerRockDespawn(false)
{
	REGISTER_FIELD(fallingRockDamage, float);
	REGISTER_FIELD(despawnMaxTimer, float);
}

void BossChargeRockScript::Start()
{
	despawnTimer = despawnMaxTimer;

	rigidBody = owner->GetComponent<ComponentRigidBody>();
}

void BossChargeRockScript::Update(float deltaTime)
{
	if (triggerRockDespawn)
	{
		despawnTimer -= deltaTime;
		if (despawnTimer <= 0.0f)
		{
			DestroyRock();
		}
	}
}

void BossChargeRockScript::OnCollisionEnter(ComponentRigidBody* other)
{
	if (rockState == RockStates::SKY && other->GetOwner()->CompareTag("Rock"))
	{
		DeactivateRock();
	}
	else if (rockState == RockStates::FALLING)
	{
		if (other->GetOwner()->CompareTag("Enemy") || other->GetOwner()->CompareTag("Player"))
		{
			other->GetOwner()->GetComponent<HealthSystem>()->TakeDamage(fallingRockDamage);
			rockState = RockStates::HIT_ENEMY;
			DeactivateRock();

			// VFX Here: Rock hit an enemy on the head while falling
		}
		else if (other->GetOwner()->CompareTag("Floor"))
		{
			// VFX Here: Rock hit the floor
			rockState = RockStates::FLOOR;
		}
	}
	else
	{
		triggerRockDespawn = true;
	}
}

void BossChargeRockScript::SetRockState(RockStates newState)
{
	rockState = newState;
}

void BossChargeRockScript::DeactivateRock()
{
	if (rockState == RockStates::HIT_ENEMY)
	{
		// Only disable the root node of the rock and the rigid so the particles can still be seen
		owner->GetComponent<ComponentRigidBody>()->Disable();
		if (!owner->GetChildren().empty())
		{
			owner->GetChildren().front()->Disable();
		}

		// VFX Here: Disappear/Break rock (particles in the parent will still play, only the fbx will disappear)
	}
	else
	{
		owner->Disable();
	}

	triggerRockDespawn = true;
}

void BossChargeRockScript::DestroyRock() const
{
	App->GetModule<ModuleScene>()->GetLoadedScene()->DestroyGameObject(owner);
}