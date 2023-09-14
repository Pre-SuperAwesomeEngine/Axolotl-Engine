#include "StdAfx.h"
#include "SwitchPlayerManagerScript.h"

#include "ModulePlayer.h"
#include "ModuleInput.h"

#include "Components/ComponentScript.h"
#include "Components/ComponentTransform.h"
#include "Components/ComponentRigidBody.h"
#include "Components/ComponentPlayer.h"

#include "../Scripts/PlayerManagerScript.h"
#include "../Scripts/PlayerMoveScript.h"
#include "../Scripts/PlayerJumpScript.h"

#include "../Scripts/CameraControllerScript.h"
#include "Application.h"

REGISTERCLASS(SwitchPlayerManagerScript);

SwitchPlayerManagerScript::SwitchPlayerManagerScript() : Script(), camera(nullptr), input(nullptr)
{
	REGISTER_FIELD(secondPlayer, GameObject*);
}

void SwitchPlayerManagerScript::Start()
{
	input = App->GetModule<ModuleInput>();
	
	mainCamera = App->GetModule<ModulePlayer>()->GetCameraPlayerObject();

	camera = mainCamera->GetComponent<CameraControllerScript>();
	cameraTransform = mainCamera->GetComponent<ComponentTransform>();

	currentPlayer = App->GetModule<ModulePlayer>()->GetPlayer();
	LOG_DEBUG("Player 1: {}", currentPlayer);
	LOG_DEBUG("Player 2: {}", secondPlayer);

	camera->ChangeCurrentPlayer(currentPlayer->GetComponent<ComponentTransform>());
}

void SwitchPlayerManagerScript::Update(float deltaTime)
{
	if (!isChangingPlayer)
	{
		if (input->GetKey(SDL_SCANCODE_C) != KeyState::IDLE && secondPlayer)
		{
			CheckChangeCurrentPlayer();
		}
	}
	else 
	{
		HandleChangeCurrentPlayer();
	}
}

void SwitchPlayerManagerScript::CheckChangeCurrentPlayer()
{
	movementManager = currentPlayer->GetComponent<PlayerMoveScript>();
	jumpManager = currentPlayer->GetComponent<PlayerJumpScript>();
	camera->ToggleCameraState();
	movementManager->ChangingCurrentPlayer(true);
	jumpManager->ChangingCurrentPlayer(true);

	changePlayerTimer.Start();
	isChangingPlayer = true;
}

void SwitchPlayerManagerScript::HandleChangeCurrentPlayer()
{
	if (changePlayerTimer.Read() >= 2000)
	{	
		camera->ChangeCurrentPlayer(secondPlayer->GetComponent<ComponentTransform>());
		movementManager->ChangingNewCurrentPlayer(false);
		jumpManager->ChangingCurrentPlayer(false);

		changePlayerTimer.Stop();
		isChangingPlayer = false;
		isNewPlayerEnabled = !isNewPlayerEnabled;
		GameObject* changePlayerGameObject = currentPlayer;
		currentPlayer = App->GetModule<ModulePlayer>()->GetPlayer();
		secondPlayer = changePlayerGameObject;
	}

	else if (changePlayerTimer.Read() >= 1500 && !isNewPlayerEnabled)
	{
		movementManager->ChangingCurrentPlayer(false);
		// The position where the newCurrentPlayer will appear
		rigidBodyVec3 = btVector3(cameraTransform->GetGlobalPosition().x, currentPlayer->GetComponent<ComponentTransform>()->GetGlobalPosition().y,
				cameraTransform->GetGlobalPosition().z);

		// Disabling the current player
		currentPlayer->GetComponent<ComponentPlayer>()->SetActualPlayer(false);

		currentPlayer->Disable();
		

		movementManager = secondPlayer->GetComponent<PlayerMoveScript>();
		jumpManager = secondPlayer->GetComponent<PlayerJumpScript>();
		movementManager->ChangingNewCurrentPlayer(true);
		jumpManager->ChangingCurrentPlayer(true);
		
		// Enabling the new current player
		secondPlayer->Enable();
		secondPlayer->GetComponent<ComponentPlayer>()->SetActualPlayer(true);

		secondPlayer->GetComponent<ComponentRigidBody>()->SetRigidBodyOrigin(rigidBodyVec3);
		isNewPlayerEnabled = !isNewPlayerEnabled;
	}

	else if (changePlayerTimer.Read() >= 1000 && !isNewPlayerEnabled)
	{
		jumpManager->ChangingCurrentPlayer(false);
	}
}