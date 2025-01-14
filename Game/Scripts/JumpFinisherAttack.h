#pragma once

#include "Scripting\Script.h"
#include "RuntimeInclude.h"

RUNTIME_MODIFIABLE_INCLUDE;

class ModuleInput;

class JumpFinisherArea;
class JumpFinisherAttackBullet;
class PlayerManagerScript;

class ComponentRigidBody;

class JumpFinisherAttack : public Script
{
public:
	JumpFinisherAttack();
	~JumpFinisherAttack() override = default;

	void Start() override;
	
	void PerformGroundSmash();
	void VisualLandingEffect();
	void PushEnemies(float pushForce, float stunTime, std::vector<ComponentRigidBody*>& enemies);
	void ShootForceBullet(float pushForce, float stunTime);

	bool IsActive() const;

	void SetBulletHitTheFloor(bool bulletHitTheFloor);
	bool GetBulletHitTheFloor() const;

private:
	ModuleInput* input;

	JumpFinisherArea* forceArea;
	GameObject* forceAttackBullet;

	ComponentRigidBody* rigidBody;
	PlayerManagerScript* playerManager;

	bool activated;
	bool bulletHitTheFloor;
};