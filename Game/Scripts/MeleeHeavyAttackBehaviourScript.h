#pragma once

#include "Scripting\Script.h"
#include "RuntimeInclude.h"

RUNTIME_MODIFIABLE_INCLUDE;

enum class ExplosionState
{
	NOTDEAD,
	WAITING_EXPLOSION,
	EXPLODING,
	DEAD
};

class ComponentRigidBody;
class ComponentTransform;
class ComponentAgent;
class ComponentAudioSource;
class ComponentAnimation;
class ComponentParticleSystem;
class EnemyDeathScript;
class HealthSystem;
class AIMovement;

class MeleeHeavyAttackBehaviourScript : public Script
{
public:
	MeleeHeavyAttackBehaviourScript();
	~MeleeHeavyAttackBehaviourScript() override = default;

	void Start() override;
	void Update(float deltaTime) override;

	void TriggerExplosion();
	void UpdateDroneColor();

	ExplosionState HasExploded() const;
	
	void SetIsPaused(bool isPaused);


private:
	void OnCollisionEnter(ComponentRigidBody* other) override;
	void OnCollisionExit(ComponentRigidBody* other) override;

	ExplosionState attackState;

	GameObject* targetPlayer;

	ComponentRigidBody* rigidBody;
	ComponentTransform* transform;
	EnemyDeathScript* parentDeathScript;
	ComponentAudioSource* componentAudioSource;
	ComponentAgent* ownerAgent;
	ComponentAnimation* componentAnimation;
	ComponentParticleSystem* particleSystem;
	AIMovement* aiMovement;
	HealthSystem* parentHealth;

	float explosionDamage;
	float explosionTime;
	float explosionDistance;
	bool isPaused;
};
