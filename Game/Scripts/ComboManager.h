#pragma once

#include "Scripting\Script.h"
#include "RuntimeInclude.h"

RUNTIME_MODIFIABLE_INCLUDE;

enum class AttackType
{
	NONE,
	SOFTNORMAL,
	HEAVYNORMAL,
	JUMPATTACK,
	SOFTFINISHER,
	HEAVYFINISHER
};

class ComboManager : public Script
{
public:
	ComboManager();
	~ComboManager() override = default;

	void CheckSpecial();
	AttackType CheckAttackInput(bool jumping);
	void SuccessfulAttack(int specialCount, bool type);

private:
	void Start() override;
	void Update(float deltaTime) override;

	ModuleInput* input;
	bool specialActivated;
	int specialCount;
	int maxSpecialCount;
	int comboCount;
	int maxComboCount;
};