#pragma once

#include "Scripting\Script.h"
#include <vector>

class ComponentButton;

class UIOptionsMenu : public Script
{
public:
	UIOptionsMenu();
	~UIOptionsMenu() override = default;

	void Start() override;
	void Update(float deltaTime) override;

private:
	struct OptionsButtonInfo
	{
		ComponentButton* button;
		GameObject* canvas;
		GameObject* hovered;
	};

	std::vector<OptionsButtonInfo> buttonsAndCanvas;
	int selectedPositon = 0;

	GameObject* gameOptionButton;
	GameObject* videoOptionButton;
	GameObject* audioOptionButton;
	GameObject* hudOptionButton;
	GameObject* keysOptionButton;

	GameObject* gameOptionCanvas;
	GameObject* videoOptionCanvas;
	GameObject* audioOptionCanvas;
	GameObject* hudOptionCanvas;
	GameObject* keysOptionCanvas;

	GameObject* gameOptionHover;
	GameObject* videoOptionHover;
	GameObject* audioOptionHover;
	GameObject* hudOptionHover;
	GameObject* keysOptionHover;

	ComponentButton* gameOptionComponentButton;
	ComponentButton* videoOptionComponentButton;
	ComponentButton* audioOptionComponentButton;
	ComponentButton* hudOptionComponentButton;
	ComponentButton* keysOptionComponentButton;

};








