#include "ComponentBoundingBoxes.h"

#include "ComponentTransform.h"
#include "GameObject/GameObject.h"

#include "Application.h"
#include "ModuleDebugDraw.h"
#include "Modules/ModuleScene.h"
#include "Scene/Scene.h"
#include "FileSystem/Json.h"

#include "imgui.h"

ComponentBoundingBoxes::ComponentBoundingBoxes(bool active, GameObject* owner)
	: Component(ComponentType::BOUNDINGBOX, active, owner, false)
{
	localAABB = { {0 ,0, 0}, {0, 0, 0} };
	encapsuledAABB = localAABB;
	objectOBB = { localAABB };
	drawBoundingBoxes = false;
}

void ComponentBoundingBoxes::CalculateBoundingBoxes() 
{
	ComponentTransform* transform = (ComponentTransform*)((Component*)this)->GetOwner()->GetComponent(ComponentType::TRANSFORM);
	objectOBB = OBB(localAABB);
	objectOBB.Transform(transform->GetGlobalMatrix());
	encapsuledAABB = objectOBB.MinimalEnclosingAABB();
}

void ComponentBoundingBoxes::Draw()
{
	if (drawBoundingBoxes) App->debug->DrawBoundingBox(GetObjectOBB());
}


void ComponentBoundingBoxes::Display()
{
	if (ImGui::CollapsingHeader("BOUNDING BOX", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Text("Draw Bounding Box"); ImGui::SameLine();
		ImGui::Checkbox("##Draw Bounding Box", &drawBoundingBoxes);
	}

	ImGui::Separator();
}

void ComponentBoundingBoxes::SaveOptions(Json& meta)
{
	// Do not delete these
	meta["type"] = GetNameByType(type).c_str();
	meta["active"] = (bool)active;
	meta["owner"] = (GameObject*)owner;
	meta["removed"] = (bool)canBeRemoved;

	meta["AABBMin_X"] = (float)localAABB.minPoint.x;
	meta["AABBMin_Y"] = (float)localAABB.minPoint.y;
	meta["AABBMin_Z"] = (float)localAABB.minPoint.z;

	meta["AABBMax_X"] = (float)localAABB.maxPoint.x;
	meta["AABBMax_Y"] = (float)localAABB.maxPoint.y;
	meta["AABBMax_Z"] = (float)localAABB.maxPoint.z;
	
}

void ComponentBoundingBoxes::LoadOptions(Json& meta)
{
	// Do not delete these
	type = GetTypeByName(meta["type"]);
	active = (bool)meta["active"];
	//owner = (GameObject*) meta["owner"];
	canBeRemoved = (bool)meta["removed"];

	localAABB = { { (float)meta["AABBMin_X"], (float)meta["AABBMin_Y"], (float)meta["AABBMin_Z"] }, { (float)meta["AABBMax_X"], (float)meta["AABBMax_Y"], (float)meta["AABBMax_Z"] } };
}