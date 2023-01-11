#pragma once
#include "Components/Component.h"

#include "Math/float3.h"
#include "Math/float4x4.h"
#include "Math/Quat.h"

class ComponentTransform : public Component
{
public:
	ComponentTransform(const bool active, GameObject* owner);
	~ComponentTransform() override;

	void Update() override;
	void Display() override;

	const float3& GetPosition() const;
	const Quat& GetRotation() const;
	const float3& GetScale() const;

	void SetPosition(const float3& position);
	void SetRotation(const float3& rotation);
	void SetRotation(const Quat& rotation);
	void SetScale(const float3& scale);

	const float4x4& GetLocalMatrix() const;
	const float4x4& GetGlobalMatrix() const;

	void SetLocalMatrix(const float4x4& matrix);
	void SetGlobalMatrix(const float4x4& matrix);

	void CalculateLocalMatrix();
	void CalculateGlobalMatrix();

	void ResetGlobalMatrix();

private:
	GameObject* ownerParent;

	float3 pos = float3(0.0f, 0.0f, 0.0f);
	Quat rot = Quat::identity;
	float3 sca = float3(1.0f, 1.0f, 1.0f);

	float4x4 localMatrix = float4x4::identity;
	float4x4 globalMatrix = float4x4::identity;
};

inline const float3& ComponentTransform::GetPosition() const
{
	return pos;
}

inline const Quat& ComponentTransform::GetRotation() const
{
	return rot;
}

inline const float3& ComponentTransform::GetScale() const
{
	return sca;
}

inline void ComponentTransform::SetPosition(const float3& position)
{
	pos = position;
}

inline void ComponentTransform::SetRotation(const float3& rotation)
{
	rot = Quat::FromEulerXYZ(DegToRad(rotation.x), DegToRad(rotation.y), DegToRad(rotation.z));
}

inline void ComponentTransform::SetRotation(const Quat& rotation)
{
	rot = rotation;
}

inline void ComponentTransform::SetScale(const float3& scale)
{
	sca = scale;
}

inline const float4x4& ComponentTransform::GetLocalMatrix() const
{
	return localMatrix;
}

inline const float4x4& ComponentTransform::GetGlobalMatrix() const
{
	return globalMatrix;
}

inline void ComponentTransform::SetLocalMatrix(const float4x4& matrix)
{
	localMatrix = matrix;
}

inline void ComponentTransform::SetGlobalMatrix(const float4x4& matrix)
{
	globalMatrix = matrix;
}

inline void ComponentTransform::ResetGlobalMatrix()
{
	globalMatrix = float4x4::identity;
}