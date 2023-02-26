#include "CameraEngine.h"

#include "Application.h"
#include "ModuleWindow.h"
#include "ModuleInput.h"
#include "ModuleRender.h"
#include "ModuleEditor.h"
#include "ModuleScene.h"

#include "Scene/Scene.h"

#include "GameObject/GameObject.h"

#include "Components/ComponentTransform.h"
#include "Components/ComponentBoundingBoxes.h"
#include "Components/ComponentMeshRenderer.h"

#include "Resources/ResourceMesh.h"

#include "Windows/EditorWindows/WindowScene.h"

#include "Math/float3x3.h"
#include "Math/Quat.h"
#include "Geometry/Sphere.h"
#include "Geometry/Triangle.h"

CameraEngine::CameraEngine() 
: Camera(CameraType::C_ENGINE)
{
};

CameraEngine::~CameraEngine()
{
};

bool CameraEngine::Init()
{
	int w, h;
	SDL_GetWindowSize(App->window->GetWindow(), &w, &h);
	aspectRatio = float(w) / h;

	viewPlaneDistance = DEFAULT_FRUSTUM_DISTANCE;
	frustum->SetKind(FrustumProjectiveSpace::FrustumSpaceGL, FrustumHandedness::FrustumRightHanded);
	frustum->SetViewPlaneDistances(0.1f, viewPlaneDistance);
	frustum->SetHorizontalFovAndAspectRatio(math::DegToRad(90), aspectRatio);

	acceleration = DEFAULT_SHIFT_ACCELERATION;
	moveSpeed = DEFAULT_MOVE_SPEED;
	rotationSpeed = DEFAULT_ROTATION_SPEED;
	mouseSpeedModifier = DEFAULT_MOUSE_SPEED_MODIFIER;
	frustumMode = DEFAULT_FRUSTUM_MODE;
	frustumOffset = DEFAULT_FRUSTUM_OFFSET;

	position = float3(0.f, 2.f, 5.f);

	frustum->SetPos(position);
	frustum->SetFront(-float3::unitZ);
	frustum->SetUp(float3::unitY);

	if (frustumMode == EFrustumMode::offsetFrustum)
	{
		RecalculateOffsetPlanes();
	}

	return true;
}

bool CameraEngine::Start()
{
	return true;
}

bool CameraEngine::Update()
{

	projectionMatrix = frustum->ProjectionMatrix();
	viewMatrix = frustum->ViewMatrix();

	App->input->SetDefaultCursor();

#ifdef ENGINE
	bool sceneFocused = App->editor->IsSceneFocused();
#else
	bool sceneFocused = true;
#endif // ENGINE

	if (sceneFocused)
	{
		//We block everything on while Focus (slerp) to avoid camera problems
		if (isFocusing)
		{
			if (focusFlag) Focus(App->scene->GetSelectedGameObject());
			Rotate();
		}
		else
		{
			//Shift speed
			if (App->input->GetKey(SDL_SCANCODE_LSHIFT) != KeyState::IDLE)
				Run();
			else
				Walk();

#ifdef ENGINE
			//this should probably be encapsulated in a method, or moved to the Physics part of the Engine
			// --RAYCAST CALCULATION-- //
			if (App->input->GetMouseButton(SDL_BUTTON_LEFT) != KeyState::IDLE
				&& App->input->GetKey(SDL_SCANCODE_LALT) == KeyState::IDLE)
			{
				const WindowScene* windowScene = App->editor->GetScene();
				LineSegment ray = CreateRaycastFromMousePosition(windowScene);
				CalculateHittedGameObjects(ray);
			}
			// --RAYCAST CALCULATION-- //
#endif // ENGINE

			//Move and rotate with right buttons and ASDWQE
			if (App->input->GetMouseButton(SDL_BUTTON_RIGHT) != KeyState::IDLE &&
				App->input->GetKey(SDL_SCANCODE_LALT) == KeyState::IDLE)
			{
				focusFlag = false;
				App->input->SetFreeLookCursor();
				UnlimitedCursor();
				Move();
				FreeLook();
			}

			//Zoom with mouse wheel
			if (App->input->IsMouseWheelScrolled() && App->input->GetMouseButton(SDL_BUTTON_RIGHT) == KeyState::IDLE)
			{
				focusFlag = false;
				Zoom();
			}

			//Move camera UP/DOWN and RIGHT/LEFT with mouse mid button
			if (App->input->GetMouseButton(SDL_BUTTON_MIDDLE) != KeyState::IDLE)
			{
				focusFlag = false;
				App->input->SetMoveCursor();
				UnlimitedCursor();
				Move();
			}

			//Focus
			if (App->scene->GetSelectedGameObject() != App->scene->GetLoadedScene()->GetRoot() &&
				App->input->GetKey(SDL_SCANCODE_F) != KeyState::IDLE)
			{
				focusFlag = true;
				isFocusing = true;
			}

			//Orbit object with ALT + LEFT MOUSE CLICK
			if (App->scene->GetSelectedGameObject() != App->scene->GetLoadedScene()->GetRoot() &&
				App->input->GetKey(SDL_SCANCODE_LALT) != KeyState::IDLE &&
				App->input->GetMouseButton(SDL_BUTTON_LEFT) != KeyState::IDLE)
			{
				const OBB& obb = static_cast<ComponentBoundingBoxes*>(
					App->scene->GetSelectedGameObject()->GetComponent(ComponentType::BOUNDINGBOX))->GetObjectOBB();
				focusFlag = false;
				App->input->SetOrbitCursor();
				UnlimitedCursor();
				Orbit(obb);
			}

			//Zoom with ALT + RIGHT MOUSE CLICK
			if (App->input->GetKey(SDL_SCANCODE_LALT) != KeyState::IDLE &&
				App->input->GetMouseButton(SDL_BUTTON_RIGHT) != KeyState::IDLE &&
				App->input->GetMouseButton(SDL_BUTTON_LEFT) == KeyState::IDLE) //Not pressing mouse left button
			{
				App->input->SetZoomCursor();
				UnlimitedCursor();
				Zoom();
			}

			KeyboardRotate();

			if (frustumMode == EFrustumMode::offsetFrustum)
			{
				RecalculateOffsetPlanes();
			}
		}
	}

	return UPDATE_CONTINUE;
}

void CameraEngine::Move()
{
	float deltaTime = App->GetDeltaTime();

	//Increase/decrease camera velocity with mouse wheel
	if (App->input->IsMouseWheelScrolled() && App->input->GetMouseButton(SDL_BUTTON_MIDDLE) == KeyState::IDLE)
	{
		moveSpeed += App->input->GetMouseWheel().y;
		if (moveSpeed < 1.0f)
			moveSpeed = 1.0f;
		if (moveSpeed > 900.0f)
			moveSpeed = 900.0f;
	}

	//Forward
	if (App->input->GetKey(SDL_SCANCODE_W) != KeyState::IDLE)
	{
		position += frustum->Front().Normalized() *
			moveSpeed * acceleration * deltaTime;
		SetPosition(position);
	}

	//Backward
	if (App->input->GetKey(SDL_SCANCODE_S) != KeyState::IDLE)
	{
		position += -(frustum->Front().Normalized()) *
			moveSpeed * acceleration * deltaTime;
		SetPosition(position);
	}

	//Left
	if (App->input->GetKey(SDL_SCANCODE_A) != KeyState::IDLE)
	{
		position += -(frustum->WorldRight()) * moveSpeed * acceleration * deltaTime;
		SetPosition(position);
	}

	//Right
	if (App->input->GetKey(SDL_SCANCODE_D) != KeyState::IDLE)
	{
		position += frustum->WorldRight() * moveSpeed * acceleration * deltaTime;
		SetPosition(position);
	}

	//Up
	if (App->input->GetKey(SDL_SCANCODE_E) != KeyState::IDLE)
	{
		position += frustum->Up() * moveSpeed * acceleration * deltaTime;
		SetPosition(position);
	}

	//Down
	if (App->input->GetKey(SDL_SCANCODE_Q) != KeyState::IDLE)
	{
		position += -(frustum->Up()) * moveSpeed * acceleration * deltaTime;
		SetPosition(position);
	}

	//Move UP/DOWN and RIGHT/LEFT with mid mouse button
	if (App->input->GetMouseButton(SDL_BUTTON_MIDDLE) != KeyState::IDLE)
	{
		float deltaTime = App->GetDeltaTime();
		float mouseSpeedPercentage = 0.05f;
		float xrel = -App->input->GetMouseMotion().x * (rotationSpeed * mouseSpeedPercentage) * deltaTime;
		float yrel = App->input->GetMouseMotion().y * (rotationSpeed * mouseSpeedPercentage) * deltaTime;

		position += (frustum->WorldRight()) * xrel;
		position += (frustum->Up()) * yrel;
		SetPosition(position);
	}
}

void CameraEngine::KeyboardRotate()
{
	float yaw = 0.f, pitch = 0.f;

	float rotationAngle = RadToDeg(frustum->Front().Normalized().AngleBetween(float3::unitY));

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
	Quat pitchQuat(frustum->WorldRight(), pitch * deltaTime * rotationSpeed * acceleration);
	Quat yawQuat(float3::unitY, yaw * deltaTime * rotationSpeed * acceleration);

	float3x3 rotationMatrixX = float3x3::FromQuat(pitchQuat);
	float3x3 rotationMatrixY = float3x3::FromQuat(yawQuat);
	float3x3 rotationDeltaMatrix = rotationMatrixY * rotationMatrixX;

	ApplyRotation(rotationDeltaMatrix);
}

void CameraEngine::FreeLook()
{
	float deltaTime = App->GetDeltaTime();
	float mouseSpeedPercentage = 0.05f;
	float xrel = -App->input->GetMouseMotion().x * (rotationSpeed * mouseSpeedPercentage) * deltaTime;
	float yrel = -App->input->GetMouseMotion().y * (rotationSpeed * mouseSpeedPercentage) * deltaTime;

	float3x3 x = float3x3(Cos(xrel), 0.0f, Sin(xrel), 0.0f, 1.0f, 0.0f, -Sin(xrel), 0.0f, Cos(xrel));
	float3x3 y = float3x3::RotateAxisAngle(frustum->WorldRight().Normalized(), yrel);
	float3x3 xy = x * y;

	vec oldUp = frustum->Up().Normalized();
	vec oldFront = frustum->Front().Normalized();

	float3 newUp = xy.MulDir(oldUp);

	if (newUp.y > 0.f)
	{
		frustum->SetUp(xy.MulDir(oldUp));
		frustum->SetFront(xy.MulDir(oldFront));
	}
	else
	{
		y = float3x3::RotateAxisAngle(frustum->WorldRight().Normalized(), 0);
		xy = x * y;

		frustum->SetUp(xy.MulDir(oldUp));
		frustum->SetFront(xy.MulDir(oldFront));
	}
}

void CameraEngine::Run()
{
	acceleration = DEFAULT_SHIFT_ACCELERATION;
}

void CameraEngine::Walk()
{
	acceleration = 1.f;
}

void CameraEngine::Zoom()
{
	float deltaTime = App->GetDeltaTime();
	if (App->input->IsMouseWheelScrolled())
	{
		float zoomSpeed = App->input->GetMouseWheel().y * DEFAULT_MOUSE_ZOOM_SPEED;

		position += frustum->Front().Normalized() *
			zoomSpeed * deltaTime;
	}
	else
	{
		float zoomSpeed = App->input->GetMouseMotion().x * DEFAULT_MOUSE_ZOOM_SPEED;

		position += frustum->Front().Normalized() *
			zoomSpeed * deltaTime;
	}
	SetPosition(position);
}

void CameraEngine::Focus(const OBB& obb)
{
	Sphere boundingSphere = obb.MinimalEnclosingSphere();

	float radius = boundingSphere.r;
	if (boundingSphere.r < 1.f) radius = 1.f;
	float fov = frustum->HorizontalFov();
	float camDistance = radius / sin(fov / 2.0);
	vec camDirection = (boundingSphere.pos - frustum->Pos()).Normalized();

	//position = boundingSphere.pos - (camDirection * camDistance);

	float3 endposition = boundingSphere.pos - (camDirection * camDistance);
	position = position.Lerp(endposition, App->GetDeltaTime() * rotationSpeed);

	SetPosition(position);
	SetLookAt(boundingSphere.pos);
}

void CameraEngine::Focus(GameObject* gameObject)
{
	std::list<GameObject*> insideGameObjects = gameObject->GetGameObjectsInside();
	AABB minimalAABB;
	std::vector<math::vec> outputArray{};
	for (GameObject* object : insideGameObjects)
	{
		if (object)
		{
			ComponentBoundingBoxes* boundingBox =
				static_cast<ComponentBoundingBoxes*>(object->GetComponent(ComponentType::BOUNDINGBOX));
			outputArray.push_back(boundingBox->GetEncapsuledAABB().minPoint);
			outputArray.push_back(boundingBox->GetEncapsuledAABB().maxPoint);
		}
	}
	minimalAABB = minimalAABB.MinimalEnclosingAABB(outputArray.data(), (int)outputArray.size());

	Focus(minimalAABB);
}

void CameraEngine::Orbit(const OBB& obb)
{

	FreeLook();

	float3 posToOrbit = obb.CenterPoint();

	vec camDirection = frustum->Front().Normalized();
	float camDistance = posToOrbit.Distance(frustum->Pos());

	float3 newPosition = posToOrbit - (camDirection * camDistance);

	SetPosition(newPosition);
}

void CameraEngine::UnlimitedCursor()
{
	int mouseX, mouseY;
	SDL_GetMouseState(&mouseX, &mouseY);

	if (mouseWarped)
	{
		App->input->SetMouseMotionX(mouseX - lastMouseX);
		App->input->SetMouseMotionY(mouseY - lastMouseY);
		mouseWarped = false;
	}
	int width, height;
	SDL_GetWindowSize(App->window->GetWindow(), &width, &height);
	if (mouseX <= 0)
	{
		lastMouseX = width - 1;
		lastMouseY = mouseY;
		SDL_WarpMouseInWindow(App->window->GetWindow(), width - 1, mouseY);
		mouseWarped = true;
	}
	if (mouseX >= width - 1)
	{
		lastMouseX = 0;
		lastMouseY = mouseY;
		SDL_WarpMouseInWindow(App->window->GetWindow(), 0, mouseY);
		mouseWarped = true;
	}
	if (mouseY <= 0)
	{
		lastMouseX = mouseX;
		lastMouseY = height - 1;
		SDL_WarpMouseInWindow(App->window->GetWindow(), mouseX, height - 1);
		mouseWarped = true;
	}
	if (mouseY >= height - 1)
	{
		lastMouseX = mouseX;
		lastMouseY = 0;
		SDL_WarpMouseInWindow(App->window->GetWindow(), mouseX, 0);
		mouseWarped = true;
	}
}

void CameraEngine::Rotate()
{
	float yaw = 0.f, pitch = 0.f;

	float rotationAngle = RadToDeg(frustum->Front().Normalized().AngleBetween(float3::unitY));

	float deltaTime = App->GetDeltaTime();
	Quat pitchQuat(frustum->WorldRight(), pitch * deltaTime * rotationSpeed * acceleration);
	Quat yawQuat(float3::unitY, yaw * deltaTime * rotationSpeed * acceleration);

	float3x3 rotationMatrixX = float3x3::FromQuat(pitchQuat);
	float3x3 rotationMatrixY = float3x3::FromQuat(yawQuat);
	float3x3 rotationDeltaMatrix = rotationMatrixY * rotationMatrixX;

	ApplyRotation(rotationDeltaMatrix);
}