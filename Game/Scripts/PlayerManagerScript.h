#pragma once

#include "Scripting\Script.h"
#include "RuntimeInclude.h"
#include "ModuleInput.h"
#include "Bullet\LinearMath\btVector3.h"

RUNTIME_MODIFIABLE_INCLUDE;

class HealthSystem;
class PlayerAttackScript;
class PlayerJumpScript;
class PlayerMoveScript;
class PlayerHackingUseScript;
class PlayerForceUseScript;
class DebugGame;

enum class PlayerActions
{
	IDLE,
	WALKING,
	DASHING,
	JUMPING,
	DOUBLEJUMPING,
	FALLING
};

class PlayerManagerScript : public Script
{
public:
	PlayerManagerScript();
	~PlayerManagerScript() override = default;

	void Start() override;

	bool InsideForceOrHackingZone();

	float GetPlayerAttack() const;
	float GetPlayerDefense() const;
	float GetPlayerSpeed() const;
	float GetPlayerRotationSpeed() const;

	void IncreasePlayerAttack(float attackIncrease);
	void IncreasePlayerDefense(float defenseIncrease);
	void IncreasePlayerSpeed(float speedIncrease);

	void ParalyzePlayer(bool paralyzed);
	void PausePlayer(bool paused);
	void FullPausePlayer(bool paused);
	void TriggerJump(bool forcedJump);
	bool IsParalyzed() const;
	bool IsPaused() const;

	bool IsGrounded() const;
	bool IsTeleporting() const;
	GameObject* GetMovementParticleSystem() const;

	PlayerJumpScript* GetJumpManager() const;
	PlayerMoveScript* GetMovementManager() const;
	PlayerAttackScript* GetAttackManager() const;

	void StopHackingParticles() const;

	void SetPlayerSpeed(float playerSpeed);
	PlayerActions GetPlayerState() const;
	void SetPlayerState(PlayerActions playerState);

private:
	bool isActivePlayer;
	bool isPaused;

	ModuleInput* input;

	PlayerActions playerState;
	float playerAttack;
	float playerDefense;
	float playerSpeed;
	float playerRotationSpeed;
	btVector3 playerGravity;

	// All Main PlayerManagers
	HealthSystem* healthManager;
	PlayerMoveScript* movementManager;
	PlayerJumpScript* jumpManager;
	PlayerAttackScript* attackManager;
	DebugGame* debugManager;
	PlayerHackingUseScript* hackingManager;
	PlayerForceUseScript* forceManager;
	btVector3 rigidBodyLinearVelocity;
	btVector3 rigidBodyGravity;
	btVector3 rigidBodyManager;

	GameObject* movementParticleSystem;
};