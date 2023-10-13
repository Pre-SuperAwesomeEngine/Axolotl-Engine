#include "PlayerForceUseScript.h"

#include "Application.h"
#include "ModuleInput.h"
#include "ModuleScene.h"

#include "Components/ComponentTransform.h"
#include "Components/ComponentRigidBody.h"
#include "Components/ComponentCamera.h"
#include "Components/ComponentScript.h"
#include "Components/ComponentAnimation.h"
#include "Components/ComponentAudioSource.h"
#include "DataModels/Camera/CameraGameObject.h"

#include "Scene/Scene.h"
#include "DataStructures/Quadtree.h"
#include "Physics/Physics.h"

#include "MathGeoLib/Include/Geometry/Ray.h"
#include "Auxiliar/Audio/AudioData.h"

#include "../Scripts/PlayerRotationScript.h"
#include "../Scripts/PlayerManagerScript.h"
#include "../Scripts/CameraControllerScript.h"
#include "../Scripts/PlayerMoveScript.h"


#include "AxoLog.h"

REGISTERCLASS(PlayerForceUseScript);

PlayerForceUseScript::PlayerForceUseScript() : Script(), gameObjectAttached(nullptr),
	gameObjectAttachedParent(nullptr), tag("Forceable"), tag2("ForceableDoors"), distancePointGameObjectAttached(0.0f),
	maxDistanceForce(20.0f), minDistanceForce(1.f), maxTimeForce(20.0f), isForceActive(false),
	currentTimeForce(0.0f), breakForce(false), componentAnimation(nullptr), componentAudioSource(nullptr),
	playerManagerScript(nullptr)
{
	REGISTER_FIELD(maxDistanceForce, float);
	REGISTER_FIELD(minDistanceForce, float);
	REGISTER_FIELD(maxTimeForce, float);

}

void PlayerForceUseScript::Start()
{
	componentAudioSource = owner->GetComponent<ComponentAudioSource>();
	componentAnimation = owner->GetComponent<ComponentAnimation>();
	currentTimeForce = maxTimeForce;

	rotationHorizontalScript = owner->GetComponent<PlayerRotationScript>();
	playerManagerScript = owner->GetComponent<PlayerManagerScript>();
	moveScript = owner->GetComponent<PlayerMoveScript>();

	input = App->GetModule<ModuleInput>();
	transform = owner->GetComponent<ComponentTransform>();
	rigidBody = owner->GetComponent<ComponentRigidBody>();
}

void PlayerForceUseScript::Update(float deltaTime)
{
	if (input->GetKey(SDL_SCANCODE_E) != KeyState::IDLE && !gameObjectAttached && currentTimeForce > 14.0f)
	{
		componentAnimation->SetParameter("IsStartingForce", true);
		componentAnimation->SetParameter("IsStoppingForce", false);
		RaycastHit hit;
		btVector3 rigidBodyOrigin = rigidBody->GetRigidBodyOrigin();
		rigidBody->SetGravity(btVector3(0.0, 0.0, 0.0));
		float3 origin = float3(rigidBodyOrigin.getX(), rigidBodyOrigin.getY(), rigidBodyOrigin.getZ());
		int raytries = 0;
		
		while (gameObjectAttached == nullptr && raytries < 4)
		{
			Ray ray(origin + float3(0.f, 1.f * static_cast<float>(raytries), 0.f), transform->GetGlobalForward());
			LineSegment line(ray, 300);
			raytries++;

			if (Physics::RaycastToTag(line, hit, owner, tag) || Physics::RaycastToTag(line, hit, owner, tag2))
			{
				gameObjectAttached = hit.gameObject;
				ComponentTransform* hittedTransform = gameObjectAttached->GetComponent<ComponentTransform>();
				distancePointGameObjectAttached = transform->GetGlobalPosition().Distance(hit.hitPoint);
				ComponentRigidBody* rigidBody = gameObjectAttached->GetComponent<ComponentRigidBody>();
				objectStaticness = rigidBody->IsStatic();
				rigidBody->SetIsStatic(false);
				offsetFromPickedPoint = hittedTransform->GetGlobalPosition() - hit.hitPoint;
				pickedRotation = hittedTransform->GetGlobalRotation();
				pickedPlayerPosition = owner->GetComponent<ComponentTransform>()->GetGlobalPosition();

				if (gameObjectAttached->GetTag() == "ForceableDoors" && !rigidBody->IsTrigger())
				{
					rigidBody->SetIsTrigger(true);
				}

				if (distancePointGameObjectAttached > maxDistanceForce)
				{
					gameObjectAttached = nullptr;
					return;
				}

				else if (distancePointGameObjectAttached < minDistanceForce)
				{
					distancePointGameObjectAttached = minDistanceForce;
				}

				if (rotationHorizontalScript)
				{
					lastHorizontalSensitivity = rotationHorizontalScript->GetHorizontalSensitivity();
					rotationHorizontalScript->SetHorizontalSensitivity(lastHorizontalSensitivity / 2.0f);
					lastVerticalSensitivity = rotationHorizontalScript->GetVerticalSensitivity();
					rotationHorizontalScript->SetVerticalSensitivity(lastVerticalSensitivity / 2.0f);
				}

				if (playerManagerScript)
				{
					lastMoveSpeed = playerManagerScript->GetPlayerSpeed();
					playerManagerScript->SetPlayerSpeed(lastMoveSpeed / 2.0f);
				}

				rigidBody->SetKpForce(50.0f);
				rigidBody->SetKpTorque(50.0f);
			}
		}
	}
	else if ((input->GetKey(SDL_SCANCODE_E) == KeyState::IDLE
		&& gameObjectAttached)
		|| currentTimeForce < 0.0f
		|| breakForce)
	{
		rigidBody->SetGravity(btVector3(0.0, -150.0, 0.0));
		ComponentRigidBody* rigidBody = gameObjectAttached->GetComponent<ComponentRigidBody>();
		if (gameObjectAttached->GetTag() == "ForceableDoors" && rigidBody->IsTrigger())
		{
			rigidBody->SetIsTrigger(false);
		}
		gameObjectAttached = nullptr;
		rigidBody->DisablePositionController();
		rigidBody->DisableRotationController();
		rigidBody->SetIsStatic(objectStaticness);

		if (playerManagerScript)
		{
			playerManagerScript->SetPlayerSpeed(lastMoveSpeed);
			rotationHorizontalScript->SetHorizontalSensitivity(lastHorizontalSensitivity);
			rotationHorizontalScript->SetVerticalSensitivity(lastVerticalSensitivity);
		}

		if (isForceActive)
		{
			componentAudioSource->PostEvent(AUDIO::SFX::PLAYER::ABILITIES::FORCE_STOP);
			isForceActive = false;
		}

		breakForce = false;
	}

	if (input->GetKey(SDL_SCANCODE_E) == KeyState::IDLE)
	{
		rigidBody->SetGravity(btVector3(0.0, -150.0, 0.0));
		componentAnimation->SetParameter("IsStoppingForce", true);
		componentAnimation->SetParameter("IsStartingForce", false);
	}

	if (gameObjectAttached)
	{
		if (!isForceActive)
		{
			componentAudioSource->PostEvent(AUDIO::SFX::PLAYER::ABILITIES::FORCE_USE);
			isForceActive = true;
		}
		ComponentRigidBody* hittedRigidBody = gameObjectAttached->GetComponent<ComponentRigidBody>();
		ComponentTransform* hittedTransform = gameObjectAttached->GetComponent<ComponentTransform>();

		if (input->IsMouseWheelScrolled())
		{
			distancePointGameObjectAttached += (input->GetMouseWheel().y) / 3.0f;
			distancePointGameObjectAttached = std::min(distancePointGameObjectAttached, maxDistanceForce);
			distancePointGameObjectAttached = std::max(distancePointGameObjectAttached, minDistanceForce);
		}
		// Get next rotation of game object
		Quat targetRotation =
			Quat::RotateFromTo(hittedTransform->GetGlobalForward(),
				(transform->GetGlobalPosition() - hittedTransform->GetGlobalPosition()).Normalized());

		// Set rotation
		hittedRigidBody->SetRotationTarget(targetRotation);
		
		// Get next position of the gameObject
		float3 nextPosition = transform->GetGlobalForward();
		nextPosition.Normalize();
		nextPosition *= distancePointGameObjectAttached;
		nextPosition += transform->GetGlobalPosition();
		nextPosition += offsetFromPickedPoint;

		nextPosition.y = hittedTransform->GetGlobalPosition().y;

		float3 currentDistance = hittedTransform->GetGlobalPosition() - nextPosition;
		
		if (std::abs(currentDistance.x) > 2 && std::abs(currentDistance.z) > 2 && currentTimeForce < 14.5f)
		{
			breakForce = true;
			currentTimeForce = 10;
			return;
		}

		float difX = pickedPlayerPosition.x - owner->GetComponent<ComponentTransform>()->GetGlobalPosition().x;
		float difY = pickedPlayerPosition.y - owner->GetComponent<ComponentTransform>()->GetGlobalPosition().y;
		float difZ = pickedPlayerPosition.z - owner->GetComponent<ComponentTransform>()->GetGlobalPosition().z;
		if (abs(difX) + abs(difY) + abs(difZ) > 0.2f)
		{
			breakForce = true;
			currentTimeForce = 10;
			return;
		}

		InputMethod inputMethod = input->GetCurrentInputMethod();
		if (inputMethod == InputMethod::KEYBOARD)
		{
			float2 mouseMotion = input->GetMouseMotion();
			nextPosition.y -= mouseMotion.y * 0.2f * deltaTime;
		}
		else if (inputMethod == InputMethod::GAMEPAD)
		{
			// We divide by 1000 (*0.001) to move at aprox same speed as if we where using the mouse
			Sint16 verticalMovement = input->GetLeftJoystickMovement().verticalMovement * 0.001f;
			nextPosition.y -= verticalMovement * 0.2f * deltaTime;
		}

		// Set position
		hittedRigidBody->SetPositionTarget(nextPosition);
		
		currentTimeForce -= deltaTime;
	}
	else if (currentTimeForce < maxTimeForce)
	{
		currentTimeForce = std::min(maxTimeForce, currentTimeForce + (deltaTime * 4));
	}
}