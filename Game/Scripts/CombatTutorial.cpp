#include "StdAfx.h"
#include "..\Game\Scripts\TutorialSystem.h"
#include "..\Game\Scripts\CombatTutorial.h"

#include "Application.h"
#include "ModuleInput.h"
#include "ModuleCamera.h"
#include "ModuleScene.h"
#include "ModulePlayer.h"

#include "Scene/Scene.h"
#include "GameObject/GameObject.h"

#include "Components/ComponentRigidBody.h"
#include "Components/ComponentAudioSource.h"
#include "Components/ComponentAnimation.h"
#include "Components/ComponentPlayer.h"
#include "Components/ComponentScript.h"
#include "Components/ComponentTransform.h"

#include "..\Game\Scripts\HealthSystem.h"
#include "..\Game\Scripts\PlayerManagerScript.h"
#include "..\Game\Scripts\UIImageDisplacementControl.h"
#include "../Scripts/PowerUpLogicScript.h"
#include "..\Game\Scripts\PlayerMoveScript.h"
#include "CameraControllerScript.h"

#include "Auxiliar/Audio/AudioData.h"

REGISTERCLASS(CombatTutorial);

CombatTutorial::CombatTutorial() : Script(), combatDummy(nullptr), userControllable(false), combatTutorialUI(nullptr), 
debugPowerUp(nullptr), finalWaitTime(5.0f), finalTotalWaitTime(5.0f),tutorialActivable(false), nextStateActive(true), door(nullptr)
{
	REGISTER_FIELD(combatDummy, GameObject*);
	REGISTER_FIELD(userControllable, bool);
	REGISTER_FIELD(combatTutorialUI, GameObject*);
	REGISTER_FIELD(debugPowerUp, GameObject*);
	REGISTER_FIELD(finalWaitTime, float);
	REGISTER_FIELD(finalTotalWaitTime, float);
	REGISTER_FIELD(door, GameObject*);

}

void CombatTutorial::Start()
{
	input = App->GetModule<ModuleInput>();
	player = App->GetModule<ModulePlayer>()->GetPlayer();
	componentAnimation = door->GetComponent<ComponentAnimation>();
	
	tutorialUI = combatTutorialUI->GetComponent<TutorialSystem>();
	

	if (combatDummy)
	{
		dummyHealthSystem = combatDummy->GetComponent<HealthSystem>();
		componentMoveScript = player->GetComponent<PlayerMoveScript>();
		dummyHealthSystem->SetIsImmortal(true);
	}

	if (door)
	{
		doorRigidbody = door->GetComponent<ComponentRigidBody>();
	}
}

void CombatTutorial::Update(float deltaTime)
{
	//Normal Attacks XXX - XXY
	if (tutorialActivable && userControllable && input->GetKey(SDL_SCANCODE_F) == KeyState::DOWN && !tutorialUI->GetDisplacementControl()->IsMoving())
	{
		
		tutorialUI->UnDeployUI();
		dummyHealthSystem->SetIsImmortal(true);
		doorRigidbody->SetIsTrigger(false);
		componentMoveScript->SetIsParalyzed(true);
		
		if (tutorialUI->GetTutorialCurrentState() == static_cast<int>(tutorialUI->GetNumControllableState()))
		{
			dummyHealthSystem->SetIsImmortal(false);
			userControllable = false;
			nextStateActive = false;
			componentMoveScript->SetIsParalyzed(false);
		}
	}

	else if (tutorialActivable && input->GetKey(SDL_SCANCODE_G) == KeyState::DOWN && !tutorialUI->GetDisplacementControl()->IsMoving())
	{
		tutorialUI->TutorialSkip();
		componentAnimation->SetParameter("IsActive", true);
		doorRigidbody->Disable();

	}
	else if (dummyHealthSystem->GetCurrentHealth() <= dummyHealthSystem->GetMaxHealth() * 0.75f
		&& dummyHealthSystem->GetCurrentHealth() > dummyHealthSystem->GetMaxHealth() * 0.50f && !nextStateActive)
	{
		//JumpAttack
		LOG_INFO("Tutorial:JumpAttack");

		tutorialUI->UnDeployUI();
		dummyHealthSystem->SetIsImmortal(false);

		nextStateActive = true;
	}

	else if (dummyHealthSystem->GetCurrentHealth() <= dummyHealthSystem->GetMaxHealth() * 0.50f
		&& dummyHealthSystem->GetCurrentHealth() > dummyHealthSystem->GetMaxHealth() * 0.25f && nextStateActive)
	{
		//SpecialLightAttack
		LOG_INFO("Tutorial:SpecialLightAttack");

		tutorialUI->UnDeployUI();
		dummyHealthSystem->SetIsImmortal(false);

		nextStateActive = false;
	}
	else if (dummyHealthSystem->GetCurrentHealth() <= dummyHealthSystem->GetMaxHealth() * 0.25f
		&& dummyHealthSystem->GetCurrentHealth() > 0.0f && !nextStateActive)
	{
		//SpecialHeavyAttack
		LOG_INFO("Tutorial:SpecialHeavyAttack");

		tutorialUI->UnDeployUI();
		dummyHealthSystem->SetIsImmortal(false);

		//tutorialUI->NextState();
		nextStateActive = true;
	}
	else if (dummyHealthSystem->GetCurrentHealth() <= 0.0f && nextStateActive)
	{
		//SpecialHeavyAttack

		tutorialUI->UnDeployUI();
		dummyHealthSystem->SetIsImmortal(false);

		if (debugPowerUp != nullptr)
		{
			PowerUpLogicScript* newPowerUpLogic = debugPowerUp->GetComponent<PowerUpLogicScript>();
			ComponentTransform* ownerTransform = player->GetComponent<ComponentTransform>();

			newPowerUpLogic->ActivatePowerUp(ownerTransform->GetOwner());
		}

		userControllable = true;
		tutorialFinished = true;
		nextStateActive = false;
	}

	if (tutorialFinished && !nextStateActive)
	{
		finalWaitTime -= deltaTime;
	}

	if (tutorialFinished && !nextStateActive && finalWaitTime <= 0.0f)
	{
		tutorialUI->UnDeployUI();
		tutorialFinished = false;
		tutorialActivable = false;
		finalWaitTime = finalTotalWaitTime;
		componentAnimation->SetParameter("IsActive", true);
		doorRigidbody->Disable();
		doorRigidbody->SetIsTrigger(true);
		LOG_INFO("Tutorial:END");

	}
}

void CombatTutorial::OnCollisionEnter(ComponentRigidBody* other)
{

		if (other->GetOwner()->CompareTag("Player"))
		{
			PlayerManagerScript* playerManager = other->GetOwner()->GetComponent<PlayerManagerScript>();
			App->GetModule<ModulePlayer>()->SetInCombat(true);
			tutorialActivable = true;
			userControllable = true;
			//Launches intro
			tutorialUI->TutorialStart();
			LOG_INFO("TutorialEntered");
		}
}

void CombatTutorial::OnCollisionExit(ComponentRigidBody* other)
{
	if (other->GetOwner()->CompareTag("Player") && !tutorialFinished)
	{
		App->GetModule<ModulePlayer>()->SetInCombat(false);
		tutorialUI->TutorialEnd();
		LOG_INFO("TutorialExit");
	}
}
