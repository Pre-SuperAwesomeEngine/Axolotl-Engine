#include "PlayerMoveScript.h"

#include "ModuleInput.h"
#include "ModulePlayer.h"
#include "Camera/Camera.h"
#include "Geometry/Frustum.h"

#include "Components/ComponentRigidBody.h"
#include "Components/ComponentTransform.h"
#include "Components/ComponentAudioSource.h"
#include "Components/ComponentAnimation.h"
#include "Components/ComponentScript.h"

#include "Auxiliar/Audio/AudioData.h"

#include "../Scripts/PlayerManagerScript.h"
#include "PlayerForceUseScript.h"

REGISTERCLASS(PlayerMoveScript);

PlayerMoveScript::PlayerMoveScript() : Script(), componentTransform(nullptr),
	componentAudio(nullptr), playerState(PlayerActions::IDLE), componentAnimation(nullptr),
	dashForce(2000.0f), nextDash(0.0f), isDashing(false), canDash(true), playerManager(nullptr), isParalized(false)
{
	REGISTER_FIELD(dashForce, float);
	REGISTER_FIELD(canDash, bool);
	REGISTER_FIELD(isParalized, bool)
}

void PlayerMoveScript::Start()
{
	componentTransform = owner->GetComponent<ComponentTransform>();
	componentAudio = owner->GetComponent<ComponentAudioSource>();
	componentAnimation = owner->GetComponent<ComponentAnimation>();
	playerManager = owner->GetComponent<PlayerManagerScript>();
	forceScript = owner->GetComponent<PlayerForceUseScript>();

	rigidBody = owner->GetComponent<ComponentRigidBody>();
	btRb = rigidBody->GetRigidBody();

	camera = App->GetModule<ModulePlayer>()->GetCameraPlayer();
	input = App->GetModule<ModuleInput>();

	cameraFrustum = *camera->GetFrustum();

	previousMovements = 0;
	currentMovements = 0;
}

void PlayerMoveScript::PreUpdate(float deltaTime)
{
	if (!forceScript->IsForceActive())
	{
		Move(deltaTime);
	}
	
}

void PlayerMoveScript::Move(float deltaTime)
{
	btRb->setAngularFactor(btVector3(0.0f, 0.0f, 0.0f));

	btVector3 movement(0, 0, 0);
	float3 totalDirection = float3::zero;
	
	float newSpeed = playerManager->GetPlayerSpeed();
	bool shiftPressed = false;

	previousMovements = currentMovements;
	currentMovements = 0;

	//run
	if (isParalized)
	{
		return;
	}

	// Run
	/*
	if (input->GetKey(SDL_SCANCODE_LSHIFT) != KeyState::IDLE)
	{
		newSpeed *= 2;
		shiftPressed = true;
	}
	*/

	// Forward
	if (input->GetKey(SDL_SCANCODE_W) != KeyState::IDLE)
	{
		if (playerState == PlayerActions::IDLE)
		{
			componentAudio->PostEvent(AUDIO::SFX::PLAYER::LOCOMOTION::FOOTSTEPS_WALK);
			componentAnimation->SetParameter("IsRunning", true);
			playerState = PlayerActions::WALKING;
		}

		totalDirection += cameraFrustum.Front().Normalized();
		currentMovements |= MovementFlag::W_DOWN;
	}

	// Back
	if (input->GetKey(SDL_SCANCODE_S) != KeyState::IDLE)
	{
		if (playerState == PlayerActions::IDLE)
		{
			componentAudio->PostEvent(AUDIO::SFX::PLAYER::LOCOMOTION::FOOTSTEPS_WALK);
			componentAnimation->SetParameter("IsRunning", true);
			playerState = PlayerActions::WALKING;
		}
		totalDirection += -cameraFrustum.Front().Normalized();
		currentMovements |= MovementFlag::S_DOWN;
	}

	// Right
	if (input->GetKey(SDL_SCANCODE_D) != KeyState::IDLE)
	{
		if (playerState == PlayerActions::IDLE)
		{
			componentAudio->PostEvent(AUDIO::SFX::PLAYER::LOCOMOTION::FOOTSTEPS_WALK);
			componentAnimation->SetParameter("IsRunning", true);
			playerState = PlayerActions::WALKING;
		}

		totalDirection += cameraFrustum.WorldRight().Normalized();
		currentMovements |= MovementFlag::D_DOWN;
	}

	// Left
	if (input->GetKey(SDL_SCANCODE_A) != KeyState::IDLE)
	{
		if (playerState == PlayerActions::IDLE)
		{
			componentAudio->PostEvent(AUDIO::SFX::PLAYER::LOCOMOTION::FOOTSTEPS_WALK);
			componentAnimation->SetParameter("IsRunning", true);
			playerState = PlayerActions::WALKING;
		}

		totalDirection += -cameraFrustum.WorldRight().Normalized();
		currentMovements |= MovementFlag::A_DOWN;
	}

	if (previousMovements ^ currentMovements)
	{
		cameraFrustum = *camera->GetFrustum();
	}

	if (!totalDirection.IsZero())
	{
		totalDirection.y = 0;
		totalDirection = totalDirection.Normalized();

		MoveRotate(totalDirection, deltaTime);

		movement = btVector3(totalDirection.x, totalDirection.y, totalDirection.z) * deltaTime * newSpeed;
	}

	if (input->GetKey(SDL_SCANCODE_W) == KeyState::IDLE &&
		input->GetKey(SDL_SCANCODE_A) == KeyState::IDLE &&
		input->GetKey(SDL_SCANCODE_S) == KeyState::IDLE &&
		input->GetKey(SDL_SCANCODE_D) == KeyState::IDLE)
	{
		if (playerState == PlayerActions::WALKING)
		{
			componentAudio->PostEvent(AUDIO::SFX::PLAYER::LOCOMOTION::FOOTSTEPS_WALK_STOP);
			componentAnimation->SetParameter("IsRunning", false);
			playerState = PlayerActions::IDLE;
		}

	}

	// Dash
	if (input->GetKey(SDL_SCANCODE_C) == KeyState::DOWN && canDash)
	{
		/*
		if (!isDashing)
		{
			componentAnimation->SetParameter("IsDashing", true);
			componentAudio->PostEvent(AUDIO::SFX::PLAYER::LOCOMOTION::FOOTSTEPS_WALK_STOP);
			componentAudio->PostEvent(AUDIO::SFX::PLAYER::LOCOMOTION::DASH);

			if (!movement.isZero())
			{
				if (shiftPressed)
				{
					movement /= 2;
				}
				btRb->setLinearVelocity(movement);
				btRb->applyCentralImpulse(movement.normalized() * dashForce);
				isDashing = true;
			}
		}

		if (nextDash == 0)
		{
			canDash = false;
			nextDash = 3000 + static_cast<float>(SDL_GetTicks());
		}
		*/
	}

	else
	{
		componentAnimation->SetParameter("IsRolling", false);

		btVector3 currentVelocity = btRb->getLinearVelocity();
		btVector3 newVelocity(movement.getX(), currentVelocity.getY(), movement.getZ());

		if (!isDashing)
		{
			btRb->setLinearVelocity(newVelocity);
		}
		else
		{
			if (math::Abs(currentVelocity.getX()) < dashForce / 100.f && math::Abs(currentVelocity.getZ()) < dashForce / 100.f)
			{
				btRb->setLinearVelocity(newVelocity);
				isDashing = false;
			}
		}
	}

	// Cooldown Dash
	if (nextDash != 0 && nextDash < SDL_GetTicks())
	{
		canDash = true;
		nextDash = 0;
	}
}

void PlayerMoveScript::MoveRotate(const float3& targetDirection, float deltaTime)
{
	btTransform worldTransform = btRb->getWorldTransform();
	Quat rot = Quat::LookAt(componentTransform->GetGlobalForward().Normalized(), targetDirection, float3::unitY, float3::unitY);
	Quat rotation = componentTransform->GetGlobalRotation();
	Quat targetRotation = rot * componentTransform->GetGlobalRotation();

	Quat rotationError = targetRotation * rotation.Normalized().Inverted();
	rotationError.Normalize();

	if (!rotationError.Equals(Quat::identity, 0.05f))
	{
		float3 axis;
		float angle;
		rotationError.ToAxisAngle(axis, angle);
		axis.Normalize();

		float3 velocityRotation = axis * angle * playerManager->GetPlayerRotationSpeed();
		Quat angularVelocityQuat(velocityRotation.x, velocityRotation.y, velocityRotation.z, 0.0f);
		Quat wq_0 = angularVelocityQuat * rotation;

		float deltaValue = 0.5f * deltaTime;
		Quat deltaRotation = Quat(deltaValue * wq_0.x,
			deltaValue * wq_0.y,
			deltaValue * wq_0.z,
			deltaValue * wq_0.w);

		if (deltaRotation.Length() > rotationError.Length())
		{
			worldTransform.setRotation({ targetRotation.x,
				targetRotation.y,
				targetRotation.z,
				targetRotation.w });
		}

		else
		{
			Quat nextRotation(rotation.x + deltaRotation.x,
				rotation.y + deltaRotation.y,
				rotation.z + deltaRotation.z,
				rotation.w + deltaRotation.w);
			nextRotation.Normalize();

			worldTransform.setRotation({ nextRotation.x,
				nextRotation.y,
				nextRotation.z,
				nextRotation.w });
		}
	}

	btRb->setWorldTransform(worldTransform);
	btRb->getMotionState()->setWorldTransform(worldTransform);
}

bool PlayerMoveScript::GetIsParalized() const
{
	return isParalized;
}

void PlayerMoveScript::SetIsParalized(bool isParalized)
{
	this->isParalized = isParalized;
}