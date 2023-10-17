#include "StdAfx.h"

#include "UIGameManager.h"

#include "Application.h"
#include "Components/ComponentPlayer.h"
#include "Components/ComponentScript.h"
#include "Components/UI/ComponentSlider.h"
#include "ModulePlayer.h"
#include "ModuleScene.h"
#include "SwitchPlayerManagerScript.h"
#include "UIImageDisplacementControl.h"
#include "HealthSystem.h"
#include "ModuleUI.h"
#include "Components/ComponentPlayer.h"
#include "PlayerManagerScript.h"

REGISTERCLASS(UIGameManager);

UIGameManager::UIGameManager() : Script(), mainMenuObject(nullptr), manager(nullptr), menuIsOpen(false),
hudCanvasObject(nullptr), healPwrUpObject(nullptr), attackPwrUpObject(nullptr), defensePwrUpObject(nullptr),
speedPwrUpObject(nullptr), pwrUpActive(false), savePwrUp(PowerUpType::NONE), sliderHudHealthBixFront(nullptr), 
sliderHudHealthBixBack(nullptr), sliderHudHealthAlluraFront(nullptr), sliderHudHealthAlluraBack(nullptr),
debugModeObject(nullptr), imgMouse(nullptr), imgController(nullptr), inputMethod(true), prevInputMethod(true),
loadRetryScene("Insert here the actual Level")
{
	REGISTER_FIELD(manager, GameObject*);
	REGISTER_FIELD(mainMenuObject, GameObject*);
	REGISTER_FIELD(hudCanvasObject, GameObject*);
	REGISTER_FIELD(debugModeObject, GameObject*);
	REGISTER_FIELD(imgMouse, GameObject*);
	REGISTER_FIELD(imgController, GameObject*);

	REGISTER_FIELD(sliderHudHealthBixFront, GameObject*);
	REGISTER_FIELD(sliderHudHealthBixBack, GameObject*);
	REGISTER_FIELD(sliderHudHealthAlluraFront, GameObject*);
	REGISTER_FIELD(sliderHudHealthAlluraBack, GameObject*);

	REGISTER_FIELD(healPwrUpObject, GameObject*);
	REGISTER_FIELD(attackPwrUpObject, GameObject*);
	REGISTER_FIELD(defensePwrUpObject, GameObject*);
	REGISTER_FIELD(speedPwrUpObject, GameObject*);

	REGISTER_FIELD(gameStates, GameObject*);
	REGISTER_FIELD(loadRetryScene, std::string);


}

void UIGameManager::Start()
{
	player = App->GetModule<ModulePlayer>()->GetPlayer()->GetComponent<ComponentPlayer>();
	
	healthSystemClassBix = player->GetOwner()->GetComponent<HealthSystem>();

	ui = App->GetModule<ModuleUI>();

	componentSliderPlayerFront = sliderHudHealthBixFront->GetComponent<ComponentSlider>();
	componentSliderPlayerBack = sliderHudHealthBixBack->GetComponent<ComponentSlider>();
	componentSliderPlayerFront->SetMaxValue(healthSystemClassBix->GetMaxHealth());
	componentSliderPlayerBack->SetMaxValue(healthSystemClassBix->GetMaxHealth());

	if (manager) 
	{
		SwitchPlayerManagerScript* switchPlayer = manager->GetComponent<SwitchPlayerManagerScript>();
		secondPlayer = switchPlayer->GetSecondPlayer()->GetComponent<ComponentPlayer>();

		healthSystemClassAllura = secondPlayer->GetOwner()->GetComponent<HealthSystem>();

		componentSliderSecondPlayerFront = sliderHudHealthAlluraFront->GetComponent<ComponentSlider>();
		componentSliderSecondPlayerBack = sliderHudHealthAlluraBack->GetComponent<ComponentSlider>();
		componentSliderSecondPlayerFront->SetMaxValue(healthSystemClassAllura->GetMaxHealth());
		componentSliderSecondPlayerBack->SetMaxValue(healthSystemClassAllura->GetMaxHealth());
	}
}

void UIGameManager::Update(float deltaTime)
{
	uiTime += deltaTime;

	input = App->GetModule<ModuleInput>();

	// Player input method true=GAMEPAD false=KEYBOARD
	if (input->GetCurrentInputMethod() == InputMethod::GAMEPAD)
	{
		player->SetMouse(false);
		inputMethod = true;
	}
	else if (input->GetCurrentInputMethod() == InputMethod::KEYBOARD)
	{
		if (menuIsOpen)
		{
			player->SetMouse(menuIsOpen);
		}

		inputMethod = false;
	}
	InputMethodImg(inputMethod);

	//Life controller
	if (componentSliderPlayerBack->GetCurrentValue() > 0)
	{
		if (healthSystemClassBix->GetCurrentHealth() != componentSliderPlayerBack->GetCurrentValue()
			|| healthSystemClassBix->GetCurrentHealth() != componentSliderPlayerFront->GetCurrentValue())
		{
			ModifySliderHealthValue(healthSystemClassBix, componentSliderPlayerBack, componentSliderPlayerFront, deltaTime);
		}
	}
	
	if (manager && componentSliderSecondPlayerBack->GetCurrentValue() > 0)
	{
		if (healthSystemClassAllura->GetCurrentHealth() != componentSliderSecondPlayerBack->GetCurrentValue()
			|| healthSystemClassAllura->GetCurrentHealth() != componentSliderSecondPlayerFront->GetCurrentValue())
		{
			ModifySliderHealthValue(healthSystemClassAllura, componentSliderSecondPlayerBack, componentSliderSecondPlayerFront, deltaTime);
		}
	}

	if (componentSliderPlayerBack->GetCurrentValue() <= 0 || componentSliderSecondPlayerBack->GetCurrentValue() <= 0)
	{
		LoseGameState(deltaTime);
		return;
	}

	//IN-GAME MENU
	if (input->GetKey(SDL_SCANCODE_ESCAPE) == KeyState::DOWN)
	{
		menuIsOpen = !menuIsOpen;
		MenuIsOpen();
	}

	// DEBUG MODE
	if (input->GetKey(SDL_SCANCODE_B) == KeyState::DOWN && debugModeObject != nullptr)
	{
		if (!debugModeObject->IsEnabled())
		{
			debugModeObject->Enable();
		}
		else
		{
			debugModeObject->Disable();
		}
	}

	// POWER UP SYSTEM
	if (pwrUpActive)
	{
		ActiveSliderUIPwrUP(deltaTime);
	}
}

// In  Game Menu Secction
void UIGameManager::SetMenuIsOpen(bool menuState)
{
	menuIsOpen = menuState;
	MenuIsOpen();
}

void UIGameManager::MenuIsOpen()
{
	if (menuIsOpen)
	{
		if (inputMethod)
		{
			ui->ResetCurrentButtonIndex();
		}

		App->GetModule<ModulePlayer>()->GetPlayer()->GetComponent<PlayerManagerScript>()->ParalyzePlayer(true);

		mainMenuObject->Enable();
		hudCanvasObject->Disable();
	}
	else
	{
		App->GetModule<ModulePlayer>()->GetPlayer()->GetComponent<PlayerManagerScript>()->ParalyzePlayer(false);
		mainMenuObject->Disable();
		hudCanvasObject->Enable();
	}
}

//Power Ups Secction
void UIGameManager::EnableUIPwrUp(enum class PowerUpType pwrUp)
{
	if (!pwrUpActive)
	{
		switch (pwrUp)
		{
		case PowerUpType::NONE:
			break;
		case PowerUpType::HEAL:
			componentSliderHealPwrUp->ModifyCurrentValue(componentSliderHealPwrUp->GetMaxValue());
			healPwrUpObject->Enable();
			break;
		case PowerUpType::ATTACK:
			componentSliderAttackPwrUp->ModifyCurrentValue(componentSliderAttackPwrUp->GetMaxValue());
			attackPwrUpObject->Enable();
			break;
		case PowerUpType::DEFENSE:
			componentSliderDefensePwrUp->ModifyCurrentValue(componentSliderDefensePwrUp->GetMaxValue());
			defensePwrUpObject->Enable();
			break;
		case PowerUpType::SPEED:
			componentSliderSpeedPwrUp->ModifyCurrentValue(componentSliderSpeedPwrUp->GetMaxValue());
			speedPwrUpObject->Enable();
			break;
		default:
			break;
		}
	}
		savePwrUp = pwrUp;
}

void UIGameManager::ActiveUIPwrUP(float maxPowerUpTimer)
{
	if (!pwrUpActive)
	{
		pwrUpActive = true;
		activePwrUp = savePwrUp;
		savePwrUp = PowerUpType::NONE;
		powerUpTimer = maxPowerUpTimer;
		currentPowerUpTime = 0.0f;
	}
}

void UIGameManager::ActiveSliderUIPwrUP(float time)
{
	currentPowerUpTime += time;

	differencePowerUpTime = powerUpTimer - currentPowerUpTime;

	if (powerUpTimer >= currentPowerUpTime)
	{
		switch (activePwrUp)
		{
		case PowerUpType::NONE:
			break;
		case PowerUpType::HEAL:
			componentSliderHealPwrUp->ModifyCurrentValue(differencePowerUpTime);
			break;
		case PowerUpType::ATTACK:
			componentSliderAttackPwrUp->ModifyCurrentValue(differencePowerUpTime);
			break;
		case PowerUpType::DEFENSE:
			componentSliderDefensePwrUp->ModifyCurrentValue(differencePowerUpTime);
			break;
		case PowerUpType::SPEED:
			componentSliderSpeedPwrUp->ModifyCurrentValue(differencePowerUpTime);
			break;
		}
	}
}

void UIGameManager::DisableUIPwrUP()
{
	switch (activePwrUp)
	{
	case PowerUpType::NONE:
		break;
	case PowerUpType::HEAL:
		healPwrUpObject->Disable();
		break;
	case PowerUpType::ATTACK:
		attackPwrUpObject->Disable();
		break;
	case PowerUpType::DEFENSE:
		defensePwrUpObject->Disable();
		break;
	case PowerUpType::SPEED:
		speedPwrUpObject->Disable();
		break;
	default:
		break;
	}
	pwrUpActive = false;
	EnableUIPwrUp(savePwrUp);
}

void UIGameManager::SetMaxPowerUpTime(float maxPowerUpTime)
{
	componentSliderHealPwrUp = healPwrUpObject->GetComponent<ComponentSlider>();
	componentSliderAttackPwrUp = attackPwrUpObject->GetComponent<ComponentSlider>();
	componentSliderDefensePwrUp = defensePwrUpObject->GetComponent<ComponentSlider>();
	componentSliderSpeedPwrUp = speedPwrUpObject->GetComponent<ComponentSlider>();

	componentSliderHealPwrUp->SetMaxValue(maxPowerUpTime);
	componentSliderAttackPwrUp->SetMaxValue(maxPowerUpTime);
	componentSliderDefensePwrUp->SetMaxValue(maxPowerUpTime);
	componentSliderSpeedPwrUp->SetMaxValue(maxPowerUpTime);
}

//Healt System Secction
void UIGameManager::ModifySliderHealthValue(HealthSystem* healthSystemClass, ComponentSlider* componentSliderBack, ComponentSlider* componentSliderFront, float deltaTime)
{

	float currentHealth = healthSystemClass->GetCurrentHealth();
	// We use 2 slider to do a effect in the health bar
	damage = currentHealth - componentSliderFront->GetCurrentValue();
	damageBack = currentHealth - componentSliderBack->GetCurrentValue();

	if (damageBack <= 0.0f && damage <= 0.0f)
	{
		componentSliderBack->ModifyCurrentValue(componentSliderBack->GetCurrentValue() + std::max(damageBack, -25.0f * deltaTime));
		componentSliderFront->ModifyCurrentValue(componentSliderFront->GetCurrentValue() + std::max(damage, -100.0f * deltaTime));
	}
	else
	{
		componentSliderBack->ModifyCurrentValue(componentSliderBack->GetCurrentValue() + std::min(damageBack, 100.0f * deltaTime));
		componentSliderFront->ModifyCurrentValue(componentSliderFront->GetCurrentValue() + std::min(damage, 100.0f * deltaTime));
	}

	if (currentHealth <= 0.0f)
	{
		gameOverTimer = 3.0f;
	}
}

// Game States Secction
void UIGameManager::LoseGameState(float deltaTime)
{
	if (gameOverTimer > 0.0f)
	{
		gameOverTimer -= deltaTime;
		return;
	}
	//PUT CODE TO PAUSE GAME
	if (!gameStates->IsEnabled())
	{
		gameStates->Enable();
		gameStates->GetChildren()[1]->GetChildren()[0]->Enable();
		LOG_INFO("YOU LOSE THE GAME");
	}
	
	// A button to Retry the game
	if (input->GetKey(SDL_SCANCODE_SPACE) == KeyState::DOWN)
	{
#ifndef ENGINE
		App->GetModule<ModuleScene>()->SetSceneToLoad("Lib/Scenes/"+loadRetryScene);
#endif // 

		LOG_INFO("YOU PRESSED A BUTTON AND LOAD RETRY SCENE");
	}

	// B button to go to the main menu
	if (input->GetKey(SDL_SCANCODE_E) == KeyState::DOWN)
	{
#ifndef ENGINE
		App->GetModule<ModuleScene>()->SetSceneToLoad("Lib/Scenes/00_MainMenu_VS3.axolotl");
#endif // 

		LOG_INFO("YOU PRESSED B BUTTON AND LOAD MAIN MENU");
	}
}

void UIGameManager::WinGameState()
{
	//PUT CODE TO PAUSE GAME
	if (!gameStates->IsEnabled())
	{
		gameStates->Enable();
		gameStates->GetChildren()[1]->GetChildren()[1]->Enable();
		LOG_INFO("YOU WIN THE GAME");
	}

	// B button to go to the main menu
	if (input->GetKey(SDL_SCANCODE_E) == KeyState::DOWN)
	{
#ifndef ENGINE
		App->GetModule<ModuleScene>()->SetSceneToLoad("Lib/Scenes/00_MainMenu_VS3.axolotl");
#endif // 

		LOG_INFO("YOU PRESSED B BUTTON AND LOAD MAIN MENU");
	}
}


//Input method used by player
void UIGameManager::InputMethodImg(bool input)
{
	if (currentInputTime == 0.0f && prevInputMethod != inputMethod)
	{
		if (input)
		{
			prevInputMethod = input;

			if (imgMouse && imgController)
			{
				imgMouse->Disable();
				imgController->Enable();
			}
		}
		else
		{
			prevInputMethod = input;

			if (imgMouse && imgController)
			{
				imgMouse->Enable();
				imgController->Disable();
			}
		}
		currentInputTime++;
	}
	else
	{
		if (currentInputTime >= 20.0f)
		{
			if (imgMouse && imgController)
			{
				imgController->Disable();
				imgMouse->Disable();
			}

			currentInputTime = 0.0f;
		}
		else
		{
			currentInputTime++;
		}
	}

}


