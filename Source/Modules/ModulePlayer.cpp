
#include "Application.h"

#include "ModuleCamera.h"
#include "ModuleEditor.h"
#include "ModuleInput.h"
#include "ModulePlayer.h"
#include "ModuleRender.h"
#include "ModuleScene.h"
#include "Scene/Scene.h"

#include "Camera/Camera.h"
#include "Camera/CameraGameObject.h"
#include "Components/ComponentCamera.h"
#include "Components/ComponentMeshCollider.h"
#include "Components/ComponentPlayer.h"
#include "Components/ComponentRigidBody.h"
#include "GameObject/GameObject.h"

#include "DataStructures/Quadtree.h"

#include "Components/ComponentTransform.h"

ModulePlayer::ModulePlayer() : cameraPlayer(nullptr), player(nullptr), componentPlayer(nullptr), speed(3){};

ModulePlayer::~ModulePlayer(){};

bool ModulePlayer::Start()
{
	// Initialize the player
#ifndef ENGINE
	LoadNewPlayer();
#endif // GAMEMODE
	return true;
}

update_status ModulePlayer::PreUpdate()
{
	CheckIfActivateMouse();

	return update_status::UPDATE_CONTINUE;
}

GameObject* ModulePlayer::GetPlayer()
{
	return player;
}

void ModulePlayer::SetPlayer(GameObject* newPlayer)
{
	player = newPlayer;
	componentPlayer = static_cast<ComponentPlayer*>(player->GetComponent(ComponentType::PLAYER));
}

Camera* ModulePlayer::GetCameraPlayer()
{
	return cameraPlayer;
}

void ModulePlayer::LoadNewPlayer()
{
	ModuleScene* scene = App->GetModule<ModuleScene>();
	Scene* loadedScene = scene->GetLoadedScene();
	ModuleEditor* editor = App->GetModule<ModuleEditor>();
	std::vector<ComponentCamera*> cameras = loadedScene->GetSceneCameras();
	for (ComponentCamera* camera : cameras)
	{
		GameObject* parentOfOwner = camera->GetOwner()->GetParent();
		if (parentOfOwner->GetComponent(ComponentType::PLAYER))
		{
			SetPlayer(parentOfOwner);
			cameraPlayer = camera->GetCamera();
#ifdef ENGINE
			cameraPlayer->SetAspectRatio(editor->GetAvailableRegion().first / editor->GetAvailableRegion().second);
			loadedScene->GetRootQuadtree()->RemoveGameObjectAndChildren(player);
#else
			scene->RemoveGameObjectAndChildren(player);
#endif // ENGINE			
			App->GetModule<ModuleCamera>()->SetSelectedCamera(0);

			CheckIfActivateMouse();

			return;
		}
	}
	ENGINE_LOG("Player is not load");
}

void ModulePlayer::UnloadNewPlayer()
{
	App->GetModule<ModuleCamera>()->SetSelectedCamera(-1);
	player = nullptr;
}

bool ModulePlayer::IsStatic()
{
	return componentPlayer->IsStatic();
}

void ModulePlayer::CheckIfActivateMouse()
{
	if (componentPlayer->HaveMouseActivated())
	{
		App->GetModule<ModuleInput>()->SetShowCursor(true);
	}
	else
	{
		App->GetModule<ModuleInput>()->SetShowCursor(false);
	}

}
