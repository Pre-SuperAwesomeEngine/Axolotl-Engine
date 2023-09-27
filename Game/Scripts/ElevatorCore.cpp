#include "StdAfx.h"
#include "ElevatorCore.h"

#include "Application.h"
#include "ModuleInput.h"
#include "ModuleCamera.h"
#include "ModuleScene.h"
#include "ModulePlayer.h"

#include "Components/ComponentRigidBody.h"
#include "Components/ComponentAudioSource.h"
#include "Components/ComponentTransform.h"
#include "Components/ComponentScript.h"
#include "PlayerManagerScript.h"
#include "PlayerMoveScript.h"
#include "PlayerJumpScript.h"
#include "PlayerAttackScript.h"


#include "Scene/Scene.h"

#include "GameObject/GameObject.h"

#include "DataStructures/Quadtree.h"
#include "Auxiliar/Audio/AudioData.h"


REGISTERCLASS(ElevatorCore);

ElevatorCore::ElevatorCore() : Script(),
componentAudio(nullptr), activeState(ActiveActions::INACTIVE), positionState(PositionState::UP)
{
	REGISTER_FIELD(elevator, GameObject*);
	REGISTER_FIELD(finalPos, float);
	REGISTER_FIELD(coolDown, float);
}

ElevatorCore::~ElevatorCore()
{
}

void ElevatorCore::Start()
{
	LOG_DEBUG("Name of object {}", owner->GetName());
	componentAudio = owner->GetComponent<ComponentAudioSource>();
	GameObject::GameObjectView children = owner->GetChildren();
	auto childWithRigid = std::find_if(std::begin(children),
		std::end(children),
		[](const GameObject* child)
		{
			return child->HasComponent<ComponentRigidBody>();
		});
	// not just assert, since it would crash on the next line
	if (childWithRigid == std::end(children))
	{
		LOG_ERROR("Expected one of {}'s children to have a ComponentRigidBody, but none was found", GetOwner());
		throw ComponentNotFoundException("ComponentRigidBody not found in children");
	}
	componentRigidBody = (*childWithRigid)->GetComponent<ComponentRigidBody>();
	transform = elevator->GetComponentInternal<ComponentTransform>();
	triggerEntrance = owner->GetComponent<ComponentRigidBody>();
	finalUpPos = 0;
	bixPrefab = App->GetModule<ModulePlayer>()->GetPlayer();
	playerTransform = bixPrefab->GetComponent<ComponentTransform>();
	currentTime = 0;
}

void ElevatorCore::Update(float deltaTime)
{
	float3 playerPos = playerTransform->GetGlobalPosition();

	if (activeState == ActiveActions::ACTIVE_PLAYER) 
	{
		if (positionState == PositionState::UP)
		{
			MoveDownElevator(true);
		}
		else 
		{
			MoveUpElevator(true);
		}
	}

	else if (activeState == ActiveActions::ACTIVE_AUTO)
	{
		if (positionState == PositionState::UP)
		{
			MoveDownElevator(false);
		}
		else
		{
			MoveUpElevator(false);
		}
	}

	else 
	{
		if (playerPos.y < finalPos && positionState == PositionState::DOWN)
		{
			activeState = ActiveActions::ACTIVE_AUTO;
			
		}

		/*
		if (isBossDead() && positionState == PositionState::UP)
		{
			activeState = ActiveActions::ACTIVE_AUTO;
		}
		*/

		else
		{
			if (currentTime >= 0)
			{
				currentTime -= deltaTime;
			}
		}

	}
}

void ElevatorCore::MoveUpElevator(bool isPlayerInside)
{
	float3 pos = transform->GetGlobalPosition();
	btVector3 triggerOrigin = triggerEntrance->GetRigidBodyOrigin();

	pos.y += 0.1f;
	float newYTrigger = triggerOrigin.getY() + 0.1f;
	triggerOrigin.setY(newYTrigger);

	transform->SetGlobalPosition(pos);
	transform->RecalculateLocalMatrix();
	transform->UpdateTransformMatrices();

	triggerEntrance->SetRigidBodyOrigin(triggerOrigin);
	elevator->GetComponentInternal<ComponentRigidBody>()->UpdateRigidBody();

	if (pos.y >= finalUpPos)
	{
		positionState = PositionState::UP;
		activeState = ActiveActions::INACTIVE;
		currentTime = coolDown;
		if (isPlayerInside)
		{
			EnableAllInteractions();
		}
	}

	if (isPlayerInside)
	{
		float3 playerPos = playerTransform->GetGlobalPosition();
		playerPos.y += 0.1f;

		playerTransform->SetGlobalPosition(playerPos);
		playerTransform->RecalculateLocalMatrix();
		playerTransform->UpdateTransformMatrices();

		bixPrefab->GetComponentInternal<ComponentRigidBody>()->UpdateRigidBody();
	}

}

void ElevatorCore::MoveDownElevator(bool isPlayerInside)
{
	float3 pos = transform->GetGlobalPosition();
	float3 playerPos = playerTransform->GetGlobalPosition();
	btVector3 triggerOrigin = triggerEntrance->GetRigidBodyOrigin();

	pos.y -= 0.1f;
	float newYTrigger = triggerOrigin.getY() - 0.1f;
	triggerOrigin.setY(newYTrigger);

	transform->SetGlobalPosition(pos);
	transform->RecalculateLocalMatrix();
	transform->UpdateTransformMatrices();

	triggerEntrance->SetRigidBodyOrigin(triggerOrigin);
	elevator->GetComponentInternal<ComponentRigidBody>()->UpdateRigidBody();

	if (pos.y <= finalPos)
	{
		positionState = PositionState::DOWN;
		activeState = ActiveActions::INACTIVE;
		currentTime = coolDown;
		if (isPlayerInside)
		{
			EnableAllInteractions();
		}
	}

	if (isPlayerInside)
	{
		float3 playerPos = playerTransform->GetGlobalPosition();
		playerPos.y -= 0.1f;

		playerTransform->SetGlobalPosition(playerPos);
		playerTransform->RecalculateLocalMatrix();
		playerTransform->UpdateTransformMatrices();

		bixPrefab->GetComponentInternal<ComponentRigidBody>()->UpdateRigidBody();
	}
}


void ElevatorCore::OnCollisionEnter(ComponentRigidBody* other)
{
	LOG_DEBUG("{} enters in CollisionEnter of ElevatorCore", other->GetOwner());
	if (!App->GetModule<ModuleScene>()->GetLoadedScene()->GetCombatMode())
	{
		if (other->GetOwner()->CompareTag("Player"))
		{

			PlayerActions currentAction = bixPrefab->GetComponent<PlayerManagerScript>()->GetPlayerState();
			bool isJumping = currentAction == PlayerActions::JUMPING ||
				currentAction == PlayerActions::DOUBLEJUMPING ||
				currentAction == PlayerActions::FALLING;

			if (!isJumping && currentTime <= 0)
			{
				//componentAnimation->SetParameter("IsActive", true);
				componentAudio->PostEvent(AUDIO::SFX::AMBIENT::SEWERS::BIGDOOR_OPEN);
				activeState = ActiveActions::ACTIVE_PLAYER;

				DisableAllInteractions();

			}
			
		}
	}
}

void ElevatorCore::OnCollisionExit(ComponentRigidBody* other)
{
	LOG_DEBUG("{} enters in CollisionExit of ElevatorCore", other->GetOwner());
	if (!App->GetModule<ModuleScene>()->GetLoadedScene()->GetCombatMode())
	{
		if (other->GetOwner()->CompareTag("Player"))
		{
			//componentAnimation->SetParameter("IsActive", false);
			//componentAudio->PostEvent(AUDIO::SFX::AMBIENT::SEWERS::BIGDOOR_CLOSE);
		}
	}
}

void ElevatorCore::DisableAllInteractions()
{
	//bixPrefab->SetParent(elevator);
	bixPrefab->GetComponentInternal<ComponentRigidBody>()->SetStatic(true);

	PlayerManagerScript* manager = bixPrefab->GetComponentInternal<PlayerManagerScript>();
	manager->ParalyzePlayer(true);
}

void ElevatorCore::EnableAllInteractions()
{
	//bixPrefab->SetParent(App->GetModule<ModuleScene>()->GetLoadedScene()->GetRoot());
	bixPrefab->GetComponentInternal<ComponentRigidBody>()->SetStatic(false);

	PlayerManagerScript* manager = bixPrefab->GetComponentInternal<PlayerManagerScript>();
	manager->ParalyzePlayer(false);
}