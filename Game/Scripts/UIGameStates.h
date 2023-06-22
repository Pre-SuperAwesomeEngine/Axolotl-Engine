#pragma once

#include "Scripting\Script.h"

class ComponentPlayer;
class ModuleInput;

class UIGameStates : public Script
{
public:
	UIGameStates();
	~UIGameStates() override = default;

	void Start() override;
	void Update(float deltaTime) override;
	void SetMenuIsOpen(bool menuState);
	void MenuIsOpen();
	bool menuIsOpen;
	void WinStateScene(bool setState);
	void LoseStateScene(bool setState);

private:

	GameObject* loseStateObject;
	GameObject* winStateObject;
	GameObject* mainMenuObject;
	GameObject* hudStateObject;
	GameObject* setPlayer;
	ComponentPlayer* player;
	ModuleInput* input;
	std::string WinSceneName;
	std::string LoseSceneName;

};

inline void UIGameStates::SetMenuIsOpen(bool menuState)
{
	menuIsOpen = menuState;
	MenuIsOpen();
}











