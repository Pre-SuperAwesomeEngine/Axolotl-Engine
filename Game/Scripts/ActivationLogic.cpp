#include "StdAfx.h"
#include "ActivationLogic.h"

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

#include "../Scripts/CameraControllerScript.h"
#include "../Scripts/PlayerManagerScript.h"
#include "../Scripts/HackZoneScript.h"

#include "DataStructures/Quadtree.h"
#include "Auxiliar/Audio/AudioData.h"

REGISTERCLASS(ActivationLogic);

ActivationLogic::ActivationLogic() : Script(),
componentAudio(nullptr), activeState(ActiveActions::INACTIVE)
{
	REGISTER_FIELD(linkedHackZone, HackZoneScript*);
	REGISTER_FIELD(interactWithEnemies, bool);
	REGISTER_FIELD(enemisToSpawn, GameObject*);
}

ActivationLogic::~ActivationLogic()
{
}

void ActivationLogic::Start()
{
	componentAudio = owner->GetComponent<ComponentAudioSource>();
	componentAnimation = owner->GetComponent<ComponentAnimation>();
	GameObject::GameObjectView children = owner->GetChildren();
	auto childWithRigid = std::find_if(std::begin(children),
									   std::end(children),
									   [](const GameObject* child)
									   {
										   return child->HasComponent<ComponentRigidBody>();
									   });
	componentRigidBody = (*childWithRigid)->GetComponent<ComponentRigidBody>();
	//componentRigidBody->Disable();
}

void ActivationLogic::Update(float deltaTime)
{
	if (!componentRigidBody->IsEnabled() && App->GetModule<ModulePlayer>()->GetCameraPlayerObject()->GetComponent<CameraControllerScript>()->IsInCombat())
	{
		componentAnimation->SetParameter("IsActive", false);
		componentRigidBody->Enable();
		componentAudio->PostEvent(AUDIO::SFX::AMBIENT::SEWERS::BIGDOOR_CLOSE);
	}
}

void ActivationLogic::OnCollisionEnter(ComponentRigidBody* other)
{
	if (linkedHackZone && !linkedHackZone->IsCompleted())
	{
		return;
	}

	if (!App->GetModule<ModulePlayer>()->GetCameraPlayerObject()->GetComponent<CameraControllerScript>()->IsInCombat())
	{
		if (other->GetOwner()->CompareTag("Player"))
		{
			PlayerManagerScript* playerManager = other->GetOwner()->GetComponent<PlayerManagerScript>();
			if (!playerManager->IsTeleporting())
			{
				componentAnimation->SetParameter("IsActive", true);
				componentRigidBody->Disable();
				componentAudio->PostEvent(AUDIO::SFX::AMBIENT::SEWERS::BIGDOOR_OPEN);
			}
		}
	}
}

void ActivationLogic::OnCollisionExit(ComponentRigidBody* other)
{
	if (other->GetOwner()->CompareTag("Player"))
	{
		componentAnimation->SetParameter("IsActive", false);
		// Until the trigger works 100% of the time better cross a closed door than be closed forever
		componentRigidBody->Enable();
		componentAudio->PostEvent(AUDIO::SFX::AMBIENT::SEWERS::BIGDOOR_CLOSE);
	}
}