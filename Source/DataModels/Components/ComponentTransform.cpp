#include "ComponentTransform.h"
#include "ComponentLight.h"

#include "Application.h"
#include "Modules/ModuleScene.h"
#include "ModuleDebugDraw.h"

#include "GameObject/GameObject.h"
#include "Scene/Scene.h"
#include "FileSystem/Json.h"

#include "imgui.h"

ComponentTransform::ComponentTransform(const bool active, GameObject* owner)
	: Component(ComponentType::TRANSFORM, active, owner, false)
{
}

void ComponentTransform::Update()
{
	CalculateLocalMatrix();
	CalculateGlobalMatrix();

	Component* comp = this->GetOwner()->GetComponent(ComponentType::LIGHT);

	if (comp != nullptr)
	{
		ComponentLight* lightComp = (ComponentLight*)comp;
		if (lightComp->GetLightType() == LightType::DIRECTIONAL)
		{
			App->scene->GetLoadedScene()->RenderDirectionalLight();
		}
	}
}

void ComponentTransform::Display()
{
	float3 translation = GetPosition();
	float3 rotation = RadToDeg(GetRotation().ToEulerXYZ());
	float3 scale = GetScale();

	float dragSpeed = 0.025f;

	if (App->scene->GetLoadedScene()->GetRoot() == this->GetOwner()) // The root must not be moved through the inspector
		dragSpeed = 0.0f;

	if (ImGui::CollapsingHeader("TRANSFORM", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (ImGui::BeginTable("TransformTable", 2))
		{
			ImGui::TableNextColumn();
			ImGui::Text("Translation"); ImGui::SameLine();

			ImGui::TableNextColumn();
			ImGui::Text("x:"); ImGui::SameLine();
			ImGui::SetNextItemWidth(80.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5.0f, 1.0f));
			ImGui::DragFloat("##XTrans", &translation.x, dragSpeed,
				std::numeric_limits<float>::min(), std::numeric_limits<float>::min()
			); ImGui::PopStyleVar(); ImGui::SameLine();

			ImGui::Text("y:"); ImGui::SameLine();
			ImGui::SetNextItemWidth(80.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5.0f, 1.0f));
			ImGui::DragFloat("##YTrans", &translation.y, dragSpeed,
				std::numeric_limits<float>::min(), std::numeric_limits<float>::min()
			); ImGui::PopStyleVar(); ImGui::SameLine();

			ImGui::Text("z:"); ImGui::SameLine();
			ImGui::SetNextItemWidth(80.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5.0f, 1.0f));
			ImGui::DragFloat("##ZTrans", &translation.z, dragSpeed,
				std::numeric_limits<float>::min(), std::numeric_limits<float>::min()
			); ImGui::PopStyleVar();

			ImGui::TableNextColumn();
			ImGui::Text("Rotation"); ImGui::SameLine();

			ImGui::TableNextColumn();
			ImGui::Text("x:"); ImGui::SameLine();
			ImGui::SetNextItemWidth(80.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5.0f, 1.0f));
			ImGui::DragFloat("##XRot", &rotation.x, dragSpeed,
				std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), "%0.3f");
			ImGui::PopStyleVar(); ImGui::SameLine();

			ImGui::Text("y:"); ImGui::SameLine();
			ImGui::SetNextItemWidth(80.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5.0f, 1.0f));
			ImGui::DragFloat("##YRot", &rotation.y, dragSpeed,
				std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), "%0.3f");
			ImGui::PopStyleVar(); ImGui::SameLine();

			ImGui::Text("z:"); ImGui::SameLine();
			ImGui::SetNextItemWidth(80.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5.0f, 1.0f));
			ImGui::DragFloat("##ZRot", &rotation.z, dragSpeed,
				std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), "%0.3f");
			ImGui::PopStyleVar();

			ImGui::TableNextColumn();
			ImGui::Text("Scale"); ImGui::SameLine();

			ImGui::TableNextColumn();
			ImGui::Text("x:"); ImGui::SameLine();
			ImGui::SetNextItemWidth(80.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5.0f, 1.0f));
			ImGui::DragFloat("##XScale", &scale.x, dragSpeed,
				0.0001f, std::numeric_limits<float>::max()
			); ImGui::PopStyleVar(); ImGui::SameLine();

			ImGui::Text("y:"); ImGui::SameLine();
			ImGui::SetNextItemWidth(80.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5.0f, 1.0f));
			ImGui::DragFloat("##YScale", &scale.y, dragSpeed,
				0.0001f, std::numeric_limits<float>::max()
			); ImGui::PopStyleVar(); ImGui::SameLine();

			ImGui::Text("z:"); ImGui::SameLine();
			ImGui::SetNextItemWidth(80.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5.0f, 1.0f));
			ImGui::DragFloat("##ZScale", &scale.z, dragSpeed,
				0.0001f, std::numeric_limits<float>::max()
			); ImGui::PopStyleVar();

			ImGui::EndTable();
		}
	}
	ImGui::Separator();

	if (scale.x <= 0) scale.x = 0.0001;
	if (scale.y <= 0) scale.y = 0.0001;
	if (scale.z <= 0) scale.z = 0.0001;

	SetPosition(translation);
	SetRotation(rotation);
	SetScale(scale);
}

void ComponentTransform::SaveOptions(Json& meta)
{
	meta["type"] = GetNameByType(type).c_str();
	meta["active"] = (bool) active;
	meta["owner"] = (GameObject*) owner;
	meta["removed"] = (bool) canBeRemoved;

	meta["localPos_X"] = (float)pos.x;
	meta["localPos_Y"] = (float)pos.y;
	meta["localPos_Z"] = (float)pos.z;

	meta["localRot_X"] = (float)rot.x;
	meta["localRot_Y"] = (float)rot.y;
	meta["localRot_Z"] = (float)rot.z;

	meta["localSca_X"] = (float)sca.x;
	meta["localSca_Y"] = (float)sca.y;
	meta["localSca_Z"] = (float)sca.z;
}

void ComponentTransform::LoadOptions(Json& meta)
{
	type = GetTypeByName(meta["type"]);
	active = (bool) meta["active"];
	//owner = (GameObject*) meta["owner"];
	canBeRemoved = (bool) meta["removed"];

	pos.x = (float) meta["localPos_X"];
	pos.y = (float) meta["localPos_Y"];
	pos.z = (float) meta["localPos_Z"];
				    
	rot.x = (float) meta["localRot_X"];
	rot.y = (float) meta["localRot_Y"];
	rot.z = (float) meta["localRot_Z"];
				    
	sca.x = (float) meta["localSca_X"];
	sca.y = (float) meta["localSca_Y"];
	sca.z = (float) meta["localSca_Z"];

	CalculateLocalMatrix();
	CalculateGlobalMatrix();
}

void ComponentTransform::CalculateLocalMatrix()
{
	float4x4 localMatrix = float4x4::FromTRS((float3)GetPosition(), (Quat)GetRotation(), (float3)GetScale());

	SetLocalMatrix(localMatrix);
}

void ComponentTransform::CalculateGlobalMatrix()
{
	assert(GetOwner()->GetParent() != nullptr);

	float3 parentPos, parentSca, localPos, localSca;
	Quat parentRot, localRot;

	ComponentTransform* parentTransform = (ComponentTransform*)GetOwner()->GetParent()
															->GetComponent(ComponentType::TRANSFORM);

	parentTransform->GetGlobalMatrix().Decompose(parentPos, parentRot, parentSca);
	GetLocalMatrix().Decompose(localPos, localRot, localSca);

	float3 position = localPos + parentPos;
	Quat rotation = localRot * parentRot;
	float3 scale = parentSca.Mul(localSca);

	float4x4 globalMatrix = float4x4::FromTRS(position, rotation, scale);
	SetGlobalMatrix(globalMatrix);
}

const float3& ComponentTransform::GetGlobalPosition() const
{
	float3 globalPos, globalSca;
	Quat globalRot;
	globalMatrix.Decompose(globalPos, globalRot, globalSca);

	return globalPos;
}

const Quat& ComponentTransform::GetGlobalRotation() const
{
	float3 globalPos, globalSca;
	Quat globalRot;
	globalMatrix.Decompose(globalPos, globalRot, globalSca);

	return globalRot;
}

const float3& ComponentTransform::GetGlobalScale() const
{
	float3 globalPos, globalSca;
	Quat globalRot;
	globalMatrix.Decompose(globalPos, globalRot, globalSca);

	return globalSca;
}