#pragma once

#include "Scripting\Script.h"
#include "RuntimeInclude.h"

RUNTIME_MODIFIABLE_INCLUDE;

class PatrolBehaviourScript;
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

class EnemyVenomiteScript : public Script
{
public:
	EnemyVenomiteScript();
	~EnemyVenomiteScript() override = default;

	void Start() override;
	void Update(float deltaTime) override;

private:
	VenomiteBehaviours venomiteState;

	PatrolBehaviourScript* patrolScript;
	HealthSystem* healthScript;

	ComponentTransform* ownerTransform;
	ComponentAnimation* componentAnimation;
	ComponentAudioSource* componentAudioSource;
};