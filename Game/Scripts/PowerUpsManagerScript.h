#pragma once

#include "Scripting\Script.h"
#include "RuntimeInclude.h"

RUNTIME_MODIFIABLE_INCLUDE;

enum class PowerUpType;

class PowerUpsManagerScript : public Script
{
public:
	PowerUpsManagerScript();
	~PowerUpsManagerScript() override = default;

	void Start() override;
	void Update(float deltaTime) override;

	bool SavePowerUp(const PowerUpType& type);
	void DropSavedPowerUp();
	void UseSavedPowerUp();

	const PowerUpType& GetSavedPowerUpType() const;
	const PowerUpType& GetActivePowerUpType() const;

	float GetRadiusSeeking();
	bool GetDebugDraw();

private:
	void EliminateCurrentPowerUpEffect();

	float amountHealed;
	float attackIncrease;
	float defenseIncrease;
	float speedIncrease;

	float maxPowerUpTimer;
	float currentPowerUpTimer;

	float radiusSeeking;
	bool debugDraw;

	GameObject* player;
	GameObject* setUIManager;

	// To be changed to Player Manager
	PowerUpType activePowerUp;
	PowerUpType savedPowerUp;
};

inline float PowerUpsManagerScript::GetRadiusSeeking()
{
	return radiusSeeking;
}

inline bool PowerUpsManagerScript::GetDebugDraw()
{
	return debugDraw;
}
