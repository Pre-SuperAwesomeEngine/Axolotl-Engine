#include "Application.h"

#include "ModuleEngineCamera.h"
#include "ModuleWindow.h"
#include "ModuleInput.h"
#include "ModuleRender.h"
#include "ModuleEditor.h"

#include "Math/float3x3.h"
#include "Math/Quat.h"
#include "Math/MathAll.h"
#include "Geometry/Sphere.h"
#include "Geometry/Plane.h"

#include <GL/glew.h>

ModuleEngineCamera::ModuleEngineCamera() {};

ModuleEngineCamera::~ModuleEngineCamera() {};

bool ModuleEngineCamera::Init()
{
	int w, h;

	SDL_GetWindowSize(App->window->GetWindow(), &w, &h);
	aspectRatio = float(w) / h;

	frustum.SetKind(FrustumProjectiveSpace::FrustumSpaceGL, FrustumHandedness::FrustumRightHanded);
	frustum.SetViewPlaneDistances(0.1f, 100.0f);
	frustum.SetHorizontalFovAndAspectRatio(math::DegToRad(90), aspectRatio);

	acceleration = DEFAULT_SHIFT_ACCELERATION;
	moveSpeed = DEFAULT_MOVE_SPEED;
	rotationSpeed = DEFAULT_ROTATION_SPEED;
	mouseSpeedModifier = DEFAULT_MOUSE_SPEED_MODIFIER;

	position = float3(0.f, 2.f, 5.f);

	frustum.SetPos(position);
	frustum.SetFront(-float3::unitZ);
	frustum.SetUp(float3::unitY);

	return true;
}

bool ModuleEngineCamera::Start()
{
	if (App->renderer->AnyModelLoaded())
		Focus(App->renderer->GetModel(0)->GetAABB());

	return true;
}

update_status ModuleEngineCamera::Update()
{
	projectionMatrix = frustum.ProjectionMatrix();
	viewMatrix = frustum.ViewMatrix();

	if (App->editor->IsSceneFocused())
	{
		if (App->input->GetKey(SDL_SCANCODE_LSHIFT) != KeyState::IDLE)
			Run();
		else
			Walk();

		if (App->input->GetMouseButton(SDL_BUTTON_RIGHT) != KeyState::IDLE)
		{
			Move();
			FreeLook();
		}

		if (App->input->IsMouseWheelScrolled())
		{
			Zoom();
		}

		if (App->renderer->AnyModelLoaded() && App->input->GetKey(SDL_SCANCODE_F) != KeyState::IDLE)
			Focus(App->renderer->GetModel(0)->GetOBB());

		if (App->renderer->AnyModelLoaded() &&
			App->input->GetKey(SDL_SCANCODE_LALT) != KeyState::IDLE &&
			App->input->GetMouseButton(SDL_BUTTON_LEFT) != KeyState::IDLE)
		{
			const OBB& obb = App->renderer->GetModel(0)->GetOBB();

			SetLookAt(obb.CenterPoint());
			Orbit(obb);
		}

		KeyboardRotate();
		SelectObjects();
	}

	return UPDATE_CONTINUE;
}

void ModuleEngineCamera::Move()
{
	float deltaTime = App->GetDeltaTime();

	if (App->input->GetKey(SDL_SCANCODE_W) != KeyState::IDLE)
	{
		position = position + frustum.Front().Normalized() * 
			moveSpeed * acceleration * deltaTime;
		frustum.SetPos(position);
	}

	if (App->input->GetKey(SDL_SCANCODE_S) != KeyState::IDLE)
	{
		position = position + -(frustum.Front().Normalized()) * 
			moveSpeed * acceleration * deltaTime;
		frustum.SetPos(position);
	}

	if (App->input->GetKey(SDL_SCANCODE_A) != KeyState::IDLE)
	{
		position = position + -(frustum.WorldRight()) * moveSpeed * acceleration * deltaTime;
		frustum.SetPos(position);
	}

	if (App->input->GetKey(SDL_SCANCODE_D) != KeyState::IDLE)
	{
		position = position + frustum.WorldRight() * moveSpeed * acceleration * deltaTime;
		frustum.SetPos(position);
	}

	if (App->input->GetKey(SDL_SCANCODE_E) != KeyState::IDLE)
	{
		position = position + frustum.Up() * moveSpeed * acceleration * deltaTime;
		frustum.SetPos(position);
	}

	if (App->input->GetKey(SDL_SCANCODE_Q) != KeyState::IDLE)
	{
		position = position + -(frustum.Up()) * moveSpeed * acceleration * deltaTime;
		frustum.SetPos(position);
	}
}

void ModuleEngineCamera::KeyboardRotate()
{
	float yaw = 0.f, pitch = 0.f;

	float rotationAngle = RadToDeg(frustum.Front().Normalized().AngleBetween(float3::unitY));

	if (App->input->GetKey(SDL_SCANCODE_UP) != KeyState::IDLE)
	{
		if (rotationAngle + rotationSpeed * acceleration < 180) 
			pitch = math::DegToRad(-DEFAULT_ROTATION_DEGREE);
	}

	if (App->input->GetKey(SDL_SCANCODE_DOWN) != KeyState::IDLE)
	{
		if (rotationAngle - rotationSpeed * acceleration > 0) 
			pitch = math::DegToRad(DEFAULT_ROTATION_DEGREE);
	}

	if (App->input->GetKey(SDL_SCANCODE_LEFT) != KeyState::IDLE)
		yaw = math::DegToRad(DEFAULT_ROTATION_DEGREE);

	if (App->input->GetKey(SDL_SCANCODE_RIGHT) != KeyState::IDLE)
		yaw = math::DegToRad(-DEFAULT_ROTATION_DEGREE);

	float deltaTime = App->GetDeltaTime();
	Quat pitchQuat(frustum.WorldRight(), pitch * deltaTime * rotationSpeed * acceleration);
	Quat yawQuat(float3::unitY, yaw * deltaTime * rotationSpeed * acceleration);

	float3x3 rotationMatrixX = float3x3::FromQuat(pitchQuat);
	float3x3 rotationMatrixY = float3x3::FromQuat(yawQuat);
	float3x3 rotationDeltaMatrix = rotationMatrixY * rotationMatrixX;

	ApplyRotation(rotationDeltaMatrix);
}

void ModuleEngineCamera::SelectObjects() {
	if (App->renderer->AnyModelLoaded()) {
		for (int i = 0; i < App->renderer->GetModelCount(); ++i) {
			for (int j = 0; j < App->renderer->GetModel(i)->GetMeshCount(); ++j) {
			 App->debug->DrawBoundingBox(App->renderer->GetModel(i)->GetAABB());
			}
		}
	}
}

void ModuleEngineCamera::ApplyRotation(const float3x3& rotationMatrix) 
{
	vec oldFront = frustum.Front().Normalized();
	vec oldUp = frustum.Up().Normalized();

	frustum.SetFront(rotationMatrix.MulDir(oldFront));
	frustum.SetUp(rotationMatrix.MulDir(oldUp));
}

void ModuleEngineCamera::FreeLook()
{
	float yaw = 0.f, pitch = 0.f;
	float xrel = App->input->GetMouseMotionX();
	float yrel = App->input->GetMouseMotionY();
	float rotationAngle = RadToDeg(frustum.Front().Normalized().AngleBetween(float3::unitY));

	if (abs(xrel) + abs(yrel) != 0  && mouseSpeedModifier < MAX_MOUSE_SPEED_MODIFIER)
	{
		mouseSpeedModifier += 0.5f;
	}
	else if (abs(xrel) + abs(yrel) == 0)
		mouseSpeedModifier = DEFAULT_MOUSE_SPEED_MODIFIER;

	if (yrel > 0) { if (rotationAngle - yrel > 0) pitch = math::DegToRad(yrel); }
	else if (yrel < 0) { if (rotationAngle - yrel < 180) pitch = math::DegToRad(yrel); }
	
	yaw = math::DegToRad(-xrel);

	float deltaTime = App->GetDeltaTime();
	Quat pitchQuat(frustum.WorldRight(), pitch * mouseSpeedModifier * deltaTime);
	Quat yawQuat(float3::unitY, yaw * mouseSpeedModifier * deltaTime);

	float3x3 rotationMatrixX = float3x3::FromQuat(pitchQuat);
	float3x3 rotationMatrixY = float3x3::FromQuat(yawQuat);
	float3x3 rotationDeltaMatrix = rotationMatrixY * rotationMatrixX;

	ApplyRotation(rotationDeltaMatrix);
}

void ModuleEngineCamera::Run()
{
	acceleration = DEFAULT_SHIFT_ACCELERATION;
}

void ModuleEngineCamera::Walk()
{
	acceleration = 1.f;
}

void ModuleEngineCamera::Zoom()
{
	float newHFOV = GetHFOV() - App->input->GetMouseWheelY();

	if (newHFOV <= MAX_HFOV && newHFOV >= MIN_HFOV)
		SetHFOV(math::DegToRad(newHFOV));
}

void ModuleEngineCamera::Focus(const OBB &obb)
{
	math::Sphere minSphere = obb.MinimalEnclosingSphere();

	float3 point = minSphere.Centroid();

	position = point - frustum.Front().Normalized() * minSphere.Diameter();

	SetLookAt(point);

	frustum.SetPos(position);
}

void ModuleEngineCamera::Orbit(const OBB& obb)
{
	float distance = obb.CenterPoint().Distance(position);

	vec oldPosition = position + frustum.Front().Normalized() * distance;

	float deltaTime = App->GetDeltaTime();
	Quat verticalOrbit(
		frustum.WorldRight(),
		DegToRad(-App->input->GetMouseMotionY() * rotationSpeed * 
			ORBIT_SPEED_MULTIPLIER * deltaTime
		)
	);
	Quat sideOrbit(
		float3::unitY, 
		DegToRad(-App->input->GetMouseMotionX() * rotationSpeed *
			ORBIT_SPEED_MULTIPLIER * deltaTime
		)
	);

	float3x3 rotationMatrixX = float3x3::FromQuat(verticalOrbit);
	float3x3 rotationMatrixY = float3x3::FromQuat(sideOrbit);
	float3x3 rotationDeltaMatrix = rotationMatrixY * rotationMatrixX;

	ApplyRotation(rotationDeltaMatrix);

	vec newPosition = position + frustum.Front().Normalized() * distance;

	position = position + (oldPosition - newPosition);

	frustum.SetPos(position);
}

bool ModuleEngineCamera::IsInside(const OBB& obb)
{
	math::vec cornerPoints[8];
	math::Plane frustumPlanes[6];

	
	frustum.GetPlanes(frustumPlanes);
	obb.GetCornerPoints(cornerPoints);

	for (int itPlanes = 0; itPlanes < 6; itPlanes++)
	{
		bool onPlane = false;
		for (int itPoints = 0 ; itPoints < 8; itPoints++)
		{
			if (!frustumPlanes[itPlanes].IsOnPositiveSide(cornerPoints[itPoints]))
			{
				onPlane = true;
				break;
			}
		}
		if (!onPlane)
			return false;
	}
	
	return true;
}

void ModuleEngineCamera::SetHFOV(float fov)
{
	frustum.SetHorizontalFovAndAspectRatio(fov, aspectRatio);
}

void ModuleEngineCamera::SetVFOV(float fov)
{
	frustum.SetVerticalFovAndAspectRatio(fov, aspectRatio);
}

void ModuleEngineCamera::SetAspectRatio(float aspect)
{
	aspectRatio = aspect;
	SetHFOV(frustum.HorizontalFov());
}

void ModuleEngineCamera::SetPlaneDistance(float zNear, float zFar)
{
	frustum.SetViewPlaneDistances(zNear, zFar);
}

void ModuleEngineCamera::SetPosition(const float3& position)
{
	frustum.SetPos(position);
}

void ModuleEngineCamera::SetOrientation(const float3& orientation)
{
	frustum.SetUp(orientation);
}

void ModuleEngineCamera::SetLookAt(const float3& lookAt)
{
	float3 direction = lookAt - position;

	float3x3 rotationMatrix = float3x3::LookAt(
		frustum.Front().Normalized(), direction.Normalized(), frustum.Up(), float3::unitY
	);
	
	ApplyRotation(rotationMatrix);
}

void ModuleEngineCamera::SetMoveSpeed(float speed)
{
	moveSpeed = speed;
}

void ModuleEngineCamera::SetRotationSpeed(float speed)
{
	rotationSpeed = speed;
}

const float4x4& ModuleEngineCamera::GetProjectionMatrix() const
{
	return projectionMatrix;
}

const float4x4& ModuleEngineCamera::GetViewMatrix() const
{
	return viewMatrix;
}

float ModuleEngineCamera::GetHFOV() const
{
	return math::RadToDeg(frustum.HorizontalFov());
}

float ModuleEngineCamera::GetVFOV() const
{
	return math::RadToDeg(frustum.VerticalFov());
}

float ModuleEngineCamera::GetZNear() const
{
	return frustum.NearPlaneDistance();
}

float ModuleEngineCamera::GetZFar() const
{
	return frustum.FarPlaneDistance();
}

float ModuleEngineCamera::GetMoveSpeed() const
{
	return moveSpeed;
}

float ModuleEngineCamera::GetRotationSpeed() const
{
	return rotationSpeed;
}

float ModuleEngineCamera::GetDistance(const float3& point) const
{
	return frustum.Pos().Distance(point);
}
