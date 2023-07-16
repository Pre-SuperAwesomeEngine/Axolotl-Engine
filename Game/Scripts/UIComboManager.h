#pragma once

#include "Scripting\Script.h"
#include "RuntimeInclude.h"

RUNTIME_MODIFIABLE_INCLUDE;

enum class InputVisualType 
{
	SOFT,
	HEAVY
};

class GameObject;
class ComponentSlider;

class UIComboManager : public Script
{
public:
	UIComboManager();
	~UIComboManager() override = default;

	void Start() override;
	void Update(float deltaTime) override;

	void SetActivateSpecial(bool activate);
	void SetComboBarValue(float value);

	void AddInputVisuals(InputVisualType type);

	void CleanInputVisuals();

private:
	GameObject* inputPrefabSoft;
	GameObject* inputPrefabHeavy;

	std::deque<GameObject*> inputVisuals;
	std::vector<GameObject*> inputPositions;

	ComponentSlider* comboBar;
};