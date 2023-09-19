#pragma once

#include "Scripting\Script.h"
#include "RuntimeInclude.h"
#include "Timer/Timer.h"
#include "Bullet\LinearMath\btVector3.h"

RUNTIME_MODIFIABLE_INCLUDE;

class CameraControllerScript;
class ComponentTransform;
class ComponentRigidBody;
class PlayerManagerScript;
class ModuleInput;

class SwitchPlayerManagerScript : public Script
{
public:
	SwitchPlayerManagerScript();

	void Update(float deltaTime) override;

	void CheckChangeCurrentPlayer();
	void HandleChangeCurrentPlayer();

	void SwitchHealthBars();

	void SetIsSwitchAvailable(bool available);

	void VisualSwicthEffect();
	GameObject* GetSecondPlayer();

private:
	void Start() override;

	ModuleInput* input;
	Timer changePlayerTimer;

	GameObject* playerHealthBar;
	GameObject* secondPlayerHealthBar;
	GameObject* mainCamera;
	CameraControllerScript* camera;
	ComponentTransform* cameraTransform;

	GameObject* switchPlayersParticlesPrefab;
	GameObject* actualSwitchPlayersParticles;

	GameObject* currentPlayer;
	GameObject* secondPlayer;
	PlayerManagerScript* playerManager;
	ComponentTransform* playerTransform;

	btVector3 rigidBodyVec3;

	float3 positionPlayer;

	bool isChangingPlayer;
	bool isNewPlayerEnabled;
	bool isSwitchAvailable;
};
