
#include "Application.h"

#include "ModulePlayer.h"
#include "ModuleScene.h"
#include "ModuleCamera.h"
#include "Scene/Scene.h"
#include "ModuleInput.h"

#include "Camera/Camera.h"
#include "Camera/CameraGameObject.h"
#include "Components/ComponentCamera.h"
#include "Components/ComponentPlayer.h"
#include "Components/ComponentMeshCollider.h"
#include "GameObject/GameObject.h"

#include "DataStructures/Quadtree.h"

#include "Components/ComponentTransform.h"

ModulePlayer::ModulePlayer(): cameraPlayer(nullptr), player(nullptr),componentPlayer(nullptr) {};

ModulePlayer::~ModulePlayer() {
};

bool ModulePlayer::Init()
{
	return true;
}

bool ModulePlayer::Start()
{
	//Initialize the player
	LoadNewPlayer();
	return true;
}

update_status ModulePlayer::PreUpdate()
{
	if (player && !componentPlayer->IsStatic() && App->camera->GetSelectedPosition() == 0)
	{
		Move();
		Rotate();
	}
	return update_status::UPDATE_CONTINUE;
}

update_status ModulePlayer::Update()
{
	player->Update();
	ComponentTransform* trans = static_cast<ComponentTransform*>(player->GetComponent(ComponentType::TRANSFORM));
	trans->UpdateTransformMatrices();
	return update_status::UPDATE_CONTINUE;
}

GameObject* ModulePlayer::GetPlayer()
{
	return player.get();
}

void ModulePlayer::SetPlayer(std::unique_ptr<GameObject> newPlayer)
{
	player = std::move(newPlayer);
	componentPlayer = static_cast<ComponentPlayer*>(player->GetComponent(ComponentType::PLAYER));
}

Camera* ModulePlayer::GetCameraPlayer()
{
	return cameraPlayer;
}

void ModulePlayer::Move()
{
	float deltaTime = (App->GetDeltaTime() < 1.f)?App->GetDeltaTime():1.f;
	ComponentTransform* trans = static_cast<ComponentTransform*>(player->GetComponent(ComponentType::TRANSFORM));
	ComponentMeshCollider* collider = static_cast<ComponentMeshCollider*>(player->GetComponent(ComponentType::MESHCOLLIDER));
	float3 position = trans->GetPosition();

	math::vec points[8];
	trans->GetObjectOBB().GetCornerPoints(points);
	std::vector<float3> frontPoints = { points[1], points[3], points[5], points[7] };
	std::vector<float3> backPoints = { points[0], points[2], points[4], points[6] };
	std::vector<float3> leftPoints = { points[4], points[6], points[5],  points[7] };
	std::vector<float3> rightPoints = { points[0], points[2], points[1], points[3] };

	float3 direction = (points[1] - points[0]).Normalized();
	float3 sideDirection = (points[4] - points[0]).Normalized();

	//Forward
	if (App->input->GetKey(SDL_SCANCODE_W) != KeyState::IDLE && !collider->IsColliding(frontPoints, direction, speed * deltaTime * 1.1f, trans->GetLocalAABB().Size().y * 0.15f))
	{
		position += trans->GetGlobalForward().Normalized() * speed * deltaTime;
		trans->SetPosition(position);

		trans->UpdateTransformMatrices();
		trans->GetObjectOBB().GetCornerPoints(points);
		backPoints = { points[0], points[2], points[4], points[6] };
		leftPoints = { points[4], points[6], points[5],  points[7] };
		rightPoints = { points[0], points[2], points[1], points[3] };
	}

	//Backward
	if (App->input->GetKey(SDL_SCANCODE_S) != KeyState::IDLE && !collider->IsColliding(backPoints, -direction, speed * deltaTime * 1.1f, trans->GetLocalAABB().Size().y * 0.15f))
	{
		position += -trans->GetGlobalForward().Normalized() * speed * deltaTime;
		trans->SetPosition(position);

		trans->UpdateTransformMatrices();
		trans->GetObjectOBB().GetCornerPoints(points);
		leftPoints = { points[4], points[6], points[5],  points[7] };
		rightPoints = { points[0], points[2], points[1], points[3] };
	}

	//Left
	if (App->input->GetKey(SDL_SCANCODE_A) != KeyState::IDLE && !collider->IsColliding(leftPoints, -sideDirection, speed  * deltaTime * 1.1f, trans->GetLocalAABB().Size().y * 0.15f))
	{
		position += trans->GetGlobalRight().Normalized() * speed*2/3 * deltaTime;
		trans->SetPosition(position);

		trans->UpdateTransformMatrices();
		trans->GetObjectOBB().GetCornerPoints(points);
		rightPoints = { points[0], points[2], points[1], points[3] };
	}

	//Right
	if (App->input->GetKey(SDL_SCANCODE_D) != KeyState::IDLE && !collider->IsColliding(rightPoints, sideDirection, speed * deltaTime * 1.1f, trans->GetLocalAABB().Size().y * 0.15f))
	{
		position += -trans->GetGlobalRight().Normalized() * speed*2/3 * deltaTime;
		trans->SetPosition(position);

		trans->UpdateTransformMatrices();
	}
}

void ModulePlayer::Rotate()
{
	if (App->input->GetMouseMotion().x != 0)
	{
		float deltaTime = App->GetDeltaTime();
		ComponentTransform* trans = static_cast<ComponentTransform*>(player->GetComponent(ComponentType::TRANSFORM));
		float3 newRot = trans->GetRotationXYZ();
		newRot.y += - App->input->GetMouseMotion().x * deltaTime;
		trans->SetRotation(newRot);
		trans->UpdateTransformMatrices();


		//Corroborate that you don't fuse with a wall
		ComponentMeshCollider* collider = static_cast<ComponentMeshCollider*>(player->GetComponent(ComponentType::MESHCOLLIDER));
		math::vec points[8];
		trans->GetObjectOBB().GetCornerPoints(points);
		std::vector<float3> frontPoints = { points[1], points[3], points[5], points[7] };
		float3 direction = (points[1] - points[0]).Normalized();
		if (collider->IsColliding(frontPoints, -direction, trans->GetLocalAABB().Size().z * 0.7))
		{
			float deltaTime = App->GetDeltaTime();
			ComponentTransform* trans = static_cast<ComponentTransform*>(player->GetComponent(ComponentType::TRANSFORM));
			float3 newRot = trans->GetRotationXYZ();
			newRot.y += App->input->GetMouseMotion().x * deltaTime;
			trans->SetRotation(newRot);
			trans->UpdateTransformMatrices();
		}

	}
}

void ModulePlayer::LoadNewPlayer()
{
	std::vector<GameObject*> cameras = App->scene->GetLoadedScene()->GetSceneCameras();
	for (GameObject* camera : cameras)
	{
		if (camera->GetParent()->GetComponent(ComponentType::PLAYER))
		{
			SetPlayer(camera->GetParent()->GetParent()->RemoveChild(camera->GetParent()));
			cameraPlayer = static_cast<ComponentCamera*>(camera->GetComponent(ComponentType::CAMERA))->GetCamera();
			App->scene->RemoveGameObjectAndChildren(camera->GetParent());
			App->camera->SetSelectedCamera(0);
			if(componentPlayer->HaveMouseActivated()) 
			{
				App->input->SetShowCursor(true);
			}
			else 
			{
				App->input->SetShowCursor(false);
			}
		}
	}
}


bool ModulePlayer::IsStatic()
{
	return componentPlayer->IsStatic();
}