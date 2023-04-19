#include "ComponentRigidBody.h"
#include "ComponentTransform.h"
#include "ComponentMockState.h"
#include "ComponentTransform.h"

#include "ModuleScene.h"
#include "Scene/Scene.h"
#include "DataStructures/Quadtree.h"
#include "Geometry/Frustum.h"
#include "Math/float3x3.h"

#include "GameObject/GameObject.h"
#include "Application.h"

#include "FileSystem/Json.h"

#include "Geometry/LineSegment.h"
#include "Geometry/Ray.h"
#include "Physics/Physics.h"

#include <iostream>


ComponentRigidBody::ComponentRigidBody(bool active, GameObject* owner)
	: Component(ComponentType::RIGIDBODY, active, owner, true),
	isKinematic(true), mass(1.0f), g(9.81f), v0(float3(0.0f, 0.0f, 0.0f))
{
}

ComponentRigidBody::~ComponentRigidBody()
{
}

void ComponentRigidBody::Update()
{
//#ifndef ENGINE
//	if (isKinematic)
//	{
//		float3 currentPos = transform->GetPosition();
//		Ray ray(currentPos, -float3::unitY);
//		LineSegment line(ray, App->scene->GetLoadedScene()->GetRootQuadtree()->GetBoundingBox().Size().y);
//		RaycastHit hit;
//
//		bool hasHit = Physics::Raycast(line, hit);
//		float3 x;
//		float t = App->GetDeltaTime();
//		float3 x0 = currentPos;
//		float3 a = float3(0.0f, -0.5 * g * t * t, 0.0f);
//
//		v0.y -= g * t;
//		x = x0 + v0 * t + a;
//
//		float verticalDistanceToFeet = math::Abs(transform->GetEncapsuledAABB().MinY() - x0.y);
//		if (hasHit && x.y <= hit.hitPoint.y + verticalDistanceToFeet + (x-x0).Length())
//
//		{
//
//			x = hit.hitPoint + float3(0.0f, verticalDistanceToFeet,0.0f);
//			v0 = float3::zero;
//			
//			if (hit.gameObject != nullptr && hit.gameObject->GetComponent(ComponentType::MOCKSTATE) != nullptr)
//			{
//				ComponentMockState* mockState = static_cast<ComponentMockState*>(hit.gameObject->GetComponent(ComponentType::MOCKSTATE));
//
//				if (mockState->GetIsWinState())
//				{
//					//TODO: win state
//					std::string sceneName = mockState->GetSceneName();
//					App->scene->SetSceneToLoad("Lib/Scenes/" + sceneName + ".axolotl");
//				}
//				else if (mockState->GetIsFailState())
//				{
//					//TODO fail state
//				}
//			}
//		}
//
//		transform->SetPosition(x);
//	}
//#endif
#ifndef ENGINE
	if (isKinematic)
	{
		float3 currentPos = transform->GetPosition();
		Ray ray(currentPos, -float3::unitY);
		LineSegment line(ray, App->scene->GetLoadedScene()->GetRootQuadtree()->GetBoundingBox().Size().y);
		RaycastHit hit;

		bool hasHit = Physics::Raycast(line, hit);
		float3 x;
		float t = App->GetDeltaTime();
		float3 x0 = currentPos;
		float3 a = float3(0.0f, -0.5 * g * t * t, 0.0f);

		v0.y -= g * t;
		x = x0 + v0 * t + a;

		float verticalDistanceToFeet = math::Abs(transform->GetEncapsuledAABB().MinY() - x0.y);
		if (hasHit && x.y <= hit.hitPoint.y + verticalDistanceToFeet + (x - x0).Length())

		{

			x = hit.hitPoint + float3(0.0f, verticalDistanceToFeet, 0.0f);
			v0 = float3::zero;

			if (hit.gameObject != nullptr && hit.gameObject->GetComponent(ComponentType::MOCKSTATE) != nullptr)
			{
				ComponentMockState* mockState = static_cast<ComponentMockState*>(hit.gameObject->GetComponent(ComponentType::MOCKSTATE));

				if (mockState->GetIsWinState())
				{
					//TODO: win state
					std::string sceneName = mockState->GetSceneName();
					App->scene->SetSceneToLoad("Lib/Scenes/" + sceneName + ".axolotl");
				}
				else if (mockState->GetIsFailState())
				{
					//TODO fail state
				}
			}
		}

		transform->SetPosition(x);
	}
#endif
}

void ComponentRigidBody::Draw()
{
	
}

void ComponentRigidBody::AddForce(const float3& force, ForceMode mode)
{
	switch (mode)
	{
	case ForceMode::Force:
		externalForce += force / mass;
		break;
	case ForceMode::Acceleration:
		externalForce += force;
		break;
	case ForceMode::Impulse:
		v0 += force / mass;
		break;
	case ForceMode::VelocityChange:
		v0 += force;
		break;
	}
}

void ComponentRigidBody::AddTorque(const float3& torque, ForceMode mode)
{
	switch (mode)
	{
	case ForceMode::Force:
		externalTorque += torque / mass;
		break;
	case ForceMode::Acceleration:
		externalTorque += torque;
		break;
	case ForceMode::Impulse:
		w0 += torque / inertiaTensor;
		break;
	case ForceMode::VelocityChange:
		w0 += torque;
		break;
	}
}


void ComponentRigidBody::ApplyForce()
{
	if (usePositionController)
	{
		float3 position = transform->GetPosition();
		float deltaTime = App->GetDeltaTime();

		float3 positionError = targetPosition - position;
		float3 velocityPosition = positionError * KpForce + externalForce;
		float3 nextPos = position + velocityPosition * deltaTime;

		transform->SetPosition(nextPos);
		externalForce = float3::zero;
	}
}

void ComponentRigidBody::ApplyTorque()
{
	if (useRotationController)
	{

		Quat rotation = transform->GetRotation().RotatePart().ToQuat();
		float deltaTime = App->GetDeltaTime();

		Quat rotationError = targetRotation * rotation.Normalized().Inverted();
		rotationError.Normalize();

		float3 axis;
		float angle;
		rotationError.ToAxisAngle(axis, angle);
		axis.Normalize();

		float3 velocityRotation = axis * angle * KpTorque + externalTorque;
		Quat angularVelocityQuat(velocityRotation.x, velocityRotation.y, velocityRotation.z, 0.0f);
		Quat wq_0 = angularVelocityQuat * rotation;

		float deltaValue = 0.5f * deltaTime;
		Quat deltaRotation = Quat(deltaValue * wq_0.x, deltaValue * wq_0.y, deltaValue * wq_0.z, deltaValue * wq_0.w);

		Quat nextRotation(rotation.x + deltaRotation.x,
			rotation.y + deltaRotation.y,
			rotation.z + deltaRotation.z,
			rotation.w + deltaRotation.w);
		nextRotation.Normalize();

		float4x4 nextRotationMatrix = float4x4::FromQuat(nextRotation);
		transform->SetRotation(nextRotationMatrix);
		externalTorque = float3::zero;
	}
}

void ComponentRigidBody::SaveOptions(Json& meta)
{
	// Do not delete these
	meta["type"] = GetNameByType(type).c_str();
	meta["active"] = (bool)active;
	meta["removed"] = (bool)canBeRemoved;
}

void ComponentRigidBody::LoadOptions(Json& meta)
{
	// Do not delete these
	type = GetTypeByName(meta["type"]);
	active = (bool)meta["active"];
	canBeRemoved = (bool)meta["removed"];
}
