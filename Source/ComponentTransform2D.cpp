#include "ComponentTransform2D.h"
#include "FileSystem/Json.h"

#include "GameObject/GameObject.h"
#include "Math/float3x3.h"

ComponentTransform2D::ComponentTransform2D(const bool active, GameObject* owner)
	: Component(ComponentType::TRANSFORM2D, active, owner, true),
	pos(float3::zero), rot(float4x4::identity), sca(float3::one),
	globalPos(float3::zero), globalRot(float4x4::identity), globalSca(float3::one),
	rotXYZ(float3::zero), localMatrix(float4x4::identity), globalMatrix(float4x4::identity)
{
}

ComponentTransform2D::~ComponentTransform2D()
{
}

void ComponentTransform2D::Update()
{

}

void ComponentTransform2D::SaveOptions(Json& meta)
{
	meta["type"] = GetNameByType(type).c_str();
	meta["active"] = static_cast<bool>(active);
	meta["removed"] = static_cast<bool>(canBeRemoved);

	meta["localPositionX"] = static_cast<float>(pos.x);
	meta["localPositionY"] = static_cast<float>(pos.y);
	meta["localPositionZ"] = static_cast<float>(pos.z);

	float3 rotation = GetRotationXYZ();
	meta["localRot_X"] = (float)rotation.x;
	meta["localRot_Y"] = (float)rotation.y;
	meta["localRot_Z"] = (float)rotation.z;

	meta["localScaleX"] = static_cast<float>(sca.x);
	meta["localScaleY"] = static_cast<float>(sca.y);
	meta["localScaleZ"] = static_cast<float>(sca.z);

	meta["sizeX"] = static_cast<float>(size.x);
	meta["sizeY"] = static_cast<float>(size.y);
}

void ComponentTransform2D::LoadOptions(Json& meta)
{
	type = GetTypeByName(meta["type"]);
	active = static_cast<bool>(meta["active"]);
	canBeRemoved = static_cast<bool>(meta["removed"]);

	pos.x = static_cast<float>(meta["localPositionX"]);
	pos.y = static_cast<float>(meta["localPositionY"]);
	pos.z = static_cast<float>(meta["localPositionZ"]);

	float3 rotation;
	rotation.x = (float)meta["localRot_X"];
	rotation.y = (float)meta["localRot_Y"];
	rotation.z = (float)meta["localRot_Z"];
	SetRotation(rotation);

	sca.x = static_cast<float>(meta["localScaleX"]);
	sca.y = static_cast<float>(meta["localScaleY"]);
	sca.z = static_cast<float>(meta["localScaleZ"]);

	size.x = static_cast<float>(meta["sizeX"]);
	size.y = static_cast<float>(meta["sizeY"]);
	CalculateMatrices();
}


void ComponentTransform2D::CalculateMatrices()
{
	localMatrix = float4x4::FromTRS(pos, rot, sca);

	const GameObject* parent = GetOwner()->GetParent();

	if (parent)
	{
		ComponentTransform2D* parentTransform = static_cast<ComponentTransform2D*>(parent->GetComponent(ComponentType::TRANSFORM2D));
		if (parentTransform)
		{

			// Set global matrix
			globalMatrix = parentTransform->GetGlobalMatrix().Mul(localMatrix);

			globalPos = globalMatrix.TranslatePart();
			globalRot = static_cast<float4x4>(globalMatrix.RotatePart());
			globalSca = globalMatrix.GetScale();
		}
		else
		{
			globalMatrix = localMatrix;

			globalPos = globalMatrix.TranslatePart();
			globalRot = static_cast<float4x4>(globalMatrix.RotatePart());
			globalSca = globalMatrix.GetScale();

		}
	}

	for (GameObject* child : GetOwner()->GetChildren())
	{
		ComponentTransform2D* childTransform = static_cast<ComponentTransform2D*>
			(child->GetComponent(ComponentType::TRANSFORM2D));
		childTransform->CalculateMatrices();
	}
}
