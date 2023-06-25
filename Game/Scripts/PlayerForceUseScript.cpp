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

#include "PlayerRotationScript.h"
#include "PlayerMoveScript.h"

REGISTERCLASS(PlayerForceUseScript);

PlayerForceUseScript::PlayerForceUseScript() : Script(), gameObjectAttached(nullptr),
gameObjectAttachedParent(nullptr), tag("Forceable"), distancePointGameObjectAttached(0.0f),
maxDistanceForce(20.0f), minDistanceForce(6.0f), maxTimeForce(15.0f), isForceActive(false),
currentTimeForce(0.0f), breakForce(false), componentAnimation(nullptr), componentAudioSource (nullptr)
{
	REGISTER_FIELD(maxDistanceForce, float);
	REGISTER_FIELD(maxTimeForce, float);
}

void PlayerForceUseScript::Start()
{
	componentAnimation = owner->GetComponent<ComponentAnimation>();
	componentAudioSource = owner->GetParent()->GetComponent<ComponentAudioSource>();

	currentTimeForce = maxTimeForce;

	rotationScript = owner->GetParent()->GetComponent<PlayerRotationScript>();
	moveScript = owner->GetParent()->GetComponent<PlayerMoveScript>();
}

void PlayerForceUseScript::Update(float deltaTime)
{
	const ModuleInput* input = App->GetModule<ModuleInput>();
	const ComponentTransform* transform = owner->GetComponent<ComponentTransform>();

	if (input->GetKey(SDL_SCANCODE_E) != KeyState::IDLE && !gameObjectAttached && currentTimeForce > 14.0f)
	{
		//componentAnimation->SetParameter("IsUsingForce", true);
		RaycastHit hit;
		Ray ray(transform->GetGlobalPosition(), transform->GetGlobalForward());
		LineSegment line(ray, 300);
		if (Physics::RaycastToTag(line, hit, owner, tag))
		{
			gameObjectAttached = hit.gameObject;
			ComponentTransform* hittedTransform = gameObjectAttached->GetComponent<ComponentTransform>();
			distancePointGameObjectAttached = transform->GetGlobalPosition().Distance(hittedTransform->GetGlobalPosition());

			if (distancePointGameObjectAttached > maxDistanceForce)
			{
				gameObjectAttached = nullptr;
				return;
			}
			else if (distancePointGameObjectAttached < minDistanceForce)
			{
				distancePointGameObjectAttached = minDistanceForce;
			}

			if (rotationScript)
			{
				lastHorizontalSensitivity = rotationScript->GetField<float>("RotationSensitivity")->getter();
				rotationScript->GetField<float>("RotationSensitivityHorizontal")->setter(lastHorizontalSensitivity / 2.0f);
				lastVerticalSensitivity = rotationScript->GetField<float>("RotationSensitivity")->getter();
				rotationScript->GetField<float>("RotationSensitivityVertical")->setter(lastVerticalSensitivity / 2.0f);
			}


			if (moveScript)
			{
				lastMoveSpeed = moveScript->GetField<float>("Speed")->getter();
				moveScript->GetField<float>("Speed")->setter(lastMoveSpeed / 2.0f);
			}

			ComponentRigidBody* rigidBody = gameObjectAttached->GetComponent<ComponentRigidBody>();
			rigidBody->SetKpForce(50.0f);
			rigidBody->SetKpTorque(50.0f);
		}
	}
	else if ((input->GetKey(SDL_SCANCODE_E) == KeyState::IDLE
		&& gameObjectAttached)
		|| currentTimeForce < 0.0f
		|| breakForce)
	{
		//componentAnimation->SetParameter("IsUsingForce", false);

		ComponentRigidBody* rigidBody = gameObjectAttached->GetComponent<ComponentRigidBody>();
		gameObjectAttached = nullptr;
		rigidBody->DisablePositionController();
		rigidBody->DisableRotationController();

		if (rotationScript)
		{
			rotationScript->GetField<float>("RotationSensitivityHorizontal")->setter(lastHorizontalSensitivity);
			rotationScript->GetField<float>("RotationSensitivityVertical")->setter(lastVerticalSensitivity);
		}

		if (moveScript)
		{
			moveScript->GetField<float>("Speed")->setter(lastMoveSpeed);
		}

		if (isForceActive)
		{
			componentAudioSource->PostEvent(AUDIO::SFX::PLAYER::ABILITIES::FORCE_STOP);
			isForceActive = false;
		}

		breakForce = false;
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
		// Get next position of the gameObject
		float3 nextPosition = transform->GetGlobalForward();
		nextPosition.Normalize();
		nextPosition *= distancePointGameObjectAttached;
		nextPosition += transform->GetGlobalPosition();

		float currentDistance = hittedTransform->GetGlobalPosition().Distance(nextPosition);

		if (std::abs(currentDistance) > 1.5f && currentTimeForce < 14.5f)
		{
			breakForce = true;
			currentTimeForce = 10;
			return;
		}

		// Get next rotation of game object
		ComponentTransform* parentTransform = owner->GetParent()->GetComponent<ComponentTransform>();

		Quat targetRotation =
			Quat::RotateFromTo(hittedTransform->GetGlobalForward(),
				(parentTransform->GetGlobalPosition() - hittedTransform->GetGlobalPosition()).Normalized());

		// Set position and rotation
		hittedRigidBody->SetPositionTarget(nextPosition);
		hittedRigidBody->SetRotationTarget(targetRotation);

		currentTimeForce -= deltaTime;
	}
	else if (currentTimeForce < maxTimeForce)
	{
		currentTimeForce = std::min(maxTimeForce, currentTimeForce + (deltaTime * 4));
	}
}