#include "StdAfx.h"

#include "ComponentPointLight.h"
#include "ComponentTransform.h"

#include "FileSystem/Json.h"

#include "Modules/ModuleScene.h"
#include "Modules/ModuleDebugDraw.h"

#include "Scene/Scene.h"

#ifndef ENGINE
	#include "Modules/ModuleEditor.h"

	#include "Windows/WindowDebug.h"
#endif // ENGINE

#include "Application.h"

#include "debugdraw.h"

ComponentPointLight::ComponentPointLight() : ComponentLight(LightType::POINT, true), radius(1.0f)
{
}

ComponentPointLight::ComponentPointLight(const ComponentPointLight& componentPointLight) :
	ComponentLight(componentPointLight),
	radius(componentPointLight.GetRadius())
{
}

ComponentPointLight::ComponentPointLight(GameObject* parent) :
	ComponentLight(LightType::POINT, parent, true),
	radius(1.0f)
{
}

ComponentPointLight::ComponentPointLight(float radius, const float3& color, float intensity) :
	ComponentLight(LightType::POINT, color, intensity, true),
	radius(radius)
{
}

ComponentPointLight::ComponentPointLight(float radius, const float3& color, float intensity, GameObject* parent) :
	ComponentLight(LightType::POINT, color, intensity, parent, true),
	radius(radius)
{
}

ComponentPointLight::~ComponentPointLight()
{
	deleting = true;
	Scene* currentScene = App->GetModule<ModuleScene>()->GetLoadedScene();

	if (currentScene)
	{
		currentScene->UpdateScenePointLights();
		currentScene->RenderPointLights();
	}
}

void ComponentPointLight::Draw() const
{
	bool canDrawLight =
#ifdef ENGINE
		IsEnabled() && App->GetPlayState() != Application::PlayState::RUNNING &&
		GetOwner() == App->GetModule<ModuleScene>()->GetSelectedGameObject();
#else
		IsEnabled() && App->GetModule<ModuleEditor>()->GetDebugOptions()->GetDrawPointLight();
#endif // ENGINE

	if (!canDrawLight)
	{
		return;
	}
	ComponentTransform* transform = GetOwner()->GetComponentInternal<ComponentTransform>();

	float3 position = transform->GetGlobalPosition();

	dd::sphere(position, dd::colors::White, radius);


#ifdef ENGINE
	if (transform->IsDrawBoundingBoxes())
	{
		App->GetModule<ModuleDebugDraw>()->DrawBoundingBox(transform->GetObjectOBB());
	}
#endif
}

void ComponentPointLight::SetRadius(float radius)
{
	isDirty = true;
	this->radius = radius;

	ComponentTransform* transform = GetOwner()->GetComponentInternal<ComponentTransform>();
	float3 scale = float3(radius * 2);
	transform->ScaleLocalAABB(scale);
}

void ComponentPointLight::OnTransformChanged()
{
	Scene* currentScene = App->GetModule<ModuleScene>()->GetLoadedScene();

	currentScene->UpdateScenePointLight(this);
	currentScene->RenderPointLight(this);

	ComponentTransform* transform = GetOwner()->GetComponentInternal<ComponentTransform>();
	
	float3 translation = float3(-0.5f);
	float3 scale = float3(radius * 2.0f);

	transform->TranslateLocalAABB(translation);
	transform->ScaleLocalAABB(scale);
}

void ComponentPointLight::SignalEnable()
{
	ModuleScene* moduleScene = App->GetModule<ModuleScene>();

	if (moduleScene->IsLoading())
	{
		return;
	}

	Scene* currentScene = moduleScene->GetLoadedScene();

	currentScene->UpdateScenePointLights();
	currentScene->RenderPointLights();
}

void ComponentPointLight::SignalDisable()
{
	ModuleScene* moduleScene = App->GetModule<ModuleScene>();

	if (moduleScene->IsLoading())
	{
		return;
	}

	Scene* currentScene = moduleScene->GetLoadedScene();
	
	currentScene->UpdateScenePointLights();
	currentScene->RenderPointLights();
}

void ComponentPointLight::InternalSave(Json& meta)
{
	meta["color_light_X"] = (float) color.x;
	meta["color_light_Y"] = (float) color.y;
	meta["color_light_Z"] = (float) color.z;

	meta["intensity"] = (float) intensity;

	meta["lightType"] = GetNameByLightType(lightType).c_str();
	meta["radius"] = (float) radius;
}

void ComponentPointLight::InternalLoad(const Json& meta)
{
	color.x = (float) meta["color_light_X"];
	color.y = (float) meta["color_light_Y"];
	color.z = (float) meta["color_light_Z"];

	intensity = (float) meta["intensity"];

	lightType = GetLightTypeByName(meta["lightType"]);
	radius = meta["radius"];
}