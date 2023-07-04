#pragma once

#include "Scripting\Script.h"
#include "RuntimeInclude.h"

#include "../Scripts/EnemyClass.h"

RUNTIME_MODIFIABLE_INCLUDE;

class PatrolBehaviourScript;
class SeekBehaviourScript;
class RangedFastAttackBehaviourScript;
class MeleeFastAttackBehaviourScript;
class HealthSystem;

class ComponentTransform;
class ComponentAnimation;
class ComponentAudioSource;

enum class VenomiteBehaviours
{
	IDLE,
	PATROL,
	RANGED_ATTACK,
	SEEK,
	MELEE_ATTACK
};

class EnemyVenomiteScript : public Script, public EnemyClass
{
public:
	EnemyVenomiteScript();
	~EnemyVenomiteScript() override = default;

	void Start() override;
	void Update(float deltaTime) override;

	void SetStunnedTime(float newTime);

private:
	VenomiteBehaviours venomiteState;

	PatrolBehaviourScript* patrolScript;
	SeekBehaviourScript* seekScript;
	std::vector<RangedFastAttackBehaviourScript*> rangedAttackScripts;
	MeleeFastAttackBehaviourScript* meleeAttackScript;
	HealthSystem* healthScript;

	ComponentTransform* ownerTransform;
	ComponentAnimation* componentAnimation;
	ComponentAudioSource* componentAudioSource;

	float rangedAttackDistance;
	float meleeAttackDistance;
	float timeStunned;
	bool stunned;

	GameObject* batonGameObject;
	GameObject* blasterGameObject;
};