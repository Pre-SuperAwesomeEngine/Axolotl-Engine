#pragma once
#pragma warning (disable: 26495)

#include "Module.h"
#include "ModuleDebugDraw.h"

#include <map>

#include "Math/Quat.h"
#include "Geometry/Plane.h"
#include "Geometry/LineSegment.h"

#define DEFAULT_MOVE_SPEED 9.f
#define DEFAULT_ROTATION_DEGREE 30
#define DEFAULT_ROTATION_SPEED 5.f
#define DEFAULT_MOUSE_SPEED_MODIFIER 0.f
#define DEFAULT_MOUSE_ZOOM_SPEED 2.f
#define DEFAULT_SHIFT_ACCELERATION 2.f
#define DEFAULT_FRUSTUM_MODE 0
#define DEFAULT_FRUSTUM_OFFSET 1.f
#define DEFAULT_FRUSTUM_DISTANCE 20000.f

#define ORBIT_SPEED_MULTIPLIER 2.f

#define MAX_MOUSE_SPEED_MODIFIER 5.f
#define MAX_HFOV 120
#define MAX_VFOV 85
#define MAX_FRUSTUM 2.f

#define MIN_HFOV 60
#define MIN_VFOV 34
#define MIN_FRUSTUM -2.f

enum class EFrustumMode
{
	NORMALFRUSTUM,
	OFFSETFRUSTUM,
	NOFRUSTUM
};

class GameObject;
class WindowScene;

class ModuleEngineCamera : public Module
{
public:
	ModuleEngineCamera();
	~ModuleEngineCamera() override;

	bool Init() override;
	bool Start() override;

	update_status Update();

	void Move();
	void KeyboardRotate();
	void Rotate();
	void ApplyRotation(const float3x3& rotationMatrix);
	void FreeLook();
	void UnlimitedCursor();
	void Run();
	void Walk();
	void Zoom();
	void Focus(const OBB& obb);
	void Focus(GameObject* gameObject);
	void Orbit(const OBB& obb);
	
	bool IsInside(const OBB& obb);
	bool IsInside(const AABB& aabb);
	bool IsInsideOffset(const OBB& obb);
	void RecalculateOffsetPlanes();

	void SetHFOV(float fov);
	void SetVFOV(float fov);
	void SetAspectRatio(float aspect);
	void SetPlaneDistance(float zNear, float zFar);
	void SetPosition(const float3& position);
	void SetOrientation(const float3& orientation);
	void SetLookAt(const float3& lookAt);
	void SetMoveSpeed(float speed);
	void SetRotationSpeed(float speed);
	void SetFrustumOffset(float offset);
	void SetFrustumMode(EFrustumMode mode);
	void SetViewPlaneDistance(float distance);

	const float4x4& GetProjectionMatrix() const;
	const float4x4& GetViewMatrix() const;

	float GetHFOV() const;
	float GetVFOV() const;
	float GetZNear() const;
	float GetZFar() const;
	float GetMoveSpeed() const;
	float GetRotationSpeed() const;
	float GetDistance(const float3& point) const;
	float GetFrustumOffset() const;
	float GetViewPlaneDistance() const;
	EFrustumMode GetFrustumMode() const;
	const float3& GetPosition() const;
	
private:
	bool CreateRaycastFromMousePosition(const WindowScene* windowScene, LineSegment& ray);
	
	void CalculateHitGameObjects(const LineSegment& ray);
	void CalculateHitSelectedGo(std::map<float, const GameObject*>& hitGameObjects,
		const LineSegment& ray);
	void SetNewSelectedGameObject(const std::map<float, const GameObject*>& hitGameObjects,
								  const LineSegment& ray);

	Frustum frustum;

	float3 position;

	float4x4 projectionMatrix;
	float4x4 viewMatrix;
	Quat currentRotation = Quat::identity;
	float aspectRatio;
	float acceleration;
	float moveSpeed;
	float rotationSpeed;
	float mouseSpeedModifier;
	float frustumOffset;
	float viewPlaneDistance;

	EFrustumMode frustumMode;

	math::Plane offsetFrustumPlanes[6];
	bool mouseWarped;
	bool focusFlag;
	bool isFocusing;
	int lastMouseX, lastMouseY;
	int mouseState;
};

inline const float3& ModuleEngineCamera::GetPosition() const
{
	return position;
}

inline float ModuleEngineCamera::GetViewPlaneDistance() const
{
	return viewPlaneDistance;
}

inline void ModuleEngineCamera::SetViewPlaneDistance(float distance)
{
	viewPlaneDistance = distance;
	frustum.SetViewPlaneDistances(0.1f, distance);
}