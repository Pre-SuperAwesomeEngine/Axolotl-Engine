#include "StdAfx.h"
#include "TutorialSystem.h"

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

#include "..\Game\Scripts\PlayerManagerScript.h"
#include "..\Game\Scripts\PlayerManagerScript.h"
#include "..\Game\Scripts\UIImageDisplacementControl.h"
#include "..\Game\Scripts\HealthSystem.h"
#include "Components/UI/ComponentTransform2D.h"

#include "Auxiliar/Audio/AudioData.h"


REGISTERCLASS(TutorialSystem);

TutorialSystem::TutorialSystem() :
	Script(), userControllable(false), tutorialCurrentState(0), tutorialTotalStates(0), stateWaitTime(0.0f), 
	totalStateWaitTime(0.0f), dummy(nullptr), numNotControllableStates(0.0f), initialPos(45.0f, -195.0f, 0.0f)
{

	REGISTER_FIELD(userControllable, bool);
	REGISTER_FIELD(totalStateWaitTime, float);
	REGISTER_FIELD(stateWaitTime, float);

	REGISTER_FIELD(numNotControllableStates, float);

	/*REGISTER_FIELD(tutorialCurrentState, float);
	REGISTER_FIELD(tutorialTotalStates, float);*/

	REGISTER_FIELD(tutorialUI, std::vector<GameObject*>);
	REGISTER_FIELD(dummy, GameObject*);
}



void TutorialSystem::Start()
{
	
	currentTutorialUI = tutorialUI.front();
	displacementControl = currentTutorialUI->GetComponent<UIImageDisplacementControl>();
	transform2D = currentTutorialUI->GetComponent<ComponentTransform2D>();
	isWaiting = false;
	stateWaitTime = totalStateWaitTime;
	dummyHealthSystem = dummy->GetComponent<HealthSystem>();
	
	
}

void TutorialSystem::Update(float deltaTime)
{
	if (isWaiting)
	{
		stateWaitTime-= deltaTime;
	}

	if (stateWaitTime <= 0)
	{
		isWaiting = false;
		stateWaitTime = totalStateWaitTime;
		DeployUI();
		
		
	}

}

void TutorialSystem::TutorialStart()
{
	
	tutorialCurrentState = 0;
	tutorialTotalStates =  tutorialUI.size();
	currentTutorialUI->Enable();
	transform2D->SetPosition(initialPos);
	transform2D->CalculateMatrices();
	displacementControl->SetMovingToEnd(true);
	displacementControl->MoveImageToEndPosition();
	//transform2D = currentTutorialUI->GetComponent<ComponentTransform2D>();
}

void TutorialSystem::NextState()
{
	if (tutorialCurrentState < tutorialTotalStates)
	{
		UnDeployUI();
		
	}

	else if (tutorialCurrentState >= tutorialTotalStates)
	{
		TutorialEnd();

	}
	

}

void TutorialSystem::DeployUI()
{
	transform2D->SetPosition(initialPos);
	transform2D->CalculateMatrices();
	currentTutorialUI->Enable();
	
	displacementControl->SetMovingToEnd(true);
	displacementControl->MoveImageToEndPosition();

	if (tutorialCurrentState == int(numNotControllableStates))
	{
		dummyHealthSystem->SetIsImmortal(false);

	}	
	
}

void TutorialSystem::UnDeployUI()
{
	isWaiting = true;
	
	displacementControl->SetMovingToEnd(false);
	displacementControl->MoveImageToStarPosition();
	
	displacementControl->SetIsMoving(true);

	//currentTutorialUI->Disable();
	//NextTutorialUI->Enable();
	tutorialCurrentState++;
	currentTutorialUI = tutorialUI[tutorialCurrentState];
	displacementControl = currentTutorialUI->GetComponent<UIImageDisplacementControl>();
	//tutorialUI[tutorialCurrentState - 1]->Disable();
	dummyHealthSystem->SetIsImmortal(true);
}

void TutorialSystem::TutorialEnd()
{
	displacementControl->SetMovingToEnd(false);
	displacementControl->MoveImageToStarPosition();
	//currentTutorialUI->Disable();
	tutorialCurrentState = 0;
	currentTutorialUI = tutorialUI.front();
	
}

int TutorialSystem::GetTutorialCurrentState() const
{

	return tutorialCurrentState;

}

float TutorialSystem::GetNumControllableState() const
{

	return numNotControllableStates;

}

int TutorialSystem::GetTutorialSlideSize() const
{
	return tutorialTotalStates;
}

void TutorialSystem::SetTutorialSlideSize(int tutorialTotalStates)
{
	this->tutorialTotalStates = tutorialTotalStates;
}
