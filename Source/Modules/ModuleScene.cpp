#include "Application.h"
#include "ModuleScene.h"
#include "ModuleRender.h"
#include "DataStructures/Quadtree.h"

#include "GameObject/GameObject.h"
#include "Scene/Scene.h"

#include <assert.h>

#include "FileSystem/ModuleFileSystem.h"
#include "Components/Component.h"
#include "Components/ComponentCamera.h"
#include "Components/ComponentLight.h"

ModuleScene::ModuleScene()
{
}

ModuleScene::~ModuleScene()
{
}

bool ModuleScene::Init()
{
	return true;
}

bool ModuleScene::Start()
{
#if !defined(GAME)
	if (loadedScene == nullptr)
	{
		loadedScene = CreateEmptyScene();
	}
#else
	if (loadedScene == nullptr)
	{
		LoadSceneFromJson("Lib/Scenes/Final_Scene_Lights_mini_Lights.axolotl");
	}
#endif
	selectedGameObject = loadedScene->GetRoot();
	return true;
}

update_status ModuleScene::Update()
{
	UpdateGameObjectAndDescendants(loadedScene->GetRoot());

	//SaveSceneToJson("AuxScene");

	return UPDATE_CONTINUE;
}

void ModuleScene::UpdateGameObjectAndDescendants(const std::shared_ptr<GameObject>& gameObject) const
{
	assert(gameObject != nullptr);

	if (!gameObject->IsEnabled())
		return;

	if (gameObject != loadedScene->GetRoot())
		gameObject->Update();

	for (std::weak_ptr<GameObject> child : gameObject->GetChildren())
	{
		UpdateGameObjectAndDescendants(child.lock());
	}
}

void ModuleScene::OnPlay()
{
	ENGINE_LOG("Play pressed");

	Json jsonScene(tmpDoc, tmpDoc);

	std::shared_ptr<GameObject> root = loadedScene->GetRoot();
	root->SaveOptions(jsonScene);

	rapidjson::StringBuffer buffer;
	jsonScene.toBuffer(buffer);
}

void ModuleScene::OnPause()
{
	ENGINE_LOG("Pause pressed");
}

void ModuleScene::OnStop()
{
	ENGINE_LOG("Stop pressed");
	Json Json(tmpDoc, tmpDoc);

	SetSceneFromJson(Json);
	//clear the document
	rapidjson::Document().Swap(tmpDoc).SetObject();
}

std::shared_ptr<Scene> ModuleScene::CreateEmptyScene() const
{
	std::shared_ptr<Scene> newScene = std::make_shared<Scene>();
	newScene->InitNewEmptyScene();
	return newScene;
}

void ModuleScene::SaveSceneToJson(const std::string& name)
{
	rapidjson::Document doc;
	Json jsonScene(doc, doc);

	std::shared_ptr<GameObject> root = loadedScene->GetRoot();
	root->SetName(App->fileSystem->GetFileName(name).c_str());
	root->SaveOptions(jsonScene);

	rapidjson::StringBuffer buffer;
	jsonScene.toBuffer(buffer);

	std::string path = SCENE_PATH + name;

	App->fileSystem->Save(path.c_str(), buffer.GetString(), (unsigned int)buffer.GetSize());
}

void ModuleScene::LoadSceneFromJson(const std::string& filePath)
{
	std::string fileName = App->fileSystem->GetFileName(filePath).c_str();
	char* buffer{};
#if !defined(GAME)
	std::string assetPath = SCENE_PATH + fileName + SCENE_EXTENSION;

	bool resourceExists = App->fileSystem->Exists(assetPath.c_str());
	if (!resourceExists)
		App->fileSystem->CopyFileInAssets(filePath, assetPath);
	App->fileSystem->Load(assetPath.c_str(), buffer);
#else
	App->fileSystem->Load(filePath.c_str(), buffer);
#endif
	rapidjson::Document doc;
	Json Json(doc, doc);

	Json.fromBuffer(buffer);

	SetSceneFromJson(Json);

	delete buffer;
}

void ModuleScene::SetSceneFromJson(Json& Json)
{
	std::shared_ptr<Scene> sceneToLoad = std::make_shared<Scene>();
	std::shared_ptr<GameObject> newRoot = std::make_shared<GameObject>(std::string(Json["name"]).c_str());

	loadedScene = sceneToLoad;

	std::vector<std::shared_ptr<GameObject> > loadedObjects{};
	newRoot->LoadOptions(Json, loadedObjects);

	sceneToLoad->SetSceneQuadTree(std::make_unique<Quadtree>(AABB(float3(-20000, -1000, -20000), float3(20000, 1000, 20000))));
	Quadtree* sceneQuadtree = sceneToLoad->GetSceneQuadTree();
	std::vector<std::shared_ptr<GameObject> > loadedCameras{};
	std::shared_ptr<GameObject> ambientLight = nullptr;
	std::shared_ptr<GameObject> directionalLight = nullptr;

	std::vector<std::weak_ptr<GameObject> > vecOfWeak(loadedObjects.begin(), loadedObjects.end());
	for (std::shared_ptr<GameObject> obj : loadedObjects)
	{
		sceneQuadtree = sceneToLoad->GetSceneQuadTree();
		std::vector<std::shared_ptr<ComponentCamera> > camerasOfObj = obj->GetComponentsByType<ComponentCamera>(ComponentType::CAMERA);
		if (!camerasOfObj.empty())
		{
			loadedCameras.push_back(obj);
		}

		std::vector<std::shared_ptr<ComponentLight> > lightsOfObj = obj->GetComponentsByType<ComponentLight>(ComponentType::LIGHT);
		for (std::shared_ptr<ComponentLight> light : lightsOfObj)
		{
			if (light->GetLightType() == LightType::AMBIENT)
			{
				ambientLight = obj;
			}
			else if (light->GetLightType() == LightType::DIRECTIONAL)
			{
				directionalLight = obj;
			}
		}
		//Quadtree treatment
		if (!sceneQuadtree->InQuadrant(obj))
		{
			if (!sceneQuadtree->IsFreezed())
			{
				sceneQuadtree->ExpandToFit(obj);
				//sceneToLoad->FillQuadtree(loadedObjects);
			}
		}
		else
		{
			sceneQuadtree->Add(obj);
		}
	}

	App->renderer->FillRenderList(sceneQuadtree);

	sceneToLoad->SetRoot(newRoot);
	selectedGameObject = newRoot;

	sceneToLoad->SetSceneGameObjects(vecOfWeak);
	sceneToLoad->SetSceneCameras(vecOfWeak);
	sceneToLoad->SetAmbientLight(ambientLight);
	sceneToLoad->SetDirectionalLight(directionalLight);
	//sceneToLoad->SetSceneQuadTree(sceneQuadtree);

	sceneToLoad->InitLights();

}
