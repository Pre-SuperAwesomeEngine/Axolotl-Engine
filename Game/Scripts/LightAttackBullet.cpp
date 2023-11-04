#include "StdAfx.h"
#include "LightAttackBullet.h"

#include "Application.h"

#include "ModuleScene.h"
#include "ModulePlayer.h"
#include "Scene/Scene.h"

#include "Components/ComponentRigidBody.h"
#include "Components/ComponentAudioSource.h"
#include "Components/ComponentTransform.h"
#include "Components/ComponentScript.h"
#include "Components/ComponentParticleSystem.h"
#include "Components/ComponentMeshRenderer.h"
#include "Components/ComponentLine.h"

#include "../Scripts/HealthSystem.h"
#include "../Scripts/EnemyClass.h"
#include "../Scripts/PlayerAttackScript.h"

#include "Auxiliar/Audio/AudioData.h"

REGISTERCLASS(LightAttackBullet);

LightAttackBullet::LightAttackBullet() :
	Script(),
	enemy(nullptr),
	velocity(25.0f),
	audioSource(nullptr),
	stunTime(10.0f),
	damageAttack(10.0f),
	defaultTargetPos(0,0,0),
	maxDistanceBullet(10.0f),
	maxLifeTimeBullet(5.0f),
	currentLifeTimeBullet(0.0f),
	maxDistanceBullet(100.0f),
	particleSystem(nullptr), 
	particleSystemTimer(1.0f), 
	triggerParticleSystemTimer(false), 
	particleSystemCurrentTimer(0.0f),
	lifeTime(12.0f),
	parentTransform(nullptr),
	targetTransform(nullptr)
{
	REGISTER_FIELD(particleSystemTimer, float);
	REGISTER_FIELD(maxDistanceBullet, float);
	REGISTER_FIELD(maxLifeTimeBullet, float);
	REGISTER_FIELD(initPos, ComponentTransform*);
	REGISTER_FIELD(velocity, float);
}

void LightAttackBullet::Start()
{
	rigidBody = owner->GetComponent<ComponentRigidBody>();
	parentTransform = owner->GetParent()->GetComponent<ComponentTransform>();
	bulletTransform = owner->GetComponent<ComponentTransform>();

	audioSource = owner->GetComponent<ComponentAudioSource>();	

	defaultTargetPos = parentTransform->GetGlobalForward();
	defaultTargetPos.y = 0;
	defaultTargetPos.Normalize();
	defaultTargetPos = defaultTargetPos * maxDistanceBullet;
	defaultTargetPos += parentTransform->GetGlobalPosition();	

	particleSystem = owner->GetComponent<ComponentParticleSystem>();
	playerAttackScript = App->GetModule<ModulePlayer>()->GetPlayer()->GetComponent<PlayerAttackScript>();

	ResetDefaultValues();
}

void LightAttackBullet::Update(float deltaTime)
{
	if (isPaused)
	{
		return;
	}

	if (enemy != nullptr)
	{
		if (!enemy->IsEnabled()) 
		{
			DestroyBullet();
			return;
		}

		float3 targetPos = targetTransform->GetGlobalPosition();
		targetPos.y += 1; 
		float3 forward = targetPos - owner->GetComponent<ComponentTransform>()->GetGlobalPosition();
		forward.Normalize();

		btRigidBody* btRb = rigidBody->GetRigidBody();
		btRb->setLinearVelocity(
			btVector3(
				forward.x,
				0,
				forward.z) * velocity);
	}

	lifeTime -= deltaTime;
	if (lifeTime <= 0.0f)
	{
		DestroyBullet();
	}

	currentLifeTimeBullet += deltaTime;

	if (maxLifeTimeBullet <= currentLifeTimeBullet)
	{
		owner->Disable();
	}

	if (!triggerParticleSystemTimer)
	{
		return;
	}
	
	// When the particles are ready to be played, play them and after them, delete the bullet
	particleSystemCurrentTimer -= deltaTime;
	
	if (particleSystemCurrentTimer <= 0.0f)
	{
		particleSystemCurrentTimer = particleSystemTimer;
		triggerParticleSystemTimer = false;

		if (particleSystem)
		{
			particleSystem->Stop();
		}
		DestroyBullet();
	}
}

void LightAttackBullet::StartMoving()
{
	rigidBody->Enable();

	RepositionBullet();

	rigidBody->SetDefaultPosition();
	rigidBody->SetUseRotationController(true);

	float3 forward = parentTransform->GetGlobalForward();
	forward.Normalize();

	btRigidBody* btRb = rigidBody->GetRigidBody();
	btRb->setLinearVelocity(
		btVector3(
			forward.x,
			0,
			forward.z) * velocity);

}

void LightAttackBullet::SetPauseBullet(bool isPaused)
{
	this->isPaused = isPaused;
	float3 forward = parentTransform->GetGlobalForward();
	forward.Normalize();
	btRigidBody* btRb = rigidBody->GetRigidBody();

	if (isPaused)
	{
		btRb->setLinearVelocity(btVector3(0.f, 0.f, 0.f));
	}
	else
	{
		btRb->setLinearVelocity(
			btVector3(
				forward.x,
				0,
				forward.z) * velocity);
	}
}

//Function to reposition the bullet to the front of the player before shooting
void LightAttackBullet::RepositionBullet()
{
	float3 currentForward = bulletTransform->GetGlobalForward().Normalized();
	float3 desiredForward;
	float3 height = float3(0.0f, 1.0f, 0.0f);
	// Create a new bullet
	if (enemy)
	{
		desiredForward = (targetTransform->
			GetGlobalPosition() - parentTransform->GetGlobalPosition()).Normalized();

		bulletTransform->SetGlobalPosition(parentTransform->GetGlobalPosition() + height
			+ (desiredForward).Normalized());

		float angle = Quat::FromEulerXYZ(currentForward.x, currentForward.y, currentForward.z).
			AngleBetween(Quat::FromEulerXYZ(desiredForward.x, desiredForward.y, desiredForward.z));
		angle = math::Abs(math::RadToDeg(angle));

		bulletTransform->SetGlobalRotation(bulletTransform->GetGlobalRotation().LookAt(currentForward, desiredForward,
			float3::unitZ, float3::unitY));
	}
	else
	{
		desiredForward = parentTransform->GetGlobalForward();
		bulletTransform->SetGlobalPosition(parentTransform->GetGlobalPosition() + height
			+ (desiredForward).Normalized());

		float angle = Quat::FromEulerXYZ(currentForward.x, currentForward.y, currentForward.z).
			AngleBetween(Quat::FromEulerXYZ(desiredForward.x, desiredForward.y, desiredForward.z));
		angle = math::Abs(math::RadToDeg(angle));

		bulletTransform->SetGlobalRotation(bulletTransform->GetGlobalRotation().LookAt(currentForward, desiredForward,
			float3::unitZ, float3::unitY));
	}

	bulletTransform->RecalculateLocalMatrix();
	bulletTransform->UpdateTransformMatrices();
}

void LightAttackBullet::SetStunTime(float nStunTime)
{
	stunTime = nStunTime;
}

void LightAttackBullet::SetEnemy(GameObject* nEnemy)
{
	enemy = nEnemy;
	if (enemy)
	{
		targetTransform = enemy->GetComponent<ComponentTransform>();
	}
}

void LightAttackBullet::SetDamage(float nDamageAttack)
{
	damageAttack = nDamageAttack;
}

void LightAttackBullet::ResetDefaultValues()
{
	currentLifeTimeBullet = 0.0f;

	defaultTargetPos = parentTransform->GetGlobalForward();
	defaultTargetPos.y = 0;
	defaultTargetPos.Normalize();
	defaultTargetPos = defaultTargetPos * maxDistanceBullet;
	defaultTargetPos += parentTransform->GetGlobalPosition();

	bulletTransform->SetGlobalPosition(initPos->GetGlobalPosition());
	bulletTransform->SetGlobalRotation(initPos->GetGlobalRotation());
	bulletTransform->RecalculateLocalMatrix();

	rigidBody->UpdateRigidBody();
	rigidBody->Enable();
	rigidBody->SetDefaultPosition();
	rigidBody->SetUseRotationController(true);

	if (particleSystem)
	{
		particleSystem->Enable();
		particleSystemCurrentTimer = particleSystemTimer;
	}

	owner->GetComponent<ComponentMeshRenderer>()->Enable();
}

void LightAttackBullet::OnCollisionEnter(ComponentRigidBody* other)
{
	if (other->GetOwner()->CompareTag("Enemy") || other->GetOwner()->CompareTag("PriorityTarget"))
	{
		if (playerAttackScript->IsMelee())
		{
			audioSource->PostEvent(AUDIO::SFX::NPC::SHOT_IMPACT); // Provisional sfx
		}
		else
		{
			audioSource->PostEvent(AUDIO::SFX::PLAYER::WEAPON::ELECTRIC_SHOT); // Provisional sfx
		}
		other->GetOwner()->GetComponent<HealthSystem>()->TakeDamage(damageAttack);
		other->GetOwner()->GetComponent<EnemyClass>()->SetStunnedTime(stunTime);

		// Disable the visuals and the rigidbody while the particles are being played
		//rigidBody->SetIsTrigger(true);
		
		owner->GetComponent<ComponentMeshRenderer>()->Disable();

		if (owner->HasComponent<ComponentLine>())
		{
			owner->GetComponent<ComponentLine>()->Disable();
		}

		if (particleSystem)
		{
			particleSystem->Play();
		}
		triggerParticleSystemTimer = true;
	}

	else if (!other->IsTrigger() && !other->GetOwner()->CompareTag("Player"))
	{
		if (playerAttackScript->IsMelee())
		{
			audioSource->PostEvent(AUDIO::SFX::NPC::SHOT_IMPACT); // Provisional sfx
		}
		else
		{
			audioSource->PostEvent(AUDIO::SFX::PLAYER::WEAPON::ELECTRIC_SHOT); // Provisional sfx
		}
		owner->Disable();
	}
}

void LightAttackBullet::DestroyBullet()
{	
	App->GetModule<ModuleScene>()->GetLoadedScene()->RemoveParticleSystem(particleSystem);
	App->GetModule<ModuleScene>()->GetLoadedScene()->DestroyGameObject(owner);
}

