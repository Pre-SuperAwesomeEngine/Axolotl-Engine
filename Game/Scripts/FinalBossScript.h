#pragma once

#include "../Scripts/EnemyClass.h"
#include "RuntimeInclude.h"

RUNTIME_MODIFIABLE_INCLUDE;

class ComponentRigidBody;

class PatrolBehaviourScript;
class HealthSystem;

enum class FinalBossStates
{
	NEUTRAL, // The final boss is calm
	AGGRESSIVE, // The final boss is furious (increases atk cadence, no defensive attack will trigger)
	DEFENSIVE, // The final boss needs to recharge (increases speed and evasion, will trigger defensive attack)
	LAST_RESORT // The final boss is almost dead (triggers final attack and increases atk cadence and damage)
};

class FinalBossScript : public EnemyClass
{
public:
	FinalBossScript();
	~FinalBossScript() override = default;

	void Start() override;
	void Update(float deltaTime) override;

private:
	void ManageChangePhase();

	FinalBossStates bossState;

	ComponentRigidBody* rigidBody;

	PatrolBehaviourScript* patrolScript;
	HealthSystem* bossHealthSystem;
};